/******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xemacps_pcs.h
* @addtogroup emacps Overview
* @{
*
* Contains the declarations of PCS block api's.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 3.22  vineeth  11/15/24 First Release
* </pre>
*
******************************************************************************/

#ifndef XEMACPS_PCS_H		/* prevent circular inclusions */
#define XEMACPS_PCS_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include <stdbool.h>
#include "xemacps.h"
#include "xemacps_hw.h"
/*****************************************************************************/

/**
 * @brief Configures the PCS block for high-speed operation.
 *
 * @param[in] EmacPsInstancePtr Pointer to the EMACPS instance.
 */
void XEmacPs_SetupPCS(XEmacPs *EmacPsInstancePtr);

/**
 * @brief Reads the USX PCS block synchronization state.
 *
 * @param[in] BaseAddr Base address of the device.
 *
 * @return TRUE if block synchronization is achieved, FALSE otherwise.
 */
bool XEmacPs_USXPCSGetState(UINTPTR BaseAddr);

/**
 * @brief Detects whether the device supports high-speed PCS modes.
 *
 * @param[in] BaseAddr Base address of the device.
 *
 * @return TRUE if high-speed PCS is supported, FALSE otherwise.
 */
bool XEmacPs_IsHighSpeedPCS(UINTPTR);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* end of protection macro */
