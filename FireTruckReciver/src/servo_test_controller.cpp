#include "servo_test_controller.h"

void ServoTestController::initialize()
{
    testServo.attach(SERVO_TEST_PIN);
    lastDebugTime = 0;

    Serial.println("=== Servo Test Controller Initialized ===");
    Serial.print("Servo on pin ");
    Serial.println(SERVO_TEST_PIN);
    Serial.print("Potentiometer on A");
    Serial.println(POTENTIOMETER_TEST_PIN - A0);
    Serial.print("Safe servo range: ");
    Serial.print(SERVO_MIN_ANGLE);
    Serial.print("-");
    Serial.print(SERVO_MAX_ANGLE);
    Serial.println(" degrees");
    Serial.println("Use potentiometer OR radio throttle to control servo");

    // Center servo at startup
    testServo.write(SERVO_CENTER_ANGLE);
    delay(500);
}

void ServoTestController::updateFromPotentiometer()
{
    // Read potentiometer value (0-1023)
    int potValue = analogRead(POTENTIOMETER_TEST_PIN);

    // Map potentiometer value to SAFE servo angle range
    int servoAngle = map(potValue, 0, 1023, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

    // Additional safety constraint
    servoAngle = constrain(servoAngle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

    // Move servo to calculated angle
    testServo.write(servoAngle);

    // Debug output
    debugOutput(potValue, servoAngle);
}

void ServoTestController::updateFromRadio(int throttleValue)
{
    // Map radio throttle (0-255) to SAFE servo angle range
    // Throttle 127 = center = SERVO_CENTER_ANGLE
    int servoAngle = map(throttleValue, 0, 255, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

    // Additional safety constraint
    servoAngle = constrain(servoAngle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

    // Move servo to calculated angle
    testServo.write(servoAngle);

    // Debug output with radio values
    unsigned long currentTime = millis();
    if (currentTime - lastDebugTime >= DEBUG_INTERVAL)
    {
        Serial.print("Radio Throttle: ");
        Serial.print(throttleValue);
        Serial.print(" | Safe Servo Angle: ");
        Serial.println(servoAngle);
        lastDebugTime = currentTime;
    }
}

void ServoTestController::debugOutput(int potValue, int servoAngle)
{
    unsigned long currentTime = millis();
    if (currentTime - lastDebugTime >= DEBUG_INTERVAL)
    {
        Serial.print("Pot Value: ");
        Serial.print(potValue);
        Serial.print(" | Safe Servo Angle: ");
        Serial.println(servoAngle);
        lastDebugTime = currentTime;
    }
}
