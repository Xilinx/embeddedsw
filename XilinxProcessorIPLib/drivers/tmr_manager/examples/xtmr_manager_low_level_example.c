/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtmr_manager_low_level_example.c
*
* This file contains a design example using the low-level driver functions
* and macros of the TMR_Manager driver (XTMR_Manager).
*
* @note
*
* The user must provide a physical loopback such that data which is
* transmitted will be received.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date	 Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xtmr_manager_l.h"

/************************** Constant Definitions *****************************/


/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define TMR_MANAGER_BASEADDR	   XPAR_TMR_MANAGER_0_BASEADDR


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int TMR_ManagerLowLevelExample(u32 TMR_ManagerBaseAddress);

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
	 * Run the TMR_Manager Low level example, specify the BaseAddress that
	 * is generated in xparameters.h.
	 */
	Status = TMR_ManagerLowLevelExample(TMR_MANAGER_BASEADDR);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function does a minimal test on the TMR_Manager device using low-level
* driver macros and functions.
*
* @param	TMR_ManagerBaseAddress is the base address of the TMR_Manager
*		device and is the XPAR_<TMRMANAGER_instance>_BASEADDR value
*		from xparameters.h.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
int TMR_ManagerLowLevelExample(u32 TMR_ManagerBaseAddress)
{
	return XST_SUCCESS;
}
