/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
#define UAC2_CS_CUR				0x01	/**< Uac2 Cs Cur. */
#define UAC2_CS_RANGE				0x02	/**< Uac2 Cs Range. */

/* A.17 Control Selector Codes */
/* A.17.1 Clock Source Control Selectors */
#define UAC2_CS_CONTROL_UNDEFINED		0x00	/**< Uac2 Cs Control Undefined. */
#define UAC2_CS_CONTROL_SAM_FREQ		0x01	/**< Uac2 Cs Control Sam Freq. */
#define UAC2_CS_CONTROL_CLOCK_VALID		0x02	/**< Uac2 Cs Control Clock Valid. */

/* A.17.2 Clock Selector Control Selectors */
#define UAC2_CX_CONTROL_UNDEFINED		0x00	/**< Uac2 Cx Control Undefined. */
#define UAC2_CX_CONTROL_CLOCK_SEL		0x01	/**< Uac2 Cx Control Clock Sel. */

/* A.17.7 Feature Unit Control Selectors */
#define UAC2_FU_CONTROL_UNDEFINED		0x00	/**< Uac2 Fu Control Undefined. */
#define UAC2_FU_MUTE_CONTROL			0x01	/**< Uac2 Fu Mute Control. */
#define UAC2_FU_VOLUME_CONTROL			0x02	/**< Uac2 Fu Volume Control. */

/** @brief Audio Config. */
#define AUDIO_CONFIG				(1 << 0)
/** @brief Audio Unconfig. */
#define AUDIO_UNCONFIG				(1 << 1)
/** @brief Record Start. */
#define RECORD_START				(1 << 2)
/** @brief Record Stop. */
#define RECORD_STOP				(1 << 3)
/** @brief Play Start. */
#define PLAY_START				(1 << 4)
/** @brief Play Stop. */
#define PLAY_STOP				(1 << 5)

/************************** Variable Definitions ******************************/
/** @brief Audio device state and buffer management structure. */
struct audio_dev {
	u8			*virtualdisk;	/**< Virtualdisk. */
	u32			disksize;	/**< Disksize. */
	u32			framesize;	/**< Framesize. */
	u32			interval;	/**< Interval. */
	u32			packetsize;	/**< Packetsize. */
	u32			packetresidue;	/**< Packetresidue. */
	u32			residue;	/**< Residue. */
	u32			index;	/**< Index. */
	u32			firstpkt;	/**< Firstpkt. */
	u32			bytesRecv;	/**< Bytes Recv. */
	TaskHandle_t		xMainTask;	/**< X Main Task. */
	TaskHandle_t		xRecordTask;	/**< X Record Task. */
	TaskHandle_t		xPlayTask;	/**< X Play Task. */
	xSemaphoreHandle	xSemaphoreRecord;	/**< X Semaphore Record. */
	xSemaphoreHandle	xSemaphorePlay;	/**< X Semaphore Play. */
};

extern struct audio_dev audio_dev;
extern u8 BufferPtrTemp[1024];	/**< Buffer Ptr Temp. */

/************************** Function Prototypes *******************************/
void Usb_ClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void prvRecordTask(void *pvParameters);
void prvPlayBackTask(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_AUDIO_H */
