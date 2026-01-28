#include "Globals.h"
#include "Board.h"
#include "Input.h"
#include "Comms.h"

// --- Global Objects ---
Board board;
InputController input;
Comms comms;

// --- Global State ---
volatile bool restartRequest = false;

// Internal Board Representation for logging
// 0 = Empty, 1 = Piece
int currentBoard[8][8];

// --- ISR ---
void restartISR() {
    restartRequest = true;
}

// --- Helper Functions ---

void resetInternalBoard() {
    // Rows 0-1 (White Pieces? Black?) - Original logic
    // Rows 0-1 (Indices 0,1) -> Ranks 8,7 (Black)
    // Rows 6-7 (Indices 6,7) -> Ranks 2,1 (White)
    // Original setup: 0,1=1; 2-5=0; 6,7=1.
    for(int i=0; i<8; i++) {
        for(int j=0; j<8; j++) {
            if(i < 2 || i > 5) currentBoard[i][j] = 1;
            else currentBoard[i][j] = 0;
        }
    }
}

void printBoard() {
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            Serial.print(currentBoard[i][j]);
            Serial.print(",");
        }
        Serial.println();
    }
}

void updateInternalBoard(String move) {
    // Move format: "e2e4"
    if (move.length() < 4) return;

    int colFrom = move[0] - 'a';
    int rowFrom = 7 - (move[1] - '1'); // '1'->7, '8'->0
    
    int colTo = move[2] - 'a';
    int rowTo = 7 - (move[3] - '1');

    if (colFrom >= 0 && colFrom < 8 && rowFrom >= 0 && rowFrom < 8)
        currentBoard[rowFrom][colFrom] = 0;
        
    if (colTo >= 0 && colTo < 8 && rowTo >= 0 && rowTo < 8)
        currentBoard[rowTo][colTo] = 1;
}

void waitForPhysicalMove(String move) {
    String from = move.substring(0, 2);
    String to = move.substring(2, 4);
    from.toUpperCase(); // Normalize to match Board.h definitions (A1, B2...)
    to.toUpperCase();
    
    Serial.println(">>> EXECUTE PC MOVE: " + from + " -> " + to + " <<<");
    
    bool fromDone = false;
    bool toDone = false;

    // Special handling: If 'from' square starts empty? (Should not happen if synced)
    // We strictly wait for Lift(from) and Place(to).
    
    while (!restartRequest && (!fromDone || !toDone)) {
        String change = board.getChange();
        
        if (change.startsWith("-")) {
            String sq = change.substring(1);
            if (sq == from) {
                fromDone = true;
                Serial.println(" [OK] Source lifted: " + sq);
                comms.send("L" + sq, 'p'); 
            } 
            else if (sq == to) {
                 Serial.println(" (Info) Capture lifted: " + sq);
            }
            else {
                 // ERROR: Wrong piece lifted
                 Serial.println("WRONG PIECE: " + sq + ". Expected " + from);
                 comms.send(sq, 'w'); // w = Wrong Warning
                 
                 // Wait until user puts it back!
                 while (!restartRequest) {
                     String fix = board.getChange();
                     if (fix == "+" + sq) {
                         Serial.println("Wrong piece restored.");
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
                 Serial.println(" [OK] Dest placed: " + sq);
                 comms.send("P" + sq, 'p'); // P = Placed Correctly
             } else {
                 Serial.println(" (Info) Piece placed: " + sq);
             }
        }
        
        delay(10);
    }
    
    // Small delay to ensure user hand is away / settle
    delay(500); 
    Serial.println("PC Move Physical Execution Complete.");
    
    // Notify Pi that physical move is done (so it can update display to "Your Go")
    // 'x' = eXecuted
    comms.send("done", 'x');
}

void setupGameSequence() {
    // Ensure button is not "stuck" from boot
    input.waitForRelease();

    // --- Step 1: Select Quadrant (Category) ---
    // 1=Easy, 2=Medium, 3=Hard, 4=Extreme
    Serial.println("Select Category (1:Easy, 2:Med, 3:Hard, 4:Extr)");
    int category = input.selectOption(1, 4, 'Q');
    input.waitForRelease();

    // --- Step 2: Select Sub-Level ---
    // --- Step 2: Select Sub-Level ---
    // 1 to 5 (Total 20 levels: 4 categories * 5 levels)
    Serial.println("Select Level (1-5)");
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

    Serial.print("Selected Index: "); Serial.println(totalIndex);
    Serial.print("Skill: "); Serial.println(stockfishSkill);
    Serial.print("Time: "); Serial.println(moveTime);

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
                Serial.print("From (Lift): "); Serial.println(moveFrom);
                
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
                Serial.print("To (Place): "); Serial.println(moveTo);
                break;
            }
            else if (change.startsWith("-")) {
                Serial.println("Ignored Lift during move (Capture removal?)");
            }
            delay(10);
        }

        if (restartRequest) return "";

        // Check validity
        if (moveFrom == moveTo) {
            Serial.println("Ignored move (placed back on same square). Please move again.");
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

    // Init Logic
    resetInternalBoard();
}

void loop() {
    // Always check restart at top of loop
    if (restartRequest) {
        Serial.println("Starting a new game....");
        comms.send("", 'n');
        resetInternalBoard();
        restartRequest = false;
        // Proceed to setup
    }

    // 1. Wait for Pi to be ready
    // Note: Original code calls waitForPiToStart() only once in setup().
    // But then loop() runs continuously.
    // If restart happens, we might need to re-handshake?
    // Original restart() just resets board and sends 'n'. It does NOT call waitForPiToStart again.
    // So we assume Pi handles 'n' and is ready.
    // However, on startup we MUST wait.
    static bool firstRun = true;
    if (firstRun) {
        comms.waitForPiStart();
        if(restartRequest) return; // loop again to handle restart
        setupGameSequence();
        firstRun = false;
    }

    // 2. Human Turn
    String humanMove = getHumanMove();
    if (restartRequest) return; 

    comms.send(humanMove, 'M');
    printBoard();

    // 3. Check Legality (Pi validation)
    bool legal = comms.checkPiForError();
    if (restartRequest) return;

    if (!legal) {
        Serial.println("Move discarded, please return the pieces and try another move");
        printBoard();
        // Loop restart (Human tries again)
        return;
    }

    Serial.println("Move is legal");
    updateInternalBoard(humanMove);
    printBoard();

    // 4. Pi Turn
    String piMoveFull = comms.receiveMove();
    if (restartRequest) return;
    if (piMoveFull == "error") {
       return;
    }

    String piMove = piMoveFull.substring(0, 4); 
    String suggested = "";
    if (piMoveFull.length() > 5) suggested = piMoveFull.substring(5);

    Serial.println("Suggested next move = " + suggested);
    Serial.println(piMoveFull);

    // WAIT FOR USER TO EXECUTE PC MOVE ON BOARD
    waitForPhysicalMove(piMove);
    
    updateInternalBoard(piMoveFull); 
    printBoard();
}
