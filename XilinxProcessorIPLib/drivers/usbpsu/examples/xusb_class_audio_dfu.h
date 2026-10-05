/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_class_audio_dfu.h
 *
 * This file contains definitions used in the AUDIO-DFU composite class code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   22/02/18 First release
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef XUSB_CLASS_DFU_H
#define XUSB_CLASS_DFU_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xusb_ch9.h"

/************************** Constant Definitions *****************************/
/* DFU states */
#define STATE_APP_IDLE                  0x00	/**< State App Idle. */
#define STATE_APP_DETACH                0x01	/**< State App Detach. */
#define STATE_DFU_IDLE                  0x02	/**< State Dfu Idle. */
#define STATE_DFU_DOWNLOAD_SYNC         0x03	/**< State Dfu Download Sync. */
#define STATE_DFU_DOWNLOAD_BUSY         0x04	/**< State Dfu Download Busy. */
#define STATE_DFU_DOWNLOAD_IDLE         0x05	/**< State Dfu Download Idle. */
#define STATE_DFU_MANIFEST_SYNC         0x06	/**< State Dfu Manifest Sync. */
#define STATE_DFU_MANIFEST              0x07	/**< State Dfu Manifest. */
#define STATE_DFU_MANIFEST_WAIT_RESET   0x08	/**< State Dfu Manifest Wait Reset. */
#define STATE_DFU_UPLOAD_IDLE           0x09	/**< State Dfu Upload Idle. */
#define STATE_DFU_ERROR                 0x0a	/**< State Dfu Error. */

/* DFU status */
#define DFU_STATUS_OK                   0x00	/**< DFU status: No error. */
#define DFU_STATUS_ERROR_TARGET         0x01	/**< Dfu Status Error Target. */
#define DFU_STATUS_ERROR_FILE           0x02	/**< Dfu Status Error File. */
#define DFU_STATUS_ERROR_WRITE          0x03	/**< Dfu Status Error Write. */
#define DFU_STATUS_ERROR_ERASE          0x04	/**< Dfu Status Error Erase. */
#define DFU_STATUS_ERROR_CHECK_ERASED   0x05	/**< Dfu Status Error Check Erased. */
#define DFU_STATUS_ERROR_PROG           0x06	/**< Dfu Status Error Prog. */
#define DFU_STATUS_ERROR_VERIFY         0x07	/**< Dfu Status Error Verify. */
#define DFU_STATUS_ERROR_ADDRESS        0x08	/**< Dfu Status Error Address. */
#define DFU_STATUS_ERROR_NOTDONE        0x09	/**< Dfu Status Error Notdone. */
#define DFU_STATUS_ERROR_FIRMWARE       0x0a	/**< Dfu Status Error Firmware. */
#define DFU_STATUS_ERROR_VENDOR         0x0b	/**< Dfu Status Error Vendor. */
#define DFU_STATUS_ERROR_USBR           0x0c	/**< Dfu Status Error Usbr. */
#define DFU_STATUS_ERROR_POR            0x0d	/**< Dfu Status Error Por. */
#define DFU_STATUS_ERROR_UNKNOWN        0x0e	/**< Dfu Status Error Unknown. */
#define DFU_STATUS_ERROR_STALLEDPKT     0x0f	/**< Dfu Status Error Stalledpkt. */

/* DFU commands */
#define DFU_DETACH      		0	/**< Dfu Detach. */
#define DFU_DNLOAD      		1	/**< Dfu Dnload. */
#define DFU_UPLOAD      		2	/**< Dfu Upload. */
#define DFU_GETSTATUS   		3	/**< Dfu Getstatus. */
#define DFU_CLRSTATUS   		4	/**< Dfu Clrstatus. */
#define DFU_GETSTATE    		5	/**< Dfu Getstate. */
#define DFU_ABORT       		6	/**< Dfu Abort. */

/* DFU alternate setting value when in run-time mode */
#define DFU_ALT_SETTING 		1	/**< Dfu Alt Setting. */

/* A.14 Audio Class-Specific Request Codes */
#define UAC2_CS_CUR			0x01	/**< Uac2 Cs Cur. */
#define UAC2_CS_RANGE			0x02	/**< Uac2 Cs Range. */

/* A.17 Control Selector Codes */
/* A.17.1 Clock Source Control Selectors */
#define UAC2_CS_CONTROL_UNDEFINED	0x00	/**< Uac2 Cs Control Undefined. */
#define UAC2_CS_CONTROL_SAM_FREQ	0x01	/**< Uac2 Cs Control Sam Freq. */
#define UAC2_CS_CONTROL_CLOCK_VALID	0x02	/**< Uac2 Cs Control Clock Valid. */

