/******************************************************************************
* Copyright (c) 2013 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xilskey_utils.h
 * @cond xilskey_internal
 * @{
 *
 * @note
 *
 *		For Ultrascale
 *	    ------------------------------------------------------------------------
 *	    If user wants to Debug and avoid writing Fuses, then user needs to
 *	    define "DEBUG_FUSE_WRITE_DISABLE" that will disable writing to Fuses.
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
* 6.6   vns     06/06/18 Added doxygen tags
*       vns     09/18/18 Added error code for zynqmp efuseps
*       vns     10/11/18 Added new error code for SPKID bit revert request
*                        XSK_EFUSEPS_ERROR_SPKID_BIT_CANT_REVERT
* 6.7   arc     01/05/19 Fixed MISRA-C violations.
*       arc     25/02/19 Added Timeout Macro and new error code for
*                        bbram zeroisation and error in write CRC
*                        XSK_ZYNQMP_BBRAMPS_ERROR_IN_ZEROISE
*                        XSK_ZYNQMP_BBRAMPS_ERROR_IN_WRITE_CRC
*       mmd     03/17/19 Added timeout and PUF underflow error
*       psl     03/19/19 FIxed MISRA-C violation
*       psl     03/29/19 Removed GPIO ID macro.
* 6.8   psl     06/07/19 Added doxygen tags
*       psl     08/12/19 Fixed MISRA-C violation
*       psl     08/23/19 Added Debug define to avoid writing of eFuse.
* 6.9   kpt     02/27/20 Added Error code XSK_EFUSEPS_ERROR_PGM_NOT_DONE.
*               02/27/20 Replaced XSYSMON_DEVICE_ID with
*                        XSYSMON_PSU_DEVICE_ID.
*       kpt     03/31/20 Added Error Codes
*                        XSK_EFUSEPS_RD_FROM_EFUSE_NOT_ALLOWED
*                        XSK_EFUSEPS_ERROR_PUF_AUX_ALREADY_PROGRAMMED
*                        XSK_EFUSEPS_ERROR_PUF_CHASH_ALREADY_PROGRAMMED
* 7.0   am      10/04/20 Resolved MISRA C violations
* 7.1   am      11/26/20 Resolved Coverity warnings
* 		kal	    02/28/21 Added Error code
* 			             XSK_EFUSEPS_PUF_CANT_BE_USED_FOR_USER_DATA
*       kpt     05/11/21 Added Error code XSK_EFUSEPS_ERROR_PUF_USER_DATA
*       kpt     05/21/21 Added Error Codes
*                        XSK_EFUSEPS_ERROR_PPK0_BIT_CANT_REVERT
*                        XSK_EFUSEPS_ERROR_PPK1_BIT_CANT_REVERT
*
* </pre>
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
#if defined (ARMR5) || defined (__aarch64__) || defined (ARMA53_32) || defined (PSU_PMU)
#include "xsysmonpsu.h"
#include "xplatform_info.h"
#else
#include "xadcps.h"
#endif
#endif
#include "xstatus.h"
#include "xil_util.h"

/************************** Constant Definitions ****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
#ifdef XPAR_XSK_MICROBLAZE_PLATFORM
#define XSK_MICROBLAZE_PLATFORM
#else
#define XSK_ARM_PLATFORM
#if defined (ARMR5) || defined (__aarch64__) || defined (ARMA53_32) || defined (PSU_PMU)
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

#ifdef DEBUG_FUSE_WRITE_DISABLE
#define XilsKey_DbgPrint	    xil_printf
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
#define XSYSMON_PSU_DEVICE_ID	XPAR_XSYSMONPSU_0_DEVICE_ID

/* ZynqMp efusePs ps Ref Clk frequency */
#define XSK_ZYNQMP_EFUSEPS_PS_REF_CLK_FREQ	XPAR_PSU_PSS_REF_CLK_FREQ_HZ
#endif

#ifdef XSK_MICROBLAZE_PLATFORM
#define XTMRCTR_DEVICE_ID		(XPAR_TMRCTR_0_DEVICE_ID)
#define XSK_EFUSEPL_CLCK_FREQ_ULTRA	(XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ)
#define XSK_TMRCTR_NUM			(0U)
#endif

#define REVERSE_POLYNOMIAL	(0x82F63B78U)
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
#define XSK_EFUSEPS_TEMP_MIN			(-40.0f)
#define XSK_EFUSEPS_TEMP_MAX			(125.0f)
#define XSK_EFUSEPS_READ_VPAUX_MIN		(1.71)
#define XSK_EFUSEPS_READ_VPAUX_MAX		(1.98)
#define XSK_EFUSEPS_WRITE_VPAUX_MIN	(1.71)
#define XSK_EFUSEPS_WRITE_VPAUX_MAX	(1.98)

/* ZynqMP eFUSE voltage ranges */
#define XSK_ZYNQMP_EFUSEPS_VCC_PSINTLP_MIN	(0.675)
#define XSK_ZYNQMP_EFUSEPS_VCC_PSINTLP_MAX	(0.935)
/* VCC AUX should be 1.8 +/-10% */
#define XSK_ZYNQMP_EFUSEPS_VCC_AUX_MIN		(1.62f)
#define XSK_ZYNQMP_EFUSEPS_VCC_AUX_MAX		(1.98f)

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
#define	XSK_EFUSEPL_WRITE_TEMP_MIN 				(15U)
#define	XSK_EFUSEPL_WRITE_TEMP_MAX				(125U)

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

#define XSK_EFUSE_DEBUG_GENERAL	0x00000001U    /* general debug  messages */

#if defined (XSK_EFUSE_DEBUG)
#define xeFUSE_dbg_current_types (XSK_EFUSE_DEBUG_GENERAL)
#else
#define xeFUSE_dbg_current_types 0U
#endif
#ifdef STDOUT_BASEADDRESS
#define xeFUSE_printf(type,...) \
		if (((type) & xeFUSE_dbg_current_types) != 0U)  {xil_printf (__VA_ARGS__); }
