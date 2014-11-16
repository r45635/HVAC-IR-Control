
//* AnalysIR Firmware
//* Date: 04th September 2014
//* Release: 1.0.x
//* Note: Please use and read in conjunction with the README and the AnalysIR Getting Started Guide.
//* Licence: Free to use & modify without restriction or warranty.
//*          Please acknowledge AnalysIR as the Author in any derivative and include a link to http://www.AnalysIR.com in any publication.
//*
//* Tested with Arduino IDE v1.5.5, Energia IDE 0101E0011, & Mpide 0023-windows-20140821 only.

//see *** note 1 in README
#define enableIRrx attachInterrupt(0, rxIR_Interrupt_Handler, CHANGE) //set up interrupt handler for IR rx   on pin 2 - demodulated signal
#define enableIRrxMOD attachInterrupt(1, rxIR3_Interrupt_Handler, FALLING) //Same for Pin3 - modulated signal

//set true for Microcontroller system being used, set all others to false
#define Arduino328 true   //Any standard Arduino - Duemilenova, Nano or compatible with ATMega328x @16Mhz
#define ArduinoUNO false   //Any  Arduino - UNO @16Mhz
#define ArduinoLeonardo false  //Select this for for Arduino YÃºn & Arduino (pro)Micro also, when connected via USB as the YÃºn is also a Leonardo
#define ArduinoMega1280 false    //Also Mega2560 - untested
#define ArduinoDUE false         //
#define ArduinoYUN false         //
#define MSP430F5529 false        //
#define TivaC false              //
#define FubarinoMini false        // 
#define Teensy3 false		//tested on teensy 3.1, but should also work for Teensy 3.0
//............................................................................................

//Definitions for Buffers here...size RAM dependent
#define maxPULSES 700 //increase until memory is used up.
#define modPULSES 256 //increase until memory is used up, max 256, leave at 256.

//General Definitions
// Reporting of available SRAM is only supported for 8 bit AVR MCUs
#define AVR8BITMCU true		//this is re-set to false, for non 8bit AVRs, in platform specific defines below

//Baud rate is now fixed to 115200 for all devices, to avoid issues with some platforms
#define BAUDRATE 115200

//see *** note 4 in README
//Pin definitions, these can be redefined for the various platforms
#define IR_Rx_PIN  2 //IR digital input pin definition - drives interrupt
#define IR_Mod_PIN 3 //IR digital Raw Modulation input pin definition - drives interrupt
#define ledPin    13 //used to signal waiting for serial COM port, when using Leonardo

//see *** note 2 in README
//Default is all standard Arduinos, with ATMega328, USB serial & 16MHz oscillator.
#if Arduino328 || ArduinoUNO
#define pin2HIGH (PIND & 0b00000100) //Port D bit 2

//ArduinoLeonardo
#elif ArduinoLeonardo || ArduinoYUN
#define pin2HIGH (PIND & 0b00000010) //Port D bit 1
//Following must be redefined as interrupt pins are different on Leonardo
#define enableIRrx attachInterrupt(1, rxIR_Interrupt_Handler, CHANGE) //set up interrupt handler for IR rx   on pin 2 - demodulated signal
#define enableIRrxMOD attachInterrupt(0, rxIR3_Interrupt_Handler, FALLING); //Same for Pin3 - modulated signal
#define maxPULSES 906 //More RAM is available on Leonardo

#elif ArduinoMega1280
#define pin2HIGH (PINE & 0b00010000) //Port E bit 4
#define maxPULSES 1024 //even More RAM is available on Megas...can increase this for longer signals if neccessary

//Arduino DUE
#elif ArduinoDUE
#define AVR8BITMCU false
#define enableIRrxMOD attachInterrupt(IR_Mod_PIN, rxIR3_Interrupt_Handler, FALLING) //set up interrupt handler for IR rx   on pin 2 - demodulated signal
#define enableIRrx attachInterrupt(IR_Rx_PIN, rxIR_Interrupt_Handler, CHANGE) //set up interrupt handler for IR rx   on pin 2 - demodulated signal
#define pin2HIGH digitalRead(IR_Rx_PIN)
#define maxPULSES 1024 //More RAM is available on DUE (96K)

