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

#define READ_BYTE_LEN 9

I2C i2c(P1_I2C_SDA, P1_I2C_SCL);
const char addr = (0x62 << 1);

int counter = 0;

void decode_SCD4_read(char *data, uint16_t *CO2ppm, float *temp, float *RH)
{
    *CO2ppm = (data[0] << 8) + data[1];
    uint16_t a = (data[3] << 8) + data[4];
    // FIXME offset
    int offset = -45;
    *temp = (float)offset + (float)175 * (a / (float)(1 << 16));
    a = (data[6] << 8) + data[7];
    *RH = (float)100 * (a / (float)(1 << 16));
}

int main()
{
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

        i2c.write(addr, start_command, 2);
        ThisThread::sleep_for(BLINKING_RATE);
        i2c.write(addr, read_command, 2);
        char value[READ_BYTE_LEN] = {0};
        i2c.read(addr, value, READ_BYTE_LEN);

        printf("i2c return : 0x%02X", value[0]);
        for (int i = 1; i < READ_BYTE_LEN; i++)
            printf("|0x%02X", value[i]);
        printf(" -- counter:%d", counter);
        uint16_t CO2 = 0;
        float temperature = 0.0;
        float RH = 0.0;
        decode_SCD4_read(value, &CO2, &temperature, &RH);

        printf(" -- CO2 ppm:%d|temp:%f °C|RH:%f %%\n", CO2,temperature,RH);

        counter++;
        // led.write(button.read());
    }
}