#else
#define xeFUSE_printf(type, ...)
#endif

#define XSK_GLOBAL_TIMER_BASE_ADDRESS           (0xF8F00000U)

/**
 * Global_Timer_Counter _Register0 (0xf8f00200)
 */
#define XSK_GLOBAL_TIMER_COUNT_REG_LOW          (XSK_GLOBAL_TIMER_BASE_ADDRESS + 0x200U)

/**
 * Global_Timer_Counter _Register1 (0xf8f00204)
 */
#define XSK_GLOBAL_TIMER_COUNT_REG_HIGH         (XSK_GLOBAL_TIMER_BASE_ADDRESS + 0x204U)

/**
 * Global_Timer_Control_Register (0xf8f00208)
 */
#define XSK_GLOBAL_TIMER_CTRL_REG               (XSK_GLOBAL_TIMER_BASE_ADDRESS + 0x208U)


/**
 * System Level Control Registers Start Addr
 */
#define XSK_SLCR_START_ADDRESS					(0xF8000000U)
/**
 * ARM PLL Control Register
 */
#define XSK_ARM_PLL_CTRL_REG					(XSK_SLCR_START_ADDRESS + 0x100U)
/**
 * ARM Clock Control Register
 */
#define XSK_ARM_CLK_CTRL_REG					(XSK_SLCR_START_ADDRESS + 0x120U)

/**
 *  PL eFUSE aes key size in characters
 */
#define XSK_EFUSEPL_AES_KEY_STRING_SIZE         	(64U)
/**
 *  PL eFUSE user low key size in characters
 */
#define XSK_EFUSEPL_USER_LOW_KEY_STRING_SIZE    	(2U)
/**
 *  PL eFUSE user high key size in characters
 */
#define XSK_EFUSEPL_USER_HIGH_KEY_STRING_SIZE   	(6U)
/**
 *  AES Key size in Bytes
 */
#define XSK_EFUSEPL_AES_KEY_SIZE_IN_BYTES		(32U)
/**
 *  32 bit User Key size in Bytes
 */
#define XSK_EFUSEPL_USER_KEY_SIZE_IN_BYTES		(4U)

/* 128 bit User key size in bytes */
#define XSK_EFUSEPL_128BIT_USERKEY_SIZE_IN_BYTES	(16U)
/**
 *  AES Key size in Bits
 */
#define XSK_EFUSEPL_AES_KEY_SIZE_IN_BITS			(256U)
/**
 *  User Low Key size in Bytes
 */
#define XSK_EFUSEPL_USER_LOW_KEY_SIZE_IN_BITS	(8U)
/**
 *  User High Key size in Bytes
 */
#define XSK_EFUSEPL_USER_HIGH_KEY_SIZE_IN_BITS	(24U)
/**
 * Key length definition for RSA KEY Hash
 */
#define XSK_EFUSEPS_RSA_KEY_HASH_LEN_IN_BYTES	(32U)
/**
 *  PS eFUSE RSA key Hash size in characters
 */
#define XSK_EFUSEPS_RSA_KEY_HASH_STRING_SIZE     (64U)
/**
 * Ultrascale Efuse PL RSA Key size in Bytes
 */
#define XSK_EFUSEPL_RSA_KEY_HASH_SIZE_IN_BYTES			(48U)
/**
 *  AES Key size in Chars
 */
#define XSK_EFUSEPS_AES_KEY_SIZE_IN_CHARS	(64U)

#define XSK_POLL_TIMEOUT	0xFFFFFFFFU

#define XSK_STRING_SIZE_2		(2U)
#define XSK_STRING_SIZE_6		(6U)
#define XSK_STRING_SIZE_8 		(8U)
#define XSK_STRING_SIZE_64		(64U)
#define XSK_STRING_SIZE_96		(96U)

/************************** Variable Declarations ****************************/
extern u32 TimerTicksfor1000ns;

/************************** Variable Definitions ****************************/
#ifdef XSK_MICROBLAZE_PLATFORM
typedef enum {
	 XSK_SLR_NUM_0,
	 XSK_SLR_NUM_1,
	 XSK_SLR_NUM_2,
	 XSK_SLR_NUM_3
}XSK_SlrNum;
typedef enum {
	 XSK_SLR_CONFIG_ORDER_0,
	 XSK_SLR_CONFIG_ORDER_1,
	 XSK_SLR_CONFIG_ORDER_2,
	 XSK_SLR_CONFIG_ORDER_3
}XSK_SlrCfgOrder;
typedef enum {
	XSK_TARGET_MAX_1_SLRS = 1,
	XSK_TARGET_MAX_2_SLRS,
	XSK_TARGET_MAX_3_SLRS,
	XSK_TARGET_MAX_4_SLRS
}XSK_MaxSlrs;
#endif

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
@}
@endcond */

/**
 * @addtogroup xilskey_plefuse_error_codes PL EFUSE error codes
 * @{
 */
