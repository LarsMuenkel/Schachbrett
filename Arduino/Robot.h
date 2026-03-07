#ifndef ROBOT_H
#define ROBOT_H

#include <Arduino.h>
#include <Servo.h>
#include <math.h>
#include <avr/pgmspace.h>

// =============================================================
//   ROBOT ARM CONFIGURATION (MEASURE AND ADJUST THESE)
// =============================================================
// =============================================================

struct RobotPose {
    uint8_t base;
    uint8_t shoulder;
    uint8_t elbow;
    uint8_t wrist;
    uint8_t gripper; 
};


// --- CALIBRATED POSITIONS COPY START ---
// Copy this into a header file or Robot.h
const RobotPose CALIBRATED_SQUARES[8][8] PROGMEM = {
  { // File A
    {65, 134, 50, 65, 0},  // A1
    {61, 112, 84, 53, 0}, // A2
    {57, 99, 85, 80, 0},  // A3
    {52, 96, 73, 111, 0}, // A4
    {48, 82, 85, 116, 0}, // A5
    {41, 71, 89, 128, 0}, // A6
    {32, 65, 89, 139, 0}, // A7
    {17, 62, 88, 147, 0}  // A8
  },
  { // File B
    {73, 128, 57, 70, 0}, // B1
    {68, 114, 64, 88, 0}, // B2
    {64, 103, 68, 107, 0},  // B3
    {60, 92, 73, 121, 0}, // B4
    {57, 75, 86, 126, 0}, // B5
    {50, 66, 89, 138, 0}, // B6
    {40, 50, 98, 145, 0}, // B7
    {24, 21, 116, 147, 0} // B8
  },
  { // File C
    {80, 125, 54, 81, 0}, // C1
    {75, 104, 79, 80, 0}, // C2
    {73, 84, 95, 88, 0},  // C3
    {69, 72, 97, 107, 0}, // C4
    {68, 63, 97, 126, 0}, // C5
    {62, 48, 103, 137, 0},  // C6
    {54, 44, 98, 156, 0}, // C7
    {38, 42, 92, 169, 0}  // C8
  },
  { // File D
    {87, 128, 50, 88, 0}, // D1
    {82, 99, 87, 75, 0},  // D2
    {84, 90, 82, 103, 0}, // D3
    {82, 78, 86, 120, 0}, // D4
    {80, 67, 90, 135, 0}, // D5
    {77, 57, 90, 150, 0}, // D6
    {71, 42, 96, 161, 0}, // D7
    {59, 24, 99, 173, 0}  // D8
  },
  { // File E
    {96, 125, 50, 87, 0}, // E1
    {94, 97, 89, 74, 0},  // E2
    {94, 89, 82, 107, 0}, // E3
    {95, 81, 80, 126, 0}, // E4
    {94, 67, 88, 137, 0}, // E5
    {95, 51, 95, 149, 0}, // E6
    {96, 43, 94, 164, 0}, // E7
    {97, 20, 100, 174, 0} // E8
  },
  { // File F
    {104, 127, 50, 84, 0},  // F1
    {103, 112, 61, 101, 0}, // F2
    {107, 88, 83, 102, 0},  // F3
    {108, 79, 83, 121, 0},  // F4
    {108, 73, 83, 137, 0},  // F5
    {112, 53, 95, 145, 0},  // F6
    {119, 45, 94, 160, 0},  // F7
    {130, 38, 91, 173, 0} // F8
  },
  { // File G
    {112, 124, 65, 61, 0},  // G1
    {112, 107, 75, 81, 0},  // G2
    {116, 93, 81, 98, 0},  // G3
    {118, 86, 77, 121, 0},  // G4
    {120, 70, 89, 129, 0},  // G5
    {127, 58, 95, 138, 0},  // G6
    {134, 52, 93, 152, 0},  // G7
    {146, 40, 96, 162, 0} // G8
  },
  { // File H
    {116, 131, 53, 69, 0},  // H1
    {121, 111, 74, 70, 0},  // H2
    {123, 98, 80, 91, 0}, // H3
    {127, 88, 80, 109, 0},  // H4
    {129, 70, 97, 112, 0},  // H5
    {136, 52, 108, 119, 0}, // H6
    {145, 50, 101, 137, 0}, // H7
    {158, 60, 86, 155, 0} // H8
  }
};



