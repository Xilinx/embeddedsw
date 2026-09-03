/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/******************************************************************************/
/**
*
* @file xnvm_efuse_common.h
*
* This file contains function declarations of eFUSE APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------------
* 3.0   kal  07/16/2022 Initial release
* 3.1   skg  10/25/2022 Added comments for macros and enums
* 3.2   kum  04/11/2023 Moved Voltage and Temp macros for LP,MP,HP ranges from versal xnvm_efuse.h
*	    vss  09/19/2023	Fixed MISRA-C Rule 8.7 violation
* 3.3   vss  12/31/2023 Added support for Program the eFuse protection bits only once
*       ng   11/22/2023 Fixed doxygen grouping
* 3.7   obs  03/26/2026 Updated function signatures to use XNvm_SysmonPsv type
*                       Added XNVM_SYSMON_* macros and XNvm_SysMon_*() inline
*                       wrappers using proper XNVM namespace
*
* </pre>
*
*******************************************************************************/

/**
 * @addtogroup xnvm_efuse_server_apis XilNvm eFUSE Server APIs
 * @{
 */

#ifndef XNVM_EFUSE_COMMON_H
#define XNVM_EFUSE_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_io.h"
#include "xil_types.h"
#include "xstatus.h"
#ifdef VERSAL_PLM
#include "xplmi_sysmon.h"
#else
#include "xsysmonpsv.h"
#endif
#include "xnvm_defs.h"
#include "xnvm_efuse_error.h"

/*************************** Constant Definitions *****************************/
/**
 *  @name eFuse ctrls constants
 *  @{
 */
#define XNVM_EFUSE_CTRL_WR_LOCKED	(0x01U)	/**< eFuse control write locked */
#define XNVM_EFUSE_CTRL_WR_UNLOCKED	(0x00U) /**< eFuse control write unlocked */
/** @} */

/**
 *  @name Temperature limits definition for versal Efuses
 *  @{
 */
#define XNVM_EFUSE_FULL_RANGE_TEMP_MIN	(-55.0f) /**< Minimum temperature for full range check */
#define XNVM_EFUSE_FULL_RANGE_TEMP_MAX	(125.0f) /**< Maximum temperature for full range check */

#define XNVM_EFUSE_TEMP_LP_MIN		(0.0f)   /**< Minimum temperature for low power range check */
#define XNVM_EFUSE_TEMP_LP_MAX		(100.0f) /**< Maximum temperature for low power range check */
#define XNVM_EFUSE_TEMP_MP_MIN		(-40.0f) /**< Minimum temperature for medium power range check */
#define XNVM_EFUSE_TEMP_MP_MAX		(110.0f) /**< Maximum temperature for medium power range check */
#define XNVM_EFUSE_TEMP_HP_MIN		(-55.0f) /**< Minimum temperature for high power range check */
#define XNVM_EFUSE_TEMP_HP_MAX		(125.0f) /**< Maximum temperature for high power range check */
/** @} */

/**
 *  @name eFuse range check definitions
 *  @{
 */
#define XNVM_EFUSE_FULL_RANGE_CHECK		(0U)	/**< To check both voltage
						* and temperature in full range */
#define XNVM_EFUSE_LP_RANGE_CHECK		(1U)	/**< To check both voltage
						* and temperature in low power range */
#define XNVM_EFUSE_MP_RANGE_CHECK		(2U)	/**< To check both voltage
						* and temperature in medium power range */
#define XNVM_EFUSE_HP_RANGE_CHECK		(3U)	/**< To check both voltage
						* and temperature in high power range */
/** @} */

/**
 *  @name eFuse voltage limits definitions
 *  @{
 */
#define XNVM_EFUSE_VCC_PMC_LP_MIN		(0.676f) /**< Minimum voltage for
						* low power range check */
#define XNVM_EFUSE_VCC_PMC_LP_MAX		(0.724f) /**< Maximum voltage for
						* low power range check */
#define XNVM_EFUSE_VCC_PMC_MP_MIN		(0.775f) /**< Minimum voltage for
						* medium power range check */
#define XNVM_EFUSE_VCC_PMC_MP_MAX		(0.825f) /**< Maximum voltage for
						* medium power range check */
#define XNVM_EFUSE_VCC_PMC_HP_MIN		(0.854f) /**< Minimum voltage for
						* high power range check */
#define XNVM_EFUSE_VCC_PMC_HP_MAX		(0.906f) /**< Maximum voltage for
						* high power range check */
/** @} */

#define XNVM_EFUSE_SYSMON_LOCK_CODE		(0xF9E8D7C6U)
			/**< System monitor lock code for eFUSE operations */

#define XNVM_EFUSE_PROTECTION_BIT_CLEAR		(0U)
                        /**< To check the corresponding protection eFuse is set or not */

/**
 *  @name SysMon register offset aliases for XilNvm
 *  @{
 */