typedef enum {
	XSK_EFUSEPL_ERROR_NONE = 0,	/**< 0 <br>No error. */

	/**
	 * @name EFUSE Read error codes
	 */
	XSK_EFUSEPL_ERROR_ROW_NOT_ZERO = 0x10,/**< 0x10 <br>Row is not zero. */
	XSK_EFUSEPL_ERROR_READ_ROW_OUT_OF_RANGE,/**< 0x11 <br>Read Row is out
						 * of range. */
	XSK_EFUSEPL_ERROR_READ_MARGIN_OUT_OF_RANGE,/**< 0x12 <br>Read Margin
						     *  is out of range. */
	XSK_EFUSEPL_ERROR_READ_BUFFER_NULL,	/**< 0x13 <br>No buffer
						  *  for read. */
	XSK_EFUSEPL_ERROR_READ_BIT_VALUE_NOT_SET,/**< 0x14 <br>Read bit
						   *  not set. */
	XSK_EFUSEPL_ERROR_READ_BIT_OUT_OF_RANGE,/**< 0x15 <br>Read bit is out
						  *  of range. */
	XSK_EFUSEPL_ERROR_READ_TMEPERATURE_OUT_OF_RANGE,/**< 0x16 <br>Temperature
							  *  obtained
							  *  from XADC is out
							  *  of range to read.*/
	XSK_EFUSEPL_ERROR_READ_VCCAUX_VOLTAGE_OUT_OF_RANGE,/**< 0x17 <br>VCCAUX
							     *  obtained
							     *  from XADC is
							     *  out of range to
							     *  read. */
	XSK_EFUSEPL_ERROR_READ_VCCINT_VOLTAGE_OUT_OF_RANGE,/**< 0x18 <br>VCCINT
							     *  obtained
							     *  from XADC is
							     *  out of range to
							     *  read. */

	/**
	 * @name EFUSE Write error codes
	 */

	XSK_EFUSEPL_ERROR_WRITE_ROW_OUT_OF_RANGE,/**< 0x19 <br>To write row
						   *  is out of range.  */
	XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE,/**< 0x1A <br>To read bit is
						   *  out of range. */
	XSK_EFUSEPL_ERROR_WRITE_TMEPERATURE_OUT_OF_RANGE,/**< 0x1B <br>To eFUSE
							   *  write Temperature
							   * obtained from XADC
							   * is outof range. */
	XSK_EFUSEPL_ERROR_WRITE_VCCAUX_VOLTAGE_OUT_OF_RANGE,
							/**< 0x1C <br>To
							  * write eFUSE
							  * VCCAUX obtained
							  * from XADC is
							  * out of range.*/
	XSK_EFUSEPL_ERROR_WRITE_VCCINT_VOLTAGE_OUT_OF_RANGE, /**< 0x1D <br>To
								* write into
								* eFUSE VCCINT
								* obtained from
								* XADC is out
								* of range. */

	/**
	 * @name EFUSE CNTRL error codes
	 */
	XSK_EFUSEPL_ERROR_FUSE_CNTRL_WRITE_DISABLED,/**< 0x1E <br>Fuse
						      *  control write
						      * is disabled. */
	XSK_EFUSEPL_ERROR_CNTRL_WRITE_BUFFER_NULL,/**< 0x1F <br>Buffer
						    *  pointer that is
						    *  supposed to
						    *  contain control data
						    *  is null.*/

	/**
	 * @name EFUSE KEY error codes
	 */
	XSK_EFUSEPL_ERROR_NOT_VALID_KEY_LENGTH, /**< 0x20 <br>Key length
						  *  invalid. */
	XSK_EFUSEPL_ERROR_ZERO_KEY_LENGTH, /**< 0x21 <br>Key length zero. */
	XSK_EFUSEPL_ERROR_NOT_VALID_KEY_CHAR, /**< 0x22 <br>Invalid key
						*  characters. */
	XSK_EFUSEPL_ERROR_NULL_KEY, /**< 0x23 <br>Null key. */
	/**
	 * @name SECURE KEY error codes
	 */
	 XSK_EFUSEPL_ERROR_FUSE_SEC_WRITE_DISABLED, /**< 0x24 <br>Secure bits
						      *  write is disabled. */
	 XSK_EFUSEPL_ERROR_FUSE_SEC_READ_DISABLED, /**< 0x25 <br>Secure bits
						     *  reading is disabled.*/
	 XSK_EFUSEPL_ERROR_SEC_WRITE_BUFFER_NULL, /**< 0x26 <br>Buffer to write
						    *  into secure block
						    *  is NULL. */

	XSK_EFUSEPL_ERROR_READ_PAGE_OUT_OF_RANGE, /**< 0x27 <br>Page is
						    *  out of range. */
	XSK_EFUSEPL_ERROR_FUSE_ROW_RANGE, /**< 0x28 <br>Row is out of range. */
	XSK_EFUSEPL_ERROR_IN_PROGRAMMING_ROW, /**< 0x29 <br>Error programming
						*  fuse row.*/
	XSK_EFUSEPL_ERROR_PRGRMG_ROWS_NOT_EMPTY, /**< 0x2A <br>Error when
						   *  tried to program non Zero
						    * rows of eFUSE.*/

	/* Error in Hw module */
	XSK_EFUSEPL_ERROR_HWM_TIMEOUT = 0x80, /**< 0x80 <br>Error when hardware
						*  module is exceeded the time
						*  for programming eFUSE.*/
	XSK_EFUSEPL_ERROR_USER_FUSE_REVERT = 0x90, /**< 0x90 <br>Error occurs
							* when user
							* requests to revert
							* already programmed
							* user eFUSE bit.*/

	/**
	 * @name XSKEfusepl_Program_Efuse() error codes
	 */
	XSK_EFUSEPL_ERROR_KEY_VALIDATION = 0xF000, /**< 0xF000<br>Invalid
						     *  key. */
	XSK_EFUSEPL_ERROR_PL_STRUCT_NULL = 0x1000, /**< 0x1000<br>Null PL
						     *  structure. */
	XSK_EFUSEPL_ERROR_JTAG_SERVER_INIT = 0x1100, /**< 0x1100<br>JTAG server
							* initialization
							* error. */
	XSK_EFUSEPL_ERROR_READING_FUSE_CNTRL = 0x1200, /**< 0x1200<br>Error
						  * reading fuse control. */
	XSK_EFUSEPL_ERROR_DATA_PROGRAMMING_NOT_ALLOWED = 0x1300,
				/**< 0x1300<br>Data programming not allowed. */
	XSK_EFUSEPL_ERROR_FUSE_CTRL_WRITE_NOT_ALLOWED = 0x1400,
				/**< 0x1400<br>Fuse control write is disabled.*/
	XSK_EFUSEPL_ERROR_READING_FUSE_AES_ROW = 0x1500, /**< 0x1500<br>Error
							   *  reading fuse
							   *  AES row. */
	XSK_EFUSEPL_ERROR_AES_ROW_NOT_EMPTY = 0x1600, /**< 0x1600<br>AES row
							*   is not empty. */
	XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_AES_ROW = 0x1700, /**< 0x1700<br>
							* Error programming fuse
							* AES row. */
	XSK_EFUSEPL_ERROR_READING_FUSE_USER_DATA_ROW = 0x1800,
						      /**< 0x1800<br>
							* Error reading fuse
							* user row. */
	XSK_EFUSEPL_ERROR_USER_DATA_ROW_NOT_EMPTY = 0x1900,
				/**< 0x1900<br>User row is not empty. */
	XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_DATA_ROW = 0x1A00,
							/**< 0x1A00<br>Error
							  * programming fuse
							  * user row. */
	XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_CNTRL_ROW = 0x1B00,/**< 0x1B00<br>
							*  Error programming
							*  fuse control row. */
	XSK_EFUSEPL_ERROR_XADC = 0x1C00, /**< 0x1C00<br>XADC error. */
	XSK_EFUSEPL_ERROR_INVALID_REF_CLK= 0x3000U,/**< 0x3000<br>Invalid
						    *  reference clock. */
	XSK_EFUSEPL_ERROR_FUSE_SEC_WRITE_NOT_ALLOWED = 0x1D00,
			/**< 0x1D00<br>Error in programming secure block. */
	XSK_EFUSEPL_ERROR_READING_FUSE_STATUS = 0x1E00,/**< 0x1E00<br>Error in
							 * reading FUSE
							 * status. */
	XSK_EFUSEPL_ERROR_FUSE_BUSY = 0x1F00, /**< 0x1F00<br>Fuse busy. */
	XSK_EFUSEPL_ERROR_READING_FUSE_RSA_ROW = 0x2000, /**< 0x2000<br>Error
							   *  in reading
							   *  FUSE RSA block. */
	XSK_EFUSEPL_ERROR_TIMER_INTIALISE_ULTRA = 0x2200, /**< 0x2200<br>Error
							    * in initiating
							    * Timer. */
	XSK_EFUSEPL_ERROR_READING_FUSE_SEC = 0x2300, /**< 0x2300<br>Error in
							* reading
							* FUSE secure bits. */
	XSK_EFUSEPL_ERROR_PRGRMG_FUSE_SEC_ROW = 0x2500, /**< 0x2500<br>Error
							  *  in programming
							  * Secure bits of
							  * efuse. */
	XSK_EFUSEPL_ERROR_PRGRMG_USER_KEY = 0x4000, /**< 0x4000<br>Error in
							* programming 32
							* bit user key. */
	XSK_EFUSEPL_ERROR_PRGRMG_128BIT_USER_KEY = 0x5000,
						/**< 0x5000<br>Error in
						  *  programming 128 bit
						  *  User key.*/
	XSK_EFUSEPL_ERROR_PRGRMG_RSA_HASH = 0x8000 /**< 0x8000<br>Error in
						     * programming RSA hash. */
}XSKEfusePl_ErrorCodes;
/**
 * @}
*/

