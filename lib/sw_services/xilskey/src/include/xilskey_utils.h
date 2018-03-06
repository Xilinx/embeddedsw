/******************************************************************************
*
* Copyright (C) 2013 - 2018 Xilinx, Inc.  All rights reserved.
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
* 3.00  vns     31/07/15 Added Xilskey_Timer_Intialise API and modified
*                        prototype of XilSKey_Efuse_StartTimer
*                        Modified efuse PS macro
*                        XSK_EFUSEPS_RSA_KEY_HASH_STRING_SIZE to
*                        XSK_EFUSEPL_RSA_KEY_HASH_STRING_SIZE
*                        Added efuse functionality for Ultrascale.
* 4.0   vns     10/01/15 Added efuse functionality for ZynqMp platform.
*                        Added XilSKey_Ceil API. Added error code for efuse and
*                        bbram PS for Zynq MP.
*                        Modified Xilskey_CrcCalculation API name to
*                        XilSKey_CrcCalculation. and Xilskey_Timer_Intialise API
*                        to XilSKey_Timer_Intialise
*       vns     10/20/15 Added cplusplus boundary blocks.
* 6.0   vns     07/07/16 Added hardware module time out error code
*               07/18/16 Added error codes for eFUSE PS User FUSEs programming
*                        Added sysmonpsu driver for temperature and voltage
*                        checks.
* 6.2   vns     03/10/17 Added error codes for LBist, LPD/FPD SC enable bits
*                        programming.
* 6.4   vns     02/27/18 Added support for virtex and virtex ultrascale plus
*
 *****************************************************************************/

#ifndef XILSKEY_UTILS_H
#define XILSKEY_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#include "xparameters.h"
#ifdef XPAR_XSK_MICROBLAZE_PLATFORM
#include "xsysmon.h"
#include "xtmrctr.h"
#else
#if defined (ARMR5) || defined (__aarch64__) || defined (ARMA53_32)
#include "xsysmonpsu.h"
#include "xplatform_info.h"
#else
#include "xadcps.h"
#endif
#endif
#include "xstatus.h"

/************************** Constant Definitions ****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
#ifdef XPAR_XSK_MICROBLAZE_PLATFORM
#define XSK_MICROBLAZE_PLATFORM
#else
#define XSK_ARM_PLATFORM
#if defined (ARMR5) || defined (__aarch64__) || defined (ARMA53_32)
#define XSK_ZYNQ_ULTRA_MP_PLATFORM
#else
#define XSK_ZYNQ_PLATFORM
#endif

#endif

/* Definitions for Ultrascale and Ultrascale plus */
#ifdef XSK_MICROBLAZE_PLATFORM

#ifdef XPAR_XSK_MICROBLAZE_ULTRA_PLUS
#define XSK_MICROBLAZE_ULTRA_PLUS
#endif

#ifdef XPAR_XSK_MICROBLAZE_ULTRA
#define XSK_MICROBLAZE_ULTRA
#endif

#endif
/**
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifdef XSK_ZYNQ_PLATFORM
#define XADC_DEVICE_ID 		XPAR_XADCPS_0_DEVICE_ID
#endif

#ifdef XSK_ZYNQ_ULTRA_MP_PLATFORM
#define XSYSMON_DEVICE_ID	XPAR_XSYSMONPSU_0_DEVICE_ID

/* ZynqMp efusePs ps Ref Clk frequency */
#define XSK_ZYNQMP_EFUSEPS_PS_REF_CLK_FREQ	XPAR_PSU_PSS_REF_CLK_FREQ_HZ
#endif

#ifdef XSK_MICROBLAZE_PLATFORM
#define XTMRCTR_DEVICE_ID		(XPAR_TMRCTR_0_DEVICE_ID)
#define XSK_EFUSEPL_CLCK_FREQ_ULTRA	(XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ)
#define XSK_TMRCTR_NUM			(0)
#define XSK_GPIO_DEVICE_ID		(XPAR_AXI_GPIO_0_DEVICE_ID)
#endif

#define REVERSE_POLYNOMIAL	(0x82F63B78)
				/**< Polynomial for calculating CRC */
/** @name CSU_DMA pause types
 * @{
 */
