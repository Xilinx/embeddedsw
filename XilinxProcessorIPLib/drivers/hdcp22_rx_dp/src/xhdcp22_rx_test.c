/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xhdcp22_rx_test.c
* @addtogroup hdcp22_rx_v2_2
* @{
* @details
*
* This file contains the implementation of the test framework used to
* perform standalone receiver testing and software loopback testing
* for the Xilinx HDCP 2.2 Receiver. The test vectors and keys contained
* in this file are from the Errata to HDCP on HDMI Specification
* Revision 2.2, Feburary 09, 2015.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  JB   02/19/19 First Release.
*</pre>
*
*****************************************************************************/
#ifdef _XHDCP22_RX_TEST_

/***************************** Include Files ********************************/
#include "string.h"
#include "stdio.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xtmrctr.h"
#include "xhdcp22_rx.h"
#include "xhdcp22_rx_i.h"

/************************** Constant Definitions ****************************/
/** Device address for HDCP */
#define XHDCP22_RX_TEST_DDC_BASE_ADDRESS	(0x74>>1)
/** Maximum number of iterations before test timeout */
#define XHDCP22_RX_TEST_MAX_ITERATIONS		100
#define XHDCP22_RX_TEST_RECEIVER_1          0
#define XHDCP22_RX_TEST_RECEIVER_2          1

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/
#define XHdcp22Rx_Printf(type, ...) (((type) & TRUE) ? printf (__VA_ARGS__) : 0)

/************************** Variable Definitions ****************************/

/** This variable is the test transmitter capabilities TxCaps  */
static const u8 XHdcp22_Rx_Test_TxCaps[2][3] =
{
	//********** R1 **********//
	{0x02, 0x00, 0x00},
	//********** R2 **********//
	{0x02, 0x00, 0x00}
};

/** This variable is the test receiver capabilities RxCaps */
static const u8 XHdcp22_Rx_Test_RxCaps[2][3] =
{
	//********** R1 **********//
	{0x02, 0x00, 0x01},
	//********** R2 **********//
	{0x02, 0x00, 0x00}
};

/** This variable is the test certificate */
static const u8 XHdcp22_Rx_Test_PublicCert[2][522] =
{
	//********** R1 **********//
	{0x74, 0x5b, 0xb8, 0xbd, 0x04, 0xaf, 0xb5, 0xc5, 0xc6, 0x7b, 0xc5, 0x3a, 0x34, 0x90,
	 0xa9, 0x54, 0xc0, 0x8f, 0xb7, 0xeb, 0xa1, 0x54, 0xd2, 0x4f, 0x22, 0xde, 0x83, 0xf5,
	 0x03, 0xa6, 0xc6, 0x68, 0x46, 0x9b, 0xc0, 0xb8, 0xc8, 0x6c, 0xdb, 0x26, 0xf9, 0x3c,
	 0x49, 0x2f, 0x02, 0xe1, 0x71, 0xdf, 0x4e, 0xf3, 0x0e, 0xc8, 0xbf, 0x22, 0x9d, 0x04,
	 0xcf, 0xbf, 0xa9, 0x0d, 0xff, 0x68, 0xab, 0x05, 0x6f, 0x1f, 0x12, 0x8a, 0x68, 0x62,
	 0xeb, 0xfe, 0xc9, 0xea, 0x9f, 0xa7, 0xfb, 0x8c, 0xba, 0xb1, 0xbd, 0x65, 0xac, 0x35,
	 0x9c, 0xa0, 0x33, 0xb1, 0xdd, 0xa6, 0x05, 0x36, 0xaf, 0x00, 0xa2, 0x7f, 0xbc, 0x07,
	 0xb2, 0xdd, 0xb5, 0xcc, 0x57, 0x5c, 0xdc, 0xc0, 0x95, 0x50, 0xe5, 0xff, 0x1f, 0x20,
	 0xdb, 0x59, 0x46, 0xfa, 0x47, 0xc4, 0xed, 0x12, 0x2e, 0x9e, 0x22, 0xbd, 0x95, 0xa9,
	 0x85, 0x59, 0xa1, 0x59, 0x3c, 0xc7, 0x83, 0x01, 0x00, 0x01, 0x10, 0x00, 0x0b, 0xa3,
	 0x73, 0x77, 0xdd, 0x03, 0x18, 0x03, 0x8a, 0x91, 0x63, 0x29, 0x1e, 0xa2, 0x95, 0x74,
	 0x42, 0x90, 0x78, 0xd0, 0x67, 0x25, 0xb6, 0x32, 0x2f, 0xcc, 0x23, 0x2b, 0xad, 0x21,
	 0x39, 0x3d, 0x14, 0xba, 0x37, 0xa3, 0x65, 0x14, 0x6b, 0x9c, 0xcf, 0x61, 0x20, 0x44,
	 0xa1, 0x07, 0xbb, 0xcf, 0xc3, 0x4e, 0x95, 0x5b, 0x10, 0xcf, 0xc7, 0x6f, 0xf1, 0xc3,
	 0x53, 0x7c, 0x63, 0xa1, 0x8c, 0xb2, 0xe8, 0xab, 0x2e, 0x96, 0x97, 0xc3, 0x83, 0x99,
	 0x70, 0xd3, 0xdc, 0x21, 0x41, 0xf6, 0x0a, 0xd1, 0x1a, 0xee, 0xf4, 0xcc, 0xeb, 0xfb,
	 0xa6, 0xaa, 0xb6, 0x9a, 0xaf, 0x1d, 0x16, 0x5e, 0xe2, 0x83, 0xa0, 0x4a, 0x41, 0xf6,
	 0x7b, 0x07, 0xbf, 0x47, 0x85, 0x28, 0x6c, 0xa0, 0x77, 0xa6, 0xa3, 0xd7, 0x85, 0xa5,
	 0xc4, 0xa7, 0xe7, 0x6e, 0xb5, 0x1f, 0x40, 0x72, 0x97, 0xfe, 0xc4, 0x81, 0x23, 0xa0,
	 0xc2, 0x90, 0xb3, 0x49, 0x24, 0xf5, 0xb7, 0x90, 0x2c, 0xbf, 0xfe, 0x04, 0x2e, 0x00,
	 0xa9, 0x5f, 0x86, 0x04, 0xca, 0xc5, 0x3a, 0xcc, 0x26, 0xd9, 0x39, 0x7e, 0xa9, 0x2d,
	 0x28, 0x6d, 0xc0, 0xcc, 0x6e, 0x81, 0x9f, 0xb9, 0xb7, 0x11, 0x33, 0x32, 0x23, 0x47,
	 0x98, 0x43, 0x0d, 0xa5, 0x1c, 0x59, 0xf3, 0xcd, 0xd2, 0x4a, 0xb7, 0x3e, 0x69, 0xd9,
	 0x21, 0x53, 0x9a, 0xf2, 0x6e, 0x77, 0x62, 0xae, 0x50, 0xda, 0x85, 0xc6, 0xaa, 0xc4,
	 0xb5, 0x1c, 0xcd, 0xa8, 0xa5, 0xdd, 0x6e, 0x62, 0x73, 0xff, 0x5f, 0x7b, 0xd7, 0x3c,
	 0x17, 0xba, 0x47, 0x0c, 0x89, 0x0e, 0x62, 0x79, 0x43, 0x94, 0xaa, 0xa8, 0x47, 0xf4,
	 0x4c, 0x38, 0x89, 0xa8, 0x81, 0xad, 0x23, 0x13, 0x27, 0x0c, 0x17, 0xcf, 0x3d, 0x83,
	 0x84, 0x57, 0x36, 0xe7, 0x22, 0x26, 0x2e, 0x76, 0xfd, 0x56, 0x80, 0x83, 0xf6, 0x70,
	 0xd4, 0x5c, 0x91, 0x48, 0x84, 0x7b, 0x18, 0xdb, 0x0e, 0x15, 0x3b, 0x49, 0x26, 0x23,
	 0xe6, 0xa3, 0xe2, 0xc6, 0x3a, 0x23, 0x57, 0x66, 0xb0, 0x72, 0xb8, 0x12, 0x17, 0x4f,
	 0x86, 0xfe, 0x48, 0x0d, 0x53, 0xea, 0xfe, 0x31, 0x48, 0x7d, 0x86, 0xde, 0xeb, 0x82,
	 0x86, 0x1e, 0x62, 0x03, 0x98, 0x59, 0x00, 0x37, 0xeb, 0x61, 0xe9, 0xf9, 0x7a, 0x40,
	 0x78, 0x1c, 0xba, 0xbc, 0x0b, 0x88, 0xfb, 0xfd, 0x9d, 0xd5, 0x01, 0x11, 0x94, 0xe0,
	 0x35, 0xbe, 0x33, 0xe8, 0xe5, 0x36, 0xfb, 0x9c, 0x45, 0xcb, 0x75, 0xaf, 0xd6, 0x35,
	 0xff, 0x78, 0x92, 0x7f, 0xa1, 0x7c, 0xa8, 0xfc, 0xb7, 0xf7, 0xa8, 0x52, 0xa9, 0xc6,
	 0x84, 0x72, 0x3d, 0x1c, 0xc9, 0xdf, 0x35, 0xc6, 0xe6, 0x00, 0xe1, 0x48, 0x72, 0xce,
	 0x83, 0x1b, 0xcc, 0xf8, 0x33, 0x2d, 0x4f, 0x98, 0x75, 0x00, 0x3c, 0x41, 0xdf, 0x7a,
	 0xed, 0x38, 0x53, 0xb1},
	//********** R2 **********//
	{0x8b, 0xa4, 0x47, 0x42, 0xfb, 0xe4, 0x68, 0x63, 0x8a, 0xda, 0x97, 0x2d, 0xde, 0x9a, 0x8d,
	 0x1c, 0xb1, 0x65, 0x4b, 0x85, 0x8d, 0xe5, 0x46, 0xd6, 0xdb, 0x95, 0xa5, 0xf6, 0x66, 0x74,
	 0xea, 0x81, 0x0b, 0x9a, 0x58, 0x58, 0x66, 0x26, 0x86, 0xa6, 0xb4, 0x56, 0x2b, 0x29, 0x43,
	 0xe5, 0xbb, 0x81, 0x74, 0x86, 0xa7, 0xb7, 0x16, 0x2f, 0x07, 0xec, 0xd1, 0xb5, 0xf9, 0xae,
	 0x4f, 0x98, 0x89, 0xa9, 0x91, 0x7d, 0x58, 0x5b, 0x8d, 0x20, 0xd5, 0xc5, 0x08, 0x40, 0x3b,
	 0x86, 0xaf, 0xf4, 0xd6, 0xb9, 0x20, 0x95, 0xe8, 0x90, 0x3b, 0x8f, 0x9f, 0x36, 0x5b, 0x46,
	 0xb6, 0xd4, 0x1e, 0xf5, 0x05, 0x88, 0x80, 0x14, 0xe7, 0x2c, 0x77, 0x5d, 0x6e, 0x54, 0xe9,
	 0x65, 0x81, 0x5a, 0x68, 0x92, 0xa5, 0xd6, 0x40, 0x78, 0x11, 0x97, 0x65, 0xd7, 0x64, 0x36,
	 0x5e, 0x8d, 0x2a, 0x87, 0xa8, 0xeb, 0x7d, 0x06, 0x2c, 0x10, 0xf8, 0x0a, 0x7d, 0x01, 0x00,
	 0x01, 0x10, 0x00, 0x06, 0x40, 0x99, 0x8f, 0x5a, 0x54, 0x71, 0x23, 0xa7, 0x6a, 0x64, 0x3f,
	 0xbd, 0xdd, 0x52, 0xb2, 0x79, 0x6f, 0x88, 0x26, 0x94, 0x9e, 0xaf, 0xa4, 0xde, 0x7d, 0x8d,
	 0x88, 0x10, 0xc8, 0xf6, 0x56, 0xf0, 0x8f, 0x46, 0x28, 0x48, 0x55, 0x51, 0xc5, 0xaf, 0xa1,
	 0xa9, 0x9d, 0xac, 0x9f, 0xb1, 0x26, 0x4b, 0xeb, 0x39, 0xad, 0x88, 0x46, 0xaf, 0xbc, 0x61,
	 0xa8, 0x7b, 0xf9, 0x7b, 0x3e, 0xe4, 0x95, 0xd9, 0xa8, 0x79, 0x48, 0x51, 0x00, 0xbe, 0xa4,
	 0xb6, 0x96, 0x7f, 0x3d, 0xfd, 0x76, 0xa6, 0xb7, 0xbb, 0xb9, 0x77, 0xdc, 0x54, 0xfb, 0x52,
	 0x9c, 0x79, 0x8f, 0xed, 0xd4, 0xb1, 0xbc, 0x0f, 0x7e, 0xb1, 0x7e, 0x70, 0x6d, 0xfc, 0xb9,
	 0x7e, 0x66, 0x9a, 0x86, 0x23, 0x3a, 0x98, 0x5e, 0x32, 0x8d, 0x75, 0x18, 0x54, 0x64, 0x36,
	 0xdd, 0x92, 0x01, 0x39, 0x90, 0xb9, 0xe3, 0xaf, 0x6f, 0x98, 0xa5, 0xc0, 0x80, 0xc6, 0x2f,
	 0xa1, 0x02, 0xad, 0x8d, 0xf4, 0xd6, 0x66, 0x7b, 0x45, 0xe5, 0x74, 0x18, 0xb1, 0x27, 0x24,
	 0x01, 0x1e, 0xea, 0xd8, 0xf3, 0x79, 0x92, 0xe9, 0x03, 0xf5, 0x57, 0x8d, 0x65, 0x2a, 0x8d,
	 0x1b, 0xf0, 0xda, 0x58, 0x3f, 0x58, 0xa0, 0xf4, 0xb4, 0xbe, 0xcb, 0x21, 0x66, 0xe9, 0x21,
	 0x7c, 0x76, 0xf3, 0xc1, 0x7e, 0x2e, 0x7c, 0x3d, 0x61, 0x20, 0x1d, 0xc5, 0xc0, 0x71, 0x28,
	 0x2e, 0xb7, 0x0f, 0x1f, 0x7a, 0xc1, 0xd3, 0x6a, 0x1e, 0xa3, 0x54, 0x34, 0x8e, 0x0d, 0xd7,
	 0x96, 0x93, 0x78, 0x50, 0xc1, 0xee, 0x27, 0x72, 0x3a, 0xbd, 0x57, 0x22, 0xf0, 0xd7, 0x6d,
	 0x9d, 0x65, 0xc4, 0x07, 0x9c, 0x82, 0xa6, 0xd4, 0xf7, 0x6b, 0x9a, 0xe9, 0xc0, 0x6c, 0x4a,
	 0x4f, 0x6f, 0xbe, 0x8e, 0x01, 0x37, 0x50, 0x3a, 0x66, 0xd9, 0xe9, 0xd9, 0xf9, 0x06, 0x9e,
	 0x00, 0xa9, 0x84, 0xa0, 0x18, 0xb3, 0x44, 0x21, 0x24, 0xa3, 0x6c, 0xcd, 0xb7, 0x0f, 0x31,
	 0x2a, 0xe8, 0x15, 0xb6, 0x93, 0x6f, 0xb9, 0x86, 0xe5, 0x28, 0x01, 0x1a, 0x5e, 0x10, 0x3f,
	 0x1f, 0x4d, 0x35, 0xa2, 0x8d, 0xb8, 0x54, 0x26, 0x68, 0x3a, 0xcd, 0xcb, 0x5f, 0xfa, 0x37,
	 0x4a, 0x60, 0x10, 0xb1, 0x0a, 0xfe, 0xba, 0x9b, 0x96, 0x5d, 0x7e, 0x99, 0xcf, 0x01, 0x98,
	 0x65, 0x87, 0xad, 0x40, 0xd5, 0x82, 0x1d, 0x61, 0x54, 0xa2, 0xd3, 0x16, 0x3e, 0xf7, 0xe3,
	 0x05, 0x89, 0x8d, 0x8a, 0x50, 0x87, 0x47, 0xbe, 0x29, 0x18, 0x01, 0xb7, 0xc3, 0xdd, 0x43,
	 0x23, 0x7a, 0xcd, 0x85, 0x1d, 0x4e, 0xa9, 0xc0, 0x1a, 0xa4, 0x77, 0xab, 0xe7, 0x31, 0x9a,
	 0x33, 0x1b, 0x7a, 0x86, 0xe1, 0xe5, 0xca, 0x0c, 0x43, 0x1a, 0xfa, 0xec, 0x4c, 0x05, 0xc6,
	 0xd1, 0x43, 0x12, 0xf9, 0x4d, 0x3e, 0xf7, 0xd6, 0x05, 0x9c, 0x1c, 0xdd}
};

/** This variable is the test private key */
static const u8 XHdcp22_Rx_Test_PrivateKey[2][320] =
{
	//********** R1 **********//
	{/* P */
	 0xec, 0xbe, 0xe5, 0x5b, 0x9e, 0x7a, 0x50, 0x8a, 0x96, 0x80, 0xc8, 0xdb, 0xb0, 0xed, 0x44,
	 0xf2, 0xba, 0x1d, 0x5d, 0x80, 0xc1, 0xc8, 0xb3, 0xc2, 0x74, 0xde, 0xee, 0x28, 0xec, 0xdc,
	 0x78, 0xc8, 0x67, 0x53, 0x07, 0xf2, 0xf8, 0x75, 0x9c, 0x4c, 0xa5, 0x6c, 0x48, 0x94, 0xc8,
	 0xeb, 0xad, 0xd7, 0x7d, 0xd2, 0xea, 0xdf, 0x74, 0x20, 0x62, 0xc9, 0x81, 0xa8, 0x3c, 0x36,
	 0xb9, 0xea, 0x40, 0xfd,
	 /* Q */
	 0xbe, 0x00, 0x19, 0x76, 0xc6, 0xb4, 0xba, 0x19, 0xd4, 0x69, 0xfa, 0x4d, 0xe2, 0xf8, 0x30,
	 0x27, 0x36, 0x2b, 0x4c, 0xc4, 0x34, 0xab, 0xd3, 0xd9, 0x8c, 0xd6, 0xb8, 0x0d, 0x37, 0x5e,
	 0x59, 0x4b, 0x76, 0x70, 0x68, 0x2b, 0x1f, 0x4c, 0x3d, 0x47, 0x5f, 0xa5, 0xb1, 0xcd, 0x74,
	 0x56, 0x88, 0xfe, 0x7c, 0xf8, 0x3b, 0x30, 0x6f, 0xfd, 0xc3, 0xed, 0x87, 0x3c, 0xa1, 0x53,
	 0x84, 0xc3, 0xd2, 0x7f,
	 /* DP, d*mod(p-1)*/
	 0x60, 0x71, 0x9b, 0xe9, 0xe8, 0xf3, 0x97, 0x1f, 0xfe, 0x13, 0xd4, 0xbf, 0x7a, 0xa2, 0x0d,
	 0xf6, 0x7b, 0xcf, 0x3e, 0xaa, 0x17, 0x47, 0x75, 0xc3, 0x7f, 0xec, 0xd9, 0x44, 0x9e, 0xc9,
	 0x6a, 0x02, 0xe9, 0xe4, 0xaf, 0x56, 0x51, 0xd5, 0x47, 0xa9, 0x09, 0xb2, 0xc5, 0x16, 0xa7,
	 0x8b, 0x2b, 0x34, 0xa0, 0x33, 0x6e, 0x2f, 0x3d, 0x95, 0x7b, 0xe8, 0xef, 0x02, 0xe4, 0x14,
	 0xbf, 0x44, 0x28, 0xd9,
	 /* DQ, d*mod(q-1) */
	 0x10, 0x0e, 0x2e, 0x18, 0xad, 0x5d, 0xe4, 0x43, 0xfe, 0x81, 0x1e, 0x17, 0xaa, 0xd0, 0x52,
	 0x31, 0x5e, 0x10, 0x76, 0xa2, 0x35, 0xd9, 0x37, 0x43, 0xb0, 0xf5, 0x0c, 0x04, 0x81, 0xe3,
	 0x45, 0x24, 0x6d, 0x53, 0xbe, 0x59, 0xb6, 0x81, 0x58, 0xc4, 0x49, 0x3e, 0xd5, 0x31, 0x89,
	 0x5d, 0x2e, 0xa2, 0x62, 0xa9, 0x0f, 0x47, 0x5e, 0x8f, 0x51, 0x19, 0x27, 0x4e, 0x66, 0x4b,
	 0x8a, 0x72, 0x89, 0xbd,
	 /* QINV, (q^-1)*mod(p) */
	 0x3e, 0x53, 0x0a, 0xf4, 0x8e, 0x75, 0xe1, 0x52, 0xc6, 0x24, 0xe9, 0xf7, 0xbb, 0xac, 0x3f,
	 0x22, 0x5f, 0xe8, 0xe0, 0x79, 0x35, 0xff, 0x91, 0xee, 0x22, 0x56, 0xd2, 0x00, 0x68, 0x32,
	 0xc4, 0xe1, 0x5f, 0xff, 0xf8, 0xb1, 0x1d, 0xee, 0xdc, 0x57, 0x81, 0xd1, 0xab, 0x8b, 0x37,
	 0x22, 0xe3, 0x9f, 0xd0, 0xa1, 0xc1, 0xce, 0x1d, 0xd0, 0x24, 0x23, 0xa0, 0x0e, 0xf7, 0xa6,
	 0xdb, 0xa3, 0xea, 0xd3},
	//********** R2 **********//
	{/* P */
	 0xf5, 0xf6, 0xfa, 0x44, 0xa2, 0x16, 0x2f, 0xa7, 0x1f, 0x7f, 0x16, 0x05, 0x99, 0x26, 0xc4,
	 0x1b, 0x80, 0x7f, 0xfa, 0x52, 0x4e, 0x3e, 0xaa, 0x3d, 0x1e, 0xb0, 0xf1, 0x9a, 0xc6, 0x3d,
	 0x8f, 0x57, 0x2b, 0x9e, 0xcd, 0xe8, 0x03, 0xd6, 0xf3, 0x91, 0x75, 0xe2, 0x19, 0x44, 0x9e,
	 0x11, 0x58, 0x5f, 0xd6, 0x88, 0x7c, 0xc4, 0xc1, 0x5b, 0x45, 0x9b, 0x84, 0xcf, 0x72, 0x1d,
	 0x35, 0xbf, 0x24, 0xd5,
	 /* Q */
	 0xed, 0xba, 0x08, 0xbf, 0x42, 0x2c, 0x0e, 0xfa, 0x3a, 0xc4, 0xd2, 0xc7, 0x01, 0x51, 0x25,
	 0xae, 0xb0, 0xa1, 0xcc, 0xdb, 0x67, 0x9b, 0xaa, 0x50, 0xf0, 0x80, 0xac, 0x4b, 0x9f, 0x5c,
	 0xba, 0x1e, 0xf4, 0x7f, 0xa9, 0xb3, 0x21, 0x8b, 0x62, 0x2c, 0x36, 0xda, 0xcd, 0xa7, 0x4d,
	 0xa4, 0xd6, 0x44, 0xed, 0xb1, 0x34, 0xe7, 0x69, 0x10, 0x77, 0x5a, 0x6a, 0xff, 0xf5, 0x63,
	 0x8a, 0x2c, 0x43, 0x09,
	 /* DP, d*mod(p-1)*/
	 0x61, 0x5a, 0xc4, 0x6c, 0x6e, 0x0b, 0x82, 0x09, 0x10, 0x3a, 0x69, 0x29, 0x06, 0x19, 0x85,
	 0xfd, 0xac, 0xba, 0xfb, 0x05, 0xa0, 0xda, 0xc4, 0xdf, 0x34, 0x4a, 0xad, 0x16, 0xa9, 0xe8,
	 0xab, 0xd7, 0xc0, 0xf8, 0x36, 0x5f, 0xe3, 0x45, 0x2d, 0x5b, 0x21, 0xe1, 0xc0, 0x46, 0x9c,
	 0x9a, 0x18, 0xf4, 0xb6, 0x21, 0x87, 0xe1, 0x08, 0xf7, 0x6b, 0x71, 0xc6, 0xfb, 0xa5, 0x1b,
	 0x52, 0xae, 0xb9, 0x91,
	 /* DQ, d*mod(q-1) */
	 0x5a, 0x83, 0x7f, 0xbb, 0x1a, 0xbd, 0xdd, 0xc2, 0x06, 0xc8, 0x54, 0x1c, 0xb3, 0x72, 0xab,
	 0x2f, 0x55, 0x4f, 0x75, 0xc9, 0x80, 0x2c, 0x73, 0xef, 0xb7, 0x72, 0xb6, 0xa7, 0x60, 0x79,
	 0x14, 0xe0, 0x9e, 0x65, 0x51, 0x3e, 0xc4, 0x21, 0xe6, 0xf2, 0x40, 0xbc, 0x94, 0x9b, 0x03,
	 0xe4, 0x24, 0x35, 0x40, 0x6f, 0x3d, 0x5e, 0x72, 0xd1, 0x73, 0x30, 0x39, 0x17, 0x55, 0xde,
	 0x5d, 0x88, 0xb6, 0xc9,
	 /* QINV, (q^-1)*mod(p) */
	 0xbc, 0x91, 0x2a, 0x93, 0x6a, 0x8d, 0x24, 0x3c, 0xd5, 0x7d, 0x12, 0x3b, 0xa3, 0x71, 0xc7,
	 0x3a, 0xf0, 0x64, 0x72, 0x50, 0x7e, 0x18, 0x71, 0xe1, 0xb4, 0x3b, 0x1e, 0xfc, 0x38, 0xca,
	 0xe6, 0x8c, 0x16, 0x51, 0x97, 0xd6, 0x3f, 0x04, 0xee, 0x23, 0x8b, 0x45, 0x0c, 0x4b, 0x98,
	 0x36, 0x18, 0x27, 0x29, 0x1b, 0x4d, 0x73, 0x7e, 0xe8, 0xb0, 0x1a, 0xc7, 0xfb, 0x5c, 0xea,
	 0x78, 0xd0, 0x6e, 0x97}
};

/** This variable is the test global constant Lc128 */
static const u8 XHdcp22_Rx_Test_Lc128[2][16] =
{
	//********** R1 **********//
	{0x93, 0xce, 0x5a, 0x56, 0xa0, 0xa1, 0xf4, 0xf7, 0x3c, 0x65, 0x8a, 0x1b, 0xd2, 0xae, 0xf0, 0xf7},
	//********** R2 **********//
	{0x93, 0xce, 0x5a, 0x56, 0xa0, 0xa1, 0xf4, 0xf7, 0x3c, 0x65, 0x8a, 0x1b, 0xd2, 0xae, 0xf0, 0xf7}
};

/** This variable is the test transmitter random value Rtx */
static const u8 XHdcp22_Rx_Test_Rtx[2][8] =
{
	//********** R1 **********//
	{0x18, 0xfa, 0xe4, 0x20, 0x6a, 0xfb, 0x51, 0x49},
	//********** R2 **********//
	{0xf9, 0xf1, 0x30, 0xa8, 0x2d, 0x5b, 0xe5, 0xc3}
};

/** This variable is the test receiver random value Rrx */
const u8 XHdcp22_Rx_Test_Rrx[2][8] =
{
	//********** R1 **********//
	{0x3b, 0xa0, 0xbe, 0xde, 0x0c, 0x46, 0xa9, 0x91},
	//********** R2 **********//
	{0xe1, 0x7a, 0xb0, 0xfd, 0x0f, 0x54, 0x40, 0x52}
};

/** This variable is the test master key Km */
static const u8 XHdcp22_Rx_Test_Km[2][16] =
{
	//********** R1 **********//
	{0x68, 0xbc, 0xc5, 0x1b, 0xa9, 0xdb, 0x1b, 0xd0, 0xfa, 0xf1, 0x5e, 0x9a, 0xd8, 0xa5, 0xaf, 0xb9},
	//********** R2 **********//
	{0xca, 0x9f, 0x83, 0x95, 0x70, 0xd0, 0xd0, 0xf9, 0xcf, 0xe4, 0xeb, 0x54, 0x7e, 0x09, 0xfa, 0x3b}
};

/** This variable is the test encrypted master key EkpubKm */
static const u8 XHdcp22_Rx_Test_Ekm[2][128] =
{
	//********** R1 **********//
	{0x9b, 0x9f, 0x80, 0x19, 0xad, 0x0e,
	 0xa2, 0xf0, 0xdd, 0xa0, 0x29, 0x33,
	 0xd9, 0x6d, 0x1c, 0x77, 0x31, 0x37,
	 0x57, 0xe0, 0xe5, 0xb2, 0xbd, 0xdd,
	 0x36, 0x3e, 0x38, 0x4e, 0x7d, 0x40,
	 0x78, 0x66, 0x97, 0x7a, 0x4c, 0xce,
	 0xc5, 0xc7, 0x5d, 0x01, 0x57, 0x26,
	 0xcc, 0xa2, 0xf6, 0xde, 0x34, 0xdd,
	 0x29, 0xbe, 0x5e, 0x31, 0xe8, 0xf1,
	 0x34, 0xe8, 0x1a, 0x63, 0xa3, 0x6d,
	 0x46, 0xdc, 0x0a, 0x06, 0x08, 0x99,
	 0x9d, 0xdb, 0x3c, 0xa2, 0x9c, 0x04,
	 0xdd, 0x4e, 0xd9, 0x02, 0x7d, 0x20,
	 0x54, 0xec, 0xca, 0x86, 0x42, 0x1b,
	 0x18, 0xda, 0x30, 0x9c, 0xc4, 0xcb,
	 0xac, 0xb4, 0x54, 0xde, 0x84, 0x68,
	 0x71, 0x53, 0x6d, 0x92, 0x17, 0xca,
	 0x08, 0x8a, 0x7a, 0xf9, 0x98, 0x9a,
	 0xb6, 0x7b, 0x22, 0x92, 0xac, 0x7d,
	 0x0d, 0x6b, 0xd6, 0x7f, 0x31, 0xab,
	 0xf0, 0x10, 0xc5, 0x2a, 0x0f, 0x6d,
	 0x27, 0xa0},
	//********** R2 **********//
	{0xa8, 0x55, 0xc2, 0xc4, 0xc6, 0xbe,
	 0xef, 0xcd, 0xcb, 0x9f, 0xe3, 0x9f,
	 0x2a, 0xb7, 0x29, 0x76, 0xfe, 0xd8,
	 0xda, 0xc9, 0x38, 0xfa, 0x39, 0xf0,
	 0xab, 0xca, 0x8a, 0xed, 0x95, 0x7b,
	 0x93, 0xb2, 0xdf, 0xd0, 0x7d, 0x09,
	 0x9d, 0x05, 0x96, 0x66, 0x03, 0x6e,
	 0xba, 0xe0, 0x63, 0x0f, 0x30, 0x77,
	 0xc2, 0xbb, 0xe2, 0x11, 0x39, 0xe5,
	 0x27, 0x78, 0xee, 0x64, 0xf2, 0x85,
	 0x36, 0x57, 0xc3, 0x39, 0xd2, 0x7b,
	 0x79, 0x03, 0xb7, 0xcc, 0x82, 0xcb,
	 0xf0, 0x62, 0x82, 0x43, 0x38, 0x09,
	 0x9b, 0x71, 0xaa, 0x38, 0xa6, 0x3f,
	 0x48, 0x12, 0x6d, 0x8c, 0x5e, 0x07,
	 0x90, 0x76, 0xac, 0x90, 0x99, 0x51,
	 0x5b, 0x06, 0xa5, 0xfa, 0x50, 0xe4,
	 0xf9, 0x25, 0xc3, 0x07, 0x12, 0x37,
	 0x64, 0x92, 0xd7, 0xdb, 0xd3, 0x34,
	 0x1c, 0xe4, 0xfa, 0xdd, 0x09, 0xe6,
	 0x28, 0x3d, 0x0c, 0xad, 0xa9, 0xd8,
	 0xe1, 0xb5}
};

/** This variable is the test HPrime */
static const u8 XHdcp22_Rx_Test_HPrime[2][32] =
{
	//********** R1 **********//
	{0x69, 0xe0, 0xab, 0x21, 0x2f, 0xdb,
	 0x57, 0xe6, 0x7e, 0xfc, 0x43, 0x76,
	 0x1a, 0x2c, 0x5c, 0xce, 0x76, 0xc3,
	 0x65, 0xf1, 0x9b, 0x75, 0xc3, 0xea,
	 0xc2, 0xd2, 0x77, 0xdd, 0x5c, 0x7e,
	 0x4a, 0xc4},
	//********** R2 **********//
	{0x4f, 0xf1, 0xa2 ,0xa5 ,0x61 ,0x67,
	 0xc8, 0xe0, 0xad ,0x16 ,0xc8 ,0x95,
	 0x99, 0x1b, 0x1a ,0x21 ,0xa8 ,0x80,
	 0xc6, 0x27, 0x39 ,0x3f ,0xc7 ,0xbb,
	 0x83, 0xed, 0xa7 ,0xe5 ,0x69 ,0x07,
	 0xa5, 0xdc}
};

/** This variable is the test Pairing Ekh(Km) */
static const u8 XHdcp22_Rx_Test_EKh[2][16] =
{
	//********** R1 **********//
	{0xb8, 0x9f, 0xf9, 0x72, 0x6a, 0x6f,
	 0x2c, 0x1e, 0x29, 0xb6, 0x44, 0x8d,
	 0xdc, 0xa3, 0x10, 0xbd},
	//********** R2 **********//
	{0xe6, 0x57, 0x8e, 0xbc, 0xc7, 0x68,
	 0x44, 0x87, 0x88, 0x8a, 0x9b, 0xd7,
	 0xd6, 0xae, 0x38, 0xbe}
};

/** This variable is the test locality check nonce Rn */
static const u8 XHdcp22_Rx_Test_Rn[2][8] =
{
	//********** R1 **********//
	{0x32, 0x75, 0x3e, 0xa8, 0x78, 0xa6,
	 0x38, 0x1c},
	//********** R2 **********//
	{0xa0, 0xfe, 0x9b, 0xb8, 0x20, 0x60,
	 0x58, 0xca}
};

/** This variable is the test locality check LPrime */
static const u8 XHdcp22_Rx_Test_LPrime[2][32] =
{
	//********** R1 **********//
	{0xbc, 0x20, 0x92, 0x33, 0x54, 0x91,
	 0xc1, 0x9e, 0xa4, 0xde, 0x8b, 0x30,
	 0x49, 0xc2, 0x06, 0x6a, 0xd8, 0x11,
	 0xa2, 0x2a, 0xb1, 0x46, 0xdf, 0x74,
	 0x58, 0x47, 0x05, 0xa8, 0xb7, 0x67,
	 0xfb, 0xdd},
	//********** R2 **********//
	{0xf2, 0x0f, 0x13, 0x6e, 0x85, 0x53,
	 0xc1, 0x0c, 0xd3, 0xdd, 0xb2, 0xf9,
	 0x6d, 0x33, 0x31, 0xf9, 0xcb, 0x6e,
	 0x97, 0x8c, 0xcd, 0x5e, 0xda, 0x13,
	 0xdd, 0xea, 0x41, 0x44, 0x10, 0x9b,
	 0x51, 0xb0}
};

/** This variable is the test encrypted session key Edkey(Ks) */
static const u8 XHdcp22_Rx_Test_EKs[2][16] =
{
	//********** R1 **********//
	{0x4c, 0x32, 0x47, 0x12, 0xc4, 0xbe,
	 0xc6, 0x69, 0x0a, 0xc2, 0x19, 0x64,
	 0xde, 0x91, 0xf1, 0x83},
	//********** R2 **********//
	{0xb6, 0x8b, 0x8a, 0xa4, 0xd2, 0xcb,
	 0xba, 0xff, 0x53, 0x33, 0xc1, 0xd9,
	 0xbb, 0xb7, 0x10, 0xa9}
};

/** This variable is the test session key Ks */
static const u8 XHdcp22_Rx_Test_Ks[2][16] =
{
	//********** R1 **********//
	{0xf3, 0xdf, 0x1d, 0xd9, 0x57, 0x96,
	 0x12, 0x3f, 0x98, 0x97, 0x89, 0xb4,
	 0x21, 0xe1, 0x2d, 0xe1},
	//********** R2 **********//
	{0xf3, 0xdf, 0x1d, 0xd9, 0x57, 0x96,
	 0x12, 0x3f, 0x98, 0x97, 0x89, 0xb4,
	 0x21, 0xe1, 0x2d, 0xe1}
};

/** This variable is the test session key Riv */
static const u8 XHdcp22_Rx_Test_Riv[2][8] =
{
	//********** R1 **********//
	{0x40, 0x2b, 0x6b, 0x43, 0xc5, 0xe8,
	 0x86, 0xd8},
	//********** R2 **********//
	{0x9a, 0x6d, 0x11, 0x00, 0xa9, 0xb7,
	 0x6f, 0x64}
};

/** This variable is the test repeater topology Receiver ID List */
static const u8 XHdcp22_Rx_Test_Repeater_ReceiverIdList[] =
{
	//********** R1  **********//
	0x47, 0x8e, 0x71, 0xe2, 0x0f, // Receiver ID 0
	0x35, 0x79, 0x6a, 0x17, 0x0e, // Receiver ID 1
	0x74, 0xe8, 0x53, 0x97, 0xa2  // Receiver ID 2
};

/** This variable is the test repeater topology V */
static const u8 XHdcp22_Rx_Test_Repeater_V[] =
{
	//********** R1  **********//
	0x63, 0x6d, 0xc5, 0x08, 0x4d, 0x6c,
	0xb1, 0x0e, 0x93, 0xa5, 0x28, 0x67,
	0x0f, 0x34, 0x1f, 0x88
};

/** This variable is the test repeater topology VPrime */
static const u8 XHdcp22_Rx_Test_Repeater_VPrime[] =
{
	//********** R1  **********//
	0xbc, 0xcc, 0x7d, 0x16, 0xe6, 0xbc,
	0xb9, 0x02, 0x60, 0x08, 0x1d, 0xf7,
	0x4a, 0xb4, 0x5c, 0x8a
};

/** This variable is the test repeater management MPrime */
static const u8 XHdcp22_Rx_Test_Repeater_MPrime[] =
{
	//********** R1  **********//
	0xdd, 0x26, 0xe9, 0x52, 0x6e, 0x0e,
	0x1d, 0x69, 0xc8, 0x84, 0xe4, 0xcc,
	0xc8, 0x09, 0xaa, 0xc7, 0x71, 0xe9,
	0x97, 0xb5, 0x61, 0x89, 0x09, 0x6e,
	0x4d, 0x94, 0x24, 0xc2, 0x1b, 0x64,
	0x58, 0xc6
};

/** This is the test event vector for [NoStoredKm with Receiver] scenario */
static XHdcp22_Rx_TestState TestVector_Receiver_NoStoredKm[] =
{
	XHDCP22_RX_TEST_STATE_UNAUTHENTICATED,
	XHDCP22_RX_TEST_STATE_SEND_AKEINIT,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDCERT,
	XHDCP22_RX_TEST_STATE_SEND_AKENOSTOREDKM,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDHPRIME,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDPAIRING,
	XHDCP22_RX_TEST_STATE_SEND_LCINIT,
	XHDCP22_RX_TEST_STATE_WAIT_LCSENDLPRIME,
	XHDCP22_RX_TEST_STATE_SEND_SKESENDEKS,
	XHDCP22_RX_TEST_STATE_WAIT_AUTHENTICATED
};

/** This is the test event vector for [NoStoredKm with Receiver] scenario */
static XHdcp22_Rx_TestState TestVector_Receiver_StoredKm[] =
{
	XHDCP22_RX_TEST_STATE_UNAUTHENTICATED,
	XHDCP22_RX_TEST_STATE_SEND_AKEINIT,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDCERT,
	XHDCP22_RX_TEST_STATE_SEND_AKESTOREDKM,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDHPRIME,
	XHDCP22_RX_TEST_STATE_SEND_LCINIT,
	XHDCP22_RX_TEST_STATE_WAIT_LCSENDLPRIME,
	XHDCP22_RX_TEST_STATE_SEND_SKESENDEKS,
	XHDCP22_RX_TEST_STATE_WAIT_AUTHENTICATED
};

/** This is the test event vector for [NoStoredKm with Repeater] scenario
    Sequence: [List, ListAck, StreamManage, StreamReady] */
static XHdcp22_Rx_TestState TestVector_Repeater_NoStoredKm[] =
{
	XHDCP22_RX_TEST_STATE_UNAUTHENTICATED,
	XHDCP22_RX_TEST_STATE_SEND_AKEINIT,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDCERT,
	XHDCP22_RX_TEST_STATE_SEND_AKENOSTOREDKM,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDHPRIME,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDPAIRING,
	XHDCP22_RX_TEST_STATE_SEND_LCINIT,
	XHDCP22_RX_TEST_STATE_WAIT_LCSENDLPRIME,
	XHDCP22_RX_TEST_STATE_SEND_SKESENDEKS,
	XHDCP22_RX_TEST_STATE_UPDATE_TOPOLOGY,
	XHDCP22_RX_TEST_STATE_WAIT_RECEIVERIDLIST,
	XHDCP22_RX_TEST_STATE_SEND_RECEIVERIDLISTACK,
	XHDCP22_RX_TEST_STATE_SEND_STREAMMANAGEMENT,
	XHDCP22_RX_TEST_STATE_WAIT_STREAMREADY,
	XHDCP22_RX_TEST_STATE_WAIT_AUTHENTICATED
};

/** This is the test event vector for [StoredKm with Repeater] scenario
    Sequence: [List, ListAck, StreamManage, StreamReady] */
static XHdcp22_Rx_TestState TestVector_Repeater_StoredKm[] =
{
	XHDCP22_RX_TEST_STATE_UNAUTHENTICATED,
	XHDCP22_RX_TEST_STATE_SEND_AKEINIT,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDCERT,
	XHDCP22_RX_TEST_STATE_SEND_AKESTOREDKM,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDHPRIME,
	XHDCP22_RX_TEST_STATE_SEND_LCINIT,
	XHDCP22_RX_TEST_STATE_WAIT_LCSENDLPRIME,
	XHDCP22_RX_TEST_STATE_SEND_SKESENDEKS,
	XHDCP22_RX_TEST_STATE_UPDATE_TOPOLOGY,
	XHDCP22_RX_TEST_STATE_WAIT_RECEIVERIDLIST,
	XHDCP22_RX_TEST_STATE_SEND_RECEIVERIDLISTACK,
	XHDCP22_RX_TEST_STATE_SEND_STREAMMANAGEMENT,
	XHDCP22_RX_TEST_STATE_WAIT_STREAMREADY,
	XHDCP22_RX_TEST_STATE_WAIT_AUTHENTICATED
};

/** This is the test event vector for [Repeater Misordered Sequence 1] scenario
    Sequence: [StreamManage, StreamReady, List, ListAck] */
static XHdcp22_Rx_TestState TestVector_Repeater_Misordered_Sequence_1[] =
{
	XHDCP22_RX_TEST_STATE_UNAUTHENTICATED,
	XHDCP22_RX_TEST_STATE_SEND_AKEINIT,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDCERT,
	XHDCP22_RX_TEST_STATE_SEND_AKENOSTOREDKM,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDHPRIME,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDPAIRING,
	XHDCP22_RX_TEST_STATE_SEND_LCINIT,
	XHDCP22_RX_TEST_STATE_WAIT_LCSENDLPRIME,
	XHDCP22_RX_TEST_STATE_SEND_SKESENDEKS,
	XHDCP22_RX_TEST_STATE_SEND_STREAMMANAGEMENT,
	XHDCP22_RX_TEST_STATE_WAIT_STREAMREADY,
	XHDCP22_RX_TEST_STATE_UPDATE_TOPOLOGY,
	XHDCP22_RX_TEST_STATE_WAIT_RECEIVERIDLIST,
	XHDCP22_RX_TEST_STATE_SEND_RECEIVERIDLISTACK,
	XHDCP22_RX_TEST_STATE_WAIT_AUTHENTICATED
};

/** This is the test event vector for [NoStoredKm with Repeater]
    Sequence: [List, StreamManage, StreamReady, ListAck] */
static XHdcp22_Rx_TestState TestVector_Repeater_Misordered_Sequence_2[] =
{
	XHDCP22_RX_TEST_STATE_UNAUTHENTICATED,
	XHDCP22_RX_TEST_STATE_SEND_AKEINIT,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDCERT,
	XHDCP22_RX_TEST_STATE_SEND_AKENOSTOREDKM,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDHPRIME,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDPAIRING,
	XHDCP22_RX_TEST_STATE_SEND_LCINIT,
	XHDCP22_RX_TEST_STATE_WAIT_LCSENDLPRIME,
	XHDCP22_RX_TEST_STATE_SEND_SKESENDEKS,
	XHDCP22_RX_TEST_STATE_UPDATE_TOPOLOGY,
	XHDCP22_RX_TEST_STATE_WAIT_RECEIVERIDLIST,
	XHDCP22_RX_TEST_STATE_SEND_STREAMMANAGEMENT,
	XHDCP22_RX_TEST_STATE_WAIT_STREAMREADY,
	XHDCP22_RX_TEST_STATE_SEND_RECEIVERIDLISTACK,
	XHDCP22_RX_TEST_STATE_WAIT_AUTHENTICATED
};

/** This is the test event vector for [NoStoredKm with Repeater]
    Sequence: [List, StreamManage, ListAck, StreamReady] */
static XHdcp22_Rx_TestState TestVector_Repeater_Misordered_Sequence_3[] =
{
	XHDCP22_RX_TEST_STATE_UNAUTHENTICATED,
	XHDCP22_RX_TEST_STATE_SEND_AKEINIT,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDCERT,
	XHDCP22_RX_TEST_STATE_SEND_AKENOSTOREDKM,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDHPRIME,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDPAIRING,
	XHDCP22_RX_TEST_STATE_SEND_LCINIT,
	XHDCP22_RX_TEST_STATE_WAIT_LCSENDLPRIME,
	XHDCP22_RX_TEST_STATE_SEND_SKESENDEKS,
	XHDCP22_RX_TEST_STATE_UPDATE_TOPOLOGY,
	XHDCP22_RX_TEST_STATE_WAIT_RECEIVERIDLIST,
	XHDCP22_RX_TEST_STATE_SEND_STREAMMANAGEMENT,
	XHDCP22_RX_TEST_STATE_SEND_RECEIVERIDLISTACK,
	XHDCP22_RX_TEST_STATE_WAIT_STREAMREADY,
	XHDCP22_RX_TEST_STATE_WAIT_AUTHENTICATED
};

/** This is the test event vector for [Repeater Topology Change] */
static XHdcp22_Rx_TestState TestVector_Repeater_Topology_Change[] =
{
	XHDCP22_RX_TEST_STATE_UNAUTHENTICATED,
	XHDCP22_RX_TEST_STATE_SEND_AKEINIT,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDCERT,
	XHDCP22_RX_TEST_STATE_SEND_AKENOSTOREDKM,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDHPRIME,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDPAIRING,
	XHDCP22_RX_TEST_STATE_SEND_LCINIT,
	XHDCP22_RX_TEST_STATE_WAIT_LCSENDLPRIME,
	XHDCP22_RX_TEST_STATE_SEND_SKESENDEKS,
	XHDCP22_RX_TEST_STATE_UPDATE_TOPOLOGY,
	XHDCP22_RX_TEST_STATE_WAIT_RECEIVERIDLIST,
	XHDCP22_RX_TEST_STATE_SEND_RECEIVERIDLISTACK,
	XHDCP22_RX_TEST_STATE_SEND_STREAMMANAGEMENT,
	XHDCP22_RX_TEST_STATE_WAIT_STREAMREADY,
	XHDCP22_RX_TEST_STATE_WAIT_AUTHENTICATED,
	XHDCP22_RX_TEST_STATE_UPDATE_TOPOLOGY,
	XHDCP22_RX_TEST_STATE_WAIT_REPEATERREADY
};

/** This is the test event vector for [Repeater Topology Timeout] scenario */
static XHdcp22_Rx_TestState TestVector_Repeater_Topology_Timeout[] =
{
	XHDCP22_RX_TEST_STATE_UNAUTHENTICATED,
	XHDCP22_RX_TEST_STATE_SEND_AKEINIT,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDCERT,
	XHDCP22_RX_TEST_STATE_SEND_AKENOSTOREDKM,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDHPRIME,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDPAIRING,
	XHDCP22_RX_TEST_STATE_SEND_LCINIT,
	XHDCP22_RX_TEST_STATE_WAIT_LCSENDLPRIME,
	XHDCP22_RX_TEST_STATE_SEND_SKESENDEKS,
	XHDCP22_RX_TEST_STATE_UPDATE_TOPOLOGY,
	XHDCP22_RX_TEST_STATE_WAIT_RECEIVERIDLIST,
	XHDCP22_RX_TEST_STATE_WAIT_REAUTHREQ,
	XHDCP22_RX_TEST_STATE_SEND_AKEINIT,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDCERT,
	XHDCP22_RX_TEST_STATE_SEND_AKENOSTOREDKM,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDHPRIME,
	XHDCP22_RX_TEST_STATE_WAIT_AKESENDPAIRING,
	XHDCP22_RX_TEST_STATE_SEND_LCINIT,
	XHDCP22_RX_TEST_STATE_WAIT_LCSENDLPRIME,
	XHDCP22_RX_TEST_STATE_SEND_SKESENDEKS,
	XHDCP22_RX_TEST_STATE_UPDATE_TOPOLOGY,
	XHDCP22_RX_TEST_STATE_WAIT_RECEIVERIDLIST,
	XHDCP22_RX_TEST_STATE_SEND_RECEIVERIDLISTACK,
	XHDCP22_RX_TEST_STATE_SEND_STREAMMANAGEMENT,
	XHDCP22_RX_TEST_STATE_WAIT_STREAMREADY,
	XHDCP22_RX_TEST_STATE_WAIT_AUTHENTICATED
};

/************************** Function Prototypes *****************************/

/* Function to execute discrete test event */
static int  XHdcp22Rx_TestExecute(XHdcp22_Rx *InstancePtr);

/* Functions for loading test parameters */
int			XHdcp22Rx_TestLoadKeys(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestLoadVector(XHdcp22_Rx *InstancePtr, XHdcp22_Rx_TestFlags TestVectorFlag);
static void XHdcp22Rx_TestLoadRrx(XHdcp22_Rx *InstancePtr, const u8 *RrxPtr);

/* Functions for emulating DDC interface */
static void XHdcp22Rx_TestDdcResetRegisterMap(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestDdcGetRegisterMapIndex(XHdcp22_Rx *InstancePtr, u32 Address);
static int  XHdcp22Rx_TestDdcSetAddressCallback(XHdcp22_Rx *InstancePtr, u32 Addr);
static int  XHdcp22Rx_TestDdcSetDataCallback(XHdcp22_Rx *InstancePtr, u32 Data);
static u32  XHdcp22Rx_TestDdcGetDataCallback(XHdcp22_Rx *InstancePtr);
static u32  XHdcp22Rx_TestDdcGetWriteBufferSizeCallback(XHdcp22_Rx *InstancePtr);
static u32  XHdcp22Rx_TestDdcGetReadBufferSizeCallback(XHdcp22_Rx *InstancePtr);
static u8   XHdcp22Rx_TestDdcIsWriteBufferEmptyCallback(XHdcp22_Rx *InstancePtr);
static u8   XHdcp22Rx_TestDdcIsReadBufferEmptyCallback(XHdcp22_Rx *InstancePtr);
static void XHdcp22Rx_TestDdcClearReadBufferCallback(XHdcp22_Rx *InstancePtr);
static void XHdcp22Rx_TestDdcClearWriteBufferCallback(XHdcp22_Rx *InstancePtr);

/* Functions for test user callbacks */
static void XHdcp22Rx_TestAuthenticatedCallback(XHdcp22_Rx *InstancePtr);
static void XHdcp22Rx_TestRepeaterAuthRequestCallback(XHdcp22_Rx *InstancePtr);
static void XHdcp22Rx_TestRepeaterManageRequestCallback(XHdcp22_Rx *InstancePtr);

/* Functions for getting RxStatus */
static void XHdcp22Rx_TestGetRxStatus(XHdcp22_Rx *InstancePtr, u16 *Size, u8 *ReauthReq, u8 *Ready);

/* Functions for generating test messages */
static int  XHdcp22Rx_TestSendAKEInit(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestSendAKENoStoredKm(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestSendAKEStoredKm(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestSendLCInit(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestSendSKESendEks(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestSendReceiverIdListAck(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestSendStreamManagement(XHdcp22_Rx *InstancePtr);

/* Functions for processing test messages */
static int  XHdcp22Rx_TestReceiveAKESendCert(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestReceiveAKESendHPrime(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestReceiveAKESendPairingInfo(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestReceiveLCSendLPrime(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestReceiveReceiverIdList(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestReceiveStreamReady(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestWaitAuthenticated(XHdcp22_Rx *InstancePtr);

/* Function for repeater topology update */
static int  XHdcp22Rx_TestUpdateTopology(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestWaitReauthReq(XHdcp22_Rx *InstancePtr);
static int  XHdcp22Rx_TestWaitRepeaterReady(XHdcp22_Rx *InstancePtr);

/* Functions for reporting test results */
static void XHdcp22Rx_PrintDump(u8 Enable, char *String, const u8 *Data, int Length);
static int  XHdcp22Rx_TestCompare(XHdcp22_Rx *InstancePtr, char* String, const u8 *Expected,
							const u8 *Actual, u32 Size);
static void XHdcp22Rx_TestPrintMessage(XHdcp22_Rx *InstancePtr, XHdcp22_Rx_Message *Message,
							XHdcp22_Rx_MessageIds MessageId);
static void XHdcp22Rx_TestEvent2String(char* String, XHdcp22_Rx_TestState EventId);

/****************************************************************************/
/**
* This function configures the receiver in test mode. In the test mode
* the receiver uses the test keys defined in the HDCP 2.2 Errata.
* The DDC handles are set to the stub functions which emulate the
* DDC interface. The test flag can be set to either XHDCP22_RX_TESTMODE_NO_TX
*
* @param    InstancePtr is a pointer to an XHdcp22_Rx instance.
* @param    TestMode can be the following:
*           - XHDCP22_RX_TESTMODE_NO_TX:
*             Transmitter is emulated inside the driver and stimulation
*             of the receiver is through a predefined test vector.
*             This mode is useful for standalone self testing.
*           - XHDCP22_RX_TESTMODE_SW_TX:
*             Transmitter is emulated outside of the driver and hooked
*             into the receiver through the DDC stub interface. Stimulation
*             of the receiver is the responsibility of the transmitter.
*             This mode is useful for software loopback testing.
* @param	TestFlag is only relavent when the TestMode is set to
*           XHDCP22_RX_TESTMODE_NO_TX, and defines the test vector
*           used to drive the receiver.
* @return   XST_SUCCESS or XST_FAILURE.
*
* @note     None.
*****************************************************************************/
int XHdcp22Rx_TestSetMode(XHdcp22_Rx *InstancePtr, XHdcp22_Rx_TestMode TestMode,
	XHdcp22_Rx_TestFlags TestFlag)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(TestMode < XHDCP22_RX_TESTMODE_INVALID);
	Xil_AssertNonvoid(TestFlag < XHDCP22_RX_TEST_FLAG_INVALID);

	int Offset, Status = XST_SUCCESS;

	/* Initialize variables */
	XHdcp22Rx_TestDdcResetRegisterMap(InstancePtr);
	InstancePtr->Test.DdcRegisterMapAddress = 0;
	InstancePtr->Test.WriteMessageOffset = 0;
	InstancePtr->Test.WriteMessageSize = 0;
	memset(&InstancePtr->Test.WriteMessageBuffer, 0, sizeof(InstancePtr->Test.WriteMessageBuffer));
	InstancePtr->Test.ReadMessageOffset = 0;
	InstancePtr->Test.ReadMessageSize = 0;
	memset(&InstancePtr->Test.ReadMessageBuffer, 0, sizeof(InstancePtr->Test.ReadMessageBuffer));
	InstancePtr->Test.TestReturnCode = XST_DEVICE_BUSY;
	InstancePtr->Test.Verbose = FALSE;

	/* Set the callback functions */
	Status = XHdcp22Rx_SetCallback(InstancePtr,  XHDCP22_RX_HANDLER_DDC_SETREGADDR,
		(XHdcp22_Rx_SetHandler)XHdcp22Rx_TestDdcSetAddressCallback, InstancePtr);
	Status |= XHdcp22Rx_SetCallback(InstancePtr,  XHDCP22_RX_HANDLER_DDC_SETREGDATA,
		(XHdcp22_Rx_SetHandler)XHdcp22Rx_TestDdcSetDataCallback, InstancePtr);
	Status |= XHdcp22Rx_SetCallback(InstancePtr,  XHDCP22_RX_HANDLER_DDC_GETREGDATA,
		(XHdcp22_Rx_GetHandler)XHdcp22Rx_TestDdcGetDataCallback, InstancePtr);
	Status |= XHdcp22Rx_SetCallback(InstancePtr,  XHDCP22_RX_HANDLER_DDC_GETWBUFSIZE,
		(XHdcp22_Rx_GetHandler)XHdcp22Rx_TestDdcGetWriteBufferSizeCallback, InstancePtr);
	Status |= XHdcp22Rx_SetCallback(InstancePtr,  XHDCP22_RX_HANDLER_DDC_GETRBUFSIZE,
		(XHdcp22_Rx_GetHandler)XHdcp22Rx_TestDdcGetReadBufferSizeCallback, InstancePtr);
	Status |= XHdcp22Rx_SetCallback(InstancePtr,  XHDCP22_RX_HANDLER_DDC_ISWBUFEMPTY,
		(XHdcp22_Rx_GetHandler)XHdcp22Rx_TestDdcIsWriteBufferEmptyCallback, InstancePtr);
	Status |= XHdcp22Rx_SetCallback(InstancePtr,  XHDCP22_RX_HANDLER_DDC_ISRBUFEMPTY,
		(XHdcp22_Rx_GetHandler)XHdcp22Rx_TestDdcIsReadBufferEmptyCallback, InstancePtr);
	Status |= XHdcp22Rx_SetCallback(InstancePtr,  XHDCP22_RX_HANDLER_DDC_CLEARRBUF,
		(XHdcp22_Rx_RunHandler)XHdcp22Rx_TestDdcClearReadBufferCallback, InstancePtr);
	Status |= XHdcp22Rx_SetCallback(InstancePtr,  XHDCP22_RX_HANDLER_DDC_CLEARWBUF,
		(XHdcp22_Rx_RunHandler)XHdcp22Rx_TestDdcClearWriteBufferCallback, InstancePtr);
	Status |= XHdcp22Rx_SetCallback(InstancePtr,  XHDCP22_RX_HANDLER_AUTHENTICATED,
		(XHdcp22_Rx_RunHandler)XHdcp22Rx_TestAuthenticatedCallback, InstancePtr);
	Status |= XHdcp22Rx_SetCallback(InstancePtr,  XHDCP22_RX_HANDLER_AUTHENTICATION_REQUEST,
		(XHdcp22_Rx_RunHandler)XHdcp22Rx_TestRepeaterAuthRequestCallback, InstancePtr);
	Status |= XHdcp22Rx_SetCallback(InstancePtr,  XHDCP22_RX_HANDLER_STREAM_MANAGE_REQUEST,
		(XHdcp22_Rx_RunHandler)XHdcp22Rx_TestRepeaterManageRequestCallback, InstancePtr);

	if(Status != XST_SUCCESS)
	{
		return Status;
	}

	/*
	 * When the TX is emulated the driver is responsible for stimulating the
	 * receiver. Load the predefined directed test vector.
	 */
	if(TestMode == XHDCP22_RX_TESTMODE_NO_TX)
	{
		/* Print simulation banner */
		xil_printf("\n\n############################################################\r\n");
		xil_printf("HDCP 2.2 Receiver and Repeater Upstream Test.\r\n");
		xil_printf("Test vectors and keys are from DCP \r\n");
		xil_printf("Errata to HDCP on HDMI Revision 2.2, February 09, 2015\r\n");

		/* Load test vector */
		Status = XHdcp22Rx_TestLoadVector(InstancePtr, TestFlag);
		if(Status != XST_SUCCESS)
		{
			return Status;
		}

		/* Load the test keys from the errata */
		Status = XHdcp22Rx_TestLoadKeys(InstancePtr);
		if(Status != XST_SUCCESS)
		{
			return Status;
		}

		/* Print test vector */
		xil_printf("Simulating TestVector[0:%0d]: [", InstancePtr->Test.NextStateSize-1);
		for(Offset=0; Offset<InstancePtr->Test.NextStateSize; Offset++)
		{
			xil_printf("->0x%0x",InstancePtr->Test.NextStateVector[Offset]);
		}
		xil_printf("]\r\n");
	}

	/* Update flags */
	InstancePtr->Test.TestMode = TestMode;
	InstancePtr->Test.TestFlag = TestFlag;

	return Status;
}

/****************************************************************************/
/**
* This function is used to load the receiver test keys defined in
* Errata to HDCP on HDMI Specification Revision 2.2, February 09, 2015.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	None.
*
* @note		If a transmitter wants to perform authentication while
* 			the test keys are loaded it must use the test DCP public keys
*			defined in the errata; otherise, authentication is expected to
*			fail.
*****************************************************************************/
int XHdcp22Rx_TestLoadKeys(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status;

	/* Load Keys */
	XHdcp22Rx_LoadPublicCert(InstancePtr, XHdcp22_Rx_Test_PublicCert[InstancePtr->Test.TestReceiver]);
	Status = XHdcp22Rx_LoadPrivateKey(InstancePtr, XHdcp22_Rx_Test_PrivateKey[InstancePtr->Test.TestReceiver]);
	XHdcp22Rx_LoadLc128(InstancePtr, XHdcp22_Rx_Test_Lc128[InstancePtr->Test.TestReceiver]);
	XHdcp22Rx_TestLoadRrx(InstancePtr, XHdcp22_Rx_Test_Rrx[InstancePtr->Test.TestReceiver]);

	return Status;
}

/****************************************************************************/
/**
* This function is used to check when the test has completed.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	TRUE or FALSE.
*
* @note		None.
*****************************************************************************/
u8 XHdcp22Rx_TestIsFinished(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (InstancePtr->Test.TestReturnCode != XST_DEVICE_BUSY) ? TRUE : FALSE;
}

/****************************************************************************/
/**
* This function is used to check the pass/fail status after the test has
* completed.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	TRUE or FALSE.
*
* @note		Use the XHdcp22Rx_TestIsFinished function to check if test
* 			has completed.
*****************************************************************************/
u8 XHdcp22Rx_TestIsPassed(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (InstancePtr->Test.TestReturnCode == XST_SUCCESS) ? TRUE : FALSE;
}

/****************************************************************************/
/**
* This function is used to run the standalone receiver test environment.
* This function is non-blocking and expected to be run using a loop.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS when the test has completed successfully.
* 			XST_DEVICE_BUSY when the test is in progress.
* 			XST_FAILURE when the test has completed unsuccessfully.
*
* @note		None.
*****************************************************************************/
int XHdcp22Rx_TestRun(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status;
	char String[32];

	if(InstancePtr->Test.NextStateOffset == InstancePtr->Test.NextStateSize)
	{
	InstancePtr->Test.TestReturnCode = XST_SUCCESS;
	}
	else
	{
		XHdcp22Rx_TestEvent2String(String,
			InstancePtr->Test.NextStateVector[InstancePtr->Test.NextStateOffset]);
		XHdcp22Rx_Printf(InstancePtr->Test.Verbose, "DEBUG: Executing TestVector[%0d]: %s\r\n",
			(int)InstancePtr->Test.NextStateOffset, String);
		Status = XHdcp22Rx_TestExecute(InstancePtr);

		switch(Status)
		{
		case XST_SUCCESS:
			InstancePtr->Test.NextStateOffset++;
		InstancePtr->Test.TestReturnCode = XST_DEVICE_BUSY;
		break;
		case XST_DEVICE_BUSY:
			InstancePtr->Test.TestReturnCode = XST_DEVICE_BUSY;
			break;
		case XST_FAILURE:
			InstancePtr->Test.TestReturnCode = XST_FAILURE;
			break;
		default:
			InstancePtr->Test.TestReturnCode = XST_FAILURE;
		}
	}

	return (InstancePtr->Test.TestReturnCode);
}

/****************************************************************************/
/**
* This function sets verbose logging mode for standalone driver testing.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
* @param	Verbose is set to TRUE to enable detailed logging.
*
* @return	None.
*
* @note		Note that enabling verbose logging will impact the performance
* 			of the receiver authentication and key exchange since these
* 			log messages are not buffered, but instead printed immediately.
* 			Verbose logging is useful for debugging functional issues with
* 			the receiver when running standalone driver tests.
*****************************************************************************/
void XHdcp22Rx_TestSetVerbose(XHdcp22_Rx *InstancePtr, u8 Verbose)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->Test.Verbose = Verbose;
}

/****************************************************************************/
/**
* This function performs the test DDC write transaction. This function
* is used for both standalone and loopback driver testing.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
* @param	DeviceAddress is the DDC device address
* @param	Size is the number of bytes to send
* @param	Data is a byte array to send of length Size
* @param	Stop is a flag to set the stop condition
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		Does not support auto-increment.
*****************************************************************************/
int XHdcp22Rx_TestDdcWriteReg(XHdcp22_Rx *InstancePtr, u8 DeviceAddress, int Size, u8 *Data, u8 Stop)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u32 Offset;

	/* Update register address */
	InstancePtr->Test.DdcRegisterAddress = Data[0];
	if(Size == 1)
	{
		return XST_SUCCESS;
	}
	Size--;

	/* Check device and register address in range */
	Offset = XHdcp22Rx_TestDdcGetRegisterMapIndex(InstancePtr, InstancePtr->Test.DdcRegisterAddress);
	if((Offset >= XHDCP22_RX_TEST_DDC_REGMAP_SIZE) || (DeviceAddress != XHDCP22_RX_TEST_DDC_BASE_ADDRESS))
	{
		return XST_FAILURE;
	}

	/* Check register access then do write */
	if((InstancePtr->Test.DdcRegisterMap[Offset].Access == XHDCP22_RX_TEST_DDC_ACCESS_WO) ||
		(InstancePtr->Test.DdcRegisterMap[Offset].Access == XHDCP22_RX_TEST_DDC_ACCESS_RW))
	{
		switch(InstancePtr->Test.DdcRegisterAddress)
		{
		case XHDCP22_RX_DDC_WRITE_REG:
			/* Overwrite write buffer */
			memcpy(InstancePtr->Test.WriteMessageBuffer, Data+1, Size);
			InstancePtr->Test.WriteMessageSize = Size;
			InstancePtr->Test.WriteMessageOffset = 0;
			XHdcp22Rx_SetWriteMessageAvailable(InstancePtr);
			break;
		default:
			InstancePtr->Test.DdcRegisterMap[Offset].Value = *(Data+1);
			break;
		}

		return XST_SUCCESS;
	}

	return XST_FAILURE;
}

/****************************************************************************/
/**
* This function performs the test DDC read transaction. This function is
* used for both standalone and loopback driver testing.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
* @param	DeviceAddress is the DDC device address
* @param	Size is the number of bytes to send
* @param	Data is a byte array to send of length Size
* @param	Stop is a flag to set the stop condition
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		Does not support auto-increment.
*****************************************************************************/
int XHdcp22Rx_TestDdcReadReg(XHdcp22_Rx *InstancePtr, u8 DeviceAddress, int Size, u8 *Data, u8 Stop)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u32 Offset;

	/* Check device and register address in range */
	Offset = XHdcp22Rx_TestDdcGetRegisterMapIndex(InstancePtr, InstancePtr->Test.DdcRegisterAddress);
	if((Offset >= XHDCP22_RX_TEST_DDC_REGMAP_SIZE) || (DeviceAddress != XHDCP22_RX_TEST_DDC_BASE_ADDRESS))
	{
		return XST_FAILURE;
	}

	/* Check register access then do read */
	if((InstancePtr->Test.DdcRegisterMap[Offset].Access == XHDCP22_RX_TEST_DDC_ACCESS_RO) ||
		(InstancePtr->Test.DdcRegisterMap[Offset].Access == XHDCP22_RX_TEST_DDC_ACCESS_RW))
	{
		switch(InstancePtr->Test.DdcRegisterAddress)
		{
		case XHDCP22_RX_DDC_READ_REG:
			/* Flush read buffer */
			memcpy(Data, InstancePtr->Test.ReadMessageBuffer, Size);
			InstancePtr->Test.ReadMessageSize = 0;
			InstancePtr->Test.ReadMessageOffset = 0;

			/* Clear RxStatus registers */
			Offset = XHdcp22Rx_TestDdcGetRegisterMapIndex(InstancePtr, XHDCP22_RX_DDC_RXSTATUS0_REG);
			InstancePtr->Test.DdcRegisterMap[Offset].Value = 0;
			Offset = XHdcp22Rx_TestDdcGetRegisterMapIndex(InstancePtr, XHDCP22_RX_DDC_RXSTATUS1_REG);
			InstancePtr->Test.DdcRegisterMap[Offset].Value = 0;

			XHdcp22Rx_SetReadMessageComplete(InstancePtr);
			break;
		case XHDCP22_RX_DDC_RXSTATUS0_REG:
			if(Size == 2)
			{
				Data[0] = InstancePtr->Test.DdcRegisterMap[Offset].Value;
				Offset = XHdcp22Rx_TestDdcGetRegisterMapIndex(InstancePtr, InstancePtr->Test.DdcRegisterAddress+1);
				Data[1] = InstancePtr->Test.DdcRegisterMap[Offset].Value;
				break;
			}
		default:
			*Data = InstancePtr->Test.DdcRegisterMap[Offset].Value;
			break;
		}

		return XST_SUCCESS;
	}

	return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function replaces Rrx with a test vector if the testmode is
* #XHDCP22_RX_TESTMODE_NO_TX.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
* @param	RrxPtr is a pointer to Rrx.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XHdcp22Rx_TestGenerateRrx(XHdcp22_Rx *InstancePtr, u8* RrxPtr)
{
	/* In test mode copy the test vector */
	if(InstancePtr->Test.TestMode == XHDCP22_RX_TESTMODE_NO_TX)
	{
		memcpy(RrxPtr, InstancePtr->Test.Rrx, XHDCP22_RX_RRX_SIZE);
	}
}

/****************************************************************************/
/**
* This function is used to execute the next discrete test event.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS when the test step has completed successfully.
* 			XST_FAILURE when the test step has completed unsuccessfully.
*
* @note		None.
*****************************************************************************/
static int XHdcp22Rx_TestExecute(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Execute next event */
	switch(InstancePtr->Test.NextStateVector[InstancePtr->Test.NextStateOffset])
	{
		case XHDCP22_RX_TEST_STATE_UNAUTHENTICATED:
			return XST_SUCCESS;
		case XHDCP22_RX_TEST_STATE_SEND_AKEINIT:
			return XHdcp22Rx_TestSendAKEInit(InstancePtr);
		case XHDCP22_RX_TEST_STATE_WAIT_AKESENDCERT:
			return XHdcp22Rx_TestReceiveAKESendCert(InstancePtr);
		case XHDCP22_RX_TEST_STATE_SEND_AKENOSTOREDKM:
			return XHdcp22Rx_TestSendAKENoStoredKm(InstancePtr);
		case XHDCP22_RX_TEST_STATE_SEND_AKESTOREDKM:
			return XHdcp22Rx_TestSendAKEStoredKm(InstancePtr);
		case XHDCP22_RX_TEST_STATE_WAIT_AKESENDHPRIME:
			return XHdcp22Rx_TestReceiveAKESendHPrime(InstancePtr);
		case XHDCP22_RX_TEST_STATE_WAIT_AKESENDPAIRING:
			return XHdcp22Rx_TestReceiveAKESendPairingInfo(InstancePtr);
		case XHDCP22_RX_TEST_STATE_SEND_LCINIT:
			return XHdcp22Rx_TestSendLCInit(InstancePtr);
		case XHDCP22_RX_TEST_STATE_WAIT_LCSENDLPRIME:
			return XHdcp22Rx_TestReceiveLCSendLPrime(InstancePtr);
		case XHDCP22_RX_TEST_STATE_SEND_SKESENDEKS:
			return XHdcp22Rx_TestSendSKESendEks(InstancePtr);
		case XHDCP22_RX_TEST_STATE_UPDATE_TOPOLOGY:
			return XHdcp22Rx_TestUpdateTopology(InstancePtr);
		case XHDCP22_RX_TEST_STATE_WAIT_RECEIVERIDLIST:
			return XHdcp22Rx_TestReceiveReceiverIdList(InstancePtr);
		case XHDCP22_RX_TEST_STATE_SEND_RECEIVERIDLISTACK:
			return XHdcp22Rx_TestSendReceiverIdListAck(InstancePtr);
		case XHDCP22_RX_TEST_STATE_SEND_STREAMMANAGEMENT:
			return XHdcp22Rx_TestSendStreamManagement(InstancePtr);
		case XHDCP22_RX_TEST_STATE_WAIT_STREAMREADY:
			return XHdcp22Rx_TestReceiveStreamReady(InstancePtr);
		case XHDCP22_RX_TEST_STATE_WAIT_REAUTHREQ:
			return XHdcp22Rx_TestWaitReauthReq(InstancePtr);
		case XHDCP22_RX_TEST_STATE_WAIT_REPEATERREADY:
			return XHdcp22Rx_TestWaitRepeaterReady(InstancePtr);
		case XHDCP22_RX_TEST_STATE_WAIT_AUTHENTICATED:
			return XHdcp22Rx_TestWaitAuthenticated(InstancePtr);
		default:
			return XST_FAILURE;
	}
}

/****************************************************************************/
/**
* This function is used to load a pre-defined directed test vector.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
*****************************************************************************/
static int XHdcp22Rx_TestLoadVector(XHdcp22_Rx *InstancePtr, XHdcp22_Rx_TestFlags TestFlag)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int TestVectorSize;

	switch(TestFlag)
	{
	case XHDCP22_RX_TEST_FLAG_NOSTOREDKM_WITH_RECEIVER:
		InstancePtr->Test.TestReceiver = XHDCP22_RX_TEST_RECEIVER_2;
		InstancePtr->Test.NextStateVector = (int *)TestVector_Receiver_NoStoredKm;
		TestVectorSize = sizeof(TestVector_Receiver_NoStoredKm)/sizeof(XHdcp22_Rx_TestState);
		xil_printf("Testcase: [No_Stored_km with Receiver]\r\n");
		break;
	case XHDCP22_RX_TEST_FLAG_STOREDKM_WITH_RECEIVER:
		InstancePtr->Test.TestReceiver = XHDCP22_RX_TEST_RECEIVER_2;
		InstancePtr->Test.NextStateVector = (int *)TestVector_Receiver_StoredKm;
		TestVectorSize = sizeof(TestVector_Receiver_StoredKm)/sizeof(XHdcp22_Rx_TestState);
		xil_printf("Testcase: [Stored_km with Receiver]\r\n");
		break;
	case XHDCP22_RX_TEST_FLAG_NOSTOREDKM_WITH_REPEATER:
		InstancePtr->Test.TestReceiver = XHDCP22_RX_TEST_RECEIVER_1;
		InstancePtr->Test.NextStateVector = (int *)TestVector_Repeater_NoStoredKm;
		TestVectorSize = sizeof(TestVector_Repeater_NoStoredKm)/sizeof(XHdcp22_Rx_TestState);
		xil_printf("Testcase: [No_Stored_km with Repeater], Sequence: [List, ListAck, StreamManage, StreamReady]\r\n");
		break;
	case XHDCP22_RX_TEST_FLAG_STOREDKM_WITH_REPEATER:
		InstancePtr->Test.TestReceiver = XHDCP22_RX_TEST_RECEIVER_1;
		InstancePtr->Test.NextStateVector = (int *)TestVector_Repeater_StoredKm;
		TestVectorSize = sizeof(TestVector_Repeater_StoredKm)/sizeof(XHdcp22_Rx_TestState);
		xil_printf("Testcase: [Stored_km with Repeater], Sequence: [List, ListAck, StreamManage, StreamReady]\r\n");
		break;
	case XHDCP22_RX_TEST_FLAG_REPEATER_MISORDERED_SEQUENCE_1:
		InstancePtr->Test.TestReceiver = XHDCP22_RX_TEST_RECEIVER_1;
		InstancePtr->Test.NextStateVector = (int *)TestVector_Repeater_Misordered_Sequence_1;
		TestVectorSize = sizeof(TestVector_Repeater_Misordered_Sequence_1)/sizeof(XHdcp22_Rx_TestState);
		xil_printf("Testcase: [Repeater Misordered Sequence 1], Sequence: [StreamManage, StreamReady, List, ListAck]\r\n");
		break;
	case XHDCP22_RX_TEST_FLAG_REPEATER_MISORDERED_SEQUENCE_2:
		InstancePtr->Test.TestReceiver = XHDCP22_RX_TEST_RECEIVER_1;
		InstancePtr->Test.NextStateVector = (int *)TestVector_Repeater_Misordered_Sequence_2;
		TestVectorSize = sizeof(TestVector_Repeater_Misordered_Sequence_2)/sizeof(XHdcp22_Rx_TestState);
		xil_printf("Testcase: [Repeater Misordered Sequence 2], Sequence [List, StreamManage, StreamReady, ListAck]\r\n");
		break;
	case XHDCP22_RX_TEST_FLAG_REPEATER_MISORDERED_SEQUENCE_3:
		InstancePtr->Test.TestReceiver = XHDCP22_RX_TEST_RECEIVER_1;
		InstancePtr->Test.NextStateVector = (int *)TestVector_Repeater_Misordered_Sequence_3;
		TestVectorSize = sizeof(TestVector_Repeater_Misordered_Sequence_3)/sizeof(XHdcp22_Rx_TestState);
		xil_printf("Testcase: [Repeater Misordered Sequence 3], Sequence [List, StreamManage, ListAck, StreamReady]\r\n");
		break;
	case XHDCP22_RX_TEST_FLAG_REPEATER_TOPOLOGY_CHANGE:
		InstancePtr->Test.TestReceiver = XHDCP22_RX_TEST_RECEIVER_1;
		InstancePtr->Test.NextStateVector = (int *)TestVector_Repeater_Topology_Change;
		TestVectorSize = sizeof(TestVector_Repeater_Topology_Change)/sizeof(XHdcp22_Rx_TestState);
		xil_printf("Testcase: [Repeater Topology Change]\r\n");
		break;
	case XHDCP22_RX_TEST_FLAG_REPEATER_TOPOLOGY_TIMEOUT:
		InstancePtr->Test.TestReceiver = XHDCP22_RX_TEST_RECEIVER_1;
		InstancePtr->Test.NextStateVector = (int *)TestVector_Repeater_Topology_Timeout;
		TestVectorSize = sizeof(TestVector_Repeater_Topology_Timeout)/sizeof(XHdcp22_Rx_TestState);
		xil_printf("Testcase: [Repeater Topology Timeout]\r\n");
		break;
	default:
		xil_printf("Unknown Testcase?\r\n");
		return XST_FAILURE;
	}

	InstancePtr->Test.State = InstancePtr->Test.NextStateVector[0];
	InstancePtr->Test.NextStateSize = TestVectorSize;
	InstancePtr->Test.NextStateOffset = 0;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function is used to load the Rrx test vector.
*
* @param	RrxPtr is a pointer to the test vector.
*
* @return	None.
*
* @note		None.
******************************************************************************/
static void XHdcp22Rx_TestLoadRrx(XHdcp22_Rx *InstancePtr, const u8 *RrxPtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(RrxPtr != NULL);

	memcpy(InstancePtr->Test.Rrx, RrxPtr, XHDCP22_RX_RRX_SIZE);
}

/****************************************************************************/
/**
* This function is used to construct and reset the test DDC register
* map to the defaults. The register map is modeled after the HDCP22
* port to HDMI.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	None.
*
* @note		None.
*****************************************************************************/
static void XHdcp22Rx_TestDdcResetRegisterMap(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Construct default DDC register map */
	XHdcp22_Rx_TestDdcReg DdcRegisterMap[XHDCP22_RX_TEST_DDC_REGMAP_SIZE] =
	{
			{XHDCP22_RX_DDC_VERSION_REG, "HDCP2Version", XHDCP22_RX_TEST_DDC_ACCESS_RO, 0},
			{XHDCP22_RX_DDC_WRITE_REG, "Write_Message", XHDCP22_RX_TEST_DDC_ACCESS_WO, 0},
			{XHDCP22_RX_DDC_RXSTATUS0_REG, "RxStatus_0", XHDCP22_RX_TEST_DDC_ACCESS_RO, 0},
			{XHDCP22_RX_DDC_RXSTATUS1_REG, "RxStatus_1", XHDCP22_RX_TEST_DDC_ACCESS_RO, 0},
			{XHDCP22_RX_DDC_READ_REG, "Read_Message", XHDCP22_RX_TEST_DDC_ACCESS_RO, 0}
	};

	/* Initialize DDC registers */
	memcpy(InstancePtr->Test.DdcRegisterMap, DdcRegisterMap, sizeof(DdcRegisterMap));
}

/****************************************************************************/
/**
* This function is used to get the test DDC register map index for the
* desired register address.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
* @param	Address is the DDC register address.
*
* @return	Returns the test DDC register map index for the specified
* 			address. When the address is not found the returned offset
* 			is forced to be out of range.
*
* @note		None.
*****************************************************************************/
static int XHdcp22Rx_TestDdcGetRegisterMapIndex(XHdcp22_Rx *InstancePtr, u32 Address)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Offset = XHDCP22_RX_TEST_DDC_REGMAP_SIZE + 1;
	int Index;

	for(Index=0; Index<XHDCP22_RX_TEST_DDC_REGMAP_SIZE; Index++)
	{
		if(InstancePtr->Test.DdcRegisterMap[Index].Address == Address)
		{
			Offset = Index;
			break;
		}
	}

	return Offset;
}

/****************************************************************************/
/**
* This function is used to compare two byte arrays for consistency.
*
* @param	String is an message used to identify the data being compared.
* @param	Expected is the expected byte array.
* @param	Actual is the actual byte array to compare against the expected.
* @param	Size is the number of bytes in the arrays to be compared.
*
* @return	- XST_SUCCESS on match
*			- XST_FAILURE on mismatch
*
* @note		None.
*****************************************************************************/
static int XHdcp22Rx_TestCompare(XHdcp22_Rx *InstancePtr, char *String, const u8 *Expected,
	const u8 *Actual, u32 Size)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u32 Offset;

	for(Offset=0; Offset<Size; Offset++)
	{
		if(Actual[Offset] != Expected[Offset])
		{
			xil_printf("ERROR: Checking [%s]: !!Mismatch!! for Byte[%0d], Expected=0x%0x, Actual=0x%0x\r\n",
				String, Offset, Expected[Offset], Actual[Offset]);
			return XST_FAILURE;
		}
	}

	XHdcp22Rx_Printf(InstancePtr->Test.Verbose, "DEBUG: Checking [%s]: !!Matched!!\r\n", String);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function is used to print an octet string in big endian format.
*
* @param	Enable is an input flag to enable printing.
* @param	String is a character array info message.
* @param	Data is the octet string to be printed.
* @param	Length is the number of bytes in Data
*
* @return	None.
*
* @note		None.
******************************************************************************/
static void XHdcp22Rx_PrintDump(u8 Enable, char *String, const u8 *Data, int Length)
{
	/* Verify arguments */
	Xil_AssertVoid(String != NULL);
	Xil_AssertVoid(Data != NULL);
	Xil_AssertVoid(Length >= 0);

	int Offset;

	if(Enable)
	{
		XHdcp22Rx_Printf(Enable, "%s::Byte[0:%0d]", String, Length-1);
		for(Offset=0; Offset<Length; Offset++)
		{
			if((Offset%20) == 0)
				XHdcp22Rx_Printf(Enable, "\r\n");

			XHdcp22Rx_Printf(Enable, " %02x", Data[Offset]);
		}
		XHdcp22Rx_Printf(Enable, "\r\n");
	}
}

/****************************************************************************/
/**
* This function is used to print authentication and key exchange messages
* during standalone driver testing. The values printed should
* match the Errata.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
* @param	Message is a pointer to the actual message.
* @param	MessageId is the identifier used to determine what
* 			message to print.
*
* @return	None.
*
* @note		The messages are only printed when the test logging is set
* 			to verbose via the function XHdcp22Rx_TestSetVerbose.
*****************************************************************************/
static void XHdcp22Rx_TestPrintMessage(XHdcp22_Rx *InstancePtr, XHdcp22_Rx_Message* Message,
	XHdcp22_Rx_MessageIds MessageId)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Print selected message contents */
	switch(MessageId)
	{
	case XHDCP22_RX_MSG_ID_AKEINIT:
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKEInit::MsgId", &(Message->AKEInit.MsgId), 1);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKEInit::Rtx", Message->AKEInit.Rtx, XHDCP22_RX_RTX_SIZE);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKEInit::TxCaps", Message->AKEInit.TxCaps, XHDCP22_RX_TXCAPS_SIZE);
		break;
	case XHDCP22_RX_MSG_ID_AKESENDCERT:
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKESendCert::MsgId", &(Message->AKESendCert.MsgId), 1);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKESendCert::CertRx", Message->AKESendCert.CertRx, sizeof(XHdcp22_Rx_CertRx));
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKESendCert::Rrx", Message->AKESendCert.Rrx, XHDCP22_RX_RRX_SIZE);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKESendCert::Rrx", Message->AKESendCert.RxCaps, XHDCP22_RX_RXCAPS_SIZE);
		break;
	case XHDCP22_RX_MSG_ID_AKENOSTOREDKM:
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKENoStoredKm::MsgId", &(Message->AKENoStoredKm.MsgId), 1);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKENoStoredKm::EKpubKm", Message->AKENoStoredKm.EKpubKm, XHDCP22_RX_N_SIZE);
		break;
	case XHDCP22_RX_MSG_ID_AKESTOREDKM:
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKEStoredKm::MsgId", &(Message->AKEStoredKm.MsgId), 1);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKEStoredKm::EKhKm", Message->AKEStoredKm.EKhKm, XHDCP22_RX_EKH_SIZE);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKEStoredKm::M", Message->AKEStoredKm.M, XHDCP22_RX_RTX_SIZE+XHDCP22_RX_RRX_SIZE);
		break;
	case XHDCP22_RX_MSG_ID_AKESENDHPRIME:
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKESendHPrime::MsgId", &(Message->AKESendHPrime.MsgId), 1);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKESendHPrime::HPrime", Message->AKESendHPrime.HPrime, XHDCP22_RX_HPRIME_SIZE);
		break;
	case XHDCP22_RX_MSG_ID_AKESENDPAIRINGINFO:
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKESendPairingInfo::MsgId", &(Message->AKESendPairingInfo.MsgId), 1);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: AKESendPairingInfo::EKhKm", Message->AKESendPairingInfo.EKhKm, XHDCP22_RX_EKH_SIZE);
		break;
	case XHDCP22_RX_MSG_ID_LCINIT:
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: LCInit::MsgId", &(Message->LCInit.MsgId), 1);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: LCInit::Rn", Message->LCInit.Rn, XHDCP22_RX_RN_SIZE);
		break;
	case XHDCP22_RX_MSG_ID_LCSENDLPRIME:
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: LCSendLPrime::MsgId", &(Message->LCSendLPrime.MsgId), 1);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: LCSendLPrime::LPrime", Message->LCSendLPrime.LPrime, XHDCP22_RX_LPRIME_SIZE);
		break;
	case XHDCP22_RX_MSG_ID_SKESENDEKS:
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: SKESendEks::MsgId", &(Message->SKESendEks.MsgId), 1);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: SKESendEks::EDkeyKs", Message->SKESendEks.EDkeyKs, XHDCP22_RX_KS_SIZE);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: SKESendEks::Riv", Message->SKESendEks.Riv, XHDCP22_RX_RIV_SIZE);
		break;
	case XHDCP22_RX_MSG_ID_REPEATERAUTHSENDRXIDLIST:
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: RepeaterAuthSendReceiverIdList::MsgId",
			&(Message->RepeaterAuthSendRxIdList.MsgId), 1);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: RepeaterAuthSendReceiverIdList::RxInfo",
			Message->RepeaterAuthSendRxIdList.RxInfo, XHDCP22_RX_RXINFO_SIZE);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: RepeaterAuthSendReceiverIdList::SeqNumV",
			Message->RepeaterAuthSendRxIdList.SeqNumV, XHDCP22_RX_SEQNUMV_SIZE);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: RepeaterAuthSendReceiverIdList::VPrime",
			Message->RepeaterAuthSendRxIdList.VPrime, 16);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: RepeaterAuthSendReceiverIdList::ReceiverIdList",
			Message->RepeaterAuthSendRxIdList.ReceiverIdList,
			XHDCP22_RX_MAX_DEVICE_COUNT*XHDCP22_RX_RCVID_SIZE);
		break;
	case XHDCP22_RX_MSG_ID_REPEATERAUTHSENDACK:
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: RepeaterAuthSendAck::MsgId", &(Message->RepeaterAuthSendAck.MsgId), 1);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: RepeaterAuthSendAck::V", Message->RepeaterAuthSendAck.V, 16);
		break;
	case XHDCP22_RX_MSG_ID_REPEATERAUTHSTREAMMANAGE:
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: RepeaterAuthStreamManage::MsgId",
			&(Message->RepeaterAuthStreamManage.MsgId), 1);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: RepeaterAuthStreamManage::SeqNumM",
			Message->RepeaterAuthStreamManage.SeqNumM, XHDCP22_RX_SEQNUMM_SIZE);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: RepeaterAuthStreamManage::K",
			Message->RepeaterAuthStreamManage.K, 2);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: RepeaterAuthStreamManage::StreamIdType",
			Message->RepeaterAuthStreamManage.StreamIdType, XHDCP22_RX_STREAMID_SIZE);
		break;
	case XHDCP22_RX_MSG_ID_REPEATERAUTHSTREAMREADY:
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: RepeaterAuthStreamReady::MsgId",
			&(Message->RepeaterAuthStreamReady.MsgId), 1);
		XHdcp22Rx_PrintDump(InstancePtr->Test.Verbose,
			"DEBUG: RepeaterAuthStreamReady::MPrime",
			Message->RepeaterAuthStreamReady.MPrime, XHDCP22_RX_MPRIME_SIZE);
		break;
	default:
		print("ERROR: Unexpected test print message\r\n");
		break;
	}
}

