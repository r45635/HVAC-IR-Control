/*
 * IRremote
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
 * Modified  by Mitra Ardron <mitra@mitra.biz>
 * Added Sanyo and Mitsubishi controllers
 * Modified Sony to spot the repeat codes that some Sony's send
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 * LG added by Darryl Smith (based on the JVC protocol)
 *
 * Mitsubishi HVAC protocol added by Vincent Cruvellier.
 * Mitsubishi HVAC W001CP R61Y23304 protocol added by bt4wang.
 */

#include "IRremote2.h"
#include "IRremoteInt2.h"

// Provides ISR
#include <avr/interrupt.h>

volatile irparams_t irparams;

// These versions of MATCH, MATCH_MARK, and MATCH_SPACE are only for debugging.
// To use them, set DEBUG in IRremoteInt.h
// Normally macros are used for efficiency
#ifdef DEBUG
int MATCH(int measured, int desired) {
  Serial.print("Testing: ");
  Serial.print(TICKS_LOW(desired), DEC);
  Serial.print(" <= ");
  Serial.print(measured, DEC);
  Serial.print(" <= ");
  Serial.println(TICKS_HIGH(desired), DEC);
  return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);
}

int MATCH_MARK(int measured_ticks, int desired_us) {
  Serial.print("Testing mark ");
  Serial.print(measured_ticks * USECPERTICK, DEC);
  Serial.print(" vs ");
  Serial.print(desired_us, DEC);
  Serial.print(": ");
  Serial.print(TICKS_LOW(desired_us + MARK_EXCESS), DEC);
  Serial.print(" <= ");
  Serial.print(measured_ticks, DEC);
  Serial.print(" <= ");
  Serial.println(TICKS_HIGH(desired_us + MARK_EXCESS), DEC);
  return measured_ticks >= TICKS_LOW(desired_us + MARK_EXCESS) && measured_ticks <= TICKS_HIGH(desired_us + MARK_EXCESS);
}

int MATCH_SPACE(int measured_ticks, int desired_us) {
  Serial.print("Testing space ");
  Serial.print(measured_ticks * USECPERTICK, DEC);
  Serial.print(" vs ");
  Serial.print(desired_us, DEC);
  Serial.print(": ");
  Serial.print(TICKS_LOW(desired_us - MARK_EXCESS), DEC);
  Serial.print(" <= ");
  Serial.print(measured_ticks, DEC);
  Serial.print(" <= ");
  Serial.println(TICKS_HIGH(desired_us - MARK_EXCESS), DEC);
  return measured_ticks >= TICKS_LOW(desired_us - MARK_EXCESS) && measured_ticks <= TICKS_HIGH(desired_us - MARK_EXCESS);
}
#else
int MATCH(int measured, int desired) {
  return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);
}
int MATCH_MARK(int measured_ticks, int desired_us) {
  return MATCH(measured_ticks, (desired_us + MARK_EXCESS));
}
int MATCH_SPACE(int measured_ticks, int desired_us) {
  return MATCH(measured_ticks, (desired_us - MARK_EXCESS));
}
// Debugging versions are in IRremote.cpp
#endif



void IRsend::sendNEC(unsigned long data, int nbits)
{
  enableIROut(38);
  mark(NEC_HDR_MARK);
  space(NEC_HDR_SPACE);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(NEC_BIT_MARK);
      space(NEC_ONE_SPACE);
    }
    else {
      mark(NEC_BIT_MARK);
      space(NEC_ZERO_SPACE);
    }
    data <<= 1;
  }
  mark(NEC_BIT_MARK);
  space(0);
}

void IRsend::sendSony(unsigned long data, int nbits) {
  enableIROut(40);
  mark(SONY_HDR_MARK);
  space(SONY_HDR_SPACE);
  data = data << (32 - nbits);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(SONY_ONE_MARK);
      space(SONY_HDR_SPACE);
    }
    else {
      mark(SONY_ZERO_MARK);
      space(SONY_HDR_SPACE);
    }
    data <<= 1;
  }
}

void IRsend::sendRaw(unsigned int buf[], int len, int hz)
{
  enableIROut(hz);
  for (int i = 0; i < len; i++) {
    if (i & 1) {
      space(buf[i]);
    }
    else {
      mark(buf[i]);
    }
  }
  space(0); // Just to be sure
}

// Note: first bit must be a one (start bit)
void IRsend::sendRC5(unsigned long data, int nbits)
{
  enableIROut(36);
  data = data << (32 - nbits);
  mark(RC5_T1); // First start bit
  space(RC5_T1); // Second start bit
  mark(RC5_T1); // Second start bit
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      space(RC5_T1); // 1 is space, then mark
      mark(RC5_T1);
    }
    else {
      mark(RC5_T1);
      space(RC5_T1);
    }
    data <<= 1;
  }
  space(0); // Turn off at end
}

// Caller needs to take care of flipping the toggle bit
void IRsend::sendRC6(unsigned long data, int nbits)
{
  enableIROut(36);
  data = data << (32 - nbits);
  mark(RC6_HDR_MARK);
  space(RC6_HDR_SPACE);
  mark(RC6_T1); // start bit
  space(RC6_T1);
  int t;
  for (int i = 0; i < nbits; i++) {
    if (i == 3) {
      // double-wide trailer bit
      t = 2 * RC6_T1;
    }
    else {
      t = RC6_T1;
    }
    if (data & TOPBIT) {
      mark(t);
      space(t);
    }
    else {
      space(t);
      mark(t);
    }

    data <<= 1;
  }
  space(0); // Turn off at end
}
void IRsend::sendPanasonic(unsigned int address, unsigned long data) {
  enableIROut(35);
  mark(PANASONIC_HDR_MARK);
  space(PANASONIC_HDR_SPACE);

  for (int i = 0; i < 16; i++)
  {
    mark(PANASONIC_BIT_MARK);
    if (address & 0x8000) {
      space(PANASONIC_ONE_SPACE);
    } else {
      space(PANASONIC_ZERO_SPACE);
    }
    address <<= 1;
  }
  for (int i = 0; i < 32; i++) {
    mark(PANASONIC_BIT_MARK);
    if (data & TOPBIT) {
      space(PANASONIC_ONE_SPACE);
    } else {
      space(PANASONIC_ZERO_SPACE);
    }
    data <<= 1;
  }
  mark(PANASONIC_BIT_MARK);
  space(0);
}

