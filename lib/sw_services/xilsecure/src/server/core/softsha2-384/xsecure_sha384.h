/******************************************************************************
* Copyright (c) 2022 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha384.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   har  01/02/23  Initial release
* 5.4   yog  04/29/24  Fixed doxygen grouping and doxygen warnings.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_soft_sha384_server_apis XilSecure Soft SHA384 Server APIs
* @{
*/
#ifndef XSECURE_SHA384_H
#define XSECURE_SHA384_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @cond xsecure_internal
 * @{
 */
/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions ****************************/

#define XSECURE_HASH_SIZE_IN_BYTES		(48U)

/**************************** Type Definitions *******************************/
/** Stores the resultant Hash. */
typedef struct {
	u8 Hash[XSECURE_HASH_SIZE_IN_BYTES];
} XSecure_Sha2Hash;

/************************** Function Prototypes ******************************/
int XSecure_Sha384Digest(u8* Data, u32 Size, u8* Sha384Hash);
void XSecure_Sha384Start(void);
int XSecure_Sha384Update(u8* Data, u32 Size);
int XSecure_Sha384Finish(XSecure_Sha2Hash *ResHash);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SHA384_H */
/** @} */
