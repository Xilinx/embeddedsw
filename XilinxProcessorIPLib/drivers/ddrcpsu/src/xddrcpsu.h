/*******************************************************************************
* Copyright (C) 2016 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef XDDRCPS_H_		/* prevent circular inclusions */
#define XDDRCPS_H_

/******************************************************************************/
/**
 *
 * @file xddrcpsu.h
 * @addtogroup ddrcpsu Overview
 * @{
 * @brief Provides the Xilinx DdrcPsu driver for the ddrcpsu IP core.
 *
 * The Xilinx DdrcPsu driver. This driver supports the Xilinx ddrcpsu
 * IP core.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0	ssc   04/28/16 First Release.
 * 1.1  adk   04/08/16 Export DDR freq to xparameters.h file.
 * 1.5  ht    08/03/23 Added support for system device-tree flow.
 * 1.9  vn    08/26/26 Fix Doxygen warnings.
 * 1.9  tb    09/11/26 Align Doxygen comments with AMD standards.
 * </pre>
 *
*******************************************************************************/


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
	char *Name; /**< Device name. */
	UINTPTR BaseAddress; /**< Device base address. */
	u8 HasEccEn; /**< ECC enable flag. */
	u32 InputClockFreq; /**< Input clock frequency. */
	u8 AddrMapping; /**< Address mapping. */
	u32 DdrFreq; /**< DDR frequency in Hz. */
	u32 VideoBufSize; /**< Video buffer size. */
	u32 BrcMapping; /**< BRC mapping. */
	u8 HasDynamicDDrEn; /**< Dynamic DDR enable flag. */
	u8 Memtype; /**< Memory type. */
	u8 MemAddrMap; /**< Memory address map. */
	u32 DataMask_Dbi; /**< Data mask and DBI setting. */
	u32 AddressMirror; /**< Address mirroring setting. */
	u32 Secondclk; /**< Second clock setting. */
	u32 Parity; /**< Parity setting. */
	u32 PwrDnEn; /**< Power-down enable. */
	u32 ClockStopEn; /**< Clock-stop enable. */
	u32 LpAsr; /**< LPASR setting. */
	u32 TRefMode; /**< Refresh mode. */
	u32 Fgrm; /**< FGRM setting. */
	u32 SelfRefAbort; /**< Self-refresh abort setting. */
	u32 TRefRange; /**< Refresh range. */
} XDdrcpsu_Config;


typedef struct {
	XDdrcpsu_Config Config; /**< Configuration structure. */
	u32 IsReady;            /**< Device is initialized and ready. */
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