/****************************************************************************
/* Send IR command to Panasonic HVAC - sendHvacPanasonic
/***************************************************************************/
void IRsend::sendHvacPanasonic(
  HvacMode        HVAC_Mode,           // Example HVAC_HOT  HvacPanasonicMode
  int             HVAC_Temp,           // Example 21  (°c)
  HvacFanMode     HVAC_FanMode,        // Example FAN_SPEED_AUTO  HvacPanasonicFanMode
  HvacVanneMode   HVAC_VanneMode,      // Example VANNE_AUTO_MOVE  HvacPanasonicVanneMode
  HvacProfileMode HVAC_ProfileMode,	// Example QUIET HvacPanasonicProfileMode
  int             HVAC_SWITCH           // Example false
)
{

#define HVAC_PANASONIC_DEBUG  // Un comment to access DEBUG information through Serial Interface

  byte mask = 1; //our bitmask
  byte data[19] = { 0x02, 0x20, 0xE0, 0x04, 0x00, 0x48, 0x3C, 0x80, 0xAF, 0x00, 0x00, 0x0E, 0xE0, 0x10, 0x00, 0x01, 0x00, 0x06, 0xBE };
  byte dataconst[8] = { 0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06};

  // data array is a valid trame, only byte to be chnaged will be updated.

  byte i;

#ifdef HVAC_PANASONIC_DEBUG
  Serial.println("Basis: ");
  for (i = 0; i < 19; i++) {
    Serial.print("_");
    Serial.print(data[i], HEX);
  }
  Serial.println(".");
#endif

  // Byte 6 - On / Off
  if (HVAC_SWITCH) {
    data[5] = (byte) 0x08; // Switch HVAC Power
  } else {
    data[5] = (byte) 0x09; // Do not switch HVAC Power
  }

  // Byte 6 - Mode
  switch (HVAC_Mode)
  {
    case HVAC_HOT:   data[5] = (byte) data[5] | B01000000; break;
    case HVAC_FAN:   data[5] = (byte) data[5] | B01100000; break;
    case HVAC_COLD:  data[5] = (byte) data[5] | B00011000; break;
    case HVAC_DRY:   data[5] = (byte) data[5] | B00100000; break;
    case HVAC_AUTO:  data[5] = (byte) data[5] | B00000000; break;
    default: break;
  }

  // Byte 7 - Temperature
  // Check Min Max For Hot Mode
  byte Temp;
  if (HVAC_Temp > 30) { Temp = 30;}
  else if (HVAC_Temp < 16) { Temp = 16; } 
  else { Temp = HVAC_Temp; };
  data[6] = (byte) (Temp - 16) <<1;
  data[6] = (byte) data[6] | B00100000;
  //bits used form the temp are [4:1]
  //data|6] = data[6] << 1;
  

  // Byte 9 - FAN / VANNE
  switch (HVAC_FanMode)
  {
    case FAN_SPEED_1:       data[8] = (byte) B00110000; break;
    case FAN_SPEED_2:       data[8] = (byte) B01000000; break;
    case FAN_SPEED_3:       data[8] = (byte) B01010000; break;
    case FAN_SPEED_4:       data[8] = (byte) B01100000; break;
    case FAN_SPEED_5:       data[8] = (byte) B01010000; break;
    case FAN_SPEED_AUTO:    data[8] = (byte) B10100000; break;
    default: break;
  }

  switch (HVAC_VanneMode)
  {
    case VANNE_AUTO:        data[8] = (byte) data[8] | B00001111; break;
    case VANNE_AUTO_MOVE:   data[8] = (byte) data[8] | B00001111; break; //same as AUTO in the PANASONIC CASE
    case VANNE_H1:          data[8] = (byte) data[8] | B00000001; break;
    case VANNE_H2:          data[8] = (byte) data[8] | B00000010; break;
    case VANNE_H3:          data[8] = (byte) data[8] | B00000011; break;
    case VANNE_H4:          data[8] = (byte) data[8] | B00000100; break;
    case VANNE_H5:          data[8] = (byte) data[8] | B00000101; break;
    default: break;
  }

   // Byte 14 - Profile
  switch (HVAC_ProfileMode)
  {
    case NORMAL:        data[13] = (byte) B00010000; break;
    case QUIET:         data[13] = (byte) B01100000; break;
    case BOOST:         data[13] = (byte) B00010001; break;
    default: break;
  }  
  
  
  // Byte 18 - CRC
  data[18] = 0;
  for (i = 0; i < 18; i++) {
    data[18] = (byte) data[i] + data[18];  // CRC is a simple bits addition
  }

#ifdef HVAC_PANASONIC_DEBUG
  Serial.println("Packet to send: ");
  for (i = 0; i < 19; i++) {
    Serial.print("_"); Serial.print(data[i], HEX);
  }
  Serial.println(".");
  for (i = 0; i < 19; i++) {
    Serial.print(data[i], BIN); Serial.print(" ");
  }
  Serial.println(".");
#endif

  enableIROut(38);  // 38khz
  space(0);
  
  //Send constant frame #1
    mark(HVAC_PANASONIC_HDR_MARK);
    space(HVAC_PANASONIC_HDR_SPACE);
    for (i = 0; i < 8; i++) {
      // Send all Bits from Byte dataconst in Reverse Order
      for (mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
        if (dataconst[i] & mask) { // Bit ONE
          mark(HVAC_PANASONIC_BIT_MARK);
          space(HVAC_PANASONIC_ONE_SPACE);
        }
        else { // Bit ZERO
          mark(HVAC_PANASONIC_BIT_MARK);
          space(HVAC_PANASONIC_ZERO_SPACE);
        }
        //Next bits
      }
    }  
     mark(HVAC_PANASONIC_RPT_MARK);
     space(HVAC_PANASONIC_RPT_SPACE);
 
  //Send frame #2  
    mark(HVAC_PANASONIC_HDR_MARK);
    space(HVAC_PANASONIC_HDR_SPACE);
    for (i = 0; i < 19; i++) {
      // Send all Bits from Byte Data in Reverse Order
      for (mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
        if (data[i] & mask) { // Bit ONE
          mark(HVAC_PANASONIC_BIT_MARK);
          space(HVAC_PANASONIC_ONE_SPACE);
        }
        else { // Bit ZERO
          mark(HVAC_PANASONIC_BIT_MARK);
          space(HVAC_PANASONIC_ZERO_SPACE);
        }
        //Next bits
      }
    }
    // End of Packet and retransmission of the Packet
      mark(HVAC_PANASONIC_RPT_MARK);
      space(HVAC_PANASONIC_RPT_SPACE);
       space(0);


}



/****************************************************************************
/* Send IR command to Mitsubishi HVAC - sendHvacMitsubishi
/***************************************************************************/
void IRsend::sendHvacMitsubishi(
  HvacMode                HVAC_Mode,           // Example HVAC_HOT  HvacMitsubishiMode
  int                     HVAC_Temp,           // Example 21  (°c)
  HvacFanMode             HVAC_FanMode,        // Example FAN_SPEED_AUTO  HvacMitsubishiFanMode
  HvacVanneMode           HVAC_VanneMode,      // Example VANNE_AUTO_MOVE  HvacMitsubishiVanneMode
  int                     OFF                  // Example false
)
{

//#define  HVAC_MITSUBISHI_DEBUG;  // Un comment to access DEBUG information through Serial Interface

  byte mask = 1; //our bitmask
  byte data[18] = { 0x23, 0xCB, 0x26, 0x01, 0x00, 0x20, 0x08, 0x06, 0x30, 0x45, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F };
  // data array is a valid trame, only byte to be chnaged will be updated.

  byte i;

#ifdef HVAC_MITSUBISHI_DEBUG
  Serial.println("Packet to send: ");
  for (i = 0; i < 18; i++) {
    Serial.print("_");
    Serial.print(data[i], HEX);
  }
  Serial.println(".");
#endif

  // Byte 6 - On / Off
  if (OFF) {
    data[5] = (byte) 0x0; // Turn OFF HVAC
  } else {
    data[5] = (byte) 0x20; // Tuen ON HVAC
  }

  // Byte 7 - Mode
  switch (HVAC_Mode)
  {
    case HVAC_HOT:   data[6] = (byte) 0x08; break;
    case HVAC_COLD:  data[6] = (byte) 0x18; break;
    case HVAC_DRY:   data[6] = (byte) 0x10; break;
    case HVAC_AUTO:  data[6] = (byte) 0x20; break;
    default: break;
  }

  // Byte 8 - Temperature
  // Check Min Max For Hot Mode
  byte Temp;
  if (HVAC_Temp > 31) { Temp = 31;}
  else if (HVAC_Temp < 16) { Temp = 16; } 
  else { Temp = HVAC_Temp; };
  data[7] = (byte) Temp - 16;

  // Byte 10 - FAN / VANNE
  switch (HVAC_FanMode)
  {
    case FAN_SPEED_1:       data[9] = (byte) B00000001; break;
    case FAN_SPEED_2:       data[9] = (byte) B00000010; break;
    case FAN_SPEED_3:       data[9] = (byte) B00000011; break;
    case FAN_SPEED_4:       data[9] = (byte) B00000100; break;
    case FAN_SPEED_5:       data[9] = (byte) B00000100; break; //No FAN speed 5 for MITSUBISHI so it is consider as Speed 4
    case FAN_SPEED_AUTO:    data[9] = (byte) B10000000; break;
    case FAN_SPEED_SILENT:  data[9] = (byte) B00000101; break;
    default: break;
  }

  switch (HVAC_VanneMode)
  {
    case VANNE_AUTO:        data[9] = (byte) data[9] | B01000000; break;
    case VANNE_H1:          data[9] = (byte) data[9] | B01001000; break;
    case VANNE_H2:          data[9] = (byte) data[9] | B01010000; break;
    case VANNE_H3:          data[9] = (byte) data[9] | B01011000; break;
    case VANNE_H4:          data[9] = (byte) data[9] | B01100000; break;
    case VANNE_H5:          data[9] = (byte) data[9] | B01101000; break;
    case VANNE_AUTO_MOVE:   data[9] = (byte) data[9] | B01111000; break;
    default: break;
  }

  // Byte 18 - CRC
  data[17] = 0;
  for (i = 0; i < 17; i++) {
    data[17] = (byte) data[i] + data[17];  // CRC is a simple bits addition
  }

#ifdef HVAC_MITSUBISHI_DEBUG
  Serial.println("Packet to send: ");
  for (i = 0; i < 18; i++) {
    Serial.print("_"); Serial.print(data[i], HEX);
  }
  Serial.println(".");
  for (i = 0; i < 18; i++) {
    Serial.print(data[i], BIN); Serial.print(" ");
  }
  Serial.println(".");
#endif

  enableIROut(38);  // 38khz
  space(0);
  for (int j = 0; j < 2; j++) {  // For Mitsubishi IR protocol we have to send two time the packet data
    // Header for the Packet
    mark(HVAC_MITSUBISHI_HDR_MARK);
    space(HVAC_MITSUBISHI_HDR_SPACE);
    for (i = 0; i < 18; i++) {
      // Send all Bits from Byte Data in Reverse Order
      for (mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
        if (data[i] & mask) { // Bit ONE
          mark(HVAC_MITSUBISHI_BIT_MARK);
          space(HVAC_MITSUBISHI_ONE_SPACE);
        }
        else { // Bit ZERO
          mark(HVAC_MITSUBISHI_BIT_MARK);
          space(HVAC_MISTUBISHI_ZERO_SPACE);
        }
        //Next bits
      }
    }
    // End of Packet and retransmission of the Packet
    mark(HVAC_MITSUBISHI_RPT_MARK);
    space(HVAC_MITSUBISHI_RPT_SPACE);
    space(0); // Just to be sure
  }
}

