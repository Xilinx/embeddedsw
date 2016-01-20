/******************************************************************************
*
* Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xhdcp22_tx_test.c
*
* This is a test and logging file for the Xilinx HDCP22 TX core.
* Testing is done by acting as a stub for the DDC handlers. Some
* functions return a test-vector as specified by the
* "Errata to HDCP on HDMI Specification Revision 2.2" document.
* Further logging functionality is available, using a buffer for logging
* events. The log buffer also acts as a results buffer for unit tests.
* A unit test can execute authentication, and check if the results in the
* log-buffer match the expected results. Using test flags, some errors can be
* simulated.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  JO     06/17/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xhdcp22_tx_i.h"
#include "string.h"
#include "stdlib.h"

/************************** Constant Definitions *****************************/

/*
 * All described testvectors are for testing purposes only and can be found in
 * "Errata to HDCP on HDMI specification appendix E.pdf"
 */

/** TX: Global constant */
static const u8 XHdcp22_Tx_Test_LC128[] =
{
	0x93, 0xCE, 0x5A, 0x56, 0xA0, 0xA1, 0xF4, 0xF7, 0x3C, 0x65, 0x8A, 0x1B, 0xD2, 0xAE,
	0xF0, 0xF7
};

/** TX: public key to use for signature verification (modulus and exponent)*/
static const u8 XHdcp22_Tx_Test_Kpubdcp[] =
{
	0xA2, 0xC7, 0x55, 0x57, 0x54, 0xCB, 0xAA, 0xA7, 0x7A, 0x27, 0x92, 0xC3, 0x1A, 0x6D, 0xC2, 0x31, 0xCF, 0x12,
	0xC2, 0x24, 0xBF, 0x89, 0x72, 0x46, 0xA4, 0x8D, 0x20, 0x83, 0xB2, 0xDD, 0x04, 0xDA, 0x7E, 0x01, 0xA9, 0x19,
	0xEF, 0x7E, 0x8C, 0x47, 0x54, 0xC8, 0x59, 0x72, 0x5C, 0x89, 0x60, 0x62, 0x9F, 0x39, 0xD0, 0xE4, 0x80, 0xCA,
	0xA8, 0xD4, 0x1E, 0x91, 0xE3, 0x0E, 0x2C, 0x77, 0x55, 0x6D, 0x58, 0xA8, 0x9E, 0x3E, 0xF2, 0xDA, 0x78, 0x3E,
	0xBA, 0xD1, 0x05, 0x37, 0x07, 0xF2, 0x88, 0x74, 0x0C, 0xBC, 0xFB, 0x68, 0xA4, 0x7A, 0x27, 0xAD, 0x63, 0xA5,
	0x1F, 0x67, 0xF1, 0x45, 0x85, 0x16, 0x49, 0x8A, 0xE6, 0x34, 0x1C, 0x6E, 0x80, 0xF5, 0xFF, 0x13, 0x72, 0x85,
	0x5D, 0xC1, 0xDE, 0x5F, 0x01, 0x86, 0x55, 0x86, 0x71, 0xE8, 0x10, 0x33, 0x14, 0x70, 0x2A, 0x5F, 0x15, 0x7B,
	0x5C, 0x65, 0x3C, 0x46, 0x3A, 0x17, 0x79, 0xED, 0x54, 0x6A, 0xA6, 0xC9, 0xDF, 0xEB, 0x2A, 0x81, 0x2A, 0x80,
	0x2A, 0x46, 0xA2, 0x06, 0xDB, 0xFD, 0xD5, 0xF3, 0xCF, 0x74, 0xBB, 0x66, 0x56, 0x48, 0xD7, 0x7C, 0x6A, 0x03,
	0x14, 0x1E, 0x55, 0x56, 0xE4, 0xB6, 0xFA, 0x38, 0x2B, 0x5D, 0xFB, 0x87, 0x9F, 0x9E, 0x78, 0x21, 0x87, 0xC0,
	0x0C, 0x63, 0x3E, 0x8D, 0x0F, 0xE2, 0xA7, 0x19, 0x10, 0x9B, 0x15, 0xE1, 0x11, 0x87, 0x49, 0x33, 0x49, 0xB8,
	0x66, 0x32, 0x28, 0x7C, 0x87, 0xF5, 0xD2, 0x2E, 0xC5, 0xF3, 0x66, 0x2F, 0x79, 0xEF, 0x40, 0x5A, 0xD4, 0x14,
	0x85, 0x74, 0x5F, 0x06, 0x43, 0x50, 0xCD, 0xDE, 0x84, 0xE7, 0x3C, 0x7D, 0x8E, 0x8A, 0x49, 0xCC, 0x5A, 0xCF,
	0x73, 0xA1, 0x8A, 0x13, 0xFF, 0x37, 0x13, 0x3D, 0xAD, 0x57, 0xD8, 0x51, 0x22, 0xD6, 0x32, 0x1F, 0xC0, 0x68,
	0x4C, 0xA0, 0x5B, 0xDD, 0x5F, 0x78, 0xC8, 0x9F, 0x2D, 0x3A, 0xA2, 0xB8, 0x1E, 0x4A, 0xE4, 0x08, 0x55, 0x64,
	0x05, 0xE6, 0x94, 0xFB, 0xEB, 0x03, 0x6A, 0x0A, 0xBE, 0x83, 0x18, 0x94, 0xD4, 0xB6, 0xC3, 0xF2, 0x58, 0x9C,
	0x7A, 0x24, 0xDD, 0xD1, 0x3A, 0xB7, 0x3A, 0xB0, 0xBB, 0xE5, 0xD1, 0x28, 0xAB, 0xAD, 0x24, 0x54, 0x72, 0x0E,
	0x76, 0xD2, 0x89, 0x32, 0xEA, 0x46, 0xD3, 0x78, 0xD0, 0xA9, 0x67, 0x78, 0xC1, 0x2D, 0x18, 0xB0, 0x33, 0xDE,
	0xDB, 0x27, 0xCC, 0xB0, 0x7C, 0xC9, 0xA4, 0xBD, 0xDF, 0x2B, 0x64, 0x10, 0x32, 0x44, 0x06, 0x81, 0x21, 0xB3,
	0xBA, 0xCF, 0x33, 0x85, 0x49, 0x1E, 0x86, 0x4C, 0xBD, 0xF2, 0x3D, 0x34, 0xEF, 0xD6, 0x23, 0x7A, 0x9F, 0x2C,
	0xDA, 0x84, 0xF0, 0x83, 0x83, 0x71, 0x7D, 0xDA, 0x6E, 0x44, 0x96, 0xCD, 0x1D, 0x05, 0xDE, 0x30, 0xF6, 0x1E,
	0x2F, 0x9C, 0x99, 0x9C, 0x60, 0x07, 0x03
};

/** Authentication and key exchange
 * Tx->Rx AKE_INIT: hardcoded rtx is normally generated by TX
 */
static const u8 XHdcp22_Tx_Test_Rtx[] = { 0xF9, 0xF1, 0x30, 0xA8, 0x2D, 0x5B, 0xE5, 0xC3 };

/** Tx->Rx AKE_INIT: tx caps */
static const u8 XHdcp22_Tx_Test_TxCaps[] = { 0x02, 0x00, 0x00 };

