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

    // Soft limits based on actual mechanical testing (real limits ~110-140)
    static const int SOFT_LIMIT_LEFT = 115;  // Just inside real mechanical limit (~110)
    static const int SOFT_LIMIT_RIGHT = 135; // Just inside real mechanical limit (~140)

    // Voltage-adaptive power settings - Updated with aggressive mapping from debug testing
    int maxLeftPower = 200;  // Increased from 127 for better response
    int maxRightPower = 255; // Keep high for weak right motor compensation

    // Voltage sensing
    static const float VOLTAGE_DIVIDER_RATIO = 0.4; // For 15kΩ + 10kΩ: 10kΩ/(10kΩ+15kΩ) = 4.8V at 12V input
    static const float TARGET_VOLTAGE = 6.0;        // Target voltage for motors
    static const float ARDUINO_VREF = 5.0;          // Arduino UNO reference voltage
    unsigned long lastVoltageCheck = 0;
    static const unsigned long VOLTAGE_CHECK_INTERVAL = 1000; // Check every second

    // Alternative: Simple bench test mode (comment out voltage sensing if using this)
    static const bool BENCH_TEST_MODE = true;    // Set to false for normal operation
    static const int BENCH_TEST_MAX_POWER = 140; // Reduced from 180 for smoother operation

    // Debug mode for steering troubleshooting
    static const bool SIMPLE_DEBUG_MODE = false; // Set to false for advanced steering system

    int lastLeftPower = 0;
    int lastRightPower = 0;
    static constexpr float SMOOTHING_FACTOR = 0.8; // Higher = more responsive, lower = smoother

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
    static const unsigned long RETURN_PULSE_DURATION = 80; // Reduced from 120ms for quicker centering
    static const int RETURN_PULSE_POWER_FROM_LEFT = 80;    // Reduced from 120 for smoother centering
    static const int RETURN_PULSE_POWER_FROM_RIGHT = 70;   // Reduced from 100 for smoother centering

    // Methods
    void updatePowerLimitsBasedOnVoltage();
    float readSupplyVoltage();
};

#endif
