/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file xplmi_sysmon.c
 *
 * This is the file which contains SysMon manager code.
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
 *       td   10/19/2020 MISRA C Fixes
 * 1.03  bm   02/08/2021 Add GetSysmonPsv API
 * 1.04  td   07/08/2021 Fix doxygen warnings
 *       rb   08/11/2021 Fix compilation warnings
 *       is   01/10/2022 Updated XPlmi_SysMonOTDetect to wait additional time
 *                       after OT event clears up
 *       is   01/10/2022 Updated Copyright Year to 2022
 *       ma   01/17/2022 Enable SLVERR for Sysmon
 *       ma   03/01/2022 Write PCSR MASK register before enabling SLVERR
 * 1.05  skd  04/21/2022 Misra-C violation Rule 8.7 fixed
 *       bsv  07/20/2022 Removed magic number usage
 *       bm   07/24/2022 Set PlmLiveStatus during boot time
 * 1.07  ng   11/11/2022 Updated doxygen comments
 *       dd   03/28/2023 Updated doxygen comments
 * 1.08  rama 08/10/2023 Changed OT print to DEBUG_ALWAYS for
 *                       debug level_0 option
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xplmi_sysmon.h"
#include "sleep.h"
#include "xplmi_debug.h"
#include "xplmi_hw.h"
#include "xplmi_wdt.h"

/************************** Constant Definitions *****************************/
#define XPLMI_SYSMON_SAT0_PCSR_MASK_OFFSET		(0x10000U) /**< SAT0 PCSR mask offset */
#define XPLMI_SYSMON_SAT1_PCSR_MASK_OFFSET		(0x20000U) /**< SAT1 PCSR mask offset */
#define XPLMI_SYSMON_PCSR_CTRL_SLVERREN_MASK	(0x80000U) /**< PCSR control slave error enable mask */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlmi_WriteSysmonCtrlReg(u32 Addr, u32 Value);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief	This function provides address of Sysmon Instance
 *
 * @return	Address of SysmonInst
 *
 *****************************************************************************/
XSysMonPsv* XPlmi_GetSysmonInst(void)
{
	static XSysMonPsv SysMonInst;

	return &SysMonInst;
}

/*****************************************************************************/
/**
 * @brief	This function writes given configuration to Sysmon Ctrl register.
 *
 * @param	Addr is the address of Sysmon Mask register
 * @param	Value is the configuration to be written to Control register
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_WriteSysmonCtrlReg(u32 Addr, u32 Value)
{
	/*
	 * We need to unlock PCSR to write to control register.
	 * Lock it back in after write.
	 */
	XPlmi_Out32((Addr + (u32)XSYSMONPSV_PCSR_LOCK), PCSR_UNLOCK_VAL);
	XPlmi_UtilRMW(Addr, Value, Value);
	XPlmi_UtilRMW((Addr + (u32)XSYSMONPSV_PCSR_CONTROL), Value, Value);
	XPlmi_Out32((Addr + (u32)XSYSMONPSV_PCSR_LOCK), PCSR_LOCK_VAL);
}

/*****************************************************************************/
/**
 * @brief	This function initializes the SysMon.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_SysMonInit(void)
{
	int Status = XST_FAILURE;
	/**
	 * Get Instance of SysMon Driver
	 */
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();
	XSysMonPsv_Config *ConfigPtr;

	ConfigPtr = XSysMonPsv_LookupConfig();
	if (ConfigPtr != NULL) {
		Status = (int) XSysMonPsv_CfgInitialize(SysMonInstPtr, ConfigPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/**
		* Enable Over-temperature handling.  We need to unlock PCSR to
		* enable SysMon interrupt.  Lock it back in after write.
		*/
		XPlmi_Out32(ConfigPtr->BaseAddress + (u32)XSYSMONPSV_PCSR_LOCK,
			PCSR_UNLOCK_VAL);
		XSysMonPsv_IntrEnable(SysMonInstPtr, (u32)XSYSMONPSV_IER0_OT_MASK, 0U);
		XPlmi_Out32(ConfigPtr->BaseAddress + (u32)XSYSMONPSV_PCSR_LOCK, 0U);

		/**
		 * Enable SLVERR for Sysmon
		 */
		XPlmi_WriteSysmonCtrlReg(ConfigPtr->BaseAddress,
				XPLMI_SYSMON_PCSR_CTRL_SLVERREN_MASK);

		/**
		 * Enable SLVERR for Sysmon SAT0
		 */
		XPlmi_WriteSysmonCtrlReg((ConfigPtr->BaseAddress +
				(u32)XPLMI_SYSMON_SAT0_PCSR_MASK_OFFSET),
				XPLMI_SYSMON_PCSR_CTRL_SLVERREN_MASK);

		/**
		 * Enable SLVERR for Sysmon SAT1
		 */
		XPlmi_WriteSysmonCtrlReg((ConfigPtr->BaseAddress +
				(u32)XPLMI_SYSMON_SAT1_PCSR_MASK_OFFSET),
				XPLMI_SYSMON_PCSR_CTRL_SLVERREN_MASK);
	}
	XPlmi_Printf(DEBUG_DETAILED,
		 "%s: SysMon init status: 0x%x\n\r", __func__, Status);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function detects if we are still in over-temperature condition.
 *
 * @param	WaitInMSec is the time in milliseconds for which the firmware must
 *              wait before proceeding after over temperature event clears up
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_SysMonOTDetect(u32 WaitInMSec)
{
	u32 Val;
	u32 Count;

	/**
	 * Check for over-temperature condition in Interrupt Status Regiser.
	 */
	Val = XPlmi_In32((UINTPTR)XSYSMONPSV_BASEADDR + (UINTPTR)XSYSMONPSV_ISR_OFFSET);
	if (0U == (Val & (u32)XSYSMONPSV_ISR_OT_MASK)) {
		goto END;
	}

	XPlmi_Out32((UINTPTR)XSYSMONPSV_BASEADDR + (UINTPTR)XSYSMONPSV_PCSR_LOCK,
		PCSR_UNLOCK_VAL);

	/**
	 * Wait until over-temperature condition is resolved.
	 */
	Count = 1000U;
	while (0U != (Val & (u32)XSYSMONPSV_ISR_OT_MASK)) {
		XPlmi_Out32((UINTPTR)XSYSMONPSV_BASEADDR + (UINTPTR)XSYSMONPSV_ISR_OFFSET,
			XSYSMONPSV_ISR_OT_MASK);
		usleep(1000U);
		XPlmi_SetPlmLiveStatus();
		Count--;
		if (0U == Count) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"Warning: Over-temperature condition!\r\n");
			Count = 1000U;
		}
		Val = XPlmi_In32((UINTPTR)XSYSMONPSV_BASEADDR + (UINTPTR)XSYSMONPSV_ISR_OFFSET);
	}

	/**
	 * In case of boot after an OT event, wait for this additional time after
	 * OT clears to allow the temperature across the chip to cool down
	 */
	while (WaitInMSec-- > 0U) {
		usleep(1000U);
		XPlmi_SetPlmLiveStatus();
	}

	XPlmi_Out32((UINTPTR)XSYSMONPSV_BASEADDR + (UINTPTR)XSYSMONPSV_PCSR_LOCK, 0U);

END:
	return;
}