/** Rx->Tx AKE_Send_Cert: Rx certificate including receiver ID */
static const u8 XHdcp22_Tx_Test_CertRx[] =
{
	0x8B, 0xA4, 0x47, 0x42, 0xFB, 0xE4, 0x68, 0x63, 0x8A, 0xDA, 0x97, 0x2D, 0xDE, 0x9A, 0x8D,
	0x1C, 0xB1, 0x65, 0x4B, 0x85, 0x8D, 0xE5, 0x46, 0xD6, 0xDB, 0x95, 0xA5, 0xF6, 0x66, 0x74,
	0xEA, 0x81, 0x0B, 0x9A, 0x58, 0x58, 0x66, 0x26, 0x86, 0xA6, 0xB4, 0x56, 0x2B, 0x29, 0x43,
	0xE5, 0xBB, 0x81, 0x74, 0x86, 0xA7, 0xB7, 0x16, 0x2F, 0x07, 0xEC, 0xD1, 0xB5, 0xF9, 0xAE,
	0x4F, 0x98, 0x89, 0xA9, 0x91, 0x7D, 0x58, 0x5B, 0x8D, 0x20, 0xD5, 0xC5, 0x08, 0x40, 0x3B,
	0x86, 0xAF, 0xF4, 0xD6, 0xB9, 0x20, 0x95, 0xE8, 0x90, 0x3B, 0x8F, 0x9F, 0x36, 0x5B, 0x46,
	0xB6, 0xD4, 0x1E, 0xF5, 0x05, 0x88, 0x80, 0x14, 0xE7, 0x2C, 0x77, 0x5D, 0x6E, 0x54, 0xE9,
	0x65, 0x81, 0x5A, 0x68, 0x92, 0xA5, 0xD6, 0x40, 0x78, 0x11, 0x97, 0x65, 0xD7, 0x64, 0x36,
	0x5E, 0x8D, 0x2A, 0x87, 0xA8, 0xEB, 0x7D, 0x06, 0x2C, 0x10, 0xF8, 0x0A, 0x7D, 0x01, 0x00,
	0x01, 0x10, 0x00, 0x06, 0x40, 0x99, 0x8F, 0x5A, 0x54, 0x71, 0x23, 0xA7, 0x6A, 0x64, 0x3F,
	0xBD, 0xDD, 0x52, 0xB2, 0x79, 0x6F, 0x88, 0x26, 0x94, 0x9E, 0xAF, 0xA4, 0xDE, 0x7D, 0x8D,
	0x88, 0x10, 0xC8, 0xF6, 0x56, 0xF0, 0x8F, 0x46, 0x28, 0x48, 0x55, 0x51, 0xC5, 0xAF, 0xA1,
	0xA9, 0x9D, 0xAC, 0x9F, 0xB1, 0x26, 0x4B, 0xEB, 0x39, 0xAD, 0x88, 0x46, 0xAF, 0xBC, 0x61,
	0xA8, 0x7B, 0xF9, 0x7B, 0x3E, 0xE4, 0x95, 0xD9, 0xA8, 0x79, 0x48, 0x51, 0x00, 0xBE, 0xA4,
	0xB6, 0x96, 0x7F, 0x3D, 0xFD, 0x76, 0xA6, 0xB7, 0xBB, 0xB9, 0x77, 0xDC, 0x54, 0xFB, 0x52,
	0x9C, 0x79, 0x8F, 0xED, 0xD4, 0xB1, 0xBC, 0x0F, 0x7E, 0xB1, 0x7E, 0x70, 0x6D, 0xFC, 0xB9,
	0x7E, 0x66, 0x9A, 0x86, 0x23, 0x3A, 0x98, 0x5E, 0x32, 0x8D, 0x75, 0x18, 0x54, 0x64, 0x36,
	0xDD, 0x92, 0x01, 0x39, 0x90, 0xB9, 0xE3, 0xAF, 0x6F, 0x98, 0xA5, 0xC0, 0x80, 0xC6, 0x2F,
	0xA1, 0x02, 0xAD, 0x8D, 0xF4, 0xD6, 0x66, 0x7B, 0x45, 0xE5, 0x74, 0x18, 0xB1, 0x27, 0x24,
	0x01, 0x1E, 0xEA, 0xD8, 0xF3, 0x79, 0x92, 0xE9, 0x03, 0xF5, 0x57, 0x8D, 0x65, 0x2A, 0x8D,
	0x1B, 0xF0, 0xDA, 0x58, 0x3F, 0x58, 0xA0, 0xF4, 0xB4, 0xBE, 0xCB, 0x21, 0x66, 0xE9, 0x21,
	0x7C, 0x76, 0xF3, 0xC1, 0x7E, 0x2E, 0x7C, 0x3D, 0x61, 0x20, 0x1D, 0xC5, 0xC0, 0x71, 0x28,
	0x2E, 0xB7, 0x0F, 0x1F, 0x7A, 0xC1, 0xD3, 0x6A, 0x1E, 0xA3, 0x54, 0x34, 0x8E, 0x0D, 0xD7,
	0x96, 0x93, 0x78, 0x50, 0xC1, 0xEE, 0x27, 0x72, 0x3A, 0xBD, 0x57, 0x22, 0xF0, 0xD7, 0x6D,
	0x9D, 0x65, 0xC4, 0x07, 0x9C, 0x82, 0xA6, 0xD4, 0xF7, 0x6B, 0x9A, 0xE9, 0xC0, 0x6C, 0x4A,
	0x4F, 0x6F, 0xBE, 0x8E, 0x01, 0x37, 0x50, 0x3A, 0x66, 0xD9, 0xE9, 0xD9, 0xF9, 0x06, 0x9E,
	0x00, 0xA9, 0x84, 0xA0, 0x18, 0xB3, 0x44, 0x21, 0x24, 0xA3, 0x6C, 0xCD, 0xB7, 0x0F, 0x31,
	0x2A, 0xE8, 0x15, 0xB6, 0x93, 0x6F, 0xB9, 0x86, 0xE5, 0x28, 0x01, 0x1A, 0x5E, 0x10, 0x3F,
	0x1F, 0x4D, 0x35, 0xA2, 0x8D, 0xB8, 0x54, 0x26, 0x68, 0x3A, 0xCD, 0xCB, 0x5F, 0xFA, 0x37,
	0x4A, 0x60, 0x10, 0xB1, 0x0A, 0xFE, 0xBA, 0x9B, 0x96, 0x5D, 0x7E, 0x99, 0xCF, 0x01, 0x98,
	0x65, 0x87, 0xAD, 0x40, 0xD5, 0x82, 0x1D, 0x61, 0x54, 0xA2, 0xD3, 0x16, 0x3E, 0xF7, 0xE3,
	0x05, 0x89, 0x8D, 0x8A, 0x50, 0x87, 0x47, 0xBE, 0x29, 0x18, 0x01, 0xB7, 0xC3, 0xDD, 0x43,
	0x23, 0x7A, 0xCD, 0x85, 0x1D, 0x4E, 0xA9, 0xC0, 0x1A, 0xA4, 0x77, 0xAB, 0xE7, 0x31, 0x9A,
	0x33, 0x1B, 0x7A, 0x86, 0xE1, 0xE5, 0xCA, 0x0C, 0x43, 0x1A, 0xFA, 0xEC, 0x4C, 0x05, 0xC6,
	0xD1, 0x43, 0x12, 0xF9, 0x4D, 0x3E, 0xF7, 0xD6, 0x05, 0x9C, 0x1C, 0xDD
};

/** Rx->Tx AKE_Send_Cert: RxCaps (Device is not an HDCP Repeater) */
static const u8 XHdcp22_Tx_Test_RxCaps[] = { 0x02, 0x00, 0x00 };

/** Rx->Tx AKE_Send_Cert: Rrx */
static const u8 XHdcp22_Tx_Test_Rrx[] = { 0xE1, 0x7A, 0xB0, 0xFD, 0x0F, 0x54, 0x40, 0x52 };

/** Tx->Rx AKE_No_Stored_km: hardcoded Km is normally generated by TX */
static const u8 XHdcp22_Tx_Test_Km[] = {
	0xCA, 0x9F, 0x83, 0x95, 0x70, 0xD0, 0xD0, 0xF9, 0xCF, 0xE4, 0xEB, 0x54, 0x7E, 0x09,
	0xFA, 0x3B
};

