#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>
#include "pins.h"
#include "globals.h"
#include "motor_controller.h"
#include "steering_controller.h"
#include "steering_service.h"
#include "battery_protection.h"
#include "smart_led_controller.h"
#include "radio_controller.h"

MotorController motorController;
SteeringController steeringController;
SteeringService steeringService;

// Variabler för Failsafe
byte lastThrottle = 127; // Default to middle (stop) position
unsigned long lastReceiveTime = 0;
const unsigned long FailsafeTimeout = 1000; // 1 second timeout
const unsigned long LOOP_INTERVAL = 10;     // 100Hz update rate for receiver
unsigned long lastLoopTime = 0;

// Funktionsdeklarationer
void initializeButton();
void controlServos();
void readButtonState();
int servoJitterFilter(int angle);
void checkFailsafe();
void debugButtonState();

void setup()
{
    Serial.begin(SERIAL_SPEED);
    initializeRadio();
    motorController.initialize();
    steeringController.initialize();
    initializeButton();
    initializeSmartLEDs();
    initializeBatteryProtection();

    // 5-second startup indication with white LEDs
    Serial.println("=== LED DEBUG: Setting white LEDs HIGH for startup ===");
    digitalWrite(WHITE_LED_PIN_3, HIGH);
    digitalWrite(WHITE_LED_PIN_4, HIGH);
    digitalWrite(RED_LED_PIN_1, HIGH);
    digitalWrite(RED_LED_PIN_2, HIGH);
    Serial.println("White and Red LEDs should be ON now for 5 seconds...");
    delay(5000); // 5 seconds
    Serial.println("=== LED DEBUG: Setting white and red LEDs LOW after startup ===");
    digitalWrite(WHITE_LED_PIN_3, LOW);
    digitalWrite(WHITE_LED_PIN_4, LOW);
    digitalWrite(RED_LED_PIN_1, LOW);
    digitalWrite(RED_LED_PIN_2, LOW);
    Serial.println("White and Red LEDs should be OFF now");

    Serial.println("Receiver Ready");
    lastReceiveTime = millis();
}

void loop()
{
    // Always check for radio data to minimize latency
    receiveData(lastThrottle, lastReceiveTime);

    // Check battery voltage for protection
    checkBatteryVoltage();

    unsigned long currentTime = millis();
    if (currentTime - lastLoopTime >= LOOP_INTERVAL)
    {
        if (isBatteryProtectionActive())
        {
            // Emergency mode - only run LED emergency pattern
            batteryProtectionLEDs();
            // Skip all motor control when battery protection is active
        }
        else
        {
            // Normal operation
            int filteredAngle = steeringService.filterJitter(data.steeringAngle);

            // Only control motors if battery protection is not active
            if (!isBatteryProtectionActive())
            {
                steeringController.control(filteredAngle);
                motorController.control(lastThrottle);
            }

            readButtonState();

            // Only transmit button state occasionally to reduce radio conflicts
            if (currentTime - lastButtonTransmitTime >= BUTTON_TRANSMIT_INTERVAL)
            {
                transmitButtonState();
                lastButtonTransmitTime = currentTime;
            }

            checkFailsafe();
            blinkBlueLeds(lastThrottle);
            blinkWhiteLeds(filteredAngle, lastThrottle, lastReceiveTime, FailsafeTimeout);
        }

        lastLoopTime = currentTime;
    }
}

void initializeButton()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void controlServos()
{
    int angle = data.steeringAngle;

    // Center deadzone - completely off
    if (angle >= 120 && angle <= 134)
    {
        analogWrite(STEERING_A3_PIN, 0);
        analogWrite(STEERING_A4_PIN, 0);
        isSteeringPulseActive = false;
        return;
    }
}

int servoJitterFilter(int angle)
{
    // ta bort jitter
    Serial.print("Servo Jitter Filter: ");
    Serial.println(angle);
    if (angle >= 92 && angle <= 99)
    { // Remove center jitter
        angle = 95;
    }
    else if (angle < 33)
    { // Remove lower jitter
        angle = 30;
    }
    else if (angle > 157)
    { // Remove upper jitter
        angle = 160;
    }
    return angle;
}

void readButtonState()
{
    data.buttonState = digitalRead(BUTTON_PIN);
    data.buttonState = !data.buttonState; // Omvänd tillståndet för INPUT_PULLUP
    // debugButtonState();
}

void checkFailsafe()
{
    if (millis() - lastReceiveTime > FailsafeTimeout)
    {
        // If no valid data received within timeout, stop the motor
        lastThrottle = 127; // Set throttle to stop position
    }
}

void debugButtonState()
{
    Serial.print("Button State: ");
    Serial.println(data.buttonState);
}
