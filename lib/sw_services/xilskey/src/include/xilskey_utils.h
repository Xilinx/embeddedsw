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
 * @file xilskey_utils.h
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
* 2.00  hk      23/01/14 Corrected PL voltage checks to VCCINT and VCCAUX.
*                        CR#768077.
*                        Changed PS efuse error codes for voltage out of range
*
 *****************************************************************************/

#ifndef XILSKEY_UTILS_H
#define XILSKEY_UTILS_H
/***************************** Include Files ********************************/
#include "xadcps.h"
/************************** Constant Definitions ****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/**
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define XADC_DEVICE_ID 		XPAR_XADCPS_0_DEVICE_ID

/**
 * Temperature and voltage range for PS eFUSE reading and programming
 * Temperature in Celsius('C) and Voltage(V) is in volts
 */
#define XSK_EFUSEPS_TEMP_MIN			(-40)
#define XSK_EFUSEPS_TEMP_MAX			(125)
#define XSK_EFUSEPS_READ_VPAUX_MIN		(1.71)
#define XSK_EFUSEPS_READ_VPAUX_MAX		(1.98)
#define XSK_EFUSEPS_WRITE_VPAUX_MIN	(1.71)
#define XSK_EFUSEPS_WRITE_VPAUX_MAX	(1.98)

/**
 * Converting the celsius temperature to equivalent Binary data for xAdc
 */
#define XSK_EFUSEPS_TEMP_MIN_RAW	(XAdcPs_TemperatureToRaw(XSK_EFUSEPS_TEMP_MIN))
#define XSK_EFUSEPS_TEMP_MAX_RAW	(XAdcPs_TemperatureToRaw(XSK_EFUSEPS_TEMP_MAX))

/**
 * Converting the voltage to equivalent Binary data for xAdc
 */
#define XSK_EFUSEPS_READ_VPAUX_MIN_RAW	(XAdcPs_VoltageToRaw(XSK_EFUSEPS_READ_VPAUX_MIN))
#define XSK_EFUSEPS_READ_VPAUX_MAX_RAW	(XAdcPs_VoltageToRaw(XSK_EFUSEPS_READ_VPAUX_MAX))
#define XSK_EFUSEPS_WRITE_VPAUX_MIN_RAW	(XAdcPs_VoltageToRaw(XSK_EFUSEPS_WRITE_VPAUX_MIN))
#define XSK_EFUSEPS_WRITE_VPAUX_MAX_RAW	(XAdcPs_VoltageToRaw(XSK_EFUSEPS_WRITE_VPAUX_MAX))

/**
 * Temperature and voltage range for PL eFUSE reading and programming
 * Temperature in Celsius('C) and Voltage(V) is in volts
 */

/**
 * PL eFUSE write Min and Max Temperature and Voltages
 */
#define	XSK_EFUSEPL_WRITE_TEMP_MIN 				(15)
#define	XSK_EFUSEPL_WRITE_TEMP_MAX				(125)

#define	XSK_EFUSEPL_WRITE_VOLTAGE_VCCAUX_MIN 	(1.71)
#define	XSK_EFUSEPL_WRITE_VOLTAGE_VCCAUX_MAX		(1.98)

#define	XSK_EFUSEPL_WRITE_VOLTAGE_VCCINT_MIN 	(.87)
#define	XSK_EFUSEPL_WRITE_VOLTAGE_VCCINT_MAX		(1.1)

/**
 * PL eFUSE read Min and Max Temperature and Voltages
 */
#define	XSK_EFUSEPL_READ_TEMP_MIN				(-55)
#define	XSK_EFUSEPL_READ_TEMP_MAX				(125)
#define	XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MIN		(1.62)
#define	XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MAX		(1.98)

#define	XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MIN		(.795)
#define	XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MAX		(1.1)


/**
 * PL eFUSE write Min and Max Temperature and Voltages
 */

/**
 * Converting the celsius temperature to equivalent Binary data for xAdc
 */
#define	XSK_EFUSEPL_WRITE_TEMP_MIN_RAW 			(XAdcPs_TemperatureToRaw(XSK_EFUSEPL_WRITE_TEMP_MIN))
#define	XSK_EFUSEPL_WRITE_TEMP_MAX_RAW			(XAdcPs_TemperatureToRaw(XSK_EFUSEPL_WRITE_TEMP_MAX))

