#include "Globals.h"
#include "Board.h"
#include "Input.h"
#include "Comms.h"
#include <Servo.h>

// --- Global Objects ---
Board board;
InputController input;
Comms comms;

#include "Robot.h"
#include "Calibration.h"

RobotController robot; // Instance
Calibrator calibrator; // Instance

// --- Global State ---
volatile bool restartRequest = false;
bool useRobot = true; // Default to Robot
bool pendingStart = false;

// --- ISR ---
void restartISR() {
    restartRequest = true;
}

// --- Helper: Serial Command Parser ---
void handleSerial() {
    if (Serial.available() > 0) {
        String data = Serial.readStringUntil('\n');
        data.trim();
        
        // Handshake
        if (data == "heyArduinoChooseMode") {
             Serial.println(F("Arduino Ready.")); // Optional ack?
             pendingStart = true;
             return;
        }
        
        // Robot Toggle
        if (data.startsWith("robot ")) {
             int val = data.substring(6).toInt();
             useRobot = (val == 1);
             Serial.print(F("Robot Mode: ")); Serial.println(useRobot ? "ON" : "OFF");
        }
        // Calibration Commands: "set <id> <angle>"
        else if (data.startsWith("set ")) {
            int firstSpace = data.indexOf(' ');
            int secondSpace = data.indexOf(' ', firstSpace + 1);
            
            int id = data.substring(firstSpace + 1, secondSpace).toInt();
            int angle = data.substring(secondSpace + 1).toInt();
            
            Serial.print(F("Manual Servo ")); Serial.print(id);
            Serial.print(F(" -> ")); Serial.println(angle);
            robot.setDirect(id, angle);
        }
        // Magnet: "magnet 1" / "magnet 0"
        else if (data.startsWith("magnet ")) {
             int val = data.substring(7).toInt();
             if (val) robot.magnetOn();
             else robot.magnetOff();
             Serial.println(val ? F("Magnet ON") : F("Magnet OFF"));
        }
        // Execute Move: "exec E2 E4"
        else if (data.startsWith("exec ")) {
             String fS = data.substring(5,6);
             String rS = data.substring(6,7);
             String fD = data.substring(8,9); // "exec E2 E4" -> char 8 is E
             String rD = data.substring(9,10);
             
             // Simplistic Check
             if (data.length() >= 10) {
                 int f1 = fS.charAt(0) - 'A';
                 int r1 = rS.charAt(1) - '1'; // Wait, substring(6,7) is charAt(0) of that string
                 r1 = rS.toInt() - 1; // "2" -> 1
                 
                 int f2 = fD.charAt(0) - 'A';
                 int r2 = rD.toInt() - 1;
                 
                 Serial.print(F("Exec Move: ")); Serial.print(fS); Serial.print(rS);
                 Serial.print(F(" -> ")); Serial.print(fD); Serial.println(rD);
                 
                 // Fix parsing carefully: E2 is 'E' at 0, '2' at 1 in sub?
                 // data: "exec E2 E4"
                 // 0123456789
                 // char 5='E', 6='2', 7=' ', 8='E', 9='4'
                 f1 = data.charAt(5) - 'A';
                 r1 = data.charAt(6) - '1';
                 f2 = data.charAt(8) - 'A';
                 r2 = data.charAt(9) - '1';
                 
                 robot.executeMove(f1, r1, f2, r2);
                 Serial.println(F("Done"));
             }
        }
        // Save Calibration: "save A1 <b,s,e,w,g>" (Simulated for now, actually just prints)
        else if (data.startsWith("cal ")) {
             Serial.println(F("Use 'set <id> <angle>' to find values."));
             Serial.println(F("Then update Robot.h manually with the found values."));
             // Example: "set 1 45" (Base to 45)
        }
        // Test move: "move E2"
        else if (data.startsWith("move ")) {
             String sq = data.substring(5);
             Serial.print(F("Test Move to ")); Serial.println(sq);
             // Parse sq (A1..H8)
             char fileChar = sq.charAt(0);
             char rankChar = sq.charAt(1);
             int file = fileChar - 'A';
             int rank = rankChar - '1';
             
             if (file >= 0 && file <= 7 && rank >= 0 && rank <= 7) {
                 RobotPose p = robot.getSquarePose(file, rank);
                 robot.moveToPose(p, 10);
                 Serial.println(F("Move Complete"));
             } else {
                 Serial.println(F("Invalid Square"));
             }
        }
        else if (data == "calibrate") {
             calibrator.runRoutine();
        }
    }
}

