#ifndef AUDIO_CONTROLLER_H
#define AUDIO_CONTROLLER_H

#include <Arduino.h>
#include "pins.h"

// Audio frequencies (Hz)
const int SIREN_FREQ_LOW = 600;   // Low siren frequency
const int SIREN_FREQ_HIGH = 900;  // High siren frequency
const int HORN_FREQUENCY = 300;   // Deep horn frequency
const int BACKUP_BEEP_FREQ = 800; // Clear backup warning
const int SIREN_INTERVAL = 500;   // Siren alternation interval (ms)

class AudioController
{
public:
    void initialize();
    void enable();
    void disable();

    void playStartupSound();
    void playSiren();
    void playHorn();
    void playBackupBeep();
    void playTone(int frequency, int duration);
    void stopAllSounds();

    bool isEnabled();

    // Test functions
    void testBasicTone();
    void testHardware();
    void testAllSounds();
    void testExtremeBasic();
    void debugAudioPins();

private:
    bool audioEnabled;
    unsigned long lastSirenTime;
    bool sirenState;
};

#endif
