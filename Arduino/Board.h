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
    // Includes a robust debounce to prevent ghost inputs like bouncing H3 fields.
    String getChange() {
        uint32_t live1 = readChain(PIN_SHIFT_DATA_1);
        uint32_t live2 = readChain(PIN_SHIFT_DATA_2);

        // 1. Fast exit if no change from last known state
        if (live1 == lastRead1 && live2 == lastRead2) return "";

        // 2. We detected a change! Wait 100ms to allow physical bounce to settle
        delay(100);
        
        // 3. Read again to get the STABILIZED state
        uint32_t final1 = readChain(PIN_SHIFT_DATA_1);
        uint32_t final2 = readChain(PIN_SHIFT_DATA_2);

        // If the stabilized state is identical to our last known state, it was just a bounce.
        if (final1 == lastRead1 && final2 == lastRead2) return "";

        String changedSq = "";

        // 4. Process Chain 1 (Stable Change)
        if (final1 != lastRead1) {
            for(int i = 0; i < 32; i++) {
                if ( ((final1 >> i) & 1) != ((lastRead1 >> i) & 1) ) {
                    changedSq = SQUARES_1[i];
                    bool isPlace = ((final1 >> i) & 1); // 1 = Place, 0 = Lift

                    // Update our confirmed state
                    if (isPlace) lastRead1 |= (1UL << i);
                    else         lastRead1 &= ~(1UL << i);
                    
                    return (isPlace ? "+" : "-") + changedSq;
                }
            }
        }

        // 5. Process Chain 2 (Stable Change)
        if (final2 != lastRead2) {
            for(int i = 0; i < 32; i++) {
                if ( ((final2 >> i) & 1) != ((lastRead2 >> i) & 1) ) {
                    changedSq = SQUARES_2[i];
                    bool isPlace = ((final2 >> i) & 1);

                    // Update our confirmed state
                    if (isPlace) lastRead2 |= (1UL << i);
                    else         lastRead2 &= ~(1UL << i);
                    
                    return (isPlace ? "+" : "-") + changedSq;
                }
            }
        }

        return ""; 
    }
    
    // Check if a specific square (File 0-7, Rank 0-7) is occupied
    // Based on last known state (lastRead1/2)
    bool isOccupied(int file, int rank) {
        if (file < 0 || file > 7 || rank < 0 || rank > 7) return false;

        // Chain 1: Files A-D (0-3)
        // D(3)=0-7, C(2)=8-15, B(1)=16-23, A(0)=24-31
        if (file <= 3) {
            int blockStart = (3 - file) * 8;
            int bitIdx = blockStart + rank;
            return (lastRead1 >> bitIdx) & 1;
        } 
        // Chain 2: Files E-H (4-7)
        // H(7)=0-7, G(6)=8-15, F(5)=16-23, E(4)=24-31
        else {
            int blockStart = (7 - file) * 8;
            int bitIdx = blockStart + rank;
            return (lastRead2 >> bitIdx) & 1;
        }
    }
    
    uint32_t getState1() { return lastRead1; }
    uint32_t getState2() { return lastRead2; }

    // Force re-read of board state (after robot move, to prevent ghost detections)
    void resync() {
        lastRead1 = readChain(PIN_SHIFT_DATA_1);
        lastRead2 = readChain(PIN_SHIFT_DATA_2);
    }

    // Debug print
    void printState() {
        // Force a fresh read for debugging
        uint32_t val1 = readChain(PIN_SHIFT_DATA_1);
        uint32_t val2 = readChain(PIN_SHIFT_DATA_2);
        
        Serial.println(F("\n=== BOARD SHIFT REGISTER STATE ==="));
        Serial.print(F("Chain 1 (A-D): ")); Serial.println(val1, BIN);
        Serial.print(F("Chain 2 (E-H): ")); Serial.println(val2, BIN);
        Serial.println(F("=================================="));
        
        // Print as 8x8 grid for easy visual check (File A-H, Rank 1-8)
        for(int rank = 7; rank >= 0; rank--) { // Print Rank 8 at top
            Serial.print(rank + 1); Serial.print("  ");
            for(int file = 0; file < 8; file++) {
                bool occupied = false;
                if (file <= 3) {
                    int blockStart = (3 - file) * 8;
                    occupied = (val1 >> (blockStart + rank)) & 1;
                } else {
                    int blockStart = (7 - file) * 8;
                    occupied = (val2 >> (blockStart + rank)) & 1;
                }
                Serial.print(occupied ? "1 " : "0 ");
            }
            Serial.println();
        }
        Serial.println("   A B C D E F G H");
        Serial.println(F("==================================\n"));
    }
};

#endif