typedef enum {
	XSK_FPGA_SERIES_ULTRA,	/**< Ultrascale series */
	XSK_FPGA_SERIES_ZYNQ,	/**< Zynq series */
	XSK_FPGA_SERIES_ULTRA_PLUS /**< Ultrascale plus series */
}XSKEfusePl_Fpga;
/*@}*/

/**
 * Row numbers of Sysmon
 */
#define XSK_SYSMON_TEMP_ROW	(0)	/**< Row for Temperature */
#define XSK_SYSMON_VOL_ROW	(2)	/**< Row for Voltage */

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

/* ZynqMP eFUSE voltage ranges */
#define XSK_ZYNQMP_EFUSEPS_VCC_PSINTLP_MIN	(0.675)
#define XSK_ZYNQMP_EFUSEPS_VCC_PSINTLP_MAX	(0.935)
/* VCC AUX should be 1.8 +/-10% */
#define XSK_ZYNQMP_EFUSEPS_VCC_AUX_MIN		(1.62)
#define XSK_ZYNQMP_EFUSEPS_VCC_AUX_MAX		(1.98)

#ifdef XSK_ZYNQ_PLATFORM
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
#endif

#ifdef XSK_ZYNQ_ULTRA_MP_PLATFORM
/**
 * Converting the celsius temperature to equivalent Binary data for xAdc
 */
#define XSK_EFUSEPS_TEMP_MIN_RAW \
		(XSysMonPsu_TemperatureToRaw_OnChip(XSK_EFUSEPS_TEMP_MIN))
#define XSK_EFUSEPS_TEMP_MAX_RAW \
		(XSysMonPsu_TemperatureToRaw_OnChip(XSK_EFUSEPS_TEMP_MAX))

#define XSK_EFUSEPS_VPAUX_MIN_RAW \
		(XSysMonPsu_VoltageToRaw(XSK_ZYNQMP_EFUSEPS_VCC_AUX_MIN))
#define XSK_EFUSEPS_VPAUX_MAX_RAW \
		(XSysMonPsu_VoltageToRaw(XSK_ZYNQMP_EFUSEPS_VCC_AUX_MAX))

#define XSK_EFUSEPS_VCC_PSINTLP_MIN_RAW \
		(XSysMonPsu_VoltageToRaw(XSK_ZYNQMP_EFUSEPS_VCC_PSINTLP_MIN))
#define XSK_EFUSEPS_VCC_PSINTLP_MAX_RAW \
		(XSysMonPsu_VoltageToRaw(XSK_ZYNQMP_EFUSEPS_VCC_PSINTLP_MAX))
#endif
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

/* Ultrascale Microblaze Voltage range */
#define	XSK_EFUSEPL_VOL_VCCAUX_MIN_ULTRA	(1.746)
#define	XSK_EFUSEPL_VOL_VCCAUX_MAX_ULTRA	(1.854)
#define	XSK_EFUSEPL_TEMP_MIN_ULTRA		(-40)
#define	XSK_EFUSEPL_TEMP_MAX_ULTRA		(125)

/**
 * PL eFUSE write Min and Max Temperature and Voltages
 */
#ifdef XSK_ARM_PLATFORM
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

#else
/**
 * Converting the voltage to equivalent Binary data for xAdc
 */
#define XSK_EFUSEPL_VOL_VCCAUX_MIN_RAW_ULTRA 	(XSysMon_VoltageToRaw(XSK_EFUSEPL_VOL_VCCAUX_MIN_ULTRA))
#define XSK_EFUSEPL_VOL_VCCAUX_MAX_RAW_ULTRA 	(XSysMon_VoltageToRaw(XSK_EFUSEPL_VOL_VCCAUX_MAX_ULTRA))

#define XSK_EFUSEPL_TEMP_MIN_RAW_ULTRA	(XSysMon_TemperatureToRaw(XSK_EFUSEPL_TEMP_MIN_ULTRA))
#define XSK_EFUSEPL_TEMP_MAX_RAW_ULTRA	(XSysMon_TemperatureToRaw(XSK_EFUSEPL_TEMP_MAX_ULTRA))

#endif
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
 *  32 bit User Key size in Bytes
 */
#define XSK_EFUSEPL_USER_KEY_SIZE_IN_BYTES		(4)

