#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define LED_PIN 6
#define STEERING_PIN A0
#define THROTTLE_PIN A1

int SERIAL_SPEED = 9600;
const unsigned long LOOP_INTERVAL = 10; // Increase to 100Hz for better responsiveness
unsigned long lastLoopTime = 0;

// Debug filtering variables - same as receiver
const unsigned long DEBUG_INTERVAL = 500; // Debug output every 500ms
unsigned long lastDebugTime = 0;
unsigned long lastFailDebugTime = 0;
unsigned long lastSuccessDebugTime = 0;

int BYTE_MIN_RANGE = 0;
int BYTE_MAX_RANGE = 255;

RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};

struct Data_Package
{
  byte steeringAngle;
  bool buttonState;
  byte throttle;
};

Data_Package data; // Create a variable with the above structure

// Funktionsdeklarationer
void initializeRadio();
void readInputValues(bool shouldDebug);
void transmitData();
void receiveData();
int readAndMapSteering();
int readAndMapThrottle();
long get_byte_value(int throttleValue);
void setButtonState(bool shouldDebug);
int servoJitterFilter(int angle);

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(SERIAL_SPEED);
  while (!Serial)
  {
    ; // Vänta tills Serial är redo
  }
  initializeRadio();
  Serial.println("\n\n=== TRANSMITTER STARTING ===");

  // Blink LED to identify this as transmitter
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

void loop()
{
  unsigned long currentTime = millis();
  if (currentTime - lastLoopTime >= LOOP_INTERVAL)
  {
    // Only print loop debug messages occasionally to reduce serial spam
    bool shouldDebug = (currentTime - lastDebugTime >= DEBUG_INTERVAL);
    if (shouldDebug)
    {
      Serial.println("\n=== TRANSMITTER LOOP START ===");
      lastDebugTime = currentTime;
    }

    readInputValues(shouldDebug);
    transmitData();
    receiveData();
    setButtonState(shouldDebug);

    if (shouldDebug)
    {
      Serial.println("=== TRANSMITTER LOOP END ===");
      Serial.flush();
    }
    lastLoopTime = currentTime;
  }
  // Small delay to prevent overwhelming the system
  delayMicroseconds(100);
}

void initializeRadio()
{
  Serial.println("=== SENDER RADIO DEBUG: Initializing radio ===");
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
  radio.setChannel(76);                   // Use a specific channel to avoid interference
  radio.setRetries(1, 1);                 // Reduce retries for faster response
  radio.openWritingPipe(addresses[1]);    // 00002
  radio.openReadingPipe(1, addresses[0]); // 00001
  radio.stopListening();
  radio.flush_tx();
  radio.flush_rx();

  Serial.println("Sender Radio configuration:");
  Serial.print("Channel: ");
  Serial.println(radio.getChannel());
  Serial.print("Data Rate: ");
  Serial.println(radio.getDataRate());
  Serial.print("PA Level: ");
  Serial.println(radio.getPALevel());
  Serial.println("=== SENDER RADIO DEBUG: Initialization complete ===");
}

int removeInputJitter(int input)
{
  if (input >= 122 && input <= 132)
  { // remove center jitter - FIXED: was backwards!
    return 127;
  }
  else if (input < 20)
  {
    return 0;
  }
  else if (input > 235)
  {
    return 255;
  }
  else
  {
    return input;
  }
}

int readAndMapAnalogInput(int pin)
{
  int analogValue = analogRead(pin);
  int byteValue = map(analogValue, 0, 1023, 0, 255);
  byteValue = removeInputJitter(byteValue);
  return byteValue;
}

void readInputValues(bool shouldDebug)
{
  data.steeringAngle = readAndMapAnalogInput(STEERING_PIN);
  data.throttle = readAndMapAnalogInput(THROTTLE_PIN);
  if (shouldDebug)
  {
    Serial.print("        Sending: Angle Value: ");
    Serial.print(data.steeringAngle);
    Serial.print(" | Throttle Value: ");
    Serial.println(data.throttle);
  }
}

void transmitData()
{
  radio.stopListening();
  delayMicroseconds(200); // Increase delay to ensure radio is ready
  bool success = radio.write(&data, sizeof(Data_Package));
  unsigned long currentTime = millis();

  if (success)
  {
    // Only print success message occasionally to reduce serial spam
    if (currentTime - lastSuccessDebugTime >= DEBUG_INTERVAL)
    {
      Serial.println("Data sent successfully");
      lastSuccessDebugTime = currentTime;
    }
  }
  else
  {
    // Only print failure message occasionally to reduce serial spam
    if (currentTime - lastFailDebugTime >= DEBUG_INTERVAL)
    {
      Serial.println("Data send failed");
      lastFailDebugTime = currentTime;
    }
  }
  delayMicroseconds(200); // Add delay before switching back
}

void receiveData()
{
  radio.startListening();
  delayMicroseconds(200); // Increase delay to ensure radio is ready
  unsigned long startTime = millis();
  while (millis() - startTime < 5)
  { // Increase timeout slightly
    if (radio.available())
    {
      radio.read(&data, sizeof(Data_Package));
      break;
    }
  }
}

void setButtonState(bool shouldDebug)
{
  if (shouldDebug)
  {
    Serial.print("Received Button State: ");
    Serial.println(data.buttonState);
  }
  digitalWrite(LED_PIN, data.buttonState ? HIGH : LOW);
}
