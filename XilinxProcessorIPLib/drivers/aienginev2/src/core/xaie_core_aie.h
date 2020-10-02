/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_core_aie.h
* @{
*
* This file contains function prototypes for aie core apis.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   10/02/2020  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIECORE_AIE_H
#define XAIECORE_AIE_H
/***************************** Include Files *********************************/
#include "xaie_helper.h"
/************************** Function Prototypes  *****************************/
AieRC _XAie_CoreConfigureDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		const struct XAie_CoreMod *CoreMod);

#endif /* XAIECORE_AIE_H */
/** @} */