/* 128 bit User key size in bytes */
#define XSK_EFUSEPL_128BIT_USERKEY_SIZE_IN_BYTES	(16)
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
#define XSK_EFUSEPS_RSA_KEY_HASH_STRING_SIZE     (64)
/**
 * Ultrascale Efuse PL RSA Key size in Bytes
 */
#define XSK_EFUSEPL_RSA_KEY_HASH_SIZE_IN_BYTES			(48)


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
	 * SECURE KEY error codes
	 */
	 XSK_EFUSEPL_ERROR_FUSE_SEC_WRITE_DISABLED,
	 XSK_EFUSEPL_ERROR_FUSE_SEC_READ_DISABLED,
	 XSK_EFUSEPL_ERROR_SEC_WRITE_BUFFER_NULL,

	XSK_EFUSEPL_ERROR_READ_PAGE_OUT_OF_RANGE,
	XSK_EFUSEPL_ERROR_FUSE_ROW_RANGE,
	XSK_EFUSEPL_ERROR_IN_PROGRAMMING_ROW,
	XSK_EFUSEPL_ERROR_PRGRMG_ROWS_NOT_EMPTY,

	/* Error in Hw module */
	XSK_EFUSEPL_ERROR_HWM_TIMEOUT = 0x80,
	XSK_EFUSEPL_ERROR_USER_FUSE_REVERT = 0x90,

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
	XSK_EFUSEPL_ERROR_INVALID_REF_CLK= 0x3000,
	XSK_EFUSEPL_ERROR_FUSE_SEC_WRITE_NOT_ALLOWED = 0x1D00,
	XSK_EFUSEPL_ERROR_READING_FUSE_STATUS = 0x1E00,
	XSK_EFUSEPL_ERROR_FUSE_BUSY = 0x1F00,
	XSK_EFUSEPL_ERROR_READING_FUSE_RSA_ROW = 0x2000,
	XSK_EFUSEPL_ERROR_TIMER_INTIALISE_ULTRA = 0x2200,
	XSK_EFUSEPL_ERROR_READING_FUSE_SEC = 0x2300,
	XSK_EFUSEPL_ERROR_PRGRMG_FUSE_SEC_ROW = 0x2500,
	XSK_EFUSEPL_ERROR_PRGRMG_USER_KEY = 0x4000,
	XSK_EFUSEPL_ERROR_PRGRMG_128BIT_USER_KEY = 0x5000,
	XSK_EFUSEPL_ERROR_PRGRMG_RSA_HASH = 0x8000
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

	XSK_EFUSEPS_ERROR_AES_ALREADY_PROGRAMMED,
	XSK_EFUSEPS_ERROR_SPKID_ALREADY_PROGRAMMED,
	XSK_EFUSEPS_ERROR_PPK0_HASH_ALREADY_PROGRAMMED,
	XSK_EFUSEPS_ERROR_PPK1_HASH_ALREADY_PROGRAMMED,

	XSK_EFUSEPS_ERROR_PROGRAMMING_TBIT_PATTERN,

	XSK_EFUSEPS_ERROR_BEFORE_PROGRAMMING = 0x0080,

	XSK_EFUSEPS_ERROR_PROGRAMMING = 0x00A0,
	XSK_EFUSEPS_ERROR_READ = 0x00B0,
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

	XSK_EFUSEPS_ERROR_WRTIE_DFT_JTAG_DIS_BIT = 0x9800,
	XSK_EFUSEPS_ERROR_WRTIE_DFT_MODE_DIS_BIT = 0x9900,
	XSK_EFUSEPS_ERROR_WRTIE_AES_CRC_LK_BIT = 0x9A00,
	XSK_EFUSEPS_ERROR_WRTIE_AES_WR_LK_BIT = 0x9B00,
	XSK_EFUSEPS_ERROR_WRTIE_USE_AESONLY_EN_BIT = 0x9C00,
	XSK_EFUSEPS_ERROR_WRTIE_BBRAM_DIS_BIT = 0x9D00,
	XSK_EFUSEPS_ERROR_WRTIE_PMU_ERR_DIS_BIT = 0x9E00,
	XSK_EFUSEPS_ERROR_WRTIE_JTAG_DIS_BIT = 0x9F00,

	/**
	 * XSKEfusePs_Read() error codes
	 */
	XSK_EFUSEPS_ERROR_READ_RSA_HASH=0xA100,

	XSK_EFUSEPS_ERROR_WRONG_TBIT_PATTERN = 0xA200,
	XSK_EFUSEPS_ERROR_WRITE_AES_KEY = 0xA300,
	XSK_EFUSEPS_ERROR_WRITE_SPK_ID = 0xA400,
	XSK_EFUSEPS_ERROR_WRITE_USER_KEY = 0xA500,
	XSK_EFUSEPS_ERROR_WRITE_PPK0_HASH = 0xA600,
	XSK_EFUSEPS_ERROR_WRITE_PPK1_HASH = 0xA700,

	XSK_EFUSEPS_ERROR_CACHE_LOAD = 0xB000,
	/* Error in programmin user fuses */
	XSK_EFUSEPS_ERROR_WRITE_USER0_FUSE = 0xC000,
	XSK_EFUSEPS_ERROR_WRITE_USER1_FUSE = 0xC100,
	XSK_EFUSEPS_ERROR_WRITE_USER2_FUSE = 0xC200,
	XSK_EFUSEPS_ERROR_WRITE_USER3_FUSE = 0xC300,
	XSK_EFUSEPS_ERROR_WRITE_USER4_FUSE = 0xC400,
	XSK_EFUSEPS_ERROR_WRITE_USER5_FUSE = 0xC500,
	XSK_EFUSEPS_ERROR_WRITE_USER6_FUSE = 0xC600,
	XSK_EFUSEPS_ERROR_WRITE_USER7_FUSE = 0xC700,

	XSK_EFUSEPS_ERROR_WRTIE_USER0_LK_BIT = 0xC800,
	XSK_EFUSEPS_ERROR_WRTIE_USER1_LK_BIT = 0xC900,
	XSK_EFUSEPS_ERROR_WRTIE_USER2_LK_BIT = 0xCA00,
	XSK_EFUSEPS_ERROR_WRTIE_USER3_LK_BIT = 0xCB00,
	XSK_EFUSEPS_ERROR_WRTIE_USER4_LK_BIT = 0xCC00,
	XSK_EFUSEPS_ERROR_WRTIE_USER5_LK_BIT = 0xCD00,
	XSK_EFUSEPS_ERROR_WRTIE_USER6_LK_BIT = 0xCE00,
	XSK_EFUSEPS_ERROR_WRTIE_USER7_LK_BIT = 0xCF00,

	XSK_EFUSEPS_ERROR_WRTIE_PROG_GATE0_DIS_BIT = 0xD000,
	XSK_EFUSEPS_ERROR_WRTIE_PROG_GATE1_DIS_BIT = 0xD100,
	XSK_EFUSEPS_ERROR_WRTIE_PROG_GATE2_DIS_BIT = 0xD200,
	XSK_EFUSEPS_ERROR_WRTIE_SEC_LOCK_BIT = 0xD300,
	XSK_EFUSEPS_ERROR_WRTIE_PPK0_WR_LK_BIT = 0xD400,
	XSK_EFUSEPS_ERROR_WRTIE_PPK0_RVK_BIT = 0xD500,
	XSK_EFUSEPS_ERROR_WRTIE_PPK1_WR_LK_BIT = 0xD600,
	XSK_EFUSEPS_ERROR_WRTIE_PPK1_RVK_BIT = 0xD700,

	XSK_EFUSEPS_ERROR_WRITE_PUF_SYN_INVLD = 0xD800,
	XSK_EFUSEPS_ERROR_WRITE_PUF_SYN_WRLK = 0xD900,
	XSK_EFUSEPS_ERROR_WRITE_PUF_SYN_REG_DIS = 0xDA00,
	XSK_EFUSEPS_ERROR_WRITE_PUF_RESERVED_BIT = 0xDB00,
	XSK_EFUSEPS_ERROR_WRITE_LBIST_EN_BIT = 0xDC00,
	XSK_EFUSEPS_ERROR_WRITE_LPD_SC_EN_BIT = 0xDD00,
	XSK_EFUSEPS_ERROR_WRITE_FPD_SC_EN_BIT = 0xDE00,

	XSK_EFUSEPS_ERROR_WRITE_PBR_BOOT_ERR_BIT = 0xDF00,
	/* Error codes related to PUF */
	XSK_EFUSEPS_ERROR_PUF_INVALID_REG_MODE = 0xE000,
	XSK_EFUSEPS_ERROR_PUF_REG_WO_AUTH = 0xE100,
	XSK_EFUSEPS_ERROR_PUF_REG_DISABLED = 0xE200,
	XSK_EFUSEPS_ERROR_PUF_INVALID_REQUEST = 0xE300,
	XSK_EFUSEPS_ERROR_PUF_DATA_ALREADY_PROGRAMMED = 0xE400,
	XSK_EFUSEPS_ERROR_PUF_DATA_OVERFLOW	= 0xE500,
	XSK_EFUSEPS_ERROR_CMPLTD_EFUSE_PRGRM_WITH_ERR = 0x10000,
	/* If requested FUSE is write protected */
	XSK_EFUSEPS_ERROR_FUSE_PROTECTED = 0x00080000,
	/* If User requested to program USER FUSE to make Non-zero to 1 */
	XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT = 0x00800000,

}XSKEfusePs_ErrorCodes;

