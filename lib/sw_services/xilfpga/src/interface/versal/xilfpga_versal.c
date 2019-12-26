/******************************************************************************
 *
 * Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xilfpga_versal.c
 *
 * This file contains the definitions of bitstream loading functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ---- ----- --------  -------------------------------------------------------
 * 5.2  Nava  05/12/19  Added Versal platform support.
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xilfpga.h"
#include "xilmailbox.h"

/************************** Constant Definitions *****************************/
#define PDI_LOAD		0x30701
#define DELAYED_PDI_LOAD	0x30702
#define LOAD_PDI_MSG_LEN	0x4
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static u32 XFpga_WriteToPl(XFpga *InstancePtr);
/************************** Variable Definitions *****************************/

XMailbox XMboxInstance;
static u32 ReqBuffer[LOAD_PDI_MSG_LEN];

/*****************************************************************************/
/* This API when called initializes the XFPGA interface with default settings.
 * It Sets function pointers for the instance.
 *
 * @param InstancePtr Pointer to the XFgpa structure.
 *
 * @return Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 ******************************************************************************/
u32 XFpga_Initialize(XFpga *InstancePtr) {

	(void)memset(InstancePtr, 0, sizeof(*InstancePtr));
	InstancePtr->XFpga_WriteToPl = XFpga_WriteToPl;

	return XFPGA_SUCCESS;
}

/*****************************************************************************/
/* This function writes bitstream data into the PL.
 *
 * @param InstancePtr Pointer to the XFpga structure.
 *
 * @return	Returns Status
 *		- XFPGA_SUCCESS on success
 *		- Error code on failure
 *
 *****************************************************************************/
static u32 XFpga_WriteToPl(XFpga *InstancePtr)
{
	u32 Status = XFPGA_FAILURE;
	UINTPTR BitstreamAddr = InstancePtr->WriteInfo.BitstreamAddr;

	Status = XMailbox_Initialize(&XMboxInstance, 0);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if (InstancePtr->WriteInfo.Flags & BIT(0)) {
		ReqBuffer[0] = DELAYED_PDI_LOAD;
		ReqBuffer[1] = (u32)BitstreamAddr; /* Image ID */
	} else {
		ReqBuffer[0] = PDI_LOAD;
		ReqBuffer[1] = 0xF; /* DDR */
		ReqBuffer[2] = UPPER_32_BITS(BitstreamAddr);
		ReqBuffer[3] = (u32)BitstreamAddr;
	}
	/* Send an IPI Req Message */
	Status = XMailbox_SendData(&XMboxInstance, XMAILBOX_IPIPMC, ReqBuffer,
				   LOAD_PDI_MSG_LEN, XILMBOX_MSG_TYPE_REQ, 1);
	if (Status != XST_SUCCESS) {
		xil_printf("Sending Req Message Failed\n\r");
		goto END;
	}

END:
	return Status;
}
