/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_ver.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 2.5   tus     07/29/26 Initial Release
*
* </pre>
*
******************************************************************************/

#ifndef XLOADER_VER_H
#define XLOADER_VER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/
#include "xil_util.h"

/**************************** Constant Definitions ****************************/
#define XLOADER_MODULE_MAJOR_VERSION	2U /**< XilLoader library major version */
#define XLOADER_MODULE_MINOR_VERSION	5U /**< XilLoader library minor version */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the version number of XilLoader library.
 *
 * @return	32-bit version number
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
u32 XLoader_GetLibVersion(void)
{
	return (XIL_BUILD_VERSION(XLOADER_MODULE_MAJOR_VERSION, XLOADER_MODULE_MINOR_VERSION));
}

#ifdef __cplusplus
}
#endif

#endif /* XLOADER_VER_H */
