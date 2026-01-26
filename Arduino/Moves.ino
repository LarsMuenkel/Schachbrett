/*
 * SN74HC165N_shift_reg
 *
 * Program to shift in the bit values from a SN74HC165N 8-bit
 * parallel-in/serial-out shift register.
 *
 * This sketch demonstrates reading in 16 digital states from a
 * pair of daisy-chained SN74HC165N shift registers while using
 * only 4 digital pins on the Arduino.
 *
 * You can daisy-chain these chips by connecting the serial-out
 * (Q7 pin) on one shift register to the serial-in (Ds pin) of
 * the other.
 * 
 * Of course you can daisy chain as many as you like while still
 * using only 4 Arduino pins (though you would have to process
 * them 4 at a time into separate unsigned long variables).
 * 
*/

/* How many shift register chips are daisy-chained.
*/

/* Width of data (how many ext lines).
*/


/* Width of pulse to trigger the shift register to read and latch.
*/
#define PULSE_WIDTH_USEC   5

/* Optional delay between shift register reads.
*/





 


String ValChange;

/* This function is essentially a "shift-in" routine reading the
 * serial Data from the shift register chips and representing
 * the state of those pins in an unsigned integer (or long).
*/
BYTES_VAL_T read_shift_regs()
{
    long bitVal;
    BYTES_VAL_T bytesVal = 0;

    /* Trigger a parallel Load to latch the state of the data lines,
    */
    digitalWrite(clockEnablePin, HIGH);
    digitalWrite(ploadPin, LOW);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(ploadPin, HIGH);
    digitalWrite(clockEnablePin, LOW);

    /* Loop to read each bit value from the serial out line
     * of the SN74HC165N.
    */
    for(long i = 0; i < 32; i++)
    {
        bitVal = digitalRead(dataPin);

        /* Set the corresponding bit in bytesVal.
        */
        bytesVal |= (bitVal << ((32-1) - i));

        /* Pulse the Clock (rising edge shifts the next bit).
        */
        digitalWrite(clockPin, HIGH);
        delayMicroseconds(PULSE_WIDTH_USEC);
        digitalWrite(clockPin, LOW);
    }

    return(bytesVal);
}
BYTES_VAL_T read_shift_regs2()
{
    long bitVal2;
    BYTES_VAL_T bytesVal2 = 0;

    /* Trigger a parallel Load to latch the state of the data lines,
    */
    digitalWrite(clockEnablePin, HIGH);
    digitalWrite(ploadPin, LOW);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(ploadPin, HIGH);
    digitalWrite(clockEnablePin, LOW);

    /* Loop to read each bit value from the serial out line
     * of the SN74HC165N.
    */
    for(long i = 0; i < 32; i++)
    {
        bitVal2 = digitalRead(dataPin2);

        /* Set the corresponding bit in bytesVal.
        */
        bytesVal2 |= (bitVal2 << ((32-1) - i));

        /* Pulse the Clock (rising edge shifts the next bit).
        */
        digitalWrite(clockPin, HIGH);
        delayMicroseconds(PULSE_WIDTH_USEC);
        digitalWrite(clockPin, LOW);
    }

    return(bytesVal2);
}

/* Dump the list of zones along with their current status.
*/
void display_pin_values(){
  String Squares[32] = {"A1","A2","A3","A4","A5","A6","A7","A8", "B1","B2","B3","B4","B5","B6","B7","B8", "C1","C2","C3","C4","C5","C6","C7","C8", "D1","D2","D3","D4","D5","D6","D7","D8"};
    for(long i = 0; i < 32; i++)
    {
        if((pinValues >> i) != (oldPinValues >> i))
            ValChange = (Squares[i]);
       
    }
}
void display_pin_values2(){
  String Squares2[32] ={"E1","E2","E3","E4","E5","E6","E7","E8", "F1","F2","F3","F4","F5","F6","F7","F8", "G1","G2","G3","G4","G5","G6","G7","G8", "H1","H2","H3","H4","H5","H6","H7","H8"};
    for(long i = 0; i < 32; i++)
    {
        if((pinValues2 >> i) != (oldPinValues2 >> i))
            ValChange = (Squares2[i]);
       
    }
}





String HumansGo(){
  String moveFrom = "Nothing";
  String moveTo = "Nothing";
  String humansGo;
  while(true){
    pinValues = read_shift_regs();
    pinValues2 = read_shift_regs2();

    if(pinValues != oldPinValues){
        display_pin_values();   
          if(moveFrom == "Nothing"){
              moveFrom = ValChange;
            
          }
          else {
              moveTo = ValChange;
             
          }
              oldPinValues = pinValues;
    }
    
    if(pinValues2 != oldPinValues2){
        display_pin_values2();
          if(moveFrom == "Nothing"){
              moveFrom = ValChange;
              
          }
          else {
              moveTo = ValChange;
             
         }
        oldPinValues2 = pinValues2;
    }

    if(moveFrom != "Nothing" && moveTo != "Nothing"){
      humansGo = moveFrom + moveTo;
      return humansGo;
      moveFrom = "Nothing";
      moveTo = "Nothing";
      }
    
    delay(100);
  }
}
