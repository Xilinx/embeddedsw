/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_class_dfu.h
 *
 * This file contains definitions used in the DFU class code.
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

/*
 * DFU status
 */
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

/*
 * DFU commands
 */
#define DFU_DETACH      0	/**< Dfu Detach. */
#define DFU_DNLOAD      1	/**< Dfu Dnload. */
#define DFU_UPLOAD      2	/**< Dfu Upload. */
#define DFU_GETSTATUS   3	/**< Dfu Getstatus. */
#define DFU_CLRSTATUS   4	/**< Dfu Clrstatus. */
#define DFU_GETSTATE    5	/**< Dfu Getstate. */
#define DFU_ABORT       6	/**< Dfu Abort. */

/*
 * DFU alternate setting value when in run-time mode
 */
#define DFU_ALT_SETTING 		1	/**< Dfu Alt Setting. */

/**************************** Type Definitions *******************************/
struct dfu_if {
	struct Usb_DevData *InstancePtr;
	u8 curr_state;
	u8 next_state;
	u8 status;
	u8 got_reset;
	u32 current_inf;	/**< current interface */
	u8 got_dnload_rqst;
	u32 total_transfers;
	u32 total_bytes_dnloaded;
	u32 total_bytes_uploaded;
	volatile u8 dfu_wait_for_interrupt;	/**< Dfu_wait_for_interrupt. */
	u8 is_dfu;
	u8 runtime_to_dfu;
};

/************************** Function Prototypes ******************************/
s32 Usb_DfuSetState(struct dfu_if *DFU, u8 dfu_state );
void USB_DfuSetDwloadState(struct dfu_if *DFU, u8 *status);
void USB_DfuGetStatus(struct dfu_if *DFU, u8 *status);
void Usb_DfuDisconnect(struct Usb_DevData *InstancePtr);
void Usb_DfuReset(struct Usb_DevData *InstancePtr);
void Usb_DfuSetIntf(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void Usb_DfuClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_DFU_H */
