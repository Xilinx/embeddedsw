/******************************************************************************
*
* Copyright (C) 2013 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
 * @file xilskey_utils.c
 *
 *
 * @note	None.
 *
 *
 * MODIFICATION HISTORY:
 *
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.00a rpoolla 04/26/13 First release
* 2.00  hk      22/01/14 Corrected PL voltage checks to VCCINT and VCCAUX.
*                        CR#768077
* 2.1   kvn     04/01/15 Fixed warnings. CR#716453.
* 3.00  vns     31/07/15 Added efuse functionality for Ultrascale.
*
 *****************************************************************************/

/***************************** Include Files ********************************/
#include "xil_io.h"
#include "xil_types.h"
#include "xilskey_utils.h"
/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
#ifdef XSK_ARM_PLATFORM
static XAdcPs XAdcInst;     /**< XADC driver instance */
u16 XAdcDeviceId;	/**< XADC Device ID */
#else
XTmrCtr XTmrCtrInst;
#endif
u32 TimerTicksfor100ns; /**< Global Variable to store ticks/100ns*/
u32 TimerTicksfor500ns; /**< Global Variable for 5 micro secs for microblaze */
/************************** Function Prototypes *****************************/
static u32 XilSKey_EfusePs_ConvertCharToNibble (char InChar, u8 *Num);
extern void Jtag_Read_Sysmon(u8 Row, u32 *Row_Data);
static u32 Xilskey_RowCrcCalculation(u32 PrevCRC, u32 Data, u32 Addr);
/***************************************************************************/
/**
* This function is used to initialize the XADC driver
*
* @return
* 		- XST_SUCCESS in case of no errors.
* 		- XSK_EFUSEPS_ERROR_XADC_CONFIG Error occurred with XADC config.
* 		- XSK_EFUSEPS_ERROR_XADC_INITIALIZE Error occurred while XADC initialization
* 		- XSK_EFUSEPS_ERROR_XADC_SELF_TEST Error occurred in XADC self test.
*
* TDD Cases:
*
****************************************************************************/

u32 XilSKey_EfusePs_XAdcInit (void )
{
	u32 Status;
#ifdef XSK_ARM_PLATFORM
	XAdcPs_Config *ConfigPtr;
	XAdcPs *XAdcInstPtr = &XAdcInst;

	/**
	 * specify the Device ID that is
	 * generated in xparameters.h
	 */
	XAdcDeviceId=XADC_DEVICE_ID;

	/**
	 * Initialize the XAdc driver.
	 */
	ConfigPtr = XAdcPs_LookupConfig(XAdcDeviceId);
	if (NULL == ConfigPtr) {
		return XSK_EFUSEPS_ERROR_XADC_CONFIG;
	}

	Status = XAdcPs_CfgInitialize(XAdcInstPtr, ConfigPtr,
			ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XSK_EFUSEPS_ERROR_XADC_INITIALIZE;
	}
	/**
	 * Self Test the XADC/ADC device
	 */
	Status = XAdcPs_SelfTest(XAdcInstPtr);
	if (Status != XST_SUCCESS) {
		return XSK_EFUSEPS_ERROR_XADC_SELF_TEST;
	}

	/**
	 * Disable the Channel Sequencer before configuring the Sequence
	 * registers.
	 */
	XAdcPs_SetSequencerMode(XAdcInstPtr, XADCPS_SEQ_MODE_SAFE);

	Status = XST_SUCCESS;
#else
	Status = XST_FAILURE;
#endif
	return Status;

}

/***************************************************************************/
/**
* This function is used to copy the min, max and current value of the
* temperature and voltage which are read from XADC.
*
* @param XAdcInstancePtr Pointer to the XSKEfusePs_XAdc. User has to
* 		 fill the VType to specify the type of voltage. Valid values
* 		 for VType are
* 		 	- XSK_EFUSEPS_VPINT
* 		 	- XSK_EFUSEPS_VPDRO
* 		 	- XSK_EFUSEPS_VPAUX
*                       - XSK_EFUSEPS_VINT
*                       - XSK_EFUSEPS_VAUX
*
* @return none
*
* TDD Cases:
*
****************************************************************************/

void XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(XSKEfusePs_XAdc *XAdcInstancePtr)
{
#ifdef XSK_MICROBLAZE_PLATFORM
	/* Temperature */
	Jtag_Read_Sysmon(XSK_SYSMON_TEMP_ROW, &(XAdcInstancePtr->Temp));
	XAdcInstancePtr->Temp = (XAdcInstancePtr->Temp) >> 6;
	/* Voltage */
	Jtag_Read_Sysmon(XSK_SYSMON_VOL_ROW, &(XAdcInstancePtr->V));
#else
	XAdcPs *XAdcInstPtr = &XAdcInst;
	u8 V, VMin, VMax;


	if (NULL == XAdcInstPtr) {
		return;
	}

	/**
	 * Read the on-chip Temperature Data (Current/Maximum/Minimum)
	 * from the ADC data registers.
	 */
	XAdcInstancePtr->Temp = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_TEMP);

	XAdcInstancePtr->TempMin =
			XAdcPs_GetMinMaxMeasurement(XAdcInstPtr, XADCPS_MIN_TEMP);

	XAdcInstancePtr->TempMax =
			XAdcPs_GetMinMaxMeasurement(XAdcInstPtr, XADCPS_MAX_TEMP);

	xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,
				"Read Temperature Value: %0x -> %d in Centigrades \n",
				XAdcInstancePtr->Temp,
				(int )XAdcPs_RawToTemperature(XAdcInstancePtr->Temp));

	/**
	 * Read the VccPint Voltage Data (Current/Maximum/Minimum) from the
	 * ADC data registers.
	 */

	switch (XAdcInstancePtr->VType)
	{
	case XSK_EFUSEPS_VINT:
		V = XADCPS_CH_VCCINT;
		VMax = XADCPS_MAX_VCCINT;
		VMin = XADCPS_MIN_VCCINT;
		break;

	case XSK_EFUSEPS_VAUX:
		V = XADCPS_CH_VCCAUX;
		VMax = XADCPS_MAX_VCCAUX;
		VMin = XADCPS_MIN_VCCAUX;
		break;

	case XSK_EFUSEPS_VPAUX:
		V = XADCPS_CH_VCCPAUX;
		VMax = XADCPS_MAX_VCCPAUX;
		VMin = XADCPS_MIN_VCCPAUX;
		break;

	case XSK_EFUSEPS_VPDRO:
		V = XADCPS_CH_VCCPDRO;
		VMax = XADCPS_MAX_VCCPDRO;
		VMin = XADCPS_MIN_VCCPDRO;
		break;

	case XSK_EFUSEPS_VPINT:
	default:
		V = XADCPS_CH_VCCPINT;
		VMax = XADCPS_MAX_VCCPINT;
		VMin = XADCPS_MIN_VCCPINT;
		break;
	}

	XAdcInstancePtr->V = XAdcPs_GetAdcData(XAdcInstPtr, V);
	XAdcInstancePtr->VMin = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr, VMin);
	XAdcInstancePtr->VMax = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr, VMax);

	xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,
				"Read Voltage Value: %0x -> %d in Volts \n",
				XAdcInstancePtr->V,
				(int )XAdcPs_RawToVoltage(XAdcInstancePtr->V));

#endif

	return;
}

/****************************************************************************/
/**
*
* Initializes the global timer & starts it.
* Calculates the timer ticks for 1us.
*
*
*
* @param	RefClk - reference clock
*
* @return
*
*	None
*
* @note		None.
*
*****************************************************************************/
void XilSKey_Efuse_StartTimer()
{
#ifdef XSK_ARM_PLATFORM
		/**
         * Disable the Timer counter
         */
        Xil_Out32(XSK_GLOBAL_TIMER_CTRL_REG,0);
        /**
         * Write the lower 32 bit timer counter register.
         */
        Xil_Out32(XSK_GLOBAL_TIMER_COUNT_REG_LOW, 0x0);
        /**
         * Write the upper 32 bit timer counter register.
         */
        Xil_Out32(XSK_GLOBAL_TIMER_COUNT_REG_HIGH, 0x0);
        /**
         * Enable the Timer counter
         */
        Xil_Out32(XSK_GLOBAL_TIMER_CTRL_REG,0x1);
#else
	XTmrCtr_SetOptions(&XTmrCtrInst, XSK_TMRCTR_NUM,
					XTC_AUTO_RELOAD_OPTION);
		XTmrCtr_Start(&XTmrCtrInst, XSK_TMRCTR_NUM);

#endif
}

