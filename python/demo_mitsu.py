#!/bin/python

"""
Demo Mitsubishi HVAC
"""
import time
from hvac_ircontrol.ir_sender import LogLevel
from hvac_ircontrol.mitsubishi import ClimateMode, FanMode, WideVanneMode, Mitsubishi

if __name__ == "__main__":
    while True:
        print("=======================================================")
        print("Power OFF")
        HVAC = Mitsubishi(23, LogLevel.ErrorsOnly)
        HVAC.power_off()
        print("Wait 2 secs ...")
        time.sleep(2)
        print("It's gonna get cold here !")
        HVAC.send_command(ClimateMode.Cold, 18, FanMode.Speed4, WideVanneMode.Auto)
        print("=======================================================")
        print("Go dormant for 30 secs ...")
        time.sleep(25)