/** Tx->Rx AKE_NoStored_km: encrypted Km send by Tx */
static const u8 XHdcp22_Tx_Test_EkpubKm[] = {
	0xA8, 0x55, 0xC2, 0xC4, 0xC6, 0xBE, 0xEF, 0xCD, 0xCB, 0x9F, 0xE3, 0x9F,
	0x2A, 0xB7, 0x29, 0x76, 0xFE, 0xD8, 0xDA, 0xC9, 0x38, 0xFA, 0x39, 0xF0,
	0xAB, 0xCA, 0x8A, 0xED, 0x95, 0x7B, 0x93, 0xB2, 0xDF, 0xD0, 0x7D, 0x09,
	0x9D, 0x05, 0x96, 0x66, 0x03, 0x6E, 0xBA, 0xE0, 0x63, 0x0F, 0x30, 0x77,
	0xC2, 0xBB, 0xE2, 0x11, 0x39, 0xE5, 0x27, 0x78, 0xEE, 0x64, 0xF2, 0x85,
	0x36, 0x57, 0xC3, 0x39, 0xD2, 0x7B, 0x79, 0x03, 0xB7, 0xCC, 0x82, 0xCB,
	0xF0, 0x62, 0x82, 0x43, 0x38, 0x09, 0x9B, 0x71, 0xAA, 0x38, 0xA6, 0x3F,
	0x48, 0x12, 0x6D, 0x8C, 0x5E, 0x07, 0x90, 0x76, 0xAC, 0x90, 0x99, 0x51,
	0x5B, 0x06, 0xA5, 0xFA, 0x50, 0xE4, 0xF9, 0x25, 0xC3, 0x07, 0x12, 0x37,
	0x64, 0x92, 0xD7, 0xDB, 0xD3, 0x34, 0x1C, 0xE4, 0xFA, 0xDD, 0x09, 0xE6,
	0x28, 0x3D, 0x0C, 0xAD, 0xA9, 0xD8, 0xE1, 0xB5
};

/** Tx-Rx AKE_No_Stored_km: hardcoded masking seed */
static const u8 XHdcp22_Tx_Test_Km_MaskingSeed[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
	0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
	0x1C, 0x1D, 0x1E, 0x1F };

/** Rx->Tx AKE_Send_H_Prime: H' */
static const u8 XHdcp22_Tx_Test_H1[] = {
	0x4F, 0xF1, 0xA2, 0xA5, 0x61, 0x67, 0xC8, 0xE0, 0xAD, 0x16, 0xC8, 0x95,
	0x99, 0x1B, 0x1A, 0x21, 0xA8, 0x80, 0xC6, 0x27, 0x39, 0x3F, 0xC7, 0xBB,
	0x83, 0xED, 0xA7, 0xE5, 0x69, 0x07, 0xA5, 0xDC
};

/** Rx->Tx AKE_Send_Pairing_Info: Ekh(Km)  */
static const u8 XHdcp22_Tx_Test_Ekh_Km[] = {
	0xE6, 0x57, 0x8E, 0xBC, 0xC7, 0x68,	0x44, 0x87, 0x88, 0x8A, 0x9B, 0xD7,	0xD6, 0xAE,
	0x38, 0xBE
};

/** Locality check
 * Tx->Rx LC_Init: Rn hardcoded Rn is normally generated by TX*/
static const u8 XHdcp22_Tx_Test_Rn[] = { 0xA0, 0xFE, 0x9B, 0xB8, 0x20, 0x60, 0x58, 0xCA };

/* Session key exchange */

/** Tx->Rx SKE_Send_Eks */
static const u8 XHdcp22_Tx_Test_Riv[] = { 0x9A, 0x6D, 0x11, 0x00, 0xA9, 0xB7, 0x6F, 0x64 };

/** Tx->Rx SKE_Send_Eks */
static const u8 XHdcp22_Tx_Test_Ks[] = {
	0xF3, 0xDF, 0x1D, 0xD9, 0x57, 0x96, 0x12, 0x3F,
	0x98, 0x97, 0x89, 0xB4, 0x21, 0xE1, 0x2D, 0xE1
};

/** Tx->Rx SKE_Send_Eks */
static const u8 XHdcp22_Tx_Test_EdkeyKs[] ={
	0xB6, 0x8B, 0x8A, 0xA4, 0xD2, 0xCB, 0xBA, 0xFF,
	0x53, 0x33, 0xC1, 0xD9, 0xBB, 0xB7, 0x10, 0xA9
};

/** Rx->Tx LC_Send_L_Prime: L' */
static const u8 XHdcp22_Tx_Test_L1[] = {
	0xF2, 0x0F, 0x13, 0x6E, 0x85, 0x53, 0xC1, 0x0C, 0xD3, 0xDD, 0xB2, 0xF9,
	0x6D, 0x33, 0x31, 0xF9, 0xCB, 0x6E, 0x97, 0x8C, 0xCD, 0x5E, 0xDA, 0x13,
	0xDD, 0xEA, 0x41, 0x44, 0x10, 0x9B, 0x51, 0xB0
};

/***************** Macros (Inline Functions) Definitions *********************/

/** Testing macro to set a specific flag */
#define TEST_FLAG_SET(Flag, Flags) (Flags&Flag)==Flag

/** Case replacement to copy a case Id to a string */
#define XHDCP22_TX_CASE_TO_STR(arg) case arg : strcpy(str, #arg); break;

/** Case replacement to copy a case Id to a string with a pre-lead*/
#define XHDCP22_TX_CASE_TO_STR_PRE(pre, arg) \
case pre ## arg : strcpy(str, #arg); break;

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

static void XHdcp22Tx_TestReadMsg(u8* BufferPtr, u32 TestFlags);

/************************** Variable Definitions *****************************/

extern XHdcp22_Tx_PairingInfo XHdcp22_Tx_PairingInfoStore[XHDCP22_TX_MAX_STORED_PAIRINGINFO];

/**
 * This instance pointer is initialized on log reset,
 * and used for log/test functions that do not have access to the instance
 * pointer as parameter.
 */
static XHdcp22_Tx *Xhdcp22TxDbgInstancePtr = NULL;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function dumps a buffer with a name (string) to the output (UART).
*
* @param  String is a string printed before the buffer contents.
* @param  Buf is the buffer to print..
* @param  Buflen is the length of the buffer.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XHdcp22Tx_Dump(const char *String, const u8 *Buf, u32 Buflen)
{
	u32 Offset;

	xil_printf("\n##########################################\n\r");
	xil_printf("INFO::%s::Byte[0:%0d]", String, Buflen-1);
	for(Offset=0; Offset<Buflen; Offset++)
	{
		if((Offset%20) == 0) {
			xil_printf("\n\r");
		}

		xil_printf("%02x ", Buf[Offset]);
	}
	xil_printf("\n\r##########################################\n\r");
}

