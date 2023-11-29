#ifndef LORA_APP_H
#define LORA_APP_H

#include "mbed.h"
#include "SCD4.hpp"
#include "SX1272_LoRaRadio.h"
#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"
#include "main.hpp"

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
#define BLINKING_RATE 5500ms

class lora_app
{
private:
    /* data */
public:

    lora_app();
    void init();
    void dispatch_queue_forever();
    void connect();
    void disconnect();
    static void send_message();
    static void receive_message();
    ~lora_app();
};
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS *EVENTS_EVENT_SIZE);

#endif