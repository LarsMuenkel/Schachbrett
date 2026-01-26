import sys, getopt
from luma.core.interface.serial import i2c
from luma.core.render import canvas
from luma.oled.device import ssd1306
from PIL import ImageFont

# Argumente einlesen
argv = sys.argv[1:]
textLine1 = ''
textLine2 = ''
textLine3 = ''
textSize = 14

try:
    opts, args = getopt.getopt(argv, "ha:b:c:s:", ["firstLine=", "secondLine=", "thirdLine=", "textSize="])
except getopt.GetoptError:
    sys.exit(2)

for opt, arg in opts:
    if opt in ("-a", "--firstLine"): textLine1 = arg
    elif opt in ("-b", "--secondLine"): textLine2 = arg
    elif opt in ("-c", "--thirdLine"): textLine3 = arg
    elif opt in ("-s", "--textSize"): textSize = int(arg)

# Display initialisieren
try:
    serial = i2c(port=1, address=0x3C)
    device = ssd1306(serial)
    
    # WICHTIG: Verhindern, dass das Display beim Beenden gelöscht wird!
    # Wir überschreiben die Aufräum-Funktion mit einer leeren Funktion.
    device.cleanup = lambda: None 
    
except:
    sys.exit()

# Schriftart laden
try:
    font_path = "/home/pi/SmartChess/RaspberryPiCode/WorkSans-Medium.ttf"
    font = ImageFont.truetype(font_path, textSize)
except:
    font = ImageFont.load_default()

# Hilfsfunktion für Textgröße (behebt die DeprecationWarning)
def get_text_size(draw, text, font):
    try:
        # Neue Pillow Versionen
        left, top, right, bottom = draw.textbbox((0, 0), text, font=font)
        return right - left, bottom - top
    except AttributeError:
        # Alte Pillow Versionen (Fallback)
        return draw.textsize(text, font=font)

# Auf das Display zeichnen
with canvas(device) as draw:
    # Zeile 1
    w, h = get_text_size(draw, textLine1, font)
    draw.text(((device.width - w) / 2, 0), textLine1, font=font, fill="white")
    
    # Zeile 2
    w, h = get_text_size(draw, textLine2, font)
    draw.text(((device.width - w) / 2, 20), textLine2, font=font, fill="white")
    
    # Zeile 3
    w, h = get_text_size(draw, textLine3, font)
    draw.text(((device.width - w) / 2, 40), textLine3, font=font, fill="white")
