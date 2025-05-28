#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "pins.h"
#include "globals.h"

class MotorController {
public:
    MotorController();
    void initialize();
    void control(byte throttle);

private:
    byte lastThrottle = 127;
    static const byte THROTTLE_DEADZONE_MIN = 122;
    static const byte THROTTLE_DEADZONE_MAX = 132;
    static const byte MAX_BACKWARD = 20;
    static const byte MAX_FORWARD = 235;
};

#endif 