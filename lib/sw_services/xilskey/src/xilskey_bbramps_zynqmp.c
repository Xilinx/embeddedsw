/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 6.6   vns     06/06/18 Added doxygen tags
* 6.7	arc     01/05/19 Fixed MISRA-C violations.
*       arc     25/02/19 Added asserts for pointer parameter for NULL
*                        verification
*                        Added Timeouts and status info for the functions
*                        XilSKey_ZynqMp_Bbram_Program
*                        XilSKey_ZynqMp_Bbram_Zeroise
* 6.7   psl     03/21/19 Fixed MISRA-C violation.
* 6.8   psl     08/13/19 Fixed MISRA-C violation.
*       vns     08/29/19 Initialized Status variables
* 6.9   kal     01/31/20 Disable BBRAM programming after AES key programming.
*       vns     03/18/20 Fixed Armcc compilation errors
* 7.1   am      11/30/20 Resolved MISRA C violations
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xilskey_bbramps_zynqmp_hw.h"

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static INLINE u32 XilSKey_ZynqMp_Bbram_PrgrmEn(void);
static INLINE u32 XilSKey_ZynqMp_Bbram_CrcCalc(const u32 *AesKey);
static INLINE void XilSKey_ZynqMp_Bbram_PrgrmDisable(void);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function implements the BBRAM programming and verifying the key written.
* Program and verification of AES will work only together.
* CRC of the provided key will be calculated internally and verified after
* programming.
*
* @param	AesKey	Pointer to the key which has to be programmed.
*
* @return
* 		- Error code from XskZynqMp_Ps_Bbram_ErrorCodes enum if it fails
* 		- XST_SUCCESS if programming is done.
*
******************************************************************************/
u32 XilSKey_ZynqMp_Bbram_Program(const u32 *AesKey)
{

	u32 Status = (u32)XST_FAILURE;
	u32 AesCrc;
	const u32 *KeyPtr = AesKey;
	u32 StatusRead = 0U;
	u32 Offset;
	u32 TimeOut = 0U;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(AesKey != NULL);

	/* Set in programming mode */
	Status = XilSKey_ZynqMp_Bbram_PrgrmEn();
	if (Status != (u32)XST_SUCCESS) {
		Status = (Status | (u32)XSK_ZYNQMP_BBRAMPS_ERROR_IN_PRGRMG);
		goto END;
	}

	/* Program with provided key and check key written */
	Offset = XSK_ZYNQMP_BBRAM_0_OFFSET;
	while (Offset <= XSK_ZYNQMP_BBRAM_7_OFFSET) {
		XilSKey_WriteReg(XSK_ZYNQMP_BBRAM_BASEADDR, Offset, *KeyPtr);
		KeyPtr++;
		Offset = Offset + 4U;
	}

	/* Calculate CRC of AES */
	AesCrc = XilSKey_ZynqMp_Bbram_CrcCalc(AesKey);

	XilSKey_WriteReg(XSK_ZYNQMP_BBRAM_BASEADDR,
			XSK_ZYNQMP_BBRAM_AES_CRC_OFFSET, AesCrc);

	while (TimeOut < XSK_POLL_TIMEOUT) {
		/* Check for CRC done */
		StatusRead = XilSKey_ReadReg(XSK_ZYNQMP_BBRAM_BASEADDR,
					XSK_ZYNQMP_BBRAM_STS_OFFSET);
		if ((StatusRead & (u32)XSK_ZYNQMP_BBRAM_STS_AES_CRC_DONE_MASK)
							!= 0x00U) {
			break;
		}
		TimeOut = TimeOut + 1U;
	}

	if ((StatusRead & (u32)XSK_ZYNQMP_BBRAM_STS_AES_CRC_DONE_MASK)
							== 0x00U) {
		Status = (u32)XSK_ZYNQMP_BBRAMPS_ERROR_IN_WRITE_CRC;
		goto END;
	}

	if ((StatusRead & XSK_ZYNQMP_BBRAM_STS_AES_CRC_PASS_MASK) !=
				XSK_ZYNQMP_BBRAM_STS_AES_CRC_PASS_MASK) {
		Status = (u32)XSK_ZYNQMP_BBRAMPS_ERROR_IN_CRC_CHECK;
		goto END;
	}
END:
	XilSKey_ZynqMp_Bbram_PrgrmDisable();

	return Status;
}

