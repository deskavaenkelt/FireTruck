#include "audio_controller.h"

void AudioController::initialize()
{
    pinMode(SPEAKER_PIN, OUTPUT);
    pinMode(AUDIO_ENABLE_PIN, OUTPUT);

    audioEnabled = false;
    lastSirenTime = 0;
    sirenState = false;

    // Start with audio disabled and clean state
    digitalWrite(SPEAKER_PIN, LOW);
    digitalWrite(AUDIO_ENABLE_PIN, LOW);
    noTone(SPEAKER_PIN);
    delay(100); // Let pins stabilize

    Serial.println("=== Audio Controller Initialized (SIMPLIFIED) ===");
    Serial.print("Speaker pin (to PAM8403 L_IN): ");
    Serial.println(SPEAKER_PIN);
    Serial.print("Amplifier enable pin: ");
    Serial.println(AUDIO_ENABLE_PIN);
    Serial.println("PAM8403 Connection Guide:");
    Serial.print("L_IN  -> Pin ");
    Serial.print(SPEAKER_PIN);
    Serial.println(" (Arduino)");
    Serial.println("R_IN  -> GND (mono setup)");
    Serial.println("VCC   -> 5V (stable power!)");
    Serial.println("GND   -> GND");
    Serial.println("L_OUT -> Speaker +");
    Serial.println("R_OUT -> Speaker - (or leave unconnected)");
    Serial.println("NO VOLUME CONTROL - Fixed volume from PAM8403");
}

void AudioController::enable()
{
    Serial.println("Enabling audio system (simplified)...");

    // Stop any existing tones first
    noTone(SPEAKER_PIN);
    delay(50);

    // Enable amplifier
    digitalWrite(AUDIO_ENABLE_PIN, HIGH);
    delay(200); // Let amplifier stabilize longer

    audioEnabled = true;

    Serial.println("Audio system enabled - fixed volume");
}

void AudioController::disable()
{
    Serial.println("Disabling audio system...");

    audioEnabled = false;

    // Stop all tones first
    noTone(SPEAKER_PIN);
    delay(50);

    // Disable amplifier
    digitalWrite(AUDIO_ENABLE_PIN, LOW);
    delay(50);

    Serial.println("Audio system disabled");
}

void AudioController::playStartupSound()
{
    if (!audioEnabled)
        return;

    Serial.println("Playing startup sound");
    // Simple startup beep - non-blocking
    playTone(500, 200);
}

void AudioController::playSiren()
{
    if (!audioEnabled)
        return;

    unsigned long currentTime = millis();
    if (currentTime - lastSirenTime >= SIREN_INTERVAL)
    {
        // Stop current tone cleanly before starting new one
        noTone(SPEAKER_PIN);
        delay(5); // Short pause for clean transition

        if (sirenState)
        {
            // Play high frequency - longer duration for smooth sound
            tone(SPEAKER_PIN, SIREN_FREQ_HIGH, SIREN_INTERVAL - 10);
        }
        else
        {
            // Play low frequency - longer duration for smooth sound
            tone(SPEAKER_PIN, SIREN_FREQ_LOW, SIREN_INTERVAL - 10);
        }
        sirenState = !sirenState;
        lastSirenTime = currentTime;
    }
}

void AudioController::playHorn()
{
    if (!audioEnabled)
        return;

    Serial.println("Playing horn");
    playTone(HORN_FREQUENCY, 1000); // 1 second horn
}

void AudioController::playBackupBeep()
{
    if (!audioEnabled)
        return;

    // Short, clear backup beep
    playTone(BACKUP_BEEP_FREQ, 150);
}

void AudioController::playTone(int frequency, int duration)
{
    if (!audioEnabled)
        return;

    // Stop any existing tone first
    noTone(SPEAKER_PIN);
    delay(10); // Small stabilization delay

    Serial.print("Playing tone: ");
    Serial.print(frequency);
    Serial.print("Hz for ");
    Serial.print(duration);
    Serial.println("ms");

    // Use Arduino's tone() function for clean square waves
    tone(SPEAKER_PIN, frequency, duration);

    // Don't wait for completion to avoid blocking
    // The tone will stop automatically after duration
}

void AudioController::stopAllSounds()
{
    // Stop tone generation
    noTone(SPEAKER_PIN);
    sirenState = false;
}

bool AudioController::isEnabled()
{
    return audioEnabled;
}

