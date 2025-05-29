#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>
#include "pins.h"
#include "globals.h"
#include "motor_controller.h"
#include "steering_controller.h"
#include "steering_service.h"

MotorController motorController;
SteeringController steeringController;
SteeringService steeringService;

// Variabler för Failsafe
byte lastThrottle = 127; // Default to middle (stop) position
unsigned long lastReceiveTime = 0;
const unsigned long FailsafeTimeout = 1000; // 1 second timeout
unsigned long lastDebugTime = 0;
const unsigned long DEBUG_INTERVAL = 500; // Debug output every 500ms
const unsigned long LOOP_INTERVAL = 10;   // 100Hz update rate for receiver
unsigned long lastLoopTime = 0;
unsigned long lastButtonTransmitTime = 0;
const unsigned long BUTTON_TRANSMIT_INTERVAL = 50; // Only transmit button state every 50ms

// Funktionsdeklarationer
void initializeRadio();
// void initializeServos();
void initializeButton();
void initializeLEDs();
void receiveData();
void controlServos();
void readButtonState();
void transmitButtonState();
void debugButtonState();
int servoJitterFilter(int angle);
void checkFailsafe();
void blinkBlueLeds();
void blinkWhiteLeds(int filteredAngle);
// void controlSmartHeadlights(int filteredAngle);

void setup()
{
    Serial.begin(SERIAL_SPEED);
    initializeRadio();
    motorController.initialize();
    steeringController.initialize();
    initializeButton();
    initializeLEDs();

    // 5-second startup indication with white LEDs
    Serial.println("=== LED DEBUG: Setting white LEDs HIGH for startup ===");
    digitalWrite(WHITE_LED_PIN_3, HIGH);
    digitalWrite(WHITE_LED_PIN_4, HIGH);
    Serial.println("White LEDs should be ON now for 5 seconds...");
    delay(5000); // 5 seconds
    Serial.println("=== LED DEBUG: Setting white LEDs LOW after startup ===");
    digitalWrite(WHITE_LED_PIN_3, LOW);
    digitalWrite(WHITE_LED_PIN_4, LOW);
    Serial.println("White LEDs should be OFF now");

    Serial.println("Receiver Ready");
    lastReceiveTime = millis();
}

void loop()
{
    // Always check for radio data to minimize latency
    receiveData();

    unsigned long currentTime = millis();
    if (currentTime - lastLoopTime >= LOOP_INTERVAL)
    {
        int filteredAngle = steeringService.filterJitter(data.steeringAngle);
        steeringController.control(filteredAngle); // Use steeringController instead of steeringService
        motorController.control(lastThrottle);
        readButtonState();

        // Only transmit button state occasionally to reduce radio conflicts
        if (currentTime - lastButtonTransmitTime >= BUTTON_TRANSMIT_INTERVAL)
        {
            transmitButtonState();
            lastButtonTransmitTime = currentTime;
        }

        checkFailsafe();
        blinkBlueLeds();
        blinkWhiteLeds(filteredAngle);
        lastLoopTime = currentTime;
    }
}

void initializeRadio()
{
    Serial.println("=== RADIO DEBUG: Initializing radio ===");
    radio.begin();

    // Check if radio is connected
    if (!radio.isChipConnected())
    {
        Serial.println("ERROR: nRF24L01 not connected!");
        return;
    }
    Serial.println("nRF24L01 chip connected successfully");

    radio.setPALevel(RF24_PA_MIN);
    radio.setDataRate(RF24_2MBPS);          // Öka dataöverföringshastigheten
    radio.setChannel(76);                   // Use same channel as transmitter
    radio.setRetries(1, 1);                 // Reduce retries for faster response
    radio.openWritingPipe(addresses[0]);    // 00001
    radio.openReadingPipe(1, addresses[1]); // 00002
    radio.startListening();
    radio.flush_tx();
    radio.flush_rx();

    Serial.println("Radio configuration:");
    Serial.print("Channel: ");
    Serial.println(radio.getChannel());
    Serial.print("Data Rate: ");
    Serial.println(radio.getDataRate());
    Serial.print("PA Level: ");
    Serial.println(radio.getPALevel());
    Serial.println("=== RADIO DEBUG: Initialization complete ===");
}

// void initializeServos() {
//     steeringServo.attach(SERVO_PIN_1);
// }

void initializeButton()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void initializeLEDs()
{
    pinMode(BLUE_LED_PIN_1, OUTPUT);
    pinMode(BLUE_LED_PIN_2, OUTPUT);
    pinMode(WHITE_LED_PIN_3, OUTPUT);
    pinMode(WHITE_LED_PIN_4, OUTPUT);

    Serial.println("=== LED DEBUG: Initializing LEDs ===");
    Serial.println("Setting all LEDs to HIGH during initialization");
    digitalWrite(BLUE_LED_PIN_1, HIGH);
    digitalWrite(BLUE_LED_PIN_2, HIGH);
    digitalWrite(WHITE_LED_PIN_3, HIGH);
    digitalWrite(WHITE_LED_PIN_4, HIGH);
    Serial.println("All LEDs set to HIGH - they should be ON if wired correctly");
}

void receiveData()
{
    if (radio.available())
    {
        radio.read(&data, sizeof(Data_Package)); // Läs hela data och lagra den i 'data'-strukturen
        Serial.print("---------------------- Received Angle Value: ");
        Serial.print(data.steeringAngle);
        Serial.print(" | Received Throttle Value: ");
        Serial.println(data.throttle);
        lastThrottle = data.throttle; // Update last valid throttle value
        lastReceiveTime = millis();   // Update last receive time
    }
    else
    {
        // Only print waiting message occasionally to reduce serial spam
        unsigned long currentTime = millis();
        if (currentTime - lastDebugTime >= DEBUG_INTERVAL)
        {
            Serial.println("Waiting for the transmitter...");
            lastDebugTime = currentTime;
        }
    }
}