/**
 * @addtogroup xilskey_psefuse_error_codes PS EFUSE error codes
 * @{
 */
typedef enum {
	XSK_EFUSEPS_ERROR_NONE = 0, /**< 0<br>No error. */

	/**
	 * @name EFUSE Read error codes
	 */
	XSK_EFUSEPS_ERROR_ADDRESS_XIL_RESTRICTED = 0x01, /**< 0x01<br>Address
							   *  is restricted. */
	XSK_EFUSEPS_ERROR_READ_TMEPERATURE_OUT_OF_RANGE, /**< 0x02<br>Temperature
							   *  obtained from XADC
							   *  is out of
							   *  range. */
	XSK_EFUSEPS_ERROR_READ_VCCPAUX_VOLTAGE_OUT_OF_RANGE,/**< 0x03<br>VCCAUX
							      *  obtained from
							      *  XADC is out of
							      *  range. */
	XSK_EFUSEPS_ERROR_READ_VCCPINT_VOLTAGE_OUT_OF_RANGE,/**< 0x04<br>VCCINT
							      *  obtained from
							      *  XADC is out of
							      *  range. */

	/**
	 * @name EFUSE Write error codes
	 */
	XSK_EFUSEPS_ERROR_WRITE_TEMPERATURE_OUT_OF_RANGE,/**< 0x05<br>
							   *  Temperature
							   *  obtained from
							   *  XADC is out
							   *  of range. */
	XSK_EFUSEPS_ERROR_WRITE_VCCPAUX_VOLTAGE_OUT_OF_RANGE,/**< 0x06<br>VCCAUX
								* obtained from
								* XADC is out of
								* range. */
	XSK_EFUSEPS_ERROR_WRITE_VCCPINT_VOLTAGE_OUT_OF_RANGE,/**< 0x07<br>VCCINT
								* obtained from
								* XADC is out of
								* range. */
	XSK_EFUSEPS_ERROR_VERIFICATION,			/**< 0x08<br>Verification
							  *  error. */
	XSK_EFUSEPS_ERROR_RSA_HASH_ALREADY_PROGRAMMED,/**< 0x09<br>RSA hash was
							* already programmed. */

	/**
	 * @name FUSE CNTRL error codes
	 */
	XSK_EFUSEPS_ERROR_CONTROLLER_MODE,/**< 0x0A<br>Controller mode error */
	XSK_EFUSEPS_ERROR_REF_CLOCK,/**< 0x0B<br>Reference clock not between
				      *  20 to 60MHz */
	XSK_EFUSEPS_ERROR_READ_MODE,/**< 0x0C<br>Not supported read mode */

	/**
	 * @name XADC Error Codes
	 */
	XSK_EFUSEPS_ERROR_XADC_CONFIG,/**< 0x0D<br>XADC configuration error. */
	XSK_EFUSEPS_ERROR_XADC_INITIALIZE,/**< 0x0E<br>XADC initialization
					    *  error. */
	XSK_EFUSEPS_ERROR_XADC_SELF_TEST,/**< 0x0F<br>XADC self-test failed. */

	/**
	 * @name Utils Error Codes
	 */
	XSK_EFUSEPS_ERROR_PARAMETER_NULL,/**< 0x10<br>Passed parameter null. */
	XSK_EFUSEPS_ERROR_STRING_INVALID,/**< 0x20<br>Passed string
					   *  is invalid. */

	XSK_EFUSEPS_ERROR_AES_ALREADY_PROGRAMMED,/**< 0x12<br>AES key is
						   *  already programmed.*/
	XSK_EFUSEPS_ERROR_SPKID_ALREADY_PROGRAMMED,/**< 0x13<br>SPK ID is
						     *  already programmed. */
	XSK_EFUSEPS_ERROR_PPK0_HASH_ALREADY_PROGRAMMED,/**< 0x14<br>PPK0 hash
							*  is already
							*  programmed. */
	XSK_EFUSEPS_ERROR_PPK1_HASH_ALREADY_PROGRAMMED,/**< 0x15<br>PPK1 hash
							*  is already
							*  programmed. */

	XSK_EFUSEPS_ERROR_IN_TBIT_PATTERN,/**< 0x16<br>Error in
							*  TBITS pattern . */
	XSK_EFUSEPS_ERROR_INVALID_PARAM,/**< 0x17<br>Error for
                                         *  invalid parameters. */

	XSK_EFUSEPS_ERROR_PROGRAMMING = 0x00A0U,/**< 0x00A0<br>Error in
						 *  programming eFUSE.*/
	XSK_EFUSEPS_ERROR_PGM_NOT_DONE = 0X00A1,/**< 0x00A1<br>Program not done */
	XSK_EFUSEPS_ERROR_READ = 0x00B0U,/**< 0x00B0<br>Error in reading. */
	XSK_EFUSEPS_ERROR_BYTES_REQUEST = 0x00C0U, /**< 0x00C0<br>Error in
						* requested byte count. */
	XSK_EFUSEPS_PUF_CANT_BE_USED_FOR_USER_DATA = 0x00C1U, /**<0x00C0<br>Error
						* when requested for PUF HD
						* eFuses programming for user
						* data, but as Chash is already
						* programmed which means that
						* PUF HD is already programmed
						* with syndrome data */
	XSK_EFUSEPS_ERROR_PUF_USER_DATA = 0x00C2U, /**<0x00C2<br>Error
						* when requested for PUF HD
						* eFuses programming for user
						* data, data provided for Row 0 of
						* efuse page 2 or page 3 or data provided
						* for Row 63 of efuse page 3 is not valid */
	XSK_EFUSEPS_ERROR_RESRVD_BITS_PRGRMG = 0x00D0U, /**< 0x00D0<br>Error in
						* programming reserved bits. */
	XSK_EFUSEPS_ERROR_ADDR_ACCESS = 0x00E0U, /**< 0x00E0<br>Error in
							* accessing requested address. */
	XSK_EFUSEPS_ERROR_READ_NOT_DONE = 0x00F0U,/**< 0x00F0<br>Read not done */

	/**
	 * @name XSKEfuse_Write/Read()common error codes
	 */
	XSK_EFUSEPS_ERROR_PS_STRUCT_NULL=0x8100U,/**< 0x8100<br>PS structure
						  *  pointer is null. */
	XSK_EFUSEPS_ERROR_XADC_INIT=0x8200U,/**< 0x8200<br>XADC initialization
					     *  error. */
	XSK_EFUSEPS_ERROR_CONTROLLER_LOCK=0x8300U,/**< 0x8300<br>PS eFUSE
						   *  controller is locked. */
	XSK_EFUSEPS_ERROR_EFUSE_WRITE_PROTECTED=0x8400U,/**< 0x8400<br>PS eFUSE
							*  is write protected.*/
	XSK_EFUSEPS_ERROR_CONTROLLER_CONFIG=0x8500U,/**< 0x8500<br>Controller
						     *  configuration error. */
	XSK_EFUSEPS_ERROR_PS_PARAMETER_WRONG=0x8600U,/**< 0x8600<br>PS eFUSE
						      *  parameter is not
						      *  TRUE/FALSE. */

	/**
	 * @name XSKEfusePs_Write() error codes
	 */
	XSK_EFUSEPS_ERROR_WRITE_128K_CRC_BIT=0x9100U,/**< 0x9100<br>Error in
						      *  enabling 128K CRC. */
	XSK_EFUSEPS_ERROR_WRITE_NONSECURE_INITB_BIT=0x9200U,/**< 0x9200<br>Error
							     *  in programming
							     *  NON secure bit.
							     */
	XSK_EFUSEPS_ERROR_WRITE_UART_STATUS_BIT=0x9300U,/**< 0x9300<br>Error in
							 *  writing UART
							 *  status bit. */
	XSK_EFUSEPS_ERROR_WRITE_RSA_HASH=0x9400U,/**< 0x9400<br>Error in
						  *  writing RSA key. */
	XSK_EFUSEPS_ERROR_WRITE_RSA_AUTH_BIT=0x9500U, /**< 0x9500<br>Error in
							*  enabling RSA
							* authentication bit. */
	XSK_EFUSEPS_ERROR_WRITE_WRITE_PROTECT_BIT=0x9600U,/**< 0x9600<br>Error in
							   *  writing
							   *  write-protect bit. */
	XSK_EFUSEPS_ERROR_READ_HASH_BEFORE_PROGRAMMING=0x9700U, /**< 0x9700<br>
						* Check RSA key before trying
						* to program. */

	XSK_EFUSEPS_ERROR_WRTIE_DFT_JTAG_DIS_BIT = 0x9800U,/**< 0x9800<br>Error
							    *  in programming
							    *  DFT JTAG disable
							    *  bit. */
	XSK_EFUSEPS_ERROR_WRTIE_DFT_MODE_DIS_BIT = 0x9900U,/**< 0x9900<br>Error
							    *  in programming
							    *  DFT MODE
							    *  disable bit. */
	XSK_EFUSEPS_ERROR_WRTIE_AES_CRC_LK_BIT = 0x9A00U,/**< 0x9A00<br>Error in
							  *  enabling AES's CRC
							  *  check lock. */
	XSK_EFUSEPS_ERROR_WRTIE_AES_WR_LK_BIT = 0x9B00U,/**< 0x9B00<br>Error in
							 *  programming AES
							 *  write lock bit. */
	XSK_EFUSEPS_ERROR_WRTIE_USE_AESONLY_EN_BIT = 0x9C00U,/**< 0x9C00<br>Error
							      *  in programming
							      *  use AES only
							      *  bit. */
	XSK_EFUSEPS_ERROR_WRTIE_BBRAM_DIS_BIT = 0x9D00U,/**< 0x9D00<br>Error in
							 *  programming BBRAM
							 *  disable bit. */
	XSK_EFUSEPS_ERROR_WRTIE_PMU_ERR_DIS_BIT = 0x9E00U,/**< 0x9E00<br>Error
							   *  in programming
							   *  PMU error disable
							   *  bit. */
	XSK_EFUSEPS_ERROR_WRTIE_JTAG_DIS_BIT = 0x9F00U,/**< 0x9F00<br>Error in
							*  programming JTAG
							*  disable bit. */

	/**
	 * @name XSKEfusePs_Read() error codes
	 */
	XSK_EFUSEPS_ERROR_READ_RSA_HASH=0xA100U,/**< 0xA100<br>Error in reading
						 *  RSA key. */

	XSK_EFUSEPS_ERROR_WRONG_TBIT_PATTERN = 0xA200U,/**< 0xA200<br>Error in
							*  programming TBIT
							*  pattern. */
	XSK_EFUSEPS_ERROR_WRITE_AES_KEY = 0xA300U,/**< 0xA300<br>Error in
						   *  programming AES key. */
	XSK_EFUSEPS_ERROR_WRITE_SPK_ID = 0xA400U,/**< 0xA400<br>Error in
						  *  programming SPK ID. */
	XSK_EFUSEPS_ERROR_WRITE_USER_KEY = 0xA500U,/**< 0xA500<br>Error in
						    *  programming USER key.*/
	XSK_EFUSEPS_ERROR_WRITE_PPK0_HASH = 0xA600U,/**< 0xA600<br>Error in
						     * programming PPK0 hash. */
	XSK_EFUSEPS_ERROR_WRITE_PPK1_HASH = 0xA700U,/**< 0xA700<br>Error in
						     *  programming PPK1 hash.*/

	/* Error in programmin user fuses */
	XSK_EFUSEPS_ERROR_WRITE_USER0_FUSE = 0xC000U,/**< 0xC000<br>Error in
					   *  programming USER 0 Fuses. */
	XSK_EFUSEPS_ERROR_WRITE_USER1_FUSE = 0xC100U,/**< 0xC100<br>Error in
					   *  programming USER 1 Fuses. */
	XSK_EFUSEPS_ERROR_WRITE_USER2_FUSE = 0xC200U,/**< 0xC200<br>Error in
					   *  programming USER 2 Fuses. */
	XSK_EFUSEPS_ERROR_WRITE_USER3_FUSE = 0xC300U,/**< 0xC300<br>Error in
					   *  programming USER 3 Fuses. */
	XSK_EFUSEPS_ERROR_WRITE_USER4_FUSE = 0xC400U,/**< 0xC400<br>Error in
					   *  programming USER 4 Fuses. */
	XSK_EFUSEPS_ERROR_WRITE_USER5_FUSE = 0xC500U,/**< 0xC500<br>Error in
					   *  programming USER 5 Fuses. */
	XSK_EFUSEPS_ERROR_WRITE_USER6_FUSE = 0xC600U,/**< 0xC600<br>Error in
					  *  programming USER 6 Fuses. */
	XSK_EFUSEPS_ERROR_WRITE_USER7_FUSE = 0xC700U,/**< 0xC700<br>Error in
					  *  programming USER 7 Fuses. */

	XSK_EFUSEPS_ERROR_WRTIE_USER0_LK_BIT = 0xC800U,/**< 0xC800<br>Error in
				*   programming USER 0 fuses lock bit. */
	XSK_EFUSEPS_ERROR_WRTIE_USER1_LK_BIT = 0xC900U,/**< 0xC900<br>Error in
				*   programming USER 1 fuses lock bit.*/
	XSK_EFUSEPS_ERROR_WRTIE_USER2_LK_BIT = 0xCA00U,/**< 0xCA00<br>Error in
				*   programming USER 2 fuses lock bit.*/
	XSK_EFUSEPS_ERROR_WRTIE_USER3_LK_BIT = 0xCB00U,/**< 0xCB00<br>Error in
				*   programming USER 3 fuses lock bit.*/
	XSK_EFUSEPS_ERROR_WRTIE_USER4_LK_BIT = 0xCC00U,/**< 0xCC00<br>Error in
				*   programming USER 4 fuses lock bit.*/
	XSK_EFUSEPS_ERROR_WRTIE_USER5_LK_BIT = 0xCD00U,/**< 0xCD00<br>Error in
				*   programming USER 5 fuses lock bit.*/
	XSK_EFUSEPS_ERROR_WRTIE_USER6_LK_BIT = 0xCE00U,/**< 0xCE00<br>Error in
				*   programming USER 6 fuses lock bit.*/
	XSK_EFUSEPS_ERROR_WRTIE_USER7_LK_BIT = 0xCF00U,/**< 0xCF00<br>Error in
				*   programming USER 7 fuses lock bit.*/

	XSK_EFUSEPS_ERROR_WRTIE_PROG_GATE0_DIS_BIT = 0xD000U,/**< 0xD000<br>
					*   Error in programming PROG_GATE0
					*   disabling bit. */
	XSK_EFUSEPS_ERROR_WRTIE_PROG_GATE1_DIS_BIT = 0xD100U,/**< 0xD100<br>
					*  Error in programming PROG_GATE1
					*  disabling bit. */
	XSK_EFUSEPS_ERROR_WRTIE_PROG_GATE2_DIS_BIT = 0xD200U,/**< 0xD200<br>Error
					*  in programming PROG_GATE2
					*  disabling bit. */
	XSK_EFUSEPS_ERROR_WRTIE_SEC_LOCK_BIT = 0xD300U,/**< 0xD300<br>Error in
					*  programming SEC_LOCK bit. */
	XSK_EFUSEPS_ERROR_WRTIE_PPK0_WR_LK_BIT = 0xD400U,/**< 0xD400<br>Error in
					*  programming PPK0 write lock bit. */
	XSK_EFUSEPS_ERROR_WRTIE_PPK0_RVK_BIT = 0xD500U,/**< 0xD500<br>Error in
					*  programming PPK0 revoke bit. */
	XSK_EFUSEPS_ERROR_WRTIE_PPK1_WR_LK_BIT = 0xD600U,/**< 0xD600<br>Error in
					*  programming PPK1 write lock bit.*/
	XSK_EFUSEPS_ERROR_WRTIE_PPK1_RVK_BIT = 0xD700U,/**< 0xD700<br>Error in
					*  programming PPK0 revoke bit.*/

	XSK_EFUSEPS_ERROR_WRITE_PUF_SYN_INVLD = 0xD800U,/**< 0xD800<br>Error
					*  while programming the
					*  PUF syndrome invalidate bit.*/
	XSK_EFUSEPS_ERROR_WRITE_PUF_SYN_WRLK = 0xD900U,/**< 0xD900<br>Error while
					*  programming Syndrome write
					*  lock bit. */
	XSK_EFUSEPS_ERROR_WRITE_PUF_SYN_REG_DIS = 0xDA00U,/**< 0xDA00<br>Error
					*  while programming PUF syndrome
					*  register disable bit. */
	XSK_EFUSEPS_ERROR_WRITE_PUF_RESERVED_BIT = 0xDB00U,/**< 0xDB00<br>Error
					*  while programming PUF reserved bit. */
	XSK_EFUSEPS_ERROR_WRITE_LBIST_EN_BIT = 0xDC00U,/**< 0xDC00<br>Error
					*  while programming LBIST enable bit. */
	XSK_EFUSEPS_ERROR_WRITE_LPD_SC_EN_BIT = 0xDD00U,/**< 0xDD00<br>Error while
					* programming LPD SC enable bit. */
	XSK_EFUSEPS_ERROR_WRITE_FPD_SC_EN_BIT = 0xDE00U,/**< 0xDE00<br>Error while
					* programming FPD SC enable bit. */

	XSK_EFUSEPS_ERROR_WRITE_PBR_BOOT_ERR_BIT = 0xDF00U,/**< 0xDF00<br>Error
					* while programming PBR boot error bit. */
	/* Error codes related to PUF */
	XSK_EFUSEPS_ERROR_PUF_INVALID_REG_MODE = 0xE000U,/**< 0xE000<br>Error when
					* PUF registration is requested
					* with invalid registration mode. */
	XSK_EFUSEPS_ERROR_PUF_REG_WO_AUTH = 0xE100U,/**< 0xE100<br>Error when
					*  write not allowed without
					*  authentication enabled. */
	XSK_EFUSEPS_ERROR_PUF_REG_DISABLED = 0xE200U,/**< 0xE200<br>Error when
					*  trying to do PUF registration
					*  and when PUF registration is
					*  disabled. */
	XSK_EFUSEPS_ERROR_PUF_INVALID_REQUEST = 0xE300U,/**< 0xE300<br>Error
					*  when an invalid mode is requested. */
	XSK_EFUSEPS_ERROR_PUF_DATA_ALREADY_PROGRAMMED = 0xE400U,/**< 0xE400<br>
					*  Error when PUF is already programmed
					*  in eFUSE. */
	XSK_EFUSEPS_ERROR_PUF_DATA_OVERFLOW	= 0xE500U,/**< 0xE500<br>Error
					*  when an over flow occurs. */
	XSK_EFUSEPS_ERROR_SPKID_BIT_CANT_REVERT = 0xE600U,/**< 0xE600<br>
					*  Already programmed SPKID bit
					*  cannot be reverted */
	XSK_EFUSEPS_ERROR_PUF_DATA_UNDERFLOW = 0xE700U,/**< 0xE700<br>Error
					*  when an under flow occurs. */
	XSK_EFUSEPS_ERROR_PUF_TIMEOUT = 0xE800U,/**< 0xE800<br>Error
					*  when an PUF generation timedout. */
	XSK_EFUSEPS_ERROR_PUF_ACCESS = 0xE900,/**< 0xE900<br>Error
					*  when an PUF Access violation. */
	XSK_EFUSEPS_ERROR_PUF_CHASH_ALREADY_PROGRAMMED = 0XEA00,/**< 0xEA00<br>Error
					*  When PUF Chash already programmed
					*  in eFuse. */
	XSK_EFUSEPS_ERROR_PUF_AUX_ALREADY_PROGRAMMED = 0XEB00,/**< 0xEB00<br>Error
					*  When PUF AUX already programmed
					* in eFuse. */
	XSK_EFUSEPS_ERROR_PPK0_BIT_CANT_REVERT = 0xEC00U,/**< 0xEC00<br>
					*  Already programmed PPK0 Hash bit
					*  cannot be reverted */
	XSK_EFUSEPS_ERROR_PPK1_BIT_CANT_REVERT = 0xED00U,/**< 0xEC00<br>
					*  Already programmed PPK1 Hash bit
					*  cannot be reverted */

	XSK_EFUSEPS_ERROR_CMPLTD_EFUSE_PRGRM_WITH_ERR = 0x10000U,/**< 0x10000<br>
					*  eFUSE programming is completed with
					*  temp and vol read errors. */

	XSK_EFUSEPS_ERROR_CACHE_LOAD = 0x20000U,/**< 0x20000U<br>Error in
						*  re-loading CACHE. */
	XSK_EFUSEPS_RD_FROM_EFUSE_NOT_ALLOWED = 0x30000U,/**< 0x30000U<br>Read
								* from eFuse is
								* not allowed. */
	/* If requested FUSE is write protected */
	XSK_EFUSEPS_ERROR_FUSE_PROTECTED = 0x00080000U,/**< 0x00080000
					*  <br>Requested eFUSE is write
					*  protected. */
	/* If User requested to program USER FUSE to make Non-zero to 1 */
	XSK_EFUSEPS_ERROR_USER_BIT_CANT_REVERT = 0x00800000U,/**< 0x00800000<br>
					* Already programmed user FUSE bit
					* cannot be reverted.*/
	XSK_EFUSEPS_ERROR_BEFORE_PROGRAMMING = 0x08000000U,/**< 0x08000000U<br>Error
								*  occurred before
								*  programming. */
}XSKEfusePs_ErrorCodes;
 /** @} */

 /**
 * @addtogroup xilskey_zmpsoc_ps_error_codes Zynq UltraScale+ MPSoC error codes
 * @{
 */