/****************************************************************************/
/**
* This function is used to print the test event from enumerated type
* to a descriptive event string.
*
* @param	String is updated with the event string
* @param	EventId is the identifier used to determine what
* 			event to print.
*
* @return	None.
*
* @note		None.
*****************************************************************************/
static void XHdcp22Rx_TestEvent2String(char* String, XHdcp22_Rx_TestState EventId)
{
	/* Map event to string */
	switch(EventId)
	{
	case XHDCP22_RX_TEST_STATE_UNAUTHENTICATED:
		strcpy(String, "Unauthenticated"); break;
	case XHDCP22_RX_TEST_STATE_SEND_AKEINIT:
		strcpy(String, "Send_AKEInit"); break;
	case XHDCP22_RX_TEST_STATE_WAIT_AKESENDCERT:
		strcpy(String, "Wait_AKESendCert"); break;
	case XHDCP22_RX_TEST_STATE_SEND_AKENOSTOREDKM:
		strcpy(String, "Send_AKENoStoredKm"); break;
	case XHDCP22_RX_TEST_STATE_SEND_AKESTOREDKM:
		strcpy(String, "Send_AKEStoredKm"); break;
	case XHDCP22_RX_TEST_STATE_WAIT_AKESENDHPRIME:
		strcpy(String, "Wait_AKESendHPrime"); break;
	case XHDCP22_RX_TEST_STATE_WAIT_AKESENDPAIRING:
		strcpy(String, "Wait_AKESendPairing"); break;
	case XHDCP22_RX_TEST_STATE_SEND_LCINIT:
		strcpy(String, "Send_LCInit"); break;
	case XHDCP22_RX_TEST_STATE_WAIT_LCSENDLPRIME:
		strcpy(String, "Wait_LCSendLPrime"); break;
	case XHDCP22_RX_TEST_STATE_SEND_SKESENDEKS:
		strcpy(String, "Send_SKESendEks"); break;
	case XHDCP22_RX_TEST_STATE_UPDATE_TOPOLOGY:
		strcpy(String, "Update_Topology"); break;
	case XHDCP22_RX_TEST_STATE_WAIT_RECEIVERIDLIST:
		strcpy(String, "Wait_ReceiverIdList"); break;
	case XHDCP22_RX_TEST_STATE_SEND_RECEIVERIDLISTACK:
		strcpy(String, "Send_ReceiverIdAck"); break;
	case XHDCP22_RX_TEST_STATE_SEND_STREAMMANAGEMENT:
		strcpy(String, "Send_StreamManagement"); break;
	case XHDCP22_RX_TEST_STATE_WAIT_STREAMREADY:
		strcpy(String, "Wait_StreamReady"); break;
	case XHDCP22_RX_TEST_STATE_WAIT_REAUTHREQ:
		strcpy(String, "Wait_ReauthReq"); break;
	case XHDCP22_RX_TEST_STATE_WAIT_REPEATERREADY:
		strcpy(String, "Wait_RepeaterReady"); break;
	case XHDCP22_RX_TEST_STATE_WAIT_AUTHENTICATED:
		strcpy(String, "Wait_Authenticated"); break;
	default:
		print("ERROR: Unexpected test event\r\n"); break;
	}
}

