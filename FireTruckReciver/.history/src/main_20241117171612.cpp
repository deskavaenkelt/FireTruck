#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "pins.h"
#include "globals.h"
#include "motor_controller.h"
#include "steering_controller.h"

MotorController motorController;
SteeringController steeringController;

// Variabler för Failsafe
byte lastThrottle = 127;
unsigned long lastReceiveTime = 0;
const unsigned long FailsafeTimeout = 1000;

void setup() {
    Serial.begin(SERIAL_SPEED);
    initializeRadio();
    motorController.initialize();
    steeringController.initialize();
    initializeButton();
    initializeLEDs();
    Serial.println("Receiver Ready");
    lastReceiveTime = millis();
}

void loop() {
    receiveData();
    steeringController.control(data.steeringAngle);
    motorController.control(lastThrottle);
    readButtonState();
    transmitButtonState();
    checkFailsafe();
    blinkBlueLeds();
    blinkWhiteLeds();
}

// ... resten av dina funktioner (radio, LED, etc.) ...