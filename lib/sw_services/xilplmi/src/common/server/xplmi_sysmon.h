/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_sysmon.h
*
* This is the header file which contains definitions for the SysMon manager.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  sn   07/01/2019 Initial release
*       sn   07/04/2019 Enabled SysMon's over-temperature interrupt
*       sn   08/03/2019 Added code to wait until over-temperature condition
*                       gets resolved before restart
* 1.01  bsv  04/04/2020 Code clean up
* 1.02  bm   10/14/2020 Code clean up
* 1.03  bm   02/08/2021 Add GetSysmonInst API
* 1.04  bsv  07/16/2021 Fix doxygen warnings
*       is   01/10/2022 Updated XPlmi_SysMonOTDetect API to pass wait time arg
*       is   01/10/2022 Updated Copyright Year to 2022
*       ma   08/08/2022 Fix SW-BP-MAGIC-NUM warning
*       mss  03/13/2024 MISRA-C violation Rule 17.8 fixed
* 2.40  obs  02/12/2025 Removed XSysMonPsv driver dependency
* 2.5   abh  09/22/2026 Fixed Doxygen Warnings
*
* </pre>
*
******************************************************************************/

#ifndef XPLMI_SYSMON_H
#define XPLMI_SYSMON_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_io.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/

/** @name SysMon Base Address and Register Offsets
 * @{
 */
#define XPLMI_SYSMON_BASEADDR			(0xF1270000U) /**< SysMon Base Address */
#define XPLMI_SYSMON_PCSR_MASK			(0x00000000U) /**< PCSR Mask Register Offset */
#define XPLMI_SYSMON_PCSR_CONTROL		(0x00000004U) /**< PCSR Control Register Offset */
#define XPLMI_SYSMON_PCSR_LOCK			(0x0000000CU) /**< PCSR Lock Register Offset */
#define XPLMI_SYSMON_ISR_OFFSET			(0x00000044U) /**< Interrupt Status Register Offset */
#define XPLMI_SYSMON_IMR0_OFFSET		(0x00000048U) /**< Interrupt Mask 0 Register Offset */
#define XPLMI_SYSMON_IER0_OFFSET		(0x0000004CU) /**< Interrupt Enable 0 Register Offset */
#define XPLMI_SYSMON_IDR0_OFFSET		(0x00000050U) /**< Interrupt Disable 0 Register Offset */
#define XPLMI_SYSMON_DEVICE_TEMP_OFFSET		(0x00001030U) /**< Device Temperature Register Offset */
#define XPLMI_SYSMON_SUPPLY_OFFSET		(0x00001040U) /**< Supply Register Base Offset */
#define XPLMI_SYSMON_NEW_DATA_FLAG0		(0x00001000U) /**< New Data Flag 0 Register Offset (Supply[0-31]) */
#define XPLMI_SYSMON_NEW_DATA_FLAG1		(0x00001004U) /**< New Data Flag 1 Register Offset (Supply[32-63]) */
#define XPLMI_SYSMON_TEMP_SAT			(0x00001FACU) /**< Internal Temperature Register */
/** @} */

/** @name SysMon Interrupt and Control Masks
 * @{
 */
#define XPLMI_SYSMON_IER0_OT_MASK		(0x00000100U) /**< Over Temperature Enable Mask */
#define XPLMI_SYSMON_ISR_OT_MASK		(0x00000100U) /**< Over Temperature Status Mask */
/** @} */

/** @name SysMon Supply Format Constants
 * @{
 */
#define XPLMI_SYSMON_SUPPLY_MANTISSA_MASK	(0x0000FFFFU) /**< Supply Mantissa Mask */
#define XPLMI_SYSMON_SUPPLY_FMT_MASK		(0x00010000U) /**< Supply Format Mask */
#define XPLMI_SYSMON_SUPPLY_FMT_SHIFT		(16U) /**< Supply Format Shift */
#define XPLMI_SYSMON_SUPPLY_MODE_MASK		(0x00060000U) /**< Supply Mode Mask */
#define XPLMI_SYSMON_SUPPLY_MODE_SHIFT		(17U) /**< Supply Mode Shift */
#define XPLMI_SYSMON_SUPPLY_MANTISSA_SIGN	(15U) /**< Supply Mantissa Sign Bit */
/** @} */

/** @name Temperature Format Constants
 * @{
 */
#define XPLMI_SYSMON_EXPONENT_RANGE_16		(16U) /**< Voltage exponent value bit */
#define XPLMI_SYSMON_QFMT_SIGN			(15U) /**< Q format signed bit */
#define XPLMI_SYSMON_QFMT_FRACTION		(128U) /**< Q format fractional value */
/** @} */