// --- GRAVEYARD POSITIONS (Captured Pieces) ---
const RobotPose GRAVEYARD_POSITIONS[14] PROGMEM = {
    {180, 21, 99, 174, 0},     {162, 26, 96, 174, 0}, 
    {180, 51, 87, 171, 0},     {169, 51, 87, 171, 0}, 
    {180, 51, 87, 167, 0},     {170, 51, 87, 167, 0}, 
    {180, 61, 87, 157, 0},     {170, 61, 87, 157, 0}, 
    {0, 21, 99, 174, 0},     {18, 26, 96, 174, 0}, 
    {0, 51, 87, 171, 0},     {11, 51, 87, 171, 0}, 
    {0, 51, 87, 167, 0},     {10, 51, 87, 167, 0}
};
// --- POSITIONS COPY END ---

// --- Link Lengths (mm) ---
const float L1_SHOULDER = 120.0; // Shoulder to Elbow
const float L2_ELBOW    = 160.0; // Elbow to Wrist
const float L3_WRIST    = 170.0;  // Wrist to Magnet Tip

// --- Board Position (mm) ---
// Coordinate of Board relative to Robot Base (0,0)
const float BOARD_OFFSET_X_FORWARD = 150.0; // Distance to Rank 1 line
const float BOARD_OFFSET_Y_CENTER  = 0.0;   // Lateral offset (0 if centered)
const float SQUARE_SIZE = 35.0; 

// --- Vertical Heights (mm) ---
const float Z_BOARD_SURFACE = 10.0; 
const float Z_HOVER         = 50.0; 
const float Z_PICKUP        = 25.0; 

// --- Servo Calibration ---
const int OFFSET_BASE     = 0; 
const int OFFSET_SHOULDER = 0;
const int OFFSET_ELBOW    = 0;
const int OFFSET_WRIST    = 0;

// --- Magnet Pin ---
#ifndef PIN_MAGNET_CTRL
#define PIN_MAGNET_CTRL A0
#endif

// =============================================================

// Struct moved up


class RobotController {
private:
    Servo base, shoulder, elbow, wrist;
    const int MIN_ANGLE = 0;
    const int MAX_ANGLE = 180;

public:
    RobotPose parkPos;
    RobotPose gamePos; 

    void begin(uint8_t pBase, uint8_t pShoulder, uint8_t pElbow, uint8_t pWrist, uint8_t pGripper) {
        base.attach(pBase);
        shoulder.attach(pShoulder);
        elbow.attach(pElbow);
        wrist.attach(pWrist);
        // Gripper servo unused; using Magnet Pin A0
        
        pinMode(PIN_MAGNET_CTRL, OUTPUT);
        magnetOff();

        parkPos = {0, 75, 110, 165, 0};
        gamePos = {90, 53, 92, 125, 0};
        forcePose(parkPos);
    }
    
    // --- Magnet Control ---
    void magnetOn() {
        digitalWrite(PIN_MAGNET_CTRL, HIGH);
    }
    
    void magnetOff() {
        digitalWrite(PIN_MAGNET_CTRL, LOW);
    }

    void wakeUp() {
        // First lift arm up high to avoid sweeping over pieces (startup only)
        RobotPose safeHigh = parkPos;
        safeHigh.shoulder = 50;  // Arm pointing up
        safeHigh.elbow = 40;
        safeHigh.wrist = 90;
        moveToPose(safeHigh, 20);
        
        // Now swing to game position (arm is safely above board)
        moveToPose(gamePos, 20);
    }

    // Direct return to game position (mid-game, arm is already above board)
    void returnToGame() {
        moveToPose(gamePos, 20);
    }
    
    void park() {
        // First lift arm up high
        RobotPose safeHigh = gamePos;
        safeHigh.shoulder = 50;
        safeHigh.elbow = 40;
        safeHigh.wrist = 90;
        moveToPose(safeHigh, 20);
        
        // Then move to park position
        moveToPose(parkPos, 20);
    }

    void forcePose(RobotPose p) {
        base.write(constrain(p.base, MIN_ANGLE, MAX_ANGLE));
        shoulder.write(constrain(p.shoulder, MIN_ANGLE, MAX_ANGLE));
        elbow.write(constrain(p.elbow, MIN_ANGLE, MAX_ANGLE));
        wrist.write(constrain(p.wrist, MIN_ANGLE, MAX_ANGLE));
    }

    void moveToPose(RobotPose target, int speed) {
        int curBase = base.read();
        int curShoulder = shoulder.read();
        int curElbow = elbow.read();
        int curWrist = wrist.read();
        
        int maxDist = 0;
        maxDist = max(maxDist, abs(target.base - curBase));
        maxDist = max(maxDist, abs(target.shoulder - curShoulder));
        maxDist = max(maxDist, abs(target.elbow - curElbow));
        maxDist = max(maxDist, abs(target.wrist - curWrist));
        
        if (maxDist == 0) return;

        // Use more steps for smoother "slow motion" interpolation
        int steps = maxDist * 3; 
        if (steps < 30) steps = 30;

        for (int i = 0; i <= steps; i++) {
            float t = (float)i / (float)steps;
            
            int b = curBase + (int)((target.base - curBase) * t);
            int s = curShoulder + (int)((target.shoulder - curShoulder) * t);
            int e = curElbow + (int)((target.elbow - curElbow) * t);
            int w = curWrist + (int)((target.wrist - curWrist) * t);
            
            base.write(b);
            shoulder.write(s);
            elbow.write(e);
            wrist.write(w);
            
            // Use the speed/delay parameter provided
            delay(speed); 
        }
    }
    
