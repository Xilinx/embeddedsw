/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_proc.c
*
* This file contains the processor related code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/27/2018 Initial release
* 1.01  kc   03/23/2020 Minor code cleanup
* 1.02  bsv  09/04/2020 Removed call to Xil_ExceptionInit
*       bsv  09/13/2020 Clear security critical data in case of exceptions,
*                       also place AES, ECDSA_RSA and SHA3 in reset
* 1.03  ma   05/03/2021 Trigger FW_NCR error for post boot exceptions
* 1.04  td   07/08/2021 Fix doxygen warnings
*       bsv  08/13/2021 Removed unnecessary header file
*       ma   08/30/2021 Trigger FW_NCR error only for master and monolithic
*                       devices
*       kpt  09/09/2021 Fixed SW-BP-BLIND-WRITE in XLoader_SecureClear
* 1.05  ma   01/17/2022 Trigger SW Error when exception occurs
* 1.06  ng   11/11/2022 Updated doxygen comments
*       kpt  02/21/2023 Removed check for XLoader_SecureClear
* 1.07  bm   07/17/2023 Removed XPlm_InitProc function
*       rama 08/10/2023 Changed exception handler print to DEBUG_ALWAYS for
*                       debug level_0 option
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_proc.h"
#include "xloader_secure.h"
#include "xplmi_err_common.h"
#include "xplmi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlm_ExceptionHandler(void *Data);

/************************** Variable Definitions *****************************/
extern u32 _stack;
extern u32 _stack_end;

/*****************************************************************************/
/**
 * @brief This function enables the exceptions and interrupts
 * Enable interrupts from the hardware
 *
 * @return	None
 *
 *****************************************************************************/
void XPlm_ExceptionInit(void)
{
	int Status = XST_FAILURE;
	u16 Index;

	/** Register exception handlers */
	for (Index = XIL_EXCEPTION_ID_FIRST;
	     Index <= XIL_EXCEPTION_ID_LAST; Index++) {
		Status = XPlmi_UpdateStatus(XPLM_ERR_EXCEPTION, (int)Index);
		Xil_ExceptionRegisterHandler(Index,
			XPlm_ExceptionHandler, (void *)Status);
	}

	/** Write stack high and low register for stack protection */
	mtslr(&_stack_end);
	mtshr(&_stack);
	microblaze_enable_exceptions();
}

/*****************************************************************************/
/**
 * @brief This is a function handler for all exceptions. It clears security
 * critical data by clearing AES keys and by placing SHA3 in reset.
 *
 * @param	Data Pointer to Error Status that needs to be updated in
 * Error Register. Status is initialized during exception initialization
 * having Index and exception error code.
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlm_ExceptionHandler(void *Data)
{
	int Status = XST_FAILURE;
	u8 SlrType = (u8)(XPlmi_In32(PMC_TAP_SLR_TYPE) &
			PMC_TAP_SLR_TYPE_VAL_MASK);

	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Received Exception \n\r"
		"MSR: 0x%08x, EAR: 0x%08x, EDR: 0x%08x, ESR: 0x%08x, \n\r"
		"R14: 0x%08x, R15: 0x%08x, R16: 0x%08x, R17: 0x%08x \n\r",
		mfmsr(), mfear(), mfedr(), mfesr(),
		mfgpr(r14), mfgpr(r15), mfgpr(r16), mfgpr(r17));

	(void)XLoader_SecureClear();

	/** Trigger SW error */
	XPlmi_HandleSwError(XIL_NODETYPE_EVENT_ERROR_SW_ERR,
			XIL_EVENT_ERROR_MASK_PLM_EXCEPTION);

	Status = (int)Data;
	XPlmi_ErrMgr(Status);

	/**
	 * Check if SLR Type is Master or Monolithic
	 * and take error action accordingly.
	 */
	if ((SlrType == XPLMI_SSIT_MASTER_SLR) ||
		(SlrType == XPLMI_SSIT_MONOLITIC)) {
		/**
		 * Trigger FW_NCR error for post boot exceptions if
		 * boot mode is other than JTAG
		 */
		if((XPlmi_In32(CRP_BOOT_MODE_USER) &
				CRP_BOOT_MODE_USER_BOOT_MODE_MASK) != 0U) {
			XPlmi_TriggerFwNcrError();
		}
	}

	/* Just in case if control reaches here */
	while (TRUE) {
		;
	}
}
