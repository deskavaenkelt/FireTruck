#ifndef STEERING_SERVO_CONTROLLER_H
#define STEERING_SERVO_CONTROLLER_H

#include <Arduino.h>
#include <Servo.h>
#include "pins.h"

// Steering servo controller class
class SteeringServoController
{
private:
    Servo steeringServo;
    int currentAngle;
    unsigned long lastUpdateTime;
    static const unsigned long UPDATE_INTERVAL = 20; // Update every 20ms for smooth movement

public:
    void initialize();
    void control(int steeringAngle);
    void centerSteering();
    int getCurrentAngle();
    bool isActive();
};

#endif
