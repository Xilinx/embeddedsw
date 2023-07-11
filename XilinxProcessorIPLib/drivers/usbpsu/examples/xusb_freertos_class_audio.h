/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xusb_freertos_class_audio.h
*
* This file contains definitions used in the class code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   rb   26/03/18 First release
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
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "xparameters.h"
#include "xusb_ch9.h"
#include "xusb_freertos_ch9_audio.h"

/************************** Constant Definitions ******************************/

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

#define AUDIO_CONFIG				(1 << 0)
#define AUDIO_UNCONFIG				(1 << 1)
#define RECORD_START				(1 << 2)
#define RECORD_STOP				(1 << 3)
#define PLAY_START				(1 << 4)
#define PLAY_STOP				(1 << 5)

/************************** Variable Definitions ******************************/
struct audio_dev {
	u8			*virtualdisk;
	u32			disksize;
	u32			framesize;
	u32			interval;
	u32			packetsize;
	u32			packetresidue;
	u32			residue;
	u32			index;
	u32			firstpkt;
	u32			bytesRecv;
	TaskHandle_t		xMainTask;
	TaskHandle_t		xRecordTask;
	TaskHandle_t		xPlayTask;
	xSemaphoreHandle	xSemaphoreRecord;
	xSemaphoreHandle	xSemaphorePlay;
};

extern struct audio_dev audio_dev;
extern u8 BufferPtrTemp[1024];

/************************** Function Prototypes *******************************/
void Usb_ClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void prvRecordTask(void *pvParameters);
void prvPlayBackTask(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_AUDIO_H */