#ifdef VERSAL_PLM
#define XNVM_SYSMON_PCSR_LOCK		XPLMI_SYSMON_PCSR_LOCK       /**< PCSR Lock register offset */
#define XNVM_SYSMON_NEW_DATA_FLAG0	XPLMI_SYSMON_NEW_DATA_FLAG0  /**< New Data Flag 0 register offset (Supply[0-31]) */
#define XNVM_SYSMON_NEW_DATA_FLAG1	XPLMI_SYSMON_NEW_DATA_FLAG1  /**< New Data Flag 1 register offset (Supply[32-63]) */
#define XNVM_SYSMON_SUPPLY_OFFSET	XPLMI_SYSMON_SUPPLY_OFFSET   /**< Supply register base offset */
#define XNVM_SYSMON_TEMP_SAT		XPLMI_SYSMON_TEMP_SAT        /**< Internal temperature register offset */
#else
#define XNVM_SYSMON_PCSR_LOCK		XSYSMONPSV_PCSR_LOCK         /**< PCSR Lock register offset */
#define XNVM_SYSMON_NEW_DATA_FLAG0	XSYSMONPSV_NEW_DATA_FLAG0    /**< New Data Flag 0 register offset (Supply[0-31]) */
#define XNVM_SYSMON_NEW_DATA_FLAG1	XSYSMONPSV_NEW_DATA_FLAG1    /**< New Data Flag 1 register offset (Supply[32-63]) */
#define XNVM_SYSMON_SUPPLY_OFFSET	XSYSMONPSV_SUPPLY            /**< Supply register base offset */
#define XNVM_SYSMON_TEMP_SAT		XSYSMONPSV_TEMP_SAT          /**< Internal temperature register offset */
#endif
/** @} */

/***************************** Type Definitions *******************************/
#ifdef VERSAL_PLM
typedef XPlmi_SysMonPsv XNvm_SysmonPsv; /**< SysMon PSV instance type */
#else
typedef XSysMonPsv XNvm_SysmonPsv; /**< SysMon PSV instance type */
#endif

/**
 * Operation mode
 */
typedef enum {
	XNVM_EFUSE_MODE_RD, /**< eFuse read mode */
	XNVM_EFUSE_MODE_PGM /**< eFuse program mode */
} XNvm_EfuseOpMode;

/**
 * Read mode
 */
typedef enum {
	XNVM_EFUSE_NORMAL_RD, /**< eFuse normal read */
	XNVM_EFUSE_MARGIN_RD /**< eFuse margin read */
} XNvm_EfuseRdMode;

/**
 * eFuse Page
 */
typedef enum {
	XNVM_EFUSE_PAGE_0 = 0, /**< Efuse page 0 */
	XNVM_EFUSE_PAGE_1, /**< Efuse page 1 */
	XNVM_EFUSE_PAGE_2, /**< Efuse page 2 */
	XNVM_EFUSE_PAGE_3, /**< Efuse page 3 */
	XNVM_EFUSE_PAGE_4, /**< Efuse page 4 */
	XNVM_EFUSE_PAGE_5 /**< Efuse page 5 */
} XNvm_EfuseType;

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
 * @brief  Converts raw AdcData into Voltage value.
 *
 * @param  AdcData	System Monitor ADC Raw Data.
 *
 * @return The Voltage in volts.
 *
 ****************************************************************************/
static inline float XNvm_SysMon_RawToVoltage(u32 AdcData)
{
#ifdef VERSAL_PLM
	return XPlmi_SysMon_RawToVoltage(AdcData);
#else
	return XSysMonPsv_RawToVoltage(AdcData);
#endif
}

/****************************************************************************/
/**
 * @brief  Converts the fixed point to degree celsius.
 *
 * @param  FixedQFmt	Q8.7 representation of temperature value.
 *
 * @return The Temperature in degree celsius.
 *
 ****************************************************************************/
static inline float XNvm_SysMon_FixedToFloat(u32 FixedQFmt)
{
#ifdef VERSAL_PLM
	return XPlmi_SysMon_FixedToFloat(FixedQFmt);
#else
	return XSysMonPsv_FixedToFloat(FixedQFmt);
#endif
}

/****************************************************************************/
/**
 * @brief  Reads the device temperature from SysMon registers.
 *
 * @param  InstancePtr	Pointer to the SysMon instance.
 *
 * @return Temperature value in signed Q8.7 format.
 *
 ****************************************************************************/
static inline u32 XNvm_SysMon_ReadDeviceTemp(
		XNvm_SysmonPsv *InstancePtr)
{
#ifdef VERSAL_PLM
	return XPlmi_SysMon_ReadDeviceTemp(InstancePtr);
#else
	return XSysMonPsv_ReadDeviceTemp((XSysMonPsv *)InstancePtr,
					 XSYSMONPSV_VAL);
#endif
}

/*************************** Function Prototypes ******************************/
int XNvm_EfuseCacheReload(void);
void XNvm_EfuseDisablePowerDown(void);
int  XNvm_EfuseSetReadMode(XNvm_EfuseRdMode RdMode);
void XNvm_EfuseSetRefClk(void);
void XNvm_EfuseEnableProgramming(void);
int XNvm_EfuseDisableProgramming(void);
int XNvm_EfuseResetReadMode(void);
void XNvm_EfuseInitTimers(void);
int XNvm_EfuseSetupController(XNvm_EfuseOpMode Op, XNvm_EfuseRdMode RdMode);
int XNvm_EfuseCheckForTBits(void);
u32 XNvm_GetSysmonSupplyRegId(UINTPTR SysmonpsvSatBaseAddr);
int XNvm_EfuseTempAndVoltChecks(const XNvm_SysmonPsv *SysMonInstPtr);
u32 XNvm_EfuseReadProtectionBits(u32 Mask);


#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_COMMON_H */

/** @} */
