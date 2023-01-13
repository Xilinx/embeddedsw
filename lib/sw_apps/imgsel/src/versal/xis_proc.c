/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xis_proc.c
*
* This file contains the processor related code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  skd  01/13/23 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_proc.h"
#include "xplmi_err_common.h"
#include "xplmi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlm_ExceptionInit(void);
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
static void XPlm_ExceptionInit(void)
{
	int Status = XST_FAILURE;
	u16 Index;

	/* Register exception handlers */
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

	XPlmi_Printf(DEBUG_GENERAL, "Received Exception \n\r"
		"MSR: 0x%08x, EAR: 0x%08x, EDR: 0x%08x, ESR: 0x%08x, \n\r"
		"R14: 0x%08x, R15: 0x%08x, R16: 0x%08x, R17: 0x%08x \n\r",
		mfmsr(), mfear(), mfedr(), mfesr(),
		mfgpr(r14), mfgpr(r15), mfgpr(r16), mfgpr(r17));

	/* Just in case if control reaches here */
	while (TRUE) {
		;
	}
}

/*****************************************************************************/
/**
 * @brief This function initializes the processor, enables exceptions and start
 * timer
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
int XPlm_InitProc(void)
{
	int Status = XST_FAILURE;

	XPlm_ExceptionInit();
	Status = XPlmi_StartTimer();

	return Status;
}
