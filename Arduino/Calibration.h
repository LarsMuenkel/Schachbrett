#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Arduino.h>
#include "Robot.h"

// Helper class for the calibration routine
struct CompactPose {
    uint8_t base;
    uint8_t shoulder;
    uint8_t elbow;
    uint8_t wrist;
    // Gripper removed to save 64 bytes of RAM (Magnet is global pin)
};

class Calibrator {
private:
    // Store calibrated positions for all 64 squares (8 files x 8 ranks)
    // Using compact struct to save RAM (256 bytes)
    CompactPose calibratedPositions[8][8];
    // Graveyard positions (14 slots)
    CompactPose calibratedGraveyard[14];

    RobotPose toRobotPose(CompactPose c) {
        return { (uint8_t)c.base, (uint8_t)c.shoulder, (uint8_t)c.elbow, (uint8_t)c.wrist, 0 };
    }
    
    CompactPose fromRobotPose(RobotPose p) {
        return { (uint8_t)p.base, (uint8_t)p.shoulder, (uint8_t)p.elbow, (uint8_t)p.wrist };
    }

    void printControls() {
        Serial.println(F("\n--- CALIBRATION CONTROLS ---"));
        Serial.println(F("Adjust Servo Angles (+/- 1 degree):"));
        Serial.println(F("  Base:     'q' (+) / 'a' (-)"));
        Serial.println(F("  Shoulder: 'w' (+) / 's' (-)"));
        Serial.println(F("  Elbow:    'e' (+) / 'd' (-)"));
        Serial.println(F("  Wrist:    'r' (+) / 'f' (-)"));
        Serial.println(F("Set Exact Angle (space required):"));
        Serial.println(F("  'b <val>' (Base), 's <val>' (Shoulder)"));
        Serial.println(F("  'e <val>' (Elbow), 'w <val>' (Wrist)"));
        Serial.println(F("  e.g. 'b 90' or 'base 90'"));
        Serial.println(F("Navigation:"));
        Serial.println(F("  'ok':     Save & Next Square"));
        Serial.println(F("  'back':   Go back to previous square"));
        Serial.println(F("  'jump X<n>': Jump to Graveyard 1-14"));
        Serial.println(F("  'dump':   Print current data array"));
        Serial.println(F("  'exit':   Exit calibration mode"));
        Serial.println(F("----------------------------"));
    }

    void printPose(const RobotPose& p) {
        Serial.print(F("Current: [B:")); Serial.print(p.base);
        Serial.print(F(", S:")); Serial.print(p.shoulder);
        Serial.print(F(", E:")); Serial.print(p.elbow);
        Serial.print(F(", W:")); Serial.print(p.wrist);
        Serial.println(F("]"));
    }

public:
    Calibrator() {
        reset();
    }
    
    void reset() {
         initPredictions();
    }
    
    void initPredictions() {
        Serial.println(F("Loading calibration map from Flash..."));
        
        // Board
        for (int f = 0; f < 8; f++) {
            for (int r = 0; r < 8; r++) {
               RobotPose p = robot.getSquarePose(f, r);
               calibratedPositions[f][r] = fromRobotPose(p);
            }
        }
        
        // Graveyard
        for (int i=0; i<14; i++) {
            RobotPose p = robot.getGraveyardPose(i);
            calibratedGraveyard[i] = fromRobotPose(p);
        }
    }

