#ifndef RADIO_CONTROLLER_H
#define RADIO_CONTROLLER_H

#include <Arduino.h>
#include <RF24.h>
#include "globals.h"

// Radio timing constants
extern const unsigned long DEBUG_INTERVAL;
extern const unsigned long BUTTON_TRANSMIT_INTERVAL;

// Radio state variables (used in main.cpp)
extern unsigned long lastDebugTime;
extern unsigned long lastButtonTransmitTime;

// Function declarations
void initializeRadio();
bool receiveData(byte &lastThrottle, unsigned long &lastReceiveTime);
void transmitButtonState();
bool isRadioConnected();

#endif
