/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
 *
 * @file xsecure_sha1.h
 *
 * This file contains SHA1 driver function prototype.
 *
 * NOTE :
 *	This algorithm is tested only for little endian with data length ranging 1 byte to 8KB
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   mmd  12/20/24 Initial release
 *
 * </pre>
 *
 **************************************************************************************************/
#ifndef XSECURE_SHA1_H
#define XSECURE_SHA1_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ******************************************************/
#include "xil_types.h"

/************************** Constant Definitions **************************************************/
#define XSECURE_SHA1_HASH_SIZE			20U		/**< Size of SHA1 hash in bytes */

/***************************** Function Prototypes ************************************************/
s32 XSecure_Sha1Digest(const u8 * const Data, u32 Len, u8 * const Hash);

#ifdef __cplusplus
}
#endif

#endif	/* XSECURE_SHA1_H */
