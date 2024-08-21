/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_main.c
 *
 * This is the main file which contains code for the PLM.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_debug.h"
#include "xplm_load.h"
#include "xplm_init.h"
#include "xplm_status.h"
#include "xplm_post_boot.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief This is PLM main function
 *
 * @return	Ideally should not return, in case if it reaches end,
 *		error is returned
 *
 *****************************************************************************/
int main(void)
{
	u32 Status = XST_FAILURE;

	Status = XPlm_Init();
	if (Status != (u32)XST_SUCCESS) {
		XPlm_ErrMgr(Status);
	}

	Status = XPlm_LoadFullPdi();
	if (Status != XST_SUCCESS) {
		XPlm_ErrMgr(Status);
	}

	XPlm_Printf(DEBUG_PRINT_ALWAYS, "PLM Boot Done\n\r");

	Status = XPlm_PostBoot();
	if (Status != XST_SUCCESS) {
		XPlm_ErrMgr(Status);
	}

	/* Process Run-time Events */
	XSECURE_REDUNDANT_IMPL(XPlm_EventLoop);

	/* Should never reach here */
}