/*****************************************************************************/
/**
*
* This function simulates a read from the DDC message buffer.
* It returns hardcoded messages with valid content unless a test flag is used
* to force invalid messages.
*
* @param  BufferPtr is a pointer to the buffer that will receive a message.
* @param  TestFlags.are possible flags to influence the read.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XHdcp22Tx_TestReadMsg(u8* BufferPtr, u32 TestFlags)
{
	int BytesWritten = 0;
	u8 MsgId = BufferPtr[0]; /* MessageId */

	/* Write MessageId*/
	BufferPtr[BytesWritten++] = MsgId;
	switch (MsgId)
	{
		case XHDCP22_TX_AKE_SEND_CERT:
			/* Write HDCP22 RX certificate */
			memcpy(&BufferPtr[1], XHdcp22_Tx_Test_CertRx,
			       sizeof(XHdcp22_Tx_Test_CertRx));
			BytesWritten += sizeof(XHdcp22_Tx_Test_CertRx);

			/* Write Rrx */
			memcpy(&BufferPtr[BytesWritten], XHdcp22_Tx_Test_Rrx,
			       sizeof(XHdcp22_Tx_Test_Rrx));
			BytesWritten += sizeof(XHdcp22_Tx_Test_Rrx);

			/* Write RxCaps */
			memcpy(&BufferPtr[BytesWritten], XHdcp22_Tx_Test_RxCaps,
			       sizeof(XHdcp22_Tx_Test_RxCaps));
			if (TEST_FLAG_SET(XHDCP22_TX_TEST_CERT_RX, TestFlags) &&
			    TEST_FLAG_SET(XHDCP22_TX_TEST_INVALID_VALUE, TestFlags)) {
				BufferPtr[1] = ~BufferPtr[1];
			}
		break;
		case XHDCP22_TX_AKE_SEND_H_PRIME:
			/* Write HDCP22 H'Prime */
			memcpy(&BufferPtr[1], XHdcp22_Tx_Test_H1, sizeof(XHdcp22_Tx_Test_H1));
			if (TEST_FLAG_SET(XHDCP22_TX_TEST_H1, TestFlags) &&
			    TEST_FLAG_SET(XHDCP22_TX_TEST_INVALID_VALUE, TestFlags)) {
				BufferPtr[1] = ~BufferPtr[1];
			}
		break;
		case XHDCP22_TX_AKE_SEND_PAIRING_INFO:
			/* Write Ekh(Km) */
			memcpy(&BufferPtr[1], XHdcp22_Tx_Test_Ekh_Km,
			       sizeof(XHdcp22_Tx_Test_Ekh_Km));
		break;
		case XHDCP22_TX_LC_SEND_L_PRIME:
			/* Write LC Prime */
			memcpy(&BufferPtr[1], XHdcp22_Tx_Test_L1,
				   sizeof(XHdcp22_Tx_Test_L1));
			if (TEST_FLAG_SET(XHDCP22_TX_TEST_L1, TestFlags) &&
			    TEST_FLAG_SET(XHDCP22_TX_TEST_INVALID_VALUE, TestFlags)) {
				BufferPtr[1] = ~BufferPtr[1];
			}
		break;
		default:
		break;
	};
}

/*****************************************************************************/
/**
*
* This function clears the log pointers
*
* @param  InstancePtr is a pointer to the XHdcp22_Tx core instance.
* @param  Verbose allows to add debug logging.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XHdcp22Tx_LogReset(XHdcp22_Tx *InstancePtr, u8 Verbose)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Is used by functions for debugging purposes, that do not have
	 * access to the Instance as a parameter. */
	Xhdcp22TxDbgInstancePtr = InstancePtr;

	InstancePtr->Log.Head = 0;
	InstancePtr->Log.Tail = 0;
	InstancePtr->Log.Verbose = Verbose;
	/* Reset and start the logging timer. */
	/* Note: This timer increments continuously and will wrap at 42 second (100 Mhz clock) */
	if (InstancePtr->Timer.TmrCtr.IsReady == XIL_COMPONENT_IS_READY) {
	   XTmrCtr_SetResetValue(&InstancePtr->Timer.TmrCtr, XHDCP22_TX_TIMER_CNTR_1, 0);
	   XTmrCtr_Start(&InstancePtr->Timer.TmrCtr, XHDCP22_TX_TIMER_CNTR_1);
	}
}

/*****************************************************************************/
/**
*
* This function returns the time expired since a log reset was called
*
* @param  InstancePtr is a pointer to the XHdcp22_Tx core instance.
*
* @return The expired logging time in useconds.
*
* @note   None.
*
******************************************************************************/
u32 XHdcp22Tx_LogGetTimeUSecs(XHdcp22_Tx *InstancePtr)
{
	if (InstancePtr->Timer.TmrCtr.IsReady != XIL_COMPONENT_IS_READY)
		return 0;

	u32 PeriodUsec = (u32)InstancePtr->Timer.TmrCtr.Config.SysClockFreqHz * 1e-6;
	return 	 ( XTmrCtr_GetValue(&InstancePtr->Timer.TmrCtr,
			   XHDCP22_TX_TIMER_CNTR_1) / PeriodUsec);
}

/*****************************************************************************/
/**
*
* This function writes HDCP TX logs into buffer for functions that do
* not have access to the instance ptr as a parameter.
*
* @param  Evt specifies an action to be carried out. Please refer
*         #XHdcp22_Tx_LogEvt enum in xhdcp22_tx.h.
* @param  Data specifies the information that gets written into log
*         buffer.
*
* @return None.
*
* @note   Please use for debugging only, since this function breaks use
*         of multiple driver instances.
*
******************************************************************************/
void XHdcp22Tx_LogWrNoInst(XHdcp22_Tx_LogEvt Evt, u16 Data)
{
	XHdcp22Tx_LogWr(Xhdcp22TxDbgInstancePtr, Evt, Data);
}

/*****************************************************************************/
/**
*
* This function writes HDCP TX logs into buffer.
*
* @param  InstancePtr is a pointer to the XHdcp22_Tx core instance.
* @param  Evt specifies an action to be carried out. Please refer
*         #XHdcp22_Tx_LogEvt enum in xhdcp22_tx.h.
* @param  Data specifies the information that gets written into log
*         buffer.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XHdcp22Tx_LogWr(XHdcp22_Tx *InstancePtr, XHdcp22_Tx_LogEvt Evt, u16 Data)
{
	int LogBufSize = 0;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Evt < (XHDCP22_TX_LOG_INVALID));

	if (InstancePtr->Log.Verbose == FALSE && Evt == XHDCP22_TX_LOG_EVT_DBG) {
		return;
	}

	/* Write data and event into log buffer */
	InstancePtr->Log.LogItems[InstancePtr->Log.Head].Data = Data;
	InstancePtr->Log.LogItems[InstancePtr->Log.Head].LogEvent = Evt;
	InstancePtr->Log.LogItems[InstancePtr->Log.Head].TimeStamp =
                            XHdcp22Tx_LogGetTimeUSecs(InstancePtr);

	/* Update head pointer if reached to end of the buffer */
	LogBufSize = sizeof(InstancePtr->Log.LogItems)/sizeof(XHdcp22_Tx_LogItem);
	if (InstancePtr->Log.Head == (u16)(LogBufSize) - 1) {
		/* Clear pointer */
		InstancePtr->Log.Head = 0;
	} else {
		/* Increment pointer */
		InstancePtr->Log.Head++;
	}

	/* Check tail pointer. When the two pointer are equal, then the buffer
	 * is full.In this case then increment the tail pointer as well to
	 * remove the oldest entry from the buffer.
	 */
	if (InstancePtr->Log.Tail == InstancePtr->Log.Head) {
		if (InstancePtr->Log.Tail == (u16)(LogBufSize) - 1) {
			InstancePtr->Log.Tail = 0;
		} else {
			InstancePtr->Log.Tail++;
		}
	}
}