/**
 * ZynqMP PS BBRAM error codes
 */
typedef enum {
	XSK_ZYNQMP_BBRAMPS_ERROR_NONE = 0,
	XSK_ZYNQMP_BBRAMPS_ERROR_IN_PRGRMG_ENABLE = 0x01, /**< If this error is occurred
														  *  programming is not
														  *  possible */
	XSK_ZYNQMP_BBRAMPS_ERROR_IN_CRC_CHECK = 0xB000,  /**< If this error is occurred
													  *  programming is done but CRC
													  *  check is failed */
	XSK_ZYNQMP_BBRAMPS_ERROR_IN_PRGRMG = 0xC000		/**< programming of key is failed */
}XskZynqMp_Ps_Bbram_ErrorCodes;

/*
 * For backward compatibility with old error codes
 */

#define XSK_EFUSEPS_ERROR_READ_VCCAUX_VOLTAGE_OUT_OF_RANGE XSK_EFUSEPS_ERROR_READ_VCCPAUX_VOLTAGE_OUT_OF_RANGE
#define XSK_EFUSEPS_ERROR_READ_VCCINT_VOLTAGE_OUT_OF_RANGE XSK_EFUSEPS_ERROR_READ_VCCPINT_VOLTAGE_OUT_OF_RANGE
#define XSK_EFUSEPS_ERROR_WRITE_VCCAUX_VOLTAGE_OUT_OF_RANGE XSK_EFUSEPS_ERROR_WRITE_VCCPAUX_VOLTAGE_OUT_OF_RANGE
#define XSK_EFUSEPS_ERROR_WRITE_VCCINT_VOLTAGE_OUT_OF_RANGE XSK_EFUSEPS_ERROR_WRITE_VCCPINT_VOLTAGE_OUT_OF_RANGE
#define XilSKey_CrcCalculation XilSKey_CrcCalculation
#define Xilskey_Timer_Intialise	XilSKey_Timer_Intialise


