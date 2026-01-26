#!/bin/bash
echo "Übersetze ChessBoard.py auf Python 3..."

FILE="/home/pi/SmartChess/RaspberryPiCode/ChessBoard.py"

# 1. Ersetze .has_key(...) durch den "in" Operator
sed -i 's/specialMoves.has_key(m)/m in specialMoves/g' $FILE
sed -i 's/specialMoves.has_key(toPos)/toPos in specialMoves/g' $FILE
sed -i 's/not files.has_key(t\[-2\])/t[-2] not in files/g' $FILE
sed -i 's/not ranks.has_key(t\[-1\])/t[-1] not in ranks/g' $FILE

# 2. Repariere einen kaputten Print-Befehl am Ende der Datei (Zeile 1302)
# Das Original hat das %-Zeichen außerhalb der Klammer, was in Python 3 abstürzt.
sed -i 's/print ("%d | %s %s %s %s %s %s %s %s |") % (rank,l\[0\],l\[1\],l\[2\],l\[3\],l\[4\],l\[5\],l\[6\],l\[7\])/print ("%d | %s %s %s %s %s %s %s %s |" % (rank,l[0],l[1],l[2],l[3],l[4],l[5],l[6],l[7]))/g' $FILE

echo "Fertig! Der Code ist jetzt Python 3 kompatibel."
