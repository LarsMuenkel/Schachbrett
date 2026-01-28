# Interactive Chessboard - FINAL FIXED VERSION
from ChessBoard import ChessBoard
import subprocess, time, serial, sys

maxchess = ChessBoard()

if __name__ == '__main__':
    ports_to_try = [ '/dev/ttyACM0', '/dev/ttyUSB0', '/dev/ttyACM1', '/dev/ttyUSB1']
    ser = None
    for port in ports_to_try:
        try:
            print("Trying to connect to " + port + "...")
            ser = serial.Serial(port, 9600, timeout=1)
            print("Connected to " + port)
            break
        except:
            pass
            
    if ser is None:
        print("CRITICAL ERROR: Could not find Arduino.")
        sys.exit(1)
        
    ser.flush()

engine = subprocess.Popen(
    '/usr/games/stockfish',
    universal_newlines=True,
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE
    )

def get():
    engine.stdin.write('isready\n')
    engine.stdin.flush()
    print('\nengine:')
    while True :
        text = engine.stdout.readline().strip()
        if text == 'readyok':
            break
        if text !='':
            print('\t'+text)
        if text[0:8] == 'bestmove':
            return text

def sget():
    engine.stdin.write('isready\n')
    engine.stdin.flush()
    print('\nengine:')
    while True :
        text = engine.stdout.readline().strip()
        if text !='':
            print('\t'+text)
        if text[0:8] == 'bestmove':
            mtext=text
            return mtext

def put(command):
    print('\nyou:\n\t'+command)
    engine.stdin.write(command+'\n')
    engine.stdin.flush()

def getboard():
    while True:
        if ser.in_waiting > 0:
            try:
                btxt = ser.readline().decode('utf-8').rstrip().lower()
            except:
                continue
                
            if btxt.startswith('heypixshutdown'):
                shutdownPi()
                break
            if btxt.startswith('heypi'):
                btxt = btxt[len('heypi'):]
                
                # Check for Intermediate Signals immediately handled?
                # No, return raw so caller can decide
                print("Received from Board: " + btxt)
                return btxt
            else:
                continue
        time.sleep(0.01) # Faster polling

def sendtoboard(stxt):
    print("\n Sent to board: heyArduino" + stxt)
    stxt = bytes(str(stxt).encode('utf8'))
    time.sleep(2)
    ser.write(b"heyArduino" + stxt + "\n".encode('ascii'))

def sendToScreen(line1,line2,line3,size = '14'):
    screenScriptToRun = ["python3", "printToOLED.py", '-a '+ line1, '-b '+ line2, '-c '+ line3, '-s '+ size]
    subprocess.Popen(screenScriptToRun)

def newgame():
    sendToScreen ('NEW','GAME','','30')
    get ()
    put('uci')
    get ()
    put('setoption name Skill Level value ' +skill)
    get ()
    put('setoption name Hash value 128')
    get()
    put('ucinewgame')
    maxchess.resetBoard()
    maxchess.setPromotion(maxchess.QUEEN)
    fmove=""
    time.sleep(2)
    
    # FIX: Smaller Text
    sendToScreen ('White Start','Your Turn','','20')
    return fmove

