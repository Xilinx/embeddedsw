/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 * @file xusb_cp9.h
 *
 * This file contains the constants, typedefs, variables and functions
 * prototypes related to the USB chapter 9 related code.
 *
 * @note     None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- ------------------------------------------------------------------
 * 1.00a hvm  2/22/07 First release
 * 1.01a hvm  5/30/07 Moved the USB class specific command processing to
 *		      application files.
 * 4.02a bss  4/5/12  Made changes so that the flag __LITTLE_ENDIAN__ gets
 *		      defined when compiled with ARM CodeSourcery toolchain.
 * </pre>
 *****************************************************************************/

#ifndef XUSB_CH9_H
#define XUSB_CH9_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files **********************************/

#include "xusb.h"
#include "xusb_types.h"

#if defined(__ARMEL__) || defined(__aarch64__)
#ifndef __LITTLE_ENDIAN__
#define __LITTLE_ENDIAN__
#endif
#endif

/**************************** Type Definitions ********************************/

/*
 * Structure 32 bit int memory access to the Dual Port RAM.
 */
typedef union {
	u32 Word;
	struct {
		u8 Zero;
		u8 One;
		u8 Two;
		u8 Three;
	} Byte;
} IntChar;

/************************** Function Prototypes *******************************/

void EP0ProcessOutToken(XUsb * InstancePtr);
void EP0ProcessInToken(XUsb * InstancePtr);
int Chapter9(XUsb * InstancePtr);
int ExecuteCommand(XUsb * InstancePtr);
void GetInterface(XUsb * InstancePtr);
void SetInterface(XUsb * InstancePtr);
void SetupControlWriteStatusStage(XUsb * InstancePtr);
void GetStatus(XUsb * InstancePtr);
void GetDescriptor(XUsb * InstancePtr);
void SetDescriptor(XUsb * InstancePtr);
void GetConfiguration(XUsb * InstancePtr);
void SetConfiguration(XUsb * InstancePtr);
void SetClearFeature(XUsb * InstancePtr, int flag);
void LoadEP0(XUsb * InstancePtr);
extern void InitUsbInterface(XUsb * InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CH9_H */