typedef enum {
	XSK_ZYNQMP_BBRAMPS_ERROR_NONE = 0U, /**< 0<br>No error. */
	XSK_ZYNQMP_BBRAMPS_ERROR_IN_PRGRMG_ENABLE = 0x010U, /**< 0x010<br>If this
							  *  error is occurred
							  *  programming is not
							  *  possible. */
	XSK_ZYNQMP_BBRAMPS_ERROR_IN_ZEROISE = 0x20U,      /**< 0x20<br>
							   *  zeroize bbram is
							   *	failed. */
	XSK_ZYNQMP_BBRAMPS_ERROR_IN_CRC_CHECK = 0xB000U,  /**< 0xB000<br>If this
							  *  error is occurred
							  *  programming is done
							  *  but CRC
							  *  check is failed. */
	XSK_ZYNQMP_BBRAMPS_ERROR_IN_PRGRMG = 0xC000U,		/**< 0xC000<br>
							  *  programming of key
							  *  is failed. */
	XSK_ZYNQMP_BBRAMPS_ERROR_IN_WRITE_CRC = 0xE800U    /**< 0xE800<br>
							    * error write CRC
							    * value. */
}XskZynqMp_Ps_Bbram_ErrorCodes;
 /** @} */
/** @cond xilskey_internal
 * @{
 */
