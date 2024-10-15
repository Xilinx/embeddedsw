/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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

/************************** Constant Definitions *****************************/

static XSecure_Sha *XSecureShaInstPtr = NULL;

/************************** Function Prototypes *****************************/

static int XSecure_ShaModeInit(u32 ShaMode);
static int XSecure_ShaModeUpdate(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size,
	u32 EndLast);
static int XSecure_ShaModeFinish(u32 OutAddrLow, u32 OutAddrHigh, u32 HashSize);

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
	const u32 *Pload;

	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
        case XSECURE_API(XSECURE_API_SHA_INIT):
		Status = XSecure_ShaModeInit(Pload[0U]);
		break;
	case XSECURE_API(XSECURE_API_SHA_UPDATE):
		Status = XSecure_ShaModeUpdate(Pload[0U], Pload[1U], Pload[2U], Pload[3U]);
		break;
	case XSECURE_API(XSECURE_API_SHA_FINISH):
		Status = XSecure_ShaModeFinish(Pload[0U], Pload[1U], Pload[2U]);
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
                Status = XST_INVALID_PARAM;
                break;
	}
END:
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
static int XSecure_ShaModeInit(u32 ShaMode)
{
	int Status = XST_FAILURE;
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);

	if ((ShaMode  == XSECURE_SHA3_384) || (ShaMode  == XSECURE_SHAKE_256) ||
	    (ShaMode == XSECURE_SHA3_256) || (ShaMode  == XSECURE_SHA3_512)) {
		XSecureShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	}
	else if ((ShaMode == XSECURE_SHA2_384) || (ShaMode == XSECURE_SHA2_256) ||
		 (ShaMode == XSECURE_SHA2_512)) {
		XSecureShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);
	}
	else {
		XSecure_Printf(DEBUG_PRINT_ALWAYS, "Invalid SHA mode\r\n");
	}

	if (NULL == PmcDmaInstPtr) {
		goto END;
	}

	Status = XSecure_ShaInitialize(XSecureShaInstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
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
static int XSecure_ShaModeUpdate(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size,
				u32 EndLast)
{
	int Status = XST_FAILURE;
	u64 DataAddr = ((u64)SrcAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)SrcAddrLow;

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
static int XSecure_ShaModeFinish(u32 OutAddrLow, u32 OutAddrHigh, u32 HashSize)
{
	int Status = XST_FAILURE;
	u64 DstAddr = ((u64)OutAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)OutAddrLow;
	XSecure_Sha3Hash Hash = {0U};

	Status = XSecure_ShaFinish(XSecureShaInstPtr, (u64)(UINTPTR)&Hash, HashSize);
	if (Status == XST_SUCCESS) {
		Status = XPlmi_DmaXfr((u64)(UINTPTR)(Hash.Hash), DstAddr,
				XSECURE_SHA3_HASH_LENGTH_IN_WORDS, XPLMI_PMCDMA_0);
	}

	return Status;
}
