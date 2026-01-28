#ifndef BOARD_H
#define BOARD_H

#include "Globals.h"

// Mapping for Chain 1 (pinValues)
const char* SQUARES_1[32] = {
    "D1","D2","D3","D4","D5","D6","D7","D8", 
    "C1","C2","C3","C4","C5","C6","C7","C8", 
    "B1","B2","B3","B4","B5","B6","B7","B8", 
    "A1","A2","A3","A4","A5","A6","A7","A8"
};

// Mapping for Chain 2 (pinValues2)
const char* SQUARES_2[32] = {
    "H1","H2","H3","H4","H5","H6","H7","H8", 
    "G1","G2","G3","G4","G5","G6","G7","G8", 
    "F1","F2","F3","F4","F5","F6","F7","F8", 
    "E1","E2","E3","E4","E5","E6","E7","E8"
};

class Board {
private:
    uint32_t lastRead1;
    uint32_t lastRead2;

    uint32_t readChain(uint8_t dataPin) {
        uint32_t bytesVal = 0;
        
        // Latch Pulse
        digitalWrite(PIN_SHIFT_CLOCK_ENABLE, HIGH);
        digitalWrite(PIN_SHIFT_LOAD, LOW);
        delayMicroseconds(5);
        digitalWrite(PIN_SHIFT_LOAD, HIGH);
        digitalWrite(PIN_SHIFT_CLOCK_ENABLE, LOW);

        // Read 32 bits
        for(int i = 0; i < 32; i++) {
            uint8_t val = digitalRead(dataPin);
            // Original logic: First bit read (i=0) goes to Bit 31.
            // i=31 goes to Bit 0.
            if(val) {
                bytesVal |= (1UL << (31 - i));
            }

            // Clock Pulse
            digitalWrite(PIN_SHIFT_CLOCK, HIGH);
            delayMicroseconds(5);
            digitalWrite(PIN_SHIFT_CLOCK, LOW);
        }
        return bytesVal;
    }

public:
    void begin() {
        pinMode(PIN_SHIFT_LOAD, OUTPUT);
        pinMode(PIN_SHIFT_CLOCK_ENABLE, OUTPUT);
        pinMode(PIN_SHIFT_CLOCK, OUTPUT);
        pinMode(PIN_SHIFT_DATA_1, INPUT);
        pinMode(PIN_SHIFT_DATA_2, INPUT);

        digitalWrite(PIN_SHIFT_CLOCK, LOW);
        digitalWrite(PIN_SHIFT_LOAD, HIGH);

        // Initial Read
        lastRead1 = readChain(PIN_SHIFT_DATA_1);
        lastRead2 = readChain(PIN_SHIFT_DATA_2);
    }

    // Check for changes and return the coordinate string of the CHANGED bit.
    // Returns empty string if no change or multiple changes (logic follows original intention).
    // Check for changes with Robust Stability Check
    String getChange() {
        uint32_t live1 = readChain(PIN_SHIFT_DATA_1);
        uint32_t live2 = readChain(PIN_SHIFT_DATA_2);

        // 1. Fast exit if no change from last known state
        if (live1 == lastRead1 && live2 == lastRead2) return "";

        // 2. Stability Check: The NEW state must remain exactly consistent for 300ms
        // If it flickers back to old state or changes to a 3rd state, we reject it as noise.
        unsigned long start = millis();
        while (millis() - start < 300) {
            uint32_t check1 = readChain(PIN_SHIFT_DATA_1);
            uint32_t check2 = readChain(PIN_SHIFT_DATA_2);
            if (check1 != live1 || check2 != live2) {
                // Signal is unstable (glitching / sliding transition)
                return ""; 
            }
            delay(10);
        }

        // 3. Accepted State 'live' is stable. Find diff vs 'lastRead'.
        String changedSq = "";

        // Check Chain 1
        if (live1 != lastRead1) {
            for(int i = 0; i < 32; i++) {
                // Check Bit Difference
                if ( ((live1 >> i) & 1) != ((lastRead1 >> i) & 1) ) {
                    changedSq = SQUARES_1[i];
                    
                    bool isPlace = ((live1 >> i) & 1); // 1 = Place, 0 = Lift

                    // PARTIAL UPDATE
                    if (isPlace) {
                        lastRead1 |= (1UL << i);
                        return "+" + changedSq;
                    } else {
                        lastRead1 &= ~(1UL << i);
                        return "-" + changedSq;
                    }
                }
            }
        }

        // Check Chain 2
        if (live2 != lastRead2) {
            for(int i = 0; i < 32; i++) {
                if ( ((live2 >> i) & 1) != ((lastRead2 >> i) & 1) ) {
                    changedSq = SQUARES_2[i];
                    
                    bool isPlace = ((live2 >> i) & 1);

                    if (isPlace) {
                        lastRead2 |= (1UL << i);
                        return "+" + changedSq;
                    } else {
                        lastRead2 &= ~(1UL << i);
                        return "-" + changedSq;
                    }
                }
            }
        }

        return ""; // Should not reach here if diff exists
    }
    
    // Debug print
    void printState() {
        // Not implemented to save space, unless needed
    }
};

#endif
