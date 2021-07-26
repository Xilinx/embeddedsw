/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdppsu_common_example.h
 *
 * Contains a design example using the XDpPsu driver. It performs a self test on
 * the DisplayPort TX core by training the main link at the maximum common
 * capabilities between the TX and RX and checking the lane status.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  09/17/17 Initial creation.
 * </pre>
 *
*******************************************************************************/

#ifndef XDPPSU_COMMON_EXAMPLE_H_
/* Prevent circular inclusions by using protection macros. */
#define XDPPSU_COMMON_EXAMPLE_H_

/******************************* Include Files ********************************/

#include "xdppsu.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"

/**************************** Constant Definitions ****************************/

/* The unique device ID of the DisplayPort TX core instance to be used with the
 * examples. */
#define DPPSU_DEVICE_ID XPAR_PSU_DP_DEVICE_ID

/* The link rate setting to begin link training with. Valid values are:
 * XDPPSU_LINK_BW_SET_540GBPS, XDPPSU_LINK_BW_SET_270GBPS, and
 * XDPPSU_LINK_BW_SET_162GBPS. */
#define TRAIN_USE_LINK_RATE XDPPSU_LINK_BW_SET_540GBPS
/* The lane count setting to begin link training with. Valid values are:
 * XDPPSU_LANE_COUNT_SET_4, XDPPSU_LANE_COUNT_SET_2, and
 * XDPPSU_LANE_COUNT_SET_1. */
#define TRAIN_USE_LANE_COUNT XDPPSU_DPCD_LANE_COUNT_SET_2
/* If set to 1, TRAIN_USE_LINK_RATE and TRAIN_USE_LANE_COUNT will be ignored.
 * Instead, the maximum common link capabilities between the DisplayPort TX core
 * and the RX device will be used when establishing a link.
 * If set to 0, TRAIN_USE_LINK_RATE and TRAIN_USE_LANE_COUNT will determine the
 * link rate and lane count settings that the link training process will begin
 * with. */
#define TRAIN_USE_MAX_LINK 1

/**************************** Function Prototypes *****************************/

extern u32 DpPsu_PlatformInit(void);
extern u32 DpPsu_StreamSrcSync(XDpPsu *InstancePtr);
extern u32 DpPsu_StreamSrcSetup(XDpPsu *InstancePtr);
extern u32 DpPsu_StreamSrcConfigure(XDpPsu *InstancePtr);

u32 DpPsu_SetupExample(XDpPsu *InstancePtr, u16 DeviceId);
u32 DpPsu_StartLink(XDpPsu *InstancePtr);
u32 DpPsu_Run(XDpPsu *InstancePtr);

/*************************** Variable Declarations ****************************/

XDpPsu DpPsuInstance;

#endif /* XDPPSU_COMMON_EXAMPLE_H_ */
