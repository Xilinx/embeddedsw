/***************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xcert_sha.h
* @addtogroup xil_certapis APIs
* @{
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   sri  08/24/26 Initial release. HW SHA3-384 wrappers for Versal_2VP_P
*
* </pre>
*
***************************************************************************************************/
#ifndef XCERT_SHA_H
#define XCERT_SHA_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xsecure_sha.h"
#include "xsecure_sha_common.h"
#include "xsecure_init.h"
#include "xsecure_kat.h"
#include "xcert_genx509cert.h"
#include "xplmi.h"
#include "xplmi_tamper.h"

/*********************************** Constant Definitions *****************************************/

#define XOCP_SECURE_SHA_KAT_MASK	XPLMI_SECURE_SHA3_KAT_MASK /**< SHA3 KAT mask */

/************************************* Function Prototypes ****************************************/

/**************************************************************************************************/
/**
 * @brief	This function wrapper function to starts the SHA-3 engine.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- Error code on failure.
 *
 **************************************************************************************************/
static inline int XCert_ShaStart(void)
{
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	return XSecure_ShaStart(ShaInstPtr, XSECURE_SHA3_384);
}

/**************************************************************************************************/
/**
 * @brief	This function wrapper function call XSecure_ShaUpdate to update the data in
 *		SHA3 engine.
 *
 * @param	InDataAddr	Address of the data which has to be updated to SHA engine.
 * @param	Size		Size of the input data in bytes.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- Error code on failure.
 *
 **************************************************************************************************/
static inline int XCert_ShaUpdate(u8 *InDataAddr, u32 Size)
{
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	return XSecure_ShaUpdate(ShaInstPtr, (u64)(UINTPTR)InDataAddr, Size);
}

/**************************************************************************************************/
/**
 * @brief	This function wrapper function reads the final SHA3 hash on complete data.
 *
 * @param	ResHash	Pointer to storage for output SHA3 Hash.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- Error code on failure.
 *
 **************************************************************************************************/
static inline int XCert_ShaFinish(u8 *ResHash)
{
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	return XSecure_ShaFinish(ShaInstPtr, (u64)(UINTPTR)ResHash,
				 XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
}

/**************************************************************************************************/
/**
 * @brief	This wrapper function calls SHA3 digest on the given input data.
 *
 * @param	Data		Starting address of the data on which SHA3 hash should be
 *				calculated.
 * @param	Size		Size of the input data.
 * @param	Hash		Pointer to storage for output SHA3 Hash.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- Error code on failure.
 *
 **************************************************************************************************/
static inline int XCert_ShaDigest(u8* Data, u32 Size, u8* Hash)
{
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	return XSecure_ShaDigest(ShaInstPtr, XSECURE_SHA3_384, (u64)(UINTPTR)Data, Size,
				 (u64)(UINTPTR)Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
}

/**************************************************************************************************/
/**
 * @brief	This wrapper function perform KAT on SHA3-384 and set plm kat mask.
 *
 * @return
 *		 - XST_SUCCESS on success.
 *		 - XCERT_ERR_X509_KAT_FAILED if SHA3 hash not matched with expected hash.
 *		 - XST_FAILURE if any error from ShaUpdate or ShaFinish.
 *
 **************************************************************************************************/
static inline int XCert_ShaKat(void)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XCERT_ERR_X509_KAT_FAILED, Status, StatusTmp,
					   XSecure_Sha3Kat, ShaInstPtr);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		goto END;
	}

	XPlmi_SetKatMask(XPLMI_SECURE_SHA3_KAT_MASK);

END:
	return Status;
}

#ifdef __cplusplus
}
#endif
#endif /* XCERT_SHA_H */
/** @} */
