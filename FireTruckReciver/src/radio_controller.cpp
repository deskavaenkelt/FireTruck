#include "radio_controller.h"

// Radio timing constants
const unsigned long DEBUG_INTERVAL = 500;          // Debug output every 500ms
const unsigned long BUTTON_TRANSMIT_INTERVAL = 50; // Only transmit button state every 50ms

// Radio state variables
unsigned long lastDebugTime = 0;
unsigned long lastButtonTransmitTime = 0;

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

bool receiveData(byte &lastThrottle, unsigned long &lastReceiveTime)
{
    if (radio.available())
    {
        radio.read(&data, sizeof(Data_Package)); // Läs hela data och lagra den i 'data'-strukturen

        // Reduce debug output frequency to minimize serial spam
        static unsigned long lastReceiveDebug = 0;
        unsigned long currentTime = millis();
        if (currentTime - lastReceiveDebug >= 1000) // Only every 1 second
        {
            Serial.print("---------------------- Received Angle Value: ");
            Serial.print(data.steeringAngle);
            Serial.print(" | Received Throttle Value: ");
            Serial.println(data.throttle);
            lastReceiveDebug = currentTime;
        }

        lastThrottle = data.throttle; // Update last valid throttle value
        lastReceiveTime = millis();   // Update last receive time
        return true;                  // Data received successfully
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
        return false; // No data received
    }
}

void transmitButtonState()
{
    radio.stopListening();                    // Stoppa mottagning
    delayMicroseconds(200);                   // Increase delay to ensure radio is ready
    radio.write(&data, sizeof(Data_Package)); // Skicka hela data-paketet
    delayMicroseconds(200);                   // Increase delay before switching back
    radio.startListening();                   // Återgå till mottagning
}

bool isRadioConnected()
{
    return radio.isChipConnected();
}
