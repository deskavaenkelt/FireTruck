#include "smart_led_controller.h"

// LED timing constants - only define the ones not in globals.cpp
const unsigned long BLINK_INTERVAL = 500; // 500ms blink interval for turn signals

void initializeSmartLEDs()
{
    pinMode(BLUE_LED_PIN_1, OUTPUT);
    pinMode(BLUE_LED_PIN_2, OUTPUT);
    pinMode(WHITE_LED_PIN_3, OUTPUT);
    pinMode(WHITE_LED_PIN_4, OUTPUT);
    pinMode(RED_LED_PIN_1, OUTPUT);
    pinMode(RED_LED_PIN_2, OUTPUT);

    // Initialize LED states (variables are defined in globals.cpp)
    blueLedState = false;
    lastBlueLedUpdate = 0;

    Serial.println("Smart LED controller initialized");
    Serial.println("LED pins configured for smart lighting system");
}

void blinkBlueLeds(byte throttle)
{
    unsigned long currentTime = millis();

    // Determine if vehicle is moving
    bool isMoving = (throttle < 119 || throttle > 135); // Moving forward or backward

    // Different blink intervals based on movement
    unsigned long blinkInterval;
    if (isMoving)
    {
        blinkInterval = 150; // Fast blink when moving (150ms = ~3.3Hz)
    }
    else
    {
        blinkInterval = BLUE_BLINK_INTERVAL; // Slow blink when stationary (500ms = 1Hz)
    }

    if (currentTime - lastBlueLedUpdate >= blinkInterval)
    {
        if (isMoving)
        {
            // Bistable flip-flop behavior when moving - alternate between LEDs
            static bool flipFlopState = false;
            flipFlopState = !flipFlopState;

            if (flipFlopState)
            {
                digitalWrite(BLUE_LED_PIN_1, HIGH); // Left LED on
                digitalWrite(BLUE_LED_PIN_2, LOW);  // Right LED off
            }
            else
            {
                digitalWrite(BLUE_LED_PIN_1, LOW);  // Left LED off
                digitalWrite(BLUE_LED_PIN_2, HIGH); // Right LED on
            }

            // Reduce debug output frequency when moving
            static unsigned long lastMovingDebug = 0;
            if (currentTime - lastMovingDebug >= 1000) // Only every 1 second when moving
            {
                Serial.print("Blue LEDs (MOVING): ");
                Serial.println(flipFlopState ? "LEFT" : "RIGHT");
                lastMovingDebug = currentTime;
            }
        }
        else
        {
            // Normal synchronized blinking when stationary
            blueLedState = !blueLedState;
            digitalWrite(BLUE_LED_PIN_1, blueLedState);
            digitalWrite(BLUE_LED_PIN_2, blueLedState);

            // Less frequent debug when stationary
            static unsigned long lastStationaryDebug = 0;
            if (currentTime - lastStationaryDebug >= 2000) // Only every 2 seconds when stationary
            {
                Serial.print("Blue LEDs (STATIONARY): ");
                Serial.println(blueLedState ? "HIGH" : "LOW");
                lastStationaryDebug = currentTime;
            }
        }

        lastBlueLedUpdate = currentTime;
    }
}

void blinkWhiteLeds(int filteredAngle, byte throttle, unsigned long lastReceiveTime, unsigned long failsafeTimeout)
{
    // Smart headlight system based on throttle and steering
    controlSmartHeadlights(filteredAngle, throttle, lastReceiveTime, failsafeTimeout);
}

void controlSmartHeadlights(int filteredAngle, byte throttle, unsigned long lastReceiveTime, unsigned long failsafeTimeout)
{
    // Safety check - ensure we have received valid data
    if (millis() - lastReceiveTime > failsafeTimeout)
    {
        // No valid data - turn off all lights
        digitalWrite(WHITE_LED_PIN_3, LOW);
        digitalWrite(WHITE_LED_PIN_4, LOW);
        return;
    }

    unsigned long currentTime = millis();
    static unsigned long lastBlinkTime = 0;
    static bool blinkState = false;
    static unsigned long lastDebugTime = 0;

    // Determine vehicle state
    bool isMovingForward = (throttle > 135);             // Forward threshold
    bool isMovingBackward = (throttle < 119);            // Backward threshold
    bool isStill = (throttle >= 119 && throttle <= 135); // Still/neutral

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

    // Debug output every 2 seconds instead of 500ms to reduce serial spam
    if (currentTime - lastDebugTime >= 2000)
    {
        Serial.print("SMART LEDs: ");
        if (isStill)
            Serial.print("STILL");
        else if (isMovingForward)
            Serial.print("FORWARD");
        else if (isMovingBackward)
            Serial.print("BACKWARD");

        Serial.print(" | ");
        if (isStraight)
            Serial.print("STRAIGHT");
        else if (isTurningLeft)
            Serial.print("LEFT");
        else if (isTurningRight)
            Serial.print("RIGHT");

        Serial.print(" | Throttle: ");
        Serial.print(throttle);
        Serial.print(" | Angle: ");
        Serial.println(filteredAngle);
        lastDebugTime = currentTime;
    }

    // Control logic
    if (isStill)
    {
        // Standing still - all lights off
        digitalWrite(WHITE_LED_PIN_3, LOW); // Left LED off
        digitalWrite(WHITE_LED_PIN_4, LOW); // Right LED off
        digitalWrite(RED_LED_PIN_1, LOW);   // Red LEDs off
        digitalWrite(RED_LED_PIN_2, LOW);
    }
    else if (isMovingForward)
    {
        // Turn off red LEDs when moving forward
        digitalWrite(RED_LED_PIN_1, LOW);
        digitalWrite(RED_LED_PIN_2, LOW);

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
            // Moving backward straight - white lights off, red lights on constant
            digitalWrite(WHITE_LED_PIN_3, LOW); // Left LED off
            digitalWrite(WHITE_LED_PIN_4, LOW); // Right LED off
            digitalWrite(RED_LED_PIN_1, HIGH);  // Red LEDs on constant
            digitalWrite(RED_LED_PIN_2, HIGH);
        }
        else if (isTurningLeft)
        {
            // Moving backward + turning left - left white blinks, left red blinks, right red constant
            digitalWrite(WHITE_LED_PIN_3, blinkState ? HIGH : LOW); // Left white LED blinks
            digitalWrite(WHITE_LED_PIN_4, LOW);                     // Right white LED off
            digitalWrite(RED_LED_PIN_1, blinkState ? HIGH : LOW);   // Left red LED blinks (turn signal)
            digitalWrite(RED_LED_PIN_2, HIGH);                      // Right red LED constant (reverse light)
        }
        else if (isTurningRight)
        {
            // Moving backward + turning right - right white blinks, right red blinks, left red constant
            digitalWrite(WHITE_LED_PIN_3, LOW);                     // Left white LED off
            digitalWrite(WHITE_LED_PIN_4, blinkState ? HIGH : LOW); // Right white LED blinks
            digitalWrite(RED_LED_PIN_1, HIGH);                      // Left red LED constant (reverse light)
            digitalWrite(RED_LED_PIN_2, blinkState ? HIGH : LOW);   // Right red LED blinks (turn signal)
        }
    }
}
