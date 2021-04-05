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
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_defs.h"
#include "xsecure_sha.h"
#include "xsecure_sha_ipihandler.h"

/************************** Constant Definitions *****************************/
static XSecure_Sha3 Secure_Sha3;
static XCsuDma CsuDma;
static XCsuDma_Config *Config;

#define XSECURE_IPI_CONTINUE_MASK		(0x80000000U)
#define XSECURE_IPI_FIRST_PACKET_MASK		(0x40000000U)

/************************** Function Prototypes *****************************/

static int XSecure_ShaInitialize(void);
static int XSecure_ShaUpdate(u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size,
	u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_ShaKat();

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function calls respective IPI handler based on the API_ID
 *
 * @return	- XST_SUCCESS - If the handler execution is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_Sha3IpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	if ((Cmd->CmdId & 0xFFU) == XSECURE_API(XSECURE_API_SHA3_UPDATE)) {
		Status = XSecure_ShaUpdate(Pload[0], Pload[1],
				Pload[2], Pload[3], Pload[4]);
	}
	else if ((Cmd->CmdId & 0xFFU) == XSECURE_API(XSECURE_API_SHA3_KAT)) {
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

	Config = XCsuDma_LookupConfig(0);
	if (NULL == Config) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "config failed\n\r");
		goto END;
	}

	Status = XCsuDma_CfgInitialize(&CsuDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Initialize(&Secure_Sha3, &CsuDma);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Start(&Secure_Sha3);

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
	u64 DataAddr = ((u64)SrcAddrHigh << 32) | (u64)SrcAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << 32) | (u64)DstAddrLow;

	if ((Size & XSECURE_IPI_FIRST_PACKET_MASK) != 0x0U) {
		Status = XSecure_ShaInitialize();
	}

	if ((Size & XSECURE_IPI_CONTINUE_MASK) != 0x0U) {
		Size = Size & (~XSECURE_IPI_CONTINUE_MASK) &
			(~XSECURE_IPI_FIRST_PACKET_MASK);
		Status = XSecure_Sha3Update64Bit(&Secure_Sha3, DataAddr, Size);
	}
	else {
		if (DstAddrHigh != 0x0U) {
			XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"64 bit address not supported for DstAddr\n\r");
			Status = XST_INVALID_PARAM;
		}
		else {
			Status = XSecure_Sha3Finish(&Secure_Sha3,
					(XSecure_Sha3Hash *)(UINTPTR)DstAddr);
		}
	}

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
static int XSecure_ShaKat()
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ShaInitialize();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Kat(&Secure_Sha3);

END:
	return Status;
}