/****************************************************************************/
/**
* This function is used to send the AKEInit message.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
*****************************************************************************/
static int XHdcp22Rx_TestSendAKEInit(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u8 MessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE+1];
	XHdcp22_Rx_Message *WriteMsgPtr = (XHdcp22_Rx_Message *)(MessageBuffer+1);
	int Status;

	/* Check HDCP2 version */
	MessageBuffer[0] = XHDCP22_RX_DDC_VERSION_REG;
	XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS, 1, MessageBuffer, FALSE);
	XHdcp22Rx_TestDdcReadReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS, 1, MessageBuffer, FALSE);
	if(MessageBuffer[0] != 0x04)
	{
		return XST_FAILURE;
	}

	/* Generate message */
	WriteMsgPtr->AKEInit.MsgId = XHDCP22_RX_MSG_ID_AKEINIT;
	memcpy(WriteMsgPtr->AKEInit.Rtx,
		XHdcp22_Rx_Test_Rtx[InstancePtr->Test.TestReceiver],
		sizeof(XHdcp22_Rx_Test_Rtx[InstancePtr->Test.TestReceiver]));
	memcpy(WriteMsgPtr->AKEInit.TxCaps,
		XHdcp22_Rx_Test_TxCaps[InstancePtr->Test.TestReceiver],
		sizeof(XHdcp22_Rx_Test_TxCaps[InstancePtr->Test.TestReceiver]));

	/* Write message */
	MessageBuffer[0] = XHDCP22_RX_DDC_WRITE_REG;
	Status = XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				sizeof(XHdcp22_Rx_AKEInit)+1, MessageBuffer, FALSE);

	/* Log message */
	XHdcp22Rx_TestPrintMessage(InstancePtr, WriteMsgPtr, XHDCP22_RX_MSG_ID_AKEINIT);

	return Status;
}