/****************************************************************************/
/**
*
* Returns the timer ticks from start of timer to till now.
*
* @return
*
*	t - Timer ticks lapsed till now.
*
* @note		None.
*
*****************************************************************************/

u64 XilSKey_Efuse_GetTime(void)
{
    volatile u64 t=0;
#ifdef XSK_ARM_PLATFORM
	volatile u32 t_hi=0, t_lo=0;

    do {
			t_hi = Xil_In32(XSK_GLOBAL_TIMER_COUNT_REG_HIGH);
			t_lo = Xil_In32(XSK_GLOBAL_TIMER_COUNT_REG_LOW);
	}while(t_hi != Xil_In32(XSK_GLOBAL_TIMER_COUNT_REG_HIGH));

	t = (((u64) t_hi) << 32) | (u64) t_lo;

#else
		 t = XTmrCtr_GetValue(&XTmrCtrInst, XSK_TMRCTR_NUM);
#endif
        return t;
}
/****************************************************************************/
/**
*
* Calculates the timer ticks to wait to set the time out
*
* @param	t  - timer ticks to wait to set the timeout
* @param	us - Timeout period in us
*
*
* @return
*	t  - timer ticks to wait to set the timeout
*
* @note		None.
*
*****************************************************************************/
void XilSKey_Efuse_SetTimeOut(volatile u64* t, u64 us)
{
        volatile u64 t_end;
        t_end = XilSKey_Efuse_GetTime();
        /**
         * us: time to wait in microseconds. Convert to clock ticks and
         * add to current time.
         */
		t_end += (us * TimerTicksfor100ns);
        *t = t_end;
}

/****************************************************************************/
/**
*
* Checks whether the timer has been expired or not
*
* @param	t - timeout value in us
*
* @return
*
*	t_end - Returns the global timer value in case of timer expired
*
* @note		None.
*
*****************************************************************************/

u8 XilSKey_Efuse_IsTimerExpired(u64 t)
{
        u64 t_end;
        t_end = XilSKey_Efuse_GetTime();
        return t_end >= t;
}

/****************************************************************************/
/**
 * Converts the char into the equivalent nibble.
 *	Ex: 'a' -> 0xa, 'A' -> 0xa, '9'->0x9
 *
 * @param InChar is input character. It has to be between 0-9,a-f,A-F
 * @param Num is the output nibble.
 * @return
 * 		- XST_SUCCESS no errors occured.
 *		- XST_FAILURE an error when input parameters are not valid
 ****************************************************************************/
