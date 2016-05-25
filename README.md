
#HVAC IR Control

##Introduction
*HVAC IR Control aims to facilitate control of your HVAC emulating the IR code from an Arduino. The Do It Your Self has no limitation except the time to spent on it. I hope this repository can accelerate your development espcecially if you use Mitsubishi HVAC. Panasonic HVAC support has been added too thanks to another contributor. Should you wants to add another protocol data information releated to new brand or model then feel free to contact us*

##Project background
I started to use sketch with an Arduino associated to IR emitter and IR Receiver. radpidly i stated that library available do not covers HVAC modules. A way to progress was to use a software named AnalysIR, therefore i ordered a license for this tool. Indeed this tool was perfectly doing the job of identifying the IR trame my IR remote was sending. Nevertheless, even if the data collected was able to identify bits values, the packet of data provided to use with the different existing libraries was a set of mark and space interger values. For an Arduino it's a lot of data only for one command to process in IR. Due to this limitation of memory, i started to think about coding a function using the Hex values of the trame decoded by AnalysisIR instead of having to manage mark & space huge array of data. Without AnalysisIR software, it never have been possible for me to achieve what i did. Thanks Chris ;). 
My first code was able to take in input the hex data of the trame decoded and setting the diffrent header pulse period i went through a loop of each bit in order to produce the correct mark and space sequences. I identified quickly that the packet of data was in fact a specific packet data sent twice. After this dicovery i started to look at the values of the data from this packet. I was entering in the reverse engineering of the Mitsubishi trame... In order to understand the procol, i started to log different configurations and the packet data associated. I've used an Excel file to start to dig into the differents bits changing folloing the configurations. Hopefully the CRC was not  complex. I let this excel debug file in the repository. Might be they can be helpul for others.
Finally the packet data has been decoded at least for the part of my HVAC system. 
The protocol decoded, i decided to use IR-Remote to add specific functions to control a Mitsubishi HVAC. Simply pass the parameters and the IR trame is compiled and send to the HVAC. No more problem of memory. A colleague join this work having on his side a PANASONIC HVAC. The same methods has been applied to understand the panansonic protocol. Like for the Mitsubishi HVAC, the Panasonic HVAC functions have been added to this git, still based on a modified IR Remote.

Recently, an anonymous and humble contributor provided information on unknow parts of the Mitsubishi protocol. All new fields decoded have been added to the Protocol Information data. The code has not been modified yet. 

#Overview of Protocol and features managed
## Mitsubishi Inverter HVAC
Now there is two kind of function you may use to control HVAC from mitsubishi. Thanks to a a anonymous contributor i get opportunity to complete the protocol. Basically he got much more option than mine HVAC. Nevertheless, just a warning, id did not had possibilities to check all this new features then do not hesitate to report issue. Enjoy !
the function to send configuration is
```
void sendHvacMitsubishi(
 HvacMode                  HVAC_Mode,           // Example HVAC_HOT  HvacMitsubishiMode
 int                       HVAC_Temp,           // Example 21  (°c)
 HvacFanMode               HVAC_FanMode,        // Example FAN_SPEED_AUTO  HvacMitsubishiFanMode
 HvacVanneMode             HVAC_VanneMode,      // Example VANNE_AUTO_MOVE  HvacMitsubishiVanneMode
 int                       OFF                  // Example false (Request Turn On = False)
);
```
new function with enhanced function:
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

the function to send configuration is
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

the function to send configuration is
```
void sendHvacToshiba(
 HvacMode                  HVAC_Mode,           // Example HVAC_HOT  
 int                       HVAC_Temp,           // Example 21  (°c)
 HvacFanMode               HVAC_FanMode,        // Example FAN_SPEED_AUTO  
 int                       HVAC_SWITCH          // Example false
);
```