/****************************************************************************/
/**
* This function is used to send the AKENoStoredKm message.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
*****************************************************************************/
static int XHdcp22Rx_TestSendAKENoStoredKm(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u8 MessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE+1];
	XHdcp22_Rx_Message *WriteMsgPtr = (XHdcp22_Rx_Message *)(MessageBuffer+1);
	u32 Status;

	/* Generate message */
    memcpy(WriteMsgPtr->AKENoStoredKm.EKpubKm,
		XHdcp22_Rx_Test_Ekm[InstancePtr->Test.TestReceiver],
		sizeof(XHdcp22_Rx_Test_Ekm[InstancePtr->Test.TestReceiver]));
	WriteMsgPtr->AKENoStoredKm.MsgId = XHDCP22_RX_MSG_ID_AKENOSTOREDKM;

	/* Write message */
	MessageBuffer[0] = XHDCP22_RX_DDC_WRITE_REG;
	Status = XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				sizeof(XHdcp22_Rx_AKENoStoredKm)+1, MessageBuffer, FALSE);

	/* Log message */
	XHdcp22Rx_TestPrintMessage(InstancePtr, WriteMsgPtr, XHDCP22_RX_MSG_ID_AKENOSTOREDKM);

	return Status;
}

/****************************************************************************/
/**
* This function is used to send the AKEStoredKm message.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
*****************************************************************************/
static int XHdcp22Rx_TestSendAKEStoredKm(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u8 MessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE+1];
	XHdcp22_Rx_Message *WriteMsgPtr = (XHdcp22_Rx_Message *)(MessageBuffer+1);
	u8  M[XHDCP22_RX_RTX_SIZE+XHDCP22_RX_RRX_SIZE];
	int Status;

    /* Concatenate M = (Rtx || Rrx) */
    memcpy(M, XHdcp22_Rx_Test_Rtx[InstancePtr->Test.TestReceiver], XHDCP22_RX_RTX_SIZE);
    memcpy(M+XHDCP22_RX_RTX_SIZE, InstancePtr->Params.Rrx, XHDCP22_RX_RRX_SIZE);

    /* Generate message */
	WriteMsgPtr->AKEStoredKm.MsgId = XHDCP22_RX_MSG_ID_AKESTOREDKM;
	memcpy(WriteMsgPtr->AKEStoredKm.EKhKm,
		XHdcp22_Rx_Test_EKh[InstancePtr->Test.TestReceiver],
		sizeof(XHdcp22_Rx_Test_EKh[InstancePtr->Test.TestReceiver]));
	memcpy(WriteMsgPtr->AKEStoredKm.M, M, sizeof(M));

	/* Write message */
	MessageBuffer[0] = XHDCP22_RX_DDC_WRITE_REG;
	Status = XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				sizeof(XHdcp22_Rx_AKEStoredKm)+1, MessageBuffer, FALSE);

	/* Log message */
	XHdcp22Rx_TestPrintMessage(InstancePtr, WriteMsgPtr, XHDCP22_RX_MSG_ID_AKESTOREDKM);

	return Status;
}

