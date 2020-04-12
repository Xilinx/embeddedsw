/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaiepm_clock.h
* @{
*
*  Header file for AIE clock gating
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date       Changes
* ----- ------  --------   ----------------------------------------------------
* 1.0   Dishita 03/11/2020 Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIEPM_CLOCK_H
#define XAIEPM_CLOCK_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/

/***************************** Type Definitions ******************************/

/***************************** Macro Definitions *****************************/

/************************** Function Prototypes  *****************************/
u8 XAiePm_RequestTiles(XAieGbl *AieInst, u32 NumTiles, XAie_LocType *Loc);
#endif		/* end of protection macro */

/** @} */
