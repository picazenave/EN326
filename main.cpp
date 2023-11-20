#include "mbed.h"
#include "SCD4.hpp"
#include "SX1272_LoRaRadio.h"
#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"
using namespace events;
// Blinking rate in milliseconds
#define BLINKING_RATE 5500ms

#ifdef LED1
DigitalOut led(LED1);
#else
#error LED1 not defined
#endif

#ifdef BUTTON1
DigitalIn button(BUTTON1);
// InterruptIn button(BUTTON1);
#else
#error BUTTON1 not defined
#endif

Thread led_thread;
void led_thread_func()
{
    while (true)
    {
        led = !led;
        ThisThread::sleep_for(300ms);
    }
}

static void lora_event_handler(lorawan_event_t event);
static void send_message();
static void receive_message();

SX1272_LoRaRadio radio(LORA_MOSI, LORA_MISO, LORA_SCK, LORA_CS, LORA_RESET, LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5, LORA_ANTSW_PWR);
LoRaWANInterface lorawan(radio);
// Max payload size can be LORAMAC_PHY_MAXPAYLOAD.
// This example only communicates with much shorter messages (<30 bytes).
// If longer messages are used, these buffers must be changed accordingly.
uint8_t tx_buffer[30];
uint8_t rx_buffer[30];

static lorawan_app_callbacks_t callbacks;
/*
 * Sets up an application dependent transmission timer in ms. Used only when Duty Cycling is off for testing
 */
#define TX_TIMER 5000ms
/**
 * Maximum number of events for the event queue.
 * 10 is the safe number for the stack events, however, if application
 * also uses the queue for whatever purposes, this number should be increased.
 */
#define MAX_NUMBER_OF_EVENTS 10
/**
 * Maximum number of retries for CONFIRMED messages before giving up
 */
#define CONFIRMED_MSG_RETRY_COUNTER 3
/**
 * This event queue is the global event queue for both the
 * application and stack. To conserve memory, the stack is designed to run
 * in the same thread as the application and the application is responsible for
 * providing an event queue to the stack that will be used for ISR deferment as
 * well as application information event queuing.
 */
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS *EVENTS_EVENT_SIZE);

#define READ_BYTE_LEN 9

I2C i2c(P1_I2C_SDA, P1_I2C_SCL);
const char addr = (0x62 << 1);

int counter = 0;
scd4x_measurement_t data;
lorawan_status_t retcode;
int main()
{
    led_thread.start(led_thread_func);

    // stores the status of a call to LoRaWAN protocol
    

    // Initialize LoRaWAN stack
    if (lorawan.initialize(&ev_queue) != LORAWAN_STATUS_OK)
    {
        printf("\r\n LoRa initialization failed! \r\n");
        return -1;
    }

    printf("\r\n Mbed LoRaWANStack initialized \r\n");

    // prepare application callbacks
    callbacks.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&callbacks);

    // Set number of retries in case of CONFIRMED messages
    if (lorawan.set_confirmed_msg_retries(CONFIRMED_MSG_RETRY_COUNTER) != LORAWAN_STATUS_OK)
    {
        printf("\r\n set_confirmed_msg_retries failed! \r\n\r\n");
        return -1;
    }

    printf("\r\n CONFIRMED message retries : %d \r\n",
           CONFIRMED_MSG_RETRY_COUNTER);

    // Enable adaptive data rate
    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK)
    {
        printf("\r\n enable_adaptive_datarate failed! \r\n");
        return -1;
    }

    printf("\r\n Adaptive data  rate (ADR) - Enabled \r\n");

    retcode = lorawan.connect();

    if (retcode == LORAWAN_STATUS_OK ||
        retcode == LORAWAN_STATUS_CONNECT_IN_PROGRESS)
    {
    }
    else
    {
        printf("\r\n Connection error, code = %d \r\n", retcode);
        return -1;
    }

    printf("\r\n Connection - In Progress ...\r\n");

    // make your event queue dispatching events forever
    ev_queue.dispatch_forever();

    for (int i = 1; i < 10; i++)
        printf("a\n");

    
    SCD4::ErrorType error;
    SCD4 scd4(P1_I2C_SDA, P1_I2C_SCL);
    error = scd4.start_periodic_measurement();
    ThisThread::sleep_for(BLINKING_RATE);
    error = scd4.read_measurement(&data);

    while (true)
    {
        ThisThread::sleep_for(BLINKING_RATE);
        error = scd4.read_measurement(&data);
        if (error != SCD4::ErrorType::Ok)
        {
            printf("ERRRRRROR read measurement I2C\n");
        }
        printf(" -- counter:%d", counter);
        printf(" -- CO2 ppm:%d|temp:%f °C|RH:%f %%\n", data.co2, data.temperature, data.rh);

        counter++;
    }
}

