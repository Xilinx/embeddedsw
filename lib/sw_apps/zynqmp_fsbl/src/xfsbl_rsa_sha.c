/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xfsbl_rsa_sha.c
 *
 * This contains code for the RSA and SHA functionality.
 * If the Hash type is SHA3 then CSU h/w will be used
 * else we will use SoftSHA256 s/w library for SHA2-256.
 * For RSA-4096 we will always use CSU h/w.
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  kc   07/22/14  Initial release
 * 2.0   bv   12/02/16  Made compliance to MISRAC 2012 guidelines
 * 3.0   vns  01/23/18  Added XFsbl_Sha3PadSelect() API to change SHA3 padding
 *                      to KECCAK SHA3 padding.
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xfsbl_authentication.h"
#ifdef XFSBL_SECURE

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XSecure_Sha3 SecureSha3;

/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_ShaDigest(const u8 *In, const u32 Size, u8 *Out, u32 HashLen)
{

	if(XFSBL_HASH_TYPE_SHA3 == HashLen)
	{
		(void)XSecure_Sha3Initialize(&SecureSha3, &CsuDma);
		XSecure_Sha3Digest(&SecureSha3, In, Size, Out);
	}
}

/*****************************************************************************
 *
 * This function selects the padding type to be used for SHA3 hash calculation
 *
 * @param	PadType	Padding type to be used for hash calculation.
 *
 * @return	XST_SUCCESS on successful selection.
 *              XST_FAILURE if selection is failed.
 *
 ******************************************************************************/
u32 XFsbl_Sha3PadSelect(XSecure_Sha3PadType PadType)
{
	u32 Status;

	Status = XSecure_Sha3PadSelection(&SecureSha3, PadType);

	return Status;
}

/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_ShaStart(void * Ctx, u32 HashLen)
{
	if(XFSBL_HASH_TYPE_SHA3 == HashLen)
	{
		(void)XSecure_Sha3Initialize(&SecureSha3, &CsuDma);

		XSecure_Sha3Start(&SecureSha3);
	}
}


/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_ShaUpdate(void * Ctx, u8 * Data, u32 Size, u32 HashLen)
{
	if(XFSBL_HASH_TYPE_SHA3 == HashLen)
	{
		XSecure_Sha3Update(&SecureSha3, Data, Size);
	}
}

/*****************************************************************************
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_ShaFinish(void * Ctx, u8 * Hash, u32 HashLen)
{

	if(XFSBL_HASH_TYPE_SHA3 == HashLen)
	{
		XSecure_Sha3Finish(&SecureSha3, Hash);
	}
}

#endif
