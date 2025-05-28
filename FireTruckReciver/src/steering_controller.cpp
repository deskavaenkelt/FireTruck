#include "steering_controller.h"

SteeringController::SteeringController() {}

void SteeringController::initialize()
{
    pinMode(STEERING_A3_PIN, OUTPUT);
    pinMode(STEERING_A4_PIN, OUTPUT);
    digitalWrite(STEERING_A3_PIN, LOW);
    digitalWrite(STEERING_A4_PIN, LOW);
}

void SteeringController::control(int angle)
{
    int targetLeftPower = 0;
    int targetRightPower = 0;

    // Center deadzone - stop steering
    if (angle >= ANGLE_DEADZONE_MIN && angle <= ANGLE_DEADZONE_MAX)
    {
        targetLeftPower = 0;
        targetRightPower = 0;
    }
    // Left steering (angle < center)
    else if (angle < ANGLE_DEADZONE_MIN)
    {
        targetLeftPower = 0; // Stop right motor

        if (angle < MAX_LEFT)
        {
            // Maximum left steering
            targetRightPower = MAX_ANGLE;
        }
        else
        {
            // Proportional left steering
            targetRightPower = map(angle, MAX_LEFT, ANGLE_DEADZONE_MIN, MAX_ANGLE, 0);
        }
    }
    // Right steering (angle > center)
    else if (angle > ANGLE_DEADZONE_MAX)
    {
        targetRightPower = 0; // Stop left motor

        if (angle > MAX_RIGHT)
        {
            // Maximum right steering
            targetLeftPower = MAX_ANGLE;
        }
        else
        {
            // Proportional right steering
            targetLeftPower = map(angle, ANGLE_DEADZONE_MAX, MAX_RIGHT, 0, MAX_ANGLE);
        }
    }

    // Apply smoothing for more responsive control
    lastLeftPower = (int)(SMOOTHING_FACTOR * targetLeftPower + (1.0 - SMOOTHING_FACTOR) * lastLeftPower);
    lastRightPower = (int)(SMOOTHING_FACTOR * targetRightPower + (1.0 - SMOOTHING_FACTOR) * lastRightPower);

    // Apply the smoothed values
    analogWrite(STEERING_A3_PIN, lastLeftPower);
    analogWrite(STEERING_A4_PIN, lastRightPower);
}
