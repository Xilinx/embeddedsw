/*******************************************************************************
* Copyright (C) 2016 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xddcrpsu.h
 * @addtogroup ddrcpsu Overview
 * @{
 * @details
 *
 * The Xilinx DdrcPsu driver. This driver supports the Xilinx ddrcpsu
 * IP core.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0	ssc   04/28/16 First Release.
 * 1.1  adk   04/08/16 Export DDR freq to xparameters.h file.
 * 1.5  ht    08/03/23 Added support for system device-tree flow.
 *
 * </pre>
 *
*******************************************************************************/

#ifndef XDDRCPS_H_
/* Prevent circular inclusions by using protection macros. */
#define XDDRCPS_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/
#ifdef SDT
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/**************************** Type Definitions *******************************/
typedef struct {
	char *Name;
	UINTPTR BaseAddress;
	u8 HasEccEn;
	u32 InputClockFreq;
	u8 AddrMapping;
	u32 DdrFreq; /* DDR Freq in Hz */
	u32 VideoBufSize;
	u32 BrcMapping;
	u8 HasDynamicDDrEn;
	u8 Memtype;
	u8 MemAddrMap;
	u32 DataMask_Dbi;
	u32 AddressMirror;
	u32 Secondclk;
	u32 Parity;
	u32 PwrDnEn;
	u32 ClockStopEn;
	u32 LpAsr;
	u32 TRefMode;
	u32 Fgrm;
	u32 SelfRefAbort;
	u32 TRefRange;
} XDdrcpsu_Config;


typedef struct {
	XDdrcpsu_Config Config; /**< Configuration structure */
	u32 IsReady;            /**< Device is initialized and ready */
} XDdrcPsu;

/************************** Function Prototypes ******************************/
XDdrcpsu_Config *XDdrcPsu_LookupConfig(UINTPTR BaseAddress);
s32 XDdrcPsu_CfgInitialize(XDdrcPsu *InstancePtr, XDdrcpsu_Config *CfgPtr);

#endif

#ifdef __cplusplus
}
#endif

#endif /* XDDRCPS_H_ */
/** @} */
