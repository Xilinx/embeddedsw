/******************************************************************************
* Copyright (C) 2012 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxivdma_sinit.c
* @addtogroup axivdma_v6_12
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
* 2.00a jz   12/10/10 Added support for direct register access mode, v3 core
* 2.01a jz   01/19/11 Added ability to re-assign BD addresses
* 5.1   sha  07/15/15 Defined macro XPAR_XAXIVDMA_NUM_INSTANCES if not
*		      defined in xparameters.h
* 6.4   ms   08/07/17 Fixed compilation warnings.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxivdma.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

#ifndef XPAR_XAXIVDMA_NUM_INSTANCES
#define XPAR_XAXIVDMA_NUM_INSTANCES		0
#endif

/*****************************************************************************/
/**
 * Look up the hardware configuration for a device instance
 *
 * @param DeviceId is the unique device ID of the device to lookup for
 *
 * @return
 * The configuration structure for the device. If the device ID is not found,
 * a NULL pointer is returned.
 *
 ******************************************************************************/
XAxiVdma_Config *XAxiVdma_LookupConfig(u16 DeviceId)
{
	extern XAxiVdma_Config XAxiVdma_ConfigTable[];
	XAxiVdma_Config *CfgPtr = NULL;
	u32 i;

	for (i = 0U; i < XPAR_XAXIVDMA_NUM_INSTANCES; i++) {
		if (XAxiVdma_ConfigTable[i].DeviceId == DeviceId) {
			CfgPtr = &XAxiVdma_ConfigTable[i];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
