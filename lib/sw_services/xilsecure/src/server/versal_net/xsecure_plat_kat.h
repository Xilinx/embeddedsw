/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat_kat.h
*
* This file contains KAT interface APIs for Versal Net
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 5.0   kpt  07/15/2022 Initial release
* 5.2   kpt  07/12/2023 Add pairwise consistency for RSA
* 5.3   kpt  12/13/2023 Added RSA CRT support for PWCT
* 5.4   yog  04/29/2024 Fixed doxygen grouping.
*       kpt  06/13/2024 Add support for RSA key generation.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_kat_server_apis Xilsecure KAT Server APIs
* @{
*/
#ifndef XSECURE_PLAT_KAT_H_
#define XSECURE_PLAT_KAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_kat.h"
#include "xsecure_plat_rsa.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
/**
 * @cond xsecure_internal
 * @{
 */
int XSecure_HmacKat(XSecure_Sha3 *SecureSha3);
/**
 * @}
 * @endcond
 */
int XSecure_Sha384Kat(void);
#ifndef PLM_RSA_EXCLUDE
int XSecure_RsaPwct(XSecure_RsaPrivKey *PrivKey, XSecure_RsaPubKey *PubKey, void *ShaInstancePtr, XSecure_ShaMode Shatype);
int XSecure_RsaPrivateDecryptKat(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_PLAT_KAT_H_ */
/** @} */
