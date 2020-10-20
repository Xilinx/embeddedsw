/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_core.h
* @{
*
* Header file for core control and wait functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   03/20/2020  Remove range apis
* 1.2   Tejus   06/01/2020  Add core debug halt apis
* 1.3   Tejus   06/01/2020  Add api to read core done bit.
* 1.4   Tejus   06/05/2020  Add api to reset/unreset aie cores.
* </pre>
*
******************************************************************************/
#ifndef XAIECORE_H
#define XAIECORE_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaie_helper.h"

/************************** Constant Definitions *****************************/
/************************** Function Prototypes  *****************************/
AieRC XAie_CoreDisable(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC XAie_CoreEnable(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC XAie_CoreWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 TimeOut);
AieRC XAie_CoreWaitForDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 TimeOut);
AieRC XAie_CoreDebugHalt(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC XAie_CoreDebugUnhalt(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC XAie_CoreReadDoneBit(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 *DoneBit);
AieRC XAie_CoreReset(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC XAie_CoreUnreset(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC XAie_CoreConfigDebugControl1(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_Events Event0, XAie_Events Event1,
		XAie_Events SingleStepEvent, XAie_Events ResumeCoreEvent);
AieRC XAie_CoreClearDebugControl1(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC XAie_CoreConfigureDone(XAie_DevInst *DevInst, XAie_LocType Loc);

#endif		/* end of protection macro */
/** @} */
