#include "steering_servo_controller.h"

void SteeringServoController::initialize()
{
    steeringServo.attach(STEERING_SERVO_PIN);
    currentAngle = SERVO_CENTER_ANGLE;
    lastUpdateTime = 0;

    Serial.println("=== Steering Servo Controller Initialized ===");
    Serial.print("Steering servo on pin ");
    Serial.println(STEERING_SERVO_PIN);
    Serial.print("Full servo range: ");
    Serial.print(SERVO_MIN_ANGLE);
    Serial.print("-");
    Serial.print(SERVO_MAX_ANGLE);
    Serial.print(" degrees (");
    Serial.print(SERVO_TOTAL_RANGE);
    Serial.println(" total)");
    Serial.print("Center position: ");
    Serial.print(SERVO_CENTER_ANGLE);
    Serial.println(" degrees");
    Serial.print("Left range: ");
    Serial.print(SERVO_LEFT_RANGE);
    Serial.print("° | Right range: ");
    Serial.print(SERVO_RIGHT_RANGE);
    Serial.println("°");
    Serial.println("NOTE: Manually adjust servo arm so center position aligns with straight ahead");

    // Center steering at startup
    centerSteering();
    delay(500);
}

void SteeringServoController::control(int steeringAngle)
{
    unsigned long currentTime = millis();

    // Only update if enough time has passed for smooth movement
    if (currentTime - lastUpdateTime >= UPDATE_INTERVAL)
    {
        // Map radio steering angle (0-255) to safe servo range
        // Assuming 127 is center position from radio
        int targetAngle = map(steeringAngle, 0, 255, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

        // Additional safety constraint
        targetAngle = constrain(targetAngle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

        // Only move if angle has changed significantly (reduce jitter)
        if (abs(targetAngle - currentAngle) > 2)
        {
            currentAngle = targetAngle;
            steeringServo.write(currentAngle);

            // Debug output occasionally
            static unsigned long lastDebug = 0;
            if (currentTime - lastDebug >= 1000)
            { // Every 1 second
                int relativeAngle = currentAngle - SERVO_CENTER_ANGLE;
                String direction = (relativeAngle < 0) ? "LEFT" : (relativeAngle > 0) ? "RIGHT"
                                                                                      : "CENTER";

                Serial.print("Steering: Radio=");
                Serial.print(steeringAngle);
                Serial.print(" | Servo=");
                Serial.print(currentAngle);
                Serial.print("° | Relative=");
                Serial.print(abs(relativeAngle));
                Serial.print("° ");
                Serial.println(direction);
                lastDebug = currentTime;
            }
        }

        lastUpdateTime = currentTime;
    }
}

void SteeringServoController::centerSteering()
{
    currentAngle = SERVO_CENTER_ANGLE;
    steeringServo.write(currentAngle);
    Serial.println("Steering centered");
}

int SteeringServoController::getCurrentAngle()
{
    return currentAngle;
}

bool SteeringServoController::isActive()
{
    return steeringServo.attached();
}
