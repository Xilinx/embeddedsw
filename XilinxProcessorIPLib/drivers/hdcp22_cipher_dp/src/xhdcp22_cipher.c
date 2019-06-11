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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xhdcp22_cipher.c
* @addtogroup hdcp22_cipher_v1_1
* @{
* @details
*
* This file contains the main implementation of the Xilinx HDCP 2.2 Cipher
* device driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  JB     02/19/19 Initial Release.
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xhdcp22_cipher.h"
#include "string.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* This function initializes the HDCP22 Cipher core. This function must be called
* prior to using the HDCP22 Cipher core. Initialization of the HDCP22 Cipher includes
* setting up the instance data, and ensuring the hardware is in a quiescent
* state.
*
* @param  InstancePtr is a pointer to the XHdcp22_Cipher core instance.
* @param  CfgPtr points to the configuration structure associated with
*         the HDCP22 Cipher core core.
* @param  EffectiveAddr is the base address of the device. If address
*         translation is being used, then this parameter must reflect the
*         virtual base address. Otherwise, the physical address should be
*         used.
*
* @return
*   - XST_SUCCESS if XHdcp22Cipher_CfgInitialize was successful.
*   - XST_FAILURE if HDCP22 Cipher ID mismatched.
*
* @note		None.
*
******************************************************************************/
int XHdcp22Cipher_CfgInitialize(XHdcp22_Cipher *InstancePtr,
                                XHdcp22_Cipher_Config *CfgPtr,
                                UINTPTR EffectiveAddr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)NULL);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XHdcp22_Cipher));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr, sizeof(XHdcp22_Cipher_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Check ID */
	RegValue = XHdcp22Cipher_ReadReg(InstancePtr->Config.BaseAddress, (XHDCP22_CIPHER_VER_ID_OFFSET));
	RegValue = ((RegValue) >> (XHDCP22_CIPHER_SHIFT_16)) & (XHDCP22_CIPHER_MASK_16);
	if (RegValue != (XHDCP22_CIPHER_VER_ID)) {
		return (XST_FAILURE);
	}

	/* Reset the hardware and set the flag to indicate the driver is ready */
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function sets the Ks key in the HDCP22 Cipher core.
*
* @param  BaseAddress is the base address of the HDCP22 Cipher core instance.
* @param  KsPtr is a pointer to the Ks key.
* @param  Length indicates the number of bytes.
*
* @return None.
*
* @note   The Ks should be in big endian notation.
*
******************************************************************************/
void XHdcp22Cipher_SetKs(XHdcp22_Cipher *InstancePtr, const u8 *KsPtr, u16 Length)
{
	Xil_AssertVoid(KsPtr != NULL);
	Xil_AssertVoid(Length == 16);

	u32 i = 0;
	u32* U32Ptr = NULL;
	u32 Offset = XHDCP22_CIPHER_REG_KS_1_OFFSET;


	u8 Buf[16];
	/* The HDCP cipher expects the Ks to be written in little endian format */
	/* Swap the endianess */
	for (i=0; i<16; i++){
		Buf[(15-i)] = KsPtr[i];
	}

	/* KsPtr contains 16 bytes (128 bits). Map to 4 u32 values for writing */
	for (i=0; i<Length; i+=4)
	{
		U32Ptr = (u32 *)&Buf[i];
		XHdcp22Cipher_WriteReg(InstancePtr->Config.BaseAddress, Offset, *U32Ptr);
		/* Increase offset to the next register */
		Offset+=4;
	}
}

/*****************************************************************************/
/**
* This function sets the Lc128 key in the HDCP22 Cipher core.
*
* @param  BaseAddress is the base address of the HDCP22 Cipher core instance.
* @param  Lc128Ptr is a pointer to the LC128 key.
* @param  Length indicates the number of bytes.
*
* @return None.
*
* @note   The Lc128 should be in big endian notation.
*
******************************************************************************/
void XHdcp22Cipher_SetLc128(XHdcp22_Cipher *InstancePtr, const u8 *Lc128Ptr,  u16 Length)
{
	Xil_AssertVoid(Lc128Ptr != NULL);
	Xil_AssertVoid(Length == 16);

	u32 i = 0;
	u32* U32Ptr = NULL;
	u32 Offset = XHDCP22_CIPHER_REG_LC128_1_OFFSET;

	u8 Buf[16];
	/* The HDCP cipher expects the Ks to be written in little endian format */
	/* Swap the endianess */
	for (i=0; i<16; i++){
		Buf[(15-i)] = Lc128Ptr[i];
	}

	/* Lc128Ptr contains 16 bytes (128 bits). Map to 4 u32 values for writing */
	for (i=0; i<Length; i+=4)
	{
		U32Ptr = (u32 *)&Buf[i];
		XHdcp22Cipher_WriteReg(InstancePtr->Config.BaseAddress, Offset, *U32Ptr);
		/* Increase offset to the next register */
		Offset+=4;
	}
}

/*****************************************************************************/
/**
* This function sets the Riv key in the HDCP22 Cipher core.
*
* @param  BaseAddress is the base address of the HDCP22 Cipher core instance.
* @param  RivPtr is a pointer to the Riv key.
* @param  Length indicates the number of bytes.
*
* @return None.
*
* @note   The Riv should be in big endian notation.
*
******************************************************************************/
void XHdcp22Cipher_SetRiv(XHdcp22_Cipher *InstancePtr, const u8 *RivPtr,  u16 Length)
{
	Xil_AssertVoid(RivPtr != NULL);
	Xil_AssertVoid(Length == 8);

	u32 i = 0;
	u32* U32Ptr = NULL;
	u32 Offset = XHDCP22_CIPHER_REG_RIV_1_OFFSET;

	u8 Buf[8];
	/* The HDCP cipher expects the Riv to be written in little endian format */
	/* Swap the endianess */
	for (i=0; i<8; i++)
	{
		Buf[(7-i)] = RivPtr[i];
	}

	/* RivPtr contains 8 bytes (64 bits). Map to 2 u32 values for writing */
	for (i=0; i<Length; i+=4)
	{
		U32Ptr = (u32 *)&Buf[i];
		XHdcp22Cipher_WriteReg(InstancePtr->Config.BaseAddress, Offset, *U32Ptr);
		/* Increase offset to the next register */
		Offset+=4;
	}
}

/*****************************************************************************/
/**
 * This function sets the Lane count in the HDCP22 Cipher core.
 *
 * @param  InstancePtr is the HDCP22 Cipher core instance.
 * @param  LaneCount is the number lanes to be set in cipher core.
 *
 * @return None.
 *
 ******************************************************************************/
void XHdcp22Cipher_SetLanecount(XHdcp22_Cipher *InstancePtr, u8 LaneCount)
{
	Xil_AssertVoid(InstancePtr != NULL);
	u32 value = 0;

	value = XHdcp22Cipher_ReadReg(InstancePtr->Config.BaseAddress,
			XHDCP22_CIPHER_REG_CTRL_SET_OFFSET);
	value &= ~XHDCP22_CIPHER_REG_CTRL_LANE_CNT_MASK;
	value |= (LaneCount << XHDCP22_CIPHER_REG_CTRL_LANE_CNT_BIT_POS);
	XHdcp22Cipher_WriteReg(InstancePtr->Config.BaseAddress,
			XHDCP22_CIPHER_REG_CTRL_SET_OFFSET, value);
}

/** @} */
