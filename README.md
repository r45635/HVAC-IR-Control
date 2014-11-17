Mitsubishi-Inverter-IR-Control
==============================

Ir Send - Updated for HVAC Mitsubishi IR

Project:
Controlling Mitsubishi HVAC from IR Led, Through Arduino.

Steps:
Catch the IR trame through AnalysiR -- Done.
Identify Trame Timing Specification -- Done.
Identify Packet Data -- Done.
Reverse Engineering to identify Commands Parameters contains in the Packet Date -- Almost Done.
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
    Not finalyzed:
      - Current Clock Parameters Management,
      - Time Programation Automatic Turn On or Off Function.
  - HVAC Mitsubishi Receive Void : Not Done.
  
