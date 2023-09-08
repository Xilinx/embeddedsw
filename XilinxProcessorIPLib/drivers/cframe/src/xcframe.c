/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
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
* 1.01  bsv  05/29/2019 XCframe_ReadReg API added
* 1.02  bsv  11/06/2019 XCframe_ClearCframeErr API added
* 1.03  bsv  02/17/2020 XCframe_SafetyWriteReg API added
* 1.04  bsv  07/15/2021 Fix doxygen warnings
* 1.5   mss  09/04/2023 Fixed MISRA-C violation 10.1
*       mss  09/04/2023 Fixed MISRA-C violation 10.4
*       mss  09/04/2023 Fixed MISRA-C violation 7.2
*       mss  09/04/2023 Fixed MISRA-C violation 4.6
*       mss  09/04/2023 Fixed MISRA-C violation 8.13
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
s32 XCframe_CfgInitialize(XCframe *InstancePtr,const XCframe_Config *CfgPtr,
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
 * @param	AddrOffset is offset of the Cframe register
 * @param	FrameNo is the index of frame
 * @param       Val is pointer to the value to be written
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_WriteReg(const XCframe *InstancePtr, u32 AddrOffset,
		XCframe_FrameNo FrameNo,const Xuint128 *Val)
{
	/* TODO check if we need to disable interrupts */
	XCframe_WriteReg32(InstancePtr->Config.BaseAddress +
			((u32)FrameNo*XCFRAME_FRAME_OFFSET),
			AddrOffset, Val->Word0);
	XCframe_WriteReg32(InstancePtr->Config.BaseAddress +
			((u32)FrameNo*XCFRAME_FRAME_OFFSET) ,
			AddrOffset+4U, Val->Word1);
	XCframe_WriteReg32(InstancePtr->Config.BaseAddress +
			((u32)FrameNo*XCFRAME_FRAME_OFFSET),
			AddrOffset+8U, Val->Word2);
	XCframe_WriteReg32(InstancePtr->Config.BaseAddress +
			((u32)FrameNo*XCFRAME_FRAME_OFFSET),
			AddrOffset+12U, Val->Word3);
}

/*****************************************************************************/
/**
 * This function reads the 128 bit CFRAME register
 *
 * @param       InstancePtr is a pointer to the XCframe instance.
 * @param	AddrOffset is offset of the Cframe register
 * @param	FrameNo is the index of frame
 * @param       ValPtr    128 bit variable to store the read data
 *
 * @return      None
 *
 ******************************************************************************/
void XCframe_ReadReg(const XCframe *InstancePtr, u32 AddrOffset,
                XCframe_FrameNo FrameNo, u32* ValPtr)
{
        ValPtr[0] = XCframe_ReadReg32(InstancePtr->Config.BaseAddress +
                        ((u32)FrameNo*XCFRAME_FRAME_OFFSET), AddrOffset);
        ValPtr[1] = XCframe_ReadReg32(InstancePtr->Config.BaseAddress +
                        ((u32)FrameNo*XCFRAME_FRAME_OFFSET), AddrOffset+4U);
        ValPtr[2] = XCframe_ReadReg32(InstancePtr->Config.BaseAddress +
                        ((u32)FrameNo*XCFRAME_FRAME_OFFSET), AddrOffset+8U);
        ValPtr[3] = XCframe_ReadReg32(InstancePtr->Config.BaseAddress +
                        ((u32)FrameNo*XCFRAME_FRAME_OFFSET), AddrOffset+12U);
}

/*****************************************************************************/
/**
 * @brief This function writes to 128 bit CFRAME register and reads it back to
 * 			validate the write operation.
 *
 * @param	InstancePtr is a pointer to the XCframe instance.
 * @param	AddrOffset is offset of the Cframe register
 * @param	FrameNo is the index of frame
 * @param       Val is pointer to the value to be written
 *
 * @return	Success or Failure
 *
 ******************************************************************************/
s32 XCframe_SafetyWriteReg(const XCframe *InstancePtr, u32 AddrOffset,
		XCframe_FrameNo FrameNo,const Xuint128 *Val)
{
	s32 Status = XST_FAILURE;
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
 * @param	CframeNo is the index of frame
 * @param	Cmd to be initiated by CFRAME block
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_WriteCmd(const XCframe *InstancePtr, XCframe_FrameNo CframeNo, u32 Cmd)
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
 * @param	TrimVal is pointer to the TRIM value
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_VggTrim(const XCframe *InstancePtr,const Xuint128 *TrimVal)
{
	Xuint128 MaskVal={0};

        MaskVal.Word0 = XCFRAME_MASK_DEFVAL;
        MaskVal.Word1 = XCFRAME_MASK_DEFVAL;
        MaskVal.Word2 = XCFRAME_MASK_DEFVAL;
        XCframe_WriteReg(InstancePtr, XCFRAME_MASK_OFFSET,
                        XCFRAME_FRAME_BCAST, &MaskVal);

	XCframe_WriteReg(InstancePtr, XCFRAME_VGG_TRIM_OFFSET,
			XCFRAME_FRAME_BCAST, TrimVal);
}

/*****************************************************************************/
/**
 * This function writes the CRAM TRIM value
 *
 * @param	InstancePtr is a pointer to the XCframe instance.
 * @param	TrimValue is to hold CRAM TRIM value
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_CramTrim(const XCframe *InstancePtr, u32 TrimValue)
{
	Xuint128 TrimVal={0};
	Xuint128 MaskVal={0};

	MaskVal.Word0 = XCFRAME_MASK_DEFVAL;
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
 * @param	TrimValue is to hold BRAM TRIM value
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_BramTrim(const XCframe *InstancePtr, u32 TrimValue)
{
	Xuint128 TrimVal={0};
	Xuint128 MaskVal={0};

	MaskVal.Word0 = XCFRAME_MASK_DEFVAL;
	MaskVal.Word1 = XCFRAME_MASK_DEFVAL;
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
 * @param	TrimValue is to hold URAM TRIM value
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_UramTrim(const XCframe *InstancePtr, u32 TrimValue)
{
	Xuint128 TrimVal={0};
	Xuint128 MaskVal={0};

        MaskVal.Word0 = XCFRAME_MASK_DEFVAL;
        MaskVal.Word1 = XCFRAME_MASK_DEFVAL;
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
 * This function sets the CFRAME read parameters with mentioned CFRAME length
 * and frame number.
 *
 * @param       InstancePtr is a pointer to the XCframe instance.
 * @param	CframeNo is the index of frame
 * @param       CframeLen is total length of Cframes
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_SetReadParam(const XCframe *InstancePtr,
		XCframe_FrameNo CframeNo, u32 CframeLen)
{
	Xuint128 Value128={0};

	/* Enable ROWON, READ_CFR, CFRM_CNT */
	XCframe_WriteCmd(InstancePtr, CframeNo,	XCFRAME_CMD_REG_ROWON);
	XCframe_WriteCmd(InstancePtr, CframeNo,	XCFRAME_CMD_REG_RCFG);
	XCframe_WriteReg(InstancePtr, XCFRAME_FAR_OFFSET, CframeNo, &Value128);

	Value128.Word0=CframeLen/4U;
	XCframe_WriteReg(InstancePtr, XCFRAME_FRCNT_OFFSET, CframeNo, &Value128);
}

/*****************************************************************************/
/**
 * This function clears CFRAME ISRs and is called as part of CFRAME error recovery
 *
 * @param	InstancePtr is a pointer to the XCframe instance
 *
 * @return	None
 *
 ******************************************************************************/
void XCframe_ClearCframeErr(const XCframe *InstancePtr)
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
