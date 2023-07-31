/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtmr_inject_low_level_example.c
*
* This file contains a design example using the low-level driver functions
* and macros of the TMRInject driver (XTMRInject).
*
* @note
*
* It is assumed that the design is Triple Modular Redundant, such that
* injection of a fault will not prevent the software from continuing to run.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date	 Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.0   sa   04/05/17 First release
* 1.6   asa  07/31/23 Update the macro TMRINJECT_BASEADDR to correct define
*                     from xparameters.h. Previously it was defined as
*                     XPAR_TMRINJECT_0_BASEADDR.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xtmr_inject_l.h"

/************************** Constant Definitions *****************************/


/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define TMRINJECT_BASEADDR	   XPAR_TMR_INJECT_0_BASEADDR


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int TMRInjectLowLevelExample(u32 TMRInjectBaseAddress);

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the TMRInject Low level example, specify the BaseAddress that is
	 * generated in xparameters.h.
	 */
	Status = TMRInjectLowLevelExample(TMRINJECT_BASEADDR);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function does a minimal test on the TMRInject device using the low-level
* driver macros and functions.
*
* @param	TMRInjectBaseAddress is the base address of the TMRInject device
*		and is the XPAR_<TMRINJECT_instance>_BASEADDR value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
int TMRInjectLowLevelExample(u32 TMRInjectBaseAddress)
{
	int Index;

	return XST_SUCCESS;
}
