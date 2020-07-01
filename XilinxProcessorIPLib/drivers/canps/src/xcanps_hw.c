/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanps_hw.c
* @addtogroup canps_v3_5
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
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcanps_hw.h"
#include "xparameters.h"

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
