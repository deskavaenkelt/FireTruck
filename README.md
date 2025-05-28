# Fire Truck RC Project

Ett fjärrstyrt brandbilsprojekt med Arduino och nRF24L01 radiokommunikation.

## Översikt

Projektet består av två delar:

- **FireTruckSender**: Fjärrkontroll med joystick
- **FireTruckReciver**: Mottagare på brandbilen

## Hårdvara

### Sändare (FireTruckSender)

- Arduino UNO/Nano
- nRF24L01 radiomodul
- 2× Analog joystick (styrning + gas/broms)
- LED för statusindikering

### Mottagare (FireTruckReciver)

- Arduino UNO
- nRF24L01 radiomodul
- H-brygga för motorstyrning
- DC-motor för styrning (6V original)
- DC-motor för framdrivning
- Tryckknapp
- LED-belysning (blå + vit)

## Spänningshantering

### Problem

Brandbilen är ursprungligen designad för 6V, men kan drivas med:

- 6V (original)
- 7.2V LiPo batteri
- 12V nätaggregat (för testning)

### Lösning: Automatisk spänningsanpassning

#### Spänningsdelare för A1

För att skydda Arduino (max 5V input) och automatiskt anpassa motoreffekt:

```
H-Brygga VCC ----[15kΩ]----+----[10kΩ]---- GND
                           |
                          A1 (Arduino)
```

#### Komponentlista

- 1× 15kΩ motstånd
- 1× 10kΩ motstånd

#### Beräkningar

- **Delningsförhållande**: 10kΩ/(10kΩ+15kΩ) = 0.4
- **12V input**: 12V × 0.4 = 4.8V → Säkert för Arduino
- **7.2V input**: 7.2V × 0.4 = 2.88V
- **6V input**: 6V × 0.4 = 2.4V

#### PWM-anpassning

Systemet skalar automatiskt PWM-värden för att hålla ~6V till motorerna:

- **12V PSU**: PWM skalas till 50% (6V/12V)
- **7.2V LiPo**: PWM skalas till 83% (6V/7.2V)
- **6V batteri**: PWM körs på 100%

### Testläge

För säker testning utan spänningsdelare:

```cpp
static const bool BENCH_TEST_MODE = true; // Säker 12V-testning
```

## Pin-konfiguration

### Mottagare (FireTruckReciver)

```cpp
// Digitala pins
#define BLUE_LED_PIN_1 2
#define BLUE_LED_PIN_2 3
#define WHITE_LED_PIN_3 7
#define WHITE_LED_PIN_4 8
#define THROTTLE_A1_PIN 6      // Motor framåt/bakåt
#define THROTTLE_A2_PIN 5      // Motor framåt/bakåt
#define STEERING_A3_PIN 9      // Styrmotor vänster
#define STEERING_A4_PIN 10     // Styrmotor höger

// Analoga pins
#define BUTTON_PIN A0          // Tryckknapp
#define VOLTAGE_SENSE_PIN A1   // Spänningsavkänning
```

### Lediga pins

- **Analoga**: A2, A3, A4, A5
- **Digitala**: 11, 12, 13

## Funktioner

### Styrning

- **Proportionell styrning** med joystick
- **Return pulse-system** för bättre centrering av DC-motor
- **Asymmetrisk kompensation** för ojämn motorprestanda
- **Smoothing** för mjuka rörelser

### Motorstyrning

- **Proportionell gas/broms** med joystick
- **Failsafe** - stoppar vid förlorad radiokontakt
- **Spänningsanpassning** för olika batterispänningar

### Kommunikation

- **nRF24L01** 2.4GHz radio
- **Optimerad timing** för låg latens
- **Automatisk retry** vid förlorade paket

## Installation

### 1. Hårdvara

1. Montera komponenter enligt pin-konfiguration
2. Installera spänningsdelare (15kΩ + 10kΩ) mellan H-brygga VCC och A1
3. Anslut nRF24L01 moduler

### 2. Mjukvara

1. Installera PlatformIO
2. Klona projektet
3. Kompilera och ladda upp till respektive Arduino

### 3. Konfiguration

- För normal drift: `BENCH_TEST_MODE = false`
- För 12V-testning: `BENCH_TEST_MODE = true`

## Felsökning

### Styrning oscillerar

- Kontrollera return pulse-inställningar
- Justera `SMOOTHING_FACTOR` (0.1-0.9)

### Asymmetrisk styrning

- Justera `maxLeftPower` och `maxRightPower`
- Kontrollera motoranslutningar

### Intermittent radiokommunikation

- Kontrollera nRF24L01 anslutningar
- Justera `LOOP_INTERVAL` timing

### Spänningsavläsning

Aktivera debug-output:

```cpp
// I steering_controller.cpp, ta bort kommentarer:
Serial.print("Supply Voltage: ");
Serial.print(supplyVoltage);
Serial.println("V");
```

## Framtida förbättringar

- Servo/stegmotor för styrning
- Bättre batterispänningsindikering
- Telemetri (hastighet, batteristatus)
- Ljud och ljuseffekter

## Licens

MIT License - Se LICENSE fil för detaljer.
