# HVAC-IR-Control - Python port for RPI3
# Eric Masse (Ericmas001) - 2017-06-30
# https://github.com/Ericmas001/HVAC-IR-Control

# From original: https://github.com/r45635/HVAC-IR-Control
# (c)  Vincent Cruvellier - 10th, January 2016 - Fun with ESP8266

import ir_sender
import pigpio

class PowerMode:
    """
    PowerMode
    """
    PowerOff = 0b00000000   # 0x00      0000 0000        0
    PowerOn = 0b00100000    # 0x20      0010 0000       32

class ClimateMode:
    """
    ClimateMode
    """
    Hot = 0b00001000        # 0x08      0000 1000        8
    Cold = 0b00011000       # 0x18      0001 1000       24
    Dry = 0b00010000        # 0x10      0001 0000       16
    Auto = 0b00100000       # 0x20      0010 0000       32

class FanMode:
    """
    FanMode
    """
    Speed1 = 0b00000001     # 0x01      0000 0001        1
    Speed2 = 0b00000010     # 0x02      0000 0010        2
    Speed3 = 0b00000011     # 0x03      0000 0011        3
    Speed4 = 0b00000100     # 0x04      0000 0100        4
    Auto = 0b10000000       # 0x80      1000 0000      128
    Silent = 0b00000101     # 0x05      0000 0101        5

class WideVanneMode:
    """
    WideVanneMode
    """
    Auto = 0b01000000       # 0x40      0100 0000       64
    WvH1 = 0b01001000       # 0x48      0100 1000       72
    WvH2 = 0b01010000       # 0x50      0101 0000       80
    WvH3 = 0b01011000       # 0x58      0101 1000       88
    WvH4 = 0b01100000       # 0x60      0110 0000       96
    WvH5 = 0b01101000       # 0x68      0110 1000      104
    AutoMove = 0b01111000   # 0x78      0111 1000      120

class Delay:
    """
    Delay
    """
    HdrMark = 3400
    HdrSpace = 1750
    BitMark = 450
    OneSpace = 1300
    ZeroSpace = 420
    RptMark = 440
    RptSpace = 17100

class Index:
    """
    Index
    """
    Power = 5               # Byte 6 - On / Off
    Climate = 6             # Byte 7 - Mode
    Temperature = 7         # Byte 8 - Temperature
    FanVanne = 9            # Byte 10 - FAN / VANNE
    CRC = 17                # Byte 18 - CRC

class Constants:
    """
    Constants
    """
    Frequency = 38000       # 38khz
    MinTemp = 16
    MaxTemp = 31
    MaxMask = 0xFF
    NbBytes = 18
    NbPackets = 2           # For Mitsubishi IR protocol we have to send two time the packet data

class Mitsubishi:
    """
    Mitsubishi
    """
    def __init__(self, gpio_pin):
        self.gpio_pin = gpio_pin

    def power_off(self):
        """
        power_off
        """
        self.__send_command(
            ClimateMode.Auto,
            21,
            FanMode.Auto,
            WideVanneMode.Auto,
            PowerMode.PowerOff)

    def send_command(self, climate_mode, temperature, fan_mode, wide_vanne_mode):
        """
        send_command
        """
        self.__send_command(
            climate_mode,
            temperature,
            fan_mode,
            wide_vanne_mode,
            PowerMode.PowerOn)

    def __send_command(self, climate_mode, temperature, fan_mode, wide_vanne_mode, power_mode):

        sender = ir_sender.IrSender(self.gpio_pin, "NEC", dict(
            leading_pulse_duration=Delay.HdrMark,
            leading_gap_duration=Delay.HdrSpace,
            one_pulse_duration=Delay.BitMark,
            one_gap_duration=Delay.OneSpace,
            zero_pulse_duration=Delay.BitMark,
            zero_gap_duration=Delay.ZeroSpace,
            trailing_pulse_duration=Delay.RptMark,
            trailing_gap_duration=Delay.RptSpace))

        # data array is a valid trame, only byte to be chnaged will be updated.
        data = [0x23, 0xCB, 0x26, 0x01, 0x00, 0x20,
                0x08, 0x06, 0x30, 0x45, 0x67, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x1F]

        data[Index.Power] = power_mode
        data[Index.Climate] = climate_mode
        data[Index.Temperature] = max(Constants.MinTemp, min(Constants.MaxTemp, temperature)) - 16
        data[Index.FanVanne] = fan_mode | wide_vanne_mode

        # CRC is a simple bits addition
        data[Index.CRC] = sum(data[:-1]) % (Constants.MaxMask + 1) # sum every bytes but the last one

        # transmit packet more than once
        for _ in range(0, Constants.NbPackets):
            sender.send_data(data, Constants.MaxMask)

        sender.terminate()
