/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xpsmfw_ipi_manager.c
*
* This file contains IPI manager functions for PSM Firmware
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#include "xpsmfw_ipi_manager.h"
#include "ipi.h"

#ifdef XPAR_PSU_IPI_PSM_DEVICE_ID
/* Instance of IPI Driver */
XIpiPsu IpiInst;
XIpiPsu *IpiInstPtr = &IpiInst;
u32 IpiMaskList[XPSMFW_IPI_MASK_COUNT] = {0U};

s32 XPsmfw_IpiManagerInit(void)
 {
	s32 Status;
	XIpiPsu_Config *IpiCfgPtr;
	u32 i;

	/* Load Config for PSM IPI */
	IpiCfgPtr = XIpiPsu_LookupConfig(XPAR_XIPIPSU_0_DEVICE_ID);

	if (IpiCfgPtr == NULL) {
		Status = XST_FAILURE;
		XPsmFw_Printf(DEBUG_PRINT_ALWAYS, "IPI lookup config failed\r\n");
		goto Done;
	}
	/* Init Mask List */
	for (i = 0U; i < XPSMFW_IPI_MASK_COUNT; i++) {
		IpiMaskList[i] = IpiCfgPtr->TargetList[i].Mask;
	}
	/* Initialize the IPI driver */
	Status = XIpiPsu_CfgInitialize(IpiInstPtr, IpiCfgPtr,
			IpiCfgPtr->BaseAddress);

	/* Enable IPIs from all Masters */
	for (i = 0U; i < XPSMFW_IPI_MASK_COUNT; i++) {
		XIpiPsu_InterruptEnable(IpiInstPtr, IpiCfgPtr->TargetList[i].Mask);
	}
	XPsmFw_Printf(DEBUG_PRINT_ALWAYS, "IPI interrupts are enabled\r\n");

Done:
	return Status;
}

/**
 * XPsmFw_DispatchIpiHandler() - Interrupt handler for IPI
 *
 * @IpiInstPtr		Pointer to the IPI instance
 */
int XPsmFw_DispatchIpiHandler(u32 SrcMask)
{
	int Status;
	u32 MaskIndex;
	u32 Payload[XPSMFW_IPI_MAX_MSG_LEN];

	XPsmFw_Printf(DEBUG_PRINT_ALWAYS, "In IPI handler\r\n");

	for (MaskIndex = 0U; MaskIndex < XPSMFW_IPI_MASK_COUNT; MaskIndex++) {
		if ((SrcMask & IpiMaskList[MaskIndex]) != 0U) {
			Status = XIpiPsu_ReadMessage(IpiInstPtr, IpiMaskList[MaskIndex],
			        &Payload[0], XPSMFW_IPI_MAX_MSG_LEN, XIPIPSU_BUF_TYPE_MSG);
			if (XST_SUCCESS != Status) {
				XPsmFw_Printf(DEBUG_PRINT_ALWAYS, "Failure to read IPI msg\r\n");
			} else {
				for (int i = 0; i < 8; i++) {
					XPsmFw_Printf(DEBUG_PRINT_ALWAYS, "IPI Msg: 0x%x\r\n", Payload[i]);
				}
			}
		}
	}
	return Status;
}
#endif
