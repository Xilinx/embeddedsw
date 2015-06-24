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
* 			xilskey_epl.c
* @note
*
*  			Contains the function definitions for the PL eFUSE functionality.
*
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.00a rpoolla 04/26/13 First release
* 1.02a hk      10/28/13 Added API's to read status bits and key.PR# 735957
* 2.00  hk      22/01/14 Corrected PL voltage checks to VCCINT and VCCAUX.
*                        CR#768077
* 2.1   kvn     04/01/15 Fixed warnings. CR#716453.
*
*
****************************************************************************/
/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_types.h"
#include "xilskey_utils.h"
#include "xilskey_epl.h"
/************************** Constant Definitions *****************************/
#define XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW		(0) /**< Fuse Ctrl Row*/
#define XSK_EFUSEPL_ARRAY_FUSE_AES_KEY_SIZE		(256) /**< AES Key size*/
#define XSK_EFUSEPL_ARRAY_FUSE_USER_KEY_SIZE	(32) /**< User key size*/
#define XSK_EFUSEPL_ARRAY_MAX_ROW				(32) /**< PLeFUSE Max Rows*/
#define XSK_EFUSEPL_ARRAY_MAX_COL				(32) /**< PLeFUSE Max Columns*/
#define XSK_EFUSEPL_ARRAY_AES_DATA_ROW_START	(20) /**< AES Data Start Row*/
#define XSK_EFUSEPL_ARRAY_AES_DATA_ROW_END		(30) /**< AES Data End Row*/
#define XSK_EFUSEPL_ARRAY_USER_DATA_START_ROW	(31) /**< User Data Start Row*/
#define XSK_EFUSEPL_ARRAY_AES_DATA_BITS_IN_30th_ROW	(16) /**< AES Data bits
															* count in 30th Row
															*/
#define XSK_EFUSEPL_ARRAY_USER_DATA_BITS_IN_30th_ROW (8) /**< User Data bits
															* count in 30th Row
															*/
#define XSK_EFUSEPL_ARRAY_MAX_PAYLAOD_BITS_IN_A_ROW	(24)/**< Max Pay load
															* in Row
															*/
#define XSK_EFUSEPL_ARRAY_MAX_ECC_BITS_IN_A_ROW		(6)	/**< Max ECC bits
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * in a Row
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 */
#define XSK_EFUSEPL_ARRAY_ECC_START_BIT_IN_A_ROW	(24)/**< ECC Start Bit
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * position in
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * a Row
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 */
#define XSK_EFUSEPL_ARRAY_ECC_END_BIT_IN_A_ROW		(29)/**< ECC End Bit
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * position in
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * a Row
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 */
#define XSK_EFUSEPL_ARRAY_FUSE_CNTRL_MAX_BITS		(11)/**< Fuse Control max
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * bits
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 */
#define XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_INDEX (14)/**< Redundant bit
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * Index
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 */
#define XSK_EFUSEPL_ARRAY_FUSE_CNTRL_START_BIT	(0)	 /**< Fuse Control
															* Start bit
															*/
#define XSK_EFUSEPL_ARRAY_FUSE_CNTRL_END_BIT	(10) /**< Fuse Control
															* Start bit
															*/
#define XSK_EFUSEPL_ARRAY_UNSUPPORTED_BIT6		(6)	 /**< Unsupported bit*/
#define XSK_EFUSEPL_ARRAY_UNSUPPORTED_BIT7		(7)	 /**< Unsupporte bit*/
#define XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_START_BIT	(14)/**< Redundant
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * bit start Index
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 */
#define XSK_EFUSEPL_ARRAY_UNSUPPORTED_RED_FOR_BIT6	(20)/**< Unsupported bit*/
#define XSK_EFUSEPL_ARRAY_UNSUPPORTED_RED_FOR_BIT7	(21)/**< Unsupported bit*/
#define XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_END_BIT	(24)/**< Redundant bit
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 * End Index
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 */
#define XSK_EFUSEPL_MAX_REF_CLK_FREQ	60000000 /**< Max Ref Clk Frequency */
#define XSK_EFUSEPL_MIN_REF_CLK_FREQ	20000000 /**< Min Ref Clk Frequency */
/**************************** Type Definitions ******************************/
/**
 * Read or Write eFUSE Margin Options
 */
typedef enum {
	XSK_EFUSEPL_READ_NORMAL = 0x1,	/**< Margin 1*/
	XSK_EFUSEPL_READ_MARGIN_1 = 0x2, /**< Margin 2*/
	XSK_EFUSEPL_READ_MARGIN_2 = 0x4, /**< Margin 4*/
	XSK_EFUSEPL_READ_MARGIN_MAX = 0x7 /**< Max Margin 7*/
}XSK_EfusePl_MarginOption;
/**
 * Fuse Control Row Bit Indices
 */