    // --- INVERSE KINEMATICS ---
    RobotPose getInverseKinematics(float x, float y, float z) {
        RobotPose p = {90, 90, 90, 90, 0}; 
        
        float thetaBase = atan2(y, x); 
        p.base = 90 + degrees(thetaBase); 
        
        float r_ground = sqrt(x*x + y*y);
        
        // Wrist position target (Vertical Magnet)
        float r_wrist = r_ground;
        float z_wrist = z + L3_WRIST;
        
        float dist = sqrt(r_wrist*r_wrist + z_wrist*z_wrist);
        
        if (dist > (L1_SHOULDER + L2_ELBOW)) {
             return parkPos; // Unreachable
        }
        
        // Elbow Angle (Law of Cosines)
        float cos_gamma = (L1_SHOULDER*L1_SHOULDER + L2_ELBOW*L2_ELBOW - dist*dist) / (2.0 * L1_SHOULDER * L2_ELBOW);
        float gamma = acos(constrain(cos_gamma, -1.0, 1.0));
        p.elbow = degrees(gamma); 
        
        // Shoulder Angle
        float alpha1 = atan2(z_wrist, r_wrist);
        float cos_alpha2 = (L1_SHOULDER*L1_SHOULDER + dist*dist - L2_ELBOW*L2_ELBOW) / (2.0 * L1_SHOULDER * dist);
        float alpha2 = acos(constrain(cos_alpha2, -1.0, 1.0));
        
        float thetaShoulder = alpha1 + alpha2; 
        p.shoulder = degrees(thetaShoulder);
        
        // Wrist Angle (Keep Vertical)
        // 180 - Shoulder - (180 - Elbow) + Offset ? 
        // Simple Compensation:
        p.wrist = (180 - p.shoulder) + (180 - p.elbow); 
        
        // Apply Offsets
        p.base     += OFFSET_BASE;
        p.shoulder += OFFSET_SHOULDER;
        p.elbow    += OFFSET_ELBOW;
        p.wrist    += OFFSET_WRIST;
        
        return p;
    }

    RobotPose getSquarePose(int fileIdx, int rankIdx) {
        // Use Calibrated Lookup Table FROM PROGMEM
        if (fileIdx >= 0 && fileIdx < 8 && rankIdx >= 0 && rankIdx < 8) {
             RobotPose p;
             // Read from Flash
             p.base     = pgm_read_byte(&CALIBRATED_SQUARES[fileIdx][rankIdx].base);
             p.shoulder = pgm_read_byte(&CALIBRATED_SQUARES[fileIdx][rankIdx].shoulder);
             p.elbow    = pgm_read_byte(&CALIBRATED_SQUARES[fileIdx][rankIdx].elbow);
             p.wrist    = pgm_read_byte(&CALIBRATED_SQUARES[fileIdx][rankIdx].wrist);
             p.gripper  = 0; // Unused
             return p;
        }
        return parkPos;
    }
    
    RobotPose getGraveyardPose(int index) {
        if (index >= 0 && index < 14) {
             RobotPose p;
             p.base     = pgm_read_byte(&GRAVEYARD_POSITIONS[index].base);
             p.shoulder = pgm_read_byte(&GRAVEYARD_POSITIONS[index].shoulder);
             p.elbow    = pgm_read_byte(&GRAVEYARD_POSITIONS[index].elbow);
             p.wrist    = pgm_read_byte(&GRAVEYARD_POSITIONS[index].wrist);
             p.gripper  = 0; 
             return p;
        }
        return parkPos; // Fallback
    }
    
