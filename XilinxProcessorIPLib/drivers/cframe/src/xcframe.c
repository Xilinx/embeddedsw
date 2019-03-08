/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF PLRCHANTABILITY,
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEPLNT. IN NO EVENT SHALL
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
* @file xcframe.c
*
* This file which contains the code related to CFRAME block.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   11/10/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xcframe.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
*
* This function initializes an CFRAME core. This function must be called
* prior to using an CFRAME driver.
*
* @param	InstancePtr is a pointer to the XCframe instance.
* @param	CfgPtr is a reference to a structure containing information
*		about a specific XCframe instance.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. The caller is responsible for keeping the
*		address mapping from EffectiveAddr to the device physical
*		base address unchanged once this function is invoked.
*		Unexpected errors may occur if the address mapping changes
*		after this function is called. If address translation is not
*		used, pass in the physical address instead.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*
* @note		None.
*
******************************************************************************/
s32 XCframe_CfgInitialize(XCframe *InstancePtr, XCframe_Config *CfgPtr,
			u32 EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != ((u32)0x0));

	/* Setup the instance */
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
						sizeof(XCframe_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
 * This function writes the 128 bit CFRAME register
 *
 * @param	InstancePtr is a pointer to the XCframe instance.
 * @param	Addr   CFRAME register address
 * @param	Value128 128 bit value to be stored
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_WriteReg(XCframe *InstancePtr, u32 AddrOffset,
		XCframe_FrameNo FrameNo, Xuint128 *Val)
{
	/* TODO check if we need to disable interrupts */
	XCframe_WriteReg32(InstancePtr->Config.BaseAddress +
			(FrameNo*XCFRAME_FRAME_OFFSET),
			AddrOffset, Val->Word0);
	XCframe_WriteReg32(InstancePtr->Config.BaseAddress +
			(FrameNo*XCFRAME_FRAME_OFFSET) ,
			AddrOffset+4, Val->Word1);
	XCframe_WriteReg32(InstancePtr->Config.BaseAddress +
			(FrameNo*XCFRAME_FRAME_OFFSET),
			AddrOffset+8, Val->Word2);
	XCframe_WriteReg32(InstancePtr->Config.BaseAddress +
			(FrameNo*XCFRAME_FRAME_OFFSET),
			AddrOffset+12, Val->Word3);
}

/*****************************************************************************/
/**
 * This function writes the value to CFRAME cmd register
 *
 * @param	InstancePtr is a pointer to the XCframe instance.
 * @param	Cmd to be initiated by CFRAME block
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_WriteCmd(XCframe *InstancePtr,	XCframe_FrameNo CframeNo, u32 Cmd)
{
	Xuint128 CfrCmd={0};

	XCframe_Printf("CFRAME CMD: %0x\n\r", Cmd);

	CfrCmd.Word0 = Cmd;
	XCframe_WriteReg(InstancePtr, XCFRAME_CMD_OFFSET, CframeNo, &CfrCmd);
}

/*****************************************************************************/
/**
 * This function writes the VGG TRIM value
 *
 * @param	InstancePtr is a pointer to the XCframe instance.
 * @param	Value Trim value to be applied for
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_VggTrim(XCframe *InstancePtr,	Xuint128 *TrimVal)
{
	XCframe_WriteReg(InstancePtr, XCFRAME_VGG_TRIM_OFFSET,
			XCFRAME_FRAME_BCAST, TrimVal);
}

/*****************************************************************************/
/**
 * This function writes the BRAM TRIM value
 *
 * @param	InstancePtr is a pointer to the XCframe instance.
 * @param	Value Trim value to be applied for
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_CramTrim(XCframe *InstancePtr,	u32 TrimValue)
{
	Xuint128 TrimVal={0};

	TrimVal.Word0 = TrimValue;
	XCframe_WriteReg(InstancePtr, XCFRAME_CRAM_TRIM_OFFSET,
			XCFRAME_FRAME_BCAST, &TrimVal);
}

/*****************************************************************************/
/**
 * This function writes the BRAM TRIM value
 *
 * @param	InstancePtr is a pointer to the XCframe instance.
 * @param	Value Trim value to be applied for
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_BramTrim(XCframe *InstancePtr, u32 TrimValue)
{
	Xuint128 TrimVal={0};

	TrimVal.Word0 = TrimValue;
	XCframe_WriteReg(InstancePtr, XCFRAME_COE_TRIM_OFFSET,
			XCFRAME_FRAME_BCAST, &TrimVal);
	XCframe_WriteCmd(InstancePtr, XCFRAME_FRAME_BCAST,
			XCFRAME_CMD_REG_TRIMB);
}

/*****************************************************************************/
/**
 * This function writes the BRAM TRIM value
 *
 * @param	InstancePtr is a pointer to the XCframe instance.
 * @param	Value Trim value to be applied
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_UramTrim(XCframe *InstancePtr, u32 TrimValue)
{
	Xuint128 TrimVal={0};

	TrimVal.Word0 = TrimValue;
	XCframe_WriteReg(InstancePtr, XCFRAME_COE_TRIM_OFFSET,
			XCFRAME_FRAME_BCAST, &TrimVal);
	XCframe_WriteCmd(InstancePtr, XCFRAME_FRAME_BCAST,
				XCFRAME_CMD_REG_TRIMU);
}

/*****************************************************************************/
/**
 * This function sets the CFRAME read parameters with mentioned CFRAME length and
 * frame number
 *
 * @param
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_SetReadParam(XCframe *InstancePtr,
		XCframe_FrameNo CframeNo, u32 CframeLen)
{
	Xuint128 Value128={0};

	/* Enable ROWON, READ_CFR, CFRM_CNT */
	XCframe_WriteCmd(InstancePtr, CframeNo,	XCFRAME_CMD_REG_ROWON);
	XCframe_WriteCmd(InstancePtr, CframeNo,	XCFRAME_CMD_REG_RCFG);
	XCframe_WriteReg(InstancePtr, XCFRAME_FAR_OFFSET, CframeNo, &Value128);

	Value128.Word0=CframeLen/4;
	XCframe_WriteReg(InstancePtr, XCFRAME_FRCNT_OFFSET, CframeNo, &Value128);
}
