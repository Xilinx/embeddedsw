/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
 * 		 td   10/19/2020 MISRA C Fixes
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xplmi_sysmon.h"
#include "sleep.h"
#include "xsysmonpsv.h"
#include "xplmi_debug.h"
#include "xplmi_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief	This function initializes the SysMon.
 *
 * @param	void
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_SysMonInit(void)
{
	int Status = XST_FAILURE;
	/* Instance of SysMon Driver */
	static XSysMonPsv SysMonInst;
	static XSysMonPsv *SysMonInstPtr = &SysMonInst;
	XSysMonPsv_Config *ConfigPtr;

	ConfigPtr = XSysMonPsv_LookupConfig();
	if (ConfigPtr != NULL) {
		Status = XSysMonPsv_CfgInitialize(SysMonInstPtr, ConfigPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/*
		* Enable Over-temperature handling.  We need to unlock PCSR to
		* enable SysMon interrupt.  Lock it back in after write.
		*/
		XPlmi_Out32(ConfigPtr->BaseAddress + (u32)XSYSMONPSV_PCSR_LOCK,
			PCSR_UNLOCK_VAL);
		XSysMonPsv_IntrEnable(SysMonInstPtr, (u32)XSYSMONPSV_IER0_OT_MASK, 0U);
		XPlmi_Out32(ConfigPtr->BaseAddress + (u32)XSYSMONPSV_PCSR_LOCK, 0U);
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
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_SysMonOTDetect(void)
{
	u32 Val;
	u32 Count;

	/*
	 * If Interrupt Status Register does not indicate an over-temperature
	 * condition, we are done.
	 */
	Val = XPlmi_In32((UINTPTR)XSYSMONPSV_BASEADDR + (UINTPTR)XSYSMONPSV_ISR_OFFSET);
	if (0U == (Val & (u32)XSYSMONPSV_ISR_OT_MASK)) {
		goto END;
	}

	XPlmi_Out32((UINTPTR)XSYSMONPSV_BASEADDR + (UINTPTR)XSYSMONPSV_PCSR_LOCK,
		PCSR_UNLOCK_VAL);

	/* Wait until over-temperature condition is resolved. */
	Count = 1000U;
	while (0U != (Val & (u32)XSYSMONPSV_ISR_OT_MASK)) {
		XPlmi_Out32((UINTPTR)XSYSMONPSV_BASEADDR + (UINTPTR)XSYSMONPSV_ISR_OFFSET,
			XSYSMONPSV_ISR_OT_MASK);
		usleep(1000U);
		Count--;
		if (0U == Count) {
			XPlmi_Printf(DEBUG_GENERAL,
				"Warning: Over-temperature condition!\r\n");
			Count = 1000U;
		}
		Val = XPlmi_In32((UINTPTR)XSYSMONPSV_BASEADDR + (UINTPTR)XSYSMONPSV_ISR_OFFSET);
	}

	XPlmi_Out32((UINTPTR)XSYSMONPSV_BASEADDR + (UINTPTR)XSYSMONPSV_PCSR_LOCK, 0U);

END:
	return;
}
