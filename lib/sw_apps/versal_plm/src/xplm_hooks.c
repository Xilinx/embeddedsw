/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplm_hooks.c
*
* This file contains the hook functions for the user
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/08/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_hooks.h"
#include "xplm_default.h"
#include "xpm_api.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief This function will be called before processing the PLM CDO. Before
* this only PMC module initialization is done. Most of the HW state will be
* as POR.Please note that PS LPD UART is not initialized by this time
*
* @param	None
* @return	XST_SUCCESS on success, any other value for error
*
*****************************************************************************/
int XPlm_HookBeforePlmCdo()
{

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief This function will be called after processing the PLM CDO. All the
* PMC and LPD configuration(optional) will be completed by this time.
*
* @param	None
* @return	XST_SUCCESS on success, any other value for error
*
*****************************************************************************/
int XPlm_HookAfterPlmCdo()
{
	/* Call LibPM hook */
	XPm_HookAfterPlmCdo();
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief This function will be called after loading the boot PDI.
*
* @param	None
* @return	XST_SUCCESS on success, any other value for error
*
*****************************************************************************/
int XPlm_HookAfterBootPdi()
{

	return XST_SUCCESS;
}
