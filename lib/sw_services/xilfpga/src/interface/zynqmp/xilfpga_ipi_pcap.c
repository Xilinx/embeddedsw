/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilfpga_ipi_pcap.c
 *
 * This file contains the definitions of bitstream loading functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ---- ----- --------  -------------------------------------------------------
 * 5.2  Nava  02/14/20  Added Bitstream loading support by using IPI services.
 * 5.3  Nava  06/16/20  Modified the date format from dd/mm to mm/dd.
 * 5.3  Nava  06/29/20  Added asserts to validate input params.
 * 5.3  Nava  09/09/20  Replaced the asserts with input validations for non void
 *                      API's.
 * 6.0  Nava  12/14/20  In XFpga_PL_BitStream_Load() API the argument
 *                      AddrPtr_Size is being used for multiple purposes.
 *                      Use of the same variable for multiple purposes can
 *                      make it more difficult for a person to read (or)
 *                      understand the code and also it leads to a safety
 *                      violation. fixes this  issue by adding a separate
 *                      function arguments to read KeyAddr and
 *                      Size(Bitstream size).
 * 6.0  Nava  01/20/21  Reset the status variable to fail to avoid safety
 *                      violations.
 * 6.0  Nava  01/21/21  Make Status variable volatile to avoid compiler
 *                      optimizations.
 * 6.0  Nava  01/21/21  The usage of XMboxInstance variable is limited only
 *                      to this file. So making this variable as static.
 * 6.0  Nava  02/22/21  Fixed doxygen issues.
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
#define PM_FPGA_LOAD		0x16U
#define PM_FPGA_GET_STATUS	0x17U
#define PM_FPGA_READ		0x2EU
#define FPGA_MSG_LEN		0x5U
#define FPGA_DATA_READBACK	0x1U
#define FPGA_REG_READBACK	0x0U
#define GET_STATUS_MSG_LEN	0x1U
#define FPGA_IPI_TYPE_BLOCKING	0x1U
#define FPGA_IPI_RESP1		0x1U
#define FPGA_IPI_RESP2		0x2U
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static u32 XFpga_IPI_WriteToPl(XFpga *InstancePtr);
static u32 XFpga_IPI_GetPLConfigDataPcap(const XFpga *InstancePtr);
static u32 XFpga_IPI_GetPLConfigRegPcap(const XFpga *InstancePtr);
static u32 XFpga_IPI_PcapStatus(void);
/************************** Variable Definitions *****************************/

static XMailbox XMboxInstance;

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
	u32 Status = XFPGA_FAILURE;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = XFPGA_INVALID_PARAM;
		goto END;
	}

	(void)memset(InstancePtr, 0U, sizeof(*InstancePtr));
	InstancePtr->XFpga_WriteToPl = XFpga_IPI_WriteToPl;
	InstancePtr->XFpga_GetConfigData = XFpga_IPI_GetPLConfigDataPcap;
	InstancePtr->XFpga_GetConfigReg = XFpga_IPI_GetPLConfigRegPcap;
	InstancePtr->XFpga_GetInterfaceStatus = XFpga_IPI_PcapStatus;

	Status = XMailbox_Initialize(&XMboxInstance, 0U);
