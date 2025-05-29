#ifndef PINS_H
#define PINS_H

// PIN DEFINITIONS FOR ARDUINO MEGA 2560
// Don't use pins 0, 1 (Serial)
// Digital pins:
#define WHITE_LED_PIN_3 22 // Changed from 7 (conflicts with nRF24L01 CE)
#define WHITE_LED_PIN_4 23 // Changed from 8 (conflicts with nRF24L01 CSN)
#define BLUE_LED_PIN_1 24
#define BLUE_LED_PIN_2 25
#define DEAD_PIN_4 4
#define THROTTLE_A1_PIN 6
#define THROTTLE_A2_PIN 5
#define STEERING_A3_PIN 9
#define STEERING_A4_PIN 10

// Analog pins:
#define BUTTON_PIN 14        // A0 pin as digital pin A0-A5 = 14-19
#define VOLTAGE_SENSE_PIN A1 // Voltage divider input for battery/PSU voltage sensing

#endif

// ARDUINO MEGA 2560 PIN USAGE:
// Dead Pins: None
// nRF24L01 pins: 7 (CE), 8 (CSN), 11 (MOSI), 12 (MISO), 13 (SCK)
// Used Digital Pins: 2,3 (Blue LEDs), 5,6 (Throttle), 9,10 (Steering), 22,23 (White LEDs)
// Used Analog Pins: A0 (button), A1 (voltage sensing)
// Free Digital Pins: Many available (24-53, etc.)
// Free Analog Pins: A2-A15 (Mega has 16 analog pins!)

// nrf24l01 library standard pins for ARDUINO MEGA 2560:
// VCC  | GND  : 3.3V (NOT 5V!), GND
// CSN  | CE   : 8,7
// MOSI | SCK  : 51,52 (DIFFERENT from Uno!)
// IRQ  | MISO : not used, 50

// PIN | FUNCTION (MEGA 2560)
// 7  | CE (nRF24L01)
// 8  | CSN (nRF24L01)
// 50 | MISO (nRF24L01) - DIFFERENT from Uno!
// 51 | MOSI (nRF24L01) - DIFFERENT from Uno!
// 52 | SCK (nRF24L01) - DIFFERENT from Uno!
