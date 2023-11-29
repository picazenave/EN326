#include "lora_app.hpp"

static void lora_event_handler(lorawan_event_t event);
// static void send_message();
// static void receive_message();

SX1272_LoRaRadio radio(LORA_MOSI, LORA_MISO, LORA_SCK, LORA_CS, LORA_RESET, LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5);
LoRaWANInterface lorawan(radio);

// Max payload size can be LORAMAC_PHY_MAXPAYLOAD.
// This example only communicates with much shorter messages (<30 bytes).
// If longer messages are used, these buffers must be changed accordingly.
uint8_t tx_buffer[40];
uint8_t rx_buffer[30];

static lorawan_app_callbacks_t callbacks;

lora_app::lora_app(/* args */)
{
}

lora_app::~lora_app()
{
}

void lora_app::dispatch_queue_forever()
{
    // make your event queue dispatching events forever
    ev_queue.dispatch_forever();
}

void lora_app::init()
{
    // Initialize LoRaWAN stack
    if (lorawan.initialize(&ev_queue) != LORAWAN_STATUS_OK)
    {
        printf("\r\nLoRa initialization failed! \r\n");
        while (1)
        {
            printf("\r\nIN ERROR \r\n");
            ThisThread::sleep_for(BLINKING_RATE);
        }
    }

    printf("\r\nMbed LoRaWANStack initialized \r\n");

    // prepare application callbacks
    callbacks.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&callbacks);

    // Set number of retries in case of CONFIRMED messages
    if (lorawan.set_confirmed_msg_retries(CONFIRMED_MSG_RETRY_COUNTER) != LORAWAN_STATUS_OK)
    {
        printf("\r\nset_confirmed_msg_retries failed! \r\n\r\n");
        while (1)
        {
            printf("\r\nIN ERROR \r\n");
            ThisThread::sleep_for(BLINKING_RATE);
        }
    }

    printf("\r\nCONFIRMED message retries : %d \r\n",
           CONFIRMED_MSG_RETRY_COUNTER);

    // Enable adaptive data rate
    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK)
    {
        printf("\r\nenable_adaptive_datarate failed! \r\n");
        while (1)
        {
            printf("\r\nIN ERROR \r\n");
            ThisThread::sleep_for(BLINKING_RATE);
        }
    }

    printf("\r\nAdaptive data  rate (ADR) - Enabled \r\n");

    printf("\r\nInit done ...\r\n");
}

void lora_app::connect()
{
    retcode = lorawan.connect();

    if (retcode == LORAWAN_STATUS_OK ||
        retcode == LORAWAN_STATUS_CONNECT_IN_PROGRESS)
    {
        printf("\r\nConnection - In Progress ...\r\n");
    }
    else
    {
        printf("\r\nConnection error, code = %d \r\n", retcode);
        while (1)
        {
            printf("\r\nIN ERROR \r\n");
            ThisThread::sleep_for(BLINKING_RATE);
        }
    }
}

void lora_app::disconnect()
{
    retcode = lorawan.disconnect();

    if (retcode == LORAWAN_STATUS_OK)
    {
        printf("\r\nDisconnection - In Progress ...\r\n");
    }
    else
    {
        printf("\r\nDisconnection error, code = %d \r\n", retcode);
        while (1)
        {
            printf("\r\nIN ERROR \r\n");
            ThisThread::sleep_for(BLINKING_RATE);
        }
    }
}

extern scd4x_measurement_t data;
/**
 * Sends a message to the Network Server
 */
void lora_app::send_message()
{
    uint16_t packet_len;
    int16_t retcode;

    packet_len = sprintf((char *)tx_buffer, "{\"T\":%f,\"H\":%f,\"C02\":%d}",
                         data.temperature, data.rh, data.co2);
    printf("\r\npacket len = %d || %s \r\n", packet_len, tx_buffer);
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
                ev_queue.call_in(5000ms, send_message);
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
void lora_app::receive_message()
{
    uint8_t port;
    int flags;
    int16_t retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);

    if (retcode < 0)
    {
        printf("\r\nreceive() - Error code %d \r\n", retcode);
        return;
    }

    printf("RX Data on port %u (%d bytes): ", port, retcode);
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
        printf("\r\nConnection - Successful \r\n");
        if (MBED_CONF_LORA_DUTY_CYCLE_ON)
        {
            lora_app::send_message();
        }
        else
        {
            ev_queue.call_every(TX_TIMER, lora_app::send_message);
        }
        on_connect();
        break;
    case DISCONNECTED:
        ev_queue.break_dispatch();
        printf("\r\nDisconnected Successfully \r\n");
        on_disconnect();
        break;
    case TX_DONE:
        printf("\r\n Message Sent to Network Server \r\n");
        if (MBED_CONF_LORA_DUTY_CYCLE_ON)
        {
            lora_app::send_message();
        }
        on_tx();
        break;
    case TX_TIMEOUT:
        printf("\r\nLORA TX_TIMEOUT \r\n");
        break;
    case TX_ERROR:
        printf("\r\nLORA TX_ERROR \r\n");
        break;
    case TX_CRYPTO_ERROR:
        printf("\r\nLORA TX_CRYPTO_ERROR \r\n");
        break;
    case TX_SCHEDULING_ERROR:
        printf("\r\nTransmission Error - EventCode = %d \r\n", event);
        // try again
        if (MBED_CONF_LORA_DUTY_CYCLE_ON)
        {
            lora_app::send_message();
        }
        break;
    case RX_DONE:
        printf("\r\nReceived message from Network Server \r\n");
        lora_app::receive_message();
        on_rx();
        break;
    case RX_TIMEOUT:
        printf("\r\nLORA RX_TIMEOUT \r\n");
        break;
    case RX_ERROR:
        printf("\r\nError in reception - Code = %d \r\n", event);
        break;
    case JOIN_FAILURE:
        printf("\r\nOTAA Failed - Check Keys \r\n");
        break;
    case UPLINK_REQUIRED:
        printf("\r\nUplink required by NS \r\n");
        if (MBED_CONF_LORA_DUTY_CYCLE_ON)
        {
            lora_app::send_message();
        }
        break;
    default:
        MBED_ASSERT("Unknown Event");
    }
}