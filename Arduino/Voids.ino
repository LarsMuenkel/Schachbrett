

void setUpBoard(){
        //set up the inital starting positions of chess pieces in the array
        for(int i = 0; i < 2; i++) {
          for(int j = 0; j < 8; j++) {
            currentBoard[i][j] = 1;
            }
         }
        for(int i = 2; i < 6; i++) {
          for(int j = 0; j < 8; j++) {
             currentBoard[i][j] = 0;
             }
          }
        for(int i = 6; i < 8; i++) {
          for(int j = 0; j < 8; j++) {
             currentBoard[i][j] = 1;
            }
          }
      }


void sendToPi(String message, String messageType){
    String messageToSend = message;
    Serial.println("heypi" + messageType + messageToSend);
  }

void waitForPiToStart(){
    Serial.println("Function:waitForPiToStart...   ");
    while (true){
      if (Serial.available() > 0){
        String data = Serial.readStringUntil('\n');
        Serial.println(data);
      if (data.startsWith("heyArduinoChooseMode")){
              sendToPi("Stockfish", "G");
              Serial.println("Pi is going to start a game with Stockfish.");
             break; 
              
          }
// hier irgendwo break einfügen um aus while schleife zu kimmen funktioniert aber nicht da arberitsspeicher zuu gering ????
        }

          
      }
    }

 
    
void setUpGame(){
        String gameDifficulty;
        String gameTimeout;
  
        gameDifficulty = map(CountButtons(), 1, 8, 1 ,20);
        delay(300);
        if (gameDifficulty.length() < 2){
            //Serial.println("Single digit");
            gameDifficulty = ("0" + gameDifficulty);
            }
        //Serial.println(gameDifficulty);
        sendToPi(gameDifficulty, "-");

        //Serial.print("Set computers move timout setting: ");

        gameTimeout = map(CountButtons(), 1, 8, 3000 ,12000);
        delay(300);
        //Serial.println(gameTimeout);
        sendToPi(gameTimeout, "-");
        } 



void printBoard(){
     for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
          Serial.print(currentBoard[i][j]);
          Serial.print(",");
        }
        Serial.println();
      }
    }


bool checkPiForError(){  //check five times during the next 03 seconds to see if we received an error from maxchess on the pi - if so run errorFromPi()
    Serial.print("Function:checkPiForError...   ");
    Serial.println("Waiting for response from Raspberry Pi");
    int checked = 0;
    while (checked<30){
      if (Serial.available() > 0){
        String data = Serial.readStringUntil('\n');
        Serial.println(data);
        if (data.startsWith("heyArduinoerror")){
          Serial.print("Error received from Pi: ");
          Serial.println(data);
          return false;
         }
       } else {
        delay(100);
        Serial.println(checked);
        checked++;
      }
    }
    return true;
  }


void updateBoard(String moveToUpdate){
      //Serial.print("Function:updateBoard - Piece to update: ");
      //Serial.println(moveToUpdate);
  
      int columnMovedFrom = columnNumber(moveToUpdate[0]);
      char rowMovedFrom = moveToUpdate[1];
      int irowMovedFrom = 7-(rowMovedFrom - '1');
      currentBoard[irowMovedFrom][columnMovedFrom] = 0;

      int columnMovedTo = columnNumber(moveToUpdate[2]);
      char rowMovedTo = moveToUpdate[3];
      int irowMovedTo = 7-(rowMovedTo - '1');
      currentBoard[irowMovedTo][columnMovedTo] = 1;
      }
  int columnNumber(char column){
    Serial.println("Function: columnNumber");
    //Serial.println(column);
    switch (column){
      case 'a':
      //Serial.println("Column A converted to number 0.");
      return 0;
      case 'b':
      return 1;
      case 'c':
      return 2;
      case 'd':
      //Serial.println("Column D converted to number 3.");
      return 3;
      case 'e':
      //Serial.println("Column E converted to number 4.");
      return 4;
      case 'f':
      return 5;
      case 'g':
      return 6;
      case 'h':
      return 7;
      default:
      Serial.println("No case statement found!");
    }
  }



String receiveMoveFromPi(){
      Serial.print("Function:receiveMoveFromPi...   ");
      Serial.println("Waiting for response from Raspberry Pi");
      while (true){
        if (Serial.available() > 0){
          String data = Serial.readStringUntil('\n');
          Serial.println(data);
          if (data.startsWith("heyArduinom")){
            Serial.print("Move received from Pi: ");
            data = data.substring(11);
            Serial.println(data);
            return data;
          } else if (data.startsWith("heyArduinoerror")){
            return "error";
          }
          }
          }
          }



void restart(){
    Serial.println("Starting a new game....");
    sendToPi("","n");
    setUpBoard(); //reset all the chess piece positions
    return;
  }

  