/*****************************************************************************/
/**
*
* This function zeroize's Bbram Key.
*
* @note		BBRAM key will be zeroized.
*
******************************************************************************/
u32 XilSKey_ZynqMp_Bbram_Zeroise(void)
{

	u32 Status = (u32)XST_FAILURE;
	u32 Offset;
	u32 Timeout = 0U;

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
		Offset = Offset + 4U;
	}

	/* Issue the zeroize command */
	XilSKey_WriteReg(XSK_ZYNQMP_BBRAM_BASEADDR, XSK_ZYNQMP_BBRAM_CTRL_OFFSET,
				XSK_ZYNQMP_BBRAM_CTRL_ZEROIZE_MASK);

	/* Wait for zeroize complete bit to get set */
	while (Timeout < XSK_POLL_TIMEOUT) {
		/* Read the status register */
		Status = XilSKey_ReadReg(XSK_ZYNQMP_BBRAM_BASEADDR,
						XSK_ZYNQMP_BBRAM_STS_OFFSET);

		if((Status & (u32)XSK_ZYNQMP_BBRAM_STS_ZEROIZED_MASK) != 0x00U) {
			Status = (u32)XST_SUCCESS;
			goto END;
		}
		Timeout = Timeout + 1U;
	}
END:
	return Status;

}

/*****************************************************************************/
/**
*
* This function enables programming and zeroizes Bbram.
*
* @return
*		- Error code from XskZynqMp_Ps_Bbram_ErrorCodes enum if it fails
*		- XST_SUCCESS if programming is done.
*
******************************************************************************/
static INLINE u32 XilSKey_ZynqMp_Bbram_PrgrmEn(void)
{

	u32 StatusRead = 0U;
	u32 Status = (u32)XST_FAILURE;
	u32 TimeOut = 0U;

	/*
	 * Always issue a zeroize command (since we may
	 * already be in programming mode and it may
	 * hang waiting for zeroize complete bit)
	 */
	Status = XilSKey_ZynqMp_Bbram_Zeroise();
	if(Status != (u32)XST_SUCCESS) {
		Status = (u32)XSK_ZYNQMP_BBRAMPS_ERROR_IN_ZEROISE;
		goto END;
	}

	/* Enter programming mode */
	XilSKey_WriteReg(XSK_ZYNQMP_BBRAM_BASEADDR,
	XSK_ZYNQMP_BBRAM_PGM_MODE_OFFSET,XSK_ZYNQMP_BBRAM_PGM_MODE_SET_VAL);

	while (TimeOut < XSK_POLL_TIMEOUT) {
		/* check for zeroized */
		StatusRead = XilSKey_ReadReg(XSK_ZYNQMP_BBRAM_BASEADDR,
					XSK_ZYNQMP_BBRAM_STS_OFFSET);

		if ((StatusRead & (u32)XSK_ZYNQMP_BBRAM_STS_ZEROIZED_MASK) !=
							0x00U) {
			break;
		}
		TimeOut = TimeOut + 1U;
	}

	if ((StatusRead & (u32)XSK_ZYNQMP_BBRAM_STS_ZEROIZED_MASK) ==
							0x00U) {
		Status = (u32)XSK_ZYNQMP_BBRAMPS_ERROR_IN_ZEROISE;
		goto END;
	}

	StatusRead =
		XilSKey_ReadReg(XSK_ZYNQMP_BBRAM_BASEADDR,
				XSK_ZYNQMP_BBRAM_STS_OFFSET);

	if ((StatusRead & XSK_ZYNQMP_BBRAM_STS_PGM_MODE_MASK) !=
				XSK_ZYNQMP_BBRAM_STS_PGM_MODE_MASK) {
		Status = (u32)XSK_ZYNQMP_BBRAMPS_ERROR_IN_PRGRMG_ENABLE;
		goto END;
	}
END:
	return Status;

}
/*****************************************************************************/
/**
*
* This function disables bbram programming.
*
******************************************************************************/
static INLINE void XilSKey_ZynqMp_Bbram_PrgrmDisable(void)
{
	XilSKey_WriteReg(XSK_ZYNQMP_BBRAM_BASEADDR,
			 XSK_ZYNQMP_BBRAM_PGM_MODE_OFFSET,
			 XSK_ZYNQMP_BBRAM_PGM_MODE_RSTVAL);

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
******************************************************************************/
static INLINE u32 XilSKey_ZynqMp_Bbram_CrcCalc(const u32 *AesKey)
{
	u32 Crc = 0U;
	u32 Index;
	u32 Key_32 = 0U;

	for (Index = 0U; Index < 9U ; Index++) {
		if (Index != 0U) {
			Crc =
				XilSKey_RowCrcCalculation(
					Crc, AesKey[(u32)8U - Index], (u32)9U - Index);
		}
		else {
			Crc = XilSKey_RowCrcCalculation(Crc, Key_32, (u32)9U-Index);
		}

	}

	return Crc;

}
