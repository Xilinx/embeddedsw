/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_reset.h
* @{
*
* This file contains routines for AI engine resets.
*
******************************************************************************/
#ifndef XAIE_RESET_H
#define XAIE_RESET_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"
/**************************** Type Definitions *******************************/
/************************** Function Prototypes  *****************************/
AieRC XAie_ResetPartition(XAie_DevInst *DevInst);
AieRC XAie_ClearPartitionMems(XAie_DevInst *DevInst);
#endif		/* end of protection macro */

/** @} */
