/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
 * 5.2  Nava  12/05/19  Added Versal platform support.
 * 5.2  Nava  02/27/20  Updated the write path to read the pdi/bin image ipi
 *                      load response to handle the pdi/bin image load
 *                      errors properly.
 * 5.3  Nava  06/16/20  Modified the date format from dd/mm to mm/dd.
 * 5.3  Nava  06/29/20  Added asserts to validate input params.
 * 5.3  Nava  09/09/20  Replaced the asserts with input validations for non void
 *                      API's.
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
#define XMAILBOX_DEVICE_ID	0x0U
#define FPGA_PDI_SRC_DDR	0xFU
#define FPGA_IPI_TYPE_BLOCKING	0x1U
#define PDI_LOAD_TYPE_MASK	BIT(0)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static u32 XFpga_WriteToPl(XFpga *InstancePtr);
/************************** Variable Definitions *****************************/

XMailbox XMboxInstance;
static u32 ReqBuffer[LOAD_PDI_MSG_LEN] = {0U};

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
u32 XFpga_Initialize(XFpga *InstancePtr)
{
	u32 Status = XFPGA_INVALID_PARAM;

	/* Validate the input arguments */
	if (InstancePtr != NULL) {
		(void)memset(InstancePtr, 0U, sizeof(*InstancePtr));
		InstancePtr->XFpga_WriteToPl = XFpga_WriteToPl;
		Status = XFPGA_SUCCESS;
	}

	return Status;
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

	Status = XMailbox_Initialize(&XMboxInstance, XMAILBOX_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if (InstancePtr->WriteInfo.Flags & PDI_LOAD_TYPE_MASK) {
		ReqBuffer[0U] = DELAYED_PDI_LOAD;
		ReqBuffer[1U] = (u32)BitstreamAddr; /* Image ID */
		ReqBuffer[2U] = 0U;
		ReqBuffer[3U] = 0U;
	} else {
		ReqBuffer[0U] = PDI_LOAD;
		ReqBuffer[1U] = FPGA_PDI_SRC_DDR;
		ReqBuffer[2U] = UPPER_32_BITS(BitstreamAddr);
		ReqBuffer[3U] = LOWER_32_BITS(BitstreamAddr);
	}
	/* Send an IPI Req Message */
	Status = XMailbox_SendData(&XMboxInstance, XMAILBOX_IPIPMC, ReqBuffer,
				   LOAD_PDI_MSG_LEN, XILMBOX_MSG_TYPE_REQ,
				   FPGA_IPI_TYPE_BLOCKING);
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
