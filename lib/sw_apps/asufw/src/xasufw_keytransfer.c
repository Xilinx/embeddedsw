/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_keytransfer.c
 *
 * This file contains the key transfer code for ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog   06/25/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_keytransfer.h"
#include "xasufw_ipi.h"
#include "xasufw_status.h"
#include "xaes.h"
#include "xil_util.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_KEY_TX_PAYLOAD_SIZE 	(1U) /**< Key transfer TX payload size */
#define XASUFW_KEY_RX_PAYLOAD_SIZE 	(2U) /**< Key transfer RX payload size */
#define XASUFW_RESPONSE_INDEX_0		(0U) /**< Key transfer response index 0 */
#define XASUFW_RESPONSE_INDEX_1		(1U) /**< Key transfer response index 1 */
#define XASUFW_KEY_TRANSFER_DONE_TIMEOUT	(5000000U) /**< Key transfer done wait timeout
								value in microseconds (5sec) */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function gets three 256-bit keys (two red/black and one PUF key) from PMXC to Key
 * receiver engine in ASU using 128-bit dedicated interface between PMXC and ASU.
 *
 * @param	PufKekGenStatusPtr Pointer to the PUF KEK generation status variable.
 *
 * @return
 *		- XASUFW_SUCCESS, if all three keys are received successfully.
 *		- XASUFW_INVALID_PARAM, if input argument is NULL.
 *		- XASUFW_KEY_TRANSFER_SEND_IPI_FAILED, if failed to send key transfer IPI to PLM.
 *		- XASUFW_KEY_TRANSFER_READ_IPI_FAILED, if failed to read key transfer IPI response from PLM.
 *		- XASUFW_ERR_KV_INTERRUPT_DONE_TIMEOUT, if key transfer done timeout occurs.
 *		- XASUFW_FAILURE, if all three keys are not received.
 *
 *************************************************************************************************/
s32 XAsufw_PmcKeyTransfer(u32 *PufKekGenStatusPtr)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Response[XASUFW_KEY_RX_PAYLOAD_SIZE] = {0U};
	u32 Payload;

	/** Validate input argument. */
	if (PufKekGenStatusPtr == NULL) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Transfer efuse key_0 is black or red based on user configuration. */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_EFUSE_KEY_0_BLACK_OR_RED_OFFSET),
		XASUFW_PMXC_EFUSE_USER_KEY_0);

	/** Transfer efuse key_1 is black or red based on user configuration. */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_EFUSE_KEY_1_BLACK_OR_RED_OFFSET),
		XASUFW_PMXC_EFUSE_USER_KEY_1);

	Payload = XASUFW_PLM_IPI_HEADER(0U, XASUFW_PLM_ASU_KEY_TX_API_ID,
			XASUFW_PLM_ASU_MODULE_ID);

	/** Indicate to PMXC and ASU that ASU is ready to accept key transfer. */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_ASU_PMXC_KEY_TRANSFER_READY_OFFSET),
		XAES_ASU_PMXC_KEY_TRANSFER_READY_MASK);

	/** Send Key transfer IPI request to PLM. */
	Status = XAsufw_SendIpiToPlm(&Payload, XASUFW_KEY_TX_PAYLOAD_SIZE);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_KEY_TRANSFER_SEND_IPI_FAILED;
		goto END_CLR;
	}

	/**
	  * Wait up to 5 seconds for the HW to set the KV interrupt status bit
	  * indicating key transfer (KT) is done.
	  */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = (s32)Xil_WaitForEvent((XASU_XKEY_0_BASEADDR + XAES_KV_INTERRUPT_STATUS_OFFSET),
			XAES_KV_INTERRUPT_STATUS_DONE_MASK, XAES_KV_INTERRUPT_STATUS_DONE_MASK,
			XASUFW_KEY_TRANSFER_DONE_TIMEOUT);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ERR_KV_INTERRUPT_DONE_TIMEOUT;
		goto END_CLR;
	}

	/**
	 * Disable ASU Key transfer immediately after key transfer is done. Disabling this
	 * register will set the done bit in PLM side to indicate that key transfer is done.
	 */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_ASU_PMXC_KEY_TRANSFER_READY_OFFSET),
		XAES_ASU_PMXC_KEY_TRANSFER_READY_DISABLE);

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	/** Read Key transfer response received from PLM. */
	Status = XAsufw_ReadIpiRespFromPlm(Response, XASUFW_KEY_RX_PAYLOAD_SIZE);
	if ((Status != XASUFW_SUCCESS)) {
		Status = XASUFW_KEY_TRANSFER_READ_IPI_FAILED;
		goto END_CLR;
	}

	Status = (s32)Response[XASUFW_RESPONSE_INDEX_0];
	if (Response[XASUFW_RESPONSE_INDEX_0] != (u32)XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_INFO, "Key transfer failed. Error: %08x\r\n",
			Response[XASUFW_RESPONSE_INDEX_0]);
		Status = (s32)Response[XASUFW_RESPONSE_INDEX_0];
		goto END_CLR;
	}

	*PufKekGenStatusPtr = Response[XASUFW_RESPONSE_INDEX_1];

END_CLR:
	/** Clear KV interrupt status. */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_KV_INTERRUPT_STATUS_OFFSET),
		XAES_KV_INTERRUPT_STATUS_CLEAR_MASK);

	/** Disable ASU Key transfer. */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_ASU_PMXC_KEY_TRANSFER_READY_OFFSET),
		XAES_ASU_PMXC_KEY_TRANSFER_READY_DISABLE);
END:

	return Status;
}
/** @} */
