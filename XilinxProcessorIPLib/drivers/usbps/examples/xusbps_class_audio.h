/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusbps_class_audio.h
 *
 * This file contains the implementation of chapter 9 specific code for
 * the example.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who	Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   pm	20/02/20 First release
 *
 * </pre>
 *
 *****************************************************************************/
#ifndef  XUSB_CLASS_AUDIO_H
#define  XUSB_CLASS_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files **********************************/
#include "xusbps_ch9.h"
#include "xusbps_ch9_audio.h"
#include "xparameters.h"

/************************** Constant Definitions ******************************/
#ifdef XUSBPS_UAC1

/* Audio Class-Specific Request Codes */
#define UAC1_SET_CUR				0x01
#define UAC1_GET_CUR				0x81
#define UAC1_GET_MIN				0x82
#define UAC1_GET_MAX				0x83
#define UAC1_GET_RES				0x84

#else	/* XUSBPS_UAC2 */

/* A.14 Audio Class-Specific Request Codes */
#define UAC2_CS_CUR				0x01
#define UAC2_CS_RANGE				0x02

/* A.17 Control Selector Codes */
/* A.17.1 Clock Source Control Selectors */
#define UAC2_CS_CONTROL_UNDEFINED		0x00
#define UAC2_CS_CONTROL_SAM_FREQ		0x01
#define UAC2_CS_CONTROL_CLOCK_VALID		0x02

/* A.17.2 Clock Selector Control Selectors */
#define UAC2_CX_CONTROL_UNDEFINED		0x00
#define UAC2_CX_CONTROL_CLOCK_SEL		0x01

/* A.17.7 Feature Unit Control Selectors */
#define UAC2_FU_CONTROL_UNDEFINED		0x00
#define UAC2_FU_MUTE_CONTROL			0x01
#define UAC2_FU_VOLUME_CONTROL			0x02

#endif

/************************** Variable Definitions ******************************/

/************************** Function Prototypes *******************************/
void XUsbPs_ClassReq(XUsbPs *InstancePtr,
		XUsbPs_SetupData *SetupData);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_AUDIO_H */