/**
 * Converting the voltage to equivalent Binary data for xAdc
 */
#define	XSK_EFUSEPL_WRITE_VOLTAGE_VCCAUX_MIN_RAW (XAdcPs_VoltageToRaw(XSK_EFUSEPL_WRITE_VOLTAGE_VCCAUX_MIN))
#define	XSK_EFUSEPL_WRITE_VOLTAGE_VCCAUX_MAX_RAW	(XAdcPs_VoltageToRaw(XSK_EFUSEPL_WRITE_VOLTAGE_VCCAUX_MAX))

/**
 * Converting the voltage to equivalent Binary data for xAdc
 */
#define	XSK_EFUSEPL_WRITE_VOLTAGE_VCCINT_MIN_RAW (XAdcPs_VoltageToRaw(XSK_EFUSEPL_WRITE_VOLTAGE_VCCINT_MIN))
#define	XSK_EFUSEPL_WRITE_VOLTAGE_VCCINT_MAX_RAW	(XAdcPs_VoltageToRaw(XSK_EFUSEPL_WRITE_VOLTAGE_VCCINT_MAX))

/**
 * PL eFUSE read Min and Max Temperature and Voltages
 */

/**
 * Converting the celsius temperature to equivalent Binary data for xAdc
 */
#define	XSK_EFUSEPL_READ_TEMP_MIN_RAW	(XAdcPs_TemperatureToRaw(XSK_EFUSEPL_READ_TEMP_MIN))
#define	XSK_EFUSEPL_READ_TEMP_MAX_RAW	(XAdcPs_TemperatureToRaw(XSK_EFUSEPL_READ_TEMP_MAX))

/**
 * Converting the voltage to equivalent Binary data for xAdc
 */

#define	XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MIN_RAW 	(XAdcPs_VoltageToRaw(XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MIN))
#define	XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MAX_RAW	(XAdcPs_VoltageToRaw(XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MAX))

/**
 * Converting the voltage to equivalent Binary data for xAdc
 */
#define	XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MIN_RAW 	(XAdcPs_VoltageToRaw(XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MIN))
#define	XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MAX_RAW	(XAdcPs_VoltageToRaw(XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MAX))

/**
 * Different voltage types that can be read from xAdc
 */
#define XSK_EFUSEPS_VPINT		(1)
#define XSK_EFUSEPS_VPAUX		(2)
#define XSK_EFUSEPS_VPDRO		(3)
#define XSK_EFUSEPS_VINT		(4)
#define XSK_EFUSEPS_VAUX		(5)

#define XSK_EFUSE_DEBUG_GENERAL	0x00000001    /* general debug  messages */

#if defined (XSK_EFUSE_DEBUG)
#define xeFUSE_dbg_current_types (XSK_EFUSE_DEBUG_GENERAL)
#else
#define xeFUSE_dbg_current_types 0
#endif
#ifdef STDOUT_BASEADDRESS
#define xeFUSE_printf(type,...) \
		if (((type) & xeFUSE_dbg_current_types))  {xil_printf (__VA_ARGS__); }
#else
#define xeFUSE_printf(type, ...)
#endif

#define XSK_GLOBAL_TIMER_BASE_ADDRESS           (0xF8F00000)

/**
 * Global_Timer_Counter _Register0 (0xf8f00200)
 */
#define XSK_GLOBAL_TIMER_COUNT_REG_LOW          (XSK_GLOBAL_TIMER_BASE_ADDRESS + 0x200)

/**
 * Global_Timer_Counter _Register1 (0xf8f00204)
 */
#define XSK_GLOBAL_TIMER_COUNT_REG_HIGH         (XSK_GLOBAL_TIMER_BASE_ADDRESS + 0x204)

/**
 * Global_Timer_Control_Register (0xf8f00208)
 */
#define XSK_GLOBAL_TIMER_CTRL_REG               (XSK_GLOBAL_TIMER_BASE_ADDRESS + 0x208)


/**
 * System Level Control Registers Start Addr
 */
