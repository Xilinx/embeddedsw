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
 *       kp   09/23/26 Program both eFUSE black/red registers from one KeyForm argument
 *       vm    10/05/26 Kept the keep-alive counter advancing while the key transfer done
 *                      status is awaited
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
#include "xasufw_debug.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xaes.h"
#include "xil_util.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_KEY_TX_PAYLOAD_SIZE 	(1U) /**< Key transfer TX payload size */
#define XASUFW_KEY_RX_PAYLOAD_SIZE 	(2U) /**< Key transfer RX payload size */
#define XASUFW_RESPONSE_INDEX_0		(0U) /**< Key transfer response index 0 */
#define XASUFW_RESPONSE_INDEX_1		(1U) /**< Key transfer response index 1 */
#define XASUFW_KEY_TRANSFER_DONE_TIMEOUT	(5000000U) /**< Key transfer done wait timeout
								value as a count of polls */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function gets three 256-bit keys (two user keys and one PUF key) from PMXC
 * 		to the key receiver engine in the ASU using the 128-bit dedicated interface between
 * 		PMXC and the ASU. Both user keys are transferred in KeyForm. The caller keeps the
 * 		key it needs and clears the other two.
 *
 * @param	PufKekGenStatusPtr Pointer to the PUF KEK generation status variable.
 * @param	KeyForm		Form applied to both user keys for this transfer
 * 				(XASU_EFUSE_BLACK_KEY or XASU_EFUSE_RED_KEY).
 *
 * @return
 *		- XASUFW_SUCCESS, if all three keys are received successfully.
 *		- XASUFW_INVALID_PARAM, if PufKekGenStatusPtr is NULL or KeyForm is not black
 *		  or red.
 *		- XASUFW_KEY_TRANSFER_SEND_IPI_FAILED, if failed to send key transfer IPI to PLM.
 *		- XASUFW_KEY_TRANSFER_READ_IPI_FAILED, if failed to read key transfer IPI response
 *		  from PLM.
 *		- XASUFW_ERR_KV_INTERRUPT_DONE_TIMEOUT, if key transfer done timeout occurs.
 *		- XASUFW_FAILURE, if all three keys are not received.
 *
 *************************************************************************************************/
s32 XAsufw_PmcKeyTransfer(u32 *PufKekGenStatusPtr, u32 KeyForm)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Response[XASUFW_KEY_RX_PAYLOAD_SIZE] = {0U};
	u32 Payload;

	/** Validate input arguments. One form applies to both user keys. */
	if ((PufKekGenStatusPtr == NULL) ||
	    ((KeyForm != XASU_EFUSE_BLACK_KEY) && (KeyForm != XASU_EFUSE_RED_KEY))) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Program both eFUSE black/red registers with the form this transfer needs. */
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_EFUSE_KEY_0_BLACK_OR_RED_OFFSET), KeyForm);
	XAsufw_WriteReg((XASU_XKEY_0_BASEADDR + XAES_EFUSE_KEY_1_BLACK_OR_RED_OFFSET), KeyForm);

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
	  * Wait for the HW to set the KV interrupt status bit indicating key transfer (KT) is
	  * done, for the bounded number of polls the timeout below allows. The wait advances
	  * the keep-alive counter, because this handler holds the task dispatcher for the whole
	  * wait, which can outlast the keep-alive window PLM allows.
	  */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_WaitForEventKeepAlive(
			(XASU_XKEY_0_BASEADDR + XAES_KV_INTERRUPT_STATUS_OFFSET),
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
