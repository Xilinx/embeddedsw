/******************************************************************************
*
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* (c) Copyright 2007-2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
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
