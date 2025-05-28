#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>
#include "pins.h"
#include "globals.h"
#include "motor_controller.h"

RF24 radio(7, 8); // CE, CSN

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

// Lägg till i globala variabler
MotorController motorController;

void setup()
{
    Serial.begin(SERIAL_SPEED);
    initializeRadio();
    motorController.initialize();  // Ersätter initializeMotor()
    initializeButton();
    initializeLEDs();
    Serial.println("Receiver Ready");
    lastReceiveTime = millis(); // Initialize last receive time
}

void loop()
{
    receiveData();
    controlServos();
    motorController.control(lastThrottle);  // Ersätter controlMotor()
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

// void initializeMotor() {
//     pinMode(THROTTLE_A1_PIN, OUTPUT);
//     pinMode(THROTTLE_A2_PIN, OUTPUT);
//     pinMode(STEERING_A3_PIN, OUTPUT);
//     pinMode(STEERING_A4_PIN, OUTPUT);
//     digitalWrite(THROTTLE_A1_PIN, LOW);
//     digitalWrite(THROTTLE_A2_PIN, LOW);
//     digitalWrite(STEERING_A3_PIN, LOW);
//     digitalWrite(STEERING_A4_PIN, LOW);
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
    digitalWrite(BLUE_LED_PIN_1, LOW);
    digitalWrite(BLUE_LED_PIN_2, LOW);
    digitalWrite(WHITE_LED_PIN_3, LOW);
    digitalWrite(WHITE_LED_PIN_4, LOW);
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

    // Pulse timing logic
    if (currentTime - lastSteeringPulseTime > STEERING_PULSE_INTERVAL) {
        isSteeringPulseActive = true;
        lastSteeringPulseTime = currentTime;
    } else if (currentTime - lastSteeringPulseTime > STEERING_PULSE_DURATION) {
        isSteeringPulseActive = false;
    }

    if (!isSteeringPulseActive) {
        analogWrite(STEERING_A3_PIN, 0);
        analogWrite(STEERING_A4_PIN, 0);
        return;
    }

    // Actual steering control with pulses
    if (angle < 20) {  // Max left
        analogWrite(STEERING_A3_PIN, 0);
        analogWrite(STEERING_A4_PIN, MAX_ANGLE);
    }
    else if (angle < 120) {  // Left turn
        int power = map(angle, 20, 120, MAX_ANGLE, MAX_ANGLE * 0.4);
        analogWrite(STEERING_A3_PIN, 0);
        analogWrite(STEERING_A4_PIN, power);
    }
    else if (angle > 235) {  // Max right
        analogWrite(STEERING_A3_PIN, MAX_ANGLE);
        analogWrite(STEERING_A4_PIN, 0);
    }
    else if (angle > 134) {  // Right turn
        int power = map(angle, 134, 235, MAX_ANGLE * 0.4, MAX_ANGLE);
        analogWrite(STEERING_A3_PIN, power);
        analogWrite(STEERING_A4_PIN, 0);
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