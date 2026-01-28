import sys, getopt
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
        font = None # Fallback if needed, but load_default should work
    
    # Labels zeichnen (Zentriert in den Quadranten)
    # Font ist optional bei default, aber wir geben size explizit nicht an (default ist klein)
    draw.text((15, 10), "Easy", font=font, fill="white")
    draw.text((80, 10), "Med", font=font, fill="white")
    draw.text((15, 42), "Hard", font=font, fill="white")
    draw.text((80, 42), "Extr", font=font, fill="white")

    # Auswahl hervorheben (Dicker Rahmen um den gewählten Quadranten)
    # Koordinaten: [x0, y0, x1, y1]
    box = None
    if selection == 1:   box = [0, 0, 63, 31]
    elif selection == 2: box = [64, 0, 127, 31]
    elif selection == 3: box = [0, 32, 63, 63]
    elif selection == 4: box = [64, 32, 127, 63]
        
    if box:
        # Rahmen mehrfach zeichnen für Dicke
        draw.rectangle(box, outline="white")
        draw.rectangle([box[0]+1, box[1]+1, box[2]-1, box[3]-1], outline="white")
