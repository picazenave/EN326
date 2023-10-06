/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"

// Blinking rate in milliseconds
#define BLINKING_RATE 500ms

int main()
{
    // Initialise the digital pin LED1 as an output
#ifdef LED1
    DigitalOut led(LED1);
#else
#error LED1 not defined
#endif

#ifdef BUTTON1
    DigitalIn button(BUTTON1);
#else
#error BUTTON1 not defined
#endif

    while (true)
    {
        printf("aaaaaaaaaaaaaaaaa\n");
        led.write(button.read());
        // ThisThread::sleep_for(BLINKING_RATE);
    }
}