/*****************************************************************************/
/**
*
* This function provides the log information from the log buffer.
*
* @param  InstancePtr is a pointer to the XHdcp22_Tx core instance.
*
* @return
*         - Content of log buffer if log pointers are not equal.
*         - Otherwise Zero.
*
* @note   None.
*
******************************************************************************/
XHdcp22_Tx_LogItem* XHdcp22Tx_LogRd(XHdcp22_Tx *InstancePtr)
{
	XHdcp22_Tx_LogItem* LogPtr;
	int LogBufSize = 0;
	u16 Tail = 0;
	u16 Head = 0;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Tail = InstancePtr->Log.Tail;
	Head = InstancePtr->Log.Head;

	/* Check if there is any data in the log and return a NONE defined log item */
	LogBufSize = sizeof(InstancePtr->Log.LogItems)/sizeof(XHdcp22_Tx_LogItem);
	if (Tail == Head) {
		LogPtr = &InstancePtr->Log.LogItems[Tail];
		LogPtr->Data = 0;
		LogPtr->LogEvent = XHDCP22_TX_LOG_EVT_NONE;
		LogPtr->TimeStamp = 0;
		return LogPtr;
	}

	LogPtr = &InstancePtr->Log.LogItems[Tail];

	/* Increment tail pointer */
	if (Tail == (u16)(LogBufSize) - 1) {
		InstancePtr->Log.Tail = 0;
	}
	else {
		InstancePtr->Log.Tail++;
	}
	return LogPtr;
}

/*****************************************************************************/
/**
*
* This function prints the content of log buffer in a unit test result
* usable form. The displayed test is source that can be copied. If
* the test mode is set toXHDCP22_TX_TESTMODE_NO_RX, this function is executed.
* See also #XHdcp22Tx_LogDisplay.
*
* @param  InstancePtr is a pointer to the XHdcp22_Tx core instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XHdcp22Tx_LogDisplayUnitTest(XHdcp22_Tx *InstancePtr)
{
	XHdcp22_Tx_LogItem* LogPtr;
	char str[255];
	u32 Index = 0;

	xil_printf("\r\n--------------------------------------\r\n", Index);
	do {
		/* Read log data */
		LogPtr = XHdcp22Tx_LogRd(InstancePtr);

		switch (LogPtr->LogEvent)
		{
		case XHDCP22_TX_LOG_EVT_STATE:
			switch(LogPtr->Data)
			{
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_H0)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_H1)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A0)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A1)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A1_1)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A1_NSK0)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A1_NSK1)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A1_SK0)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A2)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A2_1)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A3)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A4)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A5)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A6)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A7)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A8)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_STATE_A9)
				default: break;
			};
			xil_printf("TestRes[%d].LogEvent = XHDCP22_TX_LOG_EVT_STATE;\r\n",
			           Index);
			xil_printf("TestRes[%d].Data = %s;\r\n", Index, str);
		break;
		case XHDCP22_TX_LOG_EVT_POLL_RESULT:
			switch(LogPtr->Data)
			{
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_INCOMPATIBLE_RX)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_AUTHENTICATION_BUSY)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_AUTHENTICATED)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_UNAUTHENTICATED)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_REAUTHENTICATE_REQUESTED)
				default: break;
			}
			xil_printf("TestRes[%d].LogEvent = XHDCP22_TX_LOG_EVT_POLL_RESULT;\r\n",
			           Index);
			xil_printf("TestRes[%d].Data = %s;\r\n", Index, str);
		break;
		case XHDCP22_TX_LOG_EVT_ENABLED:
			if (LogPtr->Data == (FALSE)) {
				strcpy(str, "FALSE");
			} else {
				strcpy(str, "TRUE");
			}
			xil_printf("TestRes[%d].LogEvent = XHDCP22_TX_LOG_EVT_ENABLED;\r\n",
			           Index);
			xil_printf("TestRes[%d].Data = %s;\r\n", Index, str);
		break;
		case XHDCP22_TX_LOG_EVT_RESET:
			xil_printf("TestRes[%d].LogEvent = XHDCP22_TX_LOG_EVT_RESET;\r\n",
			           Index);
			xil_printf("TestRes[%d].Data = %d;\r\n", Index, LogPtr->Data);
		break;
		case XHDCP22_TX_LOG_EVT_TEST_ERROR:
			switch(LogPtr->Data)
			{
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_AKE_NO_STORED_KM)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_SKE_SEND_EKS)
				XHDCP22_TX_CASE_TO_STR(XHDCP22_TX_MSG_UNDEFINED)
				default: break;
			};
			xil_printf("TestRes[%d].LogEvent = XHDCP22_TX_LOG_EVT_TEST_ERROR;\r\n",
			           Index);
			xil_printf("TestRes[%d].Data = %s;\r\n", Index, str);
		break;
		case XHDCP22_TX_LOG_EVT_ENCR_ENABLED:
			if (LogPtr->Data == (FALSE)) {
				strcpy(str, "FALSE");
			} else {
				strcpy(str, "TRUE");
			}
			xil_printf("TestRes[%d].LogEvent = XHDCP22_TX_LOG_EVT_ENCR_ENABLED;\r\n",
			           Index);
			xil_printf("TestRes[%d].Data = %s;\r\n", Index, str);
			break;
		case XHDCP22_TX_LOG_EVT_LCCHK_COUNT:
			xil_printf("TestRes[%d].LogEvent = XHDCP22_TX_LOG_EVT_LCCHK_COUNT;\r\n",
			           Index);
			xil_printf("TestRes[%d].Data = %d;\r\n", Index, LogPtr->Data);
			break;
		default: break;
		};
		Index++;
	} while (LogPtr->LogEvent != XHDCP22_TX_LOG_EVT_NONE);
	xil_printf("NumTestResItems = %d;\r\n", Index - 1);
	xil_printf("\r\n--------------------------------------\r\n", Index);
}