/* A.17.2 Clock Selector Control Selectors */
#define UAC2_CX_CONTROL_UNDEFINED	0x00	/**< Uac2 Cx Control Undefined. */
#define UAC2_CX_CONTROL_CLOCK_SEL	0x01	/**< Uac2 Cx Control Clock Sel. */

/* A.17.7 Feature Unit Control Selectors */
#define UAC2_FU_CONTROL_UNDEFINED	0x00	/**< Uac2 Fu Control Undefined. */
#define UAC2_FU_MUTE_CONTROL		0x01	/**< Uac2 Fu Mute Control. */
#define UAC2_FU_VOLUME_CONTROL		0x02	/**< Uac2 Fu Volume Control. */

#define ISO_EP				2	/**< Iso Ep. */
/** @brief Interval Per Second. */
#define INTERVAL_PER_SECOND		(u32)(8000)

/* Isoc interval for AUDIO */
#define AUDIO_INTERVAL			0x04	/**< Audio Interval. */

/* Array of supported sampling frequencies */
#define MAX_AUDIO_FREQ			0x04	/**< Max Audio Freq. */
#define CUR_AUDIO_FREQ			0x01	/**< Cur Audio Freq. */

/* Bytes per Audio transferred frame */
#define AUDIO_FRAME_SIZE		0x02	/**< Audio Frame Size. */

/* Bits per Audio transferred frame */
/** @brief Bit Resolution. */
#define BIT_RESOLUTION			((AUDIO_FRAME_SIZE) * (0x08))

#define AUDIO_INTF			0	/**< Audio Intf. */
#define AUDIO_INTF_OUT			1	/**< Audio Intf Out. */
#define AUDIO_INTF_IN			2	/**< Audio Intf In. */
#define DFU_INTF			3	/**< Dfu Intf. */

/** @brief D I V_ R O U N D_ U P(n, d). */
#define DIV_ROUND_UP(n, d)		(((n) + (d) - 1) / (d))

/**************************** Type Definitions *******************************/
/** @brief DFU interface state and control data structure. */
struct dfu_if {
	struct Usb_DevData *InstancePtr;	/**< Instance Ptr. */
	u8	curr_state;	/**< Curr_state. */
	u8	next_state;	/**< Next_state. */
	u8	status;	/**< Status. */
	u8	got_reset;	/**< Got_reset. */
	u32	current_inf;	/**< Current_inf. */
	u8	got_dnload_rqst;	/**< Got_dnload_rqst. */
	u32	total_transfers;	/**< Total_transfers. */
	u32	total_bytes_dnloaded;	/**< Total_bytes_dnloaded. */
	u32	total_bytes_uploaded;	/**< Total_bytes_uploaded. */
	u8	dfu_wait_for_interrupt;	/**< Dfu_wait_for_interrupt. */
	u8	is_dfu;	/**< Is_dfu. */
	u8	runtime_to_dfu;	/**< Runtime_to_dfu. */
};

/** @brief USB Audio interface descriptor structure. */
struct audio_if {
	u32	framesize;	/**< Framesize. */
	u32	interval;	/**< Interval. */
	u32	packetsize;	/**< Packetsize. */
	u32	packetresidue;	/**< Packetresidue. */
	u32	residue;	/**< Residue. */
	u32	index;	/**< Index. */
	u32	firstpkt;	/**< Firstpkt. */
};

/** @brief Combined audio and DFU interface structure. */
struct audio_dfu_if {
	u8			*virtualdisk;	/**< Virtualdisk. */
	u32			disksize;	/**< F_dfu. */
	struct audio_if		f_audio;	/**< Audio interface instance. */
	struct dfu_if		f_dfu;	/**< DFU interface instance. */
};

/************************** Function Prototypes ******************************/

s32 Usb_DfuSetState(struct dfu_if *DFU, u8 dfu_state );
void USB_DfuSetDwloadState(struct dfu_if *DFU, u8 *status);
void USB_DfuGetStatus(struct dfu_if *DFU, u8 *status);
void Usb_DisconnectHandler(struct Usb_DevData *InstancePtr);
void Usb_ResetHandler(struct Usb_DevData *InstancePtr);
void Usb_SetIntf(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void Usb_ClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_DFU_H */
