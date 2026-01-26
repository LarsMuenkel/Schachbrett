#define ARDU_PINS int
ARDU_PINS restartButton = 10;
ARDU_PINS buttonPin = 8;
ARDU_PINS OkButton = 11;
ARDU_PINS DelButton = 12;
ARDU_PINS ploadPin        = 7;  // Connects to Parallel load pin the 165
ARDU_PINS clockEnablePin  = 4;  // Connects to Clock Enable pin the 165
ARDU_PINS dataPin         = 5; // Connects to the Q7 pin the 165
ARDU_PINS dataPin2        = 9; // Connects to the Q7 pin the 165
ARDU_PINS clockPin        = 6; // Connects to the Clock pin the 165

#define BYTES_VAL_T unsigned long
BYTES_VAL_T pinValues;
BYTES_VAL_T oldPinValues;
BYTES_VAL_T pinValues2;
BYTES_VAL_T oldPinValues2;

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>


int currentBoard[8][8]; //an array to track the current state of the board


String pisMove;
String pisSuggestedBestMove;
bool legalOrNot;


void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ploadPin, OUTPUT);
  pinMode(clockEnablePin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);
  pinMode(dataPin2, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(OkButton, INPUT_PULLUP);
  pinMode(DelButton, INPUT_PULLUP);


  digitalWrite(clockPin, LOW);
  digitalWrite(ploadPin, HIGH);
  
  Serial.begin(9600);

  //setup the hint interupt pin
  attachInterrupt(digitalPinToInterrupt(restartButton), restart, FALLING);

  setUpBoard();
  waitForPiToStart();
  setUpGame();

  pinValues = read_shift_regs();
  pinValues2 = read_shift_regs2();
  display_pin_values();
  display_pin_values2();
  oldPinValues = pinValues;
  oldPinValues2 = pinValues2;
}




void loop() {
  // put your main code here, to run repeatedly:
  String humansMove = HumansGo();
  sendToPi(humansMove, "M");
  printBoard();
  legalOrNot = checkPiForError();
  Serial.println("Checking legality");
  if (legalOrNot == false ){
    // do nothing and start the human request again
    Serial.println("Move discarded, please return the pieces and try another move");
    printBoard();
  } else {
    Serial.println("Move is legal");
    updateBoard(humansMove);
    printBoard();
    pisMove = receiveMoveFromPi();
    pisSuggestedBestMove = pisMove.substring(5);
      Serial.println("Suggested next move = " + pisSuggestedBestMove);
      Serial.println(pisMove);
    updateBoard(pisMove);
    printBoard();
  }
}
