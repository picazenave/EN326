#include "mbed.h"
#include "SCD4.hpp"
#include "lora_app.hpp"

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

I2C i2c(P1_I2C_SDA, P1_I2C_SCL);
const char addr = (0x62 << 1);

int counter = 0;
scd4x_measurement_t data;
SCD4 scd4(P1_I2C_SDA, P1_I2C_SCL);

lora_app my_lora_app;

Thread led_thread;
uint8_t led_thread_counter = 0;
void led_thread_func()
{
    while (true)
    {
        led = !led;
        ThisThread::sleep_for(250ms);
        led_thread_counter++;
        if (led_thread_counter == 21) // make measurement every 5250ms
        {
            led_thread_counter = 0;
            SCD4::ErrorType error;
            do // retry if not valid
            {
                error = scd4.read_measurement(&data);
                if (error != SCD4::ErrorType::Ok)
                {
                    printf("ERRRRRROR read measurement I2C\n");
                }
                ThisThread::sleep_for(50ms);
            } while (data.rh < 1 || data.co2 < 100);
            printf("-- CO2 ppm:%d|temp:%f °C|RH:%f %%\n", data.co2, data.temperature, data.rh);
        }
    }
}

int main()
{
    DigitalOut _pin(LORA_ANTSW_PWR, 1);
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("LORA_ANTSW_PWR set to 1\n");

    SCD4::ErrorType error;
    error = scd4.start_periodic_measurement();
    ThisThread::sleep_for(BLINKING_RATE);
    error = scd4.read_measurement(&data);
    if (error != SCD4::ErrorType::Ok)
    {
        printf("ERRRRRROR read measurement I2C\n");
    }
    printf("-- CO2 ppm:%d|temp:%f°C|RH:%f%%\n", data.co2, data.temperature, data.rh);
    printf("scd4 init done\n");

    led_thread.start(led_thread_func);
    printf("led_thread started\n");

    my_lora_app.init();
    my_lora_app.connect();

    my_lora_app.dispatch_queue_forever();
}

void on_connect()
{
    printf("LORA connected\n");
}

void on_disconnect()
{
    printf("LORA disconnected\n");
}

void on_tx()
{
    printf("LORA TX done\n");
}

void on_rx()
{
    printf("LORA RX done\n");
}