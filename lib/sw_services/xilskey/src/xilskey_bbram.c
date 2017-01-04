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
* 6.0   vns     07/28/16 Added configuration counting feature and Obfuscated key
*                        programming and calculation of ECC for 28 bits
*                        of control word.
*
****************************************************************************/
/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_types.h"
#include "xilskey_utils.h"
#include "xilskey_bbram.h"

/************************** Constant Definitions *****************************/

/* Control word masks */
#define XSK_BBRAM_CTRL_ECC_MASK			(0x003F) /**< Error detection
							   *  mask */
#define XSK_BBRAM_CTRL_RSRVD_MASK		(0x0440) /**< Reserved bits */
#define XSK_BBRAM_CTRL_BLACK_KEY_MASK		(0x0300) /**< BLACK key mask */
#define XSK_BBRAM_CTRL_BLACK_KEY_SHIFT		(8)	/**< BLACK key shift */
#define XSK_BBRAM_CTRL_MODE_MASK		(0x3000) /**< DPA mode mask */
#define XSK_BBRAM_CTRL_MODE_SHIFT		(12)	/**< DPA mode shift */
#define XSK_BBRAM_CTRL_DPA_PROTECT_MASK		(0xC000) /**< DPA
							   * Protection mask */
#define XSK_BBRAM_CTRL_DPA_PROTECT_SHIFT	(14)	/**< DPA protection
							  *  shift */
#define XSK_BBRAM_CTRL_RD_DPA_COUNT_MASK	(0x00FF0000)
							/**< Redundant DPA
							  * count mask */
#define XSK_BBRAM_CTRL_RD_DPA_COUNT_SHIFT	(16)	/**< Redundant DPA
							  *  count shift */
#define XSK_BBRAM_CTRL_DPA_COUNT_MASK		(0xFF000000)
							/**< DPA count mask */
#define XSK_BBRAM_CTRL_DPA_COUNT_SHIFT		(24)	/**< DPA count shift */

#define XSK_BBRAM_CTRL_DEFAULT_VAL		(0x1)	/**< Enable value */
#define XSK_BBRAM_CTRL_ENABLE_VAL		(0x2)	/**< Value for
							  *  default state */

/* DEF masks for calculating ECC of 28 bit 0f a control word */
#define XSK_BBRAM_P0_MASK	0X36AD555
#define XSK_BBRAM_P1_MASK	0X2D9B333
#define XSK_BBRAM_P2_MASK	0X1C78F0F
#define XSK_BBRAM_P3_MASK	0X03F80FF
#define XSK_BBRAM_P4_MASK	0X0007FFF

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
extern int Bbram_Init(void);

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
extern int Bbram_Init_Ultra(void);

/* BBRAM Algorithm - Program key */
extern int Bbram_ProgramKey_Ultra(XilSKey_Bbram *InstancePtr);

/* BBRAM Algorithm - Verify key */
extern int Bbram_VerifyKey_Ultra(u32 *Crc);

/* De-initialization */
extern void Bbram_DeInit_Ultra(void);
/* Calculates CRC of a Row */
u32 XilSKey_RowCrcCalculation(u32 PrevCRC, u32 Data, u32 Addr);

/* Programming Zynq Bbram */
static inline int XilSKey_Bbram_Program_Zynq(XilSKey_Bbram *InstancePtr);

/* Programming Ultrascale Bbram */
static inline int XilSKey_Bbram_Program_Ultra(XilSKey_Bbram *InstancePtr);

/* CRC calculation of AES key */
static inline u32 XilSKey_Bbram_CrcCalc_Ultra(u32 *AesKey, u32 CtrlWord);

/* Framing control word of Ultrascale */
static inline void XilSKey_Bbram_Framing_Ctrl_Word_Ultra(
					XilSKey_Bbram *InstancePtr);

