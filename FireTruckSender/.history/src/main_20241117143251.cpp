#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define LED_PIN 6
#define STEERING_PIN A0
#define THROTTLE_PIN A1

int BYTE_MIN_RANGE = 0;
int BYTE_MAX_RANGE = 255;

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};

struct Data_Package {
  byte steeringAngle;
  bool buttonState;
  byte throttle;
};

Data_Package data;
Data_Package receivedData;

// Funktionsdeklarationer
void initializeRadio();
void readInputValues();
void transmitData();
void receiveData();
int readAndMapSteering();
int readAndMapThrottle();
long get_byte_value(int throttleValue);
void setButtonState();
int servoJitterFilter(int angle);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // Vänta tills Serial är redo
  }
  
  // Tydlig startup-sekvens
  for(int i = 0; i < 5; i++) {
    Serial.println("*** TRANSMITTER STARTUP ***");
    delay(1000);
  }
}

void loop() {
  // Extremt förenklad loop för test
  Serial.println("TRANSMITTER TICK");
  delay(1000);
}

void initializeRadio() {
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_2MBPS); // Öka dataöverföringshastigheten
  radio.openWritingPipe(addresses[1]); // 00002
  radio.openReadingPipe(1, addresses[0]); // 00001
  radio.stopListening();
  radio.flush_tx();
  radio.flush_rx();
}

int removeInputJitter(int input) {
  if (input >= 132 && input <= 122) { // remove center jitter
    return 127;
  } else if (input < 20) {
    return 0;
  } else if (input > 235) {
    return 255;
  } else {
    return input;
  }
}

int readAndMapAnalogInput(int pin) {
  int analogValue = analogRead(pin);
  int byteValue = map(analogValue, 0, 1023, 0, 255);
  byteValue = removeInputJitter(byteValue);
  return byteValue;
}

void readInputValues() {
  data.steeringAngle = readAndMapAnalogInput(STEERING_PIN);
  data.throttle = readAndMapAnalogInput(THROTTLE_PIN);
  Serial.println("\n--------------------");
  Serial.print("Time: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds");
  Serial.print("Steering: ");
  Serial.println(data.steeringAngle);
  Serial.print("Throttle: ");
  Serial.println(data.throttle);
}

void transmitData() {
  radio.stopListening();
  bool success = radio.write(&data, sizeof(Data_Package));
  if (success) {
    Serial.println("✓ Data sent");
  } else {
    Serial.println("✗ Send failed");
  }
  radio.startListening();
}

void receiveData() {
  radio.startListening();
  unsigned long startTime = millis();
  bool dataReceived = false;
  
  while (millis() - startTime < 10) {
    if (radio.available()) {
      radio.read(&receivedData, sizeof(Data_Package));
      data.buttonState = receivedData.buttonState;
      Serial.print("Received button state: ");
      Serial.println(receivedData.buttonState);
      dataReceived = true;
      break;
    }
  }
  
  if (!dataReceived) {
    Serial.println("No data received in timeout period");
  }
  radio.stopListening();
}

void setButtonState() {
  Serial.print("Setting LED state to: ");
  Serial.println(data.buttonState);
  
  digitalWrite(LED_PIN, data.buttonState ? HIGH : LOW);
  
  Serial.print("Current LED pin state: ");
  Serial.println(digitalRead(LED_PIN));
  Serial.println("--------------------\n");
}