//MSP430F5529
#elif MSP430F5529 //doesn't capture modulation frequency..yet
#define AVR8BITMCU false
#define IR_Rx_PIN P2_6
#define IR_Mod_PIN P2_3
#define ledPin    RED_LED //used to signal waiting for serial COM port, when using Leonardo
#define enableIRrxMOD  //no modulation frequency suppoted on this platform, until timing is better
#define enableIRrx attachInterrupt(IR_Rx_PIN, rxIR_Interrupt_Handler, CHANGE) //set up interrupt handler for IR rx   on pin 2 - demodulated signal
#define pin2HIGH P2IN & BIT6
#define maxPULSES 1024 //More RAM is available on this MSP430 (8K)

//Tiva C
#elif TivaC
#define AVR8BITMCU false
#define IR_Rx_PIN PA_4
#define IR_Mod_PIN PA_3
#define ledPin    RED_LED //used to signal waiting for serial COM port, when using Leonardo
#define enableIRrxMOD attachInterrupt(IR_Mod_PIN, rxIR3_Interrupt_Handler, FALLING) //set up interrupt handler for IR rx   on pin 2 - demodulated signal
#define enableIRrx attachInterrupt(IR_Rx_PIN, rxIR_Interrupt_Handler, CHANGE) //set up interrupt handler for IR rx   on pin 2 - demodulated signal
#define pin2HIGH digitalRead(IR_Rx_PIN)
#define maxPULSES 1024 //More RAM is available on this MSP430 (32K)

//Teensy 3.1 (should also work for Teensy 3.0)
#elif Teensy3
#define AVR8BITMCU false
#define IR_Rx_PIN 2
#define IR_Mod_PIN 3
#define pin2HIGH digitalRead(IR_Rx_PIN)
#define enableIRrxMOD attachInterrupt(IR_Mod_PIN, rxIR3_Interrupt_Handler, FALLING) //set up interrupt handler for IR rx   on pin 2 - demodulated signal
#define enableIRrx attachInterrupt(IR_Rx_PIN, rxIR_Interrupt_Handler, CHANGE) //set up interrupt handler for IR rx   on pin 2 - demodulated signal
#define maxPULSES 1024 //More RAM is available on Teensy 3.1 (64K) & Teensy 3.0 (16K)

//FubarinoMini / ChipKit
#elif FubarinoMini
#define AVR8BITMCU false
#define IR_Rx_PIN PIN_INT2  //NB  This is on Pin 0 on FBino (not Pin 2)
#define IR_Mod_PIN PIN_INT1  //NB This is on Pin 3 on FBino
#define ledPin    PIN_LED1 //used to signal waiting for serial COM port, when using Leonardo
#define enableIRrxMOD attachInterrupt(1, rxIR3_Interrupt_Handler, FALLING) //set up interrupt handler for IR rx   on pin 2 - demodulated signal
#define enableIRrx attachInterrupt(2, rxIR_Interrupt_Handler, FALLING) //set up interrupt handler for IR rx   on pin 2 - demodulated signal
#define pin2HIGH digitalRead(0)
#define maxPULSES 1024 //More RAM is available on this Fubarino Mini (32K)
#endif
//...............................................................................................

//see *** note 3 in README
unsigned int pulseIR[maxPULSES]; //temp store for pulse durations (demodulated)
volatile byte modIR[modPULSES]; //temp store for modulation pulse durations - changed in interrupt

