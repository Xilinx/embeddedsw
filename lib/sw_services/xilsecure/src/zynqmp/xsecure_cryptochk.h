/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_cryptochk.h
*
* This file contains macros and functions common to AES, SHA and RSA
* for Zynqmp
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.6   kal     08/18/21 Initial Release
*
* </pre>
* @endcond
******************************************************************************/
#ifndef XSECURE_CRYPTOCHK_H
#define XSECURE_CRYPTOCHK_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions ****************************/

/***************************** Type Definitions******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XSECURE_CRYPTO_DISABLED_ERROR		(0x40U)

/*****************************************************************************/

/************************** Function Prototypes ******************************/
u32 XSecure_CryptoCheck(void);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_CRYPTOCHK_H_ */
/**@}*/
