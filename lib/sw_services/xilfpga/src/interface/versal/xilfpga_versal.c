/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
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
 * 5.3  Nava  12/15/20  Fixed doxygen issues.
 * 6.0  Nava  01/20/21  Reset the status variable to fail to avoid safety
 *                      violations.
 * 6.0  Nava  01/21/21  Make Status variable volatile to avoid compiler
 *                      optimizations.
 * 6.0  Nava  01/21/21  The usage of XMboxInstance variable is limited only
 *                      to this file. So making this variable as static.
 * 6.0  Nava  02/11/21  Avoid reuse of request buffer.
 * 6.0  Nava  02/22/21  Fixed doxygen issues.
 * 6.0  Nava  02/23/21  To avoid the security glitch with IPI request buffer
 *                      contents added proper validation logic to fill the
 *                      IPI request buffer.
 * 6.0  Nava  03/09/21  Added function pointer validation check.
 * 6.0  Nava  05/17/21  Fixed misra-c violations.
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xilfpga.h"
#include "xilmailbox.h"

/* @cond nocomments */
/************************** Constant Definitions *****************************/
#define PDI_LOAD		0x30701U
#define DELAYED_PDI_LOAD	0x30702U
#define LOAD_PDI_MSG_LEN	0x4U
#define FPGA_IPI_RESP1		0x1U
#define XMAILBOX_DEVICE_ID	0x0U
#define FPGA_PDI_SRC_DDR	0xFU
#define FPGA_IPI_TYPE_BLOCKING	0x1U

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static u32 XFpga_WriteToPl(XFpga *InstancePtr);
/************************** Variable Definitions *****************************/


/* Create a constant pointer to XFpga_WriteToPl. */
u32 (*const Write_To_Pl)(struct XFpgatag *InstancePtr) = XFpga_WriteToPl;

/* @endcond */

/*****************************************************************************/
/**This API, when called, initializes the XFPGA interface with default settings.
 *
 * @param InstancePtr Pointer to the XFpga structure.
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
		(void)memset(InstancePtr, 0, sizeof(*InstancePtr));
		InstancePtr->XFpga_WriteToPl = XFpga_WriteToPl;

		/* Check the pointer was assigned correctly. */
		if (InstancePtr->XFpga_WriteToPl == Write_To_Pl) {
			Status = XFPGA_SUCCESS;
		}
	}

	return Status;
}

/* @cond nocomments */
/*****************************************************************************/
/**
 * This function writes bitstream data into the PL.
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
	volatile u32 Status = XFPGA_FAILURE;
	u32 RecBuffer = XFPGA_FAILURE;
	XMailbox XMboxInstance;
	u32 ReqBuffer[LOAD_PDI_MSG_LEN] = {0U};

	UINTPTR BitstreamAddr = InstancePtr->WriteInfo.BitstreamAddr;

	Status = XMailbox_Initialize(&XMboxInstance, XMAILBOX_DEVICE_ID);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	if (InstancePtr->WriteInfo.Flags == XFPGA_DELAYED_PDI_LOAD) {
		ReqBuffer[0U] = DELAYED_PDI_LOAD;
		ReqBuffer[1U] = (u32)BitstreamAddr; /* Image ID */
		ReqBuffer[2U] = 0U;
		ReqBuffer[3U] = 0U;
	} else if (InstancePtr->WriteInfo.Flags == XFPGA_PDI_LOAD) {
		ReqBuffer[0U] = PDI_LOAD;
		ReqBuffer[1U] = FPGA_PDI_SRC_DDR;
		ReqBuffer[2U] = UPPER_32_BITS(BitstreamAddr);
		ReqBuffer[3U] = LOWER_32_BITS(BitstreamAddr);
	} else {
		Status = XFPGA_FAILURE;
		goto END;
	}

	/* Send an IPI Req Message */
	Status = XFPGA_FAILURE;
	Status = XMailbox_SendData(&XMboxInstance, XMAILBOX_IPIPMC, ReqBuffer,
				   LOAD_PDI_MSG_LEN, XILMBOX_MSG_TYPE_REQ,
				   FPGA_IPI_TYPE_BLOCKING);
	if (Status != (u32)XST_SUCCESS) {
		xil_printf("Sending Req Message Failed\n\r");
		goto END;
	}

	Status = XFPGA_FAILURE;
	Status = XMailbox_Recv(&XMboxInstance, XMAILBOX_IPIPMC, &RecBuffer,
				FPGA_IPI_RESP1, XILMBOX_MSG_TYPE_RESP);
	if (Status == (u32)XST_SUCCESS) {
		Status = RecBuffer;
	}

END:
	return Status;
}

/* @endcond */
