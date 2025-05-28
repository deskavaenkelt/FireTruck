#include "steering_service.h"
#include "Arduino.h"
#include "pins.h"

void SteeringService::control(int angle) {
    if (angle >= CENTER_DEADZONE_MIN && angle <= CENTER_DEADZONE_MAX) {
        analogWrite(STEERING_A3_PIN, 0);
        analogWrite(STEERING_A4_PIN, 0);
        isSteeringPulseActive = false;
        return;
    }
}

int SteeringService::filterJitter(int angle) {
    if (angle >= CENTER_JITTER_MIN && angle <= CENTER_JITTER_MAX) {
        return CENTER_POSITION;
    }
    if (angle < MIN_ANGLE) {
        return MIN_ANGLE;
    }
    if (angle > MAX_ANGLE) {
        return MAX_ANGLE;
    }
    return angle;
} 