/******************************************************************************
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xfsbl_rsa_sha.c
 *
 * This contains code for the RSA and SHA3 functionality.
 * For SHA3 CSU h/w will be used.
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
 * 4.0   har  06/17/20  Removed references to unused algorithms
 * 5.0   bsv  03/11/21  Fixed build issues
 *       kpt  03/16/21  Updated function headers with appropriate description
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xfsbl_authentication.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XSecure_Sha3 SecureSha3;

/*****************************************************************************
 * This function calculates SHA3 hash for the given input data.
 *
 * @param       In      Pointer to the input data provided for hashing
 * @param       Size    Size of the input data provided for hashing
 * @param       Out     Pointer to the location where the resulting hash will
 *                      be written
 * @param       HashLen Length of the hash that is used to determine the sha3
 *                      hashing
 *
 * @return      None
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
 * This function updates SHA3 engine with final data which includes SHA3
 * padding and reads final hash on complete data.
 *
 * @param       Ctx     Pointer to a callback function
 * @param       Hash    Pointer to the location where the resulting
 *                      hash will be written
 * @param       HashLen Length of the hash that is used to determine sha3
 *                      hashing
 *
 * @return      None
 *
 ******************************************************************************/
void XFsbl_ShaFinish(void * Ctx, u8 * Hash, u32 HashLen)
{

	if(XFSBL_HASH_TYPE_SHA3 == HashLen)
	{
		XSecure_Sha3Finish(&SecureSha3, Hash);
	}
}
/*****************************************************************************
 * This function starts the SHA3 engine.
 *
 * @param       Ctx     Pointer to a callback function
 * @param       HashLen Length of the hash that is used to determine sha3
 *                      hashing
 *
 * @return      None
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
 * This function updates the input data to SHA3 engine.
 *
 * @param       Ctx      Pointer to a callback function
 * @param       Data     Pointer to the input data that is used for
 *                       hash calculation
 * @param       Size     Size of the input data that is provided
 *                       for hash calculation
 * @param       HashLen  Length of the hash that is used to determine sha3
 *                       hashing
 *
 * @return      None
 *
 ******************************************************************************/
void XFsbl_ShaUpdate(void * Ctx, u8 * Data, u32 Size, u32 HashLen)
{
	if(XFSBL_HASH_TYPE_SHA3 == HashLen)
	{
		XSecure_Sha3Update(&SecureSha3, Data, Size);
	}
}

#ifdef XFSBL_SECURE
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
#endif
