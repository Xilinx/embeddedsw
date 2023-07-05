/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanps_hw.c
* @addtogroup canps Overview
* @{
*
* This file contains the implementation of the canps interface reset sequence
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.02a adk  08/08/13 First release
* 3.00  kvn  02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.5	sne  07/01/20 Fixed MISRAC warnings.
* 3.7	ht   06/28/23 Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcanps_hw.h"
#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* This function resets the CAN device. Calling this function resets the device
* immediately, and any pending transmission or reception is terminated at once.
* Both Object Layer and Transfer Layer are reset. This function does not reset
* the Physical Layer. All registers are reset to the default values, and no
* previous status will be restored. TX FIFO, RX FIFO and TX High Priority
* Buffer are also reset.
*
* The CAN device will be in Configuration Mode immediately after this function
* returns.
*
* @param	BaseAddr is the baseaddress of the interface.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCanPs_ResetHw(UINTPTR BaseAddr)
{
	XCanPs_WriteReg(BaseAddr, XCANPS_SRR_OFFSET, \
			XCANPS_SRR_SRST_MASK);
}
/** @} */