/* To frame control word's ECC */
static inline u32 XilSKey_Calc_Ecc_Bbram_ultra(u32 ControlWord);
static inline u8 XilSKey_Calc_Row_Ecc_Bbram_Ultra(u8 *Value, u8 *Mask);
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
	int Status;

	if(NULL == InstancePtr)	{
		return XST_FAILURE;
	}

	/* Get timer values */
	XilSKey_Timer_Intialise();
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
	Status = Bbram_Init();
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

	/* Formation of control word */
	XilSKey_Bbram_Framing_Ctrl_Word_Ultra(InstancePtr);
	/*
	 * CRC of provided AES key and control word is being
	 * calculated for programming CRC along with Key and control
	 * in BBRAM. CRC is also used for verification while reading
	 * CRC of BBRAM.
	 */
	InstancePtr->Crc =
		XilSKey_Bbram_CrcCalc_Ultra((u32 *)(InstancePtr->AESKey),
					InstancePtr->CtrlWord);
	/*
	 * BBRAM Algorithm initialization
	 */
	Status = Bbram_Init_Ultra();
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

/*****************************************************************************/
/**
*
* This API will calculate control word based on the user inputs.
*
* @param	InstancePtr is an instance of BBRAM.
*
* @return	None.

* @note		None.
*
******************************************************************************/
static inline void XilSKey_Bbram_Framing_Ctrl_Word_Ultra(
				XilSKey_Bbram *InstancePtr)
{

	/* Configure DPA protection */
	if (InstancePtr->Enable_DpaProtect == TRUE) {
		InstancePtr->CtrlWord = (XSK_BBRAM_CTRL_ENABLE_VAL <<
					XSK_BBRAM_CTRL_DPA_PROTECT_SHIFT) &
					XSK_BBRAM_CTRL_DPA_PROTECT_MASK;
	}
	else {
		InstancePtr->CtrlWord = (XSK_BBRAM_CTRL_DEFAULT_VAL <<
					XSK_BBRAM_CTRL_DPA_PROTECT_SHIFT) &
					XSK_BBRAM_CTRL_DPA_PROTECT_MASK;
	}
	/* Configure DPA mode selection */
	if (InstancePtr->Dpa_Mode == TRUE) {
		InstancePtr->CtrlWord |= (XSK_BBRAM_CTRL_ENABLE_VAL <<
					XSK_BBRAM_CTRL_MODE_SHIFT) &
					XSK_BBRAM_CTRL_MODE_MASK;
	}
	else {
		InstancePtr->CtrlWord |= (XSK_BBRAM_CTRL_DEFAULT_VAL <<
					XSK_BBRAM_CTRL_MODE_SHIFT) &
					XSK_BBRAM_CTRL_MODE_MASK;
	}
	/* Configure DPA count */
	InstancePtr->CtrlWord |= (InstancePtr->Dpa_Count <<
				XSK_BBRAM_CTRL_RD_DPA_COUNT_SHIFT) &
				XSK_BBRAM_CTRL_RD_DPA_COUNT_MASK;
	InstancePtr->CtrlWord |= (InstancePtr->Dpa_Count <<
				XSK_BBRAM_CTRL_DPA_COUNT_SHIFT) &
				XSK_BBRAM_CTRL_DPA_COUNT_MASK;
	/* DPA reserved bits */
	InstancePtr->CtrlWord |= XSK_BBRAM_CTRL_RSRVD_MASK;

	/* Configure Is key is black key or not */
	if (InstancePtr->IsKeyObfuscated == TRUE) {
		InstancePtr->CtrlWord |= (XSK_BBRAM_CTRL_ENABLE_VAL <<
				XSK_BBRAM_CTRL_BLACK_KEY_SHIFT) &
				XSK_BBRAM_CTRL_BLACK_KEY_MASK;
	}
	else {
		InstancePtr->CtrlWord |= (XSK_BBRAM_CTRL_DEFAULT_VAL <<
				XSK_BBRAM_CTRL_BLACK_KEY_SHIFT) &
				XSK_BBRAM_CTRL_BLACK_KEY_MASK;
	}
	/*
	* Calculates ECC of upper 26 bits of control word and places
	* them in LSB 5 bits
	*/
	InstancePtr->CtrlWord = XilSKey_Calc_Ecc_Bbram_ultra(
					InstancePtr->CtrlWord);
}

