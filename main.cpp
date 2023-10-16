#include "mbed.h"
#include "SCD4.hpp"
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

#define READ_BYTE_LEN 9

I2C i2c(P1_I2C_SDA, P1_I2C_SCL);
const char addr = (0x62 << 1);

int counter = 0;

int main()
{
    led_thread.start(led_thread_func);

    for (int i = 1; i < 10; i++)
        printf("a\n");

    scd4x_measurement_t data;
    SCD4::ErrorType error;
    SCD4 scd4(P1_I2C_SDA, P1_I2C_SCL);
    error = scd4.start_periodic_measurement();
    ThisThread::sleep_for(BLINKING_RATE);
    error = scd4.read_measurement(&data);
    // error = scd4.stop_periodic_measurement();

    // scd4.start_low_power_periodic_measurement();
    while (true)
    {
        // int local_counter=0;
        // while (scd4.get_data_ready_status() == SCD4::ErrorType::DataNotReady)
        // {
        //     printf("wait: %ds\n",local_counter);
        //     ThisThread::sleep_for(1000ms);
        //     local_counter++;
        // }
        ThisThread::sleep_for(BLINKING_RATE);
        error = scd4.read_measurement(&data);
        if (error != SCD4::ErrorType::Ok)
        {
            printf("ERRRRRROR read measurement I2C\n");
        }
        printf(" -- counter:%d", counter);
        printf(" -- CO2 ppm:%d|temp:%f °C|RH:%f %%\n", data.co2, data.temperature, data.rh);

        counter++;
        // led.write(button.read());
    }
}