/***************************************************************************/
/* Send IR command to Mitsubishi HVAC - sendHvacMitsubishi 
/* Add support for W001CP R61Y23304 Remote Controller
/***************************************************************************/
void IRsend::sendHvacMitsubishi_W001CP(
  HvacMode                  HVAC_Mode,           // Example HVAC_HOT.         HvacMitsubishiMode
                                                     // This type support HVAC_HOT,HVAC_COLD,HVAC_DRY,HVAC_FAN,HVAC_AUTO.
  int                       HVAC_Temp,           // Example 21  (°c).
                                                     // This type support 17~28 in HVAC_HOT mode, 19~30 in HVAC_COLD and HVAC_DRY mode.
  HvacFanMode               HVAC_FanMode,        // Example FAN_SPEED_AUTO.   HvacMitsubishiFanMode
                                                     // This type support FAN_SPEED_1,FAN_SPEED_2,FAN_SPEED_3,FAN_SPEED_4.
  HvacVanneMode             HVAC_VanneMode,      // Example VANNE_AUTO_MOVE.  HvacMitsubishiVanneMode
                                                     // This type support support VANNE_AUTO,VANNE_H1,VANNE_H2,VANNE_H3,VANNE_H4.
  int                       OFF                  // Example false
)
{

//#define  HVAC_MITSUBISHI_DEBUG;  // Un comment to access DEBUG information through Serial Interface

  byte mask = 1; //our bitmask
  byte data[17] = { 0x23, 0xCB, 0x26, 0x21, 0x00, 0x40, 0x52, 0x35, 0x04, 0x00, 0x00, 0xBF, 0xAD, 0xCA, 0xFB, 0xFF, 0xFF };
  // byte              0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16
  // data array is a valid trame, only byte to be chnaged will be updated.

  byte i;

#ifdef HVAC_MITSUBISHI_DEBUG
  Serial.println("Packet to send: ");
  for (i = 0; i < 17; i++) {
    Serial.print("_");
    Serial.print(data[i], HEX);
  }
  Serial.println(".");
#endif

  // Byte 5 - On / Off
  if (OFF) {
    data[5] = (byte) 0x0; // Turn OFF HVAC
  } else {
    data[5] = (byte) 0x40; // Tuen ON HVAC
  }

  // Byte 6 - Temperature / Mode 
  byte tmpTM;
  
  switch (HVAC_Mode)  // Mode
  {
    case HVAC_HOT:   tmpTM = (byte) B00000010; break;
    case HVAC_COLD:  tmpTM = (byte) B00000001; break;
    case HVAC_DRY:   tmpTM = (byte) B00000101; break;
    case HVAC_FAN:   tmpTM = (byte) B00000000; break;
    case HVAC_AUTO:  tmpTM = (byte) B00000011; break;
    default: break;
  }

  byte Temp;  // Temperature, support 17~28 in HVAC_HOT mode, 19~30 in HVAC_COLD and HVAC_DRY mode
  if (HVAC_Temp > 30 && HVAC_Mode != HVAC_HOT) { Temp = 30;}
  else if (HVAC_Temp > 28 && HVAC_Mode == HVAC_HOT) { Temp = 28;}
  else if (HVAC_Temp < 19 && HVAC_Mode != HVAC_HOT) { Temp = 19; } 
  else if (HVAC_Temp < 17 && HVAC_Mode == HVAC_HOT) { Temp = 17; } 
  else { Temp = HVAC_Temp; };
  Temp = (byte) Temp - 16;
  
  data[6] = (byte) Temp * 16 | tmpTM; // Temperature bits are the high 4 bits, Mode bits are the low 4 bits.

  // Byte 7 - VANNE / FAN
  switch (HVAC_FanMode)
  {
    case FAN_SPEED_1:       data[7] = (byte) B00000001; break;
    case FAN_SPEED_2:       data[7] = (byte) B00000011; break;
    case FAN_SPEED_3:       data[7] = (byte) B00000101; break;
    case FAN_SPEED_4:       data[7] = (byte) B00000111; break;
    case FAN_SPEED_5:       data[7] = (byte) B00000111; break;  // this type doesn't support speed5, set to speed4
    case FAN_SPEED_AUTO:    data[7] = (byte) B00000101; break;  // this type doesn't support auto,   set to speed3 
    case FAN_SPEED_SILENT:  data[7] = (byte) B00000001; break;  // this type doesn't support slient, set to speed1 
    default: break;
  }

  switch (HVAC_VanneMode)
  {
    case VANNE_AUTO:        data[7] = (byte) data[7] | B11000000; break;
    case VANNE_H1:          data[7] = (byte) data[7] | B00000000; break;
    case VANNE_H2:          data[7] = (byte) data[7] | B00010000; break;
    case VANNE_H3:          data[7] = (byte) data[7] | B00100000; break;
    case VANNE_H4:          data[7] = (byte) data[7] | B00110000; break;
    case VANNE_H5:          data[7] = (byte) data[7] | B00110000; break;  // this type doesn't support vanne5,   set to vanne4 
    case VANNE_AUTO_MOVE:   data[7] = (byte) data[7] | B11000000; break;  // this type doesn't support automove, set to auto 
    default: break;
  }

  // Byte 11 - XOR of Byte 5
  data[11] = (byte) B11111111 ^ data[5];
  
  // Byte 12 - XOR of Byte 6
  data[12] = (byte) B11111111 ^ data[6];
  
  // Byte 13 - XOR of Byte 7
  data[13] = (byte) B11111111 ^ data[7];

#ifdef HVAC_MITSUBISHI_DEBUG
  Serial.println("Packet to send: ");
  for (i = 0; i < 17; i++) {
    Serial.print("_"); Serial.print(data[i], HEX);
  }
  Serial.println(".");
  for (i = 0; i < 17; i++) {
    Serial.print(data[i], BIN); Serial.print(" ");
  }
  Serial.println(".");
#endif

  enableIROut(38);  // 38khz
  space(0);
  for (int j = 0; j < 1; j++) {  // Mitsubishi W001CP IR protocol only need to send one time
    // Header for the Packet
    mark(HVAC_MITSUBISHI_HDR_MARK);
    space(HVAC_MITSUBISHI_HDR_SPACE);
    for (i = 0; i < 18; i++) {
      // Send all Bits from Byte Data in Reverse Order
      for (mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
        if (data[i] & mask) { // Bit ONE
          mark(HVAC_MITSUBISHI_BIT_MARK);
          space(HVAC_MITSUBISHI_ONE_SPACE);
        }
        else { // Bit ZERO
          mark(HVAC_MITSUBISHI_BIT_MARK);
          space(HVAC_MISTUBISHI_ZERO_SPACE);
        }
        //Next bits
      }
    }
    // End of Packet and retransmission of the Packet
    mark(HVAC_MITSUBISHI_RPT_MARK);
    space(HVAC_MITSUBISHI_RPT_SPACE);
    space(0); // Just to be sure
  }
}

