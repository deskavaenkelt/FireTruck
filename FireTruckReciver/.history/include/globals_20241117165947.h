#ifndef GLOBALS_H
#define GLOBALS_H

#include <RF24.h>

extern const int SERIAL_SPEED;
extern const int MAX_MOTOR_SPEED;
extern int SERVO_MIN_RANGE;
extern int SERVO_MAX_RANGE;

// Steering variables
extern const float SMOOTHING_FACTOR;
extern int lastSteeringOutput;
extern unsigned long lastSteeringPulseTime;
extern const unsigned long STEERING_PULSE_INTERVAL;
extern const unsigned long STEERING_PULSE_DURATION;
extern bool isSteeringPulseActive;

// LED variables
extern unsigned long lastBlueLedUpdate;
extern unsigned long lastWhiteLedUpdate;
extern bool blueLedState;
extern bool whiteLedState;
extern const unsigned long BLUE_BLINK_INTERVAL;
extern const unsigned long WHITE_BLINK_INTERVAL;

// Radio and data structures
extern RF24 radio;
extern const byte addresses[][6];

struct Data_Package {
    byte steeringAngle;
    bool buttonState;
    byte throttle;
};

extern Data_Package data;

#endif 