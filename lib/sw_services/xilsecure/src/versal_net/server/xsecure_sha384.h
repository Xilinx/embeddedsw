/******************************************************************************
* Copyright (c) 2022 - 2023, Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha384.h
* @cond xsecure_internal
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   har  01/02/23  Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XSECURE_SHA384_H
#define XSECURE_SHA384_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
/** @cond xsecure_internal
 * @{
 */

/************************** Function Prototypes ******************************/
int XSecure_Sha384Digest(u8* Data, u32 Size, u8* Sha384Hash);

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_SHA384_H */

/* @} */
