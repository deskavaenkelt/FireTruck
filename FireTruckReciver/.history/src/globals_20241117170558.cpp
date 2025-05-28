#include "globals.h"


const int SERIAL_SPEED = 9600;
const int MAX_MOTOR_SPEED = 127;
int SERVO_MIN_RANGE = 30;
int SERVO_MAX_RANGE = 160;

// Steering variables
const float SMOOTHING_FACTOR = 0.3;
int lastSteeringOutput = 0;
unsigned long lastSteeringPulseTime = 0;
const unsigned long STEERING_PULSE_INTERVAL = 30;
const unsigned long STEERING_PULSE_DURATION = 5;
bool isSteeringPulseActive = false;

// LED variables
unsigned long lastBlueLedUpdate = 0;
unsigned long lastWhiteLedUpdate = 0;
bool blueLedState = false;
bool whiteLedState = false;
const unsigned long BLUE_BLINK_INTERVAL = 500;
const unsigned long WHITE_BLINK_INTERVAL = 250;

// Radio and data
RF24 radio(7, 8);
const byte addresses[][6] = {"00001", "00002"};
Data_Package data; 