// --- Helper Functions ---

// --- Helper Functions ---

void executeRobotMove(String move) {
    String from = move.substring(0, 2);
    String to = move.substring(2, 4);
    from.toUpperCase(); 
    to.toUpperCase();
    
    Serial.print(F(">>> ROBOT EXECUTE: "));
    Serial.print(from);
    Serial.print(F(" -> "));
    Serial.println(to);
    
    int f1 = from.charAt(0) - 'A';
    int r1 = from.charAt(1) - '1';
    int f2 = to.charAt(0) - 'A';
    int r2 = to.charAt(1) - '1';
    
    // Check Capture
    bool isCapture = false;
    if (board.isOccupied(f2, r2)) {
        isCapture = true;
        Serial.print(F("Capture detected at ")); Serial.println(to);
    }
    
    // Delegate to Robot Class
    robot.performGameMove(f1, r1, f2, r2, isCapture);
    
    // Inform Pi (Robot logic handles physical move & delay)
    comms.send("done", 'x');
}

void waitForPhysicalMove(String move) {
    String from = move.substring(0, 2);
    String to = move.substring(2, 4);
    from.toUpperCase(); 
    to.toUpperCase();
    
    Serial.print(F(">>> EXECUTE PC MOVE: "));
    Serial.print(from);
    Serial.print(F(" -> "));
    Serial.println(to);
    
    bool fromDone = false;
    bool toDone = false;

    while (!restartRequest && (!fromDone || !toDone)) {
        String change = board.getChange();
        
        if (change.startsWith("-")) {
            String sq = change.substring(1);
            if (sq == from) {
                fromDone = true;
                Serial.print(F(" [OK] Source lifted: ")); Serial.println(sq);
                comms.send("L" + sq, 'p'); 
            } 
            else if (sq == to) {
                 Serial.print(F(" (Info) Capture lifted: ")); Serial.println(sq);
            }
            else {
                 Serial.print(F("WRONG PIECE: ")); Serial.println(sq); 
                 comms.send(sq, 'w'); 
                 // Wait restoral
                 while (!restartRequest) {
                     String fix = board.getChange();
                     if (fix == "+" + sq) {
                         comms.send("fixed", 'w'); 
                         break;
                     }
                     delay(10);
                 }
            }
        }
        else if (change.startsWith("+")) {
             String sq = change.substring(1);
             if (sq == to) {
                 toDone = true;
                 Serial.print(F(" [OK] Dest placed: ")); Serial.println(sq);
                 comms.send("P" + sq, 'p'); 
             }
        }
        delay(10);
    }
    
    delay(500); 
    Serial.println(F("PC Move Physical Execution Complete."));
    comms.send("done", 'x');
}

void setupGameSequence() {
    // Ensure button is not "stuck" from boot
    input.waitForRelease();

    // --- Step 1: Select Quadrant (Category) ---
    // 1=Easy, 2=Medium, 3=Hard, 4=Extreme
    Serial.println(F("Select Category (1:Easy, 2:Med, 3:Hard, 4:Extr)"));
    int category = input.selectOption(1, 4, 'Q');
    input.waitForRelease();

    // --- Step 2: Select Sub-Level ---
    // 1 to 5 (Total 20 levels: 4 categories * 5 levels)
    Serial.println(F("Select Level (1-5)"));
    int subLevel = input.selectOption(1, 5, 'L');
    input.waitForRelease();

    // --- Calculation ---
    // Map 4x5 (20 levels) to settings
    // Total index 0 to 19
    int totalIndex = ((category - 1) * 5) + (subLevel - 1);
    
    // Map to Stockfish Skill (0-20)
    int stockfishSkill = map(totalIndex, 0, 19, 0, 20);
    
    // Constant Move Time (1000ms)
    long moveTime = 1000;

    Serial.print(F("Selected Index: ")); Serial.println(totalIndex);
    Serial.print(F("Skill: ")); Serial.println(stockfishSkill);
    Serial.print(F("Time: ")); Serial.println(moveTime);

    // --- Send to Pi ---
    // 1. Send Difficulty
    String diffStr = String(stockfishSkill);
    if (diffStr.length() < 2) diffStr = "0" + diffStr;
    comms.send(diffStr, '-');
    
    // Pi expects a delay or handshake? Original just sent two messages.
    // However, Pi code:
    // 1. Waits for Level.
    // 2. Prints "Requested skill..."
    // 3. Waits for Time.
    delay(500); 

    // 2. Send Timeout
    comms.send(String(moveTime), '-');
}

