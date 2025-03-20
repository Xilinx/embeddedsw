/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha_ipihandler.c
*
* This file contains the XilSecure SHA3 IPI Handler definition.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kal   03/04/2021 Initial release
*       kpt   05/02/2021 Modified Sha3Kat function and added check to verify
*                        whether DMA is already initialized
*       bm    05/13/2021 Updated code to use common crypto instance
*       am    05/22/2021 Resolved MISRA C violation rule 17.8
* 4.6   har   07/14/2021 Fixed doxygen warnings
*       gm    07/16/2021 Added support for 64-bit address
* 5.0   kpt   07/24/2022 Moved XSecure_ShaKat into xsecure_kat_plat_ipihandler.c
* 5.1   yog   05/03/2023 Fixed MISRA C violation of Rule 10.3
* 		vss	  07/14/2023 Added support for IpiChannel check
* 5.2   ng    07/13/2023 Added SDT support
*	vss  09/11/2023 Fixed MISRA-C Rule 8.13 violation
*	vss  09/11/2023 Fixed MISRA-C Rule 10.3 and 10.4 violation
* 5.3	mb    02/23/2024 Updated HASH length to words in XPlmi_DmaXfr
*       kpt   03/22/2024 Fix MISRA C violation of Rule 10.3
*	ss    04/05/2024 Fixed doxygen warnings
* 5.4   yog   04/29/2024 Fixed doxygen grouping and doxygen warnings.
*       pre   03/02/2025 Implemented task based event notification functionality for SHA IPI events
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_sha_server_apis XilSecure SHA Server APIs
* @{
*/
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
#define XSECURE_IPI_CONTINUE_MASK		(0x80000000U)
					/**< IPI Continue Mask */


/************************** Function Prototypes *****************************/

static int XSecure_Sha3Init(void);
static int XSecure_Sha3UpdateIpi(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size,
	u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_ShaOperation(const XPlmi_Cmd *Cmd);

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID
 *
 * @param	Cmd	is pointer to the command structure
 *
 * @return
 *	-	XST_SUCCESS - If the handler execution is successful
 *	-	XST_INVALID_PARAM - If cmd is NULL or if invalid API ID is received.
 *	-	XST_DEVICE_BUSY - If IPI channel is busy
 *	-	XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_Sha3IpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	int SStatus = XST_FAILURE;
	XSecure_Sha3 *XSecureSha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** SHA IPI event handling */
	Status = XSecure_IpiEventHandling(Cmd, XPLMI_SHA3_CORE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Call the API handler according to API ID */
	if ((Cmd->CmdId & XSECURE_API_ID_MASK) ==
		XSECURE_API(XSECURE_API_SHA3_UPDATE)) {
		Status = XSecure_ShaOperation(Cmd);
	}
	else {
		Status = XST_INVALID_PARAM;
	}

END:
	if (XSecureSha3InstPtr->ShaState == XSECURE_SHA_INITIALIZED) {
		SStatus = XSecure_MakeResFree(XPLMI_SHA3_CORE);
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes SHA3 instance.
 *
 * @return
 *	-	XST_SUCCESS - If the initialization is successful
 *	-	XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
static int XSecure_Sha3Init(void)
{
	int Status = XST_FAILURE;
	XSecure_Sha3 *XSecureSha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);

	if (NULL == PmcDmaInstPtr) {
		goto END;
	}

	Status = XSecure_Sha3Start(XSecureSha3InstPtr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handler calls XSecure_Sha3Update64Bit or
 * 		XSecure_Sha3Finish based on the Continue bit in the command
 *
 * @param	SrcAddrLow	Lower 32 bit address of the input data
 * 				on which hash has to be calculated
 * @param	SrcAddrHigh	Higher 32 bit address of the input data
 * 				on which hash has to be calculated
 * @param	Size		Size of the input data in bytes to be
 * 				updated
 * @param	DstAddrLow	Lower 32 bit address of the output data
 * 				where hash to be stored
 * @param	DstAddrHigh	Higher 32 bit address of the output data
 * 				where hash to be stored
 *
 * @return
 *	-	XST_SUCCESS - If the sha update/fnish is successful
 *	-	XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
static int XSecure_Sha3UpdateIpi(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size,
				u32 DstAddrLow, u32 DstAddrHigh)
{
	int Status = XST_FAILURE;
	u32 InputSize = Size;
	XSecure_Sha3 *XSecureSha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);;
	u64 DataAddr = ((u64)SrcAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)SrcAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)DstAddrLow;
	XSecure_Sha3Hash Hash = {0U};

	if ((InputSize & XSECURE_IPI_FIRST_PACKET_MASK) != 0x0U) {
		/* Initializes and starts the sha3 engine */
		Status = XSecure_Sha3Init();
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if ((InputSize & XSECURE_IPI_CONTINUE_MASK) != 0x0U) {
		InputSize = InputSize & (~XSECURE_IPI_CONTINUE_MASK) &
			(~XSECURE_IPI_FIRST_PACKET_MASK);
		Status = XSecure_Sha3Update64Bit(XSecureSha3InstPtr,
				DataAddr, InputSize);
	}
	else {
		Status = XSecure_Sha3Finish(XSecureSha3InstPtr,
				&Hash);
		if (XST_SUCCESS == Status) {
			/* copy hash to provided destination address using DMA */
			Status = XPlmi_DmaXfr((u64)(UINTPTR)(Hash.Hash), DstAddr,
				XSECURE_SHA3_HASH_LENGTH_IN_WORDS, XPLMI_PMCDMA_0);
		}

	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used for single sha3digest or for multiple sha3 updates
 * 		based on the First packet bit and Continue bit in the command
 *
 * @param	Cmd	is pointer to the command structure
 *
 * @return
 *		 - XST_SUCCESS  If the sha Update/Finish/Digest is successful
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
static int XSecure_ShaOperation(const XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	const u32 *Pload = Cmd->Payload;
	u32 InputSize = Pload[2U];
	XSecure_Sha3 *XSecureSha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);;
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);
	u64 DataAddr = ((u64)Pload[1U] << XSECURE_ADDR_HIGH_SHIFT) | (u64)Pload[0U];
	u64 DstAddr = ((u64)Pload[4U] << XSECURE_ADDR_HIGH_SHIFT) | (u64)Pload[3U];
	XSecure_Sha3Hash Hash = {0U};

	if (((InputSize & XSECURE_IPI_FIRST_PACKET_MASK) != 0U) &&
		((InputSize & XSECURE_IPI_CONTINUE_MASK) == 0U)){
		if (NULL == PmcDmaInstPtr) {
			goto END;
		}

		InputSize = InputSize & (~XSECURE_IPI_CONTINUE_MASK) &
			(~XSECURE_IPI_FIRST_PACKET_MASK);
		/** Calculate the SHA-3 digest on the given input data */
		Status = XSecure_Sha3Digest(XSecureSha3InstPtr, (UINTPTR)DataAddr, InputSize,
				(XSecure_Sha3Hash *)&Hash);
		if (XST_SUCCESS == Status) {
			/* Initiate and complete the DMA to DMA transfer */
			Status = XPlmi_DmaXfr((u64)(UINTPTR)(Hash.Hash), DstAddr,
				XSECURE_SHA3_HASH_LENGTH_IN_WORDS, XPLMI_PMCDMA_0);
		}
	}
	else{
		Status = XSecure_Sha3UpdateIpi(Pload[0U], Pload[1U],
				Pload[2U], Pload[3U], Pload[4U]);
	}

END:
	return Status;
}
/** @} */