#define XSK_SLCR_START_ADDRESS					(0xF8000000)
/**
 * ARM PLL Control Register
 */
#define XSK_ARM_PLL_CTRL_REG					(XSK_SLCR_START_ADDRESS + 0x100)
/**
 * ARM Clock Control Register
 */
#define XSK_ARM_CLK_CTRL_REG					(XSK_SLCR_START_ADDRESS + 0x120)

/**
 *  PL eFUSE aes key size in characters
 */
#define XSK_EFUSEPL_AES_KEY_STRING_SIZE         	(64)
/**
 *  PL eFUSE user low key size in characters
 */
#define XSK_EFUSEPL_USER_LOW_KEY_STRING_SIZE    	(2)
/**
 *  PL eFUSE user high key size in characters
 */
#define XSK_EFUSEPL_USER_HIGH_KEY_STRING_SIZE   	(6)
/**
 *  AES Key size in Bytes
 */
#define XSK_EFUSEPL_AES_KEY_SIZE_IN_BYTES		(32)
/**
 *  User Key size in Bytes
 */
#define XSK_EFUSEPL_USER_KEY_SIZE_IN_BYTES		(4)
/**
 *  AES Key size in Bits
 */
#define XSK_EFUSEPL_AES_KEY_SIZE_IN_BITS			(256)
/**
 *  User Low Key size in Bytes
 */
#define XSK_EFUSEPL_USER_LOW_KEY_SIZE_IN_BITS	(8)
/**
 *  User High Key size in Bytes
 */
#define XSK_EFUSEPL_USER_HIGH_KEY_SIZE_IN_BITS	(24)
/**
 * Key length definition for RSA KEY Hash
 */
#define XSK_EFUSEPS_RSA_KEY_HASH_LEN_IN_BYTES	(32)
/**
 *  PS eFUSE RSA key Hash size in characters
 */
#define XSK_EFUSEPL_RSA_KEY_HASH_STRING_SIZE     (64)
/************************** Variable Definitions ****************************/
/**
 * 	XADC Structure
 */
typedef struct
{
	/**
	 * Current temperature
	 */
	u32 Temp;
	/**
	 * Minimum temperature
	 */
	u32 TempMin;
	/**
	 * Maximum temperature
	 */
	u32 TempMax;
	/**
	 * Voltage type to read to select from VCCPINT, VCCPAUX, VCCPDRO
	 */
	u32 VType;
	/**
	 * Current voltage of Vtype
	 */
	u32 V;
	/**
	 * Minimum voltage of Vtype
	 */
	u32 VMin;
	/**
	 * Maximum voltage of Vtype
	 */
	u32 VMax;
} XSKEfusePs_XAdc;

/**
 * PL EFUSE error codes
 */
