#include "battery_protection.h"

// Battery protection constants
const float BATTERY_CRITICAL_VOLTAGE = 6.6;        // 3.3V per cell for 2S LiPo (critical shutdown)
const float BATTERY_LOW_VOLTAGE = 7.0;             // 3.5V per cell for 2S LiPo (warning level)
const float VOLTAGE_DIVIDER_RATIO = 0.4;           // 10kΩ / (15kΩ + 10kΩ) = 0.4
const float ARDUINO_VREF = 5.0;                    // Arduino reference voltage
const unsigned long BATTERY_CHECK_INTERVAL = 5000; // Check battery every 5 seconds

// Battery protection state
bool batteryProtectionActive = false;
static unsigned long lastBatteryCheck = 0;

void initializeBatteryProtection()
{
    pinMode(VOLTAGE_SENSE_PIN, INPUT);
    batteryProtectionActive = false;
    lastBatteryCheck = 0;

    Serial.println("Battery protection system initialized");
    Serial.print("Critical voltage: ");
    Serial.print(BATTERY_CRITICAL_VOLTAGE);
    Serial.println("V");
    Serial.print("Warning voltage: ");
    Serial.print(BATTERY_LOW_VOLTAGE);
    Serial.println("V");
}

float readBatteryVoltage()
{
    // Read analog value from voltage divider
    int analogValue = analogRead(VOLTAGE_SENSE_PIN);

    // Convert ADC reading to voltage at Arduino pin
    float measuredVoltage = (analogValue / 1023.0) * ARDUINO_VREF;

    // Calculate actual battery voltage using voltage divider ratio
    float batteryVoltage = measuredVoltage / VOLTAGE_DIVIDER_RATIO;

    return batteryVoltage;
}

void checkBatteryVoltage()
{
    unsigned long currentTime = millis();

    // Only check battery voltage periodically to avoid constant ADC reads
    if (currentTime - lastBatteryCheck >= BATTERY_CHECK_INTERVAL)
    {
        float batteryVoltage = readBatteryVoltage();

        // Safety check - ignore unrealistic readings
        if (batteryVoltage < 4.0 || batteryVoltage > 15.0)
        {
            Serial.print("WARNING: Unrealistic battery voltage reading: ");
            Serial.print(batteryVoltage);
            Serial.println("V - Ignoring reading");
            lastBatteryCheck = currentTime;
            return;
        }

        // Check for critical battery level
        if (batteryVoltage <= BATTERY_CRITICAL_VOLTAGE && !batteryProtectionActive)
        {
            Serial.print("CRITICAL: Battery voltage too low: ");
            Serial.print(batteryVoltage);
            Serial.println("V - Activating battery protection!");
            activateBatteryProtection();
        }
        else if (batteryVoltage <= BATTERY_LOW_VOLTAGE && !batteryProtectionActive)
        {
            Serial.print("WARNING: Low battery voltage: ");
            Serial.print(batteryVoltage);
            Serial.println("V - Consider charging soon");
        }
        else if (!batteryProtectionActive)
        {
            // Normal voltage - print status occasionally
            static unsigned long lastNormalVoltageReport = 0;
            if (currentTime - lastNormalVoltageReport >= 30000) // Every 30 seconds
            {
                Serial.print("Battery voltage OK: ");
                Serial.print(batteryVoltage);
                Serial.println("V");
                lastNormalVoltageReport = currentTime;
            }
        }

        lastBatteryCheck = currentTime;
    }
}

void activateBatteryProtection()
{
    batteryProtectionActive = true;

    // Immediately stop all motors
    analogWrite(STEERING_A3_PIN, 0);
    analogWrite(STEERING_A4_PIN, 0);
    // Note: Motor controller will handle throttle motor shutdown

    Serial.println("=== BATTERY PROTECTION ACTIVATED ===");
    Serial.println("All motor functions disabled to protect battery");
    Serial.println("Charge battery before continuing operation");
    Serial.println("=====================================");
}

void batteryProtectionLEDs()
{
    // Emergency LED pattern - all LEDs blink synchronized rapidly
    unsigned long currentTime = millis();
    static unsigned long lastEmergencyBlink = 0;
    static bool emergencyBlinkState = false;
    const unsigned long EMERGENCY_BLINK_INTERVAL = 200; // Fast 200ms blink

    if (currentTime - lastEmergencyBlink >= EMERGENCY_BLINK_INTERVAL)
    {
        emergencyBlinkState = !emergencyBlinkState;

        // All LEDs blink together in emergency mode
        digitalWrite(BLUE_LED_PIN_1, emergencyBlinkState);
        digitalWrite(BLUE_LED_PIN_2, emergencyBlinkState);
        digitalWrite(WHITE_LED_PIN_3, emergencyBlinkState);
        digitalWrite(WHITE_LED_PIN_4, emergencyBlinkState);
        digitalWrite(RED_LED_PIN_1, emergencyBlinkState);
        digitalWrite(RED_LED_PIN_2, emergencyBlinkState);

        lastEmergencyBlink = currentTime;

        // Debug output occasionally
        static unsigned long lastEmergencyDebug = 0;
        if (currentTime - lastEmergencyDebug >= 5000) // Every 5 seconds
        {
            Serial.println("EMERGENCY: Battery protection active - all LEDs blinking");
            lastEmergencyDebug = currentTime;
        }
    }
}

bool isBatteryProtectionActive()
{
    return batteryProtectionActive;
}