/****************************************************************************/
/**
* This function is used to send the LCInit message.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
*****************************************************************************/
static int XHdcp22Rx_TestSendLCInit(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u8 MessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE+1];
	XHdcp22_Rx_Message *WriteMsgPtr = (XHdcp22_Rx_Message *)(MessageBuffer+1);
	int Status;

	/* Generate message */
	WriteMsgPtr->LCInit.MsgId = XHDCP22_RX_MSG_ID_LCINIT;
	memcpy(WriteMsgPtr->LCInit.Rn,
		XHdcp22_Rx_Test_Rn[InstancePtr->Test.TestReceiver],
		sizeof(XHdcp22_Rx_Test_Rn[InstancePtr->Test.TestReceiver]));

	/* Write message */
	MessageBuffer[0] = XHDCP22_RX_DDC_WRITE_REG;
	Status = XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				sizeof(XHdcp22_Rx_LCInit)+1, MessageBuffer, FALSE);

	/* Log message */
	XHdcp22Rx_TestPrintMessage(InstancePtr, WriteMsgPtr, XHDCP22_RX_MSG_ID_LCINIT);

	return Status;
}

/****************************************************************************/
/**
* This function is used to send the SKESendEks message.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
*****************************************************************************/
static int XHdcp22Rx_TestSendSKESendEks(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u8 MessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE+1];
	XHdcp22_Rx_Message *WriteMsgPtr = (XHdcp22_Rx_Message *)(MessageBuffer+1);
	int Status;

	/* Generate message */
	WriteMsgPtr->SKESendEks.MsgId = XHDCP22_RX_MSG_ID_SKESENDEKS;
	memcpy(WriteMsgPtr->SKESendEks.EDkeyKs,
		XHdcp22_Rx_Test_EKs[InstancePtr->Test.TestReceiver],
		sizeof(XHdcp22_Rx_Test_EKs[InstancePtr->Test.TestReceiver]));
	memcpy(WriteMsgPtr->SKESendEks.Riv,
		XHdcp22_Rx_Test_Riv[InstancePtr->Test.TestReceiver],
		sizeof(XHdcp22_Rx_Test_Riv[InstancePtr->Test.TestReceiver]));

	/* Write message */
	MessageBuffer[0] = XHDCP22_RX_DDC_WRITE_REG;
	Status = XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				sizeof(XHdcp22_Rx_SKESendEks)+1, MessageBuffer, FALSE);

	/* Log message*/
	XHdcp22Rx_TestPrintMessage(InstancePtr, WriteMsgPtr, XHDCP22_RX_MSG_ID_SKESENDEKS);

	return Status;
}

