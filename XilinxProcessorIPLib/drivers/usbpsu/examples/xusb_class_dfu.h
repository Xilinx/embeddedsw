/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_class_dfu.h
 *
 * This file contains definitions used in the DFU class code.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0	 vak  30/11/16 Added DFU support
 * 1.4	 BK   12/01/18 Renamed the file to be in sync with usb common code
 *		       changes for all USB IPs
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
/*
 * DFU states
 */
#define STATE_APP_IDLE                  0x00
#define STATE_APP_DETACH                0x01
#define STATE_DFU_IDLE                  0x02
#define STATE_DFU_DOWNLOAD_SYNC         0x03
#define STATE_DFU_DOWNLOAD_BUSY         0x04
#define STATE_DFU_DOWNLOAD_IDLE         0x05
#define STATE_DFU_MANIFEST_SYNC         0x06
#define STATE_DFU_MANIFEST              0x07
#define STATE_DFU_MANIFEST_WAIT_RESET   0x08
#define STATE_DFU_UPLOAD_IDLE           0x09
#define STATE_DFU_ERROR                 0x0a

/*
 * DFU status
 */
#define DFU_STATUS_OK                   0x00
#define DFU_STATUS_ERROR_TARGET         0x01
#define DFU_STATUS_ERROR_FILE           0x02
#define DFU_STATUS_ERROR_WRITE          0x03
#define DFU_STATUS_ERROR_ERASE          0x04
#define DFU_STATUS_ERROR_CHECK_ERASED   0x05
#define DFU_STATUS_ERROR_PROG           0x06
#define DFU_STATUS_ERROR_VERIFY         0x07
#define DFU_STATUS_ERROR_ADDRESS        0x08
#define DFU_STATUS_ERROR_NOTDONE        0x09
#define DFU_STATUS_ERROR_FIRMWARE       0x0a
#define DFU_STATUS_ERROR_VENDOR         0x0b
#define DFU_STATUS_ERROR_USBR           0x0c
#define DFU_STATUS_ERROR_POR            0x0d
#define DFU_STATUS_ERROR_UNKNOWN        0x0e
#define DFU_STATUS_ERROR_STALLEDPKT     0x0f

/*
 * DFU commands
 */
#define DFU_DETACH      0
#define DFU_DNLOAD      1
#define DFU_UPLOAD      2
#define DFU_GETSTATUS   3
#define DFU_CLRSTATUS   4
#define DFU_GETSTATE    5
#define DFU_ABORT       6

/*
 * DFU alternate setting value when in run-time mode
 */
#define DFU_ALT_SETTING 		1

/**************************** Type Definitions *******************************/
struct dfu_if {
	struct Usb_DevData *InstancePtr;
	u8 curr_state;
	u8 next_state;
	u8 status;
	u8 got_reset;
	u32 current_inf;	/* current interface */
	u8 got_dnload_rqst;
	u32 total_transfers;
	u32 total_bytes_dnloaded;
	u32 total_bytes_uploaded;
	volatile u8 dfu_wait_for_interrupt;
	u8 is_dfu;
	u8 runtime_to_dfu;
};

/************************** Function Prototypes ******************************/
s32 Usb_DfuSetState(struct dfu_if *DFU, u8 dfu_state );
void USB_DfuSetDwloadState(struct dfu_if *DFU, u8 *status);
s32 USB_DfuGetStatus(struct dfu_if *DFU, u8 *status);
void Usb_DfuDisconnect(struct Usb_DevData *InstancePtr);
void Usb_DfuReset(struct Usb_DevData *InstancePtr);
void Usb_DfuSetIntf(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void Usb_DfuClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_DFU_H */
