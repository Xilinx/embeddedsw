/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_kat.h
* This file contains versal specific code for KAT.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.3   kpt   03/12/24 Initial release
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
extern "c" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"

#ifndef PLM_RSA_EXCLUDE
#include "xil_types.h"

/************************** Constant Definitions *****************************/
int XSecure_RsaPrivateDecryptKat(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_PLAT_KAT_H_ */
/** @} */
