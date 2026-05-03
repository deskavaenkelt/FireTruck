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
#include "servo_test_controller.h"
#include "steering_servo_controller.h"
#include "audio_controller.h"

MotorController motorController;
SteeringController steeringController;           // Keep for compatibility if needed
SteeringServoController steeringServoController; // New servo-based steering
SteeringService steeringService;
ServoTestController servoTestController;
AudioController audioController;

// Variabler för Failsafe
byte lastThrottle = 127; // Default to middle (stop) position
unsigned long lastReceiveTime = 0;
const unsigned long FailsafeTimeout = 1000; // 1 second timeout
const unsigned long LOOP_INTERVAL = 10;     // 100Hz update rate for receiver
unsigned long lastLoopTime = 0;

// Test mode flag - set to true to enable servo testing
bool servoTestMode = true;    // Change to false to disable servo test
bool useServoSteering = true; // Set to true to use servo steering instead of motor

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

    // Initialize steering system
    if (useServoSteering)
    {
        steeringServoController.initialize();
        Serial.println("Using SERVO steering system");
    }
    else
    {
        steeringController.initialize();
        Serial.println("Using MOTOR steering system");
    }

    initializeButton();
    initializeSmartLEDs();
    initializeBatteryProtection();

    // Initialize audio system
    audioController.initialize();
    delay(200); // Extra delay for PAM8403 stabilization
    audioController.enable();
    delay(200); // Let it stabilize longer without volume control

    // Initialize servo test if enabled
    if (servoTestMode)
    {
        servoTestController.initialize();
    }

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

    // Test audio system with startup sound
    Serial.println("Testing audio system...");
    audioController.playStartupSound();
    delay(1000);

    // Add manual test option
    Serial.println("=== AUDIO DEBUG COMMANDS ===");
    Serial.println("Send via Serial Monitor:");
    Serial.println("'t' = Test basic tone");
    Serial.println("'h' = Hardware test (pin toggle)");
    Serial.println("'e' = EXTREME basic test (raw pin)");
    Serial.println("'s' = Test siren");
    Serial.println("'x' = Stop all sounds");

    Serial.println("Receiver Ready");
    if (servoTestMode)
    {
        Serial.println("SERVO TEST MODE ENABLED");
        Serial.println("Connect potentiometer to A2 or use radio throttle");
    }
    if (audioController.isEnabled())
    {
        Serial.println("AUDIO SYSTEM ENABLED");
        Serial.println("Button = Siren, Reverse = Backup beeps");
    }
    lastReceiveTime = millis();
}

void loop()
{
    // Check for Serial Monitor commands
    if (Serial.available())
    {
        char command = Serial.read();
        if (command == 't' || command == 'T')
        {
            Serial.println("Manual audio test triggered...");
            audioController.testBasicTone();
        }
        else if (command == 'h' || command == 'H')
        {
            Serial.println("Hardware test triggered...");
            audioController.testHardware();
        }
        else if (command == 'e' || command == 'E')
        {
            Serial.println("EXTREME basic test triggered...");
            audioController.testExtremeBasic();
        }
        else if (command == 's' || command == 'S')
        {
            Serial.println("Siren test triggered...");
            audioController.playSiren();
        }
        else if (command == 'x' || command == 'X')
        {
            Serial.println("Stop all sounds...");
            audioController.stopAllSounds();
        }
    }

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
                // Use servo steering or motor steering based on configuration
                if (useServoSteering)
                {
                    steeringServoController.control(filteredAngle);
                }
                else
                {
                    steeringController.control(filteredAngle);
                }
                motorController.control(lastThrottle);
            }

            // Only transmit button state occasionally to reduce radio conflicts
            if (currentTime - lastButtonTransmitTime >= BUTTON_TRANSMIT_INTERVAL)
            {
                transmitButtonState();
                lastButtonTransmitTime = currentTime;
            }

            checkFailsafe();
            blinkBlueLeds(lastThrottle);
            blinkWhiteLeds(filteredAngle, lastThrottle, lastReceiveTime, FailsafeTimeout);

            // Audio functionality based on movement and button
            if (lastThrottle < 120)
            { // Backing up
                static unsigned long lastBackupBeep = 0;
                if (millis() - lastBackupBeep > 800)
                { // Beep every 800ms
                    audioController.playBackupBeep();
                    lastBackupBeep = millis();
                }
            }

            // Button-activated siren from sender (not receiver button)
            if (data.buttonState)
            {
                audioController.playSiren();
            }
            else
            {
                // Only stop if we're not backing up (backup beeps should continue)
                if (lastThrottle >= 120)
                {
                    audioController.stopAllSounds();
                }
            }

            // Servo test functionality
            if (servoTestMode)
            {
                // Check if potentiometer is connected (reading > 10)
                int potReading = analogRead(POTENTIOMETER_TEST_PIN);
                if (potReading > 10)
                {
                    // Use potentiometer control
                    servoTestController.updateFromPotentiometer();
                }
                else
                {
                    // Use radio throttle control
                    servoTestController.updateFromRadio(lastThrottle);
                }
            }
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
