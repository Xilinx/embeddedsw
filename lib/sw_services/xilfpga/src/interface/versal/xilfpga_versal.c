/******************************************************************************
 *
 * Copyright (C) 2019-2020 Xilinx, Inc.  All rights reserved.
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
 * 5.2  Nava  27/02/20  Updated the write path to read the pdi/bin image ipi
 *                      load response to handle the pdi/bin image load
 *                      errors properly.
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xilfpga.h"
#include "xilmailbox.h"

/************************** Constant Definitions *****************************/
#define PDI_LOAD		0x30701U
#define DELAYED_PDI_LOAD	0x30702U
#define LOAD_PDI_MSG_LEN	0x4U
#define FPGA_IPI_RESP1		0x1U
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

	(void)memset(InstancePtr, 0U, sizeof(*InstancePtr));
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

	Status = XMailbox_Initialize(&XMboxInstance, 0U);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if (InstancePtr->WriteInfo.Flags & BIT(0U)) {
		ReqBuffer[0U] = DELAYED_PDI_LOAD;
		ReqBuffer[1U] = (u32)BitstreamAddr; /* Image ID */
	} else {
		ReqBuffer[0U] = PDI_LOAD;
		ReqBuffer[1U] = 0xFU; /* DDR */
		ReqBuffer[2U] = UPPER_32_BITS(BitstreamAddr);
		ReqBuffer[3U] = (u32)BitstreamAddr;
	}
	/* Send an IPI Req Message */
	Status = XMailbox_SendData(&XMboxInstance, XMAILBOX_IPIPMC, ReqBuffer,
				   LOAD_PDI_MSG_LEN, XILMBOX_MSG_TYPE_REQ, 1);
	if (Status != XST_SUCCESS) {
		xil_printf("Sending Req Message Failed\n\r");
		goto END;
	}

	Status = XMailbox_Recv(&XMboxInstance, XMAILBOX_IPIPMC, ReqBuffer,
				FPGA_IPI_RESP1, XILMBOX_MSG_TYPE_RESP);
	if (Status == XST_SUCCESS) {
		Status = ReqBuffer[0U];
	}

END:
	return Status;
}
