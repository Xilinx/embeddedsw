/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xocp_ver.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.3   mss     10/19/23 Initial Release
*
* </pre>
*
******************************************************************************/

#ifndef XOCP_VER_H
#define XOCP_VER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/
#include "xil_util.h"

/**************************** Constant Definitions ****************************/
#define XOCP_MAJOR_VERSION	1U /**< Major Version of XilOcp */
#define XOCP_MINOR_VERSION	3U /**< Minor version of XilOcp */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the version number of XilOcp library.
 *
 * @return	32-bit version number
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
u32 XOcp_GetLibVersion (void)
{
	return XIL_BUILD_VERSION(XOCP_MAJOR_VERSION, XOCP_MINOR_VERSION);
}

#ifdef __cplusplus
}
#endif

#endif /* XOCP_VER_H */