typedef enum {
	XSK_EFUSEPL_CNTRL_FORCE_PCYCLE_RECONFIG = 0x01,/**< Bit1 of Fuse Ctrl Row*/
	XSK_EFUSEPL_CNTRL_DISABLE_KEY_WRITE,		   /**< Bit2 of Fuse Ctrl Row*/
	XSK_EFUSEPL_CNTRL_DISABLE_AES_KEY_READ,		   /**< Bit3 of Fuse Ctrl Row*/
	XSK_EFUSEPL_CNTRL_DISABLE_USER_KEY_READ,       /**< Bit4 of Fuse Ctrl Row*/
	XSK_EFUSEPL_CNTRL_DISABLE_FUSE_CNTRL_WRITE,    /**< Bit5 of Fuse Ctrl Row*/
	XSK_EFUSEPL_CNTRL_FORCE_USE_AES_ONLY = 0x08,   /**< Bit8 of Fuse Ctrl Row*/
	XSK_EFUSEPL_CNTRL_JTAG_CHAIN_DISABLE,          /**< Bit9 of Fuse Ctrl Row*/
	XSK_EFUSEPL_CNTRL_BBRAM_KEY_DISABLE           /**< Bit10 of Fuse Ctrl Row*/
}XSKEfusePl_FuseCntrlBits;
/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/
u32 ErrorCode;	/**< Global variable which holds the error key*/
static u8 AesDataInBytes[XSK_EFUSEPL_ARRAY_FUSE_AES_KEY_SIZE];
static u8 UserDataInBytes[XSK_EFUSEPL_ARRAY_FUSE_USER_KEY_SIZE];

/************************** Function Prototypes *****************************/
/**
 * PL eFUSE interface functions
 */
static u8 XilSKey_EfusePl_ProgramBit(u8 Row, u8 Bit);

static u8 XilSKey_EfusePl_ProgramRow(u8 Row, u8 *RowData);

static u8 XilSKey_EfusePl_ProgramControlRegister(u8 *CtrlData);

static u8 XilSKey_EfusePl_ReadBit(u8 Row, u8 Bit, u8 MarginOption, u8 *BitData);

static u8 XilSKey_EfusePl_ReadRow(u32 Row, u8 MarginOption, u8 *RowData);

static u8 XilSKey_EfusePl_ReadControlRegister(u8 *CtrlData);

static u8 XilSKey_EfusePl_VerifyBit(u8 Row, u8 Bit, u8 MarginOption);

static u8 XilSKey_EfusePl_IsVectorAllZeros(u8 *RowDataPtr);

static void XilSKey_EfusePl_CalculateEcc(u8 *RowData, u8 *ECCData);
/**
 * 	JTAG Server Initialization routine
 */
extern int JtagServerInit(XilSKey_EPl *PlInstancePtr);
/**
 * 	JTAG Server Write routine
 */
extern void JtagWrite(unsigned char row, unsigned char bit);
/**
 * 	JTAG Server Read routine
 */
