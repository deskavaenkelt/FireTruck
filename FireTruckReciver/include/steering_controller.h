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

    int lastLeftPower = 0;
    int lastRightPower = 0;
    static constexpr float SMOOTHING_FACTOR = 0.7; // Higher = more responsive
};

#endif
