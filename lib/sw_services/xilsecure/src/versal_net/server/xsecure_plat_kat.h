/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat_kat.h
* @addtogroup xsecure_kat_apis XilSecure KAT APIs
* @{
* @cond xsecure_internal
*
* This file contains KAT interface APIs for versal net
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 5.0   kpt  07/15/2022 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XSECURE_KAT_PLAT_H_
#define XSECURE_KAT_PLAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xsecure_kat.h"
#include "xsecure_trng.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int XSecure_HmacKat(XSecure_Sha3 *SecureSha3);
int XSecure_TrngDRBGKat(XSecure_TrngInstance *InstancePtr);
int XSecure_TrngPreOperationalSelfTests(XSecure_TrngInstance *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_KAT_PLAT_H_ *//* @} */
