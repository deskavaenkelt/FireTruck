#include "globals.h"

// Serial speed
const int SERIAL_SPEED = 9600;

// Motor speed
const int MAX_MOTOR_SPEED = 127; // Max motorhastighet anpassad till 12V
int SERVO_MIN_RANGE = 30;
int SERVO_MAX_RANGE = 160;

// Steering variables
const float SMOOTHING_FACTOR = 0.3; // Justera mellan 0.1 (mjukare) och 0.5 (snabbare)
int lastSteeringOutput = 0;
unsigned long lastSteeringPulseTime = 0;
const unsigned long STEERING_PULSE_INTERVAL = 30; // Time between pulses (ms) org 50
const unsigned long STEERING_PULSE_DURATION = 5;  // Length of each pulse (ms) org 20
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