void controlServos()
{
    int angle = data.steeringAngle;
    int MAX_ANGLE = 127;
    unsigned long currentTime = millis();

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

void transmitButtonState()
{
    radio.stopListening();                    // Stoppa mottagning
    delayMicroseconds(200);                   // Increase delay to ensure radio is ready
    radio.write(&data, sizeof(Data_Package)); // Skicka hela data-paketet
    delayMicroseconds(200);                   // Increase delay before switching back
    radio.startListening();                   // Återgå till mottagning
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

void blinkBlueLeds()
{
    unsigned long currentTime = millis();
    if (currentTime - lastBlueLedUpdate >= BLUE_BLINK_INTERVAL)
    {
        blueLedState = !blueLedState;
        digitalWrite(BLUE_LED_PIN_1, blueLedState);
        digitalWrite(BLUE_LED_PIN_2, blueLedState);
        Serial.print("Blue LEDs: ");
        Serial.println(blueLedState ? "HIGH" : "LOW");
        lastBlueLedUpdate = currentTime;
    }
}

void blinkWhiteLeds(int filteredAngle)
{
    // Temporarily disable smart headlights - revert to simple blinking
    unsigned long currentTime = millis();
    if (currentTime - lastWhiteLedUpdate >= WHITE_BLINK_INTERVAL)
    {
        whiteLedState = !whiteLedState;
        digitalWrite(WHITE_LED_PIN_3, whiteLedState);
        digitalWrite(WHITE_LED_PIN_4, whiteLedState);
        Serial.print("White LEDs: ");
        Serial.print(whiteLedState ? "HIGH" : "LOW");
        Serial.print(" | Throttle: ");
        Serial.print(lastThrottle);
        Serial.print(" | Angle: ");
        Serial.println(filteredAngle);
        lastWhiteLedUpdate = currentTime;
    }
}

/*
void controlSmartHeadlights(int filteredAngle)
{
    // Safety check - ensure we have received valid data
    if (millis() - lastReceiveTime > FailsafeTimeout)
    {
        // No valid data - turn off all lights
        digitalWrite(WHITE_LED_PIN_3, LOW);
        digitalWrite(WHITE_LED_PIN_4, LOW);
        return;
    }

    unsigned long currentTime = millis();
    static unsigned long lastBlinkTime = 0;
    static bool blinkState = false;
    const unsigned long BLINK_INTERVAL = 500; // 500ms blink interval for turn signals

    // Determine vehicle state
    bool isMovingForward = (lastThrottle > 135);                 // Forward threshold
    bool isMovingBackward = (lastThrottle < 119);                // Backward threshold
    bool isStill = (lastThrottle >= 119 && lastThrottle <= 135); // Still/neutral

    // Determine steering state
    bool isTurningLeft = (filteredAngle < 120);
    bool isTurningRight = (filteredAngle > 134);
    bool isStraight = (filteredAngle >= 120 && filteredAngle <= 134);

    // Update blink state for turn signals
    if (currentTime - lastBlinkTime >= BLINK_INTERVAL)
    {
        blinkState = !blinkState;
        lastBlinkTime = currentTime;
    }

    // Control logic
    if (isStill)
    {
        // Standing still - all lights off
        digitalWrite(WHITE_LED_PIN_3, LOW); // Left LED off
        digitalWrite(WHITE_LED_PIN_4, LOW); // Right LED off
    }
    else if (isMovingForward)
    {
        if (isStraight)
        {
            // Moving forward straight - both headlights on constant
            digitalWrite(WHITE_LED_PIN_3, HIGH); // Left LED on
            digitalWrite(WHITE_LED_PIN_4, HIGH); // Right LED on
        }
        else if (isTurningLeft)
        {
            // Moving forward + turning left - left blinks, right constant
            digitalWrite(WHITE_LED_PIN_3, blinkState ? HIGH : LOW); // Left LED blinks
            digitalWrite(WHITE_LED_PIN_4, HIGH);                    // Right LED constant
        }
        else if (isTurningRight)
        {
            // Moving forward + turning right - right blinks, left constant
            digitalWrite(WHITE_LED_PIN_3, HIGH);                    // Left LED constant
            digitalWrite(WHITE_LED_PIN_4, blinkState ? HIGH : LOW); // Right LED blinks
        }
    }
    else if (isMovingBackward)
    {
        if (isStraight)
        {
            // Moving backward straight - all lights off (no reverse lights implemented)
            digitalWrite(WHITE_LED_PIN_3, LOW); // Left LED off
            digitalWrite(WHITE_LED_PIN_4, LOW); // Right LED off
        }
        else if (isTurningLeft)
        {
            // Moving backward + turning left - left blinks, right off
            digitalWrite(WHITE_LED_PIN_3, blinkState ? HIGH : LOW); // Left LED blinks
            digitalWrite(WHITE_LED_PIN_4, LOW);                     // Right LED off
        }
        else if (isTurningRight)
        {
            // Moving backward + turning right - right blinks, left off
            digitalWrite(WHITE_LED_PIN_3, LOW);                     // Left LED off
            digitalWrite(WHITE_LED_PIN_4, blinkState ? HIGH : LOW); // Right LED blinks
        }
    }
}
*/
