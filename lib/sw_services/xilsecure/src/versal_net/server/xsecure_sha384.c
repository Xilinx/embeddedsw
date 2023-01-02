/******************************************************************************
* Copyright (c) 2022 - 2023, Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc.  All rights reserved.
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
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xsecure_error.h"
#include "xsecure_sha384.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef unsigned int SHA32;

/************************** Function Prototypes ******************************/
void sha_384(const unsigned char* in, const SHA32 size, unsigned char* out);

/*****************************************************************************/
/**
 * @brief	This function calculates the SHA2-384 digest on the given input data
 *
 * @param	Data		Pointer to buffer which stores input data
 * @param	Size		Size of the input data
 * @param	Hash		Pointer to buffer which stores calculated SHA2 hash
 *
 * @return
 *	-	XST_SUCCESS - If digest calculation done successfully
 *	-	XSECURE_SHA384_INVALID_PARAM - On invalid parameter
 *	-	XST_FAILURE - Error condition
 ******************************************************************************/
int XSecure_Sha384Digest(u8* Data, u32 Size, u8* Hash)
{
	volatile int Status = XST_FAILURE;

	/* Validate the input arguments */
	if ((Data == NULL) || (Hash == NULL)) {
		Status = (int)XSECURE_SHA384_INVALID_PARAM;
		goto END;
	}

	sha_384(Data, Size, Hash);

	Status = XST_SUCCESS;

END:
	return Status;
}