/*****************************************************************************/
/**
*
* This API calculates XOR and Ecc for provided inputs.
*
* @param	Value is a pointer which holds 26 bits of the control word, on
* 			which Ecc needs to be calculated
* @param	Mask contains 26 bits of mask value.
*
* @return	Ecc of value and Mask will be returned

* @note		None.
*
******************************************************************************/
static inline u8 XilSKey_Calc_Row_Ecc_Bbram_Ultra(u8 *Value, u8 *Mask)
{
	u32 Index;
	u8 Xor_val = 0;
	u8 AndVal;

	for (Index = 0; Index < 26; Index++) {
		AndVal = (*(Value + Index) & *(Mask + Index));
		Xor_val = Xor_val ^ AndVal;
	}

	return Xor_val;

}

/*****************************************************************************/
/**
*
* This API calculates the error code detection for control word of BBRAM
* Ultrascale
*
* @param	CtrlWord is a 32 bit control word of BBRAM
*
* @return	Control word with Ecc will be returned.

* @note		This API calculates Ecc for 26 bits of the control word.
*
******************************************************************************/
static inline u32 XilSKey_Calc_Ecc_Bbram_ultra(u32 ControlWord)
{
	u32 P0_Mask = XSK_BBRAM_P0_MASK;
	u32 P1_Mask = XSK_BBRAM_P1_MASK;
	u32 P2_Mask = XSK_BBRAM_P2_MASK;
	u32 P3_Mask = XSK_BBRAM_P3_MASK;
	u32 P4_Mask = XSK_BBRAM_P4_MASK;
	u8 P0Mask[26];
	u8 P1Mask[26];
	u8 P2Mask[26];
	u8 P3Mask[26];
	u8 P4Mask[26];
	u8 Value_Bits[26];
	u8 Row;
	u32 Xor_Val0 = 0;
	u32 Xor_Val1 = 0;
	u32 Xor_Val2 = 0;
	u32 Xor_Val3 = 0;
	u32 Xor_Val4 = 0;
	u32 Xor_Val5 = 0;
	u32 Value;

	/* Ecc should be calculated on upper 26 bits */
	Value = ControlWord >> 6;

	XilSKey_Efuse_ConvertBitsToBytes((u8 *)&P0_Mask, P0Mask, 26);
	XilSKey_Efuse_ConvertBitsToBytes((u8 *)&P1_Mask, P1Mask, 26);
	XilSKey_Efuse_ConvertBitsToBytes((u8 *)&P2_Mask, P2Mask, 26);
	XilSKey_Efuse_ConvertBitsToBytes((u8 *)&P3_Mask, P3Mask, 26);
	XilSKey_Efuse_ConvertBitsToBytes((u8 *)&P4_Mask, P4Mask, 26);
	XilSKey_Efuse_ConvertBitsToBytes((u8 *)&Value, Value_Bits, 26);

	Xor_Val0 = XilSKey_Calc_Row_Ecc_Bbram_Ultra(Value_Bits, P0Mask);
	Xor_Val1 = XilSKey_Calc_Row_Ecc_Bbram_Ultra(Value_Bits, P1Mask);
	Xor_Val2 = XilSKey_Calc_Row_Ecc_Bbram_Ultra(Value_Bits, P2Mask);
	Xor_Val3 = XilSKey_Calc_Row_Ecc_Bbram_Ultra(Value_Bits, P3Mask);
	Xor_Val4 = XilSKey_Calc_Row_Ecc_Bbram_Ultra(Value_Bits, P4Mask);

	for (Row = 0; Row < 26; Row++) {
		Xor_Val5 = Xor_Val5 ^ Value_Bits[Row];
	}

	Xor_Val5 = Xor_Val5 ^ Xor_Val4 ^ Xor_Val3 ^
			Xor_Val2 ^ Xor_Val1 ^ Xor_Val0;

	Value = (Value << 6) | ((Xor_Val5 << 0) |
				(Xor_Val4 << 1) |
				(Xor_Val3 << 2) |
				(Xor_Val2 << 3) |
				(Xor_Val1 << 4) |
				(Xor_Val0 << 5));

	return Value;
}
