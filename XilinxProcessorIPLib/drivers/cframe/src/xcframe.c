/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 1.01  bsv  29/05/2019 XCframe_ReadReg API added
* 1.02  bsv  11/06/2019 XCframe_ClearCframeErr API added
* 1.03  bsv  17/02/2020 XCframe_SafetyWriteReg API added
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
 * This function writes to 128 bit CFRAME register
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
 * This function reads the 128 bit CFRAME register
 *
 * @param       InstancePtr is a pointer to the XCframe instance.
 * @param       Addr      CFRAME register address
 * @param       ValPtr    128 bit variable to store the read data
 *
 * @return      None
 *
 ******************************************************************************/
void XCframe_ReadReg(XCframe *InstancePtr, u32 AddrOffset,
                XCframe_FrameNo FrameNo, u32* ValPtr)
{
        ValPtr[0] = XCframe_ReadReg32(InstancePtr->Config.BaseAddress +
                        (FrameNo*XCFRAME_FRAME_OFFSET), AddrOffset);
        ValPtr[1] = XCframe_ReadReg32(InstancePtr->Config.BaseAddress +
                        (FrameNo*XCFRAME_FRAME_OFFSET), AddrOffset+4);
        ValPtr[2] = XCframe_ReadReg32(InstancePtr->Config.BaseAddress +
                        (FrameNo*XCFRAME_FRAME_OFFSET), AddrOffset+8);
        ValPtr[3] = XCframe_ReadReg32(InstancePtr->Config.BaseAddress +
                        (FrameNo*XCFRAME_FRAME_OFFSET), AddrOffset+12);
}

/*****************************************************************************/
/**
 * @brief This function writes to 128 bit CFRAME register and reads it back to
 * 			validate the write operation.
 *
 * @param	InstancePtr is a pointer to the XCframe instance.
 * @param	Addr   CFRAME register address
 * @param	Value128 128 bit value to be stored
 *
 * @return	Success or Failure
 *
 ******************************************************************************/
int XCframe_SafetyWriteReg(XCframe *InstancePtr, u32 AddrOffset,
		XCframe_FrameNo FrameNo, Xuint128 *Val)
{
	int Status = XST_FAILURE;
	u32 ReadVal[4U];

	XCframe_WriteReg(InstancePtr, AddrOffset, FrameNo, Val);
	XCframe_ReadReg(InstancePtr, AddrOffset, FrameNo, ReadVal);
	if ((ReadVal[0U] == Val->Word0) && (ReadVal[1U] == Val->Word1) &&
		(ReadVal[2U] == Val->Word2) && (ReadVal[3U] == Val->Word3)) {
		Status = XST_SUCCESS;
	}
	return Status;
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
	Xuint128 MaskVal={0};

        MaskVal.Word0 = 0xFFFFFFFF;
        MaskVal.Word1 = 0xFFFFFFFF;
        MaskVal.Word2 = 0xFFFFFFFF;
        XCframe_WriteReg(InstancePtr, XCFRAME_MASK_OFFSET,
                        XCFRAME_FRAME_BCAST, &MaskVal);

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
	Xuint128 MaskVal={0};

	MaskVal.Word0 = 0xFFFFFFFF;
	XCframe_WriteReg(InstancePtr, XCFRAME_MASK_OFFSET,
			XCFRAME_FRAME_BCAST, &MaskVal);

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
	Xuint128 MaskVal={0};

	MaskVal.Word0 = 0xFFFFFFFF;
	MaskVal.Word1 = 0xFFFFFFFF;
	XCframe_WriteReg(InstancePtr, XCFRAME_MASK_OFFSET,
			XCFRAME_FRAME_BCAST, &MaskVal);

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
	Xuint128 MaskVal={0};

        MaskVal.Word0 = 0xFFFFFFFF;
        MaskVal.Word1 = 0xFFFFFFFF;
        XCframe_WriteReg(InstancePtr, XCFRAME_MASK_OFFSET,
                        XCFRAME_FRAME_BCAST, &MaskVal);

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

/*****************************************************************************/
/**
 * This function clears CFRAME ISRs and is called as part of CFRAME error recovery
 *
 * @param XCframe Instance Pointer
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_ClearCframeErr(XCframe *InstancePtr)
{
	Xuint128 Value128={0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU};
	Xil_AssertVoid(InstancePtr != NULL);

	XCframe_FrameNo CframeNo = XCFRAME_FRAME_0;
	/* Clear CFRAME ISRs */
	while(CframeNo <= XCFRAME_FRAME_BCAST)
	{
		XCframe_WriteReg(InstancePtr, XCFRAME_CFRM_ISR_OFFSET, CframeNo++,
															&Value128);
	}
}
