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
* @note
*
*****************************************************************************/
int XilSKey_Bbram_Program(XilSKey_Bbram *InstancePtr)
{
	u32 ArmPllFdiv;
	u32 ArmClkDivisor;
	u32 RefClk;
	int Status;

	if(NULL == InstancePtr)	{
		return XST_FAILURE;
	}

	/**
	 *  Extract PLL FDIV value from ARM PLL Control Register
	 */
	ArmPllFdiv = (Xil_In32(XSK_ARM_PLL_CTRL_REG)>>12 & 0x7F);

	/**
	 *  Extract Clock divisor value from ARM Clock Control Register
	 */
	ArmClkDivisor = (Xil_In32(XSK_ARM_CLK_CTRL_REG)>>8 & 0x3F);

	/**
	 * Initialize the variables
	 */
	RefClk = ((XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ * ArmClkDivisor)/
				ArmPllFdiv);

	/*
	 * Initialize and start the timer
	 */
	XilSKey_Efuse_StartTimer(RefClk);

	/*
	 * JTAG server initialization
	 */
	if(JtagServerInitBbram(InstancePtr) != XST_SUCCESS) {
		return XST_FAILURE;
	}

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

	return XST_SUCCESS;
}