extern void JtagRead(unsigned char row, unsigned int * row_data, unsigned char marginOption);
/***************************************************************************/
/****************************************************************************/
/**
*
*
*	Programs PL eFUSE with input data given
*
*
* @param	InstancePtr - Input data to be written to PL eFUSE
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
u32 XilSKey_EfusePl_Program(XilSKey_EPl *InstancePtr)
{
	u32 Row = 0;
	u8 RowData[XSK_EFUSEPL_ARRAY_MAX_COL]={0};
	u8 CtrlData[XSK_EFUSEPL_ARRAY_FUSE_CNTRL_MAX_BITS]={0};
	u32 Index = 0;
	u32 Status;
	u32 RefClk;
	u32 ArmPllFDiv,ArmClkDivisor;
	ErrorCode = XSK_EFUSEPL_ERROR_NONE;


	if(NULL == InstancePtr)	{
		return XSK_EFUSEPL_ERROR_PL_STRUCT_NULL;
	}

	if(!(InstancePtr->SystemInitDone))
	{
		/**
		 *  Extract PLL FDIV value from ARM PLL Control Register
		 */
		ArmPllFDiv = (Xil_In32(XSK_ARM_PLL_CTRL_REG)>>12 & 0x7F);

		/**
		 *  Extract Clock divisor value from ARM Clock Control Register
		 */
		ArmClkDivisor = (Xil_In32(XSK_ARM_CLK_CTRL_REG)>>8 & 0x3F);

		/**
		 * Initialize the variables
		 */
		RefClk = ((XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ *
				ArmClkDivisor)/ ArmPllFDiv);

		/**
		 * Return error if the reference clock frequency is not in
		 * between 20 & 60MHz
		 */
		if((RefClk < XSK_EFUSEPL_MIN_REF_CLK_FREQ) ||
				(RefClk > XSK_EFUSEPL_MAX_REF_CLK_FREQ)) {
			return XSK_EFUSEPL_ERROR_INVALID_REF_CLK;
		}
		/**
		 * Initialize the system ,
		 * which means initialize the timer, xadc, and jtag
		 * server using the passed info.
		 */

		XilSKey_Efuse_StartTimer(RefClk);

		Status = XilSKey_EfusePs_XAdcInit();
		if(Status != XST_SUCCESS) {
			ErrorCode = Status;
			return (XSK_EFUSEPL_ERROR_XADC + ErrorCode);
		}
		/**
		 * Start using the Jtag server to read the JTAG ID and
		 * compare with the stored ID, if it not matches return with
		 * unique error code.
		 * By reading the Jtag ID we will be sure that the JTAG related
		 * stuff is working as expected.
		 */
		if(JtagServerInit(InstancePtr) != XST_SUCCESS) {
			return XSK_EFUSEPL_ERROR_JTAG_SERVER_INIT;
		}

		InstancePtr->SystemInitDone = 1;

	}

	/**
	 *	Read the FUSE_CNTL register bits [5:2], and if any of them is found to
	 *	be set to 1 then we can not write to the eFUSE, so return with unique
	 *	error.
	 *
	 *	If everything is well above then start programming with FUSE_AES key as
	 *	passed to this function, followed by FUSE_USER if selected.
	 *
	 *	AES key is 256 bits, but has to be written across many rows, in the PL
	 *	eFUSE each row has 24 bits of data bits and 6 bits of ecc bits.
	 *	So for 256 bits we will need full 10 rows(240 bits) and extra 1 row for
	 *	remaining 16 bits, but rest 8 bits will be taken from the FUSE_USER key
	 *	value[7:0]
	 *
	 *	check if FUSE_CNTRL allows us to read and write the AES and USER eFUSE
	 *	array.
	 */


	if(XilSKey_EfusePl_ReadControlRegister(CtrlData) != XST_SUCCESS) {
		return (XSK_EFUSEPL_ERROR_READING_FUSE_CNTRL + ErrorCode);
	}

	if (((CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_AES_KEY_READ] == TRUE) ||
		(CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_USER_KEY_READ] == TRUE)||
		(CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_KEY_WRITE]== TRUE)) &&
		((InstancePtr->ProgAESandUserLowKey == TRUE) ||
		(InstancePtr->ProgUserHighKey == TRUE))) {
		return (XSK_EFUSEPL_ERROR_DATA_PROGRAMMING_NOT_ALLOWED + ErrorCode);
	}

	if((CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_KEY_WRITE] == TRUE) &&
	    ((InstancePtr->ForcePowerCycle == TRUE)||
	     (InstancePtr->KeyWrite == TRUE)||
	     (InstancePtr->AESKeyRead == TRUE)||
	     (InstancePtr->UserKeyRead == TRUE)||
	     (InstancePtr->UseAESOnly == TRUE)||
	     (InstancePtr->JtagDisable == TRUE)||
	     (InstancePtr->AESKeyExclusive == TRUE))) {
		return (XSK_EFUSEPL_ERROR_FUSE_CTRL_WRITE_NOT_ALLOWED + ErrorCode);
	}
	/**
	 * convert each aes data bits to bytes.
	 */
	XilSKey_Efuse_ConvertBitsToBytes(&InstancePtr->AESKey[0],
									AesDataInBytes,
									XSK_EFUSEPL_ARRAY_FUSE_AES_KEY_SIZE);

	/**
	 * convert each user data bits to bytes.
	 */
	XilSKey_Efuse_ConvertBitsToBytes(&InstancePtr->UserKey[0],
									UserDataInBytes,
									XSK_EFUSEPL_ARRAY_FUSE_USER_KEY_SIZE);


	if(InstancePtr->ProgAESandUserLowKey == TRUE) {
		/**
		 * check if row 20 to 30 are empty before programming AES and
		 * USER low key.
		 */
		for(Row=XSK_EFUSEPL_ARRAY_AES_DATA_ROW_START;
			Row<=XSK_EFUSEPL_ARRAY_AES_DATA_ROW_END;
			Row++) {
			if(XilSKey_EfusePl_ReadRow(Row, XSK_EFUSEPL_READ_NORMAL,
										   RowData) != XST_SUCCESS) {
				return (XSK_EFUSEPL_ERROR_READING_FUSE_AES_ROW + ErrorCode);
			}
			if(XilSKey_EfusePl_IsVectorAllZeros(RowData) != XST_SUCCESS) {
				return (XSK_EFUSEPL_ERROR_AES_ROW_NOT_EMPTY + ErrorCode);
			}
		}

		/**
		 * program AES_KEY 256 bits and USER_KEY lower 8 bits first.
		 */
		for(Row=XSK_EFUSEPL_ARRAY_AES_DATA_ROW_START;
				Row<=XSK_EFUSEPL_ARRAY_AES_DATA_ROW_END;
				Row++) {
			if(Row < XSK_EFUSEPL_ARRAY_AES_DATA_ROW_END) {
				/**
				 * prepare row data for row from 20 to 29.
				 */
				for(Index=0;
					Index<XSK_EFUSEPL_ARRAY_MAX_PAYLAOD_BITS_IN_A_ROW;
					Index++) {
						RowData[Index] = AesDataInBytes[
					                     (Index +
										 ((Row -
										 XSK_EFUSEPL_ARRAY_AES_DATA_ROW_START) *
								XSK_EFUSEPL_ARRAY_MAX_PAYLAOD_BITS_IN_A_ROW))];
				}
			}
			else
			{
				/**
				 * prepare row data for row 30.
				 */
				for(Index=0;
					Index<XSK_EFUSEPL_ARRAY_MAX_PAYLAOD_BITS_IN_A_ROW;
					Index++) {
					if(Index < XSK_EFUSEPL_ARRAY_AES_DATA_BITS_IN_30th_ROW) {
						RowData[Index] =
								AesDataInBytes[
						            (Index +
						           	((Row -
						              XSK_EFUSEPL_ARRAY_AES_DATA_ROW_START) *
						         XSK_EFUSEPL_ARRAY_MAX_PAYLAOD_BITS_IN_A_ROW))];
					}
					else {
						RowData[Index] =
								UserDataInBytes[Index -
								   XSK_EFUSEPL_ARRAY_AES_DATA_BITS_IN_30th_ROW];
					}
				}
			}

			if(XilSKey_EfusePl_ProgramRow(Row, RowData) != XST_SUCCESS) {
				return (XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_AES_ROW +
						ErrorCode);
			}
		}
	}

	if(InstancePtr->ProgUserHighKey == TRUE) {
		/**
		 * check if 31 is empty before programming USER high key.
		 */
		if(XilSKey_EfusePl_ReadRow(XSK_EFUSEPL_ARRAY_USER_DATA_START_ROW,
									   XSK_EFUSEPL_READ_NORMAL, RowData) !=
									   XST_SUCCESS) {
			return (XSK_EFUSEPL_ERROR_READING_FUSE_USER_DATA_ROW + ErrorCode);
		}


		if(XilSKey_EfusePl_IsVectorAllZeros(RowData) != XST_SUCCESS) {
			return (XSK_EFUSEPL_ERROR_USER_DATA_ROW_NOT_EMPTY + ErrorCode);
		}

		/**
		 * Program USER_KEY high 24 bits next.
		 * Prepare row data for row 31.
		 */

		for(Index=0; Index<XSK_EFUSEPL_ARRAY_MAX_PAYLAOD_BITS_IN_A_ROW; Index++) {
			RowData[Index] =
					UserDataInBytes[Index +
					              XSK_EFUSEPL_ARRAY_USER_DATA_BITS_IN_30th_ROW];
		}

		if(XilSKey_EfusePl_ProgramRow(XSK_EFUSEPL_ARRAY_USER_DATA_START_ROW,
										RowData) != XST_SUCCESS) {
			return (XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_DATA_ROW + ErrorCode);
		}
	}
	CtrlData[XSK_EFUSEPL_CNTRL_FORCE_PCYCLE_RECONFIG] = InstancePtr->ForcePowerCycle;
	CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_KEY_WRITE] 	= InstancePtr->KeyWrite;
	CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_AES_KEY_READ] = InstancePtr->AESKeyRead;
	CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_USER_KEY_READ] = InstancePtr->UserKeyRead;
	CtrlData[XSK_EFUSEPL_CNTRL_DISABLE_FUSE_CNTRL_WRITE] = InstancePtr->CtrlWrite;
	CtrlData[XSK_EFUSEPL_CNTRL_FORCE_USE_AES_ONLY] = InstancePtr->UseAESOnly;
	CtrlData[XSK_EFUSEPL_CNTRL_JTAG_CHAIN_DISABLE] = InstancePtr->JtagDisable;
	CtrlData[XSK_EFUSEPL_CNTRL_BBRAM_KEY_DISABLE] = InstancePtr->AESKeyExclusive;

	if(XilSKey_EfusePl_ProgramControlRegister(CtrlData) != XST_SUCCESS)	{
			return (XSK_EFUSEPL_ERROR_PROGRAMMING_FUSE_CNTRL_ROW + ErrorCode);
	}
	/**
	 * If everything is ok then return PASS.
	 */
	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