typedef enum {
	XSK_EFUSEPL_ERROR_NONE = 0,

	/**
	 * EFUSE Read error codes
	 */
	XSK_EFUSEPL_ERROR_ROW_NOT_ZERO = 0x10,
	XSK_EFUSEPL_ERROR_READ_ROW_OUT_OF_RANGE,
	XSK_EFUSEPL_ERROR_READ_MARGIN_OUT_OF_RANGE,
	XSK_EFUSEPL_ERROR_READ_BUFFER_NULL,
	XSK_EFUSEPL_ERROR_READ_BIT_VALUE_NOT_SET,
	XSK_EFUSEPL_ERROR_READ_BIT_OUT_OF_RANGE,
	XSK_EFUSEPL_ERROR_READ_TMEPERATURE_OUT_OF_RANGE,
	XSK_EFUSEPL_ERROR_READ_VCCAUX_VOLTAGE_OUT_OF_RANGE,
	XSK_EFUSEPL_ERROR_READ_VCCINT_VOLTAGE_OUT_OF_RANGE,

	/**
	 * EFUSE Write error codes
	 */
	XSK_EFUSEPL_ERROR_WRITE_ROW_OUT_OF_RANGE,
	XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE,
	XSK_EFUSEPL_ERROR_WRITE_TMEPERATURE_OUT_OF_RANGE,
	XSK_EFUSEPL_ERROR_WRITE_VCCAUX_VOLTAGE_OUT_OF_RANGE,
	XSK_EFUSEPL_ERROR_WRITE_VCCINT_VOLTAGE_OUT_OF_RANGE,

	/**
	 * EFUSE CNTRL error codes
	 */
	XSK_EFUSEPL_ERROR_FUSE_CNTRL_WRITE_DISABLED,
	XSK_EFUSEPL_ERROR_CNTRL_WRITE_BUFFER_NULL,

	/**
	 * EFUSE KEY error codes
	 */
	XSK_EFUSEPL_ERROR_NOT_VALID_KEY_LENGTH,
	XSK_EFUSEPL_ERROR_ZERO_KEY_LENGTH,
	XSK_EFUSEPL_ERROR_NOT_VALID_KEY_CHAR,
	XSK_EFUSEPL_ERROR_NULL_KEY,

	/**
	 * XSKEfusepl_Program_Efuse() error codes
	 */
	XSK_EFUSEPL_ERROR_KEY_VALIDATION = 0xF000,
	XSK_EFUSEPL_ERROR_PL_STRUCT_NULL = 0x1000,
	XSK_EFUSEPL_ERROR_JTAG_SERVER_INIT = 0x1100,
	XSK_EFUSEPL_ERROR_READING_FUSE_CNTRL = 0x1200,
	XSK_EFUSEPL_ERROR_DATA_PROGRAMMING_NOT_ALLOWED = 0x1300,
	XSK_EFUSEPL_ERROR_FUSE_CTRL_WRITE_NOT_ALLOWED = 0x1400,
	XSK_EFUSEPL_ERROR_READING_FUSE_AES_ROW = 0x1500,
	XSK_EFUSEPL_ERROR_AES_ROW_NOT_EMPTY = 0x1600,
	XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_AES_ROW = 0x1700,
	XSK_EFUSEPL_ERROR_READING_FUSE_USER_DATA_ROW = 0x1800,
	XSK_EFUSEPL_ERROR_USER_DATA_ROW_NOT_EMPTY = 0x1900,
	XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_DATA_ROW = 0x1A00,
	XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_CNTRL_ROW = 0x1B00,
	XSK_EFUSEPL_ERROR_XADC = 0x1C00,
	XSK_EFUSEPL_ERROR_INVALID_REF_CLK= 0x3000
}XSKEfusePl_ErrorCodes;


/**
 * PS EFUSE error codes
 */
typedef enum {
	XSK_EFUSEPS_ERROR_NONE = 0,

	/**
	 * EFUSE Read error codes
	 */
	XSK_EFUSEPS_ERROR_ADDRESS_XIL_RESTRICTED = 0x01,
	XSK_EFUSEPS_ERROR_READ_TMEPERATURE_OUT_OF_RANGE,
	XSK_EFUSEPS_ERROR_READ_VCCPAUX_VOLTAGE_OUT_OF_RANGE,
	XSK_EFUSEPS_ERROR_READ_VCCPINT_VOLTAGE_OUT_OF_RANGE,

	/**
	 * EFUSE Write error codes
	 */
	XSK_EFUSEPS_ERROR_WRITE_TEMPERATURE_OUT_OF_RANGE,
	XSK_EFUSEPS_ERROR_WRITE_VCCPAUX_VOLTAGE_OUT_OF_RANGE,
	XSK_EFUSEPS_ERROR_WRITE_VCCPINT_VOLTAGE_OUT_OF_RANGE,
	XSK_EFUSEPS_ERROR_VERIFICATION,
	XSK_EFUSEPS_ERROR_RSA_HASH_ALREADY_PROGRAMMED,

	/**
	 * FUSE CNTRL error codes
	 */
	XSK_EFUSEPS_ERROR_CONTROLLER_MODE,
	XSK_EFUSEPS_ERROR_REF_CLOCK,
	XSK_EFUSEPS_ERROR_READ_MODE,

	/**
	 * XADC Error Codes
	 */
	XSK_EFUSEPS_ERROR_XADC_CONFIG,
	XSK_EFUSEPS_ERROR_XADC_INITIALIZE,
	XSK_EFUSEPS_ERROR_XADC_SELF_TEST,

	/**
	 * Utils Error Codes
	 */
	XSK_EFUSEPS_ERROR_PARAMETER_NULL,
	XSK_EFUSEPS_ERROR_STRING_INVALID,


	/**
	 * XSKEfuse_Write/Read()common error codes
	 */
	XSK_EFUSEPS_ERROR_PS_STRUCT_NULL=0x8100,
	XSK_EFUSEPS_ERROR_XADC_INIT=0x8200,
	XSK_EFUSEPS_ERROR_CONTROLLER_LOCK=0x8300,
	XSK_EFUSEPS_ERROR_EFUSE_WRITE_PROTECTED=0x8400,
	XSK_EFUSEPS_ERROR_CONTROLLER_CONFIG=0x8500,
	XSK_EFUSEPS_ERROR_PS_PARAMETER_WRONG=0x8600,

	/**
	 * XSKEfusePs_Write() error codes
	 */
	XSK_EFUSEPS_ERROR_WRITE_128K_CRC_BIT=0x9100,
	XSK_EFUSEPS_ERROR_WRITE_NONSECURE_INITB_BIT=0x9200,
	XSK_EFUSEPS_ERROR_WRITE_UART_STATUS_BIT=0x9300,
	XSK_EFUSEPS_ERROR_WRITE_RSA_HASH=0x9400,
	XSK_EFUSEPS_ERROR_WRITE_RSA_AUTH_BIT=0x9500,
	XSK_EFUSEPS_ERROR_WRITE_WRITE_PROTECT_BIT=0x9600,
	XSK_EFUSEPS_ERROR_READ_HASH_BEFORE_PROGRAMMING=0x9700,

	/**
	 * XSKEfusePs_Read() error codes
	 */
	XSK_EFUSEPS_ERROR_READ_RSA_HASH=0xA100,

}XSKEfusePs_ErrorCodes;

