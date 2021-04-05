/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_shaclient.c
*
* This file contains the implementation of the client interface functions for
* SHA driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/21 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_defs.h"
#include "xsecure_shaclient.h"

/************************** Constant Definitions *****************************/
static u8 Initialize = FALSE;
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XSECURE_SHA_FIRST_PACKET_SHIFT		(30U)
#define XSECURE_SHA_UPDATE_CONTINUE_SHIFT	(31U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to initialize the SHA3 engine
 *
 * @return	- XST_SUCCESS - If the initialization is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_Sha3Initialize()
{
	Initialize = TRUE;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to update the SHA3 engine
 *		with the input data
 *
 * @param	InDataAddr	Address of the output buffer to store the
 * 				output hash
 * @param	Size		Size of the data to be updated to SHA3 engine
 *
 * @return	- XST_SUCCESS - If the update is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_Sha3Update(const u64 InDataAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;

	if (Initialize == FALSE) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "Sha3 Init not done\r\n");
		goto END;
	}

	Status = XSecure_ProcessIpiWithPayload5(XSECURE_API_SHA3_UPDATE,
			(u32)InDataAddr, (u32)(InDataAddr >> 32U),
			((1U << XSECURE_SHA_UPDATE_CONTINUE_SHIFT)|
			(1U << XSECURE_SHA_FIRST_PACKET_SHIFT) | Size),
			XSECURE_IPI_UNUSED_PARAM, XSECURE_IPI_UNUSED_PARAM);
	Initialize = FALSE;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request final data to SHA3 engine
 * 		which includes SHA3 padding and reads final hash
 *
 * @param	OutDataAddr	Address of the output buffer to store the
 * 				output hash
 *
 * @return	- XST_SUCCESS - If finished without any errors
 *		- XSECURE_SHA3_INVALID_PARAM - On invalid parameter
 *		- XSECURE_SHA3_STATE_MISMATCH_ERROR - If State mismatch is occurred
 *		- XST_FAILURE - If Sha3PadType is other than KECCAK or NIST
 *
 *****************************************************************************/
int XSecure_Sha3Finish(const u64 OutDataAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload5(XSECURE_API_SHA3_UPDATE,
			XSECURE_IPI_UNUSED_PARAM, XSECURE_IPI_UNUSED_PARAM,
			XSECURE_IPI_UNUSED_PARAM, (u32)OutDataAddr,
			(u32)(OutDataAddr >> 32));

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to execute
 * 		known answer test(KAT) on SHA crypto engine
 *
 * @return
 * 	- XST_SUCCESS - When KAT Pass
 * 	- XSECURE_SHA3_INVALID_PARAM 	 - On invalid argument
 * 	- XSECURE_SHA3_LAST_UPDATE_ERROR - Error when SHA3 last update fails
 * 	- XSECURE_SHA3_KAT_FAILED_ERROR	 - Error when SHA3 hash not matched with
 * 					   expected hash
 * 	- XSECURE_SHA3_PMC_DMA_UPDATE_ERROR - Error when DMA driver fails to update
 * 					      the data to SHA3
 * 	- XSECURE_SHA3_FINISH_ERROR 	 - Error when SHA3 finish fails
 *
 ******************************************************************************/
int XSecure_Sha3Kat()
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload0(XSECURE_API_SHA3_KAT);

	return Status;
}
