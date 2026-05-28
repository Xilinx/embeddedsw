/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       mss  03/13/2024 MISRA-C violation Rule 17.8 fixed
 * 2.40  obs  02/12/2025 Removed XSysMonPsv driver dependency, use direct
 *                       register access for PLM operations
 *
 * </pre>
 *
 *
 ******************************************************************************/

/**
 * @addtogroup xilplmi_server_apis XilPlmi server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplmi_sysmon.h"
#include "sleep.h"
#include "xplmi_debug.h"
#include "xplmi_hw.h"
#include "xplmi_wdt.h"

/************************** Constant Definitions *****************************/
#define XPLMI_SYSMON_SAT0_PCSR_MASK_OFFSET	(0x10000U) /**< SAT0 PCSR mask offset */
#define XPLMI_SYSMON_SAT1_PCSR_MASK_OFFSET	(0x20000U) /**< SAT1 PCSR mask offset */
#define XPLMI_SYSMON_PCSR_CTRL_SLVERREN_MASK	(0x80000U) /**< PCSR control slave error enable mask */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlmi_WriteSysmonCtrlReg(UINTPTR Addr, u32 Value);

/************************** Variable Definitions *****************************/
/** SysMon instance - stores only base address */
static XPlmi_SysMonPsv SysMonInst = {0U};

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief	This function provides address of Sysmon Instance
 *
 * @return	Address of SysmonInst
 *
 *****************************************************************************/
XPlmi_SysMonPsv *XPlmi_GetSysmonInst(void)
{
	return &SysMonInst;
}

/*****************************************************************************/
/**
 * @brief	This function writes given configuration to Sysmon Ctrl register.
 *
 * @param	Addr is the address of Sysmon Mask register
 * @param	Value is the configuration to be written to Control register
 *
 *****************************************************************************/
static void XPlmi_WriteSysmonCtrlReg(UINTPTR Addr, u32 Value)
{
	/*
	 * We need to unlock PCSR to write to control register.
	 * Lock it back in after write.
	 */
	XPlmi_Out32((Addr + XPLMI_SYSMON_PCSR_LOCK), XPLMI_SYSMON_PCSR_UNLOCK_VAL);
	/* SysMon registers are in 32-bit address space, safe to cast for RMW */
	XPlmi_UtilRMW((u32)Addr, Value, Value);
	XPlmi_UtilRMW((u32)(Addr + XPLMI_SYSMON_PCSR_CONTROL), Value, Value);
	XPlmi_Out32((Addr + XPLMI_SYSMON_PCSR_LOCK), XPLMI_SYSMON_PCSR_LOCK_VAL);
}

/*****************************************************************************/
/**
 * @brief	This function initializes the SysMon using direct register access.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_SysMonInit(void)
{
	int Status = XST_FAILURE;
	UINTPTR BaseAddress = XPLMI_SYSMON_BASEADDR;

	/* Initialize SysMon instance with base address */
	SysMonInst.Config.BaseAddress = BaseAddress;
	SysMonInst.IsReady = XIL_COMPONENT_IS_READY;

	/**
	 * Enable SLVERR for Sysmon
	 */
	XPlmi_WriteSysmonCtrlReg(BaseAddress,
			XPLMI_SYSMON_PCSR_CTRL_SLVERREN_MASK);

	/**
	 * Enable SLVERR for Sysmon SAT0
	 */
	XPlmi_WriteSysmonCtrlReg((BaseAddress + XPLMI_SYSMON_SAT0_PCSR_MASK_OFFSET),
			XPLMI_SYSMON_PCSR_CTRL_SLVERREN_MASK);

	/**
	 * Enable SLVERR for Sysmon SAT1
	 */
	XPlmi_WriteSysmonCtrlReg((BaseAddress + XPLMI_SYSMON_SAT1_PCSR_MASK_OFFSET),
			XPLMI_SYSMON_PCSR_CTRL_SLVERREN_MASK);

	/**
	 * Enable Over-temperature handling. We need to unlock PCSR to
	 * enable SysMon interrupt. Lock it back in after write.
	 */
	XPlmi_Out32(BaseAddress + XPLMI_SYSMON_PCSR_LOCK,
			XPLMI_SYSMON_PCSR_UNLOCK_VAL);
	XPlmi_Out32(BaseAddress + XPLMI_SYSMON_IER0_OFFSET,
			XPLMI_SYSMON_IER0_OT_MASK);
	XPlmi_Out32(BaseAddress + XPLMI_SYSMON_PCSR_LOCK,
			XPLMI_SYSMON_PCSR_LOCK_VAL);

	Status = XST_SUCCESS;

	XPlmi_Printf(DEBUG_DETAILED,
		 "%s: SysMon init status: 0x%x\n\r", __func__, Status);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function detects if we are still in over-temperature condition.
 *
 * @param	WaitInMSecVal is the time in milliseconds for which the firmware must
 *              wait before proceeding after over temperature event clears up
 *
 *****************************************************************************/
void XPlmi_SysMonOTDetect(u32 WaitInMSecVal)
{
	u32 WaitInMSec = WaitInMSecVal;
	u32 Val;
	u32 Count;
	UINTPTR BaseAddress = XPLMI_SYSMON_BASEADDR;

	/**
	 * Check for over-temperature condition in Interrupt Status Register.
	 */
	Val = XPlmi_In32(BaseAddress + XPLMI_SYSMON_ISR_OFFSET);
	if (0U == (Val & XPLMI_SYSMON_ISR_OT_MASK)) {
		goto END;
	}

	XPlmi_Out32(BaseAddress + XPLMI_SYSMON_PCSR_LOCK,
		XPLMI_SYSMON_PCSR_UNLOCK_VAL);

	/**
	 * Wait until over-temperature condition is resolved.
	 */
	Count = 1000U;
	while (0U != (Val & XPLMI_SYSMON_ISR_OT_MASK)) {
		XPlmi_Out32(BaseAddress + XPLMI_SYSMON_ISR_OFFSET,
			XPLMI_SYSMON_ISR_OT_MASK);
		usleep(1000U);
		XPlmi_SetPlmLiveStatus();
		Count--;
		if (0U == Count) {
			XPlmi_Printf(DEBUG_PRINT_ALWAYS,
				"Warning: Over-temperature condition!\r\n");
			Count = 1000U;
		}
		Val = XPlmi_In32(BaseAddress + XPLMI_SYSMON_ISR_OFFSET);
	}

	/**
	 * In case of boot after an OT event, wait for this additional time after
	 * OT clears to allow the temperature across the chip to cool down
	 */
	while (WaitInMSec-- > 0U) {
		usleep(1000U);
		XPlmi_SetPlmLiveStatus();
	}

	XPlmi_Out32(BaseAddress + XPLMI_SYSMON_PCSR_LOCK,
		XPLMI_SYSMON_PCSR_LOCK_VAL);

END:
	return;
}

/** @} End of xilplmi_server_apis group */