//General variables
//see *** note 5 in README
volatile byte state = 127; //defines initial value for state normally HIGH or LOW. Set in ISR.
byte oldState = 127; //set both the same to start. Used to test for change in state signalled from ISR
unsigned long usLoop, oldTime; //timer values in uSecs. usLoops stores the time value of each loop, oldTime remembers the last time a state change was received
volatile unsigned long newMicros;//passes the time a state change occurred from ISR to main loop
unsigned long oldMicros = 0; //passes the time a state change occurred from ISR to main loop
unsigned int countD = 0; // used as a pointer through the buffers for writing and reading (de-modulated signal)
volatile byte countM = 0; // used as a pointer through the buffers for writing and reading (modulated signal)
byte countM2 = 0; //used as a counter, for the number of modulation samples used in calculating the period.
unsigned int i = 0; //used to iterate in for loop...integer
byte j = 0; //used to iterate in for loop..byte
unsigned long sum = 0; //used in calculating Modulation frequency
byte sigLen = 0; //used when calculating the modulation period. Only a byte is required.
volatile boolean intDirection = true; //only used for Fubarino/ChipKit, can be removed for other platforms if neccessary

//Serial Tx buffer - uses Serial.write for faster execution
//see *** note 6 in README
byte txBuffer[5]; //Key(+-)/1,count/1,offset/4,CR/1   <= format of packet sent to AnalysIR over serial

//see *** note 7 in README
void setup() {

  Serial.begin(BAUDRATE);//fixed at 115200 bps for all platforms
  delay(500);//to avoid potential conflict with boot-loader on some systems
  txBuffer[4] = 13; //init ascii decimal value for CR in tx buffer

  pinMode(IR_Rx_PIN, INPUT);
  pinMode(IR_Mod_PIN, INPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, HIGH); //if LED stays on after reset, then serial not recognised PC
  delay(500);//time to see blink
#if ArduinoLeonardo
  while (!Serial); //for Leonardo only, wait for serial to be connected
#endif
  digitalWrite(ledPin, LOW);

  Serial.println(F("!AnalysIR!")); // HELLO STRING - ALL COMMENTS SENT IN !....! FORMAT

  //Initialise State
  oldState = digitalRead(IR_Rx_PIN);
  state = oldState;

  //Initialise Times
  oldTime = 0; //init
  newMicros = micros(); //init
  oldMicros = newMicros;

  //following line not required - just reports free RAM on Arduino if there are problems
  reportFreeRAM(0xFFFF);// report free ram to host always, use max UInt value of 0xFFFF. 8Bit AVRs only
  //

  //turn on interrupts and GO!
  enableIRrx;//set up interrupt handler for IR rx on pin 2 - demodulated signal
  enableIRrxMOD; //set up interrupt handler for modulated IR rx on pin 3 - full modulated signal

}

//see *** note 8 in README
void loop() {
  while (true) { //avoid any extra stuff inserted by Arduino IDE at end of each normal Loop

    usLoop = micros(); //used once every loop rather than multiple calls
    //see *** note 9 in README
    if (oldState != state && countD < maxPULSES) {
      oldState = state;
      if (oldState) { //if the duration is longer than 0xFFFF(65535 uSecs) then multiple repeat pulses are stored, whith LSB used to signal mark or space
        sum = (newMicros - oldMicros); //re-use sum var here, to save RAM (normally used in reportperiod)
        while (sum > 0xFFFF && countD < (maxPULSES - 1) && countD) { //this allows for a mark/space of greater than 65535 uSecs (0xFFFF), ignore first signal
          sum -= 65535;//this assumes the length is not longer than 131070
          pulseIR[countD++] = 65535 | 0x0001; //store for later & include state
        }
        pulseIR[countD++] = sum | 0x0001; //store for later & include state
      }
      else {
        sum = (newMicros - oldMicros); //re-use sum var here, to save RAM (normally used in reportperiod)
        while (sum > 0xFFFF && countD < (maxPULSES - 1) && countD) { //this allows for a mark/space of greater than 65535 uSecs (0xFFFF), ignore first signal
          sum -= 65535;//this assumes the length is not longer than 131070
          pulseIR[countD++] = 65535 & 0xFFFE; //store for later & include state
        }
        pulseIR[countD++] = sum & 0xFFFE; //store for later & include state
      }
      oldMicros = newMicros; //remember for next time
      oldTime = usLoop; //last time IR was received
    }

    //see *** note 10 in README
    if (state && countD > 0 && (countD == maxPULSES ||  (usLoop - oldTime) > 100000)) { //if we have received maximum pulses or its 100ms since last one
      reportPulses();
      reportPeriod();//reports modulation frequency to host over serial
      countD = 0; //reset value for next time
    }

    //see *** note 11 in README
    /*  this code only used, for debugging, if we are having problems with available RAM
     reportFreeRAM(200);//report freeram as comment to host if less than 200 bytes
     */
  }
}

