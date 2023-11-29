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
scd4x_measurement_t data;
SCD4 scd4(P1_I2C_SDA, P1_I2C_SCL);

lora_app my_lora_app;

void measurement_func(){

}

int main()
{
    printf("\n\n\n\n\n\n\n\n\n\n\n");
    DigitalOut ant_pin(LORA_ANTSW_PWR,1);
    printf("LORA_ANTSW_PWR set to 1\n");

    led_thread.start(led_thread_func);
    printf("led_thread started\n");

    my_lora_app.init();
    my_lora_app.connect();


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

    my_lora_app.dispatch_queue_forever();
}

void on_connect(){
    printf("LORA connected\n");
}

void on_disconnect(){
    printf("LORA disconnected\n");
}

void on_tx(){
    printf("LORA TX done\n");
}

void on_rx(){
    printf("LORA RX done\n");
}