/******************************************************************************
* Copyright (C) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xllfifo_sinit.c
* @addtogroup llfifo_v5_4
* @{
*
* This file contains static initialization functionality for Axi Streaming FIFO
* driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 3.00a adk 9/10/2013 initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xllfifo.h"

/*****************************************************************************/
/**
 * Look up the hardware configuration for a device instance
 *
 * @param	DeviceId is the unique device ID of the device to lookup for
 *
 * @return
 *		The configuration structure for the device. If the device ID is
 *		not found,a NULL pointer is returned.
 *
 * @note	None
 *
 ******************************************************************************/
XLlFifo_Config *XLlFfio_LookupConfig(u32 DeviceId)
{
	extern XLlFifo_Config XLlFifo_ConfigTable[];
	XLlFifo_Config *CfgPtr;
	u32 Index;

	CfgPtr = NULL;

	for (Index = 0; Index < XPAR_XLLFIFO_NUM_INSTANCES; Index++) {
		if (XLlFifo_ConfigTable[Index].DeviceId == DeviceId) {

			CfgPtr = &XLlFifo_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
