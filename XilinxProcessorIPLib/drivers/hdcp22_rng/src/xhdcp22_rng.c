/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* @file xhdcp22_rng.c
* @addtogroup hdcp22_rng_v1_2
* @{
* @details
*
* This file contains the main implementation of the Xilinx HDCP 2.2 RNG
* device driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  JO     10/01/15 Initial release.
* 1.01  MH     08/04/16 Added 64 bit address support.
* 1.02  MH     02/17/16 Fixed pointer alignment problem in function
*                       XHdcp22Rng_GetRandom
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xhdcp22_rng.h"
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
* This function initializes the HDCP22 Rng core. This function must be called
* prior to using the HDCP22 Rng core. Initialization of the HDCP22 Rng includes
* setting up the instance data, and ensuring the hardware is in a quiescent
* state.
*
* @param  InstancePtr is a pointer to the XHdcp22_Rng core instance.
* @param  CfgPtr points to the configuration structure associated with
*         the HDCP22 Rng core core.
* @param  EffectiveAddr is the base address of the device. If address
*         translation is being used, then this parameter must reflect the
*         virtual base address. Otherwise, the physical address should be
*         used.
*
* @return
*   - XST_SUCCESS if XHdcp22Rng_CfgInitialize was successful.
*   - XST_FAILURE if HDCP22 Rng ID mismatched.
*
* @note		None.
*
******************************************************************************/
int XHdcp22Rng_CfgInitialize(XHdcp22_Rng *InstancePtr,
                                XHdcp22_Rng_Config *CfgPtr,
                                UINTPTR EffectiveAddr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)NULL);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XHdcp22_Rng));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr, sizeof(XHdcp22_Rng_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Check ID */
	RegValue = XHdcp22Rng_ReadReg(InstancePtr->Config.BaseAddress, (XHDCP22_RNG_VER_ID_OFFSET));
	RegValue = ((RegValue) >> (XHDCP22_RNG_SHIFT_16)) & (XHDCP22_RNG_MASK_16);
	if (RegValue != (XHDCP22_RNG_VER_ID)) {
		return (XST_FAILURE);
	}

	/* Reset the hardware and set the flag to indicate the driver is ready */
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function returns a random number.
*
* @param  InstancePtr is a pointer to the XHdcp22_Rng core instance.
* @param  BufferPtr points to the buffer that will contain a random number.
* @param  BufferLength is the length of the BufferPtr in bytes.
*         The length must be greater than or equal to RandomLength.
* @param  RandomLength is the requested length of the random number in bytes.
*         The length must be a multiple of 4
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XHdcp22Rng_GetRandom(XHdcp22_Rng *InstancePtr, u8 *BufferPtr, u16 BufferLength, u16 RandomLength)
{
	u32 i, j;
	u32 Offset = 0;
	u32 RandomWord;
	u8 *RandomPtr = (u8 *)&RandomWord;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(BufferPtr != NULL);
	Xil_AssertVoid(RandomLength%4 == 0);
	Xil_AssertVoid(BufferLength >= RandomLength);

	for (i=0; i<RandomLength; i+=4)
	{
		RandomWord = XHdcp22Rng_ReadReg(InstancePtr->Config.BaseAddress,
					XHDCP22_RNG_REG_RN_1_OFFSET + Offset);
		for (j=0; j<4; j++) {
			BufferPtr[i + j] = RandomPtr[j];
		}

		/* Increase offset to the next register and wrap after the last register
		   (RNG length is 16 bytes) */
		Offset = (Offset+4) % 16;
	}
}

/** @} */
