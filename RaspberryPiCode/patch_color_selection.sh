#!/bin/bash
# Patch v2: Farbwahl + Fix fuer StartChessGame.py
# Ausfuehren auf dem Pi: bash patch_color_selection.sh

cd /home/pi/SmartChess/RaspberryPiCode || { echo "Verzeichnis nicht gefunden!"; exit 1; }

FILE="StartChessGame.py"
BACKUP="${FILE}.bak_$(date +%Y%m%d_%H%M%S)"

cp "$FILE" "$BACKUP"
echo "Backup erstellt: $BACKUP"

python3 << 'PYEOF'
filename = "StartChessGame.py"
with open(filename, 'r') as f:
    content = f.read()

# ---- PATCH 1: Initial color screen + F:/C: handlers ----
# Find the stockfish setup block and replace it

old_block_1 = '''        sendtoboard("ReadyStockfish")

        print ("Waiting for level...")
        
        skillFromArduino = ""
        while True:
            raw_msg = getboard()
            raw_msg = raw_msg.lower()
            
            if raw_msg.startswith('q:'):'''

new_block_1 = '''        sendtoboard("ReadyStockfish")
        
        # Show initial color selection screen
        sendToScreen('>> Weiss <<', 'Schwarz', '', '18')

        print ("Waiting for color & level...")
        
        skillFromArduino = ""
        playerColor = "w"  # Default white
        while True:
            raw_msg = getboard()
            raw_msg = raw_msg.lower()
            
            # Color selection display update
            if raw_msg.startswith('f:'):
                try:
                    idx = raw_msg.split(':')[1]
                    if idx == '1':
                        sendToScreen('>> Weiss <<', 'Schwarz', '', '18')
                    else:
                        sendToScreen('Weiss', '>> Schwarz <<', '', '18')
                except:
                    pass
            # Color confirmed
            elif raw_msg.startswith('c:'):
                try:
                    playerColor = raw_msg.split(':')[1]
                    if playerColor == 'w':
                        sendToScreen('Weiss', 'gewaehlt!', '', '20')
                    else:
                        sendToScreen('Schwarz', 'gewaehlt!', '', '20')
                    time.sleep(1)
                except:
                    pass
            elif raw_msg.startswith('q:'):'''

# Also try the already-patched version
old_block_2 = '''        sendtoboard("ReadyStockfish")
        
        # Show initial color selection screen
        sendToScreen('>> Weiss <<', 'Schwarz', '', '18')

        print ("Waiting for color & level...")
        
        skillFromArduino = ""
        playerColor = "w"  # Default white
        while True:
            raw_msg = getboard()
            raw_msg = raw_msg.lower()
            
            # Color selection display update
            if raw_msg.startswith('f:'):'''

if old_block_2 in content:
    print("Bereits vollstaendig gepatcht!")
elif old_block_1 in content:
    content = content.replace(old_block_1, new_block_1)
    with open(filename, 'w') as f:
        f.write(content)
    print("Patch erfolgreich angewendet!")
else:
    # Try partial match - maybe already has playerColor but not initial screen
    old_partial = '''        sendtoboard("ReadyStockfish")

        print ("Waiting for level...")
        
        skillFromArduino = ""
        playerColor = "w"'''
    
    new_partial = '''        sendtoboard("ReadyStockfish")
        
        # Show initial color selection screen
        sendToScreen('>> Weiss <<', 'Schwarz', '', '18')

        print ("Waiting for color & level...")
        
        skillFromArduino = ""
        playerColor = "w"'''
    
    if old_partial in content:
        content = content.replace(old_partial, new_partial)
        with open(filename, 'w') as f:
            f.write(content)
        print("Patch (partial) erfolgreich angewendet!")
    else:
        print("WARNUNG: Konnte den erwarteten Block nicht finden.")
        print("Bitte manuell pruefen.")
        exit(1)

PYEOF

echo "Fertig!"
