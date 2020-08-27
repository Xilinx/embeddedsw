/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_reset_aie.h
* @{
*
* This file contains routines for AIE reset controls. This header file is not
* exposed to the user.
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaiegbl.h"

/************************** Function Prototypes  *****************************/
AieRC _XAie_RstShims(XAie_DevInst *DevInst, u32 StartCol, u32 NumCols);

/** @} */
