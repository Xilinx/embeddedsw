/******************************************************************************
* Copyright (c) 2022 - 2023, Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha384.c
*
* This file contains the implementation of the interface functions for SHA2-384
* driver.
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   har  01/02/23 Initial release
*       kal  19/05/23 Added Sha2 Start, Update and Finish APIs support
*       dd   10/11/23 MISRA-C violation Rule 1.1 fixed
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_soft_sha384_server_apis XilSecure Soft SHA384 Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xsecure_error.h"
#include "xsecure_sha384.h"
#include "SoftSHA.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
sha384_context ShaCtx;

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
 * @brief	This function calculates the SHA2-384 digest on the given input data
 *
 * @param	Data	Pointer to buffer which stores input data
 * @param	Size	Size of the input data
 * @param	Hash	Pointer to buffer which stores calculated SHA2 hash
 *
 * @return
 *		 - XST_SUCCESS  If digest calculation done successfully
 *		 - XSECURE_SHA384_INVALID_PARAM  On invalid parameter
 *
 ******************************************************************************/
int XSecure_Sha384Digest(u8* Data, u32 Size, u8* Hash)
{
	volatile int Status = XST_FAILURE;

	/* Validate the input arguments */
	if (Hash == NULL) {
		Status = (int)XSECURE_SHA384_INVALID_PARAM;
		goto END;
	}

	sha_384(Data, Size, Hash);

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function starts the SHA384 engine
 *
 ******************************************************************************/
void XSecure_Sha384Start(void)
{
	sha384_starts(&ShaCtx);
}

/*****************************************************************************/
/**
 * @brief	This function updates the data to SHA2-384 update API in driver
 *
 * @param	Data	Pointer to buffer which stores input data
 * @param	Size	Size of the input data
 *
 * @return
 *		 - XST_SUCCESS  If update done successfully
 *		 - XSECURE_SHA384_INVALID_PARAM  On invalid parameter
 *
 ******************************************************************************/
int XSecure_Sha384Update(u8* Data, u32 Size)
{
	volatile int Status = XST_FAILURE;

	/* Validate the input arguments */
	if (Data == NULL) {
		Status = (int)XSECURE_SHA384_INVALID_PARAM;
		goto END;
	}

	sha384_update(&ShaCtx, Data, Size);

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function calculates the SHA2-384 hash on the given input data
 *
 * @param	ResHash		Pointer to XSecure_Sha2Hash structure
 *
 * @return
 *		 - XST_SUCCESS  If digest calculation done successfully
 *		 - XSECURE_SHA384_INVALID_PARAM  On invalid parameter
 *
 ******************************************************************************/
int XSecure_Sha384Finish(XSecure_Sha2Hash *ResHash)
{
	volatile int Status = XST_FAILURE;

	/* Validate the input arguments */
	if (ResHash == NULL) {
		Status = (int)XSECURE_SHA384_INVALID_PARAM;
		goto END;
	}

	sha384_finish(&ShaCtx, ResHash->Hash);

	Status = XST_SUCCESS;

END:
	return Status;
}
/** @} */
