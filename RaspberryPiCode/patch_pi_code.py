import os

# Die Dateien, die wir versuchen zu patchen
target_files = ["StartChessGame.py", "StartChessGameStockfish.py"]

# Der Marker, nach dem wir suchen (Die Zeile VOR unserem Eingriff)
search_marker = "sendToScreen ('Choose computer','difficulty level:','(0 -> 8)')"

# Der Code-Block, den wir einfügen wollen
# (Platzhalter {{INDENT}} wird durch die korrekte Einrückung der Zieldatei ersetzt)
new_code_template = """
{{INDENT}}# --- PATCH START: QUADRANTEN AUSWAHL ---
{{INDENT}}skillFromArduino = ""
{{INDENT}}while True:
{{INDENT}}    try:
{{INDENT}}        raw_msg = getboard() # Liest Nachricht vom Arduino
{{INDENT}}        
{{INDENT}}        # Fall 1: Quadranten-Auswahl (z.B. Q:2)
{{INDENT}}        if raw_msg.startswith('Q:'):
{{INDENT}}            idx = raw_msg.split(':')[1]
{{INDENT}}            import subprocess
{{INDENT}}            subprocess.Popen(["python3", "printQuadrants.py", "-s", idx])
{{INDENT}}            
{{INDENT}}        # Fall 2: Level-Auswahl (z.B. L:4)
{{INDENT}}        elif raw_msg.startswith('L:'):
{{INDENT}}            idx = raw_msg.split(':')[1]
{{INDENT}}            import subprocess
{{INDENT}}            subprocess.Popen(["python3", "printToOLED.py", "-a", "Select Level", "-b", "Level " + idx, "-c", ""])
{{INDENT}}            
{{INDENT}}        # Fall 3: Finale Auswahl abgeschlossen (z.B. 05 oder -05)
{{INDENT}}        else:
{{INDENT}}            # Arduino sendet z.B. -05. Wir nehmen die letzten 2 Ziffern.
{{INDENT}}            # Original Code erwartete genau dies.
{{INDENT}}            if len(raw_msg) >= 2:
{{INDENT}}                skillFromArduino = raw_msg[-2:] 
{{INDENT}}            else:
{{INDENT}}                skillFromArduino = raw_msg
{{INDENT}}            break
{{INDENT}}            
{{INDENT}}    except Exception as e:
{{INDENT}}        print("Error in input loop: " + str(e))
{{INDENT}}        continue
{{INDENT}}# --- PATCH END ---
"""

def patch_file(filename):
    if not os.path.exists(filename):
        print("Datei nicht gefunden (uebersprungen): " + filename)
        return

    print("Bearbeite " + filename + "...")
    
    with open(filename, "r") as f:
        lines = f.readlines()
    
    new_lines = []
    patched_count = 0
    i = 0
    
    while i < len(lines):
        line = lines[i]
        new_lines.append(line)
        
        # Prüfen ob wir an der richtigen Stelle sind
        if search_marker in line:
            # Wir haben die Zeile gefunden. Jetzt die Einrückung (Leerzeichen am Anfang) ermitteln.
            indentation = line[:len(line) - len(line.lstrip())]
            
            # Prüfen, ob die nächste Zeile die alte Logik enthält (skillFromArduino = ...)
            # Damit wir nicht doppelt patchen.
            if i + 1 < len(lines) and "skillFromArduino =" in lines[i+1]:
                print(" -> Stelle gefunden. Fuege neuen Code ein...")
                
                # Template mit der richtigen Einrückung füllen
                code_to_insert = new_code_template.replace("{{INDENT}}", indentation)
                
                new_lines.append(code_to_insert)
                
                # WICHTIG: Die ALTE Zeile ("skillFromArduino = ...") überspringen wir,
                # da sie durch unseren neuen Block ersetzt wurde.
                i += 1 
                patched_count += 1
            else:
                print(" -> Marker gefunden, aber sieht bereits gepatcht aus.")
        
        i += 1
        
    if patched_count > 0:
        # Backup erstellen
        try:
             os.rename(filename, filename + ".bak")
        except FileExistsError:
             os.remove(filename + ".bak")
             os.rename(filename, filename + ".bak")
             
        # Datei neu schreiben
        with open(filename, "w") as f:
            f.writelines(new_lines)
        print(" -> Erfolgreich: " + filename + " aktualisiert! (Backup: .bak)")
    else:
        print(" -> Keine Aenderungen vorgenommen.")

# Hauptprogramm
for f in target_files:
    patch_file(f)
