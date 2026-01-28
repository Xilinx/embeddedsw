/******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha_ipihandler.c
* @addtogroup xsecure_apis XilSecure Versal_2Ve_2Vm APIs
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
*       pre  03/02/25 Implemented task based event notification functionality for SHA IPI events
*       pre  04/16/25 Fixed warning
* 5.6   mb   09/09/25 Return error code on SHA IPI event handling failure
*   	obs  09/23/25 Added support for Verifying Address Range
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
#include "xil_sutil.h"
#include "xplmi_plat.h"

/************************** Constant Definitions *****************************/

#define XSECURE_SHA_START	(0x1U)	/**< Operation flags for SHA start */
#define XSECURE_SHA_UPDATE      (0x2U)	/**< Operation flags for SHA update */
#define XSECURE_SHA_FINISH      (0x4U)	/**< Operation flags for SHA finish */

/************************** Function Prototypes *****************************/
static int XSecure_ShaOperation(u32 SubsystemId, XSecure_Sha *XSecureShaInstPtr, u32 AddrLow, u32 AddrHigh);
/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return
 *		- XST_SUCCESS - If the handler execution is successful
 *		- XST_INVALID_PARAM - If Cmd is NULL or invalid API ID
 *		- XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_ShaIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	int SStatus = XST_FAILURE;
	const u32 *Pload;
	u32 ApiId;
	XSecure_Sha *XSecureShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	XPlmi_CoreType Core =  XPLMI_SHA3_CORE;

	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	ApiId = Cmd->CmdId & XSECURE_API_ID_MASK;

	if (ApiId == (u32)XSECURE_API_SHA2_OPERATION) {
		XSecureShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);
		Core = XPLMI_SHA2_CORE;
	}

	/** SHA IPI event handling */
	Status = XSecure_IpiEventHandling(Cmd, Core);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Pload = Cmd->Payload;

	switch (ApiId) {
        case XSECURE_API(XSECURE_API_SHA3_OPERATION):
	case XSECURE_API(XSECURE_API_SHA2_OPERATION):
		Status = XSecure_ShaOperation(Cmd->SubsystemId, XSecureShaInstPtr, Pload[0U], Pload[1U]);
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
 * @brief       This function performs SHA operation based on operation flags
 * 		provided in IPI payload and returns the result.
 *
 * @param 	SubsystemId			Subsystem ID.
 * @param	XSecureShaInstPtr	Pointer to the SHA instance
 * @param	AddrLow			Lower 32 bit address of XSecure_ShaOpParams
 * 					structure
 * @param	AddrHigh		upper 32 bit address of XSecure_ShaOpParams
 * 					structure
 * @return
 *		- XST_SUCCESS - If the SHA operation is successful
 *		- XST_INVALID_PARAM - If XSecureShaInstPtr is NULL
 *		- XSECURE_ERR_INVALID_ADDR_RANGE - If address range validation fails
 *		- XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XSecure_ShaOperation(u32 SubsystemId, XSecure_Sha *XSecureShaInstPtr, u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 ShaParamsAddr = ((u64)AddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)AddrLow;
	XSecure_ShaOpParams ShaParams __attribute__ ((aligned (32U)));;

	if (XSecureShaInstPtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, ShaParamsAddr, sizeof(ShaParams), Status, XSECURE_ERR_INVALID_ADDR_RANGE, END);

	Status = XPlmi_MemCpy64((u64)(UINTPTR)&ShaParams, ShaParamsAddr, sizeof(ShaParams));
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	/**
	 * Validate internal address fields in the copied structure
	 */
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, ShaParams.DataAddr, ShaParams.DataSize, Status, XSECURE_ERR_INVALID_ADDR_RANGE, END);
	XPLMI_VERIFY_ADDR_RANGE(SubsystemId, ShaParams.HashAddr, ShaParams.HashBufSize, Status, XSECURE_ERR_INVALID_ADDR_RANGE, END);


	if ((ShaParams.OperationFlags & XSECURE_SHA_START) == XSECURE_SHA_START) {
		Status = XSecure_ShaStart(XSecureShaInstPtr, (XSecure_ShaMode)ShaParams.ShaMode);
		if (Status != XST_SUCCESS) {
			XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
			goto END;
		}
	}
	if ((ShaParams.OperationFlags & XSECURE_SHA_UPDATE) == XSECURE_SHA_UPDATE) {
		if (ShaParams.IsLast == TRUE) {
			Status = XSecure_ShaLastUpdate(XSecureShaInstPtr);
			if (Status != XST_SUCCESS) {
				XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
				goto END;
			}
		}
		Status = XSecure_ShaUpdate(XSecureShaInstPtr, ShaParams.DataAddr, ShaParams.DataSize);
		if (Status != XST_SUCCESS) {
			XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
			goto END;
		}
	}

	if ((ShaParams.OperationFlags & XSECURE_SHA_FINISH) == XSECURE_SHA_FINISH) {
		Status = XSecure_ShaFinish(XSecureShaInstPtr, ShaParams.HashAddr, ShaParams.HashBufSize);
	}

END:
	return Status;
}
/** @} */
