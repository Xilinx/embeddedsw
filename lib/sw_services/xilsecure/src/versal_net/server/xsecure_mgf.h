/******************************************************************************
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_mgf.h
* @addtogroup xsecure_mgf_apis XilSecure MGF APIs
* @{
* @cond xsecure_internal
* This file contains function declarations and enums related to mask generate function
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.2   kpt  06/28/23 Initial release
*	vss  09/21/23 Fixed doxygen warnings
*	ss   04/05/2024 Fixed doxygen warnings
*
* </pre>
*
* @endcond
******************************************************************************/

#ifndef XSECURE_MGF_H_
#define XSECURE_MGF_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"

/************************** Constant Definitions ****************************/

#define XSECURE_SHA3_HASH_LENGTH_IN_BYTES  (48U) /**< SHA3 Hash length in bytes */

/***************************** Type Definitions ******************************/

typedef enum {
	XSECURE_SHA3_384 = 0, /**< SHA3 384 */
	XSECURE_SHA384, /**< SHA3 384 */
} XSecure_ShaType;

typedef struct {
	XSecure_ShaType Shatype; /**< SHA type for MGF */
	u32 HashLen;             /**< Hash length for MGF */
	int (*ShaStart)(XSecure_ShaType Shatype, void *InstancePtr); /**< Function pointer to SHA start */
	int (*ShaUpdate)(XSecure_ShaType Shatype, void *InstancePtr, u64 InputDataAddr, u32 Size); /**< Function pointer to SHA update */
	int (*ShaFinish)(XSecure_ShaType Shatype, void *InstancePtr, u64 HashAddr); /**< Function pointer to SHA finish */
	int (*ShaDigest)(XSecure_ShaType Shatype, void *InstancePtr, u64 InputDataAddr, u32 Size, u64 HashAddr); /**< Function pointer to SHA digest */
} XSecure_HashAlgInfo;

typedef struct {
	u8 *Seed;      /**< Input seed on which mask should be generated */
	u8 *Output;    /**< Buffer to store the mask */
	u32 SeedLen;   /**< Seed length */
	u32 OutputLen; /**< Buffer length */
} XSecure_MgfInput;

/***************************** Function Prototypes ***************************/

/** Mask Generation function */
int XSecure_MaskGenFunc(XSecure_ShaType ShaType,
			void *InstancePtr, XSecure_MgfInput *MgfInput);
/** Get Hash Instance function */
XSecure_HashAlgInfo *XSecure_GetHashInstance(XSecure_ShaType Shatype);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_MGF_H_ */
/* @} */
