/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xemacps_pcs.h
* @addtogroup emacps Overview
* @{
*
* This file contains the declarations of PCS block api's.
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

/*
 * Initialization functions in xemacps_pcs.c
 */
void XEmacPs_SetupPCS(XEmacPs *EmacPsInstancePtr);
bool XEmacPs_USXPCSGetState(UINTPTR BaseAddr);
bool XEmacPs_IsHighSpeedPCS(UINTPTR);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
