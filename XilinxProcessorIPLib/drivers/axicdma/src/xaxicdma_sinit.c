/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxicdma_sinit.c
* @addtogroup axicdma_v4_8
* @{
*
* Look up the hardware settings using device ID. The hardware setting is inside
* the configuration table in xaxivdma_g.c, generated automatically by XPS or
* manually by user.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a jz   08/16/10 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xaxicdma.h"


/*****************************************************************************/
/**
 * Look up the hardware configuration for a device instance
 *
 * @param	DeviceId is the unique device ID of the device to lookup for
 *
 * @return	The configuration structure for the device. If the device ID
 *		is not found,a NULL pointer is returned.
 *
 * @note	None.
 *
 ******************************************************************************/
XAxiCdma_Config *XAxiCdma_LookupConfig(u32 DeviceId)
{
	extern XAxiCdma_Config XAxiCdma_ConfigTable[];
	XAxiCdma_Config *CfgPtr = NULL;
	int i;

	for (i = 0; i < XPAR_XAXICDMA_NUM_INSTANCES; i++) {

		if (XAxiCdma_ConfigTable[i].DeviceId == DeviceId) {

			CfgPtr = &XAxiCdma_ConfigTable[i];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