/*****************************************************************************/
/**
*
* This macro reads the given register.
*
* @param	BaseAddress is the Xilinx base address of the eFuse or Bbram
*			controller.
* @param	RegOffset is the register offset of the register.
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XilSKey_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XilSKey_ReadReg(BaseAddress, RegOffset) \
		Xil_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This macro writes the value into the given register.
*
* @param	BaseAddress is the Xilinx base address of the eFuse or Bbram
*			controller.
* @param	RegOffset is the register offset of the register.
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XilSKey_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XilSKey_WriteReg(BaseAddress, RegOffset, Data) \
		Xil_Out32(((BaseAddress) + (u32)(RegOffset)), (u32)(Data))

/************************** Function Prototypes *****************************/
u32 XilSKey_EfusePs_XAdcInit (void );
void XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(XSKEfusePs_XAdc *XAdcInstancePtr);
u32 XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks();
void XilSKey_Efuse_StartTimer();
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
u32 XilSKey_Timer_Intialise();
u32 XilSKey_Efuse_ReverseHex(u32 Input);
void XilSKey_StrCpyRange(u8 *Src, u8 *Dst, u32 From, u32 To);
u32 XilSKey_CrcCalculation(u8 *Key);
u32 XilSkey_CrcCalculation_AesKey(u8 *Key);
u32 XilSKey_Ceil(float Freq);
/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XILSKEY_UTILS_H */