/*
 * For backward compatibility with old error codes
 */

#define XSK_EFUSEPS_ERROR_READ_VCCAUX_VOLTAGE_OUT_OF_RANGE XSK_EFUSEPS_ERROR_READ_VCCPAUX_VOLTAGE_OUT_OF_RANGE
#define XSK_EFUSEPS_ERROR_READ_VCCINT_VOLTAGE_OUT_OF_RANGE XSK_EFUSEPS_ERROR_READ_VCCPINT_VOLTAGE_OUT_OF_RANGE
#define XSK_EFUSEPS_ERROR_WRITE_VCCAUX_VOLTAGE_OUT_OF_RANGE XSK_EFUSEPS_ERROR_WRITE_VCCPAUX_VOLTAGE_OUT_OF_RANGE
#define XSK_EFUSEPS_ERROR_WRITE_VCCINT_VOLTAGE_OUT_OF_RANGE XSK_EFUSEPS_ERROR_WRITE_VCCPINT_VOLTAGE_OUT_OF_RANGE
#define Xilskey_CrcCalculation XilSKey_CrcCalculation
#define Xilskey_Timer_Intialise	XilSKey_Timer_Intialise
#define XilSKey_Ceil Xil_Ceil


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
#if defined (XSK_ZYNQ_ULTRA_MP_PLATFORM) && !defined (XSK_OVERRIDE_SYSMON_CFG)
u32 XilSKey_EfusePs_XAdcCfgValidate (void);
#endif
void XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(
		XSKEfusePs_XAdc *XAdcInstancePtr);