/** @name PCSR Lock/Unlock Values
 * @{
 */
#define XPLMI_SYSMON_PCSR_UNLOCK_VAL		(0xF9E8D7C6U) /**< PCSR Unlock Value */
#define XPLMI_SYSMON_PCSR_LOCK_VAL		(0x00000000U) /**< PCSR Lock Value */
/** @} */

/** @name Miscellaneous Constants
 * @{
 */
#define XPLMI_SYSMON_NO_WAIT_TIME		(0x0U) /**< No wait time */
#define XPLMI_SYSMON_MAX_SUPPLIES		(160U) /**< Max number of supplies */
/** @} */

/**************************** Type Definitions *******************************/

/**
 * @brief Simplified SysMon configuration - only base address needed for PLM
 */
typedef struct {
	UINTPTR BaseAddress; /**< Register base address */
	u8 Supply_List[XPLMI_SYSMON_MAX_SUPPLIES]; /**< Supply list (for compatibility) */
} XPlmi_SysMonPsv_Config;

/**
 * @brief Simplified SysMon instance - only base address and ready flag needed
 */
typedef struct {
	XPlmi_SysMonPsv_Config Config; /**< Device configuration */
	u32 IsReady; /**< Is device ready */
} XPlmi_SysMonPsv;

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
 * @brief  Converts raw AdcData into Voltage value.
 *
 * @param  AdcData	System Monitor ADC Raw Data.
 *
 * @return The Voltage in volts.
 *
 * @note   None
 *
 ****************************************************************************/
static inline float XPlmi_SysMon_RawToVoltage(u32 AdcData)
{
	u32 Mantissa;
	u32 Scale;
	u32 Format;
	u32 Exponent;

	Mantissa = AdcData & XPLMI_SYSMON_SUPPLY_MANTISSA_MASK;
	Exponent = (AdcData & XPLMI_SYSMON_SUPPLY_MODE_MASK) >>
		   XPLMI_SYSMON_SUPPLY_MODE_SHIFT;
	Format = (AdcData & XPLMI_SYSMON_SUPPLY_FMT_MASK) >>
		 XPLMI_SYSMON_SUPPLY_FMT_SHIFT;

	Scale = ((u32)1U << (XPLMI_SYSMON_EXPONENT_RANGE_16 - Exponent));
	if ((Format & (Mantissa >> XPLMI_SYSMON_SUPPLY_MANTISSA_SIGN)) == 1U) {
		return ((float)Mantissa / (float)Scale) - 1.0f;
	} else {
		return (float)Mantissa / (float)Scale;
	}
}

/****************************************************************************/
/**
 * @brief  Converts the fixed point to degree celsius.
 *
 * @param  FixedQFmt	Q8.7 representation of temperature value.
 *
 * @return The Temperature in degree celsius.
 *
 * @note   None
 *
 ****************************************************************************/
static inline float XPlmi_SysMon_FixedToFloat(u32 FixedQFmt)
{
	u32 TwosComp;
	float Temperature;
	u32 Q16 = FixedQFmt & 0x0000FFFFU;
	if ((Q16 >> XPLMI_SYSMON_QFMT_SIGN) > 0U) {
		TwosComp = (~Q16 + 1U) & 0x0000FFFFU;
		Temperature = (float)(TwosComp) /
			      ((float)XPLMI_SYSMON_QFMT_FRACTION * (-1.0f));
	} else {
		Temperature = (float)Q16 /
			      ((float)XPLMI_SYSMON_QFMT_FRACTION * (1.0f));
	}
	return Temperature;
}

/****************************************************************************/
/**
 * @brief  Reads the device temperature from SysMon registers.
 *
 * @param  InstancePtr	Pointer to the XPlmi_SysMonPsv instance.
 *
 * @return Temperature value in signed Q8.7 format.
 *
 * @note   Replaces XSysMonPsv_ReadDeviceTemp() for PLM usage.
 *
 ****************************************************************************/
static inline u32 XPlmi_SysMon_ReadDeviceTemp(const XPlmi_SysMonPsv *InstancePtr)
{
	return Xil_In32(InstancePtr->Config.BaseAddress +
			XPLMI_SYSMON_DEVICE_TEMP_OFFSET);
}

/************************** Function Prototypes ******************************/
int XPlmi_SysMonInit(void);
void XPlmi_SysMonOTDetect(u32 WaitInMSecVal);
XPlmi_SysMonPsv *XPlmi_GetSysmonInst(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_SYSMON_H */