    void runRoutine() {
        Serial.println(F("ENTERING PICK-AND-PLACE CALIBRATION (Rows 5-8 -> 1-4)."));
        printControls();

        // Start with File A (0), Rank 8 (7) -> Target Rank 4 (3)
        int fileIdx = 0;
        int rankIdx = 7; 
        
        bool isPickPhase = true; // true=Adjusting Source (Pick), false=Adjusting Dest (Place)

        while (true) {
            
            RobotPose currentPose;
            int currentRank = isPickPhase ? rankIdx : (rankIdx - 4);
            
            // Graveyard check? 
            if (fileIdx > 7) {
                // Done with board
                Serial.println(F("Board Complete. Saving..."));
                dumpData();
                return;
            }
            if (fileIdx < 0) { fileIdx = 0; }

            char fileChar = 'A' + fileIdx;
            int rankNum = currentRank + 1;

            Serial.print(F("\n>>> Calibrating "));
            Serial.print(isPickPhase ? F("PICK (Source): ") : F("PLACE (Dest): "));
            Serial.print(fileChar);
            Serial.println(rankNum);

            // 1. Load Pose
            currentPose = toRobotPose(calibratedPositions[fileIdx][currentRank]);
            
            // 2. Move Robot (Hover then Touch)
            // We use a simplified move here so user doesn't wait 5s every adjustment time
            // But for the FIRST move to position, we should be gentle.
            RobotPose pUp = currentPose;
            pUp.shoulder -= 15; pUp.elbow -= 10;
            robot.moveToPose(pUp, 50);
            robot.moveToPose(currentPose, 100);

            // 3. User adjustment loop
            bool stepDone = false;
            while (!stepDone) {
                if (Serial.available() > 0) {
                    String input = Serial.readStringUntil('\n');
                    input.trim();
                    if (input.length() == 0) continue;

                    // --- CMD PARSING ---
                    bool validCmd = false;
                    bool changed = false;
                    String lowerInput = input;
                    lowerInput.toLowerCase();

                    if (lowerInput == "end" || lowerInput == "exit") {
                        Serial.println(F("Ending calibration."));
                        dumpData();
                        return;
                    }
                    else if (lowerInput == "ok") {
                        // SAVE
                        calibratedPositions[fileIdx][currentRank] = fromRobotPose(currentPose);
                        Serial.println(F("Position Saved."));
                        
                        if (isPickPhase) {
                            // EXECUTE PICKUP (Simulation)
                            // We are at 'currentPose' (Down). 
                            // Go Up, Magnet On, Up?
                            // User wants to simulate the grab.
                            Serial.println(F("Grabbing..."));
                            robot.magnetOn();
                            delay(500);
                            robot.moveToPose(pUp, 50); // Lift
                            
                            // Switch to Place Phase
                            isPickPhase = false;
                        } else {
                            // EXECUTE PLACE (Simulation)
                            Serial.println(F("Placing..."));
                            robot.magnetOff();
                            delay(500);
                            robot.moveToPose(pUp, 50); // Lift
                            
                            // Done with this pair. Move to next.
                            // Decrement Rank (7->6->5->4)
                            rankIdx--; 
                            if (rankIdx < 4) {
                                // Done with this file
                                rankIdx = 7; 
                                fileIdx++;
                                Serial.println(F("File Done. Moving to Next File."));
                            }
                            isPickPhase = true; // Back to Pick
                        }
                        stepDone = true; 
                    }
                    else if (input == "dump") { dumpData(); }
                    else {
                        // Adjustments q/a/w/s...
                        int step = 1; 
                        for (unsigned int i = 0; i < input.length(); i++) {
                            char c = input.charAt(i);
                            switch (c) {
                                case 'q': currentPose.base += step; changed = true; break;
                                case 'a': currentPose.base -= step; changed = true; break;
                                case 'w': currentPose.shoulder += step; changed = true; break;
                                case 's': currentPose.shoulder -= step; changed = true; break;
                                case 'e': currentPose.elbow += step; changed = true; break;
                                case 'd': currentPose.elbow -= step; changed = true; break;
                                case 'r': currentPose.wrist += step; changed = true; break;
                                case 'f': currentPose.wrist -= step; changed = true; break;
                            }
                        }
                    }
                    
                    if (changed) {
                        currentPose.base = constrain(currentPose.base, 0, 180);
                        currentPose.shoulder = constrain(currentPose.shoulder, 0, 180);
                        currentPose.elbow = constrain(currentPose.elbow, 0, 180);
                        currentPose.wrist = constrain(currentPose.wrist, 0, 180);
                        
                        // Direct update (Stay down)
                        robot.forcePose(currentPose);
                        printPose(currentPose);
                    }
                }
            } // end while(!stepDone)
        } // end loop
    }

    void dumpData() {
        Serial.println(F("\n\n// --- CALIBRATED POSITIONS COPY START ---"));
        Serial.println(F("// Copy this into a header file or Robot.h"));
        
        // BOARD
        Serial.println(F("const RobotPose CALIBRATED_SQUARES[8][8] PROGMEM = {"));
        for (int f = 0; f < 8; f++) {
            Serial.print(F("  { // File ")); 
            Serial.print((char)('A' + f));
            Serial.println(F(""));
            for (int r = 0; r < 8; r++) {
                RobotPose p = toRobotPose(calibratedPositions[f][r]);
                Serial.print(F("    {"));
                Serial.print(p.base); Serial.print(F(", "));
                Serial.print(p.shoulder); Serial.print(F(", "));
                Serial.print(p.elbow); Serial.print(F(", "));
                Serial.print(p.wrist); Serial.print(F(", 0}"));
                if (r < 7) Serial.print(F(","));
                Serial.print(F("\t// "));
                Serial.print((char)('A' + f));
                Serial.println(r + 1);
            }
            Serial.print(F("  }"));
            if (f < 7) Serial.print(F(","));
            Serial.println();
        }
        Serial.println(F("};"));
        
        // GRAVEYARD
        Serial.println(F("\n// --- GRAVEYARD POSITIONS (Captured Pieces) ---"));
        Serial.println(F("const RobotPose GRAVEYARD_POSITIONS[14] PROGMEM = {"));
        for (int i=0; i<14; i++) {
             RobotPose p = toRobotPose(calibratedGraveyard[i]);
             Serial.print(F("    {"));
             Serial.print(p.base); Serial.print(F(", "));
             Serial.print(p.shoulder); Serial.print(F(", "));
             Serial.print(p.elbow); Serial.print(F(", "));
             Serial.print(p.wrist); Serial.print(F(", 0}"));
             if (i < 13) Serial.print(F(", "));
             if ((i+1) % 2 == 0) Serial.println();
        }
        Serial.println(F("};"));
        
        Serial.println(F("// --- POSITIONS COPY END ---\n"));
    }
};

extern Calibrator calibrator;

#endif