/****************************************************************************
/* Send IR command to Mitsubishi HVAC - sendHvacMitsubishi
/***************************************************************************/
void IRsend::sendHvacMitsubishiFD(
      HvacMode                  HVAC_Mode,           // Example HVAC_HOT  HvacMitsubishiMode
      int                       HVAC_Temp,           // Example 21  (°c)
      HvacFanMode               HVAC_FanMode,        // Example FAN_SPEED_AUTO  HvacMitsubishiFanMode
      HvacVanneMode             HVAC_VanneMode,      // Example VANNE_AUTO_MOVE  HvacMitsubishiVanneMode
      HvacAreaMode              HVAC_AreaMode,       // Example AREA_AUTO
      HvacWideVanneMode         HVAC_WideMode,       // Example WIDE_MIDDLE
      int                      HVAC_PLASMA,          // Example true to Turn ON PLASMA Function
      int                      HVAC_CLEAN_MODE,      // Example false 
      int                      HVAC_ISEE,            // Example False
      int                      OFF                   // Example false to Turn ON HVAC / true to request to turn off
    )
{

//#define  HVAC_MITSUBISHI_DEBUG;  // Un comment to access DEBUG information through Serial Interface

  byte mask = 1; //our bitmask
  byte data[18] = { 0x23, 0xCB, 0x26, 0x01, 0x00, 0x20, 0x08, 0x06, 0x30, 0x45, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F };
  // data array is a valid trame, only byte to be changed will be updated.

  byte i;

#ifdef HVAC_MITSUBISHI_DEBUG
  Serial.println("Packet to send: ");
  for (i = 0; i < 18; i++) {
    Serial.print("_");
    Serial.print(data[i], HEX);
  }
  Serial.println(".");
#endif

  // Byte 5 - On / Off
  if (OFF) {
    data[5] = (byte) 0x0; // Turn OFF HVAC
  } else {
    data[5] = (byte) 0x20; // Turn ON HVAC
  }

  // Byte 6 - Mode
  switch (HVAC_Mode)
  {
    case HVAC_HOT:   data[6] = (byte) B00001000; break;
    case HVAC_COLD:  data[6] = (byte) B00011000; break;
    case HVAC_DRY:   data[6] = (byte) B00010000; break;
    case HVAC_AUTO:  data[6] = (byte) B00100000; break;
    default: break;
  }
  if (HVAC_ISEE)  { data[6] = (byte) data[6] | B01000000;  }

  // Byte 7 - Temperature
  // Check Min Max For Hot Mode
  byte Temp;
  if (HVAC_Temp > 31) { Temp = 31;}
  else if (HVAC_Temp < 16) { Temp = 16; } 
  else { Temp = HVAC_Temp; };
  data[7] = (byte) Temp - 16;

  // Byte 8 - Complement To HVAC Mode + Wide Vanne
  switch (HVAC_Mode)
  {
    case HVAC_HOT:   data[8] = (byte) B00000000; break;
    case HVAC_COLD:  data[8] = (byte) B00000110; break;
    case HVAC_DRY:   data[8] = (byte) B00000010; break;
    case HVAC_AUTO:  data[8] = (byte) B00000000; break;
    default: break;
  }
  switch (HVAC_WideMode)
  {
    case WIDE_LEFT_END:   data[8] = (byte) data[8] | B00010000; break;
    case WIDE_LEFT:       data[8] = (byte) data[8] | B00100000; break;
    case WIDE_MIDDLE:     data[8] = (byte) data[8] | B00110000; break;
    case WIDE_RIGHT:      data[8] = (byte) data[8] | B01000000; break;
    case WIDE_RIGHT_END:  data[8] = (byte) data[8] | B01010000; break;
    case WIDE_SWING:      data[8] = (byte) data[8] | B10000000; break;
    default: break;
  }

  // Byte 9 - FAN / VANNE
  switch (HVAC_FanMode)
  {
    case FAN_SPEED_1:       data[9] = (byte) B00000001; break;
    case FAN_SPEED_2:       data[9] = (byte) B00000010; break;
    case FAN_SPEED_3:       data[9] = (byte) B00000011; break;
    case FAN_SPEED_4:       data[9] = (byte) B00000100; break;
    case FAN_SPEED_5:       data[9] = (byte) B00000100; break; //No FAN speed 5 for MITSUBISHI so it is consider as Speed 4
    case FAN_SPEED_AUTO:    data[9] = (byte) B10000000; break;
    case FAN_SPEED_SILENT:  data[9] = (byte) B00000101; break;
    default: break;
  }
  // Byte 9 
  switch (HVAC_VanneMode)
  {
    case VANNE_AUTO:        data[9] = (byte) data[9] | B01000000; break;
    case VANNE_H1:          data[9] = (byte) data[9] | B01001000; break;
    case VANNE_H2:          data[9] = (byte) data[9] | B01010000; break;
    case VANNE_H3:          data[9] = (byte) data[9] | B01011000; break;
    case VANNE_H4:          data[9] = (byte) data[9] | B01100000; break;
    case VANNE_H5:          data[9] = (byte) data[9] | B01101000; break;
    case VANNE_AUTO_MOVE:   data[9] = (byte) data[9] | B01111000; break;
    default: break;
  }

  // Byte 13 - AREA MODE
  switch (HVAC_AreaMode)
  {
    case AREA_SWING:  data[13] = (byte) B00000000; break;
    case AREA_LEFT:   data[13] = (byte) B01000000; break;
    case AREA_AUTO:   data[13] = (byte) B10000000; break;
    case AREA_RIGHT:  data[13] = (byte) B11000000; break;
    default: break;
  }

  // Byte 14 - CLEAN MODE
  if (HVAC_CLEAN_MODE)  { data[14] = (byte) B00000100;  }

  // Byte 15 - PLASMA
  if (HVAC_PLASMA)  { data[15] = (byte) B00000100;  }

  // Byte 18 - CRC
  data[17] = 0;
  for (i = 0; i < 17; i++) {
    data[17] = (byte) data[i] + data[17];  // CRC is a simple bits addition
  }

#ifdef HVAC_MITSUBISHI_DEBUG
  Serial.println("Packet to send: ");
  for (i = 0; i < 18; i++) {
    Serial.print("_"); Serial.print(data[i], HEX);
  }
  Serial.println(".");
  for (i = 0; i < 18; i++) {
    Serial.print(data[i], BIN); Serial.print(" ");
  }
  Serial.println(".");
#endif

  enableIROut(38);  // 38khz
  space(0);
  for (int j = 0; j < 2; j++) {  // For Mitsubishi IR protocol we have to send two time the packet data
    // Header for the Packet
    mark(HVAC_MITSUBISHI_HDR_MARK);
    space(HVAC_MITSUBISHI_HDR_SPACE);
    for (i = 0; i < 18; i++) {
      // Send all Bits from Byte Data in Reverse Order
      for (mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
        if (data[i] & mask) { // Bit ONE
          mark(HVAC_MITSUBISHI_BIT_MARK);
          space(HVAC_MITSUBISHI_ONE_SPACE);
        }
        else { // Bit ZERO
          mark(HVAC_MITSUBISHI_BIT_MARK);
          space(HVAC_MISTUBISHI_ZERO_SPACE);
        }
        //Next bits
      }
    }
    // End of Packet and retransmission of the Packet
    mark(HVAC_MITSUBISHI_RPT_MARK);
    space(HVAC_MITSUBISHI_RPT_SPACE);
    space(0); // Just to be sure
  }
}


