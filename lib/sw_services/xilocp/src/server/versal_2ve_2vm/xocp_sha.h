/***************************************************************************************************
* Copyright (C) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_sha.h
* @addtogroup xil_ocpapis APIs
* @{
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.6   tvp  09/01/25 Initial release
*       tvp  09/25/25 Use use sha_pmxc instead of using softsha2
*
* </pre>
*
***************************************************************************************************/
#ifndef XOCP_SHA_H
#define XOCP_SHA_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xsecure_sha.h"
#include "xsecure_init.h"

/*********************************** Constant Definitions *****************************************/

/************************************* Type Definitions *******************************************/

/***************************** Macros (Inline Functions) Definitions ******************************/

/************************************* Function Prototypes ****************************************/

/**************************************************************************************************/
/**
 * @brief	This function wrapper function to starts the SHA2-384 engine.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
static inline int XOcp_ShaStart(void)
{
	XSecure_Sha *ShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);

	return XSecure_ShaStart(ShaInstPtr, XSECURE_SHA2_384);
}

/**************************************************************************************************/
/**
 * @brief	This function wrapper function call XSecure_Sha384Update to
 * 		update the data in SHA2-384 engine.
 *
 * @param	InDataAddr	Address of the data which has to be updated to SHA engine.
 *
 * @param	Size		Size of the input data in bytes.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static inline int XOcp_ShaUpdate(u8 *InDataAddr, u32 Size)
{
	XSecure_Sha *ShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);

	return XSecure_ShaUpdate(ShaInstPtr, (u64)(UINTPTR)InDataAddr, Size);
}

/**************************************************************************************************/
/**
 * @brief	This function wrapper function reads the final SHA2-384 hash on complete data.
 *
 * @param	ResHash	Pointer to storage for output SHA2-384 Hash.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static inline int XOcp_ShaFinish(u8 *ResHash)
{
	XSecure_Sha *ShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);

	return XSecure_ShaFinish(ShaInstPtr, (u64)(UINTPTR)ResHash, XSECURE_SHA2_384_HASH_LEN);
}

/**************************************************************************************************/
/**
 * @brief	This wrapper function calls SHA2-384 digest on the given input data.
 *
 * @param	Data		Starting address of the data on which sha2-384 hash should be
 * 				calculated.
 *
 * @param	Size		Size of the input data.
 *
 * @param	Hash		Pointer to storage for output SHA2-384 Hash.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static inline int XOcp_ShaDigest(u8* Data, u32 Size, u8* Hash)
{
	XSecure_Sha *ShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);

	return XSecure_ShaDigest(ShaInstPtr, XSECURE_SHA2_384, (u64)(UINTPTR)Data, Size,
				 (u64)(UINTPTR)Hash, XSECURE_SHA2_384_HASH_LEN);
}

/*********************************** Variable Definitions *****************************************/

#ifdef __cplusplus
}
#endif
#endif /* XOCP_SHA_H */
