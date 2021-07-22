/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_reset_aieml.h
* @{
*
* This file contains routines for AIEML reset controls. This header file is not
* exposed to the user.
*
******************************************************************************/

#ifndef XAIE_RESET_AIEML_H
#define XAIE_RESET_AIEML_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"

/************************** Function Prototypes  *****************************/
AieRC _XAieMl_RstShims(XAie_DevInst *DevInst, u32 StartCol, u32 NumCols);

#endif /* XAIE_RESET_AIEML_H */
/** @} */
