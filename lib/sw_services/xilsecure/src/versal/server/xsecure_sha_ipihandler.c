/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
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

/************************** Constant Definitions *****************************/
#define XSECURE_IPI_CONTINUE_MASK		(0x80000000U)
					/**< IPI Continue Mask */
#define XSECURE_IPI_FIRST_PACKET_MASK		(0x40000000U)
					/**< IPI First packet Mask */

/************************** Function Prototypes *****************************/

static int XSecure_ShaInitialize(void);
static int XSecure_ShaUpdate(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size,
	u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_ShaKat(void);

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return	- XST_SUCCESS - If the handler execution is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_Sha3IpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	if ((Cmd->CmdId & XSECURE_API_ID_MASK) ==
		XSECURE_API(XSECURE_API_SHA3_UPDATE)) {
		Status = XSecure_ShaUpdate(Pload[0], Pload[1],
				Pload[2], Pload[3], Pload[4]);
	}
	else if ((Cmd->CmdId & XSECURE_API_ID_MASK) ==
		XSECURE_API(XSECURE_API_SHA3_KAT)) {
		Status = XSecure_ShaKat();
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
 * @return	- XST_SUCCESS - If the initialization is successful
 * 		- ErrorCode - If there is a failure
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
 * @return	- XST_SUCCESS - If the sha update/fnish is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_ShaUpdate(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size,
				u32 DstAddrLow, u32 DstAddrHigh)
{
	int Status = XST_FAILURE;
	u32 InputSize = Size;
	XSecure_Sha3 *XSecureSha3InstPtr = XSecure_GetSha3Instance();
	u64 DataAddr = ((u64)SrcAddrHigh << 32) | (u64)SrcAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << 32) | (u64)DstAddrLow;
	XSecure_Sha3Hash Hash = {0U};
	u32 Index = 0U;

	if ((InputSize & XSECURE_IPI_FIRST_PACKET_MASK) != 0x0U) {
		Status = XSecure_ShaInitialize();
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if ((InputSize & XSECURE_IPI_CONTINUE_MASK) != 0x0U) {
		InputSize = InputSize & (~XSECURE_IPI_CONTINUE_MASK) &
			(~XSECURE_IPI_FIRST_PACKET_MASK);
		Status = XSecure_Sha3Update64Bit(XSecureSha3InstPtr, DataAddr, InputSize);
	}
	else {
		Status = XSecure_Sha3Finish(XSecureSha3InstPtr,
				(XSecure_Sha3Hash *)&Hash);
		if (XST_SUCCESS == Status) {
			for (Index = 0U; Index < XSECURE_HASH_SIZE_IN_BYTES; Index++) {
				XPlmi_OutByte64((DstAddr + Index),
						Hash.Hash[Index]);
			}
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_ShaKat server API
 *
 * @return	- XST_SUCCESS - If the sha update/fnish is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_ShaKat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_Sha3 *XSecureSha3InstPtr = XSecure_GetSha3Instance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(0U);

	if (NULL == PmcDmaInstPtr) {
		goto END;
	}

	if (XSecureSha3InstPtr->Sha3State == XSECURE_SHA3_ENGINE_STARTED) {
		Status = (int)XSECURE_SHA3_KAT_BUSY;
		goto END;
	}

	Status = XSecure_Sha3Initialize(XSecureSha3InstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_Sha3Kat(XSecureSha3InstPtr);

END:
	return Status;
}