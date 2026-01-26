# Interactive Chessboard - FINAL FIXED VERSION
from ChessBoard import ChessBoard
import subprocess, time, serial, sys

# initiate chessboard
maxchess = ChessBoard()

if __name__ == '__main__':
    # FIX 1: USB Port hardcoded to ttyUSB0
    ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
    ser.flush()

# initiate stockfish chess engine
# FIX 2: Full path to stockfish
engine = subprocess.Popen(
    '/usr/games/stockfish',
    universal_newlines=True,
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE
    )

def get():
    engine.stdin.write('isready\n')
    engine.stdin.flush() # FIX 3: Flush added
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
    engine.stdin.flush() # FIX 3
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
    engine.stdin.flush() # FIX 3

def getboard():
    """ gets a text string from the board """
    print("\n Waiting for command from the Board")
    while True:
        if ser.in_waiting > 0:
            try:
                btxt = ser.readline().decode('utf-8').rstrip().lower()
            except:
                continue # Ignore decode errors
                
            if btxt.startswith('heypixshutdown'):
                shutdownPi()
                break
            if btxt.startswith('heypi'):
                btxt = btxt[len('heypi'):]
                print("Received from Board: " + btxt)
                return btxt
                break
            else:
                continue
        time.sleep(0.1)

def sendtoboard(stxt):
    """ sends a text string to the board """
    print("\n Sent to board: heyArduino" + stxt)
    stxt = bytes(str(stxt).encode('utf8'))
    time.sleep(2)
    ser.write(b"heyArduino" + stxt + "\n".encode('ascii'))

def sendToScreen(line1,line2,line3,size = '14'):
    screenScriptToRun = ["python3", "/home/pi/SmartChess/RaspberryPiCode/printToOLED.py", '-a '+ line1, '-b '+ line2, '-c '+ line3, '-s '+ size]
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
    sendToScreen ('Please enter','your move:','')
    return fmove

def bmove(fmove):
    fmove=fmove
    brdmove = bmessage[1:5].lower()
    if maxchess.addTextMove(brdmove) == False :
                        etxt = "error"+ str(maxchess.getReason())+brdmove
                        maxchess.printBoard()
                        sendToScreen ('Illegal move!','Enter new','move...','14')
                        sendtoboard(etxt)
                        return fmove
    else:
        maxchess.printBoard()
        sendToScreen (brdmove[0:2] + '->' + brdmove[2:4] ,'','Thinking...','20')
        fmove =fmove+" " +brdmove
        cmove = "position startpos moves"+fmove
        put(cmove)
        put("go movetime " +movetime)
        text = sget()
        smove = text[9:13]
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
                        sendToScreen (smove[0:2] + '->' + smove[2:4] ,'','Your go...','20')
                        smove ="m"+smove
                        sendtoboard(smove +"-"+ hint)
                        return fmove

def shutdownPi():
    sendToScreen ('Shutting down...','Wait 20s then','disconnect power.')
    time.sleep(5)
    subprocess.call("sudo nohup shutdown -h now", shell=True)

# ---------------- MAIN LOOP START ----------------
time.sleep(2) # Wait for Arduino to boot

# FIX 4: Loop until valid mode is selected (prevents crash on boot messages)
gameplayMode = ""
while True:
    sendtoboard("ChooseMode")
    print ("Waiting for mode of play...")
    sendToScreen ('Choose opponent:','1) Against PC','2) Remote human')
    
    raw_response = getboard() 
    # raw_response is e.g. "gstockfish" or "waitforpitostart"
    
    if len(raw_response) > 1:
        gameplayMode = raw_response[1:].lower()
        print("Detected Mode: " + gameplayMode)
        
        if gameplayMode == 'stockfish':
            break
        elif gameplayMode == 'onlinehuman':
            break
        else:
            print("Ignored invalid mode: " + gameplayMode)
            time.sleep(1)
    else:
        time.sleep(1)

# MODE: STOCKFISH
if gameplayMode == 'stockfish':
    while True:
        sendtoboard("ReadyStockfish")

        print ("Waiting for level...")
        sendToScreen ('Choose computer','difficulty level:','(0 -> 8)')
        skillFromArduino = getboard()[1:3].lower()
        
        print ("Waiting for move time...")
        sendToScreen ('Choose computer','move time:','(0 -> 8)')
        movetimeFromArduino = getboard()[1:].lower()

        print ("\n Chess Program Starting \n")
        skill = skillFromArduino
        movetime = movetimeFromArduino 
        fmove = newgame()

        while True:
            bmessage = getboard()
            print ("Command received: " + bmessage)
            code = bmessage[0]

            if code == 'm':
                fmove = bmove(fmove)
            elif code == 'n':
                fmove = newgame()
            else :
                sendtoboard('error at option')

# MODE: ONLINE (Not fully patched/tested here, keeping basic structure)
elif gameplayMode == 'onlinehuman':
    print("Online mode not fully supported in this patch version.")
    # Placeholder to prevent exit
    while True:
        time.sleep(1)

