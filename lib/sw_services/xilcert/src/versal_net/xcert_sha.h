/***************************************************************************************************
* Copyright (C) 2025 - 2026, Advanced Micro Devices, Inc.  All rights reserved.
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
* 1.4   tvp  09/01/25 Initial release
* 1.6   sri  08/24/26 Keep SHA-1 for SKI/AKI on Versal Net (backward compatibility)
*
* </pre>
*
***************************************************************************************************/
#ifndef XCERT_SHA_H
#define XCERT_SHA_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************** Include Files *********************************************/
#include "xsecure_sha384.h"
#include "xsecure_plat_kat.h"
#include "xsecure_sha1.h"
/** SKI/AKI hash: SHA-1 on Versal Net for backward compatibility with existing certs */
#define XCERT_SHA_HASH_LEN                      XSECURE_SHA1_HASH_SIZE
/** Byte offset within SHA digest used for SKI/AKI key identifier */
#define XCERT_SHA_KEY_ID_OFFSET                 (0U)
#include "xcert_genx509cert.h"
#include "xplmi.h"
#include "xplmi_tamper.h"

/********************************** Constant Definitions ******************************************/

/************************************ Type Definitions ********************************************/

/**************************** Macros (Inline Functions) Definitions *******************************/

#define XOCP_SECURE_SHA_KAT_MASK	XPLMI_SECURE_SHA384_KAT_MASK /**< SHA384 kat mask */

/************************************ Function Prototypes *****************************************/

/**************************************************************************************************/
/**
 * @brief	This function wrapper function to starts the SHA2-384 engine.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 **************************************************************************************************/
static inline int XCert_ShaStart(void)
{
	XSecure_Sha384Start();

	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
 * @brief	This function wrapper function call XSecure_Sha384Update to update the data in
 * 		SHA2-384 engine.
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
static inline int XCert_ShaUpdate(u8 *InDataAddr, u32 Size)
{
	return XSecure_Sha384Update((u8 *)(UINTPTR)InDataAddr, Size);
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
static inline int XCert_ShaFinish(u8 *ResHash)
{
	return XSecure_Sha384Finish((XSecure_Sha2Hash *)(UINTPTR)ResHash);
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
static inline int XCert_ShaDigest(u8* Data, u32 Size, u8* Hash)
{
	return XSecure_Sha384Digest((u8 *)(UINTPTR)Data, Size, (u8 *)(UINTPTR)Hash);
}

/**************************************************************************************************/
/**
 * @brief	This wrapper calls SHA-1 digest for SKI/AKI on Versal Net.
 *
 * @param	Data	Starting address of input data.
 * @param	Size	Size of input data in bytes.
 * @param	Hash	Pointer to storage for output SHA-1 hash.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 **************************************************************************************************/
static inline int XCert_Sha1Digest(u8 *Data, u32 Size, u8 *Hash)
{
	return XSecure_Sha1Digest(Data, Size, Hash);
}

/**************************************************************************************************/
/**
 * @brief	This wrapper function perform KAT on SHA2-384 and set plm kat mask.
 *
 * @return
 *		 - XST_SUCCESS on success.
 *		 - XSECURE_SHA384_KAT_ERROR if SHA384 KAT fails.
 *		 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
static inline int XCert_ShaKat(void)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;

	XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XCERT_ERR_X509_KAT_FAILED, Status, StatusTmp,
					   XSecure_Sha384Kat);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		goto END;
	}

	XPlmi_SetKatMask(XPLMI_SECURE_SHA384_KAT_MASK);

END:
	return Status;
}

/*********************************** Variable Definitions *****************************************/

#ifdef __cplusplus
}
#endif
#endif /* XCERT_SHA_H */
/** @} */