    // --- FULL MOVE EXECUTION ---
    void executeMove(int fromFile, int fromRank, int toFile, int toRank) {
        // 1. Hover Source
        RobotPose pStart = getSquarePose(fromFile, fromRank);
        moveToPose(pStart, 20);
        
        // 2. Lower to Pickup
        float xSrc = BOARD_OFFSET_X_FORWARD + (fromRank * SQUARE_SIZE) + (SQUARE_SIZE/2.0);
        float ySrc = BOARD_OFFSET_Y_CENTER + ((fromFile - 3.5) * SQUARE_SIZE);
        RobotPose pPick = getInverseKinematics(xSrc, ySrc, Z_PICKUP);
        moveToPose(pPick, 40); // Slow
        
        // 3. Magnet
        magnetOn();
        delay(400); // Engage
        
        // 4. Lift
        moveToPose(pStart, 30);
        
        // 5. Travel to Dest (Hover)
        float xDest = BOARD_OFFSET_X_FORWARD + (toRank * SQUARE_SIZE) + (SQUARE_SIZE/2.0);
        float yDest = BOARD_OFFSET_Y_CENTER + ((toFile - 3.5) * SQUARE_SIZE);
        RobotPose pDestHover = getInverseKinematics(xDest, yDest, Z_HOVER);
        moveToPose(pDestHover, 20);
        
        // 6. Lower to Place
        RobotPose pPlace = getInverseKinematics(xDest, yDest, Z_PICKUP);
        moveToPose(pPlace, 40);
        
        // 7. Drop
        magnetOff();
        delay(400); // Release
        
        // 8. Lift
        moveToPose(pDestHover, 30);
        
        // 9. Park/Wait
        // robot.park(); // Or stay ready? User can decide.
    }

    // --- HIGH LEVEL ACTIONS ---
    
    // Core Primitive: Go Up -> Down -> Magnet On -> Up
    void performPickup(RobotPose pDown, int speed = 30) {
        RobotPose pUp = pDown;
        // Lift slightly (Shoulder -15, Elbow -10 seems okay as heuristic for "Up")
        pUp.shoulder -= 15; 
        pUp.elbow -= 10;
        
        // Grip lower than calibrated (2 deg) for magnet contact
        RobotPose pGrab = pDown;
        pGrab.shoulder += 2;
        
        moveToPose(pUp, speed);   // Approach Hover
        moveToPose(pGrab, speed); // Touch
        delay(2000);              // Pause 2s before Grip
        magnetOn();
        delay(1000);              // Pause 1s after Magnet On
        moveToPose(pUp, speed);   // Lift
    }

    // Core Primitive: Go Up -> Down -> Wait -> Magnet Off -> Up
    void performPlace(RobotPose pDown, int delayMs, int speed = 30) {
        RobotPose pUp = pDown;
        pUp.shoulder -= 15;
        pUp.elbow -= 10;
        
        // Release point (closer to board)
        RobotPose pPlace = pDown;
        pPlace.shoulder -= 1; 
        
        // "Nudge" point (slightly higher than release point) for settling
        RobotPose pNudge = pPlace;
        pNudge.shoulder -= 2; // Up another 2 degrees

        moveToPose(pUp, speed);     // Approach Hover
        moveToPose(pPlace, speed);  // Lower
        
        // "One step up, one step down" maneuver
        delay(500);
        moveToPose(pNudge, speed);  // Slowly Up
        delay(500);
        moveToPose(pPlace, speed);  // Slowly Down again
        
        if (delayMs > 0) delay(delayMs);
        magnetOff();
        delay(400);                 // Release
        moveToPose(pUp, speed);     // Lift
    }
    
    // Convenience for Board Squares
    void pickupPiece(int file, int rank) {
        performPickup(getSquarePose(file, rank));
    }

    void placePiece(int file, int rank, int delayMs) {
        performPlace(getSquarePose(file, rank), delayMs);
    }
    
    // Drop current hold (e.g. into Graveyard if just moved there)
    // Just releases magnet.
    void dropPiece() {
        magnetOff();
        delay(400);
    }
    
    // --- ROBOT GAME LOGIC ---
    
    int graveyardNextFree = 0;
    
    void performGameMove(int f1, int r1, int f2, int r2, bool capture) {
        // 1. Handle Capture
        if (capture) {
            // Pick Victim
            performPickup(getSquarePose(f2, r2));
            
            // Move to Graveyard
            if (graveyardNextFree < 14) {
                 RobotPose pGrave = getGraveyardPose(graveyardNextFree);
                 performPlace(pGrave, 0); // No delay needed for dropping dead pieces
                 graveyardNextFree++;
            } else {
                 dropPiece(); // Just drop it
            }
        }
        
        // 2. Perform Main Move
        performPickup(getSquarePose(f1, r1));
        
        // 3. Place with Delay (5s)
        performPlace(getSquarePose(f2, r2), 5000);
        
        // 4. Back to game position
        returnToGame();
    }
    
    void setDirect(int id, int angle) {
         if (id==1) base.write(angle);
         if (id==2) shoulder.write(angle);
         if (id==3) elbow.write(angle);
         if (id==4) wrist.write(angle);
    }
    
    void setCalibration(String s, int a, int b, int c, int d, int e) {}
};

extern RobotController robot;

#endif