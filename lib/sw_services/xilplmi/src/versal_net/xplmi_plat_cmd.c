/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_net/xplmi_plat_cmd.c
* @addtogroup xplmi_apis XilPlmi versal_net APIs
* @{
* @cond xplmi_internal
* This file contains versalnet specific cmds and modules logic.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  bm   07/06/2022 Initial release
*       ma   07/08/2022 Add ScatterWrite and ScatterWrite2 commands to versal
*       ma   07/08/2022 Add support for Tamper Trigger over IPI
*       ma   07/29/2022 Replaced XPAR_XIPIPSU_0_DEVICE_ID macro with
*                       XPLMI_IPI_DEVICE_ID
*       bm   09/14/2022 Move ScatterWrite commands from common to versal_net
* 1.01  ng   11/11/2022 Fixed doxygen file name error
*       kpt  01/04/2023 Added XPlmi_SetFipsKatMask command
*       dd   03/28/2023 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
* 1.02  bm   06/23/2023 Removed existing hardcoded logic of validating IPI commands
*       bm   07/06/2023 Added XPlmi_RunProc command
*       bm   07/06/2023 Refactored Proc logic to more generic logic
*       bm   07/06/2023 Added List commands
*       sk   08/29/2023 Updated IPI Timeout to ~5sec to avoid any DoS issue
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_modules.h"
#include "xplmi.h"
#include "xplmi_ipi.h"
#include "xplmi_generic.h"
#include "xplmi_event_logging.h"
#include "xplmi_cmd.h"
#include "xil_util.h"
#include "xplmi_cdo.h"
#include "xplmi_update.h"

/************************** Constant Definitions *****************************/
/* PSM sequence related constants definitions */
#define XPLMI_BUFFER_PSM_SEND_API_ID		(0x8U) /**< API Id */
#define XPLMI_BUFFER_PSM_SEND_API_ID_IDX		(0U) /**< API Id index */
#define XPLMI_BUFFER_PSM_SEND_START_ADDR_IDX	(1U) /**< Start address index */
#define XPLMI_BUFFER_PSM_SEND_END_ADDR_IDX	(2U) /**< End address index */
#define XPLMI_BUFFER_PAYLOAD_ARG_CNT		(8U) /**< Payload argument count */

/* IPI Max Timeout calculated based on 460MHz core freq*/
#define IPI_MAX_TIMEOUT			(0x2DB267AAU) /**< IPI Max timeout ~5sec*/

/* Command related macros */
#define XPLMI_SCATTER_WRITE_PAYLOAD_LEN			(2U) /**< Scatter write payload length */
#define XPLMI_SCATTER_WRITE2_PAYLOAD_LEN		(3U) /**< Scatter write2 payload length */
#define XPLMI_FIPS_WRITE_KATMASK_PAYLOAD_LEN	(7U) /**< FIPS write KAT mask payload length */

/* Address Buffer related macros */
#define XPLMI_MAX_ADDR_BUFFERS (1U)
#define XPLMI_MAX_ADDR_LIST_CNT (200U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/* Address Buffers List */
static XPlmi_BufferList AddressBufferList = {0U};

/*****************************************************************************/
/**
 * @brief	This function initializes Address Buffer List
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_SetAddrBufferList(void)
{
	static u32 AddrBuffer[XPLMI_MAX_ADDR_LIST_CNT];
	static XPlmi_BufferData AddressBuffers[XPLMI_MAX_ADDR_BUFFERS + 1U] = {0U};

	AddressBufferList.Data = AddressBuffers;
	AddressBufferList.Data[0U].Addr = (u32)(UINTPTR)AddrBuffer;
	AddressBufferList.BufferMemSize = sizeof(AddrBuffer);
	AddressBufferList.IsBufferMemAvailable = (u8)TRUE;
	AddressBufferList.MaxBufferCount = XPLMI_MAX_ADDR_BUFFERS;
}

/*****************************************************************************/
/**
 * @brief	This function provides In Place PLM Update support.
 *
 * @param	Cmd is pointer to the command structure
 *		Command payload parameters are
 *		- High Addr
 *		- Low Addr
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_InPlacePlmUpdate(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_DETAILED, "%s\n\r", __func__);
	Status = XPlmi_PlmUpdate(Cmd);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to send psm_sequence commands data address
 *			and length to be processed by PSM.
 *
 * @param	BuffAddr is the address of the buffer
 * @param	BuffLen is the length of the buffer in 32-bit word
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_SendToPsm(u32 BuffAddr, u32 BuffLen)
{
	int Status = XST_FAILURE;
	u32 Response[XPLMI_BUFFER_PAYLOAD_ARG_CNT] = {0U};
	u32 LocalPayload[XPLMI_BUFFER_PAYLOAD_ARG_CNT] = {0U};

	LocalPayload[XPLMI_BUFFER_PSM_SEND_API_ID_IDX] = XPLMI_BUFFER_PSM_SEND_API_ID;
	LocalPayload[XPLMI_BUFFER_PSM_SEND_START_ADDR_IDX] = BuffAddr;
	LocalPayload[XPLMI_BUFFER_PSM_SEND_END_ADDR_IDX] =  BuffLen;

#ifdef XPLMI_IPI_DEVICE_ID
	Status = XPlmi_IpiPollForAck(IPI_PMC_ISR_PSM_BIT_MASK, IPI_MAX_TIMEOUT);
	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_GENERAL,"%s: ERROR: Timeout expired\n", __func__);
		goto END;
	}

	Status = XPlmi_IpiWrite(IPI_PMC_ISR_PSM_BIT_MASK, LocalPayload,
			XPLMI_BUFFER_PAYLOAD_ARG_CNT, XIPIPSU_BUF_TYPE_MSG);
	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_GENERAL,"%s: ERROR writing to IPI \
			request buffer\n", __func__);
		goto END;
	}

	Status = XPlmi_IpiTrigger(IPI_PMC_ISR_PSM_BIT_MASK);
	if (XST_SUCCESS != Status){
		goto END;
	}

	/* Wait until current IPI interrupt is handled by target module */
	Status = XPlmi_IpiPollForAck(IPI_PMC_ISR_PSM_BIT_MASK, IPI_MAX_TIMEOUT);
	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_GENERAL,"%s: ERROR: Timeout expired\r\n", __func__);
		goto END;
	}

	Status = XPlmi_IpiRead(IPI_PMC_ISR_PSM_BIT_MASK, Response,
		       XPLMI_BUFFER_PAYLOAD_ARG_CNT, XIPIPSU_BUF_TYPE_RESP);
	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_GENERAL,"%s: ERROR: Reading from IPI \
			Response buffer\r\n", __func__);
		goto END;
	}

	Status = (int)Response[0];
