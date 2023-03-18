/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha_ipihandler.c
* @addtogroup xsecure_apis XilSecure Versal APIs
* @{
* @cond xsecure_internal
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
#include "xsecure_error.h"

/************************** Constant Definitions *****************************/
#define XSECURE_IPI_CONTINUE_MASK		(0x80000000U)
					/**< IPI Continue Mask */
#define XSECURE_IPI_FIRST_PACKET_MASK		(0x40000000U)
					/**< IPI First packet Mask */

/************************** Function Prototypes *****************************/

static int XSecure_ShaInitialize(void);
static int XSecure_ShaUpdate(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size,
	u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_ShaOperation(XPlmi_Cmd *Cmd);

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
int XSecure_Sha3IpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;

	if ((Cmd->CmdId & XSECURE_API_ID_MASK) ==
		XSECURE_API(XSECURE_API_SHA3_UPDATE)) {
		Status = XSecure_ShaOperation(Cmd);
	}
	else {
		Status = XST_INVALID_PARAM;
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
static int XSecure_ShaInitialize(void)
{
	int Status = XST_FAILURE;
	XSecure_Sha3 *XSecureSha3InstPtr = XSecure_GetSha3Instance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(0U);

	if (NULL == PmcDmaInstPtr) {
		goto END;
	}

	if (XPlmi_IsKatRan(XPLMI_SECURE_SHA3_KAT_MASK) != TRUE) {
		Status = XSECURE_ERR_KAT_NOT_EXECUTED;
		goto END;
	}

	Status = XSecure_Sha3Initialize(XSecureSha3InstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Start(XSecureSha3InstPtr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_Sha3Update64Bit or
 * 		XSecure_Sha3Finish based on the Continue bit in the command
 *
 * @param	SrcAddrLow	- Lower 32 bit address of the input data
 * 				on which hash has to be calculated
 * 		SrcAddrHigh	- Higher 32 bit address of the input data
 * 				on which hash has to be calculated
 * 		Size		- Size of the input data in bytes to be
 * 				updated
 * 		DstAddrLow	- Lower 32 bit address of the output data
 * 				where hash to be stored
 * 		DstAddrHigh	- Higher 32 bit address of the output data
 * 				where hash to be stored
 *
 * @return
 *	-	XST_SUCCESS - If the sha update/fnish is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_ShaUpdate(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size,
				u32 DstAddrLow, u32 DstAddrHigh)
{
	int Status = XST_FAILURE;
	u32 InputSize = Size;
	XSecure_Sha3 *XSecureSha3InstPtr = XSecure_GetSha3Instance();
	u64 DataAddr = ((u64)SrcAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)SrcAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << XSECURE_ADDR_HIGH_SHIFT) | (u64)DstAddrLow;
	XSecure_Sha3Hash Hash = {0U};

	if ((InputSize & XSECURE_IPI_FIRST_PACKET_MASK) != 0x0U) {
		Status = XSecure_ShaInitialize();
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
				(XSecure_Sha3Hash *)&Hash);
		if (XST_SUCCESS == Status) {
			Status = XPlmi_DmaXfr((u64)(UINTPTR)(Hash.Hash), DstAddr,
				XSECURE_HASH_SIZE_IN_BYTES, XPLMI_PMCDMA_0);
		}

	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function is used for single sha3digest or for multiple sha3 updates
 * 		based on the First packet bit and Continue bit in the command
 *
 * @param       Cmd is pointer to the command structure
 *
 * @return
 *      -       XST_SUCCESS - If the sha Update/Finish/Digest is successful
 *      -       ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_ShaOperation(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;
	u32 InputSize = Pload[2U];
	XSecure_Sha3 *XSecureSha3InstPtr = XSecure_GetSha3Instance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(0U);
	u64 DataAddr = ((u64)Pload[1U] << XSECURE_ADDR_HIGH_SHIFT) | (u64)Pload[0U];
	u64 DstAddr = ((u64)Pload[4U] << XSECURE_ADDR_HIGH_SHIFT) | (u64)Pload[3U];
	XSecure_Sha3Hash Hash = {0U};

	if (((InputSize & XSECURE_IPI_FIRST_PACKET_MASK) != 0U) &&
		((InputSize & XSECURE_IPI_CONTINUE_MASK) == 0U)){
		if (NULL == PmcDmaInstPtr) {
			goto END;
		}
		if (XPlmi_IsKatRan(XPLMI_SECURE_SHA3_KAT_MASK) != TRUE) {
			Status = XSECURE_ERR_KAT_NOT_EXECUTED;
			goto END;
		}
		Status = XSecure_Sha3Initialize(XSecureSha3InstPtr,
				PmcDmaInstPtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		InputSize = InputSize & (~XSECURE_IPI_CONTINUE_MASK) &
			(~XSECURE_IPI_FIRST_PACKET_MASK);
		Status = XSecure_Sha3Digest(XSecureSha3InstPtr, (UINTPTR)DataAddr, InputSize,
				(XSecure_Sha3Hash *)&Hash);
		if (XST_SUCCESS == Status) {
			Status = XPlmi_DmaXfr((u64)(UINTPTR)(Hash.Hash), DstAddr,
				XSECURE_HASH_SIZE_IN_BYTES, XPLMI_PMCDMA_0);
		}
	}
	else{
		Status = XSecure_ShaUpdate(Pload[0U], Pload[1U],
				Pload[2U], Pload[3U], Pload[4U]);
	}

END:
	return Status;
}
