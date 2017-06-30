# Python port for RPI3
# Eric Masse (Ericmas001) - 2017-06-30
# https://github.com/Ericmas001/HVAC-IR-Control

# From original: https://github.com/r45635/HVAC-IR-Control
# (c)  Vincent Cruvellier - 10th, January 2016 - Fun with ESP8266

import ir_sender
import pigpio

class PowerMode:
    PowerOff = 0b00000000   # 0x00      0000 0000        0
    PowerOn = 0x00100000    # 0x20      0010 0000       32

class ClimateMode:
    Hot = 0b00001000        # 0x08      0000 1000        8
    Cold = 0b00011000       # 0x18      0001 1000       24
    Dry = 0x00010000        # 0x10      0001 0000       16
    Auto = 0x00100000       # 0x20      0010 0000       32

class FanMode:
    Speed1 = 0b00000001     # 0x01      0000 0001        1
    Speed2 = 0b00000010     # 0x02      0000 0010        2
    Speed3 = 0b00000011     # 0x03      0000 0011        3
    Speed4 = 0b00000100     # 0x04      0000 0100        4
    Auto = 0b10000000       # 0x80      1000 0000      128
    Silent = 0b00000101     # 0x05      0000 0101        5

class WideVanneMode:
    Auto = 0b01000000       # 0x40      0100 0000       64
    WvH1 = 0b01001000       # 0x48      0100 1000       72
    WvH2 = 0b01010000       # 0x50      0101 0000       80
    WvH3 = 0b01011000       # 0x58      0101 1000       88
    WvH4 = 0b01100000       # 0x60      0110 0000       96
    WvH5 = 0b01101000       # 0x68      0110 1000      104
    AutoMove = 0b01111000   # 0x78      0111 1000      120

class Delay:
    HdrMark = 3400
    HdrSpace = 1750
    BitMark = 450
    OneSpace = 1300
    ZeroSpace = 420
    RptMark = 440
    RptSpace = 17100

class Temperature:
    Max = 31
    Min = 16

class Index:
    Power = 5               # Byte 6 - On / Off
    Climate = 6             # Byte 7 - Mode
    Temperature = 7         # Byte 8 - Temperature
    FanVanne = 9            # Byte 10 - FAN / VANNE
    CRC = 17                # Byte 18 - CRC

class Constants:
    Frequency = 38          # 38khz
    MinTemp = 16
    MaxTemp = 31
    NbBytes = 18
    NbPackets = 2           # For Mitsubishi IR protocol we have to send two time the packet data
