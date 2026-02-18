#ifndef COMMS_H
#define COMMS_H

#include <Arduino.h>

class Comms {
public:
    void begin() {
        Serial.begin(9600);
        // Ensure default timeout is reasonable (1 sec default)
    }

    void send(String message, char type) {
        // format: heypi<Type><Message>
        // Use separate prints to save memory? No, string concat is okay for short strings.
        Serial.print("heypi");
        Serial.print(type);
        Serial.println(message);
    }

    // Check for Start Command (Non-blocking check)
    bool checkPiStart() {
        if (Serial.available() > 0) {
            String data = Serial.peek() != -1 ? Serial.readStringUntil('\n') : ""; 
            // Note: readStringUntil helps, but peeking might be cleaner to avoid consuming other commands? 
            // Actually, if we consume "set...", we fail. 
            // Let's rely on main loop to pass data here? Or simpler:
            // Just return TRUE if we see the start command, else consume nothing if possible?
            // BUT: Serial buffer is stream. 
            
            // Allow Main Loop to read line, and pass it here?
            // Refactoring: "processInput(String data)"
            
            if (data.startsWith("heyArduinoChooseMode")) {
                Serial.println(data);
                send("Stockfish", 'G');
                Serial.println("Pi is going to start a game with Stockfish.");
                delay(500); 
                return true;
            }
            // If it's NOT a start command, is it a CAL command? 
            // We can't put back data.
            // Problem: If I read it here, handleSerial won't see it.
        }
        return false;
    }

    // Returns false if error received, true if OK.
    bool checkPiForError() {
        Serial.println("Function:checkPiForError... Waiting for response");
        int checks = 0;
        // Check for 3 seconds (30 * 100ms)
        while (checks < 30 && !restartRequest) {
            if (Serial.available() > 0) {
                String data = Serial.readStringUntil('\n');
                Serial.println(data);
                if (data.startsWith("heyArduinoerror")) {
                    Serial.println("Error received from Pi.");
                    return false;
                }
            } else {
                delay(100);
                checks++;
            }
        }
        return true;
    }

    // Blocks until move received
    String receiveMove() {
        Serial.println("Function:receiveMoveFromPi...");
        while (!restartRequest) {
            if (Serial.available() > 0) {
                String data = Serial.readStringUntil('\n');
                Serial.println(data);
                if (data.startsWith("heyArduinom")) {
                    // format: heyArduinom<Move>
                    return data.substring(11); // "heyArduinom" is 11 chars
                } else if (data.startsWith("heyArduinoerror")) {
                    return "error";
                }
            }
        }
        return ""; // Interrupted
    }
};

#endif
