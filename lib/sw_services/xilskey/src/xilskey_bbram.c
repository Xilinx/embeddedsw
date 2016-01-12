/******************************************************************************
*
* Copyright (C) 2013 - 2016 Xilinx, Inc.  All rights reserved.
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
* @file
* 			xilskey_bbram.c
* @note
*
*  			.
*
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.01a hk      09/18/13 First release
* 3.00  vns     31/07/15 Removed redundant code to initialise timer.
* 5.00  vns     09/01/16 Added BBRAM programming functionality for Ultrascale
*
****************************************************************************/
/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_types.h"
#include "xilskey_utils.h"
#include "xilskey_bbram.h"

/************************** Constant Definitions *****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Variable Definitions ****************************/
/************************** Function Prototypes *****************************/
/**
 * 	JTAG Server Initialization routine for Bbram
 */
extern int JtagServerInitBbram(XilSKey_Bbram *InstancePtr);

/**
 * BBRAM Algorithm - Initialization
 */
extern int Bbram_Init(XilSKey_Bbram *InstancePtr);

/**
 * BBRAM Algorithm - Program key
 */
extern int Bbram_ProgramKey(XilSKey_Bbram *InstancePtr);

/**
 * BBRAM Algorithm - Verify key
 */
extern int Bbram_VerifyKey(XilSKey_Bbram *InstancePtr);

/**
 * De-initialization
 */
extern void Bbram_DeInit(void);

/* BBRAM Algorithm - Initialization */
extern int Bbram_Init_Ultra(XilSKey_Bbram *InstancePtr);

/* BBRAM Algorithm - Program key */
extern int Bbram_ProgramKey_Ultra(XilSKey_Bbram *InstancePtr);

/* BBRAM Algorithm - Verify key */
extern int Bbram_VerifyKey_Ultra(u32 *Crc);

/* De-initialization */
extern void Bbram_DeInit_Ultra(void);

/* Programming Zynq Bbram */
static inline int XilSKey_Bbram_Program_Zynq(XilSKey_Bbram *InstancePtr);

/* Programming Ultrascale Bbram */
static inline int XilSKey_Bbram_Program_Ultra(XilSKey_Bbram *InstancePtr);

/* CRC calculation of AES key */
static inline u32 XilSKey_Bbram_CrcCalc_Ultra(u32 *AesKey, u32 CtrlWord);

/***************************************************************************/
/****************************************************************************/
/**
*
* This function implements the BBRAM algorithm for programming and
* verifying key. The program and verify will only work together in and
* in that order.
*
* @param  BBRAM instance pointer
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		This function will program BBRAM of Ultrascale and Zynq as
*			well.
*
*****************************************************************************/
int XilSKey_Bbram_Program(XilSKey_Bbram *InstancePtr)
{
	u32 RefClk;
	int Status;

	if(NULL == InstancePtr)	{
		return XST_FAILURE;
	}

	/* Get timer values */
	RefClk = XilSKey_Timer_Intialise();
	/*
	 * Initialize and start the timer
	 */
	XilSKey_Efuse_StartTimer();

	/*
	 * JTAG server initialization
	 */
	if(JtagServerInitBbram(InstancePtr) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if (InstancePtr->FpgaFlag == XSK_FPGA_SERIES_ZYNQ) {
		Status = XilSKey_Bbram_Program_Zynq(InstancePtr);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}
	else {
		Status = XilSKey_Bbram_Program_Ultra(InstancePtr);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}



	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This API programs and verifies BBRAM of Zynq.
*
* @param	InstancePtr is BBRAM's instance.
*
* @return
*		- XST_SUCCESS if programming and verification of BBRAM is
*		successful.
*		- XST_FAILURE if fails.
*
* @note		None.
*
*****************************************************************************/
static inline int XilSKey_Bbram_Program_Zynq(XilSKey_Bbram *InstancePtr)
{
	int Status = XST_SUCCESS;

	/*
	 * BBRAM Algorithm initialization
	 */
	Status = Bbram_Init(InstancePtr);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * BBRAM - Program key
	 */
	Status = Bbram_ProgramKey(InstancePtr);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * BBRAM - Verify key
	 */
	Status = Bbram_VerifyKey(InstancePtr);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * De-initialization
	 */
	Bbram_DeInit();

	return Status;

}

/*****************************************************************************/
/**
*
* This API programs Ultrascale BBRAM, verifies BBRAM programmed key with CRC
* check and de initialises.
*
* @param	InstancePtr is BBRAM's instance.
*
* @return
*		- XST_SUCCESS if programming and verification of BBRAM is
*		successful.
*		- XST_FAILURE if fails.
*
* @note 	None.
*
******************************************************************************/
static inline int XilSKey_Bbram_Program_Ultra(XilSKey_Bbram *InstancePtr)
{
	int Status = XST_SUCCESS;

	/*
	 * CRC of provided AES key and control word is being
	 * calculated for programming CRC along with Key and control
	 * in BBRAM. CRC is also used for verification while reading
	 * CRC of BBRAM.
	 */
	InstancePtr->Crc =
		XilSKey_Bbram_CrcCalc_Ultra((u32 *)(InstancePtr->AESKey),
					XSK_CTRL_WORD_BBRAM_ULTRA);
	/*
	 * BBRAM Algorithm initialization
	 */
	Status = Bbram_Init_Ultra(InstancePtr);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * BBRAM - Program key
	 */
	Status = Bbram_ProgramKey_Ultra(InstancePtr);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * BBRAM - Verify key
	 */
	Status = Bbram_VerifyKey_Ultra(&(InstancePtr->Crc));
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * De-initialization
	 */
	Bbram_DeInit_Ultra();

	return Status;

}

/*****************************************************************************/
/**
*
* This API will calculate CRC of Ultrascale BBRAM's AES key and control word.
*
* @param	AesKey is a 256 bit key of BBRAM.
* @param	CtrlWord is a 32 bit control word of BBRAM
*
* @return	CRC of AES key and Control word will be returned.

* @note		This API calculates CRC for all the 9 rows which are
*		programmed on BBRAM.
*
******************************************************************************/
static inline u32 XilSKey_Bbram_CrcCalc_Ultra(u32 *AesKey, u32 CtrlWord)
{
	u32 Crc = 0;
	u32 Index;

	/* CRC is calculated for AES key 8 rows and control word one row */
	for (Index = 0; Index < 9; Index++) {
		if (Index != 0) {
			Crc = XilSKey_RowCrcCalculation(Crc,
					AesKey[8 - Index], 9 - Index);
		}
		else {
			Crc = XilSKey_RowCrcCalculation(Crc, CtrlWord, 9);
		}

	}

	return Crc;

}