/*****************************************************************************/
/**
*
* This function prints the content of log buffer.
*
* @param  InstancePtr is a pointer to the HDCP22 TX core instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XHdcp22Tx_LogDisplay(XHdcp22_Tx *InstancePtr)
{
	XHdcp22_Tx_LogItem* LogPtr;
	char str[255];
	u64 TimeStampPrev = 0;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if (InstancePtr->Test.TestMode == XHDCP22_TX_TESTMODE_UNIT) {
		XHdcp22Tx_LogDisplayUnitTest(InstancePtr);
		return;
	}

	xil_printf("\r\n-------HDCP22 TX log start-------\r\n");
	strcpy(str, "UNDEFINED");
	do {
		/* Read log data */
		LogPtr = XHdcp22Tx_LogRd(InstancePtr);

		/* Print timestamp */
		if(LogPtr->LogEvent != XHDCP22_TX_LOG_EVT_NONE)
		{
			if(LogPtr->TimeStamp < TimeStampPrev) TimeStampPrev = 0;
			xil_printf("[%8ld:", LogPtr->TimeStamp);
			xil_printf("%8ld] ", (LogPtr->TimeStamp - TimeStampPrev));
			TimeStampPrev = LogPtr->TimeStamp;
		}

		/* Print log event */
		switch (LogPtr->LogEvent) {
		case (XHDCP22_TX_LOG_EVT_NONE):
			xil_printf("-------HDCP22 TX log end-------\r\n\r\n");
			break;
		case XHDCP22_TX_LOG_EVT_STATE:
			switch(LogPtr->Data)
			{
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, H0)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, H1)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A0)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A1)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A1_1)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A1_NSK0)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A1_NSK1)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A1_SK0)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A2)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A2_1)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A3)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A4)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A5)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A6)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A7)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A8)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_STATE_, A9)
				default: break;
			};
			xil_printf("Current state [%s]\r\n", str);
			break;
		case XHDCP22_TX_LOG_EVT_POLL_RESULT:
			switch(LogPtr->Data)
			{
			case XHDCP22_TX_INCOMPATIBLE_RX: strcpy(str, "INCOMPATIBLE RX"); break;
			case XHDCP22_TX_AUTHENTICATION_BUSY: strcpy(str, "AUTHENTICATION BUSY"); break;
			case XHDCP22_TX_AUTHENTICATED: strcpy(str, "AUTHENTICATED"); break;
			case XHDCP22_TX_UNAUTHENTICATED: strcpy(str, "UN-AUTHENTICATED"); break;
			case XHDCP22_TX_REAUTHENTICATE_REQUESTED: strcpy(str, "RE-AUTHENTICATION REQUESTED"); break;
			default: break;
			}
			xil_printf("Poll result [%s]\r\n", str);
			break;
		case XHDCP22_TX_LOG_EVT_ENABLED:
			if (LogPtr->Data == (FALSE)) {
				strcpy(str, "DISABLED");
			} else {
				strcpy(str, "ENABLED");
			}
			xil_printf("State machine [%s]\r\n", str);
			break;
		case XHDCP22_TX_LOG_EVT_RESET:
			xil_printf("Asserted [RESET]\r\n");
			break;
		case XHDCP22_TX_LOG_EVT_ENCR_ENABLED:
			if (LogPtr->Data == (FALSE)) {
				strcpy(str, "DISABLED");
			} else {
				strcpy(str, "ENABLED");
			}
			xil_printf("Encryption [%s]\r\n", str);
			break;
		case XHDCP22_TX_LOG_EVT_TEST_ERROR:
			switch(LogPtr->Data)
			{
			case XHDCP22_TX_AKE_NO_STORED_KM:
			      strcpy(str, "EkpubKm does not match the calculated value.");
			      break;
			case XHDCP22_TX_SKE_SEND_EKS:
			      strcpy(str, "EdkeyKs does not match the calculated value.");
			      break;
			case XHDCP22_TX_MSG_UNDEFINED:
			      strcpy(str, "Trying to write an unexpected message.");
			      break;
			default: break;
			};
			xil_printf("Error: Test error [%s]\r\n", str);
			break;
		case XHDCP22_TX_LOG_EVT_LCCHK_COUNT:
			xil_printf("Locality check count [%d]\r\n", LogPtr->Data);
			break;
		case XHDCP22_TX_LOG_EVT_DBG:
			switch(LogPtr->Data)
			{
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, STARTIMER)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, MSGAVAILABLE)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, TX_AKEINIT)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, RX_CERT)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, VERIFY_SIGNATURE)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, VERIFY_SIGNATURE_DONE)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, ENCRYPT_KM)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, ENCRYPT_KM_DONE)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, TX_NOSTOREDKM)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, TX_STOREDKM)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, RX_H1)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, RX_EKHKM)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, COMPUTE_H)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, COMPUTE_H_DONE)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, TX_LCINIT)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, RX_L1)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, COMPUTE_L)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, COMPUTE_L_DONE)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, COMPUTE_EDKEYKS)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, COMPUTE_EDKEYKS_DONE)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, TX_EKS)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, CHECK_REAUTH)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, TIMEOUT)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, TIMESTAMP)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, AES128ENC)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, AES128ENC_DONE)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, SHA256HASH)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, SHA256HASH_DONE)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, OEAPENC)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, OEAPENC_DONE)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, RSAENC)
				XHDCP22_TX_CASE_TO_STR_PRE(XHDCP22_TX_LOG_DBG_, RSAENC_DONE)
				default: break;
			};
			xil_printf("Debug: Event [%s]\r\n", str);
			break;
		case XHDCP22_TX_LOG_EVT_USER:
			xil_printf("User: %d\r\n", LogPtr->Data);
			break;
		default:
			xil_printf("Error: Unknown log event\r\n");
			break;
		}
	} while (LogPtr->LogEvent != XHDCP22_TX_LOG_EVT_NONE);
}

