import os
import sys
import re

def create_print_quadrants():
    filename = "printQuadrants.py"
    # Content of printQuadrants.py
    content = """import sys, getopt
from luma.core.interface.serial import i2c
from luma.core.render import canvas
from luma.oled.device import ssd1306
from PIL import ImageFont

selection = 1

# Argumente lesen (-s für Selection)
try:
    opts, args = getopt.getopt(sys.argv[1:], "s:", ["selection="])
    for opt, arg in opts:
        if opt in ("-s", "--selection"):
            selection = int(arg)
except getopt.GetoptError:
    sys.exit(2)

# Display Setup
try:
    serial = i2c(port=1, address=0x3C)
    device = ssd1306(serial)
    # Wichtig: Display NICHT löschen beim Beenden des Skripts
    device.cleanup = lambda: None
except:
    sys.exit()

with canvas(device) as draw:
    # Raster zeichnen (Kreuz)
    draw.line((64, 0, 64, 64), fill="white")
    draw.line((0, 32, 128, 32), fill="white")
    
    # Schriftart laden
    try:
        font = ImageFont.load_default()
    except:
        font = None 
    
    # Labels zeichnen (Zentriert in den Quadranten)
    draw.text((15, 10), "Easy", font=font, fill="white")
    draw.text((80, 10), "Med", font=font, fill="white")
    draw.text((15, 42), "Hard", font=font, fill="white")
    draw.text((80, 42), "Extr", font=font, fill="white")

    # Auswahl hervorheben (Dicker Rahmen um den gewählten Quadranten)
    box = None
    if selection == 1:   box = [0, 0, 63, 31]
    elif selection == 2: box = [64, 0, 127, 31]
    elif selection == 3: box = [0, 32, 63, 63]
    elif selection == 4: box = [64, 32, 127, 63]
        
    if box:
        draw.rectangle(box, outline="white")
        draw.rectangle([box[0]+1, box[1]+1, box[2]-1, box[3]-1], outline="white")
"""
    with open(filename, 'w') as f:
        f.write(content)
    print(f"Created/Updated {filename}")

def patch_start_chess_game():
    filename = "StartChessGame.py"
    if not os.path.exists(filename):
        print(f"Error: {filename} not found in current directory.")
        return

    print(f"Patching {filename}...")
    with open(filename, 'r') as f:
        lines = f.readlines()
    
    full_content = "".join(lines)
    
    # ---------------------------------------------------------
    # PATCH 1: ROBUST PORT DISCOVERY
    # ---------------------------------------------------------
    # Check if already patched
    if "ports_to_try =" in full_content:
        print(" -> Port discovery already patched.")
    else:
        print(" -> Applying Port discovery fix...")
        new_lines = []
        skip = False
        patched_port = False
        
        for line in lines:
            # Look for the hardcoded serial line
            if "ser = serial.Serial(" in line and "/dev/tty" in line:
                # Insert our new block
                indent = line[:len(line) - len(line.lstrip())]
                new_block = [
                    f"{indent}# FIX: Robust Port Discovery\n",
                    f"{indent}ports_to_try = ['/dev/ttyACM0', '/dev/ttyUSB0', '/dev/ttyACM1', '/dev/ttyUSB1']\n",
                    f"{indent}ser = None\n",
                    f"{indent}for port in ports_to_try:\n",
                    f"{indent}    try:\n",
                    f"{indent}        print('Trying to connect to ' + port + '...')\n",
                    f"{indent}        ser = serial.Serial(port, 9600, timeout=1)\n",
                    f"{indent}        print('Connected to ' + port)\n",
                    f"{indent}        break\n",
                    f"{indent}    except serial.SerialException:\n",
                    f"{indent}        print('Failed to connect to ' + port)\n",
                    f"{indent}        pass\n",
                    f"{indent}if ser is None:\n",
                    f"{indent}    print('CRITICAL ERROR: Could not find Arduino on standard ports.')\n",
                    f"{indent}    sys.exit(1)\n"
                ]
                new_lines.extend(new_block)
                patched_port = True
            else:
                new_lines.append(line)
        
        if patched_port:
            lines = new_lines
            full_content = "".join(lines) # Update for next patch
        else:
            print("WARNING: Could not find 'ser = serial.Serial(...)' line to patch.")

    # ---------------------------------------------------------
    # PATCH 2: QUADRANT SELECTION
    # ---------------------------------------------------------
    marker = "sendToScreen ('Choose computer','difficulty level:','(0 -> 8)')"
    if marker in full_content:
        # Check if already has the logic
        if "skillFromArduino = raw_msg[-2:]" in full_content or "skillFromArduino = \"\"" in full_content:
            print(" -> Quadrant selection already patched.")
        else:
            print(" -> Applying Quadrant selection fix...")
            new_lines = []
            
            # Logic to insert
            patch_code_str = """
        # --- PATCH START: QUADRANTEN AUSWAHL ---
        skillFromArduino = ""
        while True:
            try:
                raw_msg = getboard()
                if raw_msg.startswith('Q:'):
                    idx = raw_msg.split(':')[1]
                    import subprocess
                    subprocess.Popen(["python3", "printQuadrants.py", "-s", idx])
                elif raw_msg.startswith('L:'):
                    idx = raw_msg.split(':')[1]
                    import subprocess
                    subprocess.Popen(["python3", "printToOLED.py", "-a", "Select Level", "-b", "Level " + idx, "-c", ""])
                else:
                    # Filter garbage, accept valid skill
                    if len(raw_msg) >= 2 and raw_msg[0].lower() not in ['q', 'l']:
                        skillFromArduino = raw_msg[-2:]
                        break
                    elif len(raw_msg) > 0 and raw_msg.isdigit():
                         skillFromArduino = raw_msg
                         break
            except Exception as e:
                pass
        # --- PATCH END ---
"""
            
            for i, line in enumerate(lines):
                new_lines.append(line)
                if marker in line:
                    # Insert the patch AFTER this line
                    indent = line[:len(line) - len(line.lstrip())]
                    indented_patch = patch_code_str.replace("        ", indent) # rough indent fix
                    new_lines.append(indented_patch)
                    
                    # COMMENT OUT the old logic line if it follows
                    # We look ahead (simple version)
                    # Next line is likely `skillFromArduino = ...`
                    # We will handle this by checking subsequent lines in a simple loop or just relying on the user to check?
                    # Better: Scan specific lines to comment out.
            
            # Re-process to comment out the OLD skill assignment if it exists
            # It usually looks like `skillFromArduino = getboard()[1:3].lower()`
            final_lines = []
            commented = False
            for line in new_lines:
                if not commented and "skillFromArduino =" in line and "getboard" in line and "PATCH" not in line:
                    final_lines.append("# " + line)
                    commented = True
                else:
                    final_lines.append(line)
            
            lines = final_lines

    # WRITE BACK
    with open(filename, 'w') as f:
        f.writelines(lines)
    print(f"Successfully updated {filename}")

if __name__ == "__main__":
    create_print_quadrants()
    patch_start_chess_game()