/****************************************************************************/
/**
* This function is used to send the RepeaterAuth_Send_Ack message.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
*****************************************************************************/
static int XHdcp22Rx_TestSendReceiverIdListAck(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u8 MessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE+1];
	XHdcp22_Rx_Message *WriteMsgPtr = (XHdcp22_Rx_Message *)(MessageBuffer+1);
	int Status;

	/* Generate message */
	WriteMsgPtr->RepeaterAuthSendAck.MsgId = XHDCP22_RX_MSG_ID_REPEATERAUTHSENDACK;
	memcpy(WriteMsgPtr->RepeaterAuthSendAck.V,
		XHdcp22_Rx_Test_Repeater_V,
		sizeof(XHdcp22_Rx_Test_Repeater_V));

	/* Write message */
	MessageBuffer[0] = XHDCP22_RX_DDC_WRITE_REG;
	Status = XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				sizeof(XHdcp22_Rx_RepeaterAuthSendAck)+1, MessageBuffer, FALSE);

	/* Log message*/
	XHdcp22Rx_TestPrintMessage(InstancePtr, WriteMsgPtr, XHDCP22_RX_MSG_ID_REPEATERAUTHSENDACK);

	return Status;
}

/****************************************************************************/
/**
* This function is used to send the RepeaterAuth_Stream_Manage message.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
*****************************************************************************/
static int  XHdcp22Rx_TestSendStreamManagement(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u8 MessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE+1];
	XHdcp22_Rx_Message *WriteMsgPtr = (XHdcp22_Rx_Message *)(MessageBuffer+1);
	int Status;

	u8 SeqNumM[] = {0x00, 0x00, 0x00};
	u8 K[] = {0x00, 0x01};
	u8 StreamIdType[] = {0x00, 0x01};

	/* Generate message */
	WriteMsgPtr->RepeaterAuthStreamManage.MsgId = XHDCP22_RX_MSG_ID_REPEATERAUTHSTREAMMANAGE;
	memcpy(WriteMsgPtr->RepeaterAuthStreamManage.SeqNumM, SeqNumM, sizeof(SeqNumM));
	memcpy(WriteMsgPtr->RepeaterAuthStreamManage.K, K, sizeof(K));
	memcpy(WriteMsgPtr->RepeaterAuthStreamManage.StreamIdType, StreamIdType, sizeof(StreamIdType));

	/* Write message */
	MessageBuffer[0] = XHDCP22_RX_DDC_WRITE_REG;
	Status = XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				sizeof(XHdcp22_Rx_RepeaterAuthStreamManage)+1, MessageBuffer, FALSE);

	/* Log message*/
	XHdcp22Rx_TestPrintMessage(InstancePtr, WriteMsgPtr, XHDCP22_RX_MSG_ID_REPEATERAUTHSTREAMMANAGE);

	return Status;
}