/*****************************************************************************/
/**
*
* DDCRead handler that is used in test mode, and emulates responses from a
* HDCP22 receiver.
*
* @param  DeviceAddress is the addres to use. Normally set to 0x74.
* @param  ByteCount is the amount of bytes to read.
* @param  BufferPtr contains room for dsata to read. The first byte always refers to
*         the address to read.
* @param  RefPtr is a reference to the instance and in test mode always a #XHdcp22_Tx
*         structure.
*
* @return
*         - XST_SUCCESS if reading succeeded.
*         - XST_FAILURE if reading failed.
*
* @note     None.
*
******************************************************************************/
static int XHdcp22Tx_TestDdcRead(u8 DeviceAddress, u16 ByteCount, u8* BufferPtr,
                                 u8 Stop, void *RefPtr)
{
	XHdcp22_Tx *InstancePtr = (XHdcp22_Tx *)RefPtr;
	u16  RxStatus = 0;

	if (DeviceAddress == 0x3A)
	{
		switch (InstancePtr->Test.CurrentDdcAddress)
		{
		case XHDCP22_TX_HDCPPORT_VERSION_OFFSET:
			BufferPtr[0] = 0x04;
			break;
		case XHDCP22_TX_HDCPPORT_RXSTATUS_OFFSET:
			switch(InstancePtr->Timer.ReasonId)
			{
				case XHDCP22_TX_AKE_SEND_CERT:
					RxStatus|=XHDCP22_TX_RXSTATUS_READY_MASK;
					RxStatus|=XHDCP22_TX_AKE_SEND_CERT_SIZE;
				break;
				case XHDCP22_TX_AKE_SEND_H_PRIME:
					RxStatus|=XHDCP22_TX_RXSTATUS_READY_MASK;
					RxStatus|=XHDCP22_TX_AKE_SEND_H_PRIME_SIZE;
				break;
				case XHDCP22_TX_AKE_SEND_PAIRING_INFO:
					RxStatus|=XHDCP22_TX_RXSTATUS_READY_MASK;
					RxStatus|=XHDCP22_TX_AKE_SEND_PAIRING_INFO_SIZE;
				break;
				case XHDCP22_TX_LC_SEND_L_PRIME:
					RxStatus|=XHDCP22_TX_RXSTATUS_READY_MASK;
					RxStatus|=XHDCP22_TX_LC_SEND_L_PRIME_SIZE;
				break;
			}
			*(u16 *)BufferPtr = RxStatus;
			break;
		case XHDCP22_TX_HDCPPORT_READ_MSG_OFFSET:
			XHdcp22Tx_TestReadMsg(BufferPtr, InstancePtr->Test.TestFlags);
			break;
		case XHDCP22_TX_HDCPPORT_WRITE_MSG_OFFSET:
			break;
		default:
			BufferPtr[0] = 0;
			break;
		}
	};
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* DDCWrite handler that is used in test mode, and emulates responses from a
* HDCP22 receiver. It also checks when possible on errors, which are logged.
*
* @param  DeviceAddress is the addres to use.
* @param  ByteCount is the amount of bytes to write.
* @param  BufferPtr contains data to write. The first byte always refers to
*         the address to write to.
* @param  RefPtr is a reference to the instance and in test mode always a #XHdcp22_Tx
*         structure.
*
* @return
*         - XST_SUCCESS if reading succeeded.
*         - XST_FAILURE if reading failed.
*
* @note   None.
*
******************************************************************************/
static int XHdcp22Tx_TestDdcWrite(u8 DeviceAddress, u16 ByteCount, u8* BufferPtr,
                                  u8 Stop, void *RefPtr)
{
	XHdcp22_Tx *InstancePtr = (XHdcp22_Tx *)RefPtr;
	InstancePtr->Test.CurrentDdcAddress = BufferPtr[0];

	if (DeviceAddress == XHDCP22_TX_DDC_BASE_ADDRESS)
	{
		switch (InstancePtr->Test.CurrentDdcAddress)
		{
		case XHDCP22_TX_HDCPPORT_WRITE_MSG_OFFSET:
			switch(BufferPtr[1]) {
			case XHDCP22_TX_AKE_NO_STORED_KM:
				if (memcmp(&BufferPtr[2], XHdcp22_Tx_Test_EkpubKm,
					sizeof(XHdcp22_Tx_Test_EkpubKm)) != 0) {
						XHdcp22Tx_LogWr(InstancePtr, XHDCP22_TX_LOG_EVT_TEST_ERROR,
										XHDCP22_TX_AKE_NO_STORED_KM);
				}
				break;
			case XHDCP22_TX_SKE_SEND_EKS:
				if (memcmp(&BufferPtr[2], XHdcp22_Tx_Test_EdkeyKs,
					sizeof(XHdcp22_Tx_Test_EdkeyKs)) != 0) {
						XHdcp22Tx_LogWr(InstancePtr, XHDCP22_TX_LOG_EVT_TEST_ERROR,
										XHDCP22_TX_SKE_SEND_EKS);
				}
				break;
			case XHDCP22_TX_AKE_INIT:
			case XHDCP22_TX_AKE_STORED_KM:
			case XHDCP22_TX_LC_INIT:
			/* No need to check these, these messages are just assembled from
			   hardcoded values from this test */
			break;
			default:
				/* Write an error to the log if the message is undefined */
				XHdcp22Tx_LogWr(InstancePtr, XHDCP22_TX_LOG_EVT_TEST_ERROR,
								XHDCP22_TX_MSG_UNDEFINED);
				break;
			};
			break;
		case XHDCP22_TX_HDCPPORT_VERSION_OFFSET:
		case XHDCP22_TX_HDCPPORT_RXSTATUS_OFFSET:
		case XHDCP22_TX_HDCPPORT_READ_MSG_OFFSET:
			/* No need to check these */
			break;
		default:
			/* Write an error to the log if the message is undefined */
			XHdcp22Tx_LogWr(InstancePtr, XHDCP22_TX_LOG_EVT_TEST_ERROR,
								XHDCP22_TX_MSG_UNDEFINED);
			break;
		}
	};
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Set core into test mode for validation.
*
* @param  InstancePtr is a pointer to the HDCP22 TX core instance.
* @param  Mode is the mode to use refer to #XHdcp22_Tx_TestMode.
* @param  TestFlags are the flags that allow to trigger test conditions.
*
* @return None.
*
* @note   Test flags should be combined for a certain test.
*         Some testflags force the use of a test item as described in:
*         "Errata to HDCP on HDMI Specification revision 2.2."
*         Examples:
*         - Receiving an invalid receiver certificate requires:
*           XHDCP22_TX_TEST_CERT_RX | XHDCP22_TX_TEST_INVALID_VALUE
*         - Receiving of the certificate times out
*           XHDCP22_TX_TEST_CERT_RX | XHDCP22_TX_TEST_RECV_TIMEOUT
*         - Receiving of HPrime in a stored Km scenario with timeout error
*           XHDCP22_TX_TEST_CERT_RX | XHDCP22_TX_TEST_STORED_KM |
*           XHDCP22_TX_TEST_RECV_TIMEOUT
*
******************************************************************************/
void XHdcp22Tx_TestSetMode(XHdcp22_Tx *InstancePtr, XHdcp22_Tx_TestMode Mode,
                           u32 TestFlags)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Mode < XHDCP22_TX_TESTMODE_INVALID);

	if ((TestFlags & XHDCP22_TX_TEST_CLR_PAIRINGINFO) == XHDCP22_TX_TEST_CLR_PAIRINGINFO) {
			memset(XHdcp22_Tx_PairingInfoStore, 0, sizeof(XHdcp22_Tx_PairingInfoStore));
	}

	/* Handle test scenarios */
	if (Mode == XHDCP22_TX_TESTMODE_NO_RX || Mode == XHDCP22_TX_TESTMODE_UNIT) {
		/* Set test handlers for DDC */
		XHdcp22Tx_SetCallback(InstancePtr, XHDCP22_TX_HANDLER_DDC_WRITE, XHdcp22Tx_TestDdcWrite, InstancePtr);
		XHdcp22Tx_SetCallback(InstancePtr, XHDCP22_TX_HANDLER_DDC_READ, XHdcp22Tx_TestDdcRead, InstancePtr);

		if ((TestFlags & XHDCP22_TX_TEST_STORED_KM) == XHDCP22_TX_TEST_STORED_KM) {
			memcpy(XHdcp22_Tx_PairingInfoStore[0].ReceiverId, XHdcp22_Tx_Test_CertRx, XHDCP22_TX_CERT_RCVID_SIZE);
			memcpy(XHdcp22_Tx_PairingInfoStore[0].Km, XHdcp22_Tx_Test_Km, XHDCP22_TX_KM_SIZE);
			memcpy(XHdcp22_Tx_PairingInfoStore[0].Rrx, XHdcp22_Tx_Test_Rrx, XHDCP22_TX_RRX_SIZE);
			memcpy(XHdcp22_Tx_PairingInfoStore[0].Rtx, XHdcp22_Tx_Test_Rtx, XHDCP22_TX_RTX_SIZE);
			memcpy(XHdcp22_Tx_PairingInfoStore[0].RxCaps, XHdcp22_Tx_Test_RxCaps, XHDCP22_TX_RXCAPS_SIZE);
			memcpy(XHdcp22_Tx_PairingInfoStore[0].Ekh_Km, XHdcp22_Tx_Test_Ekh_Km, XHDCP22_TX_EKH_KM_SIZE);
		}
	}

	InstancePtr->Test.TestMode = Mode;
	InstancePtr->Test.TestFlags = TestFlags;
}

/*****************************************************************************/
/**
*
* Compare logged results with expected results as a validation method.
*
* @param  InstancePtr is a pointer to the HDCP22 TX core instance.
* @param  Expected specifies expected logitems to compare with.
* @param  nExpected specifies number of items to compare with.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
u8 XHdcp22Tx_TestCheckResults(XHdcp22_Tx* InstancePtr,
                              XHdcp22_Tx_LogItem *Expected, u32 nExpected)
{
	u32 iLog = 0;
	u32 iExpected = 0;
	u8  Result = (TRUE);

	for (iExpected=0; iExpected<nExpected; iExpected++)
	{
		if (InstancePtr->Log.LogItems[iLog].LogEvent != Expected[iExpected].LogEvent ||
			InstancePtr->Log.LogItems[iLog].Data != Expected[iExpected].Data) {
				xil_printf("FAIL in step %d: \r\n\tExpected log event=%d, actual=%d\r\n"
				           "\tExpected data=0x%.2x, actual=0x%.2x\r\n",
				           iExpected,  Expected[iExpected].LogEvent,
				           InstancePtr->Log.LogItems[iLog].LogEvent,
				           Expected[iExpected].Data,
				           InstancePtr->Log.LogItems[iLog].Data);

				Result = (FALSE);
				break;
		}
		iLog++;
	}
	return Result;
}

/*****************************************************************************/
/**
*
* Determine if a timeout should be simulated in uni test mode (NO_RX).
*
* @param  InstancePtr is a pointer to the HDcp22_Tx core instance.
*
* @return TRUE if a timeout must be simulated, FALSE if not.
*
* @note   None.
*
******************************************************************************/
u8 XHdcp22Tx_TestSimulateTimeout(XHdcp22_Tx* InstancePtr)
{
	u8 SimTimeout = FALSE;

	/* Simulate a timeout if in testmode without RX (unit test) */
	if ((InstancePtr->Test.TestMode == XHDCP22_TX_TESTMODE_NO_RX ||
	     InstancePtr->Test.TestMode == XHDCP22_TX_TESTMODE_UNIT)&&
	     InstancePtr->Test.TestFlags&XHDCP22_TX_TEST_RCV_TIMEOUT)
	{
		switch(InstancePtr->Timer.ReasonId)
		{
			case XHDCP22_TX_AKE_SEND_CERT:
				if (TEST_FLAG_SET(XHDCP22_TX_TEST_CERT_RX, InstancePtr->Test.TestFlags)) {
					SimTimeout = TRUE;
				}
				break;
			case XHDCP22_TX_AKE_SEND_H_PRIME:
				if (TEST_FLAG_SET(XHDCP22_TX_TEST_H1, InstancePtr->Test.TestFlags)) {
					SimTimeout = TRUE;
				}
				break;
			case XHDCP22_TX_AKE_SEND_PAIRING_INFO:
				if (TEST_FLAG_SET(XHDCP22_TX_TEST_EKH_KM, InstancePtr->Test.TestFlags)) {
					SimTimeout = TRUE;
				}
				break;
			case XHDCP22_TX_LC_SEND_L_PRIME:
				if (TEST_FLAG_SET(XHDCP22_TX_TEST_L1, InstancePtr->Test.TestFlags)) {
					SimTimeout = TRUE;
				}
				break;
			default:
				SimTimeout = FALSE;
				break;
		}
	}
	return SimTimeout;
}

