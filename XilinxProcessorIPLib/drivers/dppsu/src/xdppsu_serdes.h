/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdppsu_serdes.h
 * This file contains all the functions used by DisplayPort to configure SERDES
 * for link training
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  05/17/14 Initial release.
 * </pre>
 *
*******************************************************************************/
#ifndef XDPPSU_SERDES_H_
/* Prevent circular inclusions by using protection macros. */
#define XDPPSU_SERDES_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/
#include "xdppsu_hw.h"
#include "xstatus.h"
#include "xdppsu.h"
/************************** Constant Definitions *****************************/
/* The maximum voltage swing level is 3. */
#define XDPPSU_MAXIMUM_VS_LEVEL 3
/* The maximum pre-emphasis level is 2. */
#define XDPPSU_MAXIMUM_PE_LEVEL 2

/* SERDES Config Functions */
void XDpPsu_CfgTxVsLevel(XDpPsu *InstancePtr, u8 Level, u8 TxLevel);
void XDpPsu_CfgTxPeLevel(XDpPsu *InstancePtr, u8 Level, u8 TxLevel);
void XDpPsu_SetVswingPreemp(XDpPsu *InstancePtr, u8 *AuxData);

#ifdef __cplusplus
}
#endif

#endif
