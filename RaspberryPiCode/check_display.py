import sys
import time
import subprocess

print("--- Schritt 1: I2C Bus Scan ---")
try:
    # Führt den Standard-Scan-Befehl i2cdetect auf dem Pi aus
    result = subprocess.check_output(['i2cdetect', '-y', '1']).decode('utf-8')
    print("Ergebnis von i2cdetect:")
    print(result)
    
    if "3c" in result.lower():
        print("Erfolg! Display auf Adresse 0x3C gefunden.")
    else:
        print("WARNUNG: Kein Display auf 0x3C gefunden! Bitte Verkabelung prüfen.")
        print("Pins: 1 (oder 17) = 3.3V, 6 (oder 9) = GND, 3 = SDA, 5 = SCL")
        
except Exception as e:
    print("Konnte i2cdetect nicht ausführen:", e)

print("\n--- Schritt 2: Luma OLED Test ---")
try:
    from luma.core.interface.serial import i2c
    from luma.core.render import canvas
    from luma.oled.device import ssd1306
except ImportError as e:
    print("FEHLER: Die luma.oled Bibliothek fehlt! (Wahrscheinlich nicht installiert?)")
    sys.exit()

try:
    # Port 1, Address 0x3C ist Standard für Pi + SSD1306
    serial = i2c(port=1, address=0x3C)
    device = ssd1306(serial)
    
    with canvas(device) as draw:
        # Ein einfaches Rechteck und Text
        draw.rectangle(device.bounding_box, outline="white", fill="black")
        draw.text((10, 20), "HARDWARE", fill="white")
        draw.text((10, 40), "TEST OK!", fill="white")
        
    print("Text gesendet! Bitte schau auf das Display.")
    time.sleep(5)
    
except Exception as e:
    print("FEHLER beim Kommunizieren mit dem SSD1306 via Luma:")
    print(e)