u32 XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks(void);
void XilSKey_Efuse_StartTimer(void);
u64 XilSKey_Efuse_GetTime(void);
void XilSKey_Efuse_SetTimeOut(volatile u64* t, u64 us);
u8 XilSKey_Efuse_IsTimerExpired(u64 t);
void XilSKey_Efuse_ConvertBitsToBytes(const u8 * Bits, u8 * Bytes, u32 Len);
void XilSKey_EfusePs_ConvertBytesToBits(const u8 * Bytes, u8 * Bits, u32 Len);
void XilSKey_EfusePs_ConvertBytesBeToLe(const u8 *Be, u8 *Le, u32 Len);
u32 XilSKey_Efuse_ValidateKey(const char *Key, u32 Len);
u32 XilSKey_Efuse_IsValidChar(const char *c);
u32 XilSKey_RowCrcCalculation(u32 PrevCRC, u32 Data, u32 Addr);
/**
 * Common functions
 */
u32 XilSKey_Efuse_ConvertStringToHexLE(const char * Str, u8 * Buf, u32 Len);
u32 XilSKey_Efuse_ConvertStringToHexBE(const char * Str, u8 * Buf, u32 Len);
u32 XilSKey_Efuse_ValidateKey(const char *Key, u32 Len);
u32 XilSKey_Timer_Intialise(void);
u32 XilSKey_Efuse_ReverseHex(u32 Input);
#ifdef XSK_MICROBLAZE_PLATFORM
void XilSKey_GetSlrNum(u32 MasterSlr, u32 ConfigOrderIndex, u32 *SlrNum);
#endif
 /** @}
@endcond */
/**
 * @addtogroup xilskey_cmn_crc xilskey common file
 * @{
*/
u32 XilSKey_CrcCalculation(const u8 *Key);
u32 XilSkey_CrcCalculation_AesKey(const u8 *Key);
/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XILSKEY_UTILS_H */
/**@}*/
