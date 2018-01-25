/*******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
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

#endif
