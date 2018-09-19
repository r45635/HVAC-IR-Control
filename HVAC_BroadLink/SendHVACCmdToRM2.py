#!/bin/python
""" 	
	FUN With Broadlink IR Mini 
	https://github.com/r45635/HVAC-IR-Control
	19 September 2018, (c) Vincent Cruvellier 
		initial version
	Purpose:
		use BroadLink RM Mini Pro for sending my ownbuilt command.
		A few translation has been performed from the work performed On Arduino in order to 
		have a trame Braodlink compatible.
		I've used the great work provided on braodlink (https://github.com/mjg59/python-broadlink)
		I try to set explanation within the code in the purpose of letting any DIY guys adjusting
		that code example for its own purpose.
		I'ts a template/code example, no ther pretention
		
		Prerequisit:
		Python 3.6.3 (I've tested on that version) Tested both on Linux and Windows.
		Broadlink python libraries
		An RM Pro Mini from BroadLink 'RM2"
		You need to know IP and Mac of your BroadLink device
		
		An HVAC Mitsubishi, can be almost easily ported to another HVAC brand should you know already the trame 
		content and sequence like it has been performed with Panasonic by Mat2Vence ;)
"""
"""
Demo Mitsubishi HVAC
"""
import broadlink
import time
from datetime import datetime
import sys
import binascii
import argparse
import math

