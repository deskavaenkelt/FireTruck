#include "motor_controller.h"

MotorController::MotorController() {}

void MotorController::initialize() {
    pinMode(THROTTLE_A1_PIN, OUTPUT);
    pinMode(THROTTLE_A2_PIN, OUTPUT);
    digitalWrite(THROTTLE_A1_PIN, LOW);
    digitalWrite(THROTTLE_A2_PIN, LOW);
}

void MotorController::control(byte throttle) {
    lastThrottle = throttle; // Use last valid throttle value
    // Throttle used for forward and backward control
    // Joystick values: 0 to 255; down = 0; middle = 127; up = 255, add deadzone (5)
    // Motor values: 0 to MAX_MOTOR_SPEED; backward = 0; stop = MAX_MOTOR_SPEED / 2; forward = MAX_MOTOR_SPEED, add deadzone (5)

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