static u32 XilSKey_EfusePs_ConvertCharToNibble (char InChar, u8 *Num)
{
	/**
	 * Convert the char to nibble
	 */
	if ((InChar >= '0') && (InChar <= '9'))
		*Num = InChar - '0';
	else if ((InChar >= 'a') && (InChar <= 'f'))
		*Num = InChar - 'a' + 10;
	else if ((InChar >= 'A') && (InChar <= 'F'))
		*Num = InChar - 'A' + 10;
	else
		return XSK_EFUSEPS_ERROR_STRING_INVALID;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * Converts the string into the equivalent Hex buffer.
 *	Ex: "abc123" -> {0xab, 0xc1, 0x23}
 *
 * @param	Str is a Input String. Will support the lower and upper case values.
 * 		Value should be between 0-9, a-f and A-F
 *
 * @param	Buf is Output buffer.
 * @param	Len of the input string. Should have even values
 * @return
 * 		- XST_SUCCESS no errors occured.
 *		- XST_FAILURE an error when input parameters are not valid
 *		- an error when input buffer has invalid values
 *
 *	TDD Test Cases:
	---Initialization---
	Len is odd
	Len is zero
	Str is NULL
	Buf is NULL
	---Functionality---
	Str input with only numbers
	Str input with All values in A-F
	Str input with All values in a-f
	Str input with values in a-f, 0-9, A-F
	Str input with values in a-z, 0-9, A-Z
	Boundary Cases
	Memory Bounds of buffer checking
  ****************************************************************************/

u32 XilSKey_Efuse_ConvertStringToHexBE(const char * Str, u8 * Buf, u32 Len)
{
	u32 ConvertedLen=0;
	u8 LowerNibble, UpperNibble;

	/**
	 * Check the parameters
	 */
	if (Str == NULL)
		return XSK_EFUSEPS_ERROR_PARAMETER_NULL;

	if (Buf == NULL)
		return XSK_EFUSEPS_ERROR_PARAMETER_NULL;

	/**
	 * Len has to be multiple of 2
	 */
	if ((Len == 0) || (Len%2 == 1))
		return XSK_EFUSEPS_ERROR_PARAMETER_NULL;

	ConvertedLen = 0;
	while (ConvertedLen < Len) {
		/**
		 * Convert char to nibble
		 */
		if (XilSKey_EfusePs_ConvertCharToNibble (Str[ConvertedLen],&UpperNibble)
				==XST_SUCCESS) {
			/**
			 * Convert char to nibble
			 */
			if (XilSKey_EfusePs_ConvertCharToNibble (Str[ConvertedLen+1],
					&LowerNibble)==XST_SUCCESS) {
				/**
				 * Merge upper and lower nibble to Hex
				 */
				Buf[ConvertedLen/2] =
						(UpperNibble << 4) | LowerNibble;
			}
			else {
				/**
				 * Error converting Lower nibble
				 */
				return XSK_EFUSEPS_ERROR_STRING_INVALID;
			}
		}
		else {
			/**
			 * Error converting Upper nibble
			 */
			return XSK_EFUSEPS_ERROR_STRING_INVALID;
		}
		/**
		 * Converted upper and lower nibbles
		 */
		xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,"Converted %c%c to %0x\n",
				Str[ConvertedLen],Str[ConvertedLen+1],Buf[ConvertedLen/2]);
		ConvertedLen += 2;
	}

	return XST_SUCCESS;
}


/****************************************************************************/
/**
 * Converts the string into the equivalent Hex buffer.
 *	Ex: "abc123" -> {0x23, 0xc1, 0xab}
 *
 * @param	Str is a Input String. Will support the lower and upper case values.
 * 		Value should be between 0-9, a-f and A-F
 *
 * @param	Buf is Output buffer.
 * @param	Len of the input string. Should have even values
 * @return
 * 		- XST_SUCCESS no errors occured.
 *		- XST_FAILURE an error when input parameters are not valid
 *		- an error when input buffer has invalid values
 *
 *	TDD Test Cases:
	---Initialization---
	Len is odd
	Len is zero
	Str is NULL
	Buf is NULL
	---Functionality---
	Str input with only numbers
	Str input with All values in A-F
	Str input with All values in a-f
	Str input with values in a-f, 0-9, A-F
	Str input with values in a-z, 0-9, A-Z
	Boundary Cases
	Memory Bounds of buffer checking
  ****************************************************************************/


u32 XilSKey_Efuse_ConvertStringToHexLE(const char * Str, u8 * Buf, u32 Len)
{
	u32 ConvertedLen=0;
		u8 LowerNibble, UpperNibble;
		u32 index;

		/**
		 * Check the parameters
		 */
		if (Str == NULL)
			return XSK_EFUSEPS_ERROR_PARAMETER_NULL;

		if (Buf == NULL)
			return XSK_EFUSEPS_ERROR_PARAMETER_NULL;

		/**
		 * Len has to be multiple of 2
		 */
		if ((Len == 0) || (Len%2 == 1))
			return XSK_EFUSEPS_ERROR_PARAMETER_NULL;

		index = (Len/8) - 1;
		ConvertedLen = 0;
		while (ConvertedLen < Len) {
			/**
			 * Convert char to nibble
			 */
			if (XilSKey_EfusePs_ConvertCharToNibble (Str[ConvertedLen],
										&UpperNibble) == XST_SUCCESS) {
				/**
				 * Convert char to nibble
				 */
				if (XilSKey_EfusePs_ConvertCharToNibble (Str[ConvertedLen+1],
						&LowerNibble)==XST_SUCCESS)	{
					/**
					 * Merge upper and lower nibble to Hex
					 */
					Buf[index--] =
							(UpperNibble << 4) | LowerNibble;
				}
				else {
					/**
					 * Error converting Lower nibble
					 */
					return XSK_EFUSEPS_ERROR_STRING_INVALID;
				}
			}
			else {
				/**
				 * Error converting Upper nibble
				 */
				return XSK_EFUSEPS_ERROR_STRING_INVALID;
			}
			/**
			 * Converted upper and lower nibbles
			 */
			ConvertedLen += 2;
		}

		return XST_SUCCESS;
}

/***************************************************************************/
/**
* This function is used to convert the Big Endian Byte data to
* Little Endian Byte data
* For ex: 1234567890abcdef -> 78563412efcdab90
*
* @param Be Big endian data
* @param Le Little endian data
* @param Len Length of data to be converted and it should be multiple of 4
* @return
* 		- XST_SUCCESS no errors occurred.
*		- XST_FAILURE an error occurred during reading the PS eFUSE.
*
* TDD Test Cases:
*
****************************************************************************/
void XilSKey_EfusePs_ConvertBytesBeToLe(const u8 *Be, u8 *Le, u32 Len)
{
	u32 Index;

	if ((Be == NULL) || (Le == NULL) || (Len == 0))
		return;

	for (Index=0;Index<Len;Index=Index+4) {
		Le[Index+3]=Be[Index];
		Le[Index+2]=Be[Index+1];
		Le[Index+1]=Be[Index+2];
		Le[Index]=Be[Index+3];
	}
	return ;
}

/****************************************************************************/
/**
 * Convert the Bits to Bytes in Little Endian format
 *       Ex: 0x5C -> {0, 0, 1, 1, 1, 0, 1, 0}
 *
 * @param       Bits Input Buffer.
 * @param       Bytes is Output buffer.
 * @param       Len of the input buffer in bits
 * @return		None
  Test Cases:
	Input with All Zeroes
	Input with All Ones
	Input Little Endian (General Cases )
	Input Check Big Endian - False case
	Check for Len not a multiple of 8
	Check for Len 0
	Memory Bounds of buffer checking
 ****************************************************************************/
void XilSKey_Efuse_ConvertBitsToBytes(const u8 * Bits, u8 * Bytes, u32 Len)
{
	u8 Data=0;
	u32 Index=0, BitIndex=0, ByteIndex=0;

	/**
	 * Make sure the bytes array is 0'ed first.
	 */
	for(Index=0;Index<Len;Index++) {
		Bytes[Index] = 0;
	}

	while(Len) {
		/**
		 * Convert 8 Bit One Byte to 1 Bit 8 Bytes
		 */
		for(Index=0;Index<8;Index++) {
			/**
			 * Convert from LSB -> MSB - Little Endian
			 */
			Data = (Bits[BitIndex] >> Index) & 0x1;
			Bytes[ByteIndex] = Data;
			ByteIndex++;
			Len--;
			/**
			 * If len is not Byte aligned
			 */
			if(Len == 0)
				return;
		}
		BitIndex++;
	}
	return ;
}

/****************************************************************************/
/**
 * Convert the Bytes to Bits in little endian format
 * 0th byte is LSB, 7th byte is MSB
 *       Ex: {0, 0, 1, 1, 1, 0, 1, 0} -> 0x5C
 *
 * @param        Bytes Input Buffer.
 * @param        Bits is Output buffer.
 * @param        Len of the input buffer.
 * @return       None
  Test Cases:
	Input with All Zeroes
	Input with All Ones
	Input Little Endian (General Cases )
	Input Check Big Endian - False case
	Check for Len not a multiple of 8
	Check for Len 0
	Memory Bounds of buffer checking
 ****************************************************************************/
void XilSKey_EfusePs_ConvertBytesToBits(const u8 * Bytes, u8 * Bits , u32 Len)
{
	u8 Tmp=0;
	u32 Index=0, BitIndex=0, ByteIndex=0;

	/**
	 * Make sure the bits array is 0 first.
	 */
	for(Index=0;Index<((Len%8)?((Len/8)+1):(Len/8));Index++) {
		Bits[Index] = 0;
	}

	while(Len) {
		/**
		 * Convert 1 Bit 8 Bytes to 8 Bit 1 Byte
		 */
		for(Index=0;Index<8;Index++) {
			/**
			 * Store from LSB -> MSB - Little Endian
			 */
			Tmp = (Bytes[ByteIndex]) & 0x1;
			Bits[BitIndex] |= (Tmp << Index);
			ByteIndex++;
			Len--;
			/**
			 * If Len is not Byte aligned
			 */
			if(Len == 0)
				return;
		}
		BitIndex++;
	}
	return;
}