/**
 * Sends a message to the Network Server
 */
static void send_message()
{
    uint16_t packet_len;
    int16_t retcode;
    int32_t sensor_value;

    // TODO mettre sensor
    sensor_value = 69;//data.temperature;
    printf("\r\n Dummy Sensor Value = %d \r\n", sensor_value);

    packet_len = sprintf((char *)tx_buffer, "Dummy Sensor Value is %d",
                         sensor_value);

    retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, packet_len,
                           MSG_UNCONFIRMED_FLAG);

    if (retcode < 0)
    {
        retcode == LORAWAN_STATUS_WOULD_BLOCK ? printf("send - WOULD BLOCK\r\n")
                                              : printf("\r\n send() - Error code %d \r\n", retcode);

        if (retcode == LORAWAN_STATUS_WOULD_BLOCK)
        {
            // retry in 3 seconds
            if (MBED_CONF_LORA_DUTY_CYCLE_ON)
            {
                ev_queue.call_in(3000ms, send_message);
            }
        }
        return;
    }

    printf("\r\n %d bytes scheduled for transmission \r\n", retcode);
    memset(tx_buffer, 0, sizeof(tx_buffer));
}

/**
 * Receive a message from the Network Server
 */
static void receive_message()
{
    uint8_t port;
    int flags;
    int16_t retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);

    if (retcode < 0)
    {
        printf("\r\n receive() - Error code %d \r\n", retcode);
        return;
    }

    printf(" RX Data on port %u (%d bytes): ", port, retcode);
    for (uint8_t i = 0; i < retcode; i++)
    {
        printf("%02x ", rx_buffer[i]);
    }
    printf("\r\n");

    memset(rx_buffer, 0, sizeof(rx_buffer));
}

/**
 * Event handler
 */
static void lora_event_handler(lorawan_event_t event)
{
    switch (event)
    {
    case CONNECTED:
        printf("\r\n Connection - Successful \r\n");
        if (MBED_CONF_LORA_DUTY_CYCLE_ON)
        {
            send_message();
        }
        else
        {
            ev_queue.call_every(TX_TIMER, send_message);
        }

        break;
    case DISCONNECTED:
        ev_queue.break_dispatch();
        printf("\r\n Disconnected Successfully \r\n");
        break;
    case TX_DONE:
        printf("\r\n Message Sent to Network Server \r\n");
        if (MBED_CONF_LORA_DUTY_CYCLE_ON)
        {
            send_message();
        }
        break;
    case TX_TIMEOUT:
    case TX_ERROR:
    case TX_CRYPTO_ERROR:
    case TX_SCHEDULING_ERROR:
        printf("\r\n Transmission Error - EventCode = %d \r\n", event);
        // try again
        if (MBED_CONF_LORA_DUTY_CYCLE_ON)
        {
            send_message();
        }
        break;
    case RX_DONE:
        printf("\r\n Received message from Network Server \r\n");
        receive_message();
        break;
    case RX_TIMEOUT:
    case RX_ERROR:
        printf("\r\n Error in reception - Code = %d \r\n", event);
        break;
    case JOIN_FAILURE:
        printf("\r\n OTAA Failed - Check Keys \r\n");
        break;
    case UPLINK_REQUIRED:
        printf("\r\n Uplink required by NS \r\n");
        if (MBED_CONF_LORA_DUTY_CYCLE_ON)
        {
            send_message();
        }
        break;
    default:
        MBED_ASSERT("Unknown Event");
    }
}
