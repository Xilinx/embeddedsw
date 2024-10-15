/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_trng.h
* This file contains function declaration to get random number.
*
* This header file contains function declaration to get random number.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   kpt  05/05/22 Initial release
*       dc   07/12/22 Corrected comments
*       kpt  07/24/22 Moved KAT related code to xsecure_kat_plat.c
* 5.2   ng   07/05/23 Added support for system device tree flow
*       yog  08/07/23 Removed trng driver in xilsecure library
* 5.4   yog  04/29/24 Fixed doxygen grouping
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_trng_server_apis Xilsecure TRNG Server APIs
* @{
*/
#ifndef XSECURE_TRNG_H
#define XSECURE_TRNG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Function Prototypes ****************************/
int XSecure_GetRandomNum(u8 *Output, u32 Size);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
