/******************************************************************************
*
* Copyright (C) 2015 - 18 Xilinx, Inc.  All rights reserved.
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
*
* @file xilskey_bbramps_zynqmp.c
*
* This file contains the implementation of the interface functions for
* programming BBRAM of ZynqMp.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.0   vns     10/08/15 First release
* 6.5   vns     03/16/18 Fixed hanging issue when program/zeroise is requested
*                        while programming mode in enabled state.
*       vns     04/20/18 Added Zeroization at starting of the function call
*                        If in case controller is not in programming mode
*                        zeroization occurs without latency.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xilskey_bbramps_zynqmp_hw.h"

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static inline u32 XilSKey_ZynqMp_Bbram_PrgrmEn();
static inline u32 XilSKey_ZynqMp_Bbram_CrcCalc(u32 *AesKey);
extern u32 XilSKey_RowCrcCalculation(u32 PrevCRC, u32 Data, u32 Addr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function implements the BBRAM programming and verifying the key written.
* Program and verification of AES will work only together.
* CRC of the provided key will be calculated internally and verified.
*
* @param	AesKey is a pointer to the key which has to be programmed.
*
* @return
* 		- Error code from XskZynqMp_Ps_Bbram_ErrorCodes enum if it fails
* 		- XST_SUCCESS if programming is done.
*
* @note		None.
*
******************************************************************************/
u32 XilSKey_ZynqMp_Bbram_Program(u32 *AesKey)
{

	u32 Status = XST_SUCCESS;
	u32 AesCrc;
	u32 *KeyPtr = AesKey;
	u32 StatusRead;
	u32 Offset;

	/* Calculate CRC of AES */
	AesCrc = XilSKey_ZynqMp_Bbram_CrcCalc(AesKey);

	/* Set in programming mode */
	Status = XilSKey_ZynqMp_Bbram_PrgrmEn();
	if (Status != XST_SUCCESS) {
		return (Status + XSK_ZYNQMP_BBRAMPS_ERROR_IN_PRGRMG);
	}

	/* Program with provided key and check key written */
	Offset = XSK_ZYNQMP_BBRAM_0_OFFSET;
	while (Offset <= XSK_ZYNQMP_BBRAM_7_OFFSET) {
		XilSKey_WriteReg(XSK_ZYNQMP_BBRAM_BASEADDR, Offset, *KeyPtr);
		KeyPtr++;
		Offset = Offset + 4;
	}

	XilSKey_WriteReg(XSK_ZYNQMP_BBRAM_BASEADDR,
			XSK_ZYNQMP_BBRAM_AES_CRC_OFFSET, AesCrc);

	/* Check for CRC done */
	StatusRead = XilSKey_ReadReg(XSK_ZYNQMP_BBRAM_BASEADDR,
					XSK_ZYNQMP_BBRAM_STS_OFFSET);
	while ((StatusRead & XSK_ZYNQMP_BBRAM_STS_AES_CRC_DONE_MASK)
							== 0x00) {
		StatusRead =
			XilSKey_ReadReg(XSK_ZYNQMP_BBRAM_BASEADDR,
					XSK_ZYNQMP_BBRAM_STS_OFFSET);
	}

	if ((StatusRead & XSK_ZYNQMP_BBRAM_STS_AES_CRC_PASS_MASK) !=
				XSK_ZYNQMP_BBRAM_STS_AES_CRC_PASS_MASK) {
		return XSK_ZYNQMP_BBRAMPS_ERROR_IN_CRC_CHECK;
	}

	return Status;

}

/*****************************************************************************/
/**
*
* This function zeroize's Bbram Key.
*
* @param	None.
*
* @return	None.
*
* @note		BBRAM key will be zeroized.
*
******************************************************************************/
void XilSKey_ZynqMp_Bbram_Zeroise()
{

	u32 Status;
	u32 Offset;

	/*
	 * If we are not in programming mode for zeroizing immediately
	 * without latency
	 */
	XilSKey_WriteReg(XSK_ZYNQMP_BBRAM_BASEADDR, XSK_ZYNQMP_BBRAM_CTRL_OFFSET,
					XSK_ZYNQMP_BBRAM_CTRL_ZEROIZE_MASK);

	/*
	 * Write all zeros to the data regs
	 * before issuing a zeroize command. Otherwise, we
	 * may hang waiting for zeroize complete bit if
	 * we were already in programming mode
	 */
	Offset = XSK_ZYNQMP_BBRAM_0_OFFSET;
	while (Offset <= XSK_ZYNQMP_BBRAM_7_OFFSET) {
		XilSKey_WriteReg(XSK_ZYNQMP_BBRAM_BASEADDR, Offset, 0x0U);
		Offset = Offset + 4;
	}

	/* Issue the zeroize comand */
	XilSKey_WriteReg(XSK_ZYNQMP_BBRAM_BASEADDR, XSK_ZYNQMP_BBRAM_CTRL_OFFSET,
				XSK_ZYNQMP_BBRAM_CTRL_ZEROIZE_MASK);

	/* Read the status register */
	Status = XilSKey_ReadReg(XSK_ZYNQMP_BBRAM_BASEADDR,
				XSK_ZYNQMP_BBRAM_STS_OFFSET);

	/* Wait for zeroize complete bit to get set */
	while ((Status & XSK_ZYNQMP_BBRAM_STS_ZEROIZED_MASK) == 0x00) {
		Status = XilSKey_ReadReg(XSK_ZYNQMP_BBRAM_BASEADDR,
						XSK_ZYNQMP_BBRAM_STS_OFFSET);
	}

}

/*****************************************************************************/
/**
*
* This function enables programming and zeroizes Bbram.
*
* @param	None
*
* @return
*		- Error code from XskZynqMp_Ps_Bbram_ErrorCodes enum if it fails
*		- XST_SUCCESS if programming is done.
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_Bbram_PrgrmEn()
{

	u32 Status = XST_SUCCESS;
	u32 StatusRead;

	/*
	 * Always issue a zeroize command (since we may
	 * already be in programming mode and it may
	 * hang waiting for zeroize complete bit)
	 */
	XilSKey_ZynqMp_Bbram_Zeroise();

	/* Enter programming mode */
	XilSKey_WriteReg(XSK_ZYNQMP_BBRAM_BASEADDR,
	XSK_ZYNQMP_BBRAM_PGM_MODE_OFFSET,XSK_ZYNQMP_BBRAM_PGM_MODE_SET_VAL);

	/* check for zeroized */
	StatusRead = XilSKey_ReadReg(XSK_ZYNQMP_BBRAM_BASEADDR,
				XSK_ZYNQMP_BBRAM_STS_OFFSET);

	while ((StatusRead & XSK_ZYNQMP_BBRAM_STS_ZEROIZED_MASK) ==
							0x00) {
		StatusRead =
		XilSKey_ReadReg(XSK_ZYNQMP_BBRAM_BASEADDR,
				XSK_ZYNQMP_BBRAM_STS_OFFSET);
	}

	StatusRead =
		XilSKey_ReadReg(XSK_ZYNQMP_BBRAM_BASEADDR,
				XSK_ZYNQMP_BBRAM_STS_OFFSET);

	if ((StatusRead & XSK_ZYNQMP_BBRAM_STS_PGM_MODE_MASK) !=
				XSK_ZYNQMP_BBRAM_STS_PGM_MODE_MASK) {
		return XSK_ZYNQMP_BBRAMPS_ERROR_IN_PRGRMG_ENABLE;
	}

	return Status;

}

/*****************************************************************************/
/**
*
* This function calculates CRC of AES key.
*
* @param	AesKey is a pointer to the key for which CRC has to be
*		calculated.
*
* @return	CRC of AES key
*
* @note		None.
*
******************************************************************************/
static inline u32 XilSKey_ZynqMp_Bbram_CrcCalc(u32 *AesKey)
{
	u32 Crc = 0;
	u32 Index;
	u32 Key_32 = 0;

	for (Index = 0; Index < 9 ; Index++) {
		if (Index != 0) {
			Crc =
				XilSKey_RowCrcCalculation(
					Crc, AesKey[8 - Index], 9-Index);
		}
		else {
			Crc = XilSKey_RowCrcCalculation(Crc, Key_32, 9-Index);
		}

	}

	return Crc;

}
