/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xnvm_ver.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 3.2   mmd     07/04/23 Initial Release
* </pre>
*
******************************************************************************/

#ifndef XNVM_VER_H
#define XNVM_VER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/
#include "xil_util.h"

/**************************** Constant Definitions ****************************/
#define XNVM_MAJOR_VERSION	3 /**< Major Version of XilNvm */
#define XNVM_MINOR_VERSION	2 /**< Minor version of XilNvm */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the version number of XilNvm library.
 *
 * @return	32-bit version number
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
u32 XNvm_GetLibVersion (void)
{
	return (XIL_BUILD_VERSION(XNVM_MAJOR_VERSION, XNVM_MINOR_VERSION));
}

#ifdef __cplusplus
}
#endif

#endif /* XNVM_VER_H */