// Retrieves human move by polling board
String getHumanMove() {
    while (!restartRequest) {
        String moveFrom = "";
        String moveTo = "";
        
        // Wait for FROM square (MUST BE A LIFT '-')
        while (!restartRequest) {
            String change = board.getChange();
            if (change.startsWith("-")) {
                moveFrom = change.substring(1);
                Serial.print(F("From (Lift): ")); Serial.println(moveFrom);
                
                // LIVE UPDATE: Send "Lift" info to Pi
                // Type 'i' for Info/Intermediate
                comms.send(moveFrom, 'i');
                
                break;
            } 
            else if (change.startsWith("+")) {
               // Ignore unintentional placing or artifacts
            }
            delay(10);
        }
        
        if (restartRequest) return "";

        // Wait for TO square (MUST BE A PLACE '+')
        while (!restartRequest) {
            String change = board.getChange();
            if (change.startsWith("+")) {
                moveTo = change.substring(1);
                Serial.print(F("To (Place): ")); Serial.println(moveTo);
                break;
            }
            else if (change.startsWith("-")) {
                Serial.println(F("Ignored Lift during move (Capture removal?)"));
            }
            delay(10);
        }

        if (restartRequest) return "";

        // Check validity
        if (moveFrom == moveTo) {
            Serial.println(F("Ignored move (placed back on same square). Please move again."));
        } else {
            return moveFrom + moveTo;
        }
    }
    return "";
}

void setup() {
    // Init Hardware
    board.begin();
    input.begin();
    comms.begin();
    
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Interrupt
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_RESTART), restartISR, FALLING);

    // --- Robot Initialization ---
    // Startet in Parkposition und fährt dann in Gameposition
    // PIN DEFINITIONS from Globals.h
    robot.begin(PIN_ROBO_BASE, PIN_ROBO_SHOULDER, PIN_ROBO_ELBOW, PIN_ROBO_WRIST, PIN_ROBO_GRIPPER);
    
    delay(1000); 
    robot.wakeUp();
}


// --- Loop ---
void loop() {
    // Always check for debug commands
    handleSerial();

    // Check restart
    if (restartRequest) {
        Serial.println(F("Starting a new game...."));
        comms.send("", 'n');
        restartRequest = false;
        robot.wakeUp(); // Reset to game pos
    }

    // 1. Wait for Pi to be ready (Non-blocking Loop)
    static bool gameStarted = false;
    if (!gameStarted) {
        if (pendingStart) {
            gameStarted = true;
            pendingStart = false; 
            setupGameSequence();
        } else {
             // While waiting, we can handle calibration commands!
             // handleSerial is already called at top of loop.
             delay(50); 
             return; // Loop again
        }
    }

    // 2. Human Turn
    String humanMove = getHumanMove();
    if (restartRequest) return; 
    
    // We send move to Pi. Pi validates it.
    comms.send(humanMove, 'M');

    // 3. Check Legality (Pi validation)
    bool legal = comms.checkPiForError();
    if (restartRequest) return;

    if (!legal) {
        Serial.println(F("Move discarded, please return the pieces and try another move"));
        // Loop restart (Human tries again)
        return;
    }

    Serial.println(F("Move is legal"));

    // 4. Pi Turn
    String piMoveFull = comms.receiveMove();
    if (restartRequest) return;
    if (piMoveFull == "error") {
       return;
    }

    String piMove = piMoveFull.substring(0, 4); 
    String suggested = "";
    if (piMoveFull.length() > 5) suggested = piMoveFull.substring(5);

    Serial.print(F("Suggested next move = ")); Serial.println(suggested);
    Serial.println(piMoveFull);

    // WAIT FOR USER TO EXECUTE PC MOVE ON BOARD
    if (useRobot) {
        executeRobotMove(piMove);
    } else {
        waitForPhysicalMove(piMove);
    }
}
