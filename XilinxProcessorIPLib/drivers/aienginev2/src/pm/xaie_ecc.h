/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_ecc.h
* @{
*
* Header file for ecc implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date        Changes
* ----- ------   --------    --------------------------------------------------
* 1.0   Dishita  08/10/2020  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_ECC_H
#define XAIE_ECC_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"

/************************** Enum *********************************************/

/************************** Function Prototypes  *****************************/
void _XAie_EccEvntResetPM(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC _XAie_EccOnPM(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC _XAie_EccOnDM(XAie_DevInst *DevInst, XAie_LocType Loc);
#endif		/* end of protection macro */