/***************************************************************************/
/* Send IR command to Toshiba HVAC - sendHvacToshiba
/***************************************************************************/
void IRsend::sendHvacToshiba(
  HvacMode                HVAC_Mode,           // Example HVAC_HOT  
  int                     HVAC_Temp,           // Example 21  (°c)
  HvacFanMode             HVAC_FanMode,        // Example FAN_SPEED_AUTO  
  int                     OFF                  // Example false
)
{
 
#define HVAC_TOSHIBA_DATALEN 9
#define  HVAC_TOSHIBA_DEBUG;  // Un comment to access DEBUG information through Serial Interface

  byte mask = 1; //our bitmask
  //﻿F20D03FC0150000051
  byte data[HVAC_TOSHIBA_DATALEN] = { 0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0x00, 0x00, 0x00 };
  // data array is a valid trame, only byte to be chnaged will be updated.

  byte i;

#ifdef HVAC_TOSHIBA_DEBUG
  Serial.println("Packet to send: ");
  for (i = 0; i < HVAC_TOSHIBA_DATALEN; i++) {
    Serial.print("_");
    Serial.print(data[i], HEX);
  }
  Serial.println(".");
#endif

  data[6] = 0x00;
  // Byte 7 - Mode
  switch (HVAC_Mode)
  {
    case HVAC_HOT:   data[6] = (byte) B00000011; break;
    case HVAC_COLD:  data[6] = (byte) B00000001; break;
    case HVAC_DRY:   data[6] = (byte) B00000010; break;
    case HVAC_AUTO:  data[6] = (byte) B00000000; break;
    default: break;
  }


  // Byte 7 - On / Off
  if (OFF) {
    data[6] = (byte) 0x07; // Turn OFF HVAC
  } else {
     // Turn ON HVAC (default)
  }

  // Byte 6 - Temperature
  // Check Min Max For Hot Mode
  byte Temp;
  if (HVAC_Temp > 30) { Temp = 30;}
  else if (HVAC_Temp < 17) { Temp = 17; } 
  else { Temp = HVAC_Temp; };
  data[5] = (byte) Temp - 17<<4;

  // Byte 10 - FAN / VANNE
  switch (HVAC_FanMode)
  {
    case FAN_SPEED_1:       data[6] = data[6] | (byte) B01000000; break;
    case FAN_SPEED_2:       data[6] = data[6] | (byte) B01100000; break;
    case FAN_SPEED_3:       data[6] = data[6] | (byte) B10000000; break;
    case FAN_SPEED_4:       data[6] = data[6] | (byte) B10100000; break;
    case FAN_SPEED_5:       data[6] = data[6] | (byte) B11000000; break; 
    case FAN_SPEED_AUTO:    data[6] = data[6] | (byte) B00000000; break;
    case FAN_SPEED_SILENT:  data[6] = data[6] | (byte) B00000000; break;//No FAN speed SILENT for TOSHIBA so it is consider as Speed AUTO
    default: break;
  }

  // Byte 9 - CRC
  data[8] = 0;
  for (i = 0; i < HVAC_TOSHIBA_DATALEN - 1; i++) {
    data[HVAC_TOSHIBA_DATALEN-1] = (byte) data[i] ^ data[HVAC_TOSHIBA_DATALEN -1];  // CRC is a simple bits addition
  }

#ifdef HVAC_TOSHIBA_DEBUG
  Serial.println("Packet to send: ");
  for (i = 0; i < HVAC_TOSHIBA_DATALEN; i++) {
    Serial.print("_"); Serial.print(data[i], HEX);
  }
  Serial.println(".");
  for (i = 0; i < HVAC_TOSHIBA_DATALEN ; i++) {
    Serial.print(data[i], BIN); Serial.print(" ");
  }
  Serial.println(".");
#endif

  enableIROut(38);  // 38khz
  space(0);
  for (int j = 0; j < 2; j++) {  // For Mitsubishi IR protocol we have to send two time the packet data
    // Header for the Packet
    mark(HVAC_TOSHIBA_HDR_MARK);
    space(HVAC_TOSHIBA_HDR_SPACE);
    for (i = 0; i < HVAC_TOSHIBA_DATALEN; i++) {
      // Send all Bits from Byte Data in Reverse Order
      for (mask = 10000000; mask > 0; mask >>= 1) { //iterate through bit mask
        if (data[i] & mask) { // Bit ONE
          mark(HVAC_TOSHIBA_BIT_MARK);
          space(HVAC_TOSHIBA_ONE_SPACE);
        }
        else { // Bit ZERO
          mark(HVAC_TOSHIBA_BIT_MARK);
          space(HVAC_MISTUBISHI_ZERO_SPACE);
        }
        //Next bits
      }
    }
    // End of Packet and retransmission of the Packet
    mark(HVAC_TOSHIBA_RPT_MARK);
    space(HVAC_TOSHIBA_RPT_SPACE);
    space(0); // Just to be sure
  }
}



void IRsend::sendJVC(unsigned long data, int nbits, int repeat)
{
  enableIROut(38);
  data = data << (32 - nbits);
  if (!repeat) {
    mark(JVC_HDR_MARK);
    space(JVC_HDR_SPACE);
  }
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(JVC_BIT_MARK);
      space(JVC_ONE_SPACE);
    }
    else {
      mark(JVC_BIT_MARK);
      space(JVC_ZERO_SPACE);
    }
    data <<= 1;
  }
  mark(JVC_BIT_MARK);
  space(0);
}

void IRsend::sendSAMSUNG(unsigned long data, int nbits)
{
  enableIROut(38);
  mark(SAMSUNG_HDR_MARK);
  space(SAMSUNG_HDR_SPACE);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(SAMSUNG_BIT_MARK);
      space(SAMSUNG_ONE_SPACE);
    }
    else {
      mark(SAMSUNG_BIT_MARK);
      space(SAMSUNG_ZERO_SPACE);
    }
    data <<= 1;
  }
  mark(SAMSUNG_BIT_MARK);
  space(0);
}

void IRsend::mark(int time) {
  // Sends an IR mark for the specified number of microseconds.
  // The mark output is modulated at the PWM frequency.
  TIMER_ENABLE_PWM; // Enable pin 3 PWM output
  //delayMicroseconds(time);
  // Use IRlib containment for delay in µsecond
  if(time){if(time>16000) {delayMicroseconds(time % 1000); delay(time/1000); } else delayMicroseconds(time);};
}

/* Leave pin off for time (given in microseconds) */
void IRsend::space(int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  TIMER_DISABLE_PWM; // Disable pin 3 PWM output
  //delayMicroseconds(time);
  // Use IRlib containment for delay in µsecond  
  if(time){if(time>16000) {delayMicroseconds(time % 1000); delay(time/1000); } else delayMicroseconds(time);};
}

void IRsend::enableIROut(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  // The IR output will be on pin 3 (OC2B).
  // This routine is designed for 36-40KHz; if you use it for other values, it's up to you
  // to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
  // TIMER2 is used in phase-correct PWM mode, with OCR2A controlling the frequency and OCR2B
  // controlling the duty cycle.
  // There is no prescaling, so the output frequency is 16MHz / (2 * OCR2A)
  // To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
  // A few hours staring at the ATmega documentation and this will all make sense.
  // See my Secrets of Arduino PWM at http://arcfn.com/2009/07/secrets-of-arduino-pwm.html for details.


  // Disable the Timer2 Interrupt (which is used for receiving IR)
  TIMER_DISABLE_INTR; //Timer2 Overflow Interrupt

  pinMode(TIMER_PWM_PIN, OUTPUT);
  digitalWrite(TIMER_PWM_PIN, LOW); // When not sending PWM, we want it low

  // COM2A = 00: disconnect OC2A
  // COM2B = 00: disconnect OC2B; to send signal set to 10: OC2B non-inverted
  // WGM2 = 101: phase-correct PWM with OCRA as top
  // CS2 = 000: no prescaling
  // The top value for the timer.  The modulation frequency will be SYSCLOCK / 2 / OCR2A.
  TIMER_CONFIG_KHZ(khz);
}

IRrecv::IRrecv(int recvpin)
{
  irparams.recvpin = recvpin;
  irparams.blinkflag = 0;
}

// initialization
void IRrecv::enableIRIn() {
  cli();
  // setup pulse clock timer interrupt
  //Prescale /8 (16M/8 = 0.5 microseconds per tick)
  // Therefore, the timer interval can range from 0.5 to 128 microseconds
  // depending on the reset value (255 to 0)
  TIMER_CONFIG_NORMAL();

  //Timer2 Overflow Interrupt Enable
  TIMER_ENABLE_INTR;

  TIMER_RESET;

  sei();  // enable interrupts

  // initialize state machine variables
  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen = 0;

  // set pin modes
  pinMode(irparams.recvpin, INPUT);
}

// enable/disable blinking of pin 13 on IR processing
void IRrecv::blink13(int blinkflag)
{
  irparams.blinkflag = blinkflag;
  if (blinkflag)
    pinMode(BLINKLED, OUTPUT);
}

// TIMER2 interrupt code to collect raw data.
// Widths of alternating SPACE, MARK are recorded in rawbuf.
// Recorded in ticks of 50 microseconds.
// rawlen counts the number of entries recorded so far.
// First entry is the SPACE between transmissions.
// As soon as a SPACE gets long, ready is set, state switches to IDLE, timing of SPACE continues.
// As soon as first MARK arrives, gap width is recorded, ready is cleared, and new logging starts
ISR(TIMER_INTR_NAME)
{
  TIMER_RESET;

  uint8_t irdata = (uint8_t)digitalRead(irparams.recvpin);

  irparams.timer++; // One more 50us tick
  if (irparams.rawlen >= RAWBUF) {
    // Buffer overflow
    irparams.rcvstate = STATE_STOP;
  }
  switch (irparams.rcvstate) {
    case STATE_IDLE: // In the middle of a gap
      if (irdata == MARK) {
        if (irparams.timer < GAP_TICKS) {
          // Not big enough to be a gap.
          irparams.timer = 0;
        }
        else {
          // gap just ended, record duration and start recording transmission
          irparams.rawlen = 0;
          irparams.rawbuf[irparams.rawlen++] = irparams.timer;
          irparams.timer = 0;
          irparams.rcvstate = STATE_MARK;
        }
      }
      break;
    case STATE_MARK: // timing MARK
      if (irdata == SPACE) {   // MARK ended, record time
        irparams.rawbuf[irparams.rawlen++] = irparams.timer;
        irparams.timer = 0;
        irparams.rcvstate = STATE_SPACE;
      }
      break;
    case STATE_SPACE: // timing SPACE
      if (irdata == MARK) { // SPACE just ended, record it
        irparams.rawbuf[irparams.rawlen++] = irparams.timer;
        irparams.timer = 0;
        irparams.rcvstate = STATE_MARK;
      }
      else { // SPACE
        if (irparams.timer > GAP_TICKS) {
          // big SPACE, indicates gap between codes
          // Mark current code as ready for processing
          // Switch to STOP
          // Don't reset timer; keep counting space width
          irparams.rcvstate = STATE_STOP;
        }
      }
      break;
    case STATE_STOP: // waiting, measuring gap
      if (irdata == MARK) { // reset gap timer
        irparams.timer = 0;
      }
      break;
  }

  if (irparams.blinkflag) {
    if (irdata == MARK) {
      BLINKLED_ON();  // turn pin 13 LED on
    }
    else {
      BLINKLED_OFF();  // turn pin 13 LED off
    }
  }
}

void IRrecv::resume() {
  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen = 0;
}



// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
int IRrecv::decode(decode_results *results) {
  results->rawbuf = irparams.rawbuf;
  results->rawlen = irparams.rawlen;
  if (irparams.rcvstate != STATE_STOP) {
    return ERR;
  }
#ifdef DEBUG
  Serial.println("Attempting NEC decode");
#endif
  if (decodeNEC(results)) {
    return DECODED;
  }
#ifdef DEBUG
  Serial.println("Attempting Sony decode");
#endif
  if (decodeSony(results)) {
    return DECODED;
  }
#ifdef DEBUG
  Serial.println("Attempting Sanyo decode");
#endif
  if (decodeSanyo(results)) {
    return DECODED;
  }
#ifdef DEBUG
  Serial.println("Attempting Mitsubishi decode");
#endif
  if (decodeMitsubishi(results)) {
    return DECODED;
  }
#ifdef DEBUG
  Serial.println("Attempting RC5 decode");
#endif
  if (decodeRC5(results)) {
    return DECODED;
  }
#ifdef DEBUG
  Serial.println("Attempting RC6 decode");
#endif
  if (decodeRC6(results)) {
    return DECODED;
  }
#ifdef DEBUG
  Serial.println("Attempting Panasonic decode");
#endif
  if (decodePanasonic(results)) {
    return DECODED;
  }
#ifdef DEBUG
  Serial.println("Attempting LG decode");
#endif
  if (decodeLG(results)) {
    return DECODED;
  }
#ifdef DEBUG
  Serial.println("Attempting JVC decode");
#endif
  if (decodeJVC(results)) {
    return DECODED;
  }
#ifdef DEBUG
  Serial.println("Attempting SAMSUNG decode");
#endif
  if (decodeSAMSUNG(results)) {
    return DECODED;
  }
  // decodeHash returns a hash on any input.
  // Thus, it needs to be last in the list.
  // If you add any decodes, add them before this.
  if (decodeHash(results)) {
    return DECODED;
  }
  // Throw away and start over
  resume();
  return ERR;
}

// NECs have a repeat only 4 items long
long IRrecv::decodeNEC(decode_results *results) {
  long data = 0;
  int offset = 1; // Skip first space
  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], NEC_HDR_MARK)) {
    return ERR;
  }
  offset++;
  // Check for repeat
  if (irparams.rawlen == 4 &&
      MATCH_SPACE(results->rawbuf[offset], NEC_RPT_SPACE) &&
      MATCH_MARK(results->rawbuf[offset + 1], NEC_BIT_MARK)) {
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = NEC;
    return DECODED;
  }
  if (irparams.rawlen < 2 * NEC_BITS + 4) {
    return ERR;
  }
  // Initial space
  if (!MATCH_SPACE(results->rawbuf[offset], NEC_HDR_SPACE)) {
    return ERR;
  }
  offset++;
  for (int i = 0; i < NEC_BITS; i++) {
    if (!MATCH_MARK(results->rawbuf[offset], NEC_BIT_MARK)) {
      return ERR;
    }
    offset++;
    if (MATCH_SPACE(results->rawbuf[offset], NEC_ONE_SPACE)) {
      data = (data << 1) | 1;
    }
    else if (MATCH_SPACE(results->rawbuf[offset], NEC_ZERO_SPACE)) {
      data <<= 1;
    }
    else {
      return ERR;
    }
    offset++;
  }
  // Success
  results->bits = NEC_BITS;
  results->value = data;
  results->decode_type = NEC;
  return DECODED;
}

long IRrecv::decodeSony(decode_results *results) {
  long data = 0;
  if (irparams.rawlen < 2 * SONY_BITS + 2) {
    return ERR;
  }
  int offset = 0; // Dont skip first space, check its size

  // Some Sony's deliver repeats fast after first
  // unfortunately can't spot difference from of repeat from two fast clicks
  if (results->rawbuf[offset] < SONY_DOUBLE_SPACE_USECS) {
    // Serial.print("IR Gap found: ");
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = SANYO;
    return DECODED;
  }
  offset++;

  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], SONY_HDR_MARK)) {
    return ERR;
  }
  offset++;

  while (offset + 1 < irparams.rawlen) {
    if (!MATCH_SPACE(results->rawbuf[offset], SONY_HDR_SPACE)) {
      break;
    }
    offset++;
    if (MATCH_MARK(results->rawbuf[offset], SONY_ONE_MARK)) {
      data = (data << 1) | 1;
    }
    else if (MATCH_MARK(results->rawbuf[offset], SONY_ZERO_MARK)) {
      data <<= 1;
    }
    else {
      return ERR;
    }
    offset++;
  }

  // Success
  results->bits = (offset - 1) / 2;
  if (results->bits < 12) {
    results->bits = 0;
    return ERR;
  }
  results->value = data;
  results->decode_type = SONY;
  return DECODED;
}

// I think this is a Sanyo decoder - serial = SA 8650B
// Looks like Sony except for timings, 48 chars of data and time/space different
long IRrecv::decodeSanyo(decode_results *results) {
  long data = 0;
  if (irparams.rawlen < 2 * SANYO_BITS + 2) {
    return ERR;
  }
  int offset = 0; // Skip first space
  // Initial space
  /* Put this back in for debugging - note can't use #DEBUG as if Debug on we don't see the repeat cos of the delay
  Serial.print("IR Gap: ");
  Serial.println( results->rawbuf[offset]);
  Serial.println( "test against:");
  Serial.println(results->rawbuf[offset]);
  */
  if (results->rawbuf[offset] < SANYO_DOUBLE_SPACE_USECS) {
    // Serial.print("IR Gap found: ");
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = SANYO;
    return DECODED;
  }
  offset++;

  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], SANYO_HDR_MARK)) {
    return ERR;
  }
  offset++;

  // Skip Second Mark
  if (!MATCH_MARK(results->rawbuf[offset], SANYO_HDR_MARK)) {
    return ERR;
  }
  offset++;

  while (offset + 1 < irparams.rawlen) {
    if (!MATCH_SPACE(results->rawbuf[offset], SANYO_HDR_SPACE)) {
      break;
    }
    offset++;
    if (MATCH_MARK(results->rawbuf[offset], SANYO_ONE_MARK)) {
      data = (data << 1) | 1;
    }
    else if (MATCH_MARK(results->rawbuf[offset], SANYO_ZERO_MARK)) {
      data <<= 1;
    }
    else {
      return ERR;
    }
    offset++;
  }

  // Success
  results->bits = (offset - 1) / 2;
  if (results->bits < 12) {
    results->bits = 0;
    return ERR;
  }
  results->value = data;
  results->decode_type = SANYO;
  return DECODED;
}

