/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxidma_sinit.c
* @addtogroup AXIDMA Overview
* @{
* @brief Implements static initialization and configuration lookup for AXI DMA.
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

#ifndef SDT
#include "xparameters.h"
#endif
#include "xaxidma.h"


#ifndef SDT
/*****************************************************************************/
/**
 * Implementation of XAxiDma_LookupConfig(). See xaxidma.h for the API
 * description.
 *
 *****************************************************************************/
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
 * Implementation of XAxiDma_LookupConfigBaseAddr(). See xaxidma.h for the API
 * description.
 *
 *****************************************************************************/
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
#else
/*****************************************************************************/
/**
 * Implementation of XAxiDma_LookupConfig(). See xaxidma.h for the API
 * description.
 *
 *****************************************************************************/
XAxiDma_Config *XAxiDma_LookupConfig(UINTPTR BaseAddress)
{
	extern XAxiDma_Config XAxiDma_ConfigTable[];
	XAxiDma_Config *CfgPtr;
	u32 Index;

	CfgPtr = NULL;

	for (Index = (u32)0x0; XAxiDma_ConfigTable[Index].Name != NULL; Index++) {
		if ((XAxiDma_ConfigTable[Index].BaseAddr == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XAxiDma_ConfigTable[Index];
			break;
		}
	}

	return (XAxiDma_Config *)CfgPtr;
}
#endif

/** @} */