/****************************************************************************/
/**
 * Validate the key for proper characters & proper length
 *
 *
 * @param        Key - Hash Key
 * @param        Len - Valid length of key
 *
 * @return
 *			XST_SUCCESS	- In case of Success
 *			XST_FAILURE - In case of Failure
 ****************************************************************************/
u32 XilSKey_Efuse_ValidateKey(const char *Key, u32 Len)
{
	u32 i;
    /**
     * Make sure passed key is not NULL
     */
    if(Key == NULL) {
	return (XSK_EFUSEPL_ERROR_KEY_VALIDATION +
			XSK_EFUSEPL_ERROR_NULL_KEY);
    }

    if(Len == 0) {
	return (XSK_EFUSEPL_ERROR_KEY_VALIDATION +
			XSK_EFUSEPL_ERROR_ZERO_KEY_LENGTH);
    }

	/**
	 * Make sure the key has valid length
	 */
    if (strlen(Key) != Len) {
		return (XSK_EFUSEPL_ERROR_KEY_VALIDATION +
				XSK_EFUSEPL_ERROR_NOT_VALID_KEY_LENGTH);
    }

    /**
     * Make sure the key has valid characters
     */
	for(i=0;i<strlen(Key);i++) {
		if(XilSKey_Efuse_IsValidChar(&Key[i]) != XST_SUCCESS)
			return (XSK_EFUSEPL_ERROR_KEY_VALIDATION +
					XSK_EFUSEPL_ERROR_NOT_VALID_KEY_CHAR);
	}
    return XSK_EFUSEPL_ERROR_NONE;
}
/****************************************************************************/
/**
 * Checks whether the passed character is a valid hash key character
 *
 *
 * @param        c - Character to check proper value
 *
 * @return
 *			XST_SUCCESS	- In case of Success
 *			XST_FAILURE - In case of Failure
 ****************************************************************************/

u32 XilSKey_Efuse_IsValidChar(const char *c)
{
    char ValidChars[] = "0123456789abcdefABCDEF";
    char *RetVal;

    if(c == NULL)
	return XST_FAILURE;

    RetVal = strchr(ValidChars, (int)*c);
    if(RetVal == NULL) {
	return XST_FAILURE;
    }
    else {
	return XST_SUCCESS;
    }
}
/****************************************************************************/
/**
 * This API intialises the Timer based on platform
 *
 * @param	None.
 *
 * @return	RefClk will be returned.
 *
 * @note	None.
 *
 ****************************************************************************/
u32 Xilskey_Timer_Intialise()
{

	u32 RefClk;

#ifdef XSK_ARM_PLATFORM
	TimerTicksfor100ns = 0;
	u32 ArmPllFdiv;
	u32 ArmClkDivisor;
		/**
		 *  Extract PLL FDIV value from ARM PLL Control Register
		 */
	ArmPllFdiv = (Xil_In32(XSK_ARM_PLL_CTRL_REG)>>12 & 0x7F);

		/**
		 *  Extract Clock divisor value from ARM Clock Control Register
		 */
	ArmClkDivisor = (Xil_In32(XSK_ARM_CLK_CTRL_REG)>>8 & 0x3F);

		/**
		 * Initialize the variables
		 */
	RefClk = ((XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ * ArmClkDivisor)/
					ArmPllFdiv);

	/**
	 * Calculate the Timer ticks per 100ns
	 */
	TimerTicksfor100ns =
			(((RefClk * ArmPllFdiv)/ArmClkDivisor)/2)/10000000;
#else

	u32 Status;
	TimerTicksfor500ns = 0;

	RefClk = XSK_EFUSEPL_CLCK_FREQ_ULTRA;

	Status = XTmrCtr_Initialize(&XTmrCtrInst, XTMRCTR_DEVICE_ID);
	if (Status == XST_FAILURE) {
		return XST_FAILURE;
	}
	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly.
	 */
	Status = XTmrCtr_SelfTest(&XTmrCtrInst, XSK_TMRCTR_NUM);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	TimerTicksfor500ns =  XSK_EFUSEPL_CLCK_FREQ_ULTRA/200000;

#endif

	return RefClk;

}

