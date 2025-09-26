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
#include "xsecure_sha_common.h"
#include "xsecure_init.h"

/*********************************** Constant Definitions *****************************************/

/************************************* Type Definitions *******************************************/

/***************************** Macros (Inline Functions) Definitions ******************************/

/************************************* Function Prototypes ****************************************/

/**************************************************************************************************/
/**
 * @brief	This function wrapper function to starts the SHA-3 engine.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static inline int XOcp_ShaStart(void)
{
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	return XSecure_Sha3Start(ShaInstPtr);
}

/**************************************************************************************************/
/**
 * @brief	This function wrapper function call XSecure_Sha3Update to update the data in SHA3
 * 		engine.
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
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	return XSecure_Sha3Update(ShaInstPtr, (UINTPTR)InDataAddr, Size);
}

/**************************************************************************************************/
/**
 * @brief	This function wrapper function reads the final SHA3 hash on complete data.
 *
 * @param	ResHash	Pointer to storage for output SHA3 Hash.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static inline int XOcp_ShaFinish(u8 *ResHash)
{
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	return XSecure_Sha3Finish(ShaInstPtr, (XSecure_Sha3Hash *)(UINTPTR)ResHash);
}

/**************************************************************************************************/
/**
 * @brief	This wrapper function calls SHA3 digest on the given input data.
 *
 * @param	Data		Starting address of the data on which sha3 hash should be calculated.
 *
 * @param	Size		Size of the input data.
 *
 * @param	Hash		Pointer to storage for output SHA3 Hash.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static inline int XOcp_ShaDigest(u8* Data, u32 Size, u8* Hash)
{
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	return XSecure_Sha3Digest(ShaInstPtr, (UINTPTR)Data, Size,
				  (XSecure_Sha3Hash *)(UINTPTR)Hash);
}

/*********************************** Variable Definitions *****************************************/

#ifdef __cplusplus
}
#endif
#endif /* XOCP_SHA_H */
