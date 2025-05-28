#include "steering_controller.h"

SteeringController::SteeringController() {}

void SteeringController::initialize() {
    pinMode(STEERING_A3_PIN, OUTPUT);
    pinMode(STEERING_A4_PIN, OUTPUT);
    digitalWrite(STEERING_A3_PIN, LOW);
    digitalWrite(STEERING_A4_PIN, LOW);
}

void SteeringController::handlePulseTiming() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastPulseTime > STEERING_PULSE_INTERVAL) {
        isPulseActive = true;
        lastPulseTime = currentTime;
    } else if (currentTime - lastPulseTime > STEERING_PULSE_DURATION) {
        isPulseActive = false;
    }
}

void SteeringController::control(int angle) {
    handlePulseTiming();

    if (angle >= ANGLE_DEADZONE_MIN && angle <= ANGLE_DEADZONE_MAX) {
        analogWrite(STEERING_A3_PIN, 0);
        analogWrite(STEERING_A4_PIN, 0);
        return;
    }

    if (!isPulseActive) {
        analogWrite(STEERING_A3_PIN, 0);
        analogWrite(STEERING_A4_PIN, 0);
        return;
    }

    if (angle < MAX_LEFT) {
        analogWrite(STEERING_A3_PIN, 0);
        analogWrite(STEERING_A4_PIN, MAX_ANGLE);
    }
    else if (angle < ANGLE_DEADZONE_MIN) {
        int power = map(angle, MAX_LEFT, ANGLE_DEADZONE_MIN, MAX_ANGLE, MAX_ANGLE * 0.4);
        analogWrite(STEERING_A3_PIN, 0);
        analogWrite(STEERING_A4_PIN, power);
    }
    else if (angle > MAX_RIGHT) {
        analogWrite(STEERING_A3_PIN, MAX_ANGLE);
        analogWrite(STEERING_A4_PIN, 0);
    }
    else if (angle > ANGLE_DEADZONE_MAX) {
        int power = map(angle, ANGLE_DEADZONE_MAX, MAX_RIGHT, MAX_ANGLE * 0.4, MAX_ANGLE);
        analogWrite(STEERING_A3_PIN, power);
        analogWrite(STEERING_A4_PIN, 0);
    }
} 