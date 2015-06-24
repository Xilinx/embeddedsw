/******************************************************************************
*
* Copyright (C) 2013 - 2014 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
 * @file xilskey_eps.h
 *
 *
 * @note	None.
 *
 *
 * MODIFICATION HISTORY:
 *
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.00a rpoolla 04/26/13 First release
* 1.02a hk      10/28/13 Added API to read status register:
*			 u32 XilSKey_EfusePs_ReadStatus(
*				XilSKey_EPs *InstancePtr, u32 *StatusBits)
*                        RSA key read back is stored in RsaKeyReadback in
*                        Instance structure instead of RsaKeyHashValue -
*			 Change in API:
*			 u32 XilSKey_EfusePs_Read(XilSKey_EPs *PsInstancePtr)
* 2.00  hk      23/01/14 Changed PS efuse error codes for voltage out of range
* 2.1   sk      04/03/15 Initialized RSAKeyReadback with Zeros CR# 829723.
*
*
*****************************************************************************/

#ifndef XILSKEY_EPS_H
#define XILSKEY_EPS_H

/**
 * Key length definition for RSA KEY Hash
 */
#define XSK_EFUSEPS_RSA_KEY_HASH_LEN_IN_BYTES			(32)

/**
 * XSKEfusePs is the PS eFUSE driver instance. Using this
 * structure, user can define the eFUSE bits to be
 * blown.
 */
typedef struct {
	/**
	 * EnableWriteProtect:Enable the eFUSE Array write protection
	 */
	u32 EnableWriteProtect;
	/**
	 * EnableRsaAuth: Enable the RSA Authentication eFUSE Bit
	 */
	u32 EnableRsaAuth;
	/**
	 * Enable the ROM code 128K crc  eFUSE Bit
	 */
	u32 EnableRom128Crc;
	/**
	 * EnableRsaKeyHash: Enabling this RsaKeyHashValue[32] is written to
	 * eFUSE array
	 */
	u32 EnableRsaKeyHash;
	/**
	 * RsaKeyHashValue: RSA key Hash
	 */
	u8 RsaKeyHashValue[XSK_EFUSEPS_RSA_KEY_HASH_LEN_IN_BYTES];
	/**
	 * Rsa key read
	 */
	u8 RsaKeyReadback[XSK_EFUSEPS_RSA_KEY_HASH_LEN_IN_BYTES];
} XilSKey_EPs;

/**
 * PS eFUSE interface functions
 */
u32 XilSKey_EfusePs_Write(XilSKey_EPs *PsInstancePtr);
u32 XilSKey_EfusePs_Read (XilSKey_EPs *PsInstancePtr);
u32 XilSKey_EfusePs_ReadStatus(XilSKey_EPs *InstancePtr, u32 *StatusBits);

#endif /* XILSKEY_EPS_H */
