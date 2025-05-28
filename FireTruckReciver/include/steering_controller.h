#ifndef STEERING_CONTROLLER_H
#define STEERING_CONTROLLER_H

#include <Arduino.h>
#include "pins.h"
#include "globals.h"

class SteeringController
{
public:
    SteeringController();
    void initialize();
    void control(int angle);

private:
    static const int MAX_ANGLE = 127;
    static const int ANGLE_DEADZONE_MIN = 120;
    static const int ANGLE_DEADZONE_MAX = 134;
    static const int MAX_LEFT = 20;
    static const int MAX_RIGHT = 235;

    // Voltage-adaptive power settings
    int maxLeftPower = 127;  // Will be adjusted based on voltage
    int maxRightPower = 200; // Will be adjusted based on voltage

    // Voltage sensing
    static const int VOLTAGE_SENSE_PIN = A1;        // Use A1 as defined in pins.h
    static const float VOLTAGE_DIVIDER_RATIO = 0.4; // For 15kΩ + 10kΩ: 10kΩ/(10kΩ+15kΩ) = 4.8V at 12V input
    static const float TARGET_VOLTAGE = 6.0;        // Target voltage for motors
    static const float ARDUINO_VREF = 5.0;          // Arduino UNO reference voltage
    unsigned long lastVoltageCheck = 0;
    static const unsigned long VOLTAGE_CHECK_INTERVAL = 1000; // Check every second

    // Alternative: Simple bench test mode (comment out voltage sensing if using this)
    static const bool BENCH_TEST_MODE = true;    // Set to false for normal operation
    static const int BENCH_TEST_MAX_POWER = 100; // Safe power for 12V bench testing

    int lastLeftPower = 0;
    int lastRightPower = 0;
    static constexpr float SMOOTHING_FACTOR = 0.7; // Higher = more responsive

    // Return pulse system for DC motor centering
    enum SteeringState
    {
        CENTER,
        STEERING_LEFT,
        STEERING_RIGHT,
        RETURNING_FROM_LEFT,
        RETURNING_FROM_RIGHT
    };
    SteeringState currentState = CENTER;
    SteeringState previousState = CENTER;
    unsigned long returnPulseStartTime = 0;
    static const unsigned long RETURN_PULSE_DURATION = 120; // ms - slightly longer for better centering
    static const int RETURN_PULSE_POWER_FROM_LEFT = 120;    // Moderate pulse when returning from left
    static const int RETURN_PULSE_POWER_FROM_RIGHT = 100;   // Stronger pulse when returning from right (to match higher right power)

    // Methods
    void updatePowerLimitsBasedOnVoltage();
    float readSupplyVoltage();
};

#endif
