#ifndef INPUT_H
#define INPUT_H

#include "Globals.h"

class InputController {
public:
    void begin() {
        pinMode(PIN_BTN_INPUT, INPUT_PULLUP);
        pinMode(PIN_BTN_OK, INPUT_PULLUP);
        pinMode(PIN_BTN_DEL, INPUT_PULLUP);
        pinMode(PIN_BTN_RESTART, INPUT_PULLUP); // Handled by interrupt, but init here too
    }

    // Blocking selection for Setup Phase
    // Returns value between 1 and maxVal (e.g. 8 buttons -> mapped later)
    // Actually original returned raw count, called with 1..8 range assumption?
    // Original: map(CountButtons(), 1, 8, ...)
    int getSelection() {
        int counter = 0;
        while(true) {
            if (digitalRead(PIN_BTN_INPUT) == LOW) {
                counter++;
                delay(300); // Simple debounce/repeat delay
            }
            
            if (digitalRead(PIN_BTN_DEL) == LOW) {
                if(counter > 0) counter--;
                delay(300);
            }

            if (digitalRead(PIN_BTN_OK) == LOW) {
                delay(300);
                return counter;
            }
            
            // Allow restarting during setup?
            // If restart interrupt happens, strictly it resets the board.
        }
    }
    
    // Generic Selection: Returns value between min and max
    // type: Character prefix to send to Pi for display updates (e.g. 'Q' for Quadrants). 0 = None.
    int selectOption(int minVal, int maxVal, char type) {
        int current = minVal;
        // Initial visual update
        if (type != 0) {
            Serial.print("heypi"); Serial.print(type); Serial.print(":"); Serial.println(current);
        } else {
            Serial.print("Selection: "); Serial.println(current);
        }
        
        while(true) {
            bool changed = false;
            // Check Input (Increment)
            if (digitalRead(PIN_BTN_INPUT) == LOW) {
                current++;
                if (current > maxVal) current = minVal; // Wrap around
                changed = true;
                delay(200);
            }
            
            // Check Delete (Decrement)
            if (digitalRead(PIN_BTN_DEL) == LOW) {
                current--;
                if (current < minVal) current = maxVal; // Wrap around
                changed = true;
                delay(200);
            }

            if (changed) {
                if (type != 0) {
                     Serial.print("heypi"); Serial.print(type); Serial.print(":"); Serial.println(current);
                } else {
                     Serial.print("Selection: "); Serial.println(current);
                }
            }

            // Check OK
            if (digitalRead(PIN_BTN_OK) == LOW) {
                Serial.println("Confirmed.");
                delay(300);
                return current;
            }
            delay(50); // Small loop delay
        }
    }

    // Ensure buttons are released before starting input phase
    // Helps with Pin 13 LED pull-down issues or sticky buttons
    void waitForRelease() {
        // Check if button is pressed (LOW)
        if (digitalRead(PIN_BTN_OK) == LOW) {
            Serial.println("Debug: Waiting for OK button release (Pin 13)...");
            
            unsigned long startTime = millis();
            // Wait for release, but add a 3-second timeout to prevent infinite hanging
            // due to Pin 13 LED circuitry issues.
            while (digitalRead(PIN_BTN_OK) == LOW) {
                if (millis() - startTime > 3000) {
                    Serial.println("Warning: Button stuck LOW (Pin 13 LED issue?). Continuing.");
                    break;
                }
                delay(100);
            }
            Serial.println("Debug: OK button logic clear.");
        }
    }
};

#endif