void AudioController::testBasicTone()
{
    Serial.println("=== Audio Test: MAXIMUM TEST TONE ===");
    Serial.println("Testing 1000Hz for 5 seconds at max amplitude...");

    // Play a continuous test tone for longer
    tone(SPEAKER_PIN, 1000); // Continuous tone
    delay(5000);             // Let it play for 5 seconds
    noTone(SPEAKER_PIN);     // Stop it

    Serial.println("MAXIMUM test tone complete - did you hear it?");
    Serial.println("If no sound, check PAM8403 L_OUT -> Speaker connection");
}

void AudioController::debugAudioPins()
{
    Serial.println("=== Audio Pin Debug (Simplified) ===");
    Serial.print("SPEAKER_PIN (");
    Serial.print(SPEAKER_PIN);
    Serial.print("): ");
    Serial.println(digitalRead(SPEAKER_PIN));

    Serial.print("AUDIO_ENABLE_PIN (");
    Serial.print(AUDIO_ENABLE_PIN);
    Serial.print("): ");
    Serial.println(digitalRead(AUDIO_ENABLE_PIN));

    // Test if pins are working
    Serial.println("Stopping all tones...");
    noTone(SPEAKER_PIN);
    digitalWrite(AUDIO_ENABLE_PIN, LOW);
    delay(1000);

    Serial.println("Should be silent now");
}

void AudioController::testAllSounds()
{
    if (!audioEnabled)
    {
        Serial.println("Audio not enabled - enabling now...");
        enable();
        delay(500);
    }

    Serial.println("=== Testing All Fire Truck Sounds (Simplified) ===");

    Serial.println("1. Testing basic tones...");
    playTone(440, 500); // A note
    delay(200);
    playTone(880, 500); // High A note
    delay(500);

    Serial.println("2. Startup sound...");
    playStartupSound();
    delay(1000);

    Serial.println("3. Horn...");
    playHorn();
    delay(1500);

    Serial.println("4. Backup beep sequence...");
    for (int i = 0; i < 3; i++)
    {
        playBackupBeep();
        delay(800);
    }
    delay(500);

    Serial.println("5. Siren test (manual control)...");
    Serial.println("   Press button on remote to test siren");
    Serial.println("Sound test complete!");
}

void AudioController::testHardware()
{
    Serial.println("=== Hardware Test: Manual Pin Toggle ===");
    Serial.print("Testing pin ");
    Serial.println(SPEAKER_PIN);
    Serial.println("You should hear clicking or see LED blink if connected to pin");

    // Manual pin toggle test
    for (int i = 0; i < 20; i++)
    {
        digitalWrite(SPEAKER_PIN, HIGH);
        delay(100);
        digitalWrite(SPEAKER_PIN, LOW);
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Hardware test complete");
    Serial.println("If you heard clicking, pin is working");
    Serial.println("If no sound, check PAM8403 connections");
}

void AudioController::testExtremeBasic()
{
    Serial.println("=== EXTREME BASIC TEST: RAW PIN OPERATION ===");

    // Disable amplifier completely first
    digitalWrite(AUDIO_ENABLE_PIN, LOW);
    delay(500);

    // Set pin as output and test directly
    pinMode(SPEAKER_PIN, OUTPUT);

    Serial.println("Testing pin 29 WITHOUT PAM8403...");
    Serial.println("Connect LED or speaker DIRECTLY to pin 29 + GND");
    Serial.println("You should see blinking or hear clicking");

    // Extreme basic test - just toggle the pin
    for (int i = 0; i < 50; i++)
    {
        digitalWrite(SPEAKER_PIN, HIGH);
        delay(200);
        digitalWrite(SPEAKER_PIN, LOW);
        delay(200);

        if (i % 10 == 0)
        {
            Serial.print("Toggle #");
            Serial.println(i);
        }
    }

    Serial.println("If NO blinking/clicking: Pin 29 may be defective");
    Serial.println("If YES blinking/clicking: Pin works, problem is with PAM8403");

    // Test different pin as backup
    Serial.println("\nTesting backup pin 31...");
    pinMode(31, OUTPUT);
    for (int i = 0; i < 20; i++)
    {
        digitalWrite(31, HIGH);
        delay(100);
        digitalWrite(31, LOW);
        delay(100);
    }

    Serial.println("Test complete. Try 'e' command for this test");
}
