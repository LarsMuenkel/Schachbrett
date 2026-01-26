#!/bin/bash
echo "=============================================="
echo "   SmartChess REPARATUR (für Bookworm OS)     "
echo "=============================================="

# 1. Fehlende System-Programme nachholen
# Wir ersetzen libtiff5 durch libtiff6 und installieren stockfish
echo "[1/3] Installiere Stockfish & System-Treiber..."
sudo apt update
sudo apt install -y python3-dev python3-pip libfreetype6-dev libjpeg-dev build-essential libopenjp2-7 libtiff6 git stockfish

# 2. Python Pakete mit Spezial-Flag installieren
# --break-system-packages ist nötig, weil Bookworm sonst die Installation verweigert.
# Für dein Schachbrett ist das aber genau das Richtige.
echo "[2/3] Installiere Python-Bibliotheken..."
sudo pip3 install --upgrade setuptools --break-system-packages
sudo pip3 install pyserial --break-system-packages
sudo pip3 install python-chess --break-system-packages
sudo pip3 install luma.oled --break-system-packages

# 3. Nochmal sicherstellen, dass der Service aktiviert ist
echo "[3/3] Lade Service neu..."
sudo systemctl daemon-reload
sudo systemctl enable chessDIYM.service
sudo systemctl start chessDIYM.service

echo "=============================================="
echo "REPARATUR ABGESCHLOSSEN!"
echo "Mach jetzt bitte den Test:"
echo "Tippe: which stockfish"
echo "=============================================="
