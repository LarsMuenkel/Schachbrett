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
bool isFirstTurn = true; // Added missing global flag
bool isFirstTurnForBlack = false;

// --- ISR ---
void restartISR() {
    restartRequest = true;
}

// --- Helper: Serial Command Parser ---
void handleSerial() {
    if (Serial.available() > 0) {
        String data = Serial.readStringUntil('\n');
        data.trim();
        
        if (data == "debug") {
            board.printState();
            return;
        }

        // Handshake Phase 1: Pi asks for mode, we respond
        if (data == "heyArduinoChooseMode") {
             comms.send("Stockfish", 'G');  // Tell Pi we're playing Stockfish
             return;  // Wait for ReadyStockfish before starting setup
        }
        
        // Handshake Phase 2: Pi is ready, start game setup
        if (data == "heyArduinoReadyStockfish") {
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
        // --- calibratePos park / game ---
        else if (data.startsWith("calibratePos ")) {
             String posName = data.substring(13);
             posName.trim();
             
             RobotPose* targetPos = nullptr;
             if (posName == "park") {
                  targetPos = &robot.parkPos;
                  Serial.println(F("Calibrating PARK position..."));
             } else if (posName == "game") {
                  targetPos = &robot.gamePos;
                  Serial.println(F("Calibrating GAME position..."));
             } else {
                  Serial.println(F("Unknown position. Use: calibratePos park / calibratePos game"));
             }
             
             if (targetPos != nullptr) {
                  robot.moveToPose(*targetPos, 20);
                  Serial.println(F("q/a=Base, w/s=Shoulder, e/d=Elbow, r/f=Wrist (+/- 1)"));
                  Serial.println(F("Type 'ok' to save."));
                  
                  RobotPose current = *targetPos;
                  while (true) {
                       if (Serial.available() > 0) {
                            String inp = Serial.readStringUntil('\n');
                            inp.trim();
                            if (inp == "ok") {
                                 *targetPos = current;
                                 Serial.println(F("Position saved!"));
                                 Serial.print(F("{"));
                                 Serial.print(current.base); Serial.print(F(", "));
                                 Serial.print(current.shoulder); Serial.print(F(", "));
                                 Serial.print(current.elbow); Serial.print(F(", "));
                                 Serial.print(current.wrist); Serial.println(F(", 0}"));
                                 break;
                            }
                            bool changed = false;
                            for (unsigned int i = 0; i < inp.length(); i++) {
                                 switch (inp.charAt(i)) {
                                      case 'q': current.base++; changed=true; break;
                                      case 'a': current.base--; changed=true; break;
                                      case 'w': current.shoulder++; changed=true; break;
                                      case 's': current.shoulder--; changed=true; break;
                                      case 'e': current.elbow++; changed=true; break;
                                      case 'd': current.elbow--; changed=true; break;
                                      case 'r': current.wrist++; changed=true; break;
                                      case 'f': current.wrist--; changed=true; break;
                                 }
                            }
                            if (changed) {
                                 current.base = constrain(current.base, 0, 180);
                                 current.shoulder = constrain(current.shoulder, 0, 180);
                                 current.elbow = constrain(current.elbow, 0, 180);
                                 current.wrist = constrain(current.wrist, 0, 180);
                                 robot.moveToPose(current, 50);
                                 Serial.print(F("[B:")); Serial.print(current.base);
                                 Serial.print(F(" S:")); Serial.print(current.shoulder);
                                 Serial.print(F(" E:")); Serial.print(current.elbow);
                                 Serial.print(F(" W:")); Serial.print(current.wrist);
                                 Serial.println(F("]"));
                            }
                       }
                  }
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
    
    // Execute with progress updates to Pi
    // 1. Handle Capture
    if (isCapture) {
        robot.performPickup(robot.getSquarePose(f2, r2));
        if (robot.graveyardNextFree < 14) {
            RobotPose pGrave = robot.getGraveyardPose(robot.graveyardNextFree);
            robot.performPlace(pGrave, 0);
            robot.graveyardNextFree++;
        } else {
            robot.dropPiece();
        }
    }
    
    // 2. Pickup source piece
    robot.performPickup(robot.getSquarePose(f1, r1));
    comms.send("L" + from, 'p');  // Tell Pi: lifted from source
    
    // 3. Place at destination
    robot.performPlace(robot.getSquarePose(f2, r2), 5000);
    comms.send("P" + to, 'p');  // Tell Pi: placed at destination
    
    // 4. Back to game position
    robot.returnToGame();
    
    // 5. Done
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
                     
                     // Allow user to override the warning with the DEL button
                     if (digitalRead(PIN_BTN_DEL) == LOW) {
                         delay(200);
                         Serial.println(F("Wrong piece warning overridden by user."));
                         comms.send("fixed", 'w'); // Tell Pi we are "fixed" so it proceeds
                         break;
                     }
                     
                     delay(10);
                     pollRestart();
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
        pollRestart();
    }
    
    delay(500); 
    Serial.println(F("PC Move Physical Execution Complete."));
    comms.send("done", 'x');
}

void setupGameSequence() {
    // Ensure button is not "stuck" from boot
    input.waitForRelease();

    // --- Step 0a: Robot On/Off ---
    Serial.println(F("Robot Mode: 1=An, 2=Aus"));
    int robotChoice = input.selectOption(1, 2, 'R');
    input.waitForRelease();
    useRobot = (robotChoice == 1);
    Serial.println(useRobot ? F("Robot: AN") : F("Robot: AUS"));

    // --- Step 0b: Select Color (White / Black) ---
    Serial.println(F("Select Color: 1=White, 2=Black"));
    int colorChoice = input.selectOption(1, 2, 'F');
    input.waitForRelease();
    
    // Send color to Pi (with colon separator for Pi handler)
    if (colorChoice == 1) {
        Serial.println(F("heypiC:w"));
        Serial.println(F("Playing as WHITE"));
        isFirstTurnForBlack = false;
    } else {
        Serial.println(F("heypiC:b"));
        Serial.println(F("Playing as BLACK"));
        isFirstTurnForBlack = true;
    }

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
    unsigned long lastPrintTime = millis();
    
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
            pollRestart();
        }
        
        if (restartRequest) return "";

        // Wait for TO square (MUST BE A PLACE '+')
        // DEL button = clear moveFrom and start over
        bool cleared = false;
        while (!restartRequest) {
            // Check DEL button to clear move
            if (digitalRead(PIN_BTN_DEL) == LOW) {
                delay(200);
                Serial.println(F("Move cleared! Waiting for new move..."));
                comms.send("Move Reset:Warte auf:neuen Zug", 'D');
                cleared = true;
                break;
            }
            
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
            pollRestart();
        }
        
        if (cleared) continue;  // Restart from FROM detection

        if (restartRequest) return "";

        // Check validity
        if (moveFrom == moveTo) {
            Serial.println(F("Ignored move (placed back on same square). Please move again."));
            // Clear the "From" display on the Pi
            comms.send("[  ]", 'i');
        } else {
            return moveFrom + moveTo;
        }
    }
    return "";
}

// Blocks until the physical board state matches the target bits
void waitForBoardRestoral(uint32_t target1, uint32_t target2, String moveFrom, String moveTo) {
    Serial.println(F("BLOCKING: Waiting for physical board restoral..."));
    comms.send("UNDO: " + moveTo + " -> " + moveFrom, 'D');

    long lastMsg = 0;
    while (!restartRequest) {
        // Continuous read to update bits
        String change = board.getChange();
        
        // Highlight logic during Undo
        if (change.startsWith("-")) {
            String sq = change.substring(1);
            if (sq == moveTo) {
                comms.send("UNDO: [" + moveTo + "] -> " + moveFrom, 'D'); // Highlight the square lifted from
            }
        } else if (change.startsWith("+")) {
            String sq = change.substring(1);
            if (sq == moveFrom) {
                comms.send("UNDO: " + moveTo + " -> [" + moveFrom + "]", 'D'); // Highlight the square placed on
            }
        }
        
        if (board.getState1() == target1 && board.getState2() == target2) {
            Serial.println(F("Board state matches original state. Correction complete."));
            // Briefly highlight the corrected square and clear screen back to waiting
            comms.send("UNDO: " + moveTo + " -> [" + moveFrom + "]", 'D'); 
            delay(500); 
            comms.send("Korrektur OK:Warte auf:neuen Zug", 'D');
            break;
        }

        // Override: Press DEL to accept the illegal board state
        if (digitalRead(PIN_BTN_DEL) == LOW) {
            delay(200);
            Serial.println(F("User pressed DEL. Overriding restoral..."));
            comms.send("Zug ueberschrieben!:Warte auf:neuen Zug", 'D');
            break;
        }
        
        // Periodic message
        if (millis() - lastMsg > 3000) {
            Serial.println(F("Waiting: Please move the piece back to its original square..."));
            lastMsg = millis();
        }
        delay(50);
        pollRestart();
    }
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


// --- Poll Restart Button (Pin 11 has no HW interrupt on Uno) ---
void pollRestart() {
    if (digitalRead(PIN_BTN_RESTART) == LOW) {
        delay(50);
        if (digitalRead(PIN_BTN_RESTART) == LOW) {
            restartRequest = true;
        }
    }
}

// --- Loop ---
void loop() {
    // Always check for debug commands
    handleSerial();

    static bool gameStarted = false;

    // Poll restart button
    pollRestart();

    // Check restart
    if (restartRequest) {
        restartRequest = false;
        
        // Ask for confirmation (Interactive Menu)
        input.waitForRelease();
        int choice = 1; // 1 = Ja, 2 = Nein
        comms.send("Neues Spiel?:> Ja:  Nein", 'D');
        
        bool menuActive = true;
        bool confirmed = false;
        
        while (menuActive) {
            bool changed = false;
            
            // Toggle selection with INPUT or DEL
            if (digitalRead(PIN_BTN_INPUT) == LOW || digitalRead(PIN_BTN_DEL) == LOW) {
                choice = (choice == 1) ? 2 : 1;
                changed = true;
                delay(200);
            }
            
            if (changed) {
                if (choice == 1) {
                    comms.send("Neues Spiel?:> Ja:  Nein", 'D');
                } else {
                    comms.send("Neues Spiel?:  Ja:> Nein", 'D');
                }
            }
            
            if (digitalRead(PIN_BTN_OK) == LOW) {
                confirmed = (choice == 1);
                menuActive = false;
                delay(300);
            }
            delay(50);
        }
        
        input.waitForRelease();
        
        if (confirmed) {
            Serial.println(F("Restarting game..."));
            robot.magnetOff();
            robot.park();
            comms.send("", 'n');  // Tell Pi: new game
            gameStarted = false;  // Reset so setupGameSequence runs again
            pendingStart = false;
            robot.graveyardNextFree = 0;
        } else {
            Serial.println(F("Restart cancelled."));
            comms.send("Spiel geht weiter:Warte auf:neuen Zug", 'D');
            delay(1000); // Give user 1s to see the cancel message
        }
    }

    // 1. Wait for Pi to be ready (Non-blocking Loop)
    if (!gameStarted) {
        if (pendingStart) {
            gameStarted = true;
            isFirstTurn = true;
            pendingStart = false; 
            setupGameSequence();
        } else {
             // While waiting, we can handle calibration commands!
             // handleSerial is already called at top of loop.
             delay(50); 
             return; // Loop again
        }
    }

    // 2. Human Turn (Only if it's not the very first turn when playing as Black)
    if (!isFirstTurnForBlack) {
        // Ensure any previously pending sensor changes (like from a robot move) are swallowed
        if (!isFirstTurn) {
            delay(500); // Let magnets/sensors settle
            board.resync(); // Snapshot current physical state
            unsigned long silenceStart = millis();
            // 1000ms silence for robot moves
            while (millis() - silenceStart < 1000 && !restartRequest) {
                if (board.getChange() != "") silenceStart = millis(); 
                delay(10);
                pollRestart();
            }
        } else {
            // For the first turn, we still need a brief silence period!
            delay(300); // Let board stop vibrating from OK button
            board.resync();
            unsigned long silenceStart = millis();
            // 400ms is perfectly enough to guarantee the board is physically still
            while (millis() - silenceStart < 400 && !restartRequest) {
                if (board.getChange() != "") silenceStart = millis(); 
                delay(10);
                pollRestart();
            }
            isFirstTurn = false;
        }

        uint32_t before1 = board.getState1();
        uint32_t before2 = board.getState2();

        String humanMove = getHumanMove();
        if (restartRequest) return; 
        
        // We send move to Pi. Pi validates it.
        comms.send(humanMove, 'M');

        // 3. Check Legality (Pi validation)
        bool legal = comms.checkPiForError();
        if (restartRequest) return;

        if (!legal) {
            Serial.println(F("Move discarded. Please return pieces, it was illegal."));
            String mf = humanMove.substring(0, 2);
            String mt = humanMove.substring(2, 4);
            waitForBoardRestoral(before1, before2, mf, mt);
            return;
        }

        Serial.println(F("Move is legal"));
    } else {
        isFirstTurnForBlack = false; // Reset flag so next loop allows human move
        Serial.println(F("Playing as Black: Skipping human turn to let PC move first."));
    }

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
        // Resync board state after robot physically moved pieces
        delay(500);
        board.resync();
    } else {
        waitForPhysicalMove(piMove);
    }
}
