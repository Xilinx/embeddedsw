/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha_ipihandler.c
* @addtogroup xsecure_apis XilSecure Versal_AIEPG2 APIs
* @{
* @cond xsecure_internal
* This file contains the XilSecure SHA IPI Handler definition.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.4   kal  07/24/24 Initial release
*       tri  10/07/24 Added easier approach to enable SHA2 Crypto engine in PMC
*       pre  03/02/2025 Implemented task based event notification functionality for SHA IPI events
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_dma.h"
#include "xsecure_defs.h"
#include "xsecure_sha.h"
#include "xsecure_sha_ipihandler.h"
#include "xsecure_init.h"
#include "xsecure_error.h"
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xsecure_resourcehandling.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/
static int XSecure_ShaModeInit(XSecure_Sha *XSecureShaInstPtr, u32 ShaMode);
static int XSecure_ShaModeUpdate(XSecure_Sha *XSecureShaInstPtr, u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size,
	u32 EndLast);
static int XSecure_ShaModeFinish(XSecure_Sha *XSecureShaInstPtr, u32 OutAddrLow, u32 OutAddrHigh, u32 HashSize);

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return
 *	-	XST_SUCCESS - If the handler execution is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_ShaIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	int SStatus = XST_FAILURE;
	const u32 *Pload;
	u32 ApiId;
	XSecure_Sha *XSecureShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	XPlmi_CoreType Core = XPLMI_SHA3_CORE;

	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	ApiId = Cmd->CmdId & XSECURE_API_ID_MASK;

	if ((ApiId == XSECURE_API_SHA2_INIT) || (ApiId == XSECURE_API_SHA2_UPDATE) ||
	    (ApiId == XSECURE_API_SHA2_FINISH)) {
		XSecureShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);
		Core = XPLMI_SHA2_CORE;
	}

	/** SHA IPI event handling */
	Status = XSecure_IpiEventHandling(Cmd, Core);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Pload = Cmd->Payload;

	switch (ApiId) {
        case XSECURE_API(XSECURE_API_SHA_INIT):
		case XSECURE_API(XSECURE_API_SHA2_INIT):
		Status = XSecure_ShaModeInit(XSecureShaInstPtr, Pload[0U]);
		break;
	case XSECURE_API(XSECURE_API_SHA_UPDATE):
	case XSECURE_API(XSECURE_API_SHA2_UPDATE):
		Status = XSecure_ShaModeUpdate(XSecureShaInstPtr, Pload[0U], Pload[1U], Pload[2U], Pload[3U]);
		break;
	case XSECURE_API(XSECURE_API_SHA_FINISH):
	case XSECURE_API(XSECURE_API_SHA2_FINISH):
		Status = XSecure_ShaModeFinish(XSecureShaInstPtr, Pload[0U], Pload[1U], Pload[2U]);
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
                Status = XST_INVALID_PARAM;
                break;
	}
END:
	if(XSecureShaInstPtr->ShaState == XSECURE_SHA_INITIALIZED) {
		SStatus = XSecure_MakeResFree(Core);
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function initializes SHA3 instance.
 *
 * @return
 *	-	XST_SUCCESS - If the initialization is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_ShaModeInit(XSecure_Sha *XSecureShaInstPtr, u32 ShaMode)
{
	int Status = XST_FAILURE;
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);

	if (NULL == PmcDmaInstPtr) {
		goto END;
	}

	if (XSecureShaInstPtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_ShaStart(XSecureShaInstPtr, (XSecure_ShaMode)ShaMode);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_ShaUpdate64Bit or
 * 		XSecure_ShaFinish based on the Continue bit in the command
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the input data
 * 				on which hash has to be calculated
 * @param	SrcAddrHigh	- Higher 32 bit address of the input data
 * 				on which hash has to be calculated
 * @param	Size		- Size of the input data in bytes to be
 * 				updated
 * @param	DstAddrLow	- Lower 32 bit address of the output data
 * 				where hash to be stored
 * @param	DstAddrHigh	- Higher 32 bit address of the output data
 * 				where hash to be stored
 *
 * @return
 *	-	XST_SUCCESS - If the sha update/fnish is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_ShaModeUpdate(XSecure_Sha *XSecureShaInstPtr, u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size,
				u32 EndLast)
{
	int Status = XST_FAILURE;
	u64 DataAddr = ((u64)SrcAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)SrcAddrLow;

	if (XSecureShaInstPtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if (EndLast == TRUE) {
		Status = XSecure_ShaLastUpdate(XSecureShaInstPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	Status = XSecure_ShaUpdate(XSecureShaInstPtr, DataAddr, Size);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_ShaFinish function.
 *
 * @param	HashSize	- Size of the input data in bytes to be
 * 				updated
 * @param	DstAddrLow	- Lower 32 bit address of the output data
 * 				where hash to be stored
 * @param	DstAddrHigh	- Higher 32 bit address of the output data
 * 				where hash to be stored
 *
 * @return
 *	-	XST_SUCCESS - If the sha update/fnish is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_ShaModeFinish(XSecure_Sha *XSecureShaInstPtr, u32 OutAddrLow, u32 OutAddrHigh, u32 HashSize)
{
	int Status = XST_FAILURE;
	u64 DstAddr = ((u64)OutAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)OutAddrLow;
	XSecure_Sha3Hash Hash = {0U};

	if (XSecureShaInstPtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_ShaFinish(XSecureShaInstPtr, (u64)(UINTPTR)&Hash, HashSize);
	if (Status == XST_SUCCESS) {
		Status = XPlmi_DmaXfr((u64)(UINTPTR)(Hash.Hash), DstAddr,
				XSECURE_SHA3_HASH_LENGTH_IN_WORDS, XPLMI_PMCDMA_0);
	}

END:
	return Status;
}
