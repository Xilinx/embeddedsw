/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 * @file xusb_msc.h
 *
 * This file contains the mass-storage class helpers used by the mass-storage
 * examples.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- ------------------------------------------------------------------
 * 5.7   sm   10/01/26 First release. Added XUsb_MsRejectCommand() and
 *			XUsb_MsSetSense().
 *
 * </pre>
 *****************************************************************************/

#ifndef XUSB_MSC_H
#define XUSB_MSC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files **********************************/

#include "xusb.h"

/************************** Constant Definitions ******************************/

/*
 * CSW command status.
 */
#define XUSB_MSC_CSW_FAILED	0x01

/*
 * Fixed format sense data returned for REQUEST SENSE.
 */
#define XUSB_MSC_SENSE_LEN			18
#define XUSB_MSC_SENSE_KEY_NO_SENSE		0x00
#define XUSB_MSC_SENSE_KEY_ILLEGAL_REQUEST	0x05
#define XUSB_MSC_SENSE_ASC_INVALID_OPCODE	0x20
#define XUSB_MSC_SENSE_ASC_LBA_OUT_OF_RANGE	0x21
#define XUSB_MSC_SENSE_ASC_INVALID_CDB_FIELD	0x24

/************************** Function Prototypes *******************************/

void XUsb_MsRejectCommand(XUsb *InstancePtr, u8 EndPoint,
			  u32 TransferLength, u8 *CswStatus, u32 *CswResidue,
			  volatile u32 *BlockCount, volatile u8 *CmdFlag,
			  volatile u8 *SendStatus);
void XUsb_MsSetSense(u8 *SenseData, u8 SenseKey, u8 Asc, u8 Ascq);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_MSC_H */
