/***************************************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_trng.h
*
* This header file contains function declarations to use trngpsx.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.0   kpt  05/05/22 Initial release
*       dc   07/12/22 Corrected comments
*       kpt  07/24/22 Moved KAT related code to xsecure_kat_plat.c
* 5.2   ng   07/05/23 Added support for system device tree flow
*       yog  08/07/23 Removed trng driver in xilsecure library
* 5.4   yog  04/29/24 Fixed doxygen grouping
* 5.5   tvp  05/13/25 Code refactoring for Platform specific TRNG functions
*       sd   11/07/25 Update TRNG function name and return logic to return true
*                     when TRNG is uninitialized
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_trng_server_apis Xilsecure TRNG Server APIs
* @{
*/
#ifndef XSECURE_TRNG_H
#define XSECURE_TRNG_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xtrngpsx.h"

/************************************ Constant Definitions ****************************************/
#define XSECURE_TRNG_SEC_STRENGTH_IN_BYTES 		XTRNGPSX_SEC_STRENGTH_IN_BYTES
							/**< Security strength in bytes for
							 * XTRNGPSX*/
#define XSECURE_TRNG_PER_STRING_LEN_IN_BYTES 		XTRNGPSX_PERS_STRING_LEN_IN_BYTES
							/**< Personalization string length in bytes
							 * for XTRNGPSX */

/************************************* Macro Definitions ******************************************/
#define XSecure_PreOperationalSelfTests XTrngpsx_PreOperationalSelfTests
					/**< Redefine XTrngpsx_PreOperationalSelfTests as
					 * XSecure_PreOperationalSelfTests for compatibility */
#define XSecure_TrngGenerate XTrngpsx_Generate
					/**< Redefine XTrngpsx_Generate as XSecure_TrngGenerate
					 * for compatibility */
#define XSecure_Uninstantiate XTrngpsx_Uninstantiate
					/**< Redefine XTrngpsx_Uninstantiate as
					 * XSecure_Uninstantiate for compatibility */

/************************************ Variable Definitions ****************************************/

/************************************** Type Definitions ******************************************/
typedef XTrngpsx_Instance XSecure_TrngInstance;  /**< typedef XTrngpsx_Instance as
						  * XSecure_TrngInstance for compatibility */

/* This typedef contains mode information on which TRNG operates */
typedef enum {
	XSECURE_TRNG_DRNG_MODE = 1U,	/**< DRNG mode for TRNG */
	XSECURE_TRNG_PTRNG_MODE,	/**< PTRNG mode for TRNG */
	XSECURE_TRNG_HRNG_MODE		/**< HRNG mode for TRNG */
} XSecureTrng_Mode;

/************************************** Function Prototypes ***************************************/
int XSecure_GetRandomNum(u8 *Output, u32 Size);
XSecure_TrngInstance *XSecure_GetTrngInstance(void);
int XSecure_TrngInitNCfgMode(int XSecureTrngMode, u8 *Seed, u32 SeedLength, u8 *PersStr);

/************************************** Function Definitions **************************************/

/**************************************************************************************************/
/**
 * @brief	This is a wrapper function to check trngpsx initialization status.
 *
 * @param	TrngInstance Pointer to XTrngpsx Instance.
 *
 * @return
 *		 - XST_SUCCESS  If TRNGPSX is not initialized.
 *		 - XST_FAILURE  If TRNGPSX is initialized.
 *
 **************************************************************************************************/
static inline u8 XSecure_TrngIsUninitialized(XSecure_TrngInstance *TrngInstance)
{
	return (TrngInstance->State == XTRNGPSX_UNINITIALIZED_STATE);
}

/**************************************************************************************************/
/**
 * @brief	This is a wrapper function to check trngpsx health status.
 *
 * @param	TrngInstance Pointer to XTrngpsx Instance.
 *
 * @return
 *		 - XST_SUCCESS  If status of TRNGPSX is healthy.
 *		 - XST_FAILURE  If status of TRNGPSX is not healthy.
 *
 **************************************************************************************************/
static inline u8 XSecure_TrngIsHealthy(XSecure_TrngInstance *TrngInstance)
{
	return (TrngInstance->ErrorState == XTRNGPSX_HEALTHY);
}

#ifdef __cplusplus
}
#endif
#endif /** XSECURE_TRNG_H */
/** @} */
