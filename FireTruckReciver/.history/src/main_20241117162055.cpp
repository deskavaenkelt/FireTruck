#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

#define BLUE_LED_PIN_1 0
#define BLUE_LED_PIN_2 1
#define WHITE_LED_PIN_3 2
#define WHITE_LED_PIN_4 3
#define DEAD_PIN_4 4
#define THROTTLE_A1_PIN 6
#define THROTTLE_A2_PIN 5
#define STEERING_A3_PIN 9
#define STEERING_A4_PIN 10

#define BUTTON_PIN 14 // A0 pin as digital pin A0-A5 = 14-19

int SERIAL_SPEED = 9600;

// Dead Pins: 4
// Free Anaolg Pins: A1, A2, A3, A4, A5
// Free Digital Pins: None

const int MAX_MOTOR_SPEED = 127; // Max motorhastighet
int SERVO_MIN_RANGE = 30;
int SERVO_MAX_RANGE = 160;

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};
Servo steeringServo;

// Struktur för data
struct Data_Package
{
    byte steeringAngle;
    bool buttonState;
    byte throttle;
};

Data_Package data; // Skapa en variabel med ovanstående struktur

// Variabler för Failsafe
byte lastThrottle = 127; // Default to middle (stop) position
unsigned long lastReceiveTime = 0;
const unsigned long FailsafeTimeout = 1000; // 1 second timeout

// Funktionsdeklarationer
void initializeRadio();
// void initializeServos();
void initializeMotor();
void initializeButton();
void initializeLEDs();
void receiveData();
void controlServos();
void controlMotor();
void readButtonState();
void transmitButtonState();
void debugButtonState();
int servoJitterFilter(int angle);
void checkFailsafe();

void setup()
{
    Serial.begin(SERIAL_SPEED);
    initializeRadio();
    // initializeServos();
    initializeMotor();
    initializeButton();
    initializeLEDs();
    Serial.println("Receiver Ready");
    lastReceiveTime = millis(); // Initialize last receive time
}

void loop()
{
    receiveData();
    controlServos();
    controlMotor();
    readButtonState();
    transmitButtonState();
    checkFailsafe();
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

void initializeMotor()
{
    pinMode(THROTTLE_A1_PIN, OUTPUT);
    pinMode(THROTTLE_A2_PIN, OUTPUT);
    pinMode(STEERING_A3_PIN, OUTPUT);
    pinMode(STEERING_A4_PIN, OUTPUT);
    digitalWrite(THROTTLE_A1_PIN, LOW);
    digitalWrite(THROTTLE_A2_PIN, LOW);
    digitalWrite(STEERING_A3_PIN, LOW);
    digitalWrite(STEERING_A4_PIN, LOW);
}

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
    // // Handle both servo and steering motor
    // int angle = servoJitterFilter(data.steeringAngle);
    
    // // Keep existing servo code
    // if (angle == 0) {
    //     steeringServo.write(90);
    // } else {
    //     steeringServo.write(angle);
    // }
    
    // New steering motor control with min/max limits
    int angle = data.steeringAngle;
    int MAX_ANGLE = 127;
    Serial.print("-------------------------------------------------------------------------- Steering Angle: ");
    Serial.println(angle);
    if (angle >= 124 && angle <= 130) {  // Center deadzone
        analogWrite(STEERING_A3_PIN, 0);
        analogWrite(STEERING_A4_PIN, 0);
    }
    else if (angle < 20) {  // Max left
        analogWrite(STEERING_A3_PIN, 0);
        analogWrite(STEERING_A4_PIN, MAX_ANGLE);
    }
    else if (angle < 124) {  // Left turn with mapping
        analogWrite(STEERING_A3_PIN, 0);
        analogWrite(STEERING_A4_PIN, map(angle, 20, 124, MAX_ANGLE, 0));
    }
    else if (angle > 235) {  // Max right
        analogWrite(STEERING_A3_PIN, MAX_ANGLE);
        analogWrite(STEERING_A4_PIN, 0);
    }
    else if (angle > 130) {  // Right turn with mapping
        analogWrite(STEERING_A3_PIN, map(angle, 130, 235, 0, MAX_ANGLE));
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

void controlMotor()
{
    byte throttle = lastThrottle; // Use last valid throttle value
    // Throttle used for forward and backward control
    // Joystick values: 0 to 255; down = 0; middle = 127; up = 255, add deadzone (5)
    // Motor values: 0 to MAX_MOTOR_SPEED; backward = 0; stop = MAX_MOTOR_SPEED / 2; forward = MAX_MOTOR_SPEED, add deadzone (5)
    if (throttle >= 122 && throttle <= 132)
    {
        // Stop motor (dead zone)
        analogWrite(THROTTLE_A1_PIN, 0);
        analogWrite(THROTTLE_A2_PIN, 0);
    }
    else if (throttle < 20)
    {
        // Max backward speed
        analogWrite(THROTTLE_A1_PIN, MAX_MOTOR_SPEED);
        analogWrite(THROTTLE_A2_PIN, 0);
    }
    else if (throttle < 122)
    {
        // Backward speed with mapping
        analogWrite(THROTTLE_A1_PIN, map(throttle, 0, 122, MAX_MOTOR_SPEED, 0));
        analogWrite(THROTTLE_A2_PIN, 0);
    }
    else if (throttle > 235)
    {
        // Max forward speed
        analogWrite(THROTTLE_A1_PIN, 0);
        analogWrite(THROTTLE_A2_PIN, MAX_MOTOR_SPEED);
    }
    else if (throttle > 132)
    {
        // Forward speed with mapping
        analogWrite(THROTTLE_A1_PIN, 0);
        analogWrite(THROTTLE_A2_PIN, map(throttle, 132, 255, 0, MAX_MOTOR_SPEED));
    }
    Serial.print("Motor A1: ");
    Serial.print(throttle);
    Serial.print(" | Motor A2: ");
    Serial.println(throttle);
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