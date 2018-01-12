/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxidma_sinit.c
* @addtogroup axidma_v9_11
* @{
*
* Look up the hardware settings using device ID. The hardware setting is inside
* the configuration table in xaxidma_g.c, generated automatically by XPS or
* manually by the user.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a jz   08/16/10 First release
* 2.00a jz   08/10/10 Second release, added in xaxidma_g.c, xaxidma_sinit.c,
*                     updated tcl file, added xaxidma_porting_guide.h
* 3.00a jz   11/22/10 Support IP core parameters change
* 5.00a srt  08/29/11 Removed a compiler warning
* 9.5   rsp  11/01/17 Add interface to do config lookup based on base address.
* 9.6   rsp  01/11/18 In LookupConfig use UINTPTR for Baseaddr CR#976392
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xaxidma.h"


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
XAxiDma_Config *XAxiDma_LookupConfig(u32 DeviceId)
{
	extern XAxiDma_Config XAxiDma_ConfigTable[];
	XAxiDma_Config *CfgPtr;
	u32 Index;

	CfgPtr = NULL;

	for (Index = 0; Index < XPAR_XAXIDMA_NUM_INSTANCES; Index++) {
		if (XAxiDma_ConfigTable[Index].DeviceId == DeviceId) {

			CfgPtr = &XAxiDma_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/*****************************************************************************/
/**
 * Look up the hardware configuration for a device instance based on base address
 *
 * @param	Baseaddr is the base address of the device to lookup for
 *
 * @return
 *		The configuration structure for the device. If the device base
 *		address is not found,a NULL pointer is returned.
 *
 * @note	None
 *
 ******************************************************************************/
XAxiDma_Config *XAxiDma_LookupConfigBaseAddr(UINTPTR Baseaddr)
{
	extern XAxiDma_Config XAxiDma_ConfigTable[];
	XAxiDma_Config *CfgPtr;
	u32 Index;

	CfgPtr = NULL;

	for (Index = 0; Index < XPAR_XAXIDMA_NUM_INSTANCES; Index++) {
		if (XAxiDma_ConfigTable[Index].BaseAddr == Baseaddr) {

			CfgPtr = &XAxiDma_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/** @} */
