#ifndef SERVO_TEST_CONTROLLER_H
#define SERVO_TEST_CONTROLLER_H

#include <Arduino.h>
#include <Servo.h>
#include "pins.h"

// Servo test controller class
class ServoTestController
{
private:
    Servo testServo;
    unsigned long lastDebugTime;
    static const unsigned long DEBUG_INTERVAL = 500; // Debug every 500ms

public:
    void initialize();
    void updateFromPotentiometer();
    void updateFromRadio(int throttleValue);
    void debugOutput(int potValue, int servoAngle);
};

#endif