/*****************************************************************************/
/**
*
* This function replaces Rtx with a test vector if the testmode is
* #XHDCP22_TX_TESTMODE_NO_RX, #XHDCP22_TX_TESTMODE_UNIT or
* #XHDCP22_TX_TESTMODE_USE_TESTKEYS.
*
* @param  InstancePtr is a pointer to the HDCP22 TX core instance.
* @param  RtxPtr is a pointer to Rtx.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XHdcp22Tx_GenerateRtx_Test(XHdcp22_Tx *InstancePtr, u8* RtxPtr)
{
	if (InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_NO_RX &&
	    InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_UNIT &&
	    InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_USE_TESTKEYS) {
		return;
	}

	memcpy(RtxPtr, XHdcp22_Tx_Test_Rtx, XHDCP22_TX_RTX_SIZE);
}

/*****************************************************************************/
/**
*
* This function replaces Km with a test vector if the testmode is
* #XHDCP22_TX_TESTMODE_NO_RX, #XHDCP22_TX_TESTMODE_UNIT or
* #XHDCP22_TX_TESTMODE_USE_TESTKEYS.
*
* @param  InstancePtr is a pointer to the HDCP22 TX core instance..
* @param  KmPtr is a pointer to Km.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XHdcp22Tx_GenerateKm_Test(XHdcp22_Tx *InstancePtr, u8* KmPtr)
{
	if (InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_NO_RX &&
	    InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_UNIT &&
	    InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_USE_TESTKEYS) {
		return;
	}

	memcpy(KmPtr, XHdcp22_Tx_Test_Km, XHDCP22_TX_KM_SIZE);
}

/*****************************************************************************/
/**
*
* This function replaces the masking seed used in RSA-OEAP with a test vector
* #XHDCP22_TX_TESTMODE_NO_RX, #XHDCP22_TX_TESTMODE_UNIT or
* #XHDCP22_TX_TESTMODE_USE_TESTKEYS.
*
* @param  InstancePtr is a pointer to the HDCP22 TX core instance..
* @param  SeedPtr is a pointer to masking seed.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XHdcp22Tx_GenerateKmMaskingSeed_Test(XHdcp22_Tx *InstancePtr, u8* SeedPtr)
{
	if (InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_NO_RX &&
	    InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_UNIT &&
	    InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_USE_TESTKEYS) {
		return;
	}
	memcpy(SeedPtr, XHdcp22_Tx_Test_Km_MaskingSeed, XHDCP22_TX_KM_MSK_SEED_SIZE);
}

/*****************************************************************************/
/**
*
* This function replaces Rn with a test vector if the testmode is
* #XHDCP22_TX_TESTMODE_NO_RX, #XHDCP22_TX_TESTMODE_UNIT or
* #XHDCP22_TX_TESTMODE_USE_TESTKEYS.
*
* @param  InstancePtr is a pointer to the HDCP22 TX core instance..
* @param  RnPtr is a pointer to Rn.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XHdcp22Tx_GenerateRn_Test(XHdcp22_Tx *InstancePtr, u8* RnPtr)
{
	if (InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_NO_RX &&
	    InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_UNIT &&
	    InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_USE_TESTKEYS) {
		return;
	}
	memcpy(RnPtr, XHdcp22_Tx_Test_Rn, XHDCP22_TX_RN_SIZE);
}

/*****************************************************************************/
/**
*
* This function replaces Riv with a test vector if the testmode is
* #XHDCP22_TX_TESTMODE_NO_RX, #XHDCP22_TX_TESTMODE_UNIT or
* #XHDCP22_TX_TESTMODE_USE_TESTKEYS.
*
* @param  InstancePtr is a pointer to the HDCP22 TX core instance..
* @param  RivPtr is a pointer to Riv.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XHdcp22Tx_GenerateRiv_Test(XHdcp22_Tx *InstancePtr, u8* RivPtr)
{
	if (InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_NO_RX &&
	    InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_UNIT &&
	    InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_USE_TESTKEYS) {
		return;
	}
	memcpy(RivPtr, XHdcp22_Tx_Test_Riv, XHDCP22_TX_RIV_SIZE);
}

/*****************************************************************************/
/**
*
* This function replaces Ks with a test vector if the testmode is
* #XHDCP22_TX_TESTMODE_NO_RX, #XHDCP22_TX_TESTMODE_UNIT or
* #XHDCP22_TX_TESTMODE_USE_TESTKEYS.
*
* @param  InstancePtr is a pointer to the HDCP22 TX core instance.
* @param  KsPtr is a pointer to Ks.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XHdcp22Tx_GenerateKs_Test(XHdcp22_Tx *InstancePtr, u8* KsPtr)
{
	if (InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_NO_RX &&
	    InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_UNIT &&
	    InstancePtr->Test.TestMode != XHDCP22_TX_TESTMODE_USE_TESTKEYS) {
		return;
	}
	memcpy(KsPtr, XHdcp22_Tx_Test_Ks, XHDCP22_TX_KS_SIZE);
}

/*****************************************************************************/
/**
*
* This function returns a pointer to the KPubDpc testvector, or NULL if
* not in test mode.
*
* @param  InstancePtr is a pointer to the HDCP22 TX core instance.
*
* @return A pointer to the LLC public test vector KPubDpc or NULL if not in
*         test mode without RX.
* @note   None.
*
******************************************************************************/
const u8* XHdcp22Tx_GetKPubDpc_Test(XHdcp22_Tx *InstancePtr)
{
	if (InstancePtr->Test.TestMode == XHDCP22_TX_TESTMODE_SW_RX ||
	    InstancePtr->Test.TestMode == XHDCP22_TX_TESTMODE_NO_RX ||
	    InstancePtr->Test.TestMode == XHDCP22_TX_TESTMODE_UNIT ||
	    InstancePtr->Test.TestMode == XHDCP22_TX_TESTMODE_USE_TESTKEYS) {
		return XHdcp22_Tx_Test_Kpubdcp;
	}

	return NULL;
}

/*****************************************************************************/
/**
*
* This function generates random octets.
*
* @param  NumOctets number of octets to generate.
* @param  RandomNumberPtr is the pointer to a buffer that will be filled.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XHdcp22Tx_GenerateRandom_Test(int NumOctets, u8* RandomNumberPtr)
{
	u32 Rnd16 = 0;
	int i=0;
	u8 RndL, RndH;

	for (i=0; i<NumOctets; i+=2) {
		Rnd16 = rand();
		RndL = Rnd16 & 0xFF;
		RndH = (Rnd16 >> 8)&0xFF;
		RandomNumberPtr[i] = RndL;
		if (i+1 < NumOctets) {
			RandomNumberPtr[i+1] = RndH;
		}
	}
}
