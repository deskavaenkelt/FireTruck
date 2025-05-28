#include "steering_controller.h"

SteeringController::SteeringController() {}

void SteeringController::initialize()
{
    pinMode(STEERING_A3_PIN, OUTPUT);
    pinMode(STEERING_A4_PIN, OUTPUT);
    digitalWrite(STEERING_A3_PIN, LOW);
    digitalWrite(STEERING_A4_PIN, LOW);

    // Optimize PWM frequency specifically for L298N H-bridge
    // Using lowest practical frequency for completely silent operation
    // Timer 1 controls pins 9 and 10 (STEERING_A3_PIN and STEERING_A4_PIN)
    TCCR1A = (TCCR1A & 0b11111100) | 0b01; // Set WGM11:10 = 01 for Phase Correct PWM
    TCCR1B = (TCCR1B & 0b11111000) | 0x04; // Set prescaler to 256 for ~3.9kHz PWM (silent operation)

    // This gives 16MHz / (256 * 2 * 256) = 3.9kHz - completely silent, still efficient for L298N

    // Ultimate fallback if still audible: Use prescaler 0x05 for 1.95kHz (may cause slight motor roughness)

    // Initialize voltage sensing
    pinMode(VOLTAGE_SENSE_PIN, INPUT);
    updatePowerLimitsBasedOnVoltage();
}

float SteeringController::readSupplyVoltage()
{
    int analogValue = analogRead(VOLTAGE_SENSE_PIN);
    // Convert ADC reading to actual voltage
    float measuredVoltage = (analogValue / 1023.0) * ARDUINO_VREF;
    // Calculate actual supply voltage using voltage divider ratio
    float supplyVoltage = measuredVoltage / VOLTAGE_DIVIDER_RATIO;
    return supplyVoltage;
}

void SteeringController::updatePowerLimitsBasedOnVoltage()
{
    if (BENCH_TEST_MODE)
    {
        // Bench test mode with asymmetric compensation from debug testing
        maxLeftPower = BENCH_TEST_MAX_POWER;       // Left motor (for right steering)
        maxRightPower = BENCH_TEST_MAX_POWER + 20; // Right motor (for left steering) - reduced asymmetry

        Serial.println("BENCH TEST MODE - Using optimized power levels for 12V testing");
        Serial.print("Max Powers - Left: ");
        Serial.print(maxLeftPower);
        Serial.print(", Right: ");
        Serial.println(maxRightPower);
        return;
    }

    // Original voltage sensing code with safety checks
    float supplyVoltage = readSupplyVoltage();

    // Safety check - if voltage reading seems unrealistic, use safe defaults
    if (supplyVoltage < 4.0 || supplyVoltage > 15.0)
    {
        Serial.print("WARNING: Unrealistic voltage reading: ");
        Serial.print(supplyVoltage);
        Serial.println("V - Using safe default power levels");
        maxLeftPower = 80;   // Safe default
        maxRightPower = 100; // Safe default with asymmetry compensation
        return;
    }

    // Calculate power scaling factor to maintain ~6V to motors
    float scaleFactor = TARGET_VOLTAGE / supplyVoltage;
    if (scaleFactor > 1.0)
        scaleFactor = 1.0; // Don't boost above 100%

    // Apply scaling to power limits
    maxLeftPower = (int)(127 * scaleFactor);
    maxRightPower = (int)(200 * scaleFactor); // Still compensate for asymmetry

    // Additional safety - never exceed safe limits even with scaling
    if (maxLeftPower > 150)
        maxLeftPower = 150;
    if (maxRightPower > 180)
        maxRightPower = 180;

    // Debug output
    Serial.print("Supply Voltage: ");
    Serial.print(supplyVoltage);
    Serial.print("V, Scale Factor: ");
    Serial.print(scaleFactor);
    Serial.print(", Max Powers - Left: ");
    Serial.print(maxLeftPower);
    Serial.print(", Right: ");
    Serial.println(maxRightPower);
}

