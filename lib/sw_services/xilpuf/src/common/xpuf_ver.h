/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xpuf_ver.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 2.2  mmd     07/04/23 Initial Release
*      vss     09/21/23 Fixed doxygen warnings
* </pre>
*
******************************************************************************/

#ifndef XPUF_VER_H
#define XPUF_VER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/
#include "xil_util.h"

/**************************** Constant Definitions ****************************/
#define XPUF_MAJOR_VERSION	2 /**< Major version of Xilpuf */
#define XPUF_MINOR_VERSION	2 /**< Minor version of Xilpuf */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the version number of XilPuf library.
 *
 * @return	32-bit version number
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
u32 XPuf_GetLibVersion (void)
{
	return (XIL_BUILD_VERSION(XPUF_MAJOR_VERSION, XPUF_MINOR_VERSION));
}

#ifdef __cplusplus
}
#endif

#endif /* XPUF_VER_H */
