#include "mbed.h"

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

void decodeSCD4(char *data, int *CO2ppm, float *temp, float *RH)
{
    *CO2ppm=data[0];
}

int main()
{
    while (true)
    {
        char start_command[2] = {0x21, 0xb1};
        char read_command[2] = {0xec, 0x05};

        i2c.write(addr, start_command, 2);
        ThisThread::sleep_for(BLINKING_RATE);
        i2c.write(addr, read_command, 2);
        char value[6] = {0};
        i2c.read(addr, value, 6);

        printf("i2c return : 0x%02X", value[0]);
        for (int i = 0; i < 6; i++)
            printf("|0x%02X", value[i]);
        printf(" -- counter:%d\n", counter);
        counter++;
        // led.write(button.read());
    }
}
