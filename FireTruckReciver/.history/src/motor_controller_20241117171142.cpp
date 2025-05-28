#include "motor_controller.h"

MotorController::MotorController() {}

void MotorController::initialize() {
    pinMode(THROTTLE_A1_PIN, OUTPUT);
    pinMode(THROTTLE_A2_PIN, OUTPUT);
    digitalWrite(THROTTLE_A1_PIN, LOW);
    digitalWrite(THROTTLE_A2_PIN, LOW);
}

void MotorController::control(byte throttle) {
    lastThrottle = throttle;

    if (throttle >= THROTTLE_DEADZONE_MIN && throttle <= THROTTLE_DEADZONE_MAX) {
        analogWrite(THROTTLE_A1_PIN, 0);
        analogWrite(THROTTLE_A2_PIN, 0);
    }
    else if (throttle < MAX_BACKWARD) {
        analogWrite(THROTTLE_A1_PIN, MAX_MOTOR_SPEED);
        analogWrite(THROTTLE_A2_PIN, 0);
    }
    else if (throttle < THROTTLE_DEADZONE_MIN) {
        analogWrite(THROTTLE_A1_PIN, map(throttle, 0, THROTTLE_DEADZONE_MIN, MAX_MOTOR_SPEED, 0));
        analogWrite(THROTTLE_A2_PIN, 0);
    }
    else if (throttle > MAX_FORWARD) {
        analogWrite(THROTTLE_A1_PIN, 0);
        analogWrite(THROTTLE_A2_PIN, MAX_MOTOR_SPEED);
    }
    else if (throttle > THROTTLE_DEADZONE_MAX) {
        analogWrite(THROTTLE_A1_PIN, 0);
        analogWrite(THROTTLE_A2_PIN, map(throttle, THROTTLE_DEADZONE_MAX, 255, 0, MAX_MOTOR_SPEED));
    }
} 