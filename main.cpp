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
    while (true) {
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

    for (int i = 1; i < 50; i++)
            printf("\n");
    while (true)
    {
        // TODO lock bus
        // TODO check CRC
        // TODO U16_TO_BYTE_ARRAY pour les commandes
        // TODO BYTE_TO_U16
        char start_command[2] = {0x21, 0xb1};
        char read_command[2] = {0xec, 0x05};

        i2c.lock();
        i2c.write(addr, start_command, 2);
        ThisThread::sleep_for(BLINKING_RATE);
        i2c.write(addr, read_command, 2);
        char value[READ_BYTE_LEN] = {0};
        i2c.read(addr, value, READ_BYTE_LEN);
        i2c.unlock();

        printf("i2c return : 0x%02X", value[0]);
        for (int i = 1; i < READ_BYTE_LEN; i++)
            printf("|0x%02X", value[i]);
        printf(" -- counter:%d", counter);
        uint16_t CO2 = 0;
        float temperature = 0.0;
        float RH = 0.0;
        decode_SCD4_read(value, &CO2, &temperature, &RH);

        printf(" -- CO2 ppm:%d|temp:%f °C|RH:%f %%\n", CO2,temperature,RH);
        ThisThread::sleep_for(BLINKING_RATE);
        scd4x_measurement_t data;
        SCD4::ErrorType error;
        SCD4 scd4(P1_I2C_SDA, P1_I2C_SCL);
        error=scd4.read_measurement(&data);
        if(error!=SCD4::ErrorType::Ok)
        {
            printf("ERRRRRROR I2C\n");
        }
        printf(" -- counter:%d", counter);
        printf(" -- CO2 ppm:%d|temp:%f °C|RH:%f %%\n", data.co2,data.temperature,data.rh);

        counter++;
        // led.write(button.read());
    }
}
