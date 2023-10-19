/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xilskey_ver.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 7.6   mss     10/19/23 Initial Release
*
* </pre>
*
******************************************************************************/

#ifndef XSKEY_VER_H
#define XSKEY_VER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/
#include "xil_util.h"

/**************************** Constant Definitions ****************************/
#define XSKEY_MAJOR_VERSION	7U /**< Major Version of XilSkey */
#define XSKEY_MINOR_VERSION	6U /**< Minor version of XilSkey */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the version number of XilSkey library.
 *
 * @return	32-bit version number
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
u32 XSkey_GetLibVersion (void)
{
	return XIL_BUILD_VERSION(XSKEY_MAJOR_VERSION, XSKEY_MINOR_VERSION);
}

#ifdef __cplusplus
}
#endif

#endif /* XILSKEY_VER_H */
