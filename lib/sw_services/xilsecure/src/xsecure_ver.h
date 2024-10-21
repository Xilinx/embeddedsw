/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_ver.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   mmd     07/04/23 Initial Release
*	vss	09/21/23 Fixed doxygen warnings
* </pre>
*
******************************************************************************/

#ifndef XSECURE_VER_H
#define XSECURE_VER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/
#include "xil_sutil.h"

/**************************** Constant Definitions ****************************/
#define XSECURE_MAJOR_VERSION	5 /**< Major version of Xilsecure */
#define XSECURE_MINOR_VERSION	2 /**< Minor version of Xilsecure */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the version number of XilSecure library.
 *
 * @return	32-bit version number
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
u32 XSecure_GetLibVersion (void)
{
	return (XIL_BUILD_VERSION(XSECURE_MAJOR_VERSION, XSECURE_MINOR_VERSION));
}

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_VER_H */