/****************************************************************************/
/**
* This function is used to receive the AKESendCert message.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		After waiting XHDCP22_RX_TEST_MAX_ITERATIONS the test
* 			will timeout and return failure.
*****************************************************************************/
static int XHdcp22Rx_TestReceiveAKESendCert(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u8 MessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE+1];
	XHdcp22_Rx_Message *ReadMsgPtr = (XHdcp22_Rx_Message *)(MessageBuffer+1);
	int Status;
	u16 Size;
	u8 ReauthReq, Ready;

	/* Read message size */
	XHdcp22Rx_TestGetRxStatus(InstancePtr, &Size, &ReauthReq, &Ready);

	/* Read message */
	if(Size > 0)
	{
		if((Size == sizeof(XHdcp22_Rx_AKESendCert)) && (ReauthReq == FALSE) && (Ready == FALSE))
		{
			MessageBuffer[0] = XHDCP22_RX_DDC_READ_REG;
			XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				1, MessageBuffer, FALSE);
			XHdcp22Rx_TestDdcReadReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				sizeof(XHdcp22_Rx_AKESendCert), (u8 *)ReadMsgPtr, FALSE);

			if(ReadMsgPtr->MsgId == XHDCP22_RX_MSG_ID_AKESENDCERT)
			{
				XHdcp22Rx_TestPrintMessage(InstancePtr, ReadMsgPtr, XHDCP22_RX_MSG_ID_AKESENDCERT);
				Status = XHdcp22Rx_TestCompare(InstancePtr, "CertRx",
							XHdcp22_Rx_Test_PublicCert[InstancePtr->Test.TestReceiver],
							ReadMsgPtr->AKESendCert.CertRx,
							sizeof(XHdcp22_Rx_Test_PublicCert[InstancePtr->Test.TestReceiver]));
				Status = XHdcp22Rx_TestCompare(InstancePtr, "Rrx",
							XHdcp22_Rx_Test_Rrx[InstancePtr->Test.TestReceiver],
							ReadMsgPtr->AKESendCert.Rrx,
							sizeof(XHdcp22_Rx_Test_Rrx[InstancePtr->Test.TestReceiver]));
				Status = XHdcp22Rx_TestCompare(InstancePtr, "RxCaps",
							XHdcp22_Rx_Test_RxCaps[InstancePtr->Test.TestReceiver],
							ReadMsgPtr->AKESendCert.RxCaps,
							sizeof(XHdcp22_Rx_Test_RxCaps[InstancePtr->Test.TestReceiver]));
				if(Status != XST_SUCCESS)
				{
					return XST_FAILURE;
				}
				else
				{
					InstancePtr->Test.WaitCounter = 0;
					return XST_SUCCESS;
				}
			}
			else
			{
				return XST_FAILURE;
			}
		}
		else
		{
			return XST_FAILURE;
		}
	}

	/* Check for maximum iterations */
	if(InstancePtr->Test.WaitCounter == XHDCP22_RX_TEST_MAX_ITERATIONS)
	{
		return XST_FAILURE;
	}
	else
	{
		InstancePtr->Test.WaitCounter++;
		return XST_DEVICE_BUSY;
	}
}

/****************************************************************************/
/**
* This function is used to receive the AKESendHPrime message.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		After waiting XHDCP22_RX_TEST_MAX_ITERATIONS the test
* 			will timeout and return failure.
*****************************************************************************/
static int XHdcp22Rx_TestReceiveAKESendHPrime(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u8 MessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE+1];
	XHdcp22_Rx_Message *ReadMsgPtr = (XHdcp22_Rx_Message *)(MessageBuffer+1);
	int Status;
	u16 Size;
	u8 ReauthReq, Ready;

	/* Read message size */
	XHdcp22Rx_TestGetRxStatus(InstancePtr, &Size, &ReauthReq, &Ready);

	/* Read message */
	if(Size > 0)
	{
		if((Size == sizeof(XHdcp22_Rx_AKESendHPrime)) && (ReauthReq == FALSE) && (Ready == FALSE))
		{
			MessageBuffer[0] = XHDCP22_RX_DDC_READ_REG;
			XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				1, MessageBuffer, FALSE);
			XHdcp22Rx_TestDdcReadReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				sizeof(XHdcp22_Rx_AKESendHPrime), (u8 *)ReadMsgPtr, FALSE);

			if(ReadMsgPtr->MsgId == XHDCP22_RX_MSG_ID_AKESENDHPRIME)
			{
				XHdcp22Rx_TestPrintMessage(InstancePtr, ReadMsgPtr, XHDCP22_RX_MSG_ID_AKESENDHPRIME);
				Status = XHdcp22Rx_TestCompare(InstancePtr, "HPrime",
							XHdcp22_Rx_Test_HPrime[InstancePtr->Test.TestReceiver],
							ReadMsgPtr->AKESendHPrime.HPrime, XHDCP22_RX_HPRIME_SIZE);

				if(Status != XST_SUCCESS)
				{
					return XST_FAILURE;
				}
				else
				{
					InstancePtr->Test.WaitCounter = 0;
					return XST_SUCCESS;
				}
			}
			else
			{
				return XST_FAILURE;
			}
		}
		else
		{
			return XST_FAILURE;
		}
	}

	/* Check for maximum iterations */
	if(InstancePtr->Test.WaitCounter == XHDCP22_RX_TEST_MAX_ITERATIONS)
	{
		return XST_FAILURE;
	}
	else
	{
		InstancePtr->Test.WaitCounter++;
		return XST_DEVICE_BUSY;
	}
}

/****************************************************************************/
/**
* This function is used to receive the AKESendPairingInfo message.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		After waiting XHDCP22_RX_TEST_MAX_ITERATIONS the test
* 			will timeout and return failure.
*****************************************************************************/
static int XHdcp22Rx_TestReceiveAKESendPairingInfo(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u8 MessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE+1];
	XHdcp22_Rx_Message *ReadMsgPtr = (XHdcp22_Rx_Message *)(MessageBuffer+1);
	int Status;
	u16 Size;
	u8 ReauthReq, Ready;

	/* Read message size */
	XHdcp22Rx_TestGetRxStatus(InstancePtr, &Size, &ReauthReq, &Ready);

	/* Read message */
	if(Size > 0)
	{
		if((Size == sizeof(XHdcp22_Rx_AKESendPairingInfo)) && (ReauthReq == FALSE) && (Ready == FALSE))
		{
			MessageBuffer[0] = XHDCP22_RX_DDC_READ_REG;
			XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				1, MessageBuffer, FALSE);
			XHdcp22Rx_TestDdcReadReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				sizeof(XHdcp22_Rx_AKESendPairingInfo), (u8 *)ReadMsgPtr, FALSE);

			if(ReadMsgPtr->MsgId == XHDCP22_RX_MSG_ID_AKESENDPAIRINGINFO)
			{
				XHdcp22Rx_TestPrintMessage(InstancePtr, ReadMsgPtr, XHDCP22_RX_MSG_ID_AKESENDPAIRINGINFO);
				Status = XHdcp22Rx_TestCompare(InstancePtr, "EKh",
					XHdcp22_Rx_Test_EKh[InstancePtr->Test.TestReceiver],
					ReadMsgPtr->AKESendPairingInfo.EKhKm, XHDCP22_RX_EKH_SIZE);

				if(Status != XST_SUCCESS)
				{
					return XST_FAILURE;
				}
				else
				{
					InstancePtr->Test.WaitCounter = 0;
					return XST_SUCCESS;
				}
			}
			else
			{
				return XST_FAILURE;
			}
		}
		else
		{
			return XST_FAILURE;
		}
	}

	/* Check for maximum iterations */
	if(InstancePtr->Test.WaitCounter == XHDCP22_RX_TEST_MAX_ITERATIONS)
	{
		return XST_FAILURE;
	}
	else
	{
		InstancePtr->Test.WaitCounter++;
		return XST_DEVICE_BUSY;
	}
}

/****************************************************************************/
/**
* This function is used to receive the LCSendLPrime message.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		After waiting XHDCP22_RX_TEST_MAX_ITERATIONS the test
* 			will timeout and return failure.
*****************************************************************************/
static int XHdcp22Rx_TestReceiveLCSendLPrime(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u8 MessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE+1];
	XHdcp22_Rx_Message *ReadMsgPtr = (XHdcp22_Rx_Message *)(MessageBuffer+1);
	int Status;
	u16 Size;
	u8 ReauthReq, Ready;

	/* Read message size */
	XHdcp22Rx_TestGetRxStatus(InstancePtr, &Size, &ReauthReq, &Ready);

	if(Size > 0)
	{
		if((Size == sizeof(XHdcp22_Rx_LCSendLPrime)) && (ReauthReq == FALSE) && (Ready == FALSE))
		{
			MessageBuffer[0] = XHDCP22_RX_DDC_READ_REG;
			XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				1, MessageBuffer, FALSE);
			XHdcp22Rx_TestDdcReadReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				sizeof(XHdcp22_Rx_LCSendLPrime), (u8 *)ReadMsgPtr, FALSE);

			if(ReadMsgPtr->MsgId == XHDCP22_RX_MSG_ID_LCSENDLPRIME)
			{
				XHdcp22Rx_TestPrintMessage(InstancePtr, ReadMsgPtr, XHDCP22_RX_MSG_ID_LCSENDLPRIME);
				Status = XHdcp22Rx_TestCompare(InstancePtr, "LPrime",
							XHdcp22_Rx_Test_LPrime[InstancePtr->Test.TestReceiver],
							ReadMsgPtr->LCSendLPrime.LPrime, XHDCP22_RX_LPRIME_SIZE);

				if(Status != XST_SUCCESS)
				{
					return XST_FAILURE;
				}
				else
				{
					InstancePtr->Test.WaitCounter = 0;
					return XST_SUCCESS;
				}
			}
			else
			{
				return XST_FAILURE;
			}
		}
		else
		{
			return XST_FAILURE;
		}
	}

	/* Check for maximum iterations */
	if(InstancePtr->Test.WaitCounter == XHDCP22_RX_TEST_MAX_ITERATIONS)
	{
		return XST_FAILURE;
	}
	else
	{
		InstancePtr->Test.WaitCounter++;
		return XST_DEVICE_BUSY;
	}
}

/****************************************************************************/
/**
* This function is used to receive the RepeaterAuth_Send_ReceiverID_List
* message.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		After waiting XHDCP22_RX_TEST_MAX_ITERATIONS the test
* 			will timeout and return failure.
*****************************************************************************/
static int XHdcp22Rx_TestReceiveReceiverIdList(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u8 MessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE+1];
	XHdcp22_Rx_Message *ReadMsgPtr = (XHdcp22_Rx_Message *)(MessageBuffer+1);
	int Status;
	u16 Size;
	u8 ReauthReq, Ready;
	u8 RxInfo[] = {0x02, 0x31};
	u8 SeqNumV[] = {0x00, 0x00, 0x00};

	/* Read message size */
	XHdcp22Rx_TestGetRxStatus(InstancePtr, &Size, &ReauthReq, &Ready);

	if(Ready == TRUE)
	{
		if((Size > 0) && (ReauthReq == FALSE))
		{
			MessageBuffer[0] = XHDCP22_RX_DDC_READ_REG;
			XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				1, MessageBuffer, FALSE);
			XHdcp22Rx_TestDdcReadReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				Size, (u8 *)ReadMsgPtr, FALSE);

			if(ReadMsgPtr->MsgId == XHDCP22_RX_MSG_ID_REPEATERAUTHSENDRXIDLIST)
			{
				XHdcp22Rx_TestPrintMessage(InstancePtr, ReadMsgPtr, XHDCP22_RX_MSG_ID_REPEATERAUTHSENDRXIDLIST);
				Status = XHdcp22Rx_TestCompare(InstancePtr, "RxInfo",
							RxInfo, ReadMsgPtr->RepeaterAuthSendRxIdList.RxInfo, sizeof(RxInfo));
				Status |= XHdcp22Rx_TestCompare(InstancePtr, "SeqNumV",
							SeqNumV, ReadMsgPtr->RepeaterAuthSendRxIdList.SeqNumV, sizeof(SeqNumV));
				Status |= XHdcp22Rx_TestCompare(InstancePtr, "ReceiverIdList",
							XHdcp22_Rx_Test_Repeater_ReceiverIdList,
							ReadMsgPtr->RepeaterAuthSendRxIdList.ReceiverIdList, sizeof(XHdcp22_Rx_Test_Repeater_ReceiverIdList));
				Status |= XHdcp22Rx_TestCompare(InstancePtr, "VPrime",
							XHdcp22_Rx_Test_Repeater_VPrime,
							ReadMsgPtr->RepeaterAuthSendRxIdList.VPrime, sizeof(XHdcp22_Rx_Test_Repeater_VPrime));

				if(Status != XST_SUCCESS)
				{
					return XST_FAILURE;
				}
				else
				{
					InstancePtr->Test.WaitCounter = 0;
					return XST_SUCCESS;
				}
			}
			else
			{
				return XST_FAILURE;
			}
		}
		else
		{
			return XST_FAILURE;
		}
	}

	/* Check for maximum iterations */
	if(InstancePtr->Test.WaitCounter == XHDCP22_RX_TEST_MAX_ITERATIONS)
	{
		return XST_FAILURE;
	}
	else
	{
		InstancePtr->Test.WaitCounter++;
		return XST_DEVICE_BUSY;
	}
}

