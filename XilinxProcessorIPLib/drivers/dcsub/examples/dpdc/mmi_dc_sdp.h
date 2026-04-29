/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dc_sdp.h
*
* This header file contains SDP functionality declarations
*
******************************************************************************/

#ifndef MMI_DC_SDP_H_
#define MMI_DC_SDP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mmi_dc_nonlive_test.h"

/* SDP packet buffer parameters */
#define XDPDC_SDP_BUFFER_ADDR		0x03000000
#define XDPDC_SDP_BUFFER_OFFSET		0x24U
#define XDPDC_SDP_PACKET_COUNT		64U
#define XDPDC_SDP_TOTAL_BYTES		(XDPDC_SDP_PACKET_COUNT * \
					 XDPDC_SDP_BUFFER_OFFSET)

/* Function prototypes */
void XDpDc_InitSdpFrameBuffer(RunConfig *RunCfgPtr);
void XDpDc_FillSdpBuffer(RunConfig *RunCfgPtr);
void XDpDc_ConfigureSdp(RunConfig *RunCfgPtr);
void XDpDc_SetupSdpDescriptor(RunConfig *RunCfgPtr);
void XDpDc_ConfigureSdpDMA(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif

#endif /* MMI_DC_SDP_H_ */