/*
 * For backward compatibility with old error codes
 */

#define XSK_EFUSEPS_ERROR_READ_VCCAUX_VOLTAGE_OUT_OF_RANGE XSK_EFUSEPS_ERROR_READ_VCCPAUX_VOLTAGE_OUT_OF_RANGE
#define XSK_EFUSEPS_ERROR_READ_VCCINT_VOLTAGE_OUT_OF_RANGE XSK_EFUSEPS_ERROR_READ_VCCPINT_VOLTAGE_OUT_OF_RANGE
#define XSK_EFUSEPS_ERROR_WRITE_VCCAUX_VOLTAGE_OUT_OF_RANGE XSK_EFUSEPS_ERROR_WRITE_VCCPAUX_VOLTAGE_OUT_OF_RANGE
#define XSK_EFUSEPS_ERROR_WRITE_VCCINT_VOLTAGE_OUT_OF_RANGE XSK_EFUSEPS_ERROR_WRITE_VCCPINT_VOLTAGE_OUT_OF_RANGE



/************************** Function Prototypes *****************************/
u32 XilSKey_EfusePs_XAdcInit (void );
void XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(XSKEfusePs_XAdc *XAdcInstancePtr);
void XilSKey_Efuse_StartTimer(u32 RefClk);
u64 XilSKey_Efuse_GetTime();
void XilSKey_Efuse_SetTimeOut(volatile u64* t, u64 us);
u8 XilSKey_Efuse_IsTimerExpired(u64 t);
void XilSKey_Efuse_ConvertBitsToBytes(const u8 * Bits, u8 * Bytes, u32 Len);
void XilSKey_EfusePs_ConvertBytesToBits(const u8 * Bits, u8 * Bytes, u32 Len);
void XilSKey_EfusePs_ConvertBytesBeToLe(const u8 *Be, u8 *Le, u32 Len);
u32 XilSKey_Efuse_ValidateKey(const char *key, u32 len);
u32 XilSKey_Efuse_IsValidChar(const char *c);
/**
 * Common functions
 */
u32 XilSKey_Efuse_ConvertStringToHexLE(const char * Str, u8 * Buf, u32 Len);
u32 XilSKey_Efuse_ConvertStringToHexBE(const char * Str, u8 * Buf, u32 Len);
u32 XilSKey_Efuse_ValidateKey(const char *Key, u32 Len);

/***************************************************************************/



#endif /* XILSKEY_UTILS_H */