/****************************************************************************/
/**
 * Copies one string to other from specified location
 *
 * @param	Src is a pointer to Source string.
 * @param	Dst is a pointer to Destination string.
 * @param	From which position to be copied.
 * @param	To which position to be copied.
 *
 * @return	None.
 *
 * @note	None.
 *
 ****************************************************************************/
void Xilskey_StrCpyRange(u8 *Src, u8 *Dst, u32 From, u32 To)
{
	u32 Index,J = 0;
	for (Index = From; Index <= To; Index++) {
		Dst[J++] = Src[Index];
	}
	Dst[J] = '\0';

}

/****************************************************************************/
/**
 * Calculates CRC value of provided key.
 *
 * @param	Key is the string contains AES key in hexa decimal of length
 *		less than or equal to 64.
 *
 * @return	Crc of AES key value.
 *
 * @note	None.
 *
 ****************************************************************************/
u32 Xilskey_CrcCalculation(u8 *Key)
{
	u32 Crc = 0;
	u8 Key_8[8];
	u8 Key_Hex[4];
	u32 Index;
	u32 Key_32;
	u8 FullKey[64] = {0};
	u32 Length = strlen((char *)Key);


	if (Length > 64) {
		return XSK_EFUSEPL_ERROR_NOT_VALID_KEY_LENGTH;
	}
	if (Length < 64) {
		strcpy((char *)&FullKey[64-Length + 1], (char *)Key);

	}
	else {
		strcpy((char *)FullKey, (char *)Key);
	}

	for (Index = 0; Index <8;Index++) {
		Xilskey_StrCpyRange(FullKey, Key_8, ((7 - Index)*8),
					((((7 - Index) + 1)*8)-1));
		XilSKey_Efuse_ConvertStringToHexBE((char *)Key_8, Key_Hex, 8);
		Key_32 = (Key_Hex[0] << 24) | (Key_Hex[1] << 16) |
				(Key_Hex[2] << 8) | (Key_Hex[3]);
		Crc = Xilskey_RowCrcCalculation(Crc, Key_32, 20+Index);
	}

	return Crc;
}

/****************************************************************************/
/**
 * Calculates CRC value for each row of AES key.
 *
 * @param	PrevCRC holds the prev row's CRC.
 * @param	Data holds the present row's key.
 * @param	Addr stores the current row number.
 *
 * @return	Crc of current row.
 *
 * @note	None.
 *
 ****************************************************************************/
u32 Xilskey_RowCrcCalculation(u32 PrevCRC, u32 Data, u32 Addr)
{
	u32 Crc = PrevCRC;
	u32 Value = Data;
	u32 Row = Addr;
	u32 Index;

	for (Index = 0; Index < 32; Index++) {
		if ((((Value & 0x1) ^ Crc) & 0x1) != 0) {
			Crc = ((Crc >> 1) ^ REVERSE_POLYNOMIAL);
		}
		else {
			Crc = Crc >>1;
		}
		Value = Value >>1;
	}

	for (Index = 0; Index < 5; Index++) {
		if ((((Row & 0x1) ^ Crc) & 0x1) != 0) {
			Crc = ((Crc >> 1) ^ REVERSE_POLYNOMIAL);
		}
		else {
			Crc = Crc >>1;
		}
		Row = Row >> 1;
	}

	return Crc;

}

/****************************************************************************/
/**
 * This API reverse the value.
 *
 * @param	Input is a 32 bit variable
 *
 * @return	Reverse the given value.
 *
 * @note	None.
 *
 ****************************************************************************/
u32 Xilskey_Efuse_ReverseHex(u32 Input)
{
	u32 Index = 0;
	u32 Rev = 0;
	u32 Bit;

	while (Index++ < 32) {
	Bit = Input & 1;
	Input = Input >> 1;
	Rev = Rev ^ Bit;
	if (Index < 32)
		Rev = Rev << 1;
	}

	return Rev;
}
