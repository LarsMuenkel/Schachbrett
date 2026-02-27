import serial
import time
import sys
import random

# CONFIG
BAUD = 9600
MOVES_TO_PLAY = 500

def find_arduino():
    ports = ['COM3', 'COM4', 'COM5', '/dev/ttyACM0', '/dev/ttyUSB0', '/dev/ttyACM1', '/dev/ttyUSB1']
    for p in ports:
        try:
            s = serial.Serial(p, BAUD, timeout=0.1)
            time.sleep(1.5) # Wait for Arduino reset
            return s, p
        except:
            pass
    return None, None

def stress_test():
    ser, port = find_arduino()
    if not ser:
        print("ERROR: Arduino not found. Please check connection.")
        return

    print(f"Connected to {port}. Starting Stress Test ({MOVES_TO_PLAY} moves).")
    ser.flushInput()
    
    # 1. HANDSHAKE
    print("[1/4] Handshake: Sending 'heyArduinoChooseMode'...")
    ser.write(b"heyArduinoChooseMode\n")
    
    # Wait for response
    connected = False
    timeout = time.time() + 10
    while time.time() < timeout:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if "heypiG" in line or "Stockfish" in line:
                print(f"   -> Arduino Responded: {line}")
                connected = True
                break
            elif line:
                print(f"   (Init): {line}")
        except: pass
        time.sleep(0.1)
        
    if not connected:
        print("   -> No handshake response. Arduino might be in a different state. Resetting anyway.")
        # Try proceeding
    
    # 2. SETUP PHASE
    print("\n[2/4] ARDUINO SETUP PHASE")
    print(" >>> PLEASE USE ARDUINO BUTTONS TO SELECT DIFFICULTY <<<")
    
    # Wait for settings
    settings_received = 0
    while settings_received < 2:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if not line: continue
            
            if "heypi-" in line:
                print(f"   -> Setting received: {line}")
                settings_received += 1
            else:
                print(f"   (Setup): {line}")
        except: pass

    print("\n[3/4] GAME START")
    print("Arduino is ready. Please make your move on the board.")
    
    move_count = 0
    
    # Simple Toggle Strategy to make physical testing easier
    # If Human plays move X, we reply with Y.
    # User must act as both sides.
    # To keep it simple: 
    #   Human moves A1->A2. 
    #   PC moves B1->B2. 
    #   Human moves A2->A1.
    #   PC moves B2->B1.
    
    while move_count < MOVES_TO_PLAY:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if not line: continue
            
            # PARSE PROTOCOL
            prefix = "heypi"
            if prefix in line:
                idx = line.find(prefix)
                content = line[idx+len(prefix):] # Type + Msg
                msg_type = content[0]
                msg_data = content[1:]
                
                # HUMAN MOVED
                if msg_type == 'M': 
                    moves_msg = msg_data
                    move_count += 1
                    print(f"\n[MOVE {move_count}/{MOVES_TO_PLAY}] Human Played: {moves_msg}")
                    
                    # Log Check
                    print("   -> (Pi Sim) Sending: heyArduinoReady (No Error)")
                    # The real code checks for error. If no error sent, it proceeds.
                    time.sleep(0.5)
                    
                    # CALCULATE FAKE PC MOVE
                    # We create a valid move request that forces you to exercise a pin
                    # Let's say we toggle A7->A6 then A6->A7
                    pc_move = "a7a6"
                    if "a7a6" in moves_msg: pc_move = "a6a7" # Reply to user?
                    # Arduino wait logic:
                    # User moves. Arduino sends 'M'. User waits.
                    # Pi sends 'm...'. Arduino waits for user to execute PC move.
                    
                    # We just send a dummy move "h7h6".
                    # You have to physically move H7 -> H6.
                    # Then on next turn H6 -> H7.
                    if move_count % 2 == 1:
                        pc_move = "h7h6"
                    else:
                        pc_move = "h6h7"
                        
                    msg = f"heyArduinom{pc_move}-hint"
                    print(f"   -> (Pi Sim) PC Sending Move: {pc_move}")
                    ser.write((msg + "\n").encode('ascii'))
                    
                # PHYSICAL EXECUTION DONE
                elif msg_type == 'x':
                    print(f"   -> Physical Execution Confirmed. Round Done.")
                    if move_count >= MOVES_TO_PLAY:
                        print("\nSUCCESS: 500 Moves reached without crash!")
                        break

                # INTERMEDIATE / DEBUG
                elif msg_type == 'i':
                    print(f"   (Info) Lift: {msg_data}")
                elif msg_type == 'p':
                    print(f"   (Info) Progress: {msg_data}")
                elif msg_type == 'w':
                    print(f"   !!! WARNING: WRONG PIECE ({msg_data}) !!!")
                    
            else:
                # Normal debug prints from Arduino
                if "DEBUG" in line or "EXECUTE" in line or "[OK]" in line:
                    print(f"ARD: {line}")
                    
        except KeyboardInterrupt:
            print("\nTest Aborted by User.")
            break
        except Exception as e:
            print(e)

if __name__ == "__main__":
    stress_test()
