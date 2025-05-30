#ifndef SMART_LED_CONTROLLER_H
#define SMART_LED_CONTROLLER_H

#include <Arduino.h>
#include "pins.h"
#include "globals.h"

// LED timing constants - only declare the ones not in globals.h
extern const unsigned long BLINK_INTERVAL;

// Function declarations
void initializeSmartLEDs();
void blinkBlueLeds(byte throttle);
void blinkWhiteLeds(int filteredAngle, byte throttle, unsigned long lastReceiveTime, unsigned long failsafeTimeout);
void controlSmartHeadlights(int filteredAngle, byte throttle, unsigned long lastReceiveTime, unsigned long failsafeTimeout);

#endif