* Checks whether data bits (0-29) of an PL eFUSE row are all zeroes or not
*
*
*
* @param	RowDataPtr - row data pointer
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
* @note		None.
*
*****************************************************************************/
u8 XilSKey_EfusePl_IsVectorAllZeros(u8 *RowDataPtr)
{
	u32 Index;
	for(Index=0; Index<=XSK_EFUSEPL_ARRAY_ECC_END_BIT_IN_A_ROW; Index++) {
		if(RowDataPtr[Index] != 0) {
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Programs a bit of PL eFUSE row
*
*
*
* @param Row - row number
* @param Bit - bit position
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
u8 XilSKey_EfusePl_ProgramBit(u8 Row, u8 Bit)
{
	XSKEfusePs_XAdc PL_XAdc;
	/**
	 *Check if the row position is valid.
	 */
	if( (Row > XSK_EFUSEPL_ARRAY_USER_DATA_START_ROW) ||
			( (Row > XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW)
		     && (Row < XSK_EFUSEPL_ARRAY_AES_DATA_ROW_START)
			)
	  )
	{
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_ROW_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	/**
	 * Check if the bit position is valid.
	 */
	if (Bit > XSK_EFUSEPL_ARRAY_ECC_END_BIT_IN_A_ROW) {
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	/**
	 * If row=0 then bit should be either 1 to 5 and 8 to 10, 15 to 19 and
	 * 22 to 24 rest all are not supported
	 */
	if(Row == XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW) {

		if((Bit == XSK_EFUSEPL_ARRAY_FUSE_CNTRL_START_BIT) ||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_BIT6) ||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_BIT7)||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_RED_FOR_BIT6)||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_RED_FOR_BIT7)) {
			ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		if((Bit > XSK_EFUSEPL_ARRAY_FUSE_CNTRL_END_BIT) &&
		   (Bit < XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_START_BIT)) {
			ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		if(Bit > XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_END_BIT){
			ErrorCode = XSK_EFUSEPL_ERROR_WRITE_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		}
	}

	/**
	 * Monitor the Voltage and temperature using XADC, if out of range return
	 * unique error.
	 */
	PL_XAdc.VType = XSK_EFUSEPS_VAUX;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.Temp < XSK_EFUSEPL_WRITE_TEMP_MIN_RAW) ||
		(PL_XAdc.Temp > XSK_EFUSEPL_WRITE_TEMP_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_TMEPERATURE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	if((PL_XAdc.V < XSK_EFUSEPL_WRITE_VOLTAGE_VCCAUX_MIN_RAW) ||
		(PL_XAdc.V > XSK_EFUSEPL_WRITE_VOLTAGE_VCCAUX_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_VCCAUX_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	PL_XAdc.VType = XSK_EFUSEPS_VINT;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.V < XSK_EFUSEPL_WRITE_VOLTAGE_VCCINT_MIN_RAW) ||
	   (PL_XAdc.V > XSK_EFUSEPL_WRITE_VOLTAGE_VCCINT_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_VCCINT_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	JtagWrite(Row, Bit);
	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
* Programs PL eFUSE row
*
*
*
* @param	Row	- 		row number
* @param	RowData-	row data pointer
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/

u8 XilSKey_EfusePl_ProgramRow(u8 Row, u8 *RowData)
{
	u32 Bit = 0;
	u8 ECCData[XSK_EFUSEPL_ARRAY_MAX_ECC_BITS_IN_A_ROW] = {0};

	/**
	 * check if row_data is not NULL
	 */
	if(NULL == RowData) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BUFFER_NULL;
		return XST_FAILURE;
	}

	if(Row == XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW) {
		ErrorCode = XSK_EFUSEPL_ERROR_WRITE_ROW_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	for(Bit=0; Bit < XSK_EFUSEPL_ARRAY_MAX_PAYLAOD_BITS_IN_A_ROW ; Bit++ ) {
		if(RowData[Bit]) {
			if(XilSKey_EfusePl_ProgramBit(Row, Bit) != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if(XilSKey_EfusePl_VerifyBit(Row, Bit, XSK_EFUSEPL_READ_NORMAL)
											!= XST_SUCCESS) {
				return XST_FAILURE;
			}

			if(XilSKey_EfusePl_VerifyBit(Row,Bit,XSK_EFUSEPL_READ_MARGIN_1)
											!= XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
	}

	XilSKey_EfusePl_CalculateEcc(RowData, ECCData);
	for(Bit=XSK_EFUSEPL_ARRAY_ECC_START_BIT_IN_A_ROW;
		Bit <= XSK_EFUSEPL_ARRAY_ECC_END_BIT_IN_A_ROW;
		Bit++) {
		if(ECCData[Bit - XSK_EFUSEPL_ARRAY_ECC_START_BIT_IN_A_ROW]) {
			if(XilSKey_EfusePl_ProgramBit(Row, Bit) != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if(XilSKey_EfusePl_VerifyBit(Row, Bit, XSK_EFUSEPL_READ_NORMAL)
											!= XST_SUCCESS) {
				return XST_FAILURE;
			}

			if(XilSKey_EfusePl_VerifyBit(Row,Bit,XSK_EFUSEPL_READ_MARGIN_1)
											!= XST_SUCCESS) {
				return XST_FAILURE;
			}

		}
	}

	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
* Programs PL eFUSE Control Register
*
*
*
* @param	CtrlData - Control data pointer
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
u8 XilSKey_EfusePl_ProgramControlRegister(u8 *CtrlData)
{
	u8 TmpCtrlData[XSK_EFUSEPL_ARRAY_FUSE_CNTRL_MAX_BITS]={0};
	u32 Index = 0;

	/**
	 * check if cntrl_data is not NULL
	 */
	if(NULL == CtrlData) {
		ErrorCode = XSK_EFUSEPL_ERROR_CNTRL_WRITE_BUFFER_NULL;
		return XST_FAILURE;
	}

	/**
	 *	Read the FUSE_CNTRL register
	 */
	if(XilSKey_EfusePl_ReadControlRegister(TmpCtrlData) != XST_SUCCESS)	{
		return XST_FAILURE;
	}

	/**
	 * check if FUSE_CNTRL allows us to write FUSE_CNTRL eFUSE array.
	 */
	if(TmpCtrlData[XSK_EFUSEPL_CNTRL_DISABLE_FUSE_CNTRL_WRITE] == TRUE) {
		/**
		 * This means we cannot program FUSE_CNTRL register
		 */
		ErrorCode = XSK_EFUSEPL_ERROR_FUSE_CNTRL_WRITE_DISABLED;
		return XST_FAILURE;
	}


	for(Index=1;Index<XSK_EFUSEPL_ARRAY_FUSE_CNTRL_MAX_BITS;Index++)	{

		if((Index == 6) || (Index == 7)) {
			continue;
		}

		if((CtrlData[Index] == TRUE) && (TmpCtrlData[Index] == FALSE)) {
			if(XilSKey_EfusePl_ProgramBit(XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW,
											  Index) != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if(XilSKey_EfusePl_VerifyBit(XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW,
							Index, XSK_EFUSEPL_READ_NORMAL) != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if(XilSKey_EfusePl_VerifyBit(XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW,
							Index, XSK_EFUSEPL_READ_MARGIN_1) != XST_SUCCESS) {
				return XST_FAILURE;
			}
			if(XilSKey_EfusePl_ProgramBit(XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW,
							Index +
							XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_START_BIT)
							!= XST_SUCCESS) {
				return XST_FAILURE;
			}

			if(XilSKey_EfusePl_VerifyBit(XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW,
							(Index +
							XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_START_BIT),
							XSK_EFUSEPL_READ_NORMAL) != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if(XilSKey_EfusePl_VerifyBit(XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW,
							(Index +
							XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_START_BIT),
							XSK_EFUSEPL_READ_MARGIN_1) != XST_SUCCESS) {
				return XST_FAILURE;
			}

		}
	}

	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
*	Reads a PL eFUSE bit & stores.
*
*
* @param	Row			-	row number
* @param	Bit			- 	bit position in the specified row
* @param	MarginOption-	Margin Option(One of the reading method of PLeFUSE)
* @param	BitData		- 	Place holder to store the read value
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/

u8 XilSKey_EfusePl_ReadBit(u8 Row, u8 Bit, u8 MarginOption, u8 *BitData)
{
	u8 RowData[XSK_EFUSEPL_ARRAY_MAX_COL]={0};

	/**
	 * check if row_data is not NULL
	 */
	if(NULL == BitData)	{
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BUFFER_NULL;
		return XST_FAILURE;
	}
	/**
	 *Check if the bit position is valid.
	 */

	if (Bit > XSK_EFUSEPL_ARRAY_ECC_END_BIT_IN_A_ROW) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BIT_OUT_OF_RANGE;
		return XST_FAILURE;
	}


	/**
	 * If row=0 then bit should be either 1 to 5 and 8 to 10, 15 to 19 and
	 * 22 to 24 rest all are not supported
	 */
	if(Row == XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW) {

		if((Bit == XSK_EFUSEPL_ARRAY_FUSE_CNTRL_START_BIT) ||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_BIT6) ||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_BIT7)||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_RED_FOR_BIT6)||
			(Bit == XSK_EFUSEPL_ARRAY_UNSUPPORTED_RED_FOR_BIT7)) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		if((Bit > XSK_EFUSEPL_ARRAY_FUSE_CNTRL_END_BIT) &&
			(Bit < XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_START_BIT)) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		}

		if(Bit > XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_END_BIT) {
			ErrorCode = XSK_EFUSEPL_ERROR_READ_BIT_OUT_OF_RANGE;
			return XST_FAILURE;
		}
	}

	if(XilSKey_EfusePl_ReadRow(Row, MarginOption,RowData) != XST_SUCCESS) {
		return XST_FAILURE;
	}

	*BitData = RowData[Bit];

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
*	Reads row data of a specified row
*
*
* @param	Row				- 	row number
* @param	MarginOption	-	Margin Option(One of the reading method of PLeFUSE)
* @param	RowDataBytes	-	To store the read data bytes of specified row.
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Read data will be stored @row_data_bytes
*
*****************************************************************************/

u8 XilSKey_EfusePl_ReadRow(u32 Row, u8 MarginOption, u8 *RowDataBytes)
{
	XSKEfusePs_XAdc PL_XAdc;
	u32 RowDataBits=0;

	/**
	 * Check if the row position is valid.
	 */
	if((Row > XSK_EFUSEPL_ARRAY_USER_DATA_START_ROW) ||
		((Row > XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW) &&
		(Row < XSK_EFUSEPL_ARRAY_AES_DATA_ROW_START))) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_ROW_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	/**
	 * Check the read margin option.
	 */
	if( (MarginOption != XSK_EFUSEPL_READ_NORMAL ) &&
		(MarginOption != XSK_EFUSEPL_READ_MARGIN_1)&&
		(MarginOption != XSK_EFUSEPL_READ_MARGIN_2))
	{
		ErrorCode = XSK_EFUSEPL_ERROR_READ_MARGIN_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	/**
	 * check if row_data is not NULL
	 */
	if(NULL == RowDataBytes)
	{
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BUFFER_NULL;
		return XST_FAILURE;
	}


	/**
	 * Monitor the Voltage and temperature using XADC, if out of range return
	 * unique error.
	 */
	PL_XAdc.VType = XSK_EFUSEPS_VAUX;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.Temp < XSK_EFUSEPL_READ_TEMP_MIN_RAW) ||
		(PL_XAdc.Temp > XSK_EFUSEPL_READ_TEMP_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_TMEPERATURE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	if((PL_XAdc.V < XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MIN_RAW) ||
		(PL_XAdc.V > XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_VCCAUX_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	PL_XAdc.VType = XSK_EFUSEPS_VINT;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.V < XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MIN_RAW) ||
		(PL_XAdc.V > XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_VCCINT_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	/**
	 * Here we have to use the impact algorithm to read the eFUSE row.
	 * and return the data in row_data.
	 */

	if((MarginOption & XSK_EFUSEPL_READ_NORMAL) == XSK_EFUSEPL_READ_NORMAL) {

		JtagRead(Row, (unsigned int *)&RowDataBits, 0);
	}
	else if((MarginOption & XSK_EFUSEPL_READ_MARGIN_1) ==
							XSK_EFUSEPL_READ_MARGIN_1) {
		JtagRead(Row, (unsigned int *)&RowDataBits, 1);
	}
	else if((MarginOption & XSK_EFUSEPL_READ_MARGIN_2) ==
							XSK_EFUSEPL_READ_MARGIN_2) {
		JtagRead(Row, (unsigned int *)&RowDataBits, 2);
	}

	XilSKey_Efuse_ConvertBitsToBytes((u8 *)&RowDataBits, RowDataBytes, 32);

	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
*	Reads PL eFUSE Control Register data
*
*
* @param	CtrlData	-	Place holder to store the read data (control register)
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code (if any)
*
*****************************************************************************/

u8 XilSKey_EfusePl_ReadControlRegister(u8 *CtrlData)
{
	u8 RowData[XSK_EFUSEPL_ARRAY_MAX_COL]={0};
	u32 Index=0;

	/**
	 * check if cntrl_data is not NULL
	 */
	if(NULL == CtrlData) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BUFFER_NULL;
		return XST_FAILURE;
	}

	if(XilSKey_EfusePl_ReadRow(XSK_EFUSEPL_ARRAY_FUSE_CNTRL_ROW,
								 	 XSK_EFUSEPL_READ_NORMAL,
								 	 RowData) != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	for(Index=0; Index < XSK_EFUSEPL_ARRAY_FUSE_CNTRL_MAX_BITS; Index++)
	{
		CtrlData[Index] = RowData[Index] |
						  RowData[Index +
							  XSK_EFUSEPL_ARRAY_FUSE_CNTRL_REDUNDENT_START_BIT];
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
*
*	Verify the PL eFUSE bit blown by reading it back
*
*
* @param	Row			 - row number
* @param	Bit			 - bit position of the specified row
* @param	MarginOption - Margin Option(One of the reading method of PLeFUSE)
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
u8 XilSKey_EfusePl_VerifyBit(u8 Row, u8 Bit, u8 MarginOption)
{
	u8 BitData = 0;
	if(XilSKey_EfusePl_ReadBit(Row, Bit, MarginOption, &BitData)
									!= XST_SUCCESS)	{
		return XST_FAILURE;
	}

	if(BitData == FALSE) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_BIT_VALUE_NOT_SET;
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
*	Calculates and stores the ECC data of row data
*
*
* @param	RowData	- Pointer to row data on which ECC will be calculated
* @param	ECCData	- Pointer to store the calculated ECC
*
* @return
*
*	None
*
* @note		None.
*
*****************************************************************************/
void XilSKey_EfusePl_CalculateEcc(u8 *RowData, u8 *ECCData)
{
	ECCData[0] = 	RowData[0] ^ RowData[1] ^ RowData[2] ^
					RowData[3] ^ RowData[8] ^ RowData[9] ^
					RowData[10] ^ RowData[11] ^ RowData[12] ^
					RowData[13] ^ RowData[14] ^ RowData[15] ^
					RowData[16] ^ RowData[17];

	ECCData[1] = 	RowData[1] ^ RowData[2] ^ RowData[3] ^
					RowData[5] ^ RowData[6] ^ RowData[7] ^
					RowData[11] ^ RowData[12] ^ RowData[13] ^
					RowData[14] ^ RowData[18] ^ RowData[19] ^
					RowData[20];

	ECCData[2] = 	RowData[0] ^ RowData[2] ^ RowData[3] ^
					RowData[4] ^ RowData[6] ^ RowData[7] ^
					RowData[9] ^ RowData[10] ^ RowData[13] ^
					RowData[15] ^ RowData[18] ^ RowData[21] ^
					RowData[22];

	ECCData[3] = 	RowData[0] ^ RowData[1] ^ RowData[3] ^
					RowData[4] ^ RowData[5] ^ RowData[7] ^
					RowData[8] ^ RowData[10] ^ RowData[12] ^
					RowData[16] ^ RowData[19] ^ RowData[21] ^
					RowData[23];

	ECCData[4] = 	RowData[0] ^ RowData[1] ^ RowData[2] ^
					RowData[4] ^ RowData[5] ^ RowData[6] ^
					RowData[8] ^ RowData[9] ^ RowData[11] ^
					RowData[17] ^ RowData[20] ^ RowData[22] ^
					RowData[23];

	/**
	 * This is the DED data
	 */

	ECCData[5] = 	RowData[0] ^ RowData[1] ^ RowData[2] ^
					RowData[3] ^ RowData[4] ^ RowData[5] ^
					RowData[6] ^ RowData[7] ^ RowData[8] ^
					RowData[9] ^ RowData[10] ^ RowData[11] ^
					RowData[12] ^ RowData[13] ^ RowData[14] ^
					RowData[15] ^ RowData[16] ^ RowData[17] ^
					RowData[18] ^ RowData[19] ^ RowData[20] ^
					RowData[21] ^ RowData[22] ^ RowData[23] ^
					ECCData[0] ^ ECCData[1] ^ ECCData[2] ^
					ECCData[3] ^ ECCData[4];
}

/****************************************************************************/
/**
*
*
*	Reads the PL efuse status bits
*
*
* @param	InstancePtr - Input data to be written to PL eFUSE
* @param	StatusBits - Variable to store the status bits read.
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
u32 XilSKey_EfusePl_ReadStatus(XilSKey_EPl *InstancePtr, u32 *StatusBits)
{
	u32 RefClk;
	u32 ArmPllFdiv;
	u32 ArmClkDivisor;
	unsigned int RowData;
	u32 Status;
	XSKEfusePs_XAdc PL_XAdc;

	if(NULL == InstancePtr)	{
		return XSK_EFUSEPL_ERROR_PL_STRUCT_NULL;
	}

	if(!(InstancePtr->SystemInitDone))
	{

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

		/**
		 * Return error if the reference clock frequency is not in
		 * between 20 & 60MHz
		 */
		if((RefClk < XSK_EFUSEPL_MIN_REF_CLK_FREQ) ||
				(RefClk > XSK_EFUSEPL_MAX_REF_CLK_FREQ)) {
			return XSK_EFUSEPL_ERROR_INVALID_REF_CLK;
		}

		/**
		 * Initialize the timer, XADC and jtag server
		 */

		XilSKey_Efuse_StartTimer(RefClk);

		Status = XilSKey_EfusePs_XAdcInit();
		if(Status != XST_SUCCESS) {
			ErrorCode = Status;
			return (XSK_EFUSEPL_ERROR_XADC + ErrorCode);
		}

		if(JtagServerInit(InstancePtr) != XST_SUCCESS) {
			return XSK_EFUSEPL_ERROR_JTAG_SERVER_INIT;
		}

		InstancePtr->SystemInitDone = 1;

	}

	/**
	 * Monitor the Voltage and temperature using XADC, if out of range return
	 * unique error.
	 */
	PL_XAdc.VType = XSK_EFUSEPS_VAUX;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.Temp < XSK_EFUSEPL_READ_TEMP_MIN_RAW) ||
		(PL_XAdc.Temp > XSK_EFUSEPL_READ_TEMP_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_TMEPERATURE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	if((PL_XAdc.V < XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MIN_RAW) ||
		(PL_XAdc.V > XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_VCCAUX_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}
	PL_XAdc.VType = XSK_EFUSEPS_VINT;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.V < XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MIN_RAW) ||
		(PL_XAdc.V > XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_VCCINT_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	/*
	 * Read row 0 for status bits
	 */
	JtagRead(0, &RowData, 0);

	*StatusBits = RowData & 0xFFFFFF;

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
*
*	Reads the PL efuse key (AES and user)
*
*
* @param	InstancePtr - Input data to be written to PL eFUSE
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note		Updates the global variable ErrorCode with error code(if any).
*
*****************************************************************************/
u32 XilSKey_EfusePl_ReadKey(XilSKey_EPl *InstancePtr)
{
	u32 RefClk;
	u32 ArmPllFdiv;
	u32 ArmClkDivisor;
	u32 RowCount;
	unsigned int RowData;
	u32 KeyCnt;
	u32 Status;
	XSKEfusePs_XAdc PL_XAdc;

	if(NULL == InstancePtr)	{
		return XSK_EFUSEPL_ERROR_PL_STRUCT_NULL;
	}

	if(!(InstancePtr->SystemInitDone))
	{

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

		/**
		 * Return error if the reference clock frequency is not in
		 * between 20 & 60MHz
		 */
		if((RefClk < XSK_EFUSEPL_MIN_REF_CLK_FREQ) ||
				(RefClk > XSK_EFUSEPL_MAX_REF_CLK_FREQ)) {
			return XSK_EFUSEPL_ERROR_INVALID_REF_CLK;
		}

		/**
		 * Initialize the timer and jtag server
		 */

		XilSKey_Efuse_StartTimer(RefClk);

		Status = XilSKey_EfusePs_XAdcInit();
		if(Status != XST_SUCCESS) {
			ErrorCode = Status;
			return (XSK_EFUSEPL_ERROR_XADC + ErrorCode);
		}

		if(JtagServerInit(InstancePtr) != XST_SUCCESS) {
			return XSK_EFUSEPL_ERROR_JTAG_SERVER_INIT;
		}

		InstancePtr->SystemInitDone = 1;

	}

	/**
	 * Monitor the Voltage and temperature using XADC, if out of range return
	 * unique error.
	 */
	PL_XAdc.VType = XSK_EFUSEPS_VAUX;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.Temp < XSK_EFUSEPL_READ_TEMP_MIN_RAW) ||
		(PL_XAdc.Temp > XSK_EFUSEPL_READ_TEMP_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_TMEPERATURE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	if((PL_XAdc.V < XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MIN_RAW) ||
		(PL_XAdc.V > XSK_EFUSEPL_READ_VOLTAGE_VCCAUX_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_VCCAUX_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	PL_XAdc.VType = XSK_EFUSEPS_VINT;
	XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(&PL_XAdc);
	if((PL_XAdc.V < XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MIN_RAW) ||
		(PL_XAdc.V > XSK_EFUSEPL_READ_VOLTAGE_VCCINT_MAX_RAW)) {
		ErrorCode = XSK_EFUSEPL_ERROR_READ_VCCINT_VOLTAGE_OUT_OF_RANGE;
		return XST_FAILURE;
	}

	/*
	 * Read AES key and User Key and
	 * store them in the variables in instance structure
	 */

	/*
	 * AES key 4 bytes
	 */
	KeyCnt = 0;

	/*
	 * Read row 20 to 29
	 */
	for(RowCount = 20; RowCount <= 29; RowCount++)
	{
		JtagRead(RowCount, &RowData, 0);
		InstancePtr->AESKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);
		RowData = RowData >> 8;
		InstancePtr->AESKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);
		RowData = RowData >> 8;
		InstancePtr->AESKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);
	}

	/*
	 * Read row 30
	 */
	JtagRead(30, &RowData, 0);
	InstancePtr->AESKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);
	RowData = RowData >> 8;
	InstancePtr->AESKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);

	/*
	 * User key 4 bytes
	 */
	KeyCnt = 0;
	RowData = RowData >> 8;
	InstancePtr->UserKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);

	/*
	 * Read row 31
	 */
	JtagRead(31, &RowData, 0);
	InstancePtr->UserKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);
	RowData = RowData >> 8;
	InstancePtr->UserKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);
	RowData = RowData >> 8;
	InstancePtr->UserKeyReadback[KeyCnt++] = (u8)(RowData & 0xFF);

	return XST_SUCCESS;

}