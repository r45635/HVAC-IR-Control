
HVAC IR Control

 - Mitsubishi-Inverter-IR-Control
 - Panasonic IR Control
==============================

Ir Send - Updated for HVAC Mitsubishi & Panasanonic IR control

Project:
Controlling Mitsubishi HVAC from IR Led, Through Arduino. -- Owner Vincent
    Confirm ok with MSZ-GE and MFZ modules from Mitsubishi.
Controlling Panasonic HVAC from IR Led, Through Arduino. -- Owner Mathieu
    Models: ...
Contolling Others HVAC brandt: ... You're welcome to collaborate !

Steps:
Catch the IR trame through AnalysiR -- Done.
Identify Trame Timing Specification -- Done.
Identify Packet Data -- Done.
Reverse Engineering to identify Commands Parameters contains in the Packet Date -- Done.
Update Arduino-IRremote for HVAC Mitsubishi Protocol:
  - Trame Specification - Done.
  - HVAC Mitsubishi Send Void : Works with the following:
      - ON/OFF
      - HVAC Mode (Hot, Cold, Dry, Auto)
      - Temperature (Only Verify Hot : +16°c <-> +31°c),
      - FAN Mode (Speed 1, 2, 3, 4, AUTO, SILENT)
      - Vanne Mode (Position 1, 2, 3, 4, 5, AUTO, AUTO_MOVE)
      - CRC Packet implemented,
      - Repeated Packet Send Function implemented,
    Not used: // As we have the control, we don't care about this features
      - Current Clock Parameters Management, 
      - Time Programation Automatic Turn On or Off Function.
      - HVAC Mitsubishi Receive Void.
  
