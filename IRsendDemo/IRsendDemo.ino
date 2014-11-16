/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include "IRremote2.h"

// TX PIN 9

IRsend irsend;

void setup()
{
  Serial.begin(9600);
}

void loop() {
  
  byte data[18] = { 0x23, 0xCB, 0x26, 0x01, 0x00, 0x20, 0x08, 0x06, 0x30, 0x45, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F };

  //Serial.println("Enter cmd to send.");
  if (Serial.read() != -1) {
    for (int i = 0; i < 3; i++) {
      //irsend.sendSony(0xa90, 12); // Sony TV power code
      irsend.sendHvacMitsubishi();
      Serial.println("Command sent");
      delay(4000);
    }
  }
}
