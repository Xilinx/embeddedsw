/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_stl.c
*
* This file contains the wrapper code for STL interface
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rama 08/12/2020 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_default.h"

#ifdef PLM_ENABLE_STL
#include "xstl_plminterface.h"
#include "xplm_stl.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief This function initializes the STL module and registers the
 * STL handler.
 *
 * @param	None
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
int XPlm_StlInit(void)
{
	int Status = XST_FAILURE;

	Status = XStl_Init();
	if (Status != XST_SUCCESS)
	{
		Status = XPlmi_UpdateStatus(XPLM_ERR_STL_MOD, Status);
		goto END;
	}

END:
	return Status;
}
#endif /* PLM_ENABLE_STL */
