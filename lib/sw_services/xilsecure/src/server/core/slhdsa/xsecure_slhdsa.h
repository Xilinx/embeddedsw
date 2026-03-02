/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_slhdsa.h
*
* This file contains constants and function declarations used in software based SLHDSA operations
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.7   tvp  02/19/26 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_slhdsa_server_apis XilSecure SLHDSA Server APIs
* @{
*/
#ifndef XSECURE_SLH_DSA_H_
#define XSECURE_SLH_DSA_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** Include Files *******************************************/
#include "xsecure_slhdsa_instance.h"
#include "xsecure_defs.h"
#include "xil_types.h"

/***************************************** Constant Definitions ***********************************/
#define XSECURE_SLH_DSA_CTX_MAX_LEN_BYTES	(255U)	/**< Maximum possible context length */
#define XSECURE_SLH_DSA_DS_LEN_BYTES		(1U)	/**< Domain separator length in bytes */
#define XSECURE_SLH_DSA_CTX_LEN_FIELD_LEN_BYTES	(1U)	/**< Length of ctx length field */

#define XSECURE_SLH_DSA_PURE_SLH_VERIFY_DS	(0x0U)	/**< Domain separator for pure SLH-DSA
							  verification */

/***************************************** Type Definitions ***************************************/

/*************************************** Function Prototypes **************************************/
int XSecure_SlhdsaVerify(XSecure_Sha *ShaInstPtr,
			 const XSecure_SlhdsaInputParams * const SlhdsaParams);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SLH_DSA_H_ */
/** @} */
