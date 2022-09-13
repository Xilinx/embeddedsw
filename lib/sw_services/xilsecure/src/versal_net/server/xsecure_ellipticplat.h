/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_ellipticplat.h
*
* This file contains the macros and types related to elliptic curve information
* which are specific to versal_net.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   dc   07/09/22 Initial version
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XSECURE_ELLIPTIC_PLAT_H_
#define XSECURE_ELLIPTIC_PLAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_ellipticcrvs.h"
#include "xsecure_trng.h"

/************************** Constant Definitions ****************************/

/************************** Type Definitions ********************************/
/* Ecc private key generation parameters */
typedef struct {
	u32 SeedAddr;		/**< Address to the seed buffer*/
	u32 SeedLength;		/**< Seed length */
	u32 PerStringAddr;	/**< Personalized string */
	u32 KeyOutPutAddr;	/**< Key output address */
}XSecure_ElliptcPrivateKeyGen;

/***************************** Function Prototypes ***************************/
int XSecure_EllipticPrvtKeyGenerate(XSecure_EllipticCrvTyp CrvType,
	XSecure_ElliptcPrivateKeyGen *PrivateKey);

#ifdef __cplusplus
}
#endif /* XSECURE_ELLIPTIC_PLAT_H */

#endif
