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

Data_Package data; // Create a variable with the above structure

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
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(9600);
  while (!Serial) {
    ; // Vänta tills Serial är redo
  }
  initializeRadio();
  Serial.println("\n\n=== TRANSMITTER STARTING ===");
  
  // Blink LED to identify this as transmitter
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

void loop() {
  Serial.println("\n=== TRANSMITTER LOOP START ===");
  readInputValues();
  transmitData();
  receiveData();
  setButtonState();
  Serial.println("=== TRANSMITTER LOOP END ===");
  Serial.flush();
  delay(100);
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
  Serial.print("        Sending: Angle Value: ");
  Serial.print(data.steeringAngle);
  Serial.print(" | Throttle Value: ");
  Serial.println(data.throttle);
}

void transmitData() {
  radio.stopListening();
  bool success = radio.write(&data, sizeof(Data_Package));
  if (success) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Data send failed");
  }
  radio.startListening();
}

void receiveData() {
  radio.startListening();
  unsigned long startTime = millis();
  while (millis() - startTime < 10) { // Minska väntetiden till 10ms
    if (radio.available()) {
      radio.read(&data, sizeof(Data_Package));
      // Serial.println("Data received from receiver");
      break;
    }
  }
  // if (!radio.available()) {
  //   Serial.println("No data received from receiver");
  // }
  radio.stopListening();
}

void setButtonState() {
  Serial.print("Received Button State: ");
  Serial.println(data.buttonState);
  digitalWrite(LED_PIN, data.buttonState ? HIGH : LOW);
}