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
    {65, 134, 50, 71, 0},  // A1
    {61, 112, 84, 59, 0}, // A2
    {57, 99, 85, 86, 0},  // A3
    {52, 96, 73, 117, 0}, // A4
    {48, 82, 85, 122, 0}, // A5
    {41, 71, 89, 134, 0}, // A6
    {32, 65, 89, 145, 0}, // A7
    {17, 62, 88, 153, 0}  // A8
  },
  { // File B
    {73, 128, 57, 76, 0}, // B1
    {68, 114, 64, 94, 0}, // B2
    {64, 103, 68, 113, 0},  // B3
    {60, 92, 73, 127, 0}, // B4
    {57, 75, 86, 132, 0}, // B5
    {50, 66, 89, 144, 0}, // B6
    {40, 50, 98, 151, 0}, // B7
    {24, 21, 116, 153, 0} // B8
  },
  { // File C
    {80, 125, 54, 87, 0}, // C1
    {75, 104, 79, 86, 0}, // C2
    {73, 84, 95, 94, 0},  // C3
    {69, 72, 97, 113, 0}, // C4
    {68, 63, 97, 132, 0}, // C5
    {62, 48, 103, 143, 0},  // C6
    {54, 44, 98, 162, 0}, // C7
    {38, 42, 92, 175, 0}  // C8
  },
  { // File D
    {87, 128, 50, 94, 0}, // D1
    {82, 99, 87, 81, 0},  // D2
    {84, 90, 82, 109, 0}, // D3
    {82, 78, 86, 126, 0}, // D4
    {80, 67, 90, 141, 0}, // D5
    {77, 57, 90, 156, 0}, // D6
    {71, 42, 96, 167, 0}, // D7
    {59, 24, 99, 179, 0}  // D8
  },
  { // File E
    {96, 125, 50, 93, 0}, // E1
    {94, 97, 89, 80, 0},  // E2
    {94, 89, 82, 113, 0}, // E3
    {95, 81, 80, 132, 0}, // E4
    {94, 67, 88, 143, 0}, // E5
    {95, 51, 95, 155, 0}, // E6
    {96, 43, 94, 170, 0}, // E7
    {97, 20, 100, 180, 0} // E8
  },
  { // File F
    {104, 127, 50, 90, 0},  // F1
    {103, 112, 61, 107, 0}, // F2
    {107, 88, 83, 108, 0},  // F3
    {108, 79, 83, 127, 0},  // F4
    {108, 73, 83, 143, 0},  // F5
    {112, 53, 95, 151, 0},  // F6
    {119, 45, 94, 166, 0},  // F7
    {130, 38, 91, 179, 0} // F8
  },
  { // File G
    {112, 124, 65, 67, 0},  // G1
    {112, 107, 75, 87, 0},  // G2
    {116, 93, 81, 104, 0},  // G3
    {118, 86, 77, 127, 0},  // G4
    {120, 70, 89, 135, 0},  // G5
    {127, 58, 95, 144, 0},  // G6
    {134, 52, 93, 158, 0},  // G7
    {146, 40, 96, 168, 0} // G8
  },
  { // File H
    {116, 131, 53, 75, 0},  // H1
    {121, 111, 74, 76, 0},  // H2
    {123, 98, 80, 97, 0}, // H3
    {127, 88, 80, 115, 0},  // H4
    {129, 70, 97, 118, 0},  // H5
    {136, 52, 108, 125, 0}, // H6
    {145, 50, 101, 143, 0}, // H7
    {158, 60, 86, 161, 0} // H8
  }
};



// --- GRAVEYARD POSITIONS (Captured Pieces) ---
const RobotPose GRAVEYARD_POSITIONS[14] PROGMEM = {
    {180, 21, 99, 180, 0},     {162, 26, 96, 180, 0}, 
    {180, 51, 87, 177, 0},     {169, 51, 87, 177, 0}, 
    {180, 51, 87, 173, 0},     {170, 51, 87, 173, 0}, 
    {180, 61, 87, 163, 0},     {170, 61, 87, 163, 0}, 
    {0, 21, 99, 180, 0},     {18, 26, 96, 180, 0}, 
    {0, 51, 87, 177, 0},     {11, 51, 87, 177, 0}, 
    {0, 51, 87, 173, 0},     {10, 51, 87, 173, 0}
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

        parkPos = {90, 130, 45, 90, 0}; 
        park();
    }
    
    // --- Magnet Control ---
    void magnetOn() {
        digitalWrite(PIN_MAGNET_CTRL, HIGH);
    }
    
    void magnetOff() {
        digitalWrite(PIN_MAGNET_CTRL, LOW);
    }

    void wakeUp() {
        // Move to Hover Position over Center (D4)
        RobotPose ready = getSquarePose(3, 3); 
        moveToPose(ready, 20);
    }
    
    void park() {
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
    void performPickup(RobotPose pDown, int speed = 50) {
        RobotPose pUp = pDown;
        // Lift slightly (Shoulder -15, Elbow -10 seems okay as heuristic for "Up")
        pUp.shoulder -= 15; 
        pUp.elbow -= 10;
        
        // Grip slightly higher than calibrated (1 deg ~ 1-2mm) to avoid pressing
        RobotPose pGrab = pDown;
        pGrab.shoulder -= 1;
        
        moveToPose(pUp, speed);   // Approach Hover
        moveToPose(pGrab, speed); // Touch
        delay(5000);              // Pause 5s before Grip
        magnetOn();
        delay(1000);              // Pause 1s after Magnet On
        moveToPose(pUp, speed);   // Lift
    }

    // Core Primitive: Go Up -> Down -> Wait -> Magnet Off -> Up
    void performPlace(RobotPose pDown, int delayMs, int speed = 50) {
        RobotPose pUp = pDown;
        pUp.shoulder -= 15;
        pUp.elbow -= 10;
        
        // Release point (approx 3mm above board)
        RobotPose pPlace = pDown;
        pPlace.shoulder -= 2; 
        
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
        
        // 4. Park
        park();
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