END:
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
static u32 XFpga_IPI_WriteToPl(XFpga *InstancePtr)
{
	volatile u32 Status = XFPGA_FAILURE;
	u32 ReqBuffer[FPGA_MSG_LEN] = {0};
	UINTPTR BitstreamAddr = InstancePtr->WriteInfo.BitstreamAddr;

	ReqBuffer[0U] = PM_FPGA_LOAD;
	ReqBuffer[1U] = UPPER_32_BITS(BitstreamAddr);
	ReqBuffer[2U] = (u32)BitstreamAddr;

	if ((InstancePtr->WriteInfo.Flags & XFPGA_ENCRYPTION_USERKEY_EN) != 0U)
	{
		ReqBuffer[3U] = InstancePtr->WriteInfo.KeyAddr;
	} else {
		ReqBuffer[3U] = InstancePtr->WriteInfo.Size;
	}

	ReqBuffer[4U] = InstancePtr->WriteInfo.Flags;

	/* Send an IPI Req Message */
	Status = XMailbox_SendData(&XMboxInstance, XMAILBOX_IPI3, ReqBuffer,
				   FPGA_MSG_LEN, XILMBOX_MSG_TYPE_REQ,
				   FPGA_IPI_TYPE_BLOCKING);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XFPGA_FAILURE;
	Status = XMailbox_Recv(&XMboxInstance, XMAILBOX_IPI3, ReqBuffer,
			       FPGA_IPI_RESP1, XILMBOX_MSG_TYPE_RESP);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status =  ReqBuffer[0U];

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function performs the readback of fpga configuration data/registers.
 *
 * @param InstancePtr Pointer to the XFpga structure.
 *
 * @return
 *               - XFPGA_SUCCESS if successful
 *               - XFPGA_FAILURE if unsuccessful
 *
 * @note None.
 ****************************************************************************/
static u32 XFpga_IPI_GetPLConfigDataPcap(const XFpga *InstancePtr)
{
	volatile u32 Status = XFPGA_FAILURE;
	u32 ReqBuffer[FPGA_MSG_LEN] = {0};
	UINTPTR ReadbackAddr = InstancePtr->ReadInfo.ReadbackAddr;

	ReqBuffer[0U] = PM_FPGA_READ;
	ReqBuffer[1U] = InstancePtr->ReadInfo.ConfigReg_NumFrames;
	ReqBuffer[2U] = (u32)ReadbackAddr;
	ReqBuffer[3U] = UPPER_32_BITS(ReadbackAddr);
	ReqBuffer[4U] = FPGA_DATA_READBACK;

	/* Send an IPI Req Message */
	Status = XMailbox_SendData(&XMboxInstance, XMAILBOX_IPI3, ReqBuffer,
				   FPGA_MSG_LEN, XILMBOX_MSG_TYPE_REQ,
				   FPGA_IPI_TYPE_BLOCKING);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XFPGA_FAILURE;
	Status = XMailbox_Recv(&XMboxInstance, XMAILBOX_IPI3, ReqBuffer,
			       FPGA_IPI_RESP2, XILMBOX_MSG_TYPE_RESP);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status =  ReqBuffer[0U];

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function performs the readback of fpga configuration data/registers.
 *
 * @param InstancePtr Pointer to the XFpga structure.
 *
 * @return
 *               - XFPGA_SUCCESS if successful
 *               - XFPGA_FAILURE if unsuccessful
 *
 * @note None.
 ****************************************************************************/
static u32 XFpga_IPI_GetPLConfigRegPcap(const XFpga *InstancePtr)
{
	volatile u32 Status = XFPGA_FAILURE;
	u32 ReqBuffer[FPGA_MSG_LEN] = {0};
	UINTPTR ReadbackAddr = InstancePtr->ReadInfo.ReadbackAddr;

	ReqBuffer[0U] = PM_FPGA_READ;
	ReqBuffer[1U] = InstancePtr->ReadInfo.ConfigReg_NumFrames;
	ReqBuffer[2U] = (u32)ReadbackAddr;
	ReqBuffer[3U] = UPPER_32_BITS(ReadbackAddr);
	ReqBuffer[4U] = FPGA_REG_READBACK;

	/* Send an IPI Req Message */
	Status = XMailbox_SendData(&XMboxInstance, XMAILBOX_IPI3, ReqBuffer,
				   FPGA_MSG_LEN, XILMBOX_MSG_TYPE_REQ,
				   FPGA_IPI_TYPE_BLOCKING);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XFPGA_FAILURE;
	Status = XMailbox_Recv(&XMboxInstance, XMAILBOX_IPI3, ReqBuffer,
			       FPGA_IPI_RESP2, XILMBOX_MSG_TYPE_RESP);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	memcpy((char *)ReadbackAddr, (char *)&ReqBuffer[1U],
		sizeof(ReqBuffer[1U]));
	Status =  ReqBuffer[0U];

END:
	return Status;
}

/*****************************************************************************/
/**
 * Provides the STATUS of PCAP interface
 *
 * @param	None
 *
 * @return	Status of the PCAP interface.
 *
 *****************************************************************************/
static u32 XFpga_IPI_PcapStatus(void)
{
	volatile u32 Status = XFPGA_FAILURE;
	u32 RegVal = XFPGA_INVALID_INTERFACE_STATUS;
	u32 ReqBuffer[FPGA_IPI_RESP2] = {0};

	ReqBuffer[0U] = PM_FPGA_GET_STATUS;
	/* Send an IPI Req Message */
	Status = XMailbox_SendData(&XMboxInstance, XMAILBOX_IPI3, ReqBuffer,
				   GET_STATUS_MSG_LEN, XILMBOX_MSG_TYPE_REQ,
				   FPGA_IPI_TYPE_BLOCKING);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	Status = XFPGA_FAILURE;
	Status = XMailbox_Recv(&XMboxInstance, XMAILBOX_IPI3, ReqBuffer,
			       FPGA_IPI_RESP2, XILMBOX_MSG_TYPE_RESP);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	RegVal = ReqBuffer[1U];

END:
	return RegVal;
}

/* @endcond */