// Looks like Sony except for timings, 48 chars of data and time/space different
long IRrecv::decodeMitsubishi(decode_results *results) {
  // Serial.print("?!? decoding Mitsubishi:");Serial.print(irparams.rawlen); Serial.print(" want "); Serial.println( 2 * MITSUBISHI_BITS + 2);
  long data = 0;
  if (irparams.rawlen < 2 * MITSUBISHI_BITS + 2) {
    return ERR;
  }
  int offset = 0; // Skip first space
  // Initial space
  /* Put this back in for debugging - note can't use #DEBUG as if Debug on we don't see the repeat cos of the delay
  Serial.print("IR Gap: ");
  Serial.println( results->rawbuf[offset]);
  Serial.println( "test against:");
  Serial.println(results->rawbuf[offset]);
  */
  /* Not seeing double keys from Mitsubishi
  if (results->rawbuf[offset] < MITSUBISHI_DOUBLE_SPACE_USECS) {
    // Serial.print("IR Gap found: ");
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = MITSUBISHI;
    return DECODED;
  }
  */
  offset++;

  // Typical
  // 14200 7 41 7 42 7 42 7 17 7 17 7 18 7 41 7 18 7 17 7 17 7 18 7 41 8 17 7 17 7 18 7 17 7

  // Initial Space
  if (!MATCH_MARK(results->rawbuf[offset], MITSUBISHI_HDR_SPACE)) {
    return ERR;
  }
  offset++;
  while (offset + 1 < irparams.rawlen) {
    if (MATCH_MARK(results->rawbuf[offset], MITSUBISHI_ONE_MARK)) {
      data = (data << 1) | 1;
    }
    else if (MATCH_MARK(results->rawbuf[offset], MITSUBISHI_ZERO_MARK)) {
      data <<= 1;
    }
    else {
      // Serial.println("A"); Serial.println(offset); Serial.println(results->rawbuf[offset]);
      return ERR;
    }
    offset++;
    if (!MATCH_SPACE(results->rawbuf[offset], MITSUBISHI_HDR_SPACE)) {
      // Serial.println("B"); Serial.println(offset); Serial.println(results->rawbuf[offset]);
      break;
    }
    offset++;
  }

  // Success
  results->bits = (offset - 1) / 2;
  if (results->bits < MITSUBISHI_BITS) {
    results->bits = 0;
    return ERR;
  }
  results->value = data;
  results->decode_type = MITSUBISHI;
  return DECODED;
}


// Gets one undecoded level at a time from the raw buffer.
// The RC5/6 decoding is easier if the data is broken into time intervals.
// E.g. if the buffer has MARK for 2 time intervals and SPACE for 1,
// successive calls to getRClevel will return MARK, MARK, SPACE.
// offset and used are updated to keep track of the current position.
// t1 is the time interval for a single bit in microseconds.
// Returns -1 for error (measured time interval is not a multiple of t1).
int IRrecv::getRClevel(decode_results *results, int *offset, int *used, int t1) {
  if (*offset >= results->rawlen) {
    // After end of recorded buffer, assume SPACE.
    return SPACE;
  }
  int width = results->rawbuf[*offset];
  int val = ((*offset) % 2) ? MARK : SPACE;
  int correction = (val == MARK) ? MARK_EXCESS : - MARK_EXCESS;

  int avail;
  if (MATCH(width, t1 + correction)) {
    avail = 1;
  }
  else if (MATCH(width, 2 * t1 + correction)) {
    avail = 2;
  }
  else if (MATCH(width, 3 * t1 + correction)) {
    avail = 3;
  }
  else {
    return -1;
  }

  (*used)++;
  if (*used >= avail) {
    *used = 0;
    (*offset)++;
  }
#ifdef DEBUG
  if (val == MARK) {
    Serial.println("MARK");
  }
  else {
    Serial.println("SPACE");
  }
#endif
  return val;
}

long IRrecv::decodeRC5(decode_results *results) {
  if (irparams.rawlen < MIN_RC5_SAMPLES + 2) {
    return ERR;
  }
  int offset = 1; // Skip gap space
  long data = 0;
  int used = 0;
  // Get start bits
  if (getRClevel(results, &offset, &used, RC5_T1) != MARK) return ERR;
  if (getRClevel(results, &offset, &used, RC5_T1) != SPACE) return ERR;
  if (getRClevel(results, &offset, &used, RC5_T1) != MARK) return ERR;
  int nbits;
  for (nbits = 0; offset < irparams.rawlen; nbits++) {
    int levelA = getRClevel(results, &offset, &used, RC5_T1);
    int levelB = getRClevel(results, &offset, &used, RC5_T1);
    if (levelA == SPACE && levelB == MARK) {
      // 1 bit
      data = (data << 1) | 1;
    }
    else if (levelA == MARK && levelB == SPACE) {
      // zero bit
      data <<= 1;
    }
    else {
      return ERR;
    }
  }

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = RC5;
  return DECODED;
}

long IRrecv::decodeRC6(decode_results *results) {
  if (results->rawlen < MIN_RC6_SAMPLES) {
    return ERR;
  }
  int offset = 1; // Skip first space
  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], RC6_HDR_MARK)) {
    return ERR;
  }
  offset++;
  if (!MATCH_SPACE(results->rawbuf[offset], RC6_HDR_SPACE)) {
    return ERR;
  }
  offset++;
  long data = 0;
  int used = 0;
  // Get start bit (1)
  if (getRClevel(results, &offset, &used, RC6_T1) != MARK) return ERR;
  if (getRClevel(results, &offset, &used, RC6_T1) != SPACE) return ERR;
  int nbits;
  for (nbits = 0; offset < results->rawlen; nbits++) {
    int levelA, levelB; // Next two levels
    levelA = getRClevel(results, &offset, &used, RC6_T1);
    if (nbits == 3) {
      // T bit is double wide; make sure second half matches
      if (levelA != getRClevel(results, &offset, &used, RC6_T1)) return ERR;
    }
    levelB = getRClevel(results, &offset, &used, RC6_T1);
    if (nbits == 3) {
      // T bit is double wide; make sure second half matches
      if (levelB != getRClevel(results, &offset, &used, RC6_T1)) return ERR;
    }
    if (levelA == MARK && levelB == SPACE) { // reversed compared to RC5
      // 1 bit
      data = (data << 1) | 1;
    }
    else if (levelA == SPACE && levelB == MARK) {
      // zero bit
      data <<= 1;
    }
    else {
      return ERR; // Error
    }
  }
  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = RC6;
  return DECODED;
}
long IRrecv::decodePanasonic(decode_results *results) {
  unsigned long long data = 0;
  int offset = 1;

  if (!MATCH_MARK(results->rawbuf[offset], PANASONIC_HDR_MARK)) {
    return ERR;
  }
  offset++;
  if (!MATCH_MARK(results->rawbuf[offset], PANASONIC_HDR_SPACE)) {
    return ERR;
  }
  offset++;

  // decode address
  for (int i = 0; i < PANASONIC_BITS; i++) {
    if (!MATCH_MARK(results->rawbuf[offset++], PANASONIC_BIT_MARK)) {
      return ERR;
    }
    if (MATCH_SPACE(results->rawbuf[offset], PANASONIC_ONE_SPACE)) {
      data = (data << 1) | 1;
    } else if (MATCH_SPACE(results->rawbuf[offset], PANASONIC_ZERO_SPACE)) {
      data <<= 1;
    } else {
      return ERR;
    }
    offset++;
  }
  results->value = (unsigned long)data;
  results->panasonicAddress = (unsigned int)(data >> 32);
  results->decode_type = PANASONIC;
  results->bits = PANASONIC_BITS;
  return DECODED;
}