def bmove(fmove):
    fmove=fmove
    brdmove = bmessage[1:5].lower() # bmessage is global or passed
    
    # 1. CHECK HUMAN MOVE
    if maxchess.addTextMove(brdmove) == False :
                        etxt = "error"+ str(maxchess.getReason())+brdmove
                        maxchess.printBoard()
                        
                        # IMPROVED ILLEGAL MOVE DISPLAY
                        # Shows: "A2->A4", "Illegal!", "Undo/Retry"
                        move_pretty = brdmove[0:2].upper() + "->" + brdmove[2:4].upper()
                        sendToScreen (move_pretty, 'Is Illegal!', 'Undo/Retry', '16')
                        
                        sendtoboard(etxt)
                        return fmove
    else:
        # Human Move Accepted
        maxchess.printBoard()
        sendToScreen (brdmove[0:2] + '->' + brdmove[2:4] ,'','Thinking...','20')
        fmove =fmove+" " +brdmove
        cmove = "position startpos moves"+fmove
        put(cmove)
        put("go movetime " +movetime)
        
        # 2. GET COMPUTER MOVE
        text = sget()
        smove = text[9:13] # e.g. e7e5
        hint = text[21:25]
        
        if maxchess.addTextMove(smove) != True :
                        stxt = "e"+ str(maxchess.getReason())+smove
                        maxchess.printBoard()
                        sendtoboard(stxt)
        else:
                        temp=fmove
                        fmove =temp+" " +smove
                        stx = smove+hint
                        maxchess.printBoard()
                        print ("computer move: " +smove)
                        
                        smove_pretty_from = smove[0:2].upper()
                        smove_pretty_to = smove[2:4].upper()
                        smove_full = smove_pretty_from + "->" + smove_pretty_to
                        
                        # SHOW FULL OPPONENT MOVE IMMEDIATELY
                        sendToScreen ('Opponent:', smove_full ,'Execute Move', '18')
                        
                        smove_msg ="m"+smove
                        sendtoboard(smove_msg +"-"+ hint)

                        # WAIT FOR PHYSICAL EXECUTION (Wait for Arduino 'x')
                        print("Waiting for physical execution...")
                        
                        while True:
                            cfm = getboard()
                            if cfm is None: continue
                            
                            # Check for Progress 'p' (e.g. pL E2)
                            # Arduino sends: heypipLsq
                            if cfm.startswith('p'):
                                content = cfm[1:] # Lsq or Psq
                                action = content[0] # L or P
                                sq = content[1:]    # E7
                                
                                if action == 'l':
                                    # Lifted -> Mark FROM as active
                                    # e.g. "[E7] -> E5"
                                    display_str = "[" + sq + "] -> " + smove_pretty_to
                                    sendToScreen('Opponent:', display_str, 'Place Target', '18')
                                    
                                elif action == 'p':
                                    # Placed -> Mark TO as active
                                    # e.g. "E7 -> [E5]"
                                    display_str = smove_pretty_from + " -> [" + sq + "]"
                                    sendToScreen('Opponent:', display_str, 'Good!', '18')
                            
                            elif cfm.startswith('w'): # Wrong Piece Warning
                                content = cfm[1:]
                                if content == 'fixed':
                                    # Restored -> Show original instruction again
                                    sendToScreen('Opponent:', smove_full ,'Execute Move', '18')
                                else:
                                    # Wrong Piece -> Warning
                                    wrong_sq = content
                                    sendToScreen('WRONG PIECE!', wrong_sq + ' lifted', 'PUT IT BACK!', '16')
                                    
                            elif cfm.startswith('x'): # Done
                                break
                        
                        # SHOW YOUR GO
                        sendToScreen ('Your Go','','','30')
                        
                        return fmove

def shutdownPi():
    sendToScreen ('Shutting down...','Wait 20s then','disconnect power.')
    time.sleep(5)
    subprocess.call("sudo nohup shutdown -h now", shell=True)

# ---------------- MAIN LOOP ----------------
time.sleep(2) 

gameplayMode = ""
while True:
    sendtoboard("ChooseMode")
    print ("Waiting for mode of play...")
    sendToScreen ('Turn dial to','select mode','& press button')
    
    raw_response = getboard()
    
    if len(raw_response) > 1:
        gameplayMode = raw_response[1:].lower()
        if gameplayMode == 'stockfish' or gameplayMode == 'gstockfish':
            gameplayMode = 'stockfish'
            break
        elif gameplayMode == 'onlinehuman':
            break
        else:
            time.sleep(1)
    else:
        # Check intermediate even here? No.
        time.sleep(1)

if gameplayMode == 'stockfish':
    while True:
        sendtoboard("ReadyStockfish")

        print ("Waiting for level...")
        
        skillFromArduino = ""
        while True:
            raw_msg = getboard()
            raw_msg = raw_msg.lower()
            
            if raw_msg.startswith('q:'):
                try:
                    idx = raw_msg.split(':')[1]
                    import subprocess
                    subprocess.Popen(["python3", "printQuadrants.py", "-s", idx])
                except:
                    pass
            elif raw_msg.startswith('l:'):
                try:
                    idx = raw_msg.split(':')[1]
                    import subprocess
                    subprocess.Popen(["python3", "printToOLED.py", "-a", "Select Level", "-b", "Level " + idx, "-c", ""])
                except:
                    pass
            else:
                 if 'q:' not in raw_msg and 'l:' not in raw_msg:
                     if len(raw_msg) >= 1:
                        if raw_msg.startswith('-'):
                             skillFromArduino = raw_msg[1:]
                        else:
                             skillFromArduino = raw_msg
                        break
        
        print ("Skill Level Selected: " + skillFromArduino)
        
        print ("Waiting for move time...")
        movetimeFromArduino = getboard()
        if movetimeFromArduino.startswith("-"):
             movetimeFromArduino = movetimeFromArduino[1:]
        
        print ("Time Selected: " + movetimeFromArduino)

        print ("\n Chess Program Starting \n")
        skill = skillFromArduino
        movetime = movetimeFromArduino 
        fmove = newgame()

        while True:
            # Main Game Loop - Wait for Human Move
            # Modified to handle intermediate 'i' signals (Lift)
            
            bmessage = getboard() # Blocks
            print ("Command or Info received: " + bmessage)
            code = bmessage[0]
            
            # 1. Check for Intermediate Info (Lift)
            # Example: "iE2" -> Show "E2 -> ..."
            if code == 'i':
                sq = bmessage[1:]
                # We assume human move, so "White Start" or just current board
                # We can print "E2 -> ..."
                sendToScreen(sq.upper() + " -> " + "...", "", "", "20")
                continue 

            if code == 'm':
                fmove = bmove(fmove)
            elif code == 'n':
                fmove = newgame()
            else :
                # Ignore unknown or error
                pass