/****************************************************************************/
/**
* This function is used to receive the RepeaterAuth_Stream_Ready message.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		After waiting XHDCP22_RX_TEST_MAX_ITERATIONS the test
* 			will timeout and return failure.
*****************************************************************************/
static int  XHdcp22Rx_TestReceiveStreamReady(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u8 MessageBuffer[XHDCP22_RX_MAX_MESSAGE_SIZE+1];
	XHdcp22_Rx_Message *ReadMsgPtr = (XHdcp22_Rx_Message *)(MessageBuffer+1);
	int Status;
	u16 Size;
	u8 ReauthReq, Ready;

	/* Read message size */
	XHdcp22Rx_TestGetRxStatus(InstancePtr, &Size, &ReauthReq, &Ready);

	if(Size > 0)
	{
		if((Size == sizeof(XHdcp22_Rx_RepeaterAuthStreamReady)) && (ReauthReq == FALSE) && (Ready == FALSE))
		{
			MessageBuffer[0] = XHDCP22_RX_DDC_READ_REG;
			XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				1, MessageBuffer, FALSE);
			XHdcp22Rx_TestDdcReadReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS,
				sizeof(XHdcp22_Rx_RepeaterAuthStreamReady), (u8 *)ReadMsgPtr, FALSE);

			if(ReadMsgPtr->MsgId == XHDCP22_RX_MSG_ID_REPEATERAUTHSTREAMREADY)
			{
				XHdcp22Rx_TestPrintMessage(InstancePtr, ReadMsgPtr, XHDCP22_RX_MSG_ID_REPEATERAUTHSTREAMREADY);
				Status = XHdcp22Rx_TestCompare(InstancePtr, "MPrime", XHdcp22_Rx_Test_Repeater_MPrime,
							ReadMsgPtr->RepeaterAuthStreamReady.MPrime, XHDCP22_RX_MPRIME_SIZE);

				if(Status != XST_SUCCESS)
				{
					return XST_FAILURE;
				}
				else
				{
					InstancePtr->Test.WaitCounter = 0;
					return XST_SUCCESS;
				}
			}
			else
			{
				return XST_FAILURE;
			}
		}
		else
		{
			return XST_FAILURE;
		}
	}

	/* Check for maximum iterations */
	if(InstancePtr->Test.WaitCounter == XHDCP22_RX_TEST_MAX_ITERATIONS)
	{
		return XST_FAILURE;
	}
	else
	{
		InstancePtr->Test.WaitCounter++;
		return XST_DEVICE_BUSY;
	}

}

/****************************************************************************/
/**
* This function updates the repeater topology table for the repeater
* upstream interface.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
*****************************************************************************/
static int XHdcp22Rx_TestUpdateTopology(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Set repeater topology table fields */
	XHdcp22Rx_SetTopologyReceiverIdList(InstancePtr,
		XHdcp22_Rx_Test_Repeater_ReceiverIdList, 3);
	XHdcp22Rx_SetTopologyDepth(InstancePtr, 1);
	XHdcp22Rx_SetTopologyDeviceCnt(InstancePtr, 3);
	XHdcp22Rx_SetTopologyMaxDevsExceeded(InstancePtr, FALSE);
	XHdcp22Rx_SetTopologyMaxCascadeExceeded(InstancePtr, FALSE);
	XHdcp22Rx_SetTopologyHdcp20RepeaterDownstream(InstancePtr, FALSE);
	XHdcp22Rx_SetTopologyHdcp1DeviceDownstream(InstancePtr, TRUE);

	/* Trigger receiver to propagate topology table upstream */
	XHdcp22Rx_SetTopologyUpdate(InstancePtr);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This function checks if the REAUTH_REQ bit in the RxStatus register
* is asserted.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
*****************************************************************************/
static int XHdcp22Rx_TestWaitReauthReq(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u16 Size;
	u8 ReauthReq, Ready;

	XHdcp22Rx_TestGetRxStatus(InstancePtr, &Size, &ReauthReq, &Ready);

	/* Check if REAUTH_REQ bit is set */
	if (ReauthReq == TRUE)
	{
		if(Ready == FALSE)
			return XST_SUCCESS;
		else
			return XST_FAILURE;
	}

	/* Check for maximum iterations, should be greater than 2 seconds */
	if(InstancePtr->Test.WaitCounter == 100000)
	{
		return XST_FAILURE;
	}
	else
	{
		InstancePtr->Test.WaitCounter++;
		return XST_DEVICE_BUSY;
	}
}

/****************************************************************************/
/**
* This function checks if the READY bit in the RxStatus register
* is asserted.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
*****************************************************************************/
static int XHdcp22Rx_TestWaitRepeaterReady(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u16 Size;
	u8 ReauthReq, Ready;

	XHdcp22Rx_TestGetRxStatus(InstancePtr, &Size, &ReauthReq, &Ready);

	return ((Ready == TRUE) && (ReauthReq == FALSE)) ? XST_SUCCESS : XST_DEVICE_BUSY;
}

/****************************************************************************/
/**
* This function checks if the receiver has reached the authenticated state.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		After waiting XHDCP22_RX_TEST_MAX_ITERATIONS the test
* 			will timeout and return failure.
*****************************************************************************/
static int XHdcp22Rx_TestWaitAuthenticated(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status;
	u16 Size;
	u8 ReauthReq, Ready;

	/* Check RxStatus */
	XHdcp22Rx_TestGetRxStatus(InstancePtr, &Size, &ReauthReq, &Ready);
	if((Size != 0) || (ReauthReq == TRUE) || (Ready == TRUE))
	{
		return XST_FAILURE;
	}

	/* Check authentication status */
	Status = XHdcp22Rx_IsAuthenticated(InstancePtr);
	if(Status)
	{
		Status = XHdcp22Rx_TestCompare(InstancePtr, "Ks",
					XHdcp22_Rx_Test_Ks[InstancePtr->Test.TestReceiver],
					InstancePtr->Params.Ks, XHDCP22_RX_KS_SIZE);

		if(Status != XST_SUCCESS)
		{
			return XST_FAILURE;
		}
		else
		{
			InstancePtr->Test.WaitCounter = 0;
			return XST_SUCCESS;
		}
	}

	/* Check for maximum iterations */
	if(InstancePtr->Test.WaitCounter == XHDCP22_RX_TEST_MAX_ITERATIONS)
	{
		return XST_FAILURE;
	}
	else
	{
		InstancePtr->Test.WaitCounter++;
		return XST_DEVICE_BUSY;
	}
}

/****************************************************************************/
/**
* This function sets the test DDC register map address to be updated. This
* function should be called before XHdcp22Rx_TestDdcSetDataCallback.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
* @param	Addr is the DDC register map address to be updated.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		None.
*****************************************************************************/
static int XHdcp22Rx_TestDdcSetAddressCallback(XHdcp22_Rx *InstancePtr, u32 Addr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u32 Offset;

	/* Check address in range */
	Offset = XHdcp22Rx_TestDdcGetRegisterMapIndex(InstancePtr, Addr);
	if(Offset >= XHDCP22_RX_TEST_DDC_REGMAP_SIZE)
	{
		return XST_FAILURE;
	}

	/* Update register map address */
	InstancePtr->Test.DdcRegisterMapAddress = Addr;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This function sets the test DDC register map data for the address
* previously defined by XHdcp22Rx_TestDdcSetAddressCallback.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
* @param	Data is the value to update the DDC register map address.
*
* @return	XST_SUCCESS or XST_FAILURE.
*
* @note		Does not support auto-increment.
*****************************************************************************/
static int XHdcp22Rx_TestDdcSetDataCallback(XHdcp22_Rx *InstancePtr, u32 Data)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Offset;

	/* Check address in range */
	Offset = XHdcp22Rx_TestDdcGetRegisterMapIndex(InstancePtr, InstancePtr->Test.DdcRegisterMapAddress);
	if(Offset >= XHDCP22_RX_TEST_DDC_REGMAP_SIZE)
	{
		return XST_FAILURE;
	}

	/* Check register access then do set */
	if((InstancePtr->Test.DdcRegisterMap[Offset].Access == XHDCP22_RX_TEST_DDC_ACCESS_RO) ||
		(InstancePtr->Test.DdcRegisterMap[Offset].Access == XHDCP22_RX_TEST_DDC_ACCESS_RW))
	{
		switch(InstancePtr->Test.DdcRegisterMapAddress)
		{
		case XHDCP22_RX_DDC_READ_REG:
			/* Set tail of buffer */
			InstancePtr->Test.ReadMessageBuffer[InstancePtr->Test.ReadMessageSize] = (u8)Data;
			InstancePtr->Test.ReadMessageSize++;
			break;
		default:
			InstancePtr->Test.DdcRegisterMap[Offset].Value = (u8)Data;
			break;
		}

		return XST_SUCCESS;
	}

	return XST_FAILURE;
}

/****************************************************************************/
/**
* This function gets the test DDC register map data for the address
* previously defined by XHdcp22Rx_TestDdcSetAddressCallback.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	Register data of type u32. For an undefined address return
* 			0xDEADBEEF.
*
* @note		Does not support auto-increment.
*****************************************************************************/
static u32 XHdcp22Rx_TestDdcGetDataCallback(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	u32 Offset;

	/* Check address in range */
	Offset = XHdcp22Rx_TestDdcGetRegisterMapIndex(InstancePtr, InstancePtr->Test.DdcRegisterMapAddress);
	if(Offset >= XHDCP22_RX_TEST_DDC_REGMAP_SIZE)
	{
		return 0xDEADBEEF;
	}

	/* Check register access then do get */
	switch(InstancePtr->Test.DdcRegisterMapAddress)
	{
	case XHDCP22_RX_DDC_WRITE_REG:
		/* Get from head of buffer */
		InstancePtr->Test.WriteMessageOffset++;
		return ((InstancePtr->Test.WriteMessageOffset-1) < (InstancePtr->Test.WriteMessageSize)) ?
				(u32)InstancePtr->Test.WriteMessageBuffer[InstancePtr->Test.WriteMessageOffset-1] :
				0xDEADBEEF;
	default:
		return (u32)InstancePtr->Test.DdcRegisterMap[Offset].Value;
	}

	return 0xDEADBEEF;
}

/****************************************************************************/
/**
* This function returns the current size of the DDC read message buffer.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	Read message buffer size in bytes.
*
* @note		None.
*****************************************************************************/
static u32 XHdcp22Rx_TestDdcGetReadBufferSizeCallback(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return InstancePtr->Test.ReadMessageSize;
}

/****************************************************************************/
/**
* This function is used to check if the read message buffer is empty.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	TRUE or FALSE.
*
* @note		None.
*****************************************************************************/
static u8 XHdcp22Rx_TestDdcIsReadBufferEmptyCallback(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (InstancePtr->Test.WriteMessageSize == 0) ? TRUE : FALSE;
}

/****************************************************************************/
/**
* This function returns the current size of the write message buffer.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	Write message buffer size in bytes.
*
* @note		None.
*****************************************************************************/
static u32 XHdcp22Rx_TestDdcGetWriteBufferSizeCallback(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return InstancePtr->Test.WriteMessageSize;
}

/****************************************************************************/
/**
* This function is used to check if the write message buffer is emtpy.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	TRUE or FALSE.
*
* @note		None.
*****************************************************************************/
static u8  XHdcp22Rx_TestDdcIsWriteBufferEmptyCallback(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (InstancePtr->Test.WriteMessageSize == 0) ? TRUE : FALSE;
}

/****************************************************************************/
/**
* This function clears the read message buffer and resets the size.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	None.
*
* @note		None.
*****************************************************************************/
static void XHdcp22Rx_TestDdcClearReadBufferCallback(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	u32 Offset;

	/* Get reg map offset */
	Offset = XHdcp22Rx_TestDdcGetRegisterMapIndex(InstancePtr, XHDCP22_RX_DDC_READ_REG);

	/* Clear read message buffer */
	memset(InstancePtr->Test.ReadMessageBuffer, 0, XHDCP22_RX_MAX_MESSAGE_SIZE);
	InstancePtr->Test.ReadMessageSize = 0;
	InstancePtr->Test.ReadMessageOffset = 0;
	InstancePtr->Test.DdcRegisterMap[Offset].Value = 0;

	/* Clear read message size */
	Offset = XHdcp22Rx_TestDdcGetRegisterMapIndex(InstancePtr, XHDCP22_RX_DDC_RXSTATUS0_REG);
	InstancePtr->Test.DdcRegisterMap[Offset].Value = 0;
	Offset = XHdcp22Rx_TestDdcGetRegisterMapIndex(InstancePtr, XHDCP22_RX_DDC_RXSTATUS1_REG);
	InstancePtr->Test.DdcRegisterMap[Offset].Value = 0;
}

/****************************************************************************/
/**
* This function clears the write message buffer and resets the size.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	None.
*
* @note		None.
*****************************************************************************/
static void XHdcp22Rx_TestDdcClearWriteBufferCallback(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	u32 Offset;

	/* Get reg map offset */
	Offset = XHdcp22Rx_TestDdcGetRegisterMapIndex(InstancePtr, XHDCP22_RX_DDC_WRITE_REG);

	/* Clear write message buffer */
	memset(InstancePtr->Test.WriteMessageBuffer, 0, XHDCP22_RX_MAX_MESSAGE_SIZE);
	InstancePtr->Test.WriteMessageOffset = 0;
	InstancePtr->Test.WriteMessageSize = 0;
	InstancePtr->Test.DdcRegisterMap[Offset].Value = 0;
}

/****************************************************************************/
/**
* This function is a callback that is executed when the receiver
* state machine has received a repeater authentication request.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	None.
*
* @note		None.
*****************************************************************************/
static void XHdcp22Rx_TestRepeaterAuthRequestCallback(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Write a test value to the log */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_USER, 1);
}

/****************************************************************************/
/**
* This function is a callback that is executed when the receiver
* state machine has received a repeater manage request.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	None.
*
* @note		None.
*****************************************************************************/
static void XHdcp22Rx_TestRepeaterManageRequestCallback(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	u8 TypeExpected = 1;
	u8 Type;

	/* Check content type */
	Type = XHdcp22Rx_GetContentStreamType(InstancePtr);
	XHdcp22Rx_TestCompare(InstancePtr, "Type", &TypeExpected, &Type, 1);

	/* Write a test value to the log */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_USER, 2);
}

/****************************************************************************/
/**
* This function is a callback that is executed when the receiver
* state machine has transitioned to State B4 or C8.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	None.
*
* @note		None.
*****************************************************************************/
static void XHdcp22Rx_TestAuthenticatedCallback(XHdcp22_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Write a test value to the log */
	XHdcp22Rx_LogWr(InstancePtr, XHDCP22_RX_LOG_EVT_USER, 3);
}

/****************************************************************************/
/**
* This function reads the RxStatus register and returns the message
* Size, REAUTH_REQ bit,  and READY bit.
*
* @param	InstancePtr is a pointer to an XHdcp22_Rx instance.
*
* @return	None.
*
* @note		None.
*****************************************************************************/
static void XHdcp22Rx_TestGetRxStatus(XHdcp22_Rx *InstancePtr, u16 *Size, u8 *ReauthReq, u8 *Ready)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ReauthReq != NULL);
	Xil_AssertVoid(Ready != NULL);

	u8 MessageBuffer[2];

	/* Read RxStatus1[15:0] */
	MessageBuffer[0] = XHDCP22_RX_DDC_RXSTATUS0_REG;
	XHdcp22Rx_TestDdcWriteReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS, 1, MessageBuffer, FALSE);
	XHdcp22Rx_TestDdcReadReg(InstancePtr, XHDCP22_RX_TEST_DDC_BASE_ADDRESS, 2, MessageBuffer, FALSE);

	/* Extract fields */
	*Size = *(u16 *)MessageBuffer & 0x3FF;
	if(*(u16 *)MessageBuffer & 0x400)
		*Ready = TRUE;
	else
		*Ready = FALSE;
	if(*(u16 *)MessageBuffer & 0x800)
		*ReauthReq = TRUE;
	else
		*ReauthReq = FALSE;
}

#endif // _XHDCP22_RX_TEST_

/** @} */
