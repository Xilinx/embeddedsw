/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_ver.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 2.0   mss     10/19/23 Initial Release
*
* </pre>
*
******************************************************************************/

#ifndef XPLMI_VER_H
#define XPLMI_VER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/
#include "xil_util.h"

/**************************** Constant Definitions ****************************/
#define XPLMI_MAJOR_VERSION	2U /**< Major Version of XilPlmi */
#define XPLMI_MINOR_VERSION	0U /**< Minor version of XilPlmi */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the version number of XilPlmi library.
 *
 * @return	32-bit version number
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
u32 XPlmi_GetLibVersion (void)
{
	return XIL_BUILD_VERSION(XPLMI_MAJOR_VERSION, XPLMI_MINOR_VERSION);
}

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_VER_H */
