/******************************************************************************
*
* Copyright (C) 2015 -17 Xilinx, Inc.  All rights reserved.
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
*
* @file xfsbl_hooks.c
*
* This is the file which contains FSBL hook functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   04/21/14 Initial release
* 2.0   bv   12/05/16 Made compliance to MISRAC 2012 guidelines
*       ssc  03/25/17 Set correct value for SYSMON ANALOG_BUS register
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#include "xfsbl_hooks.h"
#include "psu_init.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
#ifdef XFSBL_BS
u32 XFsbl_HookBeforeBSDownload(void )
{
	u32 Status = XFSBL_SUCCESS;

	/**
	 * Add the code here
	 */


	return Status;
}


u32 XFsbl_HookAfterBSDownload(void )
{
	u32 Status = XFSBL_SUCCESS;

	/**
	 * Add the code here
	 */

	return Status;
}
#endif

u32 XFsbl_HookBeforeHandoff(u32 EarlyHandoff)
{
	u32 Status = XFSBL_SUCCESS;

	/**
	 * Add the code here
	 */

	return Status;
}

/*****************************************************************************/
/**
 * This is a hook function where user can include the functionality to be run
 * before FSBL fallback happens
 *
 * @param none
 *
 * @return error status based on implemented functionality (SUCCESS by default)
 *
  *****************************************************************************/

u32 XFsbl_HookBeforeFallback(void)
{
	u32 Status = XFSBL_SUCCESS;

	/**
	 * Add the code here
	 */

	return Status;
}

/*****************************************************************************/
/**
 * This function facilitates users to define different variants of psu_init()
 * functions based on different configurations in Vivado. The default call to
 * psu_init() can then be swapped with the alternate variant based on the
 * requirement.
 *
 * @param none
 *
 * @return error status based on implemented functionality (SUCCESS by default)
 *
  *****************************************************************************/

u32 XFsbl_HookPsuInit(void)
{
	u32 Status;

	/* Add the code here */

	Status = (u32)psu_init();

	if (XFSBL_SUCCESS != Status) {
			XFsbl_Printf(DEBUG_GENERAL,"XFSBL_PSU_INIT_FAILED\n\r");
			/**
			 * Need to check a way to communicate both FSBL code
			 * and PSU init error code
			 */
			Status = XFSBL_PSU_INIT_FAILED + Status;
	}

	/*
	 * Write 1U to PMU GLOBAL general storage register 5 to indicate
	 * PMU Firmware that psu init is completed
	 */
	XFsbl_Out32(PMU_GLOBAL_GLOB_GEN_STORAGE5, XFSBL_PSU_INIT_COMPLETED);

	/**
	 * PS_SYSMON_ANALOG_BUS register determines mapping between SysMon supply
	 * sense channel to SysMon supply registers inside the IP. This register
	 * must be programmed to complete SysMon IP configuration.
	 * The default register configuration after power-up is incorrect.
	 * Hence, fix this by writing the correct value - 0x3210.
	 */

	XFsbl_Out32(AMS_PS_SYSMON_ANALOG_BUS, PS_SYSMON_ANALOG_BUS_VAL);

	return Status;
}

/*****************************************************************************/
/**
 * This function detects type of boot based on information from
 * PMU_GLOBAL_GLOB_GEN_STORAGE1. If Power Off Suspend is supported FSBL must
 * wait for PMU to detect boot type and provide that information using register.
 * In case of resume from Power Off Suspend PMU will wait for FSBL to confirm
 * detection by writting 1 to PMU_GLOBAL_GLOB_GEN_STORAGE2.
 *
 * @return Boot type, 0 in case of cold boot, 1 for warm boot
 *
 * @note none
 *****************************************************************************/
#ifdef ENABLE_POS
u32 XFsbl_HookGetPosBootType(void)
{
	u32 WarmBoot = 0;
	u32 RegValue = 0;

	do {
		RegValue = XFsbl_In32(PMU_GLOBAL_GLOB_GEN_STORAGE1);
	} while (0U == RegValue);

	/* Clear Gen Storage register so it can be used later in system */
	XFsbl_Out32(PMU_GLOBAL_GLOB_GEN_STORAGE1, 0U);

	WarmBoot = RegValue - 1;

	/* Confirm detection in case of resume from Power Off Suspend */
	if (0 != RegValue) {
		XFsbl_Out32(PMU_GLOBAL_GLOB_GEN_STORAGE2, 1U);
	}

	return WarmBoot;
}
#endif