void SteeringController::control(int angle)
{
    // SIMPLE DEBUG MODE - Direct mapping for troubleshooting
    if (SIMPLE_DEBUG_MODE)
    {
        Serial.print("DEBUG: Raw angle input: ");
        Serial.println(angle);

        // Simple direct mapping with FULL POWER for testing (ignores all limitations)
        if (angle >= 120 && angle <= 134)
        {
            // Center deadzone
            analogWrite(STEERING_A3_PIN, 0);
            analogWrite(STEERING_A4_PIN, 0);
            Serial.println("DEBUG: Center - Both motors OFF");
        }
        else if (angle < 120)
        {
            // Left steering - activate right motor (A4) with AGGRESSIVE MAPPING
            // Make left steering equally aggressive to match right steering
            int power;
            if (angle >= 105)
            {
                // For small left movements (105-119), give high power immediately
                power = map(angle, 119, 105, 150, 255); // Start at PWM 150!
            }
            else
            {
                // For larger movements, stay at max
                power = 255;
            }
            power = constrain(power, 0, 255);
            analogWrite(STEERING_A3_PIN, 0);
            analogWrite(STEERING_A4_PIN, power);
            Serial.print("DEBUG: Left steering - A4 power: ");
            Serial.print(power);
            Serial.println(" (AGGRESSIVE MAPPING to match right steering)");
        }
        else if (angle > 134)
        {
            // Right steering - activate left motor (A3) with AGGRESSIVE MAPPING
            // Make right steering much more sensitive - start at higher PWM
            int power;
            if (angle <= 150)
            {
                // For small right movements (135-150), give high power immediately
                power = map(angle, 135, 150, 150, 255); // Start at PWM 150!
            }
            else
            {
                // For larger movements, stay at max
                power = 255;
            }
            power = constrain(power, 0, 255);
            analogWrite(STEERING_A3_PIN, power);
            analogWrite(STEERING_A4_PIN, 0);
            Serial.print("DEBUG: Right steering - A3 power: ");
            Serial.print(power);
            Serial.println(" (AGGRESSIVE MAPPING for weak right motor)");
        }
        return; // Exit early when in debug mode
    }

    // COMPLEX LOGIC (current implementation)
    unsigned long currentTime = millis();

    // Periodically update power limits based on voltage
    if (currentTime - lastVoltageCheck >= VOLTAGE_CHECK_INTERVAL)
    {
        updatePowerLimitsBasedOnVoltage();
        lastVoltageCheck = currentTime;
    }

    // Determine current steering state based on input
    SteeringState newState;
    if (angle >= ANGLE_DEADZONE_MIN && angle <= ANGLE_DEADZONE_MAX)
    {
        newState = CENTER;
    }
    else if (angle < ANGLE_DEADZONE_MIN)
    {
        newState = STEERING_LEFT;
    }
    else
    {
        newState = STEERING_RIGHT;
    }

    // Detect state transitions for return pulse logic
    if (newState != currentState)
    {
        previousState = currentState;
        currentState = newState;

        // Start return pulse when transitioning from steering to center
        if (newState == CENTER)
        {
            if (previousState == STEERING_LEFT)
            {
                currentState = RETURNING_FROM_LEFT;
                returnPulseStartTime = currentTime;
            }
            else if (previousState == STEERING_RIGHT)
            {
                currentState = RETURNING_FROM_RIGHT;
                returnPulseStartTime = currentTime;
            }
        }
    }

    int targetLeftPower = 0;
    int targetRightPower = 0;

    // Handle return pulses
    if (currentState == RETURNING_FROM_LEFT)
    {
        if (currentTime - returnPulseStartTime < RETURN_PULSE_DURATION)
        {
            // Give a stronger pulse in opposite direction (right) to help return from left
            targetLeftPower = RETURN_PULSE_POWER_FROM_LEFT;
            targetRightPower = 0;
        }
        else
        {
            // Return pulse complete, go to center
            currentState = CENTER;
            targetLeftPower = 0;
            targetRightPower = 0;
        }
    }
    else if (currentState == RETURNING_FROM_RIGHT)
    {
        if (currentTime - returnPulseStartTime < RETURN_PULSE_DURATION)
        {
            // Give a normal pulse in opposite direction (left) to help return from right
            targetLeftPower = 0;
            targetRightPower = RETURN_PULSE_POWER_FROM_RIGHT;
        }
        else
        {
            // Return pulse complete, go to center
            currentState = CENTER;
            targetLeftPower = 0;
            targetRightPower = 0;
        }
    }
    // Normal steering operations
    else if (currentState == CENTER)
    {
        targetLeftPower = 0;
        targetRightPower = 0;
    }
    // Left steering (angle < center)
    else if (currentState == STEERING_LEFT)
    {
        targetLeftPower = 0; // Stop right motor

        // AGGRESSIVE MAPPING for left steering with REALISTIC SOFT LIMITS
        if (angle >= SOFT_LIMIT_LEFT)
        {
            // Near center - small left movements, give moderate power
            targetRightPower = map(angle, 119, SOFT_LIMIT_LEFT, (int)(0.6 * maxLeftPower), (int)(0.8 * maxLeftPower));
        }
        else
        {
            // At soft limit - stop here to prevent mechanical noise
            targetRightPower = (int)(0.8 * maxLeftPower); // Good power but stop at safe limit
        }
    }
    // Right steering (angle > center)
    else if (currentState == STEERING_RIGHT)
    {
        targetRightPower = 0; // Stop left motor

        // AGGRESSIVE MAPPING for right steering with REALISTIC SOFT LIMITS
        if (angle <= SOFT_LIMIT_RIGHT)
        {
            // Near center - small right movements, give moderate power
            targetLeftPower = map(angle, ANGLE_DEADZONE_MAX, SOFT_LIMIT_RIGHT, (int)(0.6 * maxRightPower), (int)(0.8 * maxRightPower));
        }
        else
        {
            // At soft limit - stop here to prevent mechanical noise
            targetLeftPower = (int)(0.8 * maxRightPower); // Good power but stop at safe limit
        }
    }

    // Apply smoothing for more responsive control (but not during return pulses)
    if (currentState != RETURNING_FROM_LEFT && currentState != RETURNING_FROM_RIGHT)
    {
        lastLeftPower = (int)(SMOOTHING_FACTOR * targetLeftPower + (1.0 - SMOOTHING_FACTOR) * lastLeftPower);
        lastRightPower = (int)(SMOOTHING_FACTOR * targetRightPower + (1.0 - SMOOTHING_FACTOR) * lastRightPower);
    }
    else
    {
        // Direct application during return pulses for immediate response
        lastLeftPower = targetLeftPower;
        lastRightPower = targetRightPower;
    }

    // Apply the values
    analogWrite(STEERING_A3_PIN, lastLeftPower);
    analogWrite(STEERING_A4_PIN, lastRightPower);

    // Debug output for tuning
    if (lastLeftPower > 0 || lastRightPower > 0)
    {
        Serial.print("ADVANCED: Left PWM: ");
        Serial.print(lastLeftPower);
        Serial.print(" | Right PWM: ");
        Serial.print(lastRightPower);
        Serial.print(" | Angle: ");
        Serial.print(angle);
        Serial.print(" | State: ");
        Serial.println(currentState);
    }
}