#else
	Status = XST_FAILURE;

#endif /* XPLMI_IPI_DEVICE_ID */
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to transfer all psm_sequence commands to
 *			PSM RAM regions which are then processed by PSM immediately.
 *
 * @param	Cmd is a pointer to command structure
 *
 * @return
 * 			- XST_SUCCESS if success.
 * 			- XPLMI_ERR_PROC_LPD_NOT_INITIALIZED if failed to initialize LPD.
 * 			- XPLMI_UNSUPPORTED_PROC_LENGTH if received proc does not fit in the
 * 			proc memory.
 *
 *****************************************************************************/
int XPlmi_PsmSequence(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 SrcAddr = (u32)(&Cmd->Payload[0U]);
	u32 CurrPayloadLen = Cmd->PayloadLen;;
	XPlmi_BufferList *BufferList = XPlmi_GetBufferList(XPLMI_PSM_BUFFER_LIST);

	XPLMI_EXPORT_CMD(XPLMI_PSM_SEQUENCE_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ONE, XPLMI_UNLIMITED_ARG_CNT);

	if ((XPlmi_IsLpdInitialized() != (u8)TRUE) ||
		(BufferList->IsBufferMemAvailable != (u8)TRUE)) {
		XPlmi_Printf(DEBUG_GENERAL, "LPD is not initialized or Buffer memory "
			"is not available for psm sequence\r\n");
		Status = (int)XPLMI_ERR_PROC_LPD_NOT_INITIALIZED;
		goto END;
	}

	if (Cmd->ProcessedLen == 0){
		/* This is the beggining of the CMD execution. */
		/* Latch on the first address of psm_sequence */
		Cmd->ResumeData[1U] = (u32)BufferList->Data[BufferList->BufferCount].Addr;
		/* Check if new proc length fits in the proc allocated memory */
		if ((Cmd->ResumeData[1U] + (Cmd->Len * XPLMI_WORD_LEN)) >
			((u32)BufferList->Data[0U].Addr + BufferList->BufferMemSize)) {
			XPlmi_Printf(DEBUG_GENERAL,"psm_sequence too large \
				to fit in BufferList.\n\r");
			Status = (int)XPLMI_UNSUPPORTED_PROC_LENGTH;
			goto END;
		}
		/* Store the destination of the DMA to Resume Data to handle resume case*/
		Cmd->ResumeData[0U] = (u32)BufferList->Data[BufferList->BufferCount].Addr;
	}

	/* Copy the received proc to proc memory */
	Status = XPlmi_DmaTransfer(Cmd->ResumeData[0U], SrcAddr, CurrPayloadLen,
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((Cmd->ProcessedLen + Cmd->PayloadLen) == Cmd->Len ) {
		/* This is the last payload(in resume case)*/
		Status = XPlmi_SendToPsm(Cmd->ResumeData[1U], Cmd->Len);
		if (Status != XST_SUCCESS){
			XPlmi_Printf(DEBUG_GENERAL,"psm_sequence failed to execute.\
				Status=%x\n\r",Status);
		}
		/* We done here */
		goto END;
	}

	/* Update destination address to handle resume case */
	Cmd->ResumeData[0U] += (CurrPayloadLen * XPLMI_WORD_LEN);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will write single 32 bit value to multiple addresses
 *			which are specified in the payload.
 *  		Command payload parameters are
 *			- Value
 *			- Address[N]: array of N addresses
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS if success.
 * 			- XPLMI_ERR_INVALID_PAYLOAD_LEN on invalid payload length.
 *
 *****************************************************************************/
int XPlmi_ScatterWrite(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 StartIndex;
	u32 Index;

	XPLMI_EXPORT_CMD(XPLMI_SCATTER_WRITE_CMD_ID, XPLMI_MODULE_GENERIC_ID,
			XPLMI_CMD_ARG_CNT_TWO, XPLMI_UNLIMITED_ARG_CNT);

	/* Take care of resume case when long command can be split */
	if (Cmd->ProcessedLen == 0U) {
		/*
		 * Sanity check Arguments
		 */
		if (Cmd->PayloadLen < XPLMI_SCATTER_WRITE_PAYLOAD_LEN) {
			XPlmi_Print(DEBUG_GENERAL, "Scatter_write: invalid "
				"payload length 0x%x which is less than 2", Cmd->PayloadLen);
			Status = (int)XPLMI_ERR_INVALID_PAYLOAD_LEN;
			goto END;
		}

		/* Save value at very first part of the split */
		Cmd->ResumeData[0U] = Cmd->Payload[0U];
		/* Also the start index of the address[i] is 1 in first part */
		StartIndex = 1U;
	} else {
		/* Start index of the address[i] in other parts is 0 */
		StartIndex = 0U;
	}

	/* Start writing out to the addresses */
	for (Index = StartIndex; Index < Cmd->PayloadLen; Index++) {
		XPlmi_Out32(Cmd->Payload[Index], Cmd->ResumeData[0U]);
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will write 2 32-bit values to multiple addresses
 *			which are specified by the payload.
 *  		Command payload parameters are
 *			- Value1
 * 			- Value2
 *			- Address[N]: array of N addresses
 *			where Address[i] = Value1 and Address[i] + 4 = Value2
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS if success.
 * 			- XPLMI_ERR_INVALID_PAYLOAD_LEN on invalid payload length.
 *
 *****************************************************************************/
int XPlmi_ScatterWrite2(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 StartIndex;
	u32 Index;

	XPLMI_EXPORT_CMD(XPLMI_SCATTER_WRITE2_CMD_ID, XPLMI_MODULE_GENERIC_ID,
			XPLMI_CMD_ARG_CNT_FOUR, XPLMI_UNLIMITED_ARG_CNT);

	/* Take care of resume case when long command can be split */
	if (Cmd->ProcessedLen == 0U) {
		/*
		* Sanity check Arguments
		*/
		if (Cmd->PayloadLen < XPLMI_SCATTER_WRITE2_PAYLOAD_LEN) {
			XPlmi_Print(DEBUG_GENERAL, "Scatter_write2: invalid "
				"payload length 0x%x which is less than 3", Cmd->PayloadLen);
			Status = (int)XPLMI_ERR_INVALID_PAYLOAD_LEN;
			goto END;
		}

		/* Save values at very first part of the split */
		Cmd->ResumeData[0U] = Cmd->Payload[0U];
		Cmd->ResumeData[1U] = Cmd->Payload[1U];
		/* Also the start index of the address[i] is 2 in first part */
		StartIndex = 2U;
	} else {
		/* Start index of the address[i] in other parts is 0 */
		StartIndex = 0U;
	}

	/* Start writing out to the address */
	for (Index = StartIndex; Index < Cmd->PayloadLen; Index++) {
		XPlmi_Out32(Cmd->Payload[Index], Cmd->ResumeData[0U]);
		XPlmi_Out32(Cmd->Payload[Index] + XPLMI_WORD_LEN, Cmd->ResumeData[1U]);
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will store the KAT mask set by the user so that PLM
 *          can monitor the RTCA and compare it with KAT masks before going into
 *          FIPS operational state
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS if success.
 * 			- XPLMI_ERR_INVALID_PAYLOAD_LEN on invalid payload length.
 *
 *****************************************************************************/
int XPlmi_SetFipsKatMask(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	XPlmi_FipsKatMask *FipsKatMask = XPlmi_GetFipsKatMaskInstance();

	XPLMI_EXPORT_CMD(XPLMI_SET_FIPS_MASK_CMD_ID, XPLMI_MODULE_GENERIC_ID,
			XPLMI_CMD_ARG_CNT_SEVEN, XPLMI_CMD_ARG_CNT_SEVEN);

	if (Cmd->PayloadLen != XPLMI_FIPS_WRITE_KATMASK_PAYLOAD_LEN) {
		XPlmi_Print(DEBUG_GENERAL, "FIPS_SetKatMask: invalid "
				"payload length %d which is less than 7", Cmd->PayloadLen);
		Status = (int)XPLMI_ERR_INVALID_PAYLOAD_LEN;
		goto END;
	}

	Status = Xil_SMemSet(FipsKatMask, sizeof(XPlmi_FipsKatMask), 0U,
				sizeof(XPlmi_FipsKatMask));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	FipsKatMask->RomKatMask = Cmd->Payload[0U] & XPLMI_ROM_KAT_MASK;
	FipsKatMask->PlmKatMask = Cmd->Payload[1U] & XPLMI_KAT_MASK;
	FipsKatMask->DDRKatMask = Cmd->Payload[2U];
	FipsKatMask->HnicCpm5NPcideKatMask = Cmd->Payload[3U] & XPLMI_HNIC_CPM5N_PCIDE_KAT_MASK;
	FipsKatMask->PKI0KatMask = Cmd->Payload[4U];
	FipsKatMask->PKI1KatMask = Cmd->Payload[5U];
	FipsKatMask->PKI2KatMask = Cmd->Payload[6U] & XPLMI_PKI_KAT_MASK;

	Status = XPlmi_CheckAndUpdateFipsState();
END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function will run the already stored proc if present
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS if success and error code if failure
 *
 *****************************************************************************/
int XPlmi_RunProc(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;

	XPLMI_EXPORT_CMD(XPLMI_RUN_PROC_CMD_ID, XPLMI_MODULE_GENERIC_ID,
			XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);

	/* Execute Proc with the given Proc ID */
	Status = XPlmi_ExecuteProc(Cmd->Payload[0U]);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will create a list with given list of addresses
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS if success and error code if failure
 *
 *****************************************************************************/
int XPlmi_ListSet(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 ListId = 0U;

	XPLMI_EXPORT_CMD(XPLMI_LIST_SET_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);

	if (Cmd->ProcessedLen == 0U) {
		ListId = Cmd->Payload[0U];
	}

	Status = XPlmi_StoreBuffer(Cmd, ListId, &AddressBufferList);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will execute write for the requested list of
 *		addresses
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS if success and error code if failure
 *
 *****************************************************************************/
int XPlmi_ListWrite(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 ListId = Cmd->Payload[0U];
	u32 Value = Cmd->Payload[1U];
	u64 BufAddr;
	const u32 *BufPtr;
	u32 BufLen;
	u32 Index;

	XPLMI_EXPORT_CMD(XPLMI_LIST_WRITE_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);

	Status = XPlmi_SearchBufferList(&AddressBufferList, ListId, &BufAddr, &BufLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	BufPtr = (u32*)(UINTPTR)BufAddr;

	for (Index = 0U; Index < BufLen; Index++) {
		XPlmi_Out32(BufPtr[Index], Value);
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will execute mask write for the requested list of
 *		addresses
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS if success and error code if failure
 *
 *****************************************************************************/
int XPlmi_ListMaskWrite(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 ListId = Cmd->Payload[0U];
	u32 Mask = Cmd->Payload[1U];
	u32 Value = Cmd->Payload[2U];
	u64 BufAddr;
	const u32 *BufPtr;
	u32 BufLen;
	u32 Index;

	XPLMI_EXPORT_CMD(XPLMI_LIST_MASK_WRITE_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_THREE, XPLMI_CMD_ARG_CNT_THREE);

	Status = XPlmi_SearchBufferList(&AddressBufferList, ListId, &BufAddr, &BufLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	BufPtr = (u32*)(UINTPTR)BufAddr;

	for (Index = 0U; Index < BufLen; Index++) {
		XPlmi_UtilRMW(BufPtr[Index], Mask, Value);
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will execute mask poll for the requested list of
 *		addresses
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS if success and error code if failure
 *
 *****************************************************************************/
int XPlmi_ListMaskPoll(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 ListId = Cmd->Payload[0U];
	u64 BufAddr;
	const u32 *BufPtr;
	u32 BufLen;
	u32 Index;

	XPLMI_EXPORT_CMD(XPLMI_LIST_MASK_POLL_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_FOUR, XPLMI_CMD_ARG_CNT_SIX);

	Status = XPlmi_SearchBufferList(&AddressBufferList, ListId, &BufAddr, &BufLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	BufPtr = (u32 *)(UINTPTR)BufAddr;

	for (Index = 0U; Index < BufLen; Index++) {
		Status = XPlmi_GenericMaskPoll(Cmd, (u64)BufPtr[Index],
				XPLMI_LIST_MASK_POLL_32BIT_TYPE);
		if (Status != XST_SUCCESS) {
			break;
		}
	}

END:
	return Status;
}
