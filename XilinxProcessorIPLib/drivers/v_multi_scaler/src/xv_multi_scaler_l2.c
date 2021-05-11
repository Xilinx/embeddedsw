/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.	All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_multi_scaler_l2.c
* @addtogroup v_multi_scaler_v1_3
* @{
*
* The Multi Scaler Layer-2 Driver.
* The functions in this file provides an abstraction from the register peek/poke
* methodology by implementing most common use-case provided by the core.
* See xv_multi_scaler_l2.h for a detailed description of the layer-2 driver
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_multi_scaler_l2.h"
#include "xvidc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Local Global *******************************/
static const u32 (*XV_MS_Get_WidthIn[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr) = { XV_multi_scaler_Get_HwReg_WidthIn_0,
	XV_multi_scaler_Get_HwReg_WidthIn_1,
	XV_multi_scaler_Get_HwReg_WidthIn_2,
	XV_multi_scaler_Get_HwReg_WidthIn_3,
	XV_multi_scaler_Get_HwReg_WidthIn_4,
	XV_multi_scaler_Get_HwReg_WidthIn_5,
	XV_multi_scaler_Get_HwReg_WidthIn_6,
	XV_multi_scaler_Get_HwReg_WidthIn_7};
static const u32 (*XV_MS_Get_WidthOut[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr) = { XV_multi_scaler_Get_HwReg_WidthOut_0,
	XV_multi_scaler_Get_HwReg_WidthOut_1,
	XV_multi_scaler_Get_HwReg_WidthOut_2,
	XV_multi_scaler_Get_HwReg_WidthOut_3,
	XV_multi_scaler_Get_HwReg_WidthOut_4,
	XV_multi_scaler_Get_HwReg_WidthOut_5,
	XV_multi_scaler_Get_HwReg_WidthOut_6,
	XV_multi_scaler_Get_HwReg_WidthOut_7};
static const u32 (*XV_MS_Get_HeightIn[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr) = { XV_multi_scaler_Get_HwReg_HeightIn_0,
	XV_multi_scaler_Get_HwReg_HeightIn_1,
	XV_multi_scaler_Get_HwReg_HeightIn_2,
	XV_multi_scaler_Get_HwReg_HeightIn_3,
	XV_multi_scaler_Get_HwReg_HeightIn_4,
	XV_multi_scaler_Get_HwReg_HeightIn_5,
	XV_multi_scaler_Get_HwReg_HeightIn_6,
	XV_multi_scaler_Get_HwReg_HeightIn_7};
static const u32 (*XV_MS_Get_HeightOut[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr) = { XV_multi_scaler_Get_HwReg_HeightOut_0,
	XV_multi_scaler_Get_HwReg_HeightOut_1,
	XV_multi_scaler_Get_HwReg_HeightOut_2,
	XV_multi_scaler_Get_HwReg_HeightOut_3,
	XV_multi_scaler_Get_HwReg_HeightOut_4,
	XV_multi_scaler_Get_HwReg_HeightOut_5,
	XV_multi_scaler_Get_HwReg_HeightOut_6,
	XV_multi_scaler_Get_HwReg_HeightOut_7};
static const u32 (*XV_MS_Get_ColorFormatIn[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr) = { XV_multi_scaler_Get_HwReg_InPixelFmt_0,
	XV_multi_scaler_Get_HwReg_InPixelFmt_1,
	XV_multi_scaler_Get_HwReg_InPixelFmt_2,
	XV_multi_scaler_Get_HwReg_InPixelFmt_3,
	XV_multi_scaler_Get_HwReg_InPixelFmt_4,
	XV_multi_scaler_Get_HwReg_InPixelFmt_5,
	XV_multi_scaler_Get_HwReg_InPixelFmt_6,
	XV_multi_scaler_Get_HwReg_InPixelFmt_7};
static const u32 (*XV_MS_Get_ColorFormatOut[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr) = { XV_multi_scaler_Get_HwReg_OutPixelFmt_0,
	XV_multi_scaler_Get_HwReg_OutPixelFmt_1,
	XV_multi_scaler_Get_HwReg_OutPixelFmt_2,
	XV_multi_scaler_Get_HwReg_OutPixelFmt_3,
	XV_multi_scaler_Get_HwReg_OutPixelFmt_4,
	XV_multi_scaler_Get_HwReg_OutPixelFmt_5,
	XV_multi_scaler_Get_HwReg_OutPixelFmt_6,
	XV_multi_scaler_Get_HwReg_OutPixelFmt_7};
static const u32 (*XV_MS_Get_InStride[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr) = { XV_multi_scaler_Get_HwReg_InStride_0,
	XV_multi_scaler_Get_HwReg_InStride_1,
	XV_multi_scaler_Get_HwReg_InStride_2,
	XV_multi_scaler_Get_HwReg_InStride_3,
	XV_multi_scaler_Get_HwReg_InStride_4,
	XV_multi_scaler_Get_HwReg_InStride_5,
	XV_multi_scaler_Get_HwReg_InStride_6,
	XV_multi_scaler_Get_HwReg_InStride_7};
static const u32 (*XV_MS_Get_OutStride[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr) = { XV_multi_scaler_Get_HwReg_OutStride_0,
	XV_multi_scaler_Get_HwReg_OutStride_1,
	XV_multi_scaler_Get_HwReg_OutStride_2,
	XV_multi_scaler_Get_HwReg_OutStride_3,
	XV_multi_scaler_Get_HwReg_OutStride_4,
	XV_multi_scaler_Get_HwReg_OutStride_5,
	XV_multi_scaler_Get_HwReg_OutStride_6,
	XV_multi_scaler_Get_HwReg_OutStride_7};
static const u64 (*XV_MS_Get_SrcImgBuf0[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr) = { XV_multi_scaler_Get_HwReg_srcImgBuf0_0_V,
	XV_multi_scaler_Get_HwReg_srcImgBuf0_1_V,
	XV_multi_scaler_Get_HwReg_srcImgBuf0_2_V,
	XV_multi_scaler_Get_HwReg_srcImgBuf0_3_V,
	XV_multi_scaler_Get_HwReg_srcImgBuf0_4_V,
	XV_multi_scaler_Get_HwReg_srcImgBuf0_5_V,
	XV_multi_scaler_Get_HwReg_srcImgBuf0_6_V,
	XV_multi_scaler_Get_HwReg_srcImgBuf0_7_V};
static const u64 (*XV_MS_Get_SrcImgBuf1[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr) = { XV_multi_scaler_Get_HwReg_srcImgBuf1_0_V,
	XV_multi_scaler_Get_HwReg_srcImgBuf1_1_V,
	XV_multi_scaler_Get_HwReg_srcImgBuf1_2_V,
	XV_multi_scaler_Get_HwReg_srcImgBuf1_3_V,
	XV_multi_scaler_Get_HwReg_srcImgBuf1_4_V,
	XV_multi_scaler_Get_HwReg_srcImgBuf1_5_V,
	XV_multi_scaler_Get_HwReg_srcImgBuf1_6_V,
	XV_multi_scaler_Get_HwReg_srcImgBuf1_7_V};
static const u64 (*XV_MS_Get_DstImgBuf0[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr) = { XV_multi_scaler_Get_HwReg_dstImgBuf0_0_V,
	XV_multi_scaler_Get_HwReg_dstImgBuf0_1_V,
	XV_multi_scaler_Get_HwReg_dstImgBuf0_2_V,
	XV_multi_scaler_Get_HwReg_dstImgBuf0_3_V,
	XV_multi_scaler_Get_HwReg_dstImgBuf0_4_V,
	XV_multi_scaler_Get_HwReg_dstImgBuf0_5_V,
	XV_multi_scaler_Get_HwReg_dstImgBuf0_6_V,
	XV_multi_scaler_Get_HwReg_dstImgBuf0_7_V};
static const u64 (*XV_MS_Get_DstImgBuf1[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr) = { XV_multi_scaler_Get_HwReg_dstImgBuf1_0_V,
	XV_multi_scaler_Get_HwReg_dstImgBuf1_1_V,
	XV_multi_scaler_Get_HwReg_dstImgBuf1_2_V,
	XV_multi_scaler_Get_HwReg_dstImgBuf1_3_V,
	XV_multi_scaler_Get_HwReg_dstImgBuf1_4_V,
	XV_multi_scaler_Get_HwReg_dstImgBuf1_5_V,
	XV_multi_scaler_Get_HwReg_dstImgBuf1_6_V,
	XV_multi_scaler_Get_HwReg_dstImgBuf1_7_V};
static const void (*XV_MS_Set_WidthIn[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr, u32 Data) = { XV_multi_scaler_Set_HwReg_WidthIn_0,
	XV_multi_scaler_Set_HwReg_WidthIn_1,
	XV_multi_scaler_Set_HwReg_WidthIn_2,
	XV_multi_scaler_Set_HwReg_WidthIn_3,
	XV_multi_scaler_Set_HwReg_WidthIn_4,
	XV_multi_scaler_Set_HwReg_WidthIn_5,
	XV_multi_scaler_Set_HwReg_WidthIn_6,
	XV_multi_scaler_Set_HwReg_WidthIn_7};
static const void (*XV_MS_Set_WidthOut[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr, u32 Data) = { XV_multi_scaler_Set_HwReg_WidthOut_0,
	XV_multi_scaler_Set_HwReg_WidthOut_1,
	XV_multi_scaler_Set_HwReg_WidthOut_2,
	XV_multi_scaler_Set_HwReg_WidthOut_3,
	XV_multi_scaler_Set_HwReg_WidthOut_4,
	XV_multi_scaler_Set_HwReg_WidthOut_5,
	XV_multi_scaler_Set_HwReg_WidthOut_6,
	XV_multi_scaler_Set_HwReg_WidthOut_7};
static const void (*XV_MS_Set_HeightIn[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr, u32 Data) = { XV_multi_scaler_Set_HwReg_HeightIn_0,
	XV_multi_scaler_Set_HwReg_HeightIn_1,
	XV_multi_scaler_Set_HwReg_HeightIn_2,
	XV_multi_scaler_Set_HwReg_HeightIn_3,
	XV_multi_scaler_Set_HwReg_HeightIn_4,
	XV_multi_scaler_Set_HwReg_HeightIn_5,
	XV_multi_scaler_Set_HwReg_HeightIn_6,
	XV_multi_scaler_Set_HwReg_HeightIn_7};
static const void (*XV_MS_Set_HeightOut[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr, u32 Data) = { XV_multi_scaler_Set_HwReg_HeightOut_0,
	XV_multi_scaler_Set_HwReg_HeightOut_1,
	XV_multi_scaler_Set_HwReg_HeightOut_2,
	XV_multi_scaler_Set_HwReg_HeightOut_3,
	XV_multi_scaler_Set_HwReg_HeightOut_4,
	XV_multi_scaler_Set_HwReg_HeightOut_5,
	XV_multi_scaler_Set_HwReg_HeightOut_6,
	XV_multi_scaler_Set_HwReg_HeightOut_7};
static const void (*XV_MS_Set_LineRate[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr, u32 Data) = { XV_multi_scaler_Set_HwReg_LineRate_0,
	XV_multi_scaler_Set_HwReg_LineRate_1,
	XV_multi_scaler_Set_HwReg_LineRate_2,
	XV_multi_scaler_Set_HwReg_LineRate_3,
	XV_multi_scaler_Set_HwReg_LineRate_4,
	XV_multi_scaler_Set_HwReg_LineRate_5,
	XV_multi_scaler_Set_HwReg_LineRate_6,
	XV_multi_scaler_Set_HwReg_LineRate_7};
static const void (*XV_MS_Set_PixelRate[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr, u32 Data) = { XV_multi_scaler_Set_HwReg_PixelRate_0,
	XV_multi_scaler_Set_HwReg_PixelRate_1,
	XV_multi_scaler_Set_HwReg_PixelRate_2,
	XV_multi_scaler_Set_HwReg_PixelRate_3,
	XV_multi_scaler_Set_HwReg_PixelRate_4,
	XV_multi_scaler_Set_HwReg_PixelRate_5,
	XV_multi_scaler_Set_HwReg_PixelRate_6,
	XV_multi_scaler_Set_HwReg_PixelRate_7};
static const void (*XV_MS_Set_ColorFormatIn[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr, u32 Data) = { XV_multi_scaler_Set_HwReg_InPixelFmt_0,
	XV_multi_scaler_Set_HwReg_InPixelFmt_1,
	XV_multi_scaler_Set_HwReg_InPixelFmt_2,
	XV_multi_scaler_Set_HwReg_InPixelFmt_3,
	XV_multi_scaler_Set_HwReg_InPixelFmt_4,
	XV_multi_scaler_Set_HwReg_InPixelFmt_5,
	XV_multi_scaler_Set_HwReg_InPixelFmt_6,
	XV_multi_scaler_Set_HwReg_InPixelFmt_7};
static const void (*XV_MS_Set_ColorFormatOut[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr, u32 Data) = { XV_multi_scaler_Set_HwReg_OutPixelFmt_0,
	XV_multi_scaler_Set_HwReg_OutPixelFmt_1,
	XV_multi_scaler_Set_HwReg_OutPixelFmt_2,
	XV_multi_scaler_Set_HwReg_OutPixelFmt_3,
	XV_multi_scaler_Set_HwReg_OutPixelFmt_4,
	XV_multi_scaler_Set_HwReg_OutPixelFmt_5,
	XV_multi_scaler_Set_HwReg_OutPixelFmt_6,
	XV_multi_scaler_Set_HwReg_OutPixelFmt_7};
static const void (*XV_MS_Set_InStride[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr, u32 Data) = { XV_multi_scaler_Set_HwReg_InStride_0,
	XV_multi_scaler_Set_HwReg_InStride_1,
	XV_multi_scaler_Set_HwReg_InStride_2,
	XV_multi_scaler_Set_HwReg_InStride_3,
	XV_multi_scaler_Set_HwReg_InStride_4,
	XV_multi_scaler_Set_HwReg_InStride_5,
	XV_multi_scaler_Set_HwReg_InStride_6,
	XV_multi_scaler_Set_HwReg_InStride_7};
static const void (*XV_MS_Set_OutStride[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr, u32 Data) = { XV_multi_scaler_Set_HwReg_OutStride_0,
	XV_multi_scaler_Set_HwReg_OutStride_1,
	XV_multi_scaler_Set_HwReg_OutStride_2,
	XV_multi_scaler_Set_HwReg_OutStride_3,
	XV_multi_scaler_Set_HwReg_OutStride_4,
	XV_multi_scaler_Set_HwReg_OutStride_5,
	XV_multi_scaler_Set_HwReg_OutStride_6,
	XV_multi_scaler_Set_HwReg_OutStride_7};
static const void (*XV_MS_Set_SrcImgBuf0[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr, u64 Data) = { XV_multi_scaler_Set_HwReg_srcImgBuf0_0_V,
	XV_multi_scaler_Set_HwReg_srcImgBuf0_1_V,
	XV_multi_scaler_Set_HwReg_srcImgBuf0_2_V,
	XV_multi_scaler_Set_HwReg_srcImgBuf0_3_V,
	XV_multi_scaler_Set_HwReg_srcImgBuf0_4_V,
	XV_multi_scaler_Set_HwReg_srcImgBuf0_5_V,
	XV_multi_scaler_Set_HwReg_srcImgBuf0_6_V,
	XV_multi_scaler_Set_HwReg_srcImgBuf0_7_V};
static const void (*XV_MS_Set_SrcImgBuf1[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr, u64 Data) = { XV_multi_scaler_Set_HwReg_srcImgBuf1_0_V,
	XV_multi_scaler_Set_HwReg_srcImgBuf1_1_V,
	XV_multi_scaler_Set_HwReg_srcImgBuf1_2_V,
	XV_multi_scaler_Set_HwReg_srcImgBuf1_3_V,
	XV_multi_scaler_Set_HwReg_srcImgBuf1_4_V,
	XV_multi_scaler_Set_HwReg_srcImgBuf1_5_V,
	XV_multi_scaler_Set_HwReg_srcImgBuf1_6_V,
	XV_multi_scaler_Set_HwReg_srcImgBuf1_7_V};
static const void (*XV_MS_Set_DstImgBuf0[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr, u64 Data) = { XV_multi_scaler_Set_HwReg_dstImgBuf0_0_V,
	XV_multi_scaler_Set_HwReg_dstImgBuf0_1_V,
	XV_multi_scaler_Set_HwReg_dstImgBuf0_2_V,
	XV_multi_scaler_Set_HwReg_dstImgBuf0_3_V,
	XV_multi_scaler_Set_HwReg_dstImgBuf0_4_V,
	XV_multi_scaler_Set_HwReg_dstImgBuf0_5_V,
	XV_multi_scaler_Set_HwReg_dstImgBuf0_6_V,
	XV_multi_scaler_Set_HwReg_dstImgBuf0_7_V};
static const void (*XV_MS_Set_DstImgBuf1[XV_MAX_OUTS])(XV_multi_scaler
	*InstancePtr, u64 Data) = { XV_multi_scaler_Set_HwReg_dstImgBuf1_0_V,
	XV_multi_scaler_Set_HwReg_dstImgBuf1_1_V,
	XV_multi_scaler_Set_HwReg_dstImgBuf1_2_V,
	XV_multi_scaler_Set_HwReg_dstImgBuf1_3_V,
	XV_multi_scaler_Set_HwReg_dstImgBuf1_4_V,
	XV_multi_scaler_Set_HwReg_dstImgBuf1_5_V,
	XV_multi_scaler_Set_HwReg_dstImgBuf1_6_V,
	XV_multi_scaler_Set_HwReg_dstImgBuf1_7_V};

/************************** Function Prototypes ******************************/
static void XV_MultiScalerSetCoeff(XV_multi_scaler *MscPtr,
				   XV_multi_scaler_Video_Config *MS_cfg);

/*****************************************************************************/
/**
* This function starts the multi scaler core
*
* @param	InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_MultiScalerStart(XV_multi_scaler *InstancePtr)
{
	u32 status = XST_SUCCESS;
	u32 num_outs;
	Xil_AssertVoid(InstancePtr != NULL);

	num_outs = XV_multi_scaler_Get_HwReg_num_outs(InstancePtr);
	Xil_AssertVoid(num_outs != 0)

	Xil_AssertVoid(!(InstancePtr->OutBitMask ^
		(XV_MULTISCALER_OUTPUT_MASK >> (XV_MAX_OUTS - num_outs))));

	XV_multi_scaler_EnableAutoRestart(InstancePtr);
	XV_multi_scaler_Start(InstancePtr);
}

/*****************************************************************************/
/**
* This function stops the multi scaler core
*
* @param	InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_MultiScalerStop(XV_multi_scaler *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XV_multi_scaler_DisableAutoRestart(InstancePtr);
}

/*****************************************************************************/
/**
* This function returns the number of outputs
*
* @param	InstancePtr is a pointer to the core instance to be worked on.
*
* @return number of outputs
*
******************************************************************************/
u32 XV_MultiScalerGetNumOutputs(XV_multi_scaler *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	return XV_multi_scaler_Get_HwReg_num_outs(InstancePtr);
}

/*****************************************************************************/
/**
* This function sets the number of outputs
*
* @param	InstancePtr is a pointer to the core instance to be worked on.
* @param	NumOuts is the number of output channels.
*
* @return None
*
******************************************************************************/
void XV_MultiScalerSetNumOutputs(XV_multi_scaler *InstancePtr, u32 NumOuts)
{
	Xil_AssertVoid(InstancePtr != NULL);

	Xil_AssertVoid((NumOuts > 0) && (NumOuts <= InstancePtr->MaxOuts));

	XV_multi_scaler_Set_HwReg_num_outs(InstancePtr, NumOuts);
	InstancePtr->OutBitMask = 0;
}

/*****************************************************************************/
/**
* This function programs the computed filter coefficients and phase data into
* core registers
*
* @param	MscPtr is a pointer to the core instance to be worked on.
* @param	NumOut is the output channel number.
*
* @return None
*
******************************************************************************/
static void XV_MultiScalerSetCoeff(XV_multi_scaler *MscPtr,
		XV_multi_scaler_Video_Config *MS_cfg)
{
	u32 num_phases = 1<<MscPtr->PhaseShift;
	u32 num_taps	= MscPtr->NumTaps/2;
	u32 val;
	u32 i;
	u32 j;
	u32 baseAddr;
	short *coeff;
	u32 vfltcoef_offset;
	u32 hfltcoef_offset;
	float scale;

	scale = (float)MS_cfg->HeightIn / MS_cfg->HeightOut;
	if ((scale >= 2) && (scale < 2.5))
	{
		if(MscPtr->NumTaps == 6)
			coeff = &XV_multiscaler_fixedcoeff_taps6_6C[0][0];
		else
			coeff = &XV_multiscaler_fixedcoeff_taps8_12C[0][0];
	}
	if ((scale >= 2.5) && (scale < 3))
	{
		if(MscPtr->NumTaps >= 10)
			coeff = &XV_multiscaler_fixedcoeff_taps10_12C[0][0];
		else
		{
			if(MscPtr->NumTaps == 6)
				coeff = &XV_multiscaler_fixedcoeff_taps6_6C[0][0];
			else
				coeff = &XV_multiscaler_fixedcoeff_taps8_8C[0][0];
		}
	}

	if ((scale >= 3) && (scale < 3.5))
		if(MscPtr->NumTaps == 12)
			coeff = &XV_multiscaler_fixedcoeff_taps12_12C[0][0];
		else
		{
			if(MscPtr->NumTaps == 6)
				coeff = &XV_multiscaler_fixedcoeff_taps6_6C[0][0];
			if(MscPtr->NumTaps == 8)
				coeff = &XV_multiscaler_fixedcoeff_taps8_8C[0][0];
			if(MscPtr->NumTaps == 10)
				coeff = &XV_multiscaler_fixedcoeff_taps10_10C[0][0];
		}

	if ((scale >= 3.5) || (scale < 2 && scale >= 1))
	{
		if(MscPtr->NumTaps == 6)
			coeff = &XV_multiscaler_fixedcoeff_taps6_6C[0][0];
		if(MscPtr->NumTaps == 8)
			coeff = &XV_multiscaler_fixedcoeff_taps8_8C[0][0];
		if(MscPtr->NumTaps == 10)
			coeff = &XV_multiscaler_fixedcoeff_taps10_10C[0][0];
		if(MscPtr->NumTaps == 12)
			coeff = &XV_multiscaler_fixedcoeff_taps12_12C[0][0];
	}
	if(scale < 1)
		coeff = &XV_multiscaler_fixedcoeff_taps6_12C[0][0];

	vfltcoef_offset = XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE +
		MS_cfg->ChannelId *
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_FLTCOEFF_OFFSET;

	baseAddr = MscPtr->Ctrl_BaseAddress + vfltcoef_offset;
	for (i = 0; i < num_phases; i++) {
		for (j = 0; j < XV_MULTISCALER_TAPS_12; j = j + 2) {
			val = (coeff[i * XV_MULTISCALER_TAPS_12 + (j + 1)] << 16) |
				(coeff[i * XV_MULTISCALER_TAPS_12 + j] & 0x0000FFFF);
			XV_multi_scaler_WriteReg(baseAddr,
					((i * num_taps + j / 2) * 4), val);
		}
	}

	scale = (float)MS_cfg->WidthIn / MS_cfg->WidthOut;
	if ((scale >= 2) && (scale < 2.5))
	{
		if(MscPtr->NumTaps == 6)
			coeff = &XV_multiscaler_fixedcoeff_taps6_6C[0][0];
		else
			coeff = &XV_multiscaler_fixedcoeff_taps8_12C[0][0];
	}
	if ((scale >= 2.5) && (scale < 3))
	{
		if(MscPtr->NumTaps >= 10)
			coeff = &XV_multiscaler_fixedcoeff_taps10_12C[0][0];
		else
		{
			if(MscPtr->NumTaps == 6)
				coeff = &XV_multiscaler_fixedcoeff_taps6_6C[0][0];
			else
				coeff = &XV_multiscaler_fixedcoeff_taps8_8C[0][0];
		}
	}

	if ((scale >= 3) && (scale < 3.5))
		if(MscPtr->NumTaps == 12)
			coeff = &XV_multiscaler_fixedcoeff_taps12_12C[0][0];
		else
		{
			if(MscPtr->NumTaps == 6)
				coeff = &XV_multiscaler_fixedcoeff_taps6_6C[0][0];
			if(MscPtr->NumTaps == 8)
				coeff = &XV_multiscaler_fixedcoeff_taps8_8C[0][0];
			if(MscPtr->NumTaps == 10)
				coeff = &XV_multiscaler_fixedcoeff_taps10_10C[0][0];
		}

	if ((scale >= 3.5) || (scale < 2 && scale >= 1))
	{
		if(MscPtr->NumTaps == 6)
			coeff = &XV_multiscaler_fixedcoeff_taps6_6C[0][0];
		if(MscPtr->NumTaps == 8)
			coeff = &XV_multiscaler_fixedcoeff_taps8_8C[0][0];
		if(MscPtr->NumTaps == 10)
			coeff = &XV_multiscaler_fixedcoeff_taps10_10C[0][0];
		if(MscPtr->NumTaps == 12)
			coeff = &XV_multiscaler_fixedcoeff_taps12_12C[0][0];
	}
	if(scale < 1)
		coeff = &XV_multiscaler_fixedcoeff_taps6_12C[0][0];

	hfltcoef_offset = XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE +
		MS_cfg->ChannelId *
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_FLTCOEFF_OFFSET;

	baseAddr = MscPtr->Ctrl_BaseAddress + hfltcoef_offset;
	for (i = 0; i < num_phases; i++) {
		for (j = 0; j < XV_MULTISCALER_TAPS_12; j = j + 2) {
			val = (coeff[i * XV_MULTISCALER_TAPS_12 + (j + 1)] << 16) |
				(coeff[i * XV_MULTISCALER_TAPS_12 + j] & 0x0000FFFF);
			XV_multi_scaler_WriteReg(baseAddr,
					((i * num_taps + j / 2) * 4), val);
		}
	}
}

/*****************************************************************************/
/**
* This function reads the channel configuration. The ChannelId of the channel
* for which the configuration info is needed has to be filled by the application
* in the MS_cfg structure.
*
* @param	InstancePtr is a pointer to the core instance to be worked on.
* @param	MS_cfg is a pointer to the multi scaler config structure.
*
* @return None
*
******************************************************************************/
void XV_MultiScalerGetChannelConfig(XV_multi_scaler *InstancePtr,
	XV_multi_scaler_Video_Config *MS_cfg)
{
	u32 i;

	/*
	* Assert validates the input arguments
	*/
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MS_cfg != NULL);
	Xil_AssertVoid((InstancePtr->SamplesPerClock >= XVIDC_PPC_1) &&
		(InstancePtr->SamplesPerClock <= XVIDC_PPC_4));

	i = MS_cfg->ChannelId;
	Xil_AssertVoid(i < InstancePtr->MaxOuts);
	MS_cfg->WidthIn = XV_MS_Get_WidthIn[i](InstancePtr);
	MS_cfg->WidthOut = XV_MS_Get_WidthOut[i](InstancePtr);
	MS_cfg->HeightIn = XV_MS_Get_HeightIn[i](InstancePtr);
	MS_cfg->HeightOut = XV_MS_Get_HeightOut[i](InstancePtr);
	MS_cfg->ColorFormatIn = XV_MS_Get_ColorFormatIn[i](InstancePtr);
	MS_cfg->ColorFormatOut = XV_MS_Get_ColorFormatOut[i](InstancePtr);
	MS_cfg->InStride = XV_MS_Get_InStride[i](InstancePtr);
	MS_cfg->OutStride = XV_MS_Get_OutStride[i](InstancePtr);
	MS_cfg->SrcImgBuf0 = XV_MS_Get_SrcImgBuf0[i](InstancePtr);
	MS_cfg->SrcImgBuf1 = XV_MS_Get_SrcImgBuf1[i](InstancePtr);
	MS_cfg->DstImgBuf0 = XV_MS_Get_DstImgBuf0[i](InstancePtr);
	MS_cfg->DstImgBuf1 = XV_MS_Get_DstImgBuf1[i](InstancePtr);
}

/*****************************************************************************/
/**
* This function configures the scaler core registers with the specified
* configuration parameters
*
* @param	InstancePtr is a pointer to the core instance to be worked on.
* @param	MS_cfg is a pointer to the multi scaler config structure.
*
* @return None
*
******************************************************************************/
void XV_MultiScalerSetChannelConfig(XV_multi_scaler *InstancePtr,
	XV_multi_scaler_Video_Config *MS_cfg)
{
	u32 PixelRate;
	u32 LineRate;
	u32 i;
	u16 Cfmt;
	UINTPTR SrcImgBuf0;
	UINTPTR SrcImgBuf1;
	u8 buf0_numerator;
	u8 buf0_denominator;
	u8 buf1_numerator;
	u8 buf1_denominator;

	/*
	* Assert validates the input arguments
	*/
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MS_cfg != NULL);
	Xil_AssertVoid((InstancePtr->SamplesPerClock >= XVIDC_PPC_1) &&
		(InstancePtr->SamplesPerClock <= XVIDC_PPC_4));

	i = MS_cfg->ChannelId;
	Xil_AssertVoid(i < InstancePtr->MaxOuts);
	Xil_AssertVoid((MS_cfg->WidthIn > 0) &&
		(MS_cfg->WidthIn <= InstancePtr->MaxCols));
	Xil_AssertVoid((MS_cfg->WidthOut > 0) &&
		(MS_cfg->WidthOut <= InstancePtr->MaxCols));
	Xil_AssertVoid((MS_cfg->HeightIn > 0) &&
		(MS_cfg->HeightIn <= InstancePtr->MaxRows));
	Xil_AssertVoid((MS_cfg->HeightOut > 0) &&
		(MS_cfg->HeightOut <= InstancePtr->MaxRows));
	if (MS_cfg->SrcImgBuf0 <= MS_cfg->DstImgBuf0) {
		Xil_AssertVoid(MS_cfg->DstImgBuf0 > (MS_cfg->SrcImgBuf0 +
			(MS_cfg->HeightIn * MS_cfg->InStride)));
	} else {
		Xil_AssertVoid(MS_cfg->SrcImgBuf0 > (MS_cfg->DstImgBuf0 +
			(MS_cfg->HeightOut * MS_cfg->OutStride)));
	}
	if (MS_cfg->SrcImgBuf1 <= MS_cfg->DstImgBuf1) {
		Xil_AssertVoid(MS_cfg->DstImgBuf1 > (MS_cfg->SrcImgBuf1 +
			(MS_cfg->HeightIn * MS_cfg->InStride)));
	} else {
		Xil_AssertVoid(MS_cfg->SrcImgBuf1 > (MS_cfg->DstImgBuf1 +
			(MS_cfg->HeightOut * MS_cfg->OutStride)));
	}
	InstancePtr->OutBitMask |= 0x1 << i;
	if (MS_cfg->CropWin.Crop) {
		Xil_AssertVoid(MS_cfg->CropWin.StartY <= MS_cfg->HeightIn);
		Xil_AssertVoid(MS_cfg->CropWin.StartX <= MS_cfg->WidthIn);
		Xil_AssertVoid((MS_cfg->CropWin.Height > 0) &&
			(MS_cfg->CropWin.Height <= (MS_cfg->HeightIn -
			MS_cfg->CropWin.StartY)));
		Xil_AssertVoid((MS_cfg->CropWin.Width > 0) &&
			(MS_cfg->CropWin.Width <= (MS_cfg->WidthIn -
			MS_cfg->CropWin.StartX)));
		Cfmt = MS_cfg->ColorFormatIn;
		SrcImgBuf0 = MS_cfg->SrcImgBuf0;
		SrcImgBuf1 = MS_cfg->SrcImgBuf1;
		/* Table 3 Pixel formats supported in PG325 */
		switch (Cfmt) {
			case XV_MULTI_SCALER_Y_UV10:
			case XV_MULTI_SCALER_Y10:
				Xil_AssertVoid(!(MS_cfg->CropWin.StartX % 3));
				buf0_numerator = 4;
				buf0_denominator = 3;
				buf1_numerator = 4;
				buf1_denominator = 3;
				break;
			case XV_MULTI_SCALER_Y_UV8:
			case XV_MULTI_SCALER_Y8:
				buf0_numerator = 1;
				buf0_denominator = 1;
				buf1_numerator = 1;
				buf1_denominator = 1;
				break;
			case XV_MULTI_SCALER_Y_UV8_420:
				Xil_AssertVoid(!(MS_cfg->CropWin.StartX % 4));
				buf0_numerator = 1;
				buf0_denominator = 1;
				buf1_numerator = 2;
				buf1_denominator = 4;
				break;
			case XV_MULTI_SCALER_Y_UV10_420:
				Xil_AssertVoid(!(MS_cfg->CropWin.StartX % 12));
				buf0_numerator = 1;
				buf0_denominator = 1;
				buf1_numerator = 8;
				buf1_denominator = 12;
				break;
			case XV_MULTI_SCALER_RGB8:
			case XV_MULTI_SCALER_YUV8:
			case XV_MULTI_SCALER_BGR8:
				buf0_numerator = 3;
				buf0_denominator = 1;
				buf1_numerator = 1;
				buf1_denominator = 1;
				break;
			case XV_MULTI_SCALER_YUYV8:
			case XV_MULTI_SCALER_UYVY8:
				buf0_numerator = 2;
				buf0_denominator = 1;
				buf1_numerator = 1;
				buf1_denominator = 1;
				break;
			default:
				buf0_numerator = 4;
				buf0_denominator = 1;
				buf1_numerator = 1;
				buf1_denominator = 1;
				break;
		}
		SrcImgBuf0 += (MS_cfg->CropWin.StartY * MS_cfg->InStride)
			+ ((MS_cfg->CropWin.StartX * buf0_numerator) / buf0_denominator);
		SrcImgBuf1 += (MS_cfg->CropWin.StartY * MS_cfg->InStride)
			+ ((MS_cfg->CropWin.StartX * buf1_numerator) / buf1_denominator);
		XV_MS_Set_SrcImgBuf0[i](InstancePtr, SrcImgBuf0);
		XV_MS_Set_SrcImgBuf1[i](InstancePtr, SrcImgBuf1);
		PixelRate = (u32) ((float)(MS_cfg->CropWin.Width * STEP_PRECISION +
			MS_cfg->WidthOut / 2) / MS_cfg->WidthOut);
		LineRate = (u32) ((float)(MS_cfg->CropWin.Height * STEP_PRECISION +
			MS_cfg->HeightOut / 2) / MS_cfg->HeightOut);
		XV_MS_Set_HeightIn[i](InstancePtr, MS_cfg->CropWin.Height);
		XV_MS_Set_WidthIn[i](InstancePtr, MS_cfg->CropWin.Width);
	} else {
		XV_MS_Set_SrcImgBuf0[i](InstancePtr, MS_cfg->SrcImgBuf0);
		XV_MS_Set_SrcImgBuf1[i](InstancePtr, MS_cfg->SrcImgBuf1);
		PixelRate = (u32) ((float)(MS_cfg->WidthIn * STEP_PRECISION +
			MS_cfg->WidthOut / 2) / MS_cfg->WidthOut);
		LineRate = (u32) ((float)(MS_cfg->HeightIn * STEP_PRECISION +
			MS_cfg->HeightOut / 2) / MS_cfg->HeightOut);
		XV_MS_Set_HeightIn[i](InstancePtr, MS_cfg->HeightIn);
		XV_MS_Set_WidthIn[i](InstancePtr, MS_cfg->WidthIn);
	}
	XV_MultiScalerSetCoeff(InstancePtr, MS_cfg);
	XV_MS_Set_WidthOut[i](InstancePtr, MS_cfg->WidthOut);
	XV_MS_Set_HeightOut[i](InstancePtr, MS_cfg->HeightOut);
	XV_MS_Set_LineRate[i](InstancePtr, LineRate);
	XV_MS_Set_PixelRate[i](InstancePtr, PixelRate);
	XV_MS_Set_ColorFormatIn[i](InstancePtr, MS_cfg->ColorFormatIn);
	XV_MS_Set_ColorFormatOut[i](InstancePtr, MS_cfg->ColorFormatOut);
	XV_MS_Set_InStride[i](InstancePtr, MS_cfg->InStride);
	XV_MS_Set_OutStride[i](InstancePtr, MS_cfg->OutStride);
	XV_MS_Set_DstImgBuf0[i](InstancePtr, MS_cfg->DstImgBuf0);
	XV_MS_Set_DstImgBuf1[i](InstancePtr, MS_cfg->DstImgBuf1);
}
/** @} */
