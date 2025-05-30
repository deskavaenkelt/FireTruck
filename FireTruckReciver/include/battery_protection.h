#ifndef BATTERY_PROTECTION_H
#define BATTERY_PROTECTION_H

#include <Arduino.h>
#include "pins.h"

// Battery protection constants
extern const float BATTERY_CRITICAL_VOLTAGE;
extern const float BATTERY_LOW_VOLTAGE;
extern const float VOLTAGE_DIVIDER_RATIO;
extern const float ARDUINO_VREF;
extern const unsigned long BATTERY_CHECK_INTERVAL;

// Battery protection state
extern bool batteryProtectionActive;

// Function declarations
void initializeBatteryProtection();
float readBatteryVoltage();
void checkBatteryVoltage();
void activateBatteryProtection();
void batteryProtectionLEDs();
bool isBatteryProtectionActive();

#endif