# Definition of an HVAC Cmd Class Object
class HVAC_CMD:
	class __IR_SPEC:
		HVAC_MITSUBISHI_HDR_MARK	= 3400
		HVAC_MITSUBISHI_HDR_SPACE	= 1750
		HVAC_MITSUBISHI_BIT_MARK 	= 450
		HVAC_MITSUBISHI_ONE_SPACE	= 1300
		HVAC_MISTUBISHI_ZERO_SPACE	= 420
		HVAC_MITSUBISHI_RPT_MARK	= 440
		HVAC_MITSUBISHI_RPT_SPACE	= 17100
	class HVAC_Power:
		Off 		= 0
		On  		= 0x20
	class HVAC_Mode:
		Auto 		= 0b00100000
		Cold 		= 0b00011000
		Dry 		= 0b00010000
		Hot 		= 0b00001000
	class HVAC_Isee:
		On			= 0b01000000
		Off			= 0
	class HVAC_Fan:
		Auto 		= 0
		Speed_1 	= 1
		Speed_2 	= 2
		Speed_3 	= 3
		Speed_4 	= 4
		Speed_5 	= 5
		Silent 		= 0b00000101
	class HVAC_Vanne:
		Auto 		= 0b01000000
		H1		 	= 0b01001000
		H2 			= 0b01010000
		H3		 	= 0b01011000
		H4		 	= 0b01100000
		H5			= 0b01101000
		Swing 		= 0b01111000
	class HVAC_Wide:
		Left_end	= 0b00010000
		Left		= 0b00100000
		Middle		= 0b00110000
		Right		= 0b01000000
		Right_end	= 0b01010000
		Swing		= 0b10000000
	class HVAC_Area:
		Swing		= 0b00000000
		Left		= 0b01000000
		Right		= 0b10000000
		Auto		= 0b11000000
	class HVAC_Clean:
		On			= 0b00000100
		Off			= 0b00000000
	class HVAC_Plasma:
		On			= 0b00000100
		Off			= 0b00000000
	class TimeCtrl:
		OnStart		= 0b00000101
		OnEnd		= 0b00000011
		OnStartEnd	= 0b00000111
		Off			= 0b00000000
		

	# BROADLINK_DURATION_CONVERSION_FACTOR (Brodlink do not use exact duration in µs but a factor of BDCF)
	__BDCF = 269/8192 
	# The Famous Data Sequence I'm starting to know too much...
	__data 	= [0x23, 0xCB, 0x26, 0x01, 0x00, 0x20,	0x08, 0x06, 0x30, 0x45, 0x67, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x1F]
	# BraodLink Sepecifc Headr for IR command start with a specific code
	__IR_BroadLink_Code = 0x26

	_log	= True
	__StrHexCode = ""

	# Default Values for the Command
	Temp = 21
	Power 		= HVAC_Power
	Mode 		= HVAC_Mode
	Fan			= HVAC_Fan
	Isee		= HVAC_Isee
	Area 		= HVAC_Area
	Clean		= HVAC_Clean
	Plasma		= HVAC_Plasma

	def __init__(self):
		self.Power 		= self.HVAC_Power.Off
		self.Mode 		= self.HVAC_Mode.Auto
		self.Fan 		= self.HVAC_Fan.Auto
		self.Isee		= self.HVAC_Isee.Off
		self.Vanne		= self.HVAC_Vanne.Auto
		self.Wide		= self.HVAC_Wide.Swing
		self.Area		= self.HVAC_Area.Auto
		self.Clean 		= self.HVAC_Clean.Off
		self.Plasma 	= self.HVAC_Plasma.Off
		self.EndTime 	= None
		self.StartTime  = None
		self._log		= False
	
	def __val2BrCode(self, valeur, noZero=False):
	#	val2BrCode: Transform a number to a broadlink Hex string 
		valeur = int(math.ceil(valeur)) # force int, round up float if needed
		if (valeur < 256):
			# Working with just a byte
			myStr="%0.2x" % valeur
		else:
			# Working with a Dword
			datalen = "%0.04x" % valeur
			if (noZero):
				myStr = datalen[2:4] + datalen[0:2]
			else:
				myStr = "00" + datalen[2:4] + datalen[0:2]
		return myStr
	
	def __build_cmd(self):
	#	Build_Cmd: Build the Command applying all parameters defined. The cmd is stored in memory, not send. 
		now = datetime.today()
		
		self.__data[5] 	= self.Power
		self.__data[6] 	= self.Mode | self.Isee
		self.__data[7] 	= max(16, min(31, self.Temp)) - 16
		self.__data[8] 	= self.Mode | self.Wide
		self.__data[9]	= self.Fan | self.Vanne
		self.__data[9]	= (now.hour*6) + (now.minute//10)
		self.__data[10] = 0 if self.EndTime is None else ((self.EndTime.hour*6) + (self.EndTime.minute//10))
		self.__data[11] = 0 if self.StartTime is None else ((self.StartTime.hour*6) + (self.StartTime.minute//10))
		self.__data[12] = 0 # Time Control not used in this version
		self.__data[14]	= self.Clean
		self.__data[15]	= self.Plasma
		self.__data[17] = sum(self.__data[:-1]) % (0xFF + 1)
	
		StrHexCode = ""
		for i in range(0, len(self.__data)):
			mask = 1
			tmp_StrCode = ""
			for j in range(0,8):
				if self.__data[i]& mask != 0:
					tmp_StrCode = tmp_StrCode + "%0.2x" % int(self.__IR_SPEC.HVAC_MITSUBISHI_BIT_MARK*self.__BDCF) + "%0.2x" % int(self.__IR_SPEC.HVAC_MITSUBISHI_ONE_SPACE*self.__BDCF)
				else:
					tmp_StrCode = tmp_StrCode + "%0.2x" % int(self.__IR_SPEC.HVAC_MITSUBISHI_BIT_MARK*self.__BDCF) + "%0.2x" % int(self.__IR_SPEC.HVAC_MISTUBISHI_ZERO_SPACE*self.__BDCF)
				mask = mask << 1	
			StrHexCode = StrHexCode + tmp_StrCode
		
		# StrHexCode contain the Frame for the HVAC Mitsubishi IR Command requested
		
		# Exemple using the no repeat function of the Command
		# Build the start of the BroadLink Command
		StrHexCodeBR = "%0.2x" % self.__IR_BroadLink_Code 	# First byte declare Cmd Type for BroadLink
		StrHexCodeBR = StrHexCodeBR + "%0.2x" % 0x00		# Second byte is the repeation number of the Cmd
		# Build Header Sequence Block of IR HVAC
		StrHeaderTrame = self.__val2BrCode(self.__IR_SPEC.HVAC_MITSUBISHI_HDR_MARK * self.__BDCF)
		StrHeaderTrame = StrHeaderTrame + self.__val2BrCode(self.__IR_SPEC.HVAC_MITSUBISHI_HDR_SPACE * self.__BDCF)
		# Build the Repeat Sequence Block of IR HVAC 
		StrRepeatTrame = self.__val2BrCode(self.__IR_SPEC.HVAC_MITSUBISHI_RPT_MARK * self.__BDCF)
		StrRepeatTrame = StrRepeatTrame + self.__val2BrCode(self.__IR_SPEC.HVAC_MITSUBISHI_RPT_SPACE * self.__BDCF)
		# Build the Full frame for IR HVAC
		StrDataCode = StrHeaderTrame + StrHexCode + StrRepeatTrame + StrHeaderTrame + StrHexCode	
		# Calculate the lenght of the Cmd data and complete the Broadlink Command Header
		StrHexCodeBR = StrHexCodeBR + self.__val2BrCode(len(StrDataCode)/2, True)
		StrHexCodeBR = StrHexCodeBR + StrDataCode
		# Finalize the BroadLink Command ; must be end by 0x0d, 0x05 per protocol
		StrHexCodeBR = StrHexCodeBR + "0d05"
		# Voila, the full BroadLink Command is complete
		self.__StrHexCode = StrHexCodeBR

		# Exemple using the repeat function of the Command
		# Build the start of the BroadLink Command
		StrHexCodeBR = "%0.2x" % self.__IR_BroadLink_Code 	# First byte declare Cmd Type for BroadLink
		StrHexCodeBR = StrHexCodeBR + "%0.2x" % 2 			# Second byte is the repeation number of the Cmd
		# Build Header Sequence Block of IR HVAC
		StrHeaderTrame = self.__val2BrCode(self.__IR_SPEC.HVAC_MITSUBISHI_HDR_MARK * self.__BDCF)
		StrHeaderTrame = StrHeaderTrame + self.__val2BrCode(self.__IR_SPEC.HVAC_MITSUBISHI_HDR_SPACE * self.__BDCF)
		# Build the Repeat Sequence Block of IR HVAC
		StrRepeatTrame = self.__val2BrCode(self.__IR_SPEC.HVAC_MITSUBISHI_RPT_MARK * self.__BDCF)
		StrRepeatTrame = StrRepeatTrame + self.__val2BrCode(self.__IR_SPEC.HVAC_MITSUBISHI_RPT_SPACE * self.__BDCF)
		# Build the Full frame for IR HVAC
		StrDataCode = StrHeaderTrame + StrHexCode + StrRepeatTrame
		# Calculate the lenght of the Cmd data and complete the Broadlink Command Header
		StrHexCodeBR = StrHexCodeBR + self.__val2BrCode(len(StrDataCode)/2, True)
		StrHexCodeBR = StrHexCodeBR + StrDataCode
		# Finalize the BroadLink Command ; must be end by 0x0d, 0x05 per protocol
		StrHexCodeBR = StrHexCodeBR + "0d05"
		# Voila, the full BroadLink Command is complete
		self.__StrHexCode = StrHexCodeBR

		
	def print_cmd(self):
		# Display to terminal the Built Command to be sent to the Broadlink IR Emitter
		self.__build_cmd()			# Request to build the Cmd
		print(self.__StrHexCode)	# Display the Command

	def return_broadlink_cmd(self):
		myhex = self.__StrHexCode
		myhex = myhex.replace(' ', '').replace('\n', '')
		myhex = myhex.encode('ascii', 'strict')
		return binascii.unhexlify(myhex)
	
	def send_cmd(self, to_host, to_mac, to_devtype="RM2"):
		self.__build_cmd()
		#device = broadlink.rm(host=("192.168.2.96",80), mac=bytearray.fromhex("34 ea 34 8a 35 ee"),devtype="RM2")
		device = broadlink.rm(host=(to_host,80), mac=bytearray.fromhex(to_mac),devtype=to_devtype)
		if (self._log):
			print("Connecting to Broadlink device....")
		device.auth()
		time.sleep(1)
		if (self._log):
			print("Connected....")
		time.sleep(1)
		device.host
		myhex = self.__StrHexCode
		myhex = myhex.replace(' ', '').replace('\n', '')
		myhex = myhex.encode('ascii', 'strict')
		device.send_data(binascii.unhexlify(myhex))
		if (self._log):
			print("Code Sent....")



parser = argparse.ArgumentParser(description='Short sample python HVAC_IR command sender to Broadlink RM2 Mini ')

parser.add_argument('-t', '--temperature', action='store', dest='HVAC_TEMPERATURE', default=21, type=int,
                    help='Set HVAC Temperature in Celcius, Ex: 21')

parser.add_argument('-p','--power', action='store_true', default=False,
                    dest='HVAC_POWER',
                    help='HVAC Power, default = Power Off')

parser.add_argument('-c', '--climate', action='store', dest='HVAC_CLIMATE_CODE', default='C',
                    help='Define Climate Code : C=Cold*, H=HOT')

parser.add_argument('-Vv', '--vanne_vertical', action='store', dest='HVAC_VANNE_V_CODE', default='A',
                    help='Define Vertical Vanne Mode : A=Automatic*, S=Swing, B=Bottom, T:Top')				

parser.add_argument('-F', '--fan', action='store', dest='HVAC_FAN_MODE', default='A',
                    help='Define Fan speed : A=Automatic*, L=Low, M=Middle, F=Fast, S=Silent')				
					
parser.add_argument('--version', action='version', version='%(prog)s 1.0')

results = parser.parse_args()

		
MyHVAC_cmd 		= HVAC_CMD()	# create an HVAC Command Object

# Parse the Arg Parameters, if any
# Parse Power On/Off
if (results.HVAC_POWER):
	MyHVAC_cmd.Power = MyHVAC_cmd.HVAC_Power.On
else:
	MyHVAC_cmd.Power = MyHVAC_cmd.HVAC_Power.Off

# Parse HVAC Clim Mode	
if (results.HVAC_CLIMATE_CODE[0:1] == 'C'):
	MyHVAC_cmd.Mode	= MyHVAC_cmd.HVAC_Mode.Cold
elif (results.HVAC_CLIMATE_CODE[0:1] == 'H'):
	MyHVAC_cmd.Mode	= MyHVAC_cmd.HVAC_Mode.Hot
elif (results.HVAC_CLIMATE_CODE[0:1] == 'D'):
	MyHVAC_cmd.Mode	= MyHVAC_cmd.HVAC_Mode.Dry
else:
	MyHVAC_cmd.Mode	= MyHVAC_cmd.HVAC_Mode.Auto

# Parse HVAC Fan Mode
if (results.HVAC_FAN_MODE[0:1] == 'S'):
	MyHVAC_cmd.Fan	= MyHVAC_cmd.HVAC_Fan.Silent
elif (results.HVAC_FAN_MODE[0:1] == '1'):
	MyHVAC_cmd.Fan	= MyHVAC_cmd.HVAC_Fan.Speed1
elif (results.HVAC_FAN_MODE[0:1] == '2'):
	MyHVAC_cmd.Fan	= MyHVAC_cmd.HVAC_Fan.Speed2
elif (results.HVAC_FAN_MODE[0:1] == '3'):
	MyHVAC_cmd.Fan	= MyHVAC_cmd.HVAC_Fan.Speed3
elif (results.HVAC_FAN_MODE[0:1] == '4'):
	MyHVAC_cmd.Fan	= MyHVAC_cmd.HVAC_Fan.Speed4
elif (results.HVAC_FAN_MODE[0:1] == '5'):
	MyHVAC_cmd.Fan	= MyHVAC_cmd.HVAC_Fan.Speed5
else:
	MyHVAC_cmd.Fan	= MyHVAC_cmd.HVAC_Fan.Auto

# Parse HVAC_Vanne	Mode / HVAC_VANNE_V_CODE
if (results.HVAC_FAN_MODE[0:1] == 'S'):
	MyHVAC_cmd.Vanne	= MyHVAC_cmd.HVAC_Vanne.Swing
elif (results.HVAC_FAN_MODE[0:2] == 'B'):
	MyHVAC_cmd.Vanne	= MyHVAC_cmd.HVAC_Vanne.H5
elif (results.HVAC_FAN_MODE[0:1] == 'T'):
	MyHVAC_cmd.Vanne	= MyHVAC_cmd.HVAC_Vanne.H1
else:
	MyHVAC_cmd.Vanne	= MyHVAC_cmd.HVAC_Vanne.Auto

# Parse Temperature
MyHVAC_cmd.Temp	= results.HVAC_TEMPERATURE

# display the Cmd built
MyHVAC_cmd.print_cmd()
# Send the Comman to my RM2 device
MyHVAC_cmd.send_cmd(to_host="192.168.2.96", to_mac="34 ea 34 8a 35 ee")
