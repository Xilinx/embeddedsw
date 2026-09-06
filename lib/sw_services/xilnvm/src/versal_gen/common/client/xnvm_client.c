/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xnvm_client.c
 *
 * This file contains the implementation of the client interface functions which are common to the
 * whole XilNvm library and are not specific to BBRAM or eFUSE.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 3.8   tus  08/10/26 Initial release with the library version query
 *                     and server capability bits
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xnvm_client_apis XilNvm Client APIs
 * @{
 */

/*************************************** Include Files *******************************************/
#include "xnvm_client.h"
#include "xnvm_generic.h"
#include "xnvm_ver.h"
#include "xil_util.h"

/************************************ Constant Definitions ***************************************/
#define XNVM_SLR_INDEX_SHIFT		(6U) /**< SLR index shift in the IPI header */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function returns the XilNvm library version that the client is built with.
 *
 * @param	Version		Pointer to hold the 32-bit library version, where the upper 16 bits
 *			hold the major version and the lower 16 bits hold the minor version
 *
 * @return
 *		- XST_SUCCESS  If the library version is read successfully
 *		- XST_INVALID_PARAM  If Version is NULL
 *
 *************************************************************************************************/
int XNvm_GetClientLibVersion(u32 *Version)
{
	int Status = XST_FAILURE;

	if (Version == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	*Version = XNvm_GetLibVersion();
	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends an IPI request to get the XilNvm library version and
 *		capability bits running on the PLM.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	LibInfo		Pointer to Xil_LibInfo. LibVersion and Capability
 *			are filled on success. The Version and NistStatus fields
 *			are reserved for XilNvm and must be ignored.
 *
 * @return
 *		- XST_SUCCESS  If the library information is read successfully
 *		- XST_INVALID_PARAM  If any input parameter is invalid
 *		- XST_FAILURE  If there is a failure
 *
 *************************************************************************************************/
int XNvm_GetServerLibInfo(XNvm_ClientInstance *InstancePtr, Xil_LibInfo *LibInfo)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	/**
	 * - Perform input parameter validation. Return XST_INVALID_PARAM if input parameters are
	 * invalid.
	 */
	if ((InstancePtr == NULL) || (LibInfo == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * Query library info via the Features command using a valid NVM api-id as
	 * the selector, so the command succeeds and the server returns the library
	 * version/capability at fixed indices.
	 */
	XNVM_PACK_PAYLOAD1(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_FEATURES), (u32)XNVM_API_ID_BBRAM_ZEROIZE);

	/**
	 * - Send request using generic API, the server returns the library version and capability
	 * bits along with the status of the command.
	 */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, Response,
				(u32)RESPONSE_ARG_CNT);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/*
	 * XNvm_SendRequest strips the status word, so the data starts at Response[0]
	 * and matches Xil_LibInfo. Copy it in one shot; the server does not emit
	 * Capability[1], so zero it afterwards. Version/NistStatus are reserved for
	 * XilNvm.
	 */
	Status = Xil_SMemCpy(LibInfo, sizeof(Xil_LibInfo), &Response[0U],
			(u32)(XIL_LIBINFO_RESP_WORDS * sizeof(u32)),
			(u32)(XIL_LIBINFO_RESP_WORDS * sizeof(u32)));
	if (Status != XST_SUCCESS) {
		goto END;
	}
	LibInfo->Capability[1U] = 0U;

END:
	return Status;
}
/** @} */
