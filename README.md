
# HVAC IR Control

## Introduction

HVAC IR Control aims to facilitate control of your Heating, Ventilation and Air-Conditioning system (HVAC) by emulating the IR code usually sent via remote control through an Arduino. The Do-It-Your-Self has no limitation except the time to spent on it.

I hope this repository can accelerate your development especially if you use a Mitsubishi HVAC. Panasonic HVAC support has been added too thanks to another contributor. Should you want to add another protocol related to an additional brand or model then feel free to contact us.

## Project background

I started to use sketch with an Arduino associated to IR emitter and IR Receiver. I rapidly found that libraries available do not cover HVAC modules. A way to progress was to use a piece of software called AnalysIR. I ordered a license for this tool and indeed it was perfectly suited to doing the job of identifying the IR trame (TRAnsmission of MEssages) my IR remote was sending.

Nevertheless, even if the data collected was able to identify bits values, the packet of data provided to use with the different existing libraries was a set of mark and space integer values. For an Arduino that's a lot of data only for one command to process in IR. Due to this limitation of memory, I started to think about coding a function using the Hex values of the trame decoded by AnalysisIR instead of having to manually mark & space huge array of data. Without AnalysisIR software, it never have been possible for me to achieve what I did. Thanks Chris ;).

My first code was able to take in input of the hex data of the trame decoded. Setting the different header pulse periods, I went through a loop of each bit in order to produce the correct mark and space sequences. I identified quickly that the packet of data was in fact a specific packet data sent twice. After this discovery I started to look at the values of the data from this packet. I was now reverse engineering of the Mitsubishi trame. In order to understand the protocol, I started to log different configurations and the packet data associated. I used an Excel file to dig into the different bits that were changing and follow the configurations. Fortunately the CRC was not complex. I let this excel debug file in the repository. It might that it's helpull for others.

Finally the packet data has been partially decoded  (for the bits my HVAC system supports).

The protocol decoded, I decided to use IR-Remote to add specific functions to control a Mitsubishi HVAC. Simply pass the parameters and the IR trame is compiled and send to the HVAC. No more problems running out of memory. A colleague (who had a PANASONIC HVAC) joined this work and applied the same methods to understand the Panasonic protocol. As per the Mitsubishi HVAC, the Panasonic HVAC functions have now been added to this repository, still based on a modified IR Remote.

Recently, an anonymous and humble contributor provided information on previously unknown parts of the Mitsubishi protocol. All new fields decoded have been added to the Protocol Information data. The code provided has not been modified.

# Overview of Protocol and features managed

## Mitsubishi Inverter HVAC

There are now two sets of functions you may use to control HVAC from Mitsubishi.

Thanks to a a anonymous contributor I had the opportunity to complete the protocol. Their system has many more options/features than mine. Nevertheless, just by way of warning, I have not had the chance to check all this new features. If you find any issues, feel free to report. Enjoy!

The function to send configuration is:

```
void sendHvacMitsubishi(
 HvacMode                  HVAC_Mode,           // Example HVAC_HOT  HvacMitsubishiMode
 int                       HVAC_Temp,           // Example 21  (°c)
 HvacFanMode               HVAC_FanMode,        // Example FAN_SPEED_AUTO  HvacMitsubishiFanMode
 HvacVanneMode             HVAC_VanneMode,      // Example VANNE_AUTO_MOVE  HvacMitsubishiVanneMode
 int                       OFF                  // Example false (Request Turn On = False)
);
```

The new function with enhanced functions:

```
void sendHvacMitsubishiFD(
 HvacMode                  HVAC_Mode,           // Example HVAC_HOT  HvacMitsubishiMode
 int                       HVAC_Temp,           // Example 21  (°c)
 HvacFanMode               HVAC_FanMode,        // Example FAN_SPEED_AUTO  HvacMitsubishiFanMode
 HvacVanneMode             HVAC_VanneMode,      // Example VANNE_AUTO_MOVE  HvacMitsubishiVanneMode
 HvacAreaMode              HVAC_AreaMode,       // Example AREA_AUTO
 HvacWideVanneMode         HVAC_WideMode,       // Example WIDE_MIDDLE
 int                       HVAC_PLASMA,          // Example true to Turn ON PLASMA Function
 int                       HVAC_CLEAN_MODE,      // Example false
 int                       HVAC_ISEE,            // Example False
 int                       OFF                   // Example false to Turn ON HVAC / true to request to turn off
 );
```

Functions confirmed in MSZ-GE and MFZ modules from Mitsubishi.

## Panasonic HVAC

The function to send configuration is:

```
void sendHvacPanasonic(
 HvacMode                  HVAC_Mode,           // Example HVAC_HOT  HvacPanasonicMode
 int                       HVAC_Temp,           // Example 21  (°c)
 HvacFanMode               HVAC_FanMode,        // Example FAN_SPEED_AUTO  HvacPanasonicFanMode
 HvacVanneMode             HVAC_VanneMode,      // Example VANNE_AUTO_MOVE  HvacPanasonicVanneMode
 HvacProfileMode           HVAC_ProfileMode,    // Example QUIET HvacProfileMode
 int                       HVAC_SWITCH          // Example false
);
```
## Toshiba HVAC

The function to send configuration is:

```
void sendHvacToshiba(
 HvacMode                  HVAC_Mode,           // Example HVAC_HOT  
 int                       HVAC_Temp,           // Example 21  (°c)
 HvacFanMode               HVAC_FanMode,        // Example FAN_SPEED_AUTO  
 int                       HVAC_SWITCH          // Example false
);
```
