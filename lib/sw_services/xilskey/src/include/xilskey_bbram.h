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
*
* 		xilskey_bbram.h
* @note
*		 Contains the function prototypes, defines and macros for
*		 BBRAM functionality.
*
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.01a hk      09/18/13 First release
*
****************************************************************************/
#ifndef XILSKEY_BBRAM_H
#define XILSKEY_BBRAM_H

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/
/************************** Constant Definitions *****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Variable Definitions ****************************/
typedef struct {

	/**
	 * If XTRUE then part has to be power cycled to be able to be reconfigured
	 */
	u32	ForcePowerCycle;
	/**
	 * If XTRUE then permanently sets the Zynq ARM DAP controller in bypass mode
	 */
	u32 JtagDisable;
	/**
	 * This is for the aes_key value
	 */;
	u8 AESKey[32];
	/**
	 * TDI MIO Pin Number
	 */
	u32 JtagMioTDI;
	/**
	 * TDO MIO Pin Number
	 */
	u32 JtagMioTDO;
	/**
	 * TCK MIO Pin Number
	 */
	u32 JtagMioTCK;
	/**
	 * TMS MIO Pin Number
	 */
	u32 JtagMioTMS;
	/**
	 * MUX Selection MIO Pin Number
	 */
	u32 JtagMioMuxSel;
	/**
	 * Value on the MUX Selection line
	 */
	u32	JtagMuxSelLineDefVal;

}XilSKey_Bbram;
/************************** Constant Definitions *****************************/
/*
 * Constant definitions for instruction used in BBRAM key program and verify
 */
#define JPROGRAM 0x0B
#define ISC_NOOP 0x14
#define ISC_ENABLE 0x10
#define ISC_PROGRAM_KEY 0x12
#define ISC_PROGRAM 0x11
#define ISC_READ 0x15
#define ISC_DISABLE 0x16
#define BYPASS 0x3F

/*
 * Pre and post pads
 */
#define IRHEADER	0
#define IRTRAILER	4
#define DRHEADER	0
#define DRTRAILER	1

/*
 * Pre and post pads for BYPASS in de-init
 */
#define IRHEADER_BYP	0
#define IRTRAILER_BYP	0
#define DRHEADER_BYP	0
#define DRTRAILER_BYP	0

/*
 * Instruction load length
 */
#define IRLENGTH	6

/*
 * Data register lengths for program
 */
#define DRLENGTH_PROGRAM	32
/*
 * Data register lengths for verify
 */
#define DRLENGTH_VERIFY		37
/*
 * Data register lengths for data load after ISC_ENABLE
 */
#define DRLENGTH_EN		5
/*
 * Data register load after ISC_ENABLE
 */
#define DR_EN		0x15

/*
 * Timer function load for 100msec
 */
#define TIMER_100MS	1000000

/*
 * IRCAPTURE status - init complete mask
 */
#define INITCOMPLETEMASK	0x10

/*
 * Number of char's in a KEY
 */
#define NUMCHARINKEY	32
/*
 * Number of words in a KEY
 */
#define NUMWORDINKEY	8

/*
 * Data register shift before key verify
 */
#define DATAREGCLEAR	0x1FFFFFFFE0

/*
 * Number of LSB status bits in 37 bit read to shifted out
 */
#define NUMLSBSTATUSBITS	5

/*
 * Data and instruction loads in de-init
 */
#define IRDEINIT_H		0x03
#define IRDEINIT_L		0xFF
#define DRDEINIT		0x00

/*
 * Data and instruction lenghts in de-init
 */
#define IRDEINITLEN		10
#define DRDEINITLEN		2

/************************** Function Prototypes *****************************/
/*
 * Function for BBRAM program and vefiry algorithm
 */
int XilSKey_Bbram_Program(XilSKey_Bbram *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif	/* End of XILSKEY_BBRAM_H */
