#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

// --- Pin Definitions ---

// Shift Register Pins
const uint8_t PIN_SHIFT_LOAD = 8;
const uint8_t PIN_SHIFT_CLOCK_ENABLE = 4;
const uint8_t PIN_SHIFT_CLOCK = A1;
const uint8_t PIN_SHIFT_DATA_1 = 2; // First chain (Rows A-D)
const uint8_t PIN_SHIFT_DATA_2 = 7; // Second chain (Rows E-H)

// Button Pins
const uint8_t PIN_BTN_RESTART = 11; // Interrupt Pin
const uint8_t PIN_BTN_INPUT = 10;   // Increment
const uint8_t PIN_BTN_OK = A2;      // Confirm (Moved from 13 to avoid LED conflict)
const uint8_t PIN_BTN_DEL = 12;     // Decrement

// Magnet / Robot (Unused in main logic but preserved)
const uint8_t PIN_MAGNET = A0;
const uint8_t PIN_ROBO_BASE = 3;
const uint8_t PIN_ROBO_SHOULDER = 5;
const uint8_t PIN_ROBO_ELBOW = 6;
const uint8_t PIN_ROBO_WRIST = 9;

// --- Constants ---
const unsigned long DEBOUNCE_MS = 50; // Standard debounce
const unsigned long INPUT_DELAY_MS = 300; // Original delay for input counting

// --- Globals ---
extern volatile bool restartRequest;


#endif