//see *** note 12 in README
void  reportPulses() {
  for (i = 0; i < countD; i++) {
    //the following logic takes care of the inverted signal in the IR receiver
    if (pulseIR[i] & 0x01) txBuffer[0] = '+'; //Mark is sent as +...LSB bit of pulseIR is State(Mark or Space)
    else txBuffer[0] = '-';           //Space is sent as -
    txBuffer[1] = (byte) (i & 0xFF); //count
    txBuffer[3] = pulseIR[i] >> 8; //byte 1
    txBuffer[2] = pulseIR[i] & 0xFE; //LSB 0 ..remove lat bit as it was State
    Serial.write(txBuffer, 5);
  }
}

//see *** note 13 in README
void  reportPeriod() { //report period of modulation frequency in nano seconds for more accuracy
  sum = 0; // UL
  sigLen = 0; //byte
  countM2 = 0; //byte

  for (j = 1; j < (modPULSES - 1); j++) { //i is byte
    sigLen = (modIR[j] - modIR[j - 1]); //siglen is byte
    if (sigLen > 50 || sigLen < 10) continue; //this is the period range length exclude extraneous ones
    sum += sigLen; // sum is UL
    countM2++; //countM2 is byte
    modIR[j - 1] = 0; //finished with it so clear for next time
  }
  modIR[j - 1] = 0; //now clear last one, which was missed in loop

  if (countM2 == 0) return; //avoid div by zero = nothing to report
  sum =  sum * 1000 / countM2; //get it in nano secs
  // now send over serial using buffer
  txBuffer[0] = 'M'; //Modulation report is sent as 'M'
  txBuffer[1] = countM2; //number of samples used
  txBuffer[3] = sum >> 8 & 0xFF; //byte Period MSB
  txBuffer[2] = sum & 0xFF; //byte Period LSB
  Serial.write(txBuffer, 5);
  return;
}

//see *** note 14 in README
void rxIR_Interrupt_Handler() { //important to use few instruction cycles here
  //digital pin 2 on Arduino
  newMicros = micros(); //record time stamp for main loop
  state = pin2HIGH; //read changed state of interrupt pin 2 (return 4 or 0 for High/Low)

  toggleInterruptDirection(); //only relevant for Fubarino Mini & other ChipKits, otherwise ignored by compiler
}

//see *** note 15 in README
void rxIR3_Interrupt_Handler() { //important to use few instruction cycles here
  //digital pin 3 on Arduino - FALLING edge only
  modIR[countM++] = micros(); //just continually record the time-stamp, will be mostly modulations
  //just save LSB as we are measuring values of 20-50 uSecs only - so only need a byte (LSB)
}

//see *** note 16 in README
void reportFreeRAM(unsigned int f) { //report freeRam to host if less than f or f is false
#if AVR8BITMCU
  //include this code only for 8 bit AVRs, otherwise ignore
  extern int __heap_start, *__brkval;
  int v;
  int freeRAM = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  if (freeRAM <= f) {
    Serial.print(F("!Free RAM: "));//send line as comment
    Serial.print(freeRAM);
    Serial.println(F("!"));//send line as comment
  }
#endif
}

void toggleInterruptDirection(void) { //this toggles the interrupt edge for Fubarino Mini only, including any ChipKit devices added in future

#if FubarinoMini
  if (intDirection) {
    attachInterrupt(2, rxIR_Interrupt_Handler, RISING);
    intDirection = false;
  }
  else {
    attachInterrupt(2, rxIR_Interrupt_Handler, FALLING);
    intDirection = true;
  }
#endif
}
