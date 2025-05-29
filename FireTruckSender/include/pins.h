#ifndef PINS_H
#define PINS_H

// PIN DEFINITIONS
// Don't use pins 0, 1
// Digital pins:
#define LED_PIN 6

// Analog pins:
#define STEERING_PIN A0
#define THROTTLE_PIN A1

#endif

// nrf24l01 library standard pins from under side and pins to the right
// VCC  | GND  : 1,9-3,6V
// CSN  | CE   : 7,8
// MOSI | SCK  : 10,9
// IRQ  | MISO : 11,12

// PIN | FUNCTION
// 7  | CE
// 8  | CSN
// 11 | MOSI
// 12 | MISO
// 13 | SCK

// Dead Pins: None
// Free Analog Pins: A2, A3, A4, A5
// Free Digital Pins: 3, 4, 5, 9, 10
// Used Analog Pins: A0 (button), A1 (voltage sensing)
