#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>
#include "pins.h"
#include "globals.h"
#include "motor_controller.h"
#include "steering_controller.h"

MotorController motorController;
SteeringController steeringController;

// Variabler för Failsafe
byte lastThrottle = 127; // Default to middle (stop) position
unsigned long lastReceiveTime = 0;
const unsigned long FailsafeTimeout = 1000; // 1 second timeout

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
void blinkWhiteLeds();

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

void initializeRadio()
{
    radio.begin();
    radio.setPALevel(RF24_PA_MIN);
    radio.setDataRate(RF24_2MBPS);          // Öka dataöverföringshastigheten
    radio.openWritingPipe(addresses[0]);    // 00001
    radio.openReadingPipe(1, addresses[1]); // 00002
    radio.startListening();
    radio.flush_tx();
    radio.flush_rx();
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
    digitalWrite(BLUE_LED_PIN_1, HALF_PI);
    digitalWrite(BLUE_LED_PIN_2, HIGH);
    digitalWrite(WHITE_LED_PIN_3, HIGH);
    digitalWrite(WHITE_LED_PIN_4, HIGH);
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
        Serial.println("Waiting for the transmitter...");
    }
}

void controlServos()
{
    int angle = data.steeringAngle;
    int MAX_ANGLE = 127;
    unsigned long currentTime = millis();
    
    // Center deadzone - completely off
    if (angle >= 120 && angle <= 134) {
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
    delay(2);                                 // Minska fördröjningen till 2ms
    radio.write(&data, sizeof(Data_Package)); // Skicka hela data-paketet
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

void blinkBlueLeds() {
    unsigned long currentTime = millis();
    if (currentTime - lastBlueLedUpdate >= BLUE_BLINK_INTERVAL) {
        blueLedState = !blueLedState;
        digitalWrite(BLUE_LED_PIN_1, blueLedState);
        digitalWrite(BLUE_LED_PIN_2, blueLedState);
        lastBlueLedUpdate = currentTime;
    }
}

void blinkWhiteLeds() {
    unsigned long currentTime = millis();
    if (currentTime - lastWhiteLedUpdate >= WHITE_BLINK_INTERVAL) {
        whiteLedState = !whiteLedState;
        digitalWrite(WHITE_LED_PIN_3, whiteLedState);
        digitalWrite(WHITE_LED_PIN_4, whiteLedState);
        lastWhiteLedUpdate = currentTime;
    }
}