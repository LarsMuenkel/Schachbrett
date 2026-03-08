import sys
import time

try:
    from luma.core.interface.serial import i2c
    from luma.core.render import canvas
    from luma.oled.device import ssd1306
    from PIL import ImageFont
except ImportError as e:
    print("Fehler beim Laden der luma Bibliotheken: ", e)
    sys.exit(1)

print("Bibliotheken geladen. Versuche Display auf I2C Port 1, Adresse 0x3C anzusprechen...")

try:
    serial = i2c(port=1, address=0x3C)
    device = ssd1306(serial)
    
    with canvas(device) as draw:
        draw.text((10, 20), "DISPLAY TEST", fill="white")
        draw.text((10, 40), "Es funktioniert!", fill="white")
    
    print("Text wurde ans Display gesendet. Schau auf den OLED-Screen!")
    print("Das Test-Bild verschwindet in 5 Sekunden...")
    time.sleep(5)
    
except Exception as e:
    print("Fehler bei der Kommunikation mit dem OLED:")
    print(e)