long IRrecv::decodeLG(decode_results *results) {
  long data = 0;
  int offset = 1; // Skip first space

  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], LG_HDR_MARK)) {
    return ERR;
  }
  offset++;
  if (irparams.rawlen < 2 * LG_BITS + 1 ) {
    return ERR;
  }
  // Initial space
  if (!MATCH_SPACE(results->rawbuf[offset], LG_HDR_SPACE)) {
    return ERR;
  }
  offset++;
  for (int i = 0; i < LG_BITS; i++) {
    if (!MATCH_MARK(results->rawbuf[offset], LG_BIT_MARK)) {
      return ERR;
    }
    offset++;
    if (MATCH_SPACE(results->rawbuf[offset], LG_ONE_SPACE)) {
      data = (data << 1) | 1;
    }
    else if (MATCH_SPACE(results->rawbuf[offset], LG_ZERO_SPACE)) {
      data <<= 1;
    }
    else {
      return ERR;
    }
    offset++;
  }
  //Stop bit
  if (!MATCH_MARK(results->rawbuf[offset], LG_BIT_MARK)) {
    return ERR;
  }
  // Success
  results->bits = LG_BITS;
  results->value = data;
  results->decode_type = LG;
  return DECODED;
}


long IRrecv::decodeJVC(decode_results *results) {
  long data = 0;
  int offset = 1; // Skip first space
  // Check for repeat
  if (irparams.rawlen - 1 == 33 &&
      MATCH_MARK(results->rawbuf[offset], JVC_BIT_MARK) &&
      MATCH_MARK(results->rawbuf[irparams.rawlen - 1], JVC_BIT_MARK)) {
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = JVC;
    return DECODED;
  }
  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], JVC_HDR_MARK)) {
    return ERR;
  }
  offset++;
  if (irparams.rawlen < 2 * JVC_BITS + 1 ) {
    return ERR;
  }
  // Initial space
  if (!MATCH_SPACE(results->rawbuf[offset], JVC_HDR_SPACE)) {
    return ERR;
  }
  offset++;
  for (int i = 0; i < JVC_BITS; i++) {
    if (!MATCH_MARK(results->rawbuf[offset], JVC_BIT_MARK)) {
      return ERR;
    }
    offset++;
    if (MATCH_SPACE(results->rawbuf[offset], JVC_ONE_SPACE)) {
      data = (data << 1) | 1;
    }
    else if (MATCH_SPACE(results->rawbuf[offset], JVC_ZERO_SPACE)) {
      data <<= 1;
    }
    else {
      return ERR;
    }
    offset++;
  }
  //Stop bit
  if (!MATCH_MARK(results->rawbuf[offset], JVC_BIT_MARK)) {
    return ERR;
  }
  // Success
  results->bits = JVC_BITS;
  results->value = data;
  results->decode_type = JVC;
  return DECODED;
}

// SAMSUNGs have a repeat only 4 items long
long IRrecv::decodeSAMSUNG(decode_results *results) {
  long data = 0;
  int offset = 1; // Skip first space
  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], SAMSUNG_HDR_MARK)) {
    return ERR;
  }
  offset++;
  // Check for repeat
  if (irparams.rawlen == 4 &&
      MATCH_SPACE(results->rawbuf[offset], SAMSUNG_RPT_SPACE) &&
      MATCH_MARK(results->rawbuf[offset + 1], SAMSUNG_BIT_MARK)) {
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = SAMSUNG;
    return DECODED;
  }
  if (irparams.rawlen < 2 * SAMSUNG_BITS + 4) {
    return ERR;
  }
  // Initial space
  if (!MATCH_SPACE(results->rawbuf[offset], SAMSUNG_HDR_SPACE)) {
    return ERR;
  }
  offset++;
  for (int i = 0; i < SAMSUNG_BITS; i++) {
    if (!MATCH_MARK(results->rawbuf[offset], SAMSUNG_BIT_MARK)) {
      return ERR;
    }
    offset++;
    if (MATCH_SPACE(results->rawbuf[offset], SAMSUNG_ONE_SPACE)) {
      data = (data << 1) | 1;
    }
    else if (MATCH_SPACE(results->rawbuf[offset], SAMSUNG_ZERO_SPACE)) {
      data <<= 1;
    }
    else {
      return ERR;
    }
    offset++;
  }
  // Success
  results->bits = SAMSUNG_BITS;
  results->value = data;
  results->decode_type = SAMSUNG;
  return DECODED;
}

/* -----------------------------------------------------------------------
 * hashdecode - decode an arbitrary IR code.
 * Instead of decoding using a standard encoding scheme
 * (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
 *
 * The algorithm: look at the sequence of MARK signals, and see if each one
 * is shorter (0), the same length (1), or longer (2) than the previous.
 * Do the same with the SPACE signals.  Hszh the resulting sequence of 0's,
 * 1's, and 2's to a 32-bit value.  This will give a unique value for each
 * different code (probably), for most code systems.
 *
 * http://arcfn.com/2010/01/using-arbitrary-remotes-with-arduino.html
 */

// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
// Use a tolerance of 20%
int IRrecv::compare(unsigned int oldval, unsigned int newval) {
  if (newval < oldval * .8) {
    return 0;
  }
  else if (oldval < newval * .8) {
    return 2;
  }
  else {
    return 1;
  }
}

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261

/* Converts the raw code values into a 32-bit hash code.
 * Hopefully this code is unique for each button.
 * This isn't a "real" decoding, just an arbitrary value.
 */
long IRrecv::decodeHash(decode_results *results) {
  // Require at least 6 samples to prevent triggering on noise
  if (results->rawlen < 6) {
    return ERR;
  }
  long hash = FNV_BASIS_32;
  for (int i = 1; i + 2 < results->rawlen; i++) {
    int value =  compare(results->rawbuf[i], results->rawbuf[i + 2]);
    // Add value into the hash
    hash = (hash * FNV_PRIME_32) ^ value;
  }
  results->value = hash;
  results->bits = 32;
  results->decode_type = UNKNOWN;
  return DECODED;
}

/* Sharp and DISH support by Todd Treece ( http://unionbridge.org/design/ircommand )

The Dish send function needs to be repeated 4 times, and the Sharp function
has the necessary repeat built in because of the need to invert the signal.

Sharp protocol documentation:
http://www.sbprojects.com/knowledge/ir/sharp.htm

Here are the LIRC files that I found that seem to match the remote codes
from the oscilloscope:

Sharp LCD TV:
http://lirc.sourceforge.net/remotes/sharp/GA538WJSA

DISH NETWORK (echostar 301):
http://lirc.sourceforge.net/remotes/echostar/301_501_3100_5100_58xx_59xx

For the DISH codes, only send the last for characters of the hex.
i.e. use 0x1C10 instead of 0x0000000000001C10 which is listed in the
linked LIRC file.
*/

void IRsend::sendSharpRaw(unsigned long data, int nbits) {
  enableIROut(38);

  // Sending codes in bursts of 3 (normal, inverted, normal) makes transmission
  // much more reliable. That's the exact behaviour of CD-S6470 remote control.
  for (int n = 0; n < 3; n++) {
    for (int i = 1 << (nbits - 1); i > 0; i >>= 1) {
      if (data & i) {
        mark(SHARP_BIT_MARK);
        space(SHARP_ONE_SPACE);
      }
      else {
        mark(SHARP_BIT_MARK);
        space(SHARP_ZERO_SPACE);
      }
    }

    mark(SHARP_BIT_MARK);
    space(SHARP_ZERO_SPACE);
    delay(40);

    data = data ^ SHARP_TOGGLE_MASK;
  }
}

// Sharp send compatible with data obtained through decodeSharp
void IRsend::sendSharp(unsigned int address, unsigned int command) {
  sendSharpRaw((address << 10) | (command << 2) | 2, 15);
}

void IRsend::sendDISH(unsigned long data, int nbits) {
  enableIROut(56);
  mark(DISH_HDR_MARK);
  space(DISH_HDR_SPACE);
  for (int i = 0; i < nbits; i++) {
    if (data & DISH_TOP_BIT) {
      mark(DISH_BIT_MARK);
      space(DISH_ONE_SPACE);
    }
    else {
      mark(DISH_BIT_MARK);
      space(DISH_ZERO_SPACE);
    }
    data <<= 1;
  }
}
