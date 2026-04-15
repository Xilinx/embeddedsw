/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/slhdsa/xsecure_slhdsa_xmss.h
*
* This file contains structures, constants and declarations used in SLH-DSA XMSS and provides
* interface to SLH-DSA XMSS operations
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  02/19/26 Initial release
*
* </pre>
*
***************************************************************************************************/

/**
 * @addtogroup xsecure_slhdsa_server_apis XilSecure SLHDSA Server APIs
 * @{
 */
#ifndef XSECURE_SLH_DSA_XMSS_H_
#define XSECURE_SLH_DSA_XMSS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** Include Files *******************************************/
#include "xsecure_slhdsa_instance.h"
#include "xil_types.h"

/**************************************** Function Prototypes *************************************/
int XSecure_SlhdsaXmssPkFromSign(u64 IdxLeaf, u64 SignTmpAddr, u64 MAddr,
				 u64 PublicKeyAddr, u8* Node);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SLH_DSA_XMSS_H_ */
/** @} */
