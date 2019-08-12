/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xosd.c
* @addtogroup osd_v4_0
* @{
*
* This file is main code of Xilinx Video On-Screen-Display (OSD) core.
* Please see xosd.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00a xd     08/18/08 First release.
* 1.02a xd     12/21/10 Removed endian conversion for text bank loading.
* 1.03a cm     09/07/11 Updated XOsd_GetLayerAlpha(), XOsd_SetLayerAlpha(),
*                       XOsd_SetBackgroundColor() and XOsd_GetBackgroundColor()
*                       to allow 10 and 12 bit alpha and background colors.
* 2.00a cjm    12/18/12 Converted from xio.h to xil_io.h, translating
*                       basic types, MB cache functions, exceptions and
*                       assertions to xil_io format.
* 4.0   adk    02/18/14 Renamed S_AXIS_VIDEO_DATA_WIDTH ->
*                                                      SlaveAxisVideoDataWidth.
*                       Removed from XOsd_CfgInitialize:
*                       VbiStartCallBack, VbiStartRef, VbiEndCallBack,
*                       VbiEndRef.
*
*                       Added in XOsd_CfgInitialize:
*                       ProcStartCallBack, ProcStartRef.
*
*                       Renamed the following function prototypes:
*                       XOSD_CfgInitialize -> XOsd_CfgInitialize,
*                       XOSD_SetScreenSize -> XOsd_SetActiveSize,
*                       XOSD_GetScreenSize -> XOsd_GetActiveSize,
*                       XOSD_SetBackgroundColor -> XOsd_SetBackgroundColor,
*                       XOSD_GetBackgroundColor -> XOSD_GetBackgroundColor,
*                       XOSD_SetLayerDimension -> XOsd_SetLayerDimension,
*                       XOSD_GetLayerDimension -> XOsd_GetLayerDimension,
*                       XOSD_SetLayerAlpha -> XOsd_SetLayerAlpha,
*                       XOSD_GetLayerAlpha -> XOsd_GetLayerAlpha,
*                       XOSD_SetLayerAlpha -> XOsd_SetLayerAlpha,
*                       XOSD_GetLayerAlpha -> XOsd_GetLayerAlpha,
*                       XOSD_SetLayerPriority -> XOsd_SetLayerPriority,
*                       XOSD_GetLayerPriority -> XOsd_GetLayerPriority,
*                       XOSD_EnableLayer -> XOsd_EnableLayer,
*                       XOSD_DisableLayer -> XOsd_DisableLayer,
*                       XOSD_LoadColorLUTBank - > XOsd_LoadColorLUTBank,
*                       XOSD_LoadCharacterSetBank -> XOsd_LoadCharacterSetBank,
*                       XOSD_LoadTextBank - > XOsd_LoadTextBank,
*                       XOSD_SetActiveBank -> XOsd_SetActiveBank,
*                       XOSD_CreateInstruction -> XOsd_CreateInstruction,
*                       XOSD_LoadInstructionList -> XOsd_LoadInstructionList,
*                       XOSD_LookupConfig -> XOsd_LookupConfig,
*                       XOSD_IntrHandler -> XOsd_IntrHandler,
*                       XOSD_SetCallBack -> XOsd_SetCallBack.
*
*                       Changed the prototype of XOSD_GetVersion and renamed it as
*                       XOsd_GetVersion
*
*                       Removed the following function implementation:
*                       XOSD_SetBlankPolarity.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xosd.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void PopulateLayerProperty(XOsd *InstancePtr, XOsd_Config *CfgPtr);
static void StubCallBack(void *CallBackRef);
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes an OSD core. This function must be called
* prior to using an OSD core. Initialization of an OSD includes setting up
* the instance data and ensuring the hardware is in a quiescent state.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	CfgPtr points to the configuration structure associated
*		with the OSD core.
* @param	EffectiveAddr is the base address of the core. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return	- XST_SUCCESS if XOsd_CfgInitialize was successful.
*
* @note		None.
*
******************************************************************************/
int XOsd_CfgInitialize(XOsd *InstancePtr, XOsd_Config *CfgPtr,
				u32 EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(CfgPtr->LayerNum <= (XOSD_MAX_NUM_OF_LAYERS));
	Xil_AssertNonvoid(EffectiveAddr != (u32)0x00);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XOsd));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
			sizeof(XOsd_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Reset to use instruction from the OSD internal buffer */
	InstancePtr->InstructionInExternalMem = 0;

	/* Set all handlers to stub values, let user configure this
	 * data later
	 */
	InstancePtr->ProcStartCallBack =
				(XOsd_CallBack)((void *)StubCallBack);
	InstancePtr->FrameDoneCallBack =
				(XOsd_CallBack)((void *)StubCallBack);
	InstancePtr->ErrCallBack =
			(XOsd_ErrorCallBack)((void *)StubErrCallBack);

	/* Populate the layer properties into a array to help easy
	 * fetch later
	 */
	PopulateLayerProperty(InstancePtr, CfgPtr);

	/* Reset the hardware and set the flag to indicate the
	 * driver is ready
	 */
	XOsd_Reset(InstancePtr);
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function sets the screen size of the OSD Output.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	Width defines the width of the OSD output.
* @param	Height defines the height of the OSD output.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_SetActiveSize(XOsd *InstancePtr, u32 Width, u32 Height)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(Width > (u32)0x0);
	Xil_AssertVoid(Height > (u32)0x0);
	Xil_AssertVoid(Width <= (u32)(XOSD_ACTSIZE_NUM_PIXEL_MASK));
	Xil_AssertVoid(Height <= ((XOSD_ACTSIZE_NUM_LINE_MASK) >>
					(XOSD_ACTSIZE_NUM_LINE_SHIFT)));

	/* Save the dimension info in the driver instance for error handling */
	InstancePtr->ScreenWidth = Width;
	InstancePtr->ScreenHeight = Height;

	/* Update the screen size register */
	RegValue = Width;
	RegValue |= (Height << (XOSD_ACTSIZE_NUM_LINE_SHIFT)) &
					(XOSD_ACTSIZE_NUM_LINE_MASK);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress,
			(XOSD_ACTIVE_SIZE_OFFSET), RegValue);
}

/*****************************************************************************/
/**
*
* This function gets the screen size of the OSD Output.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	WidthPtr will point to the width of the OSD output after this
*		function returns.
* @param	HeightPtr will point to the height of the OSD output after this
*		function returns.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_GetActiveSize(XOsd *InstancePtr, u32 *WidthPtr, u32 *HeightPtr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(WidthPtr != NULL);
	Xil_AssertVoid(HeightPtr != NULL);

	/* Get the screen size info */
	RegValue = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
					(XOSD_ACTIVE_SIZE_OFFSET));
	*WidthPtr = (RegValue) & (XOSD_ACTSIZE_NUM_PIXEL_MASK);
	*HeightPtr = ((RegValue) & (XOSD_ACTSIZE_NUM_LINE_MASK)) >>
						(XOSD_ACTSIZE_NUM_LINE_SHIFT);
}

/*****************************************************************************/
/**
*
* This function sets the background color used by the OSD output.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	Red indicates the red value to be used in the background color.
* @param	Blue indicates the blue value to be used in the background
*		color.
* @param	Green indicates the green value to be used in the background
*		color.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_SetBackgroundColor(XOsd *InstancePtr, u16 Red, u16 Blue, u16 Green)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));

	/* Update the background color register */
	RegValue = ((u32)Green) & (XOSD_BC0_YG_MASK);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress, (XOSD_BC0_OFFSET),
			RegValue);
	RegValue = ((u32)Blue) & (XOSD_BC1_UCBB_MASK);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress, (XOSD_BC1_OFFSET),
			RegValue);
	RegValue = ((u32)Red) & (XOSD_BC2_VCRR_MASK);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress, (XOSD_BC2_OFFSET),
			RegValue);
}

/*****************************************************************************/
/**
*
* This function gets the background color used by the OSD output.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	RedPtr will point to the red value used in the background color
*		after this function returns.
* @param	BluePtr will point to the blue value used in the background
*		color after this function returns.
* @param	GreenPtr will point to the green value used in the background
*		color after this function returns.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_GetBackgroundColor(XOsd *InstancePtr, u16 *RedPtr, u16 *BluePtr,
				u16 *GreenPtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(RedPtr != NULL);
	Xil_AssertVoid(BluePtr != NULL);
	Xil_AssertVoid(GreenPtr != NULL);

	/* Get the background color */
	*GreenPtr = (u16)((XOsd_ReadReg(InstancePtr->Config.BaseAddress,
			(XOSD_BC0_OFFSET))) & (XOSD_BC0_YG_MASK));
	*BluePtr = (u16)((XOsd_ReadReg(InstancePtr->Config.BaseAddress,
			(XOSD_BC1_OFFSET))) & (XOSD_BC1_UCBB_MASK));
	*RedPtr = (u16)((XOsd_ReadReg(InstancePtr->Config.BaseAddress,
			XOSD_BC2_OFFSET)) & (XOSD_BC2_VCRR_MASK));
}


/*****************************************************************************/
/**
*
* This function sets the start position and size of an OSD layer.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	LayerIndex indicates which layer to be worked on. Valid
*		value range is from 0 to (the number of layers implemented in
*		the core - 1).
* @param	XStart indicates the horizontal start pixel of origin of the
*		layer.
* @param	YStart indicates the vertical start line of origin of the
*		layer.
* @param	XSize indicates the horizontal size of the layer.
* @param	YSize indicates the vertical size of the layer.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_SetLayerDimension(XOsd *InstancePtr, u8 LayerIndex, u16 XStart,
				u16 YStart, u16 XSize, u16 YSize)
{
	u32 LayerBaseRegAddr;
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid((u16)LayerIndex < InstancePtr->Config.LayerNum);
	Xil_AssertVoid(InstancePtr->Layers[LayerIndex].LayerType !=
						(XOSD_LAYER_TYPE_DISABLE));
	Xil_AssertVoid(XSize > (u16)0x0);
	Xil_AssertVoid(YSize > (u16)0x0);
	Xil_AssertVoid(((u32)XStart + (u32)XSize) <= InstancePtr->ScreenWidth);
	Xil_AssertVoid(((u32)YStart + (u32)YSize) <=
			InstancePtr->ScreenHeight);

	/* Calculate the base register address of the layer to work on */
	LayerBaseRegAddr = (u32)(XOSD_L0C_OFFSET) + (((u32)LayerIndex) *
				((u32)(XOSD_LAYER_SIZE)));

	/* Set the origin of the layer */
	RegValue = ((u32)XStart) & (XOSD_LXP_XSTART_MASK);
	RegValue |= (((u32)YStart) << (XOSD_LXP_YSTART_SHIFT)) &
					(XOSD_LXP_YSTART_MASK);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress, LayerBaseRegAddr +
			(XOSD_LXP), RegValue);

	/* Set the size of the layer */
	RegValue = ((u32)XSize) & (XOSD_LXS_XSIZE_MASK);
	RegValue |= (((u32)YSize) << (XOSD_LXS_YSIZE_SHIFT)) &
						(XOSD_LXS_YSIZE_MASK);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress,
		((LayerBaseRegAddr) + (XOSD_LXS)), RegValue);
}

/*****************************************************************************/
/**
*
* This function gets the start position and size of an OSD layer.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	LayerIndex indicates which layer to be worked on. Valid
*		value range is from 0 to (the number of layers implemented
*		in the core - 1).
* @param	XStartPtr will point to the horizontal start pixel value
*		of origin of the layer after this function returns.
* @param	YStartPtr will point to the vertical start line of origin
*		of the layer after this function returns.
* @param	XSizePtr will point to the horizontal size value of the
*		layer after this function returns.
* @param	YSizePtr will point to the vertical size value of the
*		layer after this function returns.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_GetLayerDimension(XOsd *InstancePtr, u8 LayerIndex, u16 *XStartPtr,
			u16 *YStartPtr, u16 *XSizePtr, u16 *YSizePtr)
{
	u32 LayerBaseRegAddr;
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid((u16)LayerIndex < InstancePtr->Config.LayerNum);
	Xil_AssertVoid(InstancePtr->Layers[LayerIndex].LayerType !=
					(XOSD_LAYER_TYPE_DISABLE));
	Xil_AssertVoid(XStartPtr != NULL);
	Xil_AssertVoid(YStartPtr != NULL);
	Xil_AssertVoid(XSizePtr != NULL);
	Xil_AssertVoid(YSizePtr != NULL);

	/* Calculate the base register address of the layer to work on */
	LayerBaseRegAddr = (u32)(XOSD_L0C_OFFSET) + (((u32)LayerIndex) *
				((u32)XOSD_LAYER_SIZE));

	/* Get the origin of the layer */
	RegValue = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
				((LayerBaseRegAddr) + (XOSD_LXP)));
	*XStartPtr = (u16)((RegValue) & (XOSD_LXP_XSTART_MASK));
	*YStartPtr = (u16)(((RegValue) & (XOSD_LXP_YSTART_MASK)) >>
				(XOSD_LXP_YSTART_SHIFT));

	/* Get the size of the layer */
	RegValue = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
				LayerBaseRegAddr + (XOSD_LXS));
	*XSizePtr = (u16)((RegValue) & (XOSD_LXS_XSIZE_MASK));
	*YSizePtr = (u16)(((RegValue) & (XOSD_LXS_YSIZE_MASK)) >>
				(XOSD_LXS_YSIZE_SHIFT));
}

/*****************************************************************************/
/**
*
* This function sets the alpha value and mode of an OSD layer.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	LayerIndex indicates which layer to be worked on. Valid value
*		range is from 0 to (the number of layers implemented in the
*		core - 1).
* @param	GlobalAlphaEnable indicates whether to enable the global alpha.
*		Use any non-0 value to enable the global alpha, and 0 to
*		disable it.
* @param	GlobalAlphaValue indicates the transparent level. 0 for 100%
*		transparent, 255 (8bit) for 0% transparent (100% opaque).
*		This argument will be ignored if parameter GlobalAlphaEnble has
*		value 0.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_SetLayerAlpha(XOsd *InstancePtr, u8 LayerIndex,
			u16 GlobalAlphaEnable, u16 GlobalAlphaValue)
{
	u32 LayerBaseRegAddr;
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid((u16)LayerIndex < InstancePtr->Config.LayerNum);
	Xil_AssertVoid(InstancePtr->Layers[LayerIndex].LayerType !=
					(XOSD_LAYER_TYPE_DISABLE));

	/* Calculate the base register address of the layer to work on */
	LayerBaseRegAddr = (u32)(XOSD_L0C_OFFSET) + (((u32)LayerIndex) *
				((u32)(XOSD_LAYER_SIZE)));

	/* Read the current Layer Control register value */
	RegValue = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
					((LayerBaseRegAddr) + (XOSD_LXC)));

	/* Update the global alpha enable and the global alpha value fields */
	if (GlobalAlphaEnable != (u16)0x0) {
		RegValue |= ((u32)(XOSD_LXC_GALPHAEN_MASK));
	}
	else {
		RegValue &= (u32)(~(XOSD_LXC_GALPHAEN_MASK));
	}
	RegValue &= (u32)(~(XOSD_LXC_ALPHA_MASK));
	RegValue |= (((u32)GlobalAlphaValue) << (XOSD_LXC_ALPHA_SHIFT)) &
					(XOSD_LXC_ALPHA_MASK);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress,
			((LayerBaseRegAddr) + (XOSD_LXC)), RegValue);
}

/*****************************************************************************/
/**
*
* This function gets the alpha value and mode of an OSD layer.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	LayerIndex indicates which layer to be worked on. Valid
*		value range is from 0 to (the number of layers implemented
*		in the core - 1).
* @param	GlobalAlphaEnablePtr will point to a flag indicating whether the
*		global alpha is enabled on a layer after this function returns.
*		Flag 1 indicates that the global alpha is enabled, 0 indicates
*		that it is not.
* @param	GlobalAlphaValuePtr will point to the transparent level after
*		this function returns. 0 for 100% transparent, 255 (8bit)
*		for 0% transparent (100% opaque).
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_GetLayerAlpha(XOsd *InstancePtr, u8 LayerIndex,
			u16 *GlobalAlphaEnablePtr, u16 *GlobalAlphaValuePtr)
{
	u32 LayerBaseRegAddr;
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid((u16)LayerIndex < InstancePtr->Config.LayerNum);
	Xil_AssertVoid(InstancePtr->Layers[LayerIndex].LayerType !=
			(XOSD_LAYER_TYPE_DISABLE));
	Xil_AssertVoid(GlobalAlphaEnablePtr != NULL);
	Xil_AssertVoid(GlobalAlphaValuePtr != NULL);

	/* Calculate the base register address of the layer to work on */
	LayerBaseRegAddr = ((u32)(XOSD_L0C_OFFSET) + (((u32)LayerIndex) *
				((u32)(XOSD_LAYER_SIZE))));

	/* Read the current layer control register value */
	RegValue = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
			((LayerBaseRegAddr) + (XOSD_LXC)));

	/* Get the info of the global alpha enable and the global alpha value
	 * fields
	 */
	*GlobalAlphaEnablePtr = (u16)((((RegValue) &
				((u32)(XOSD_LXC_GALPHAEN_MASK))) ==
					(XOSD_LXC_GALPHAEN_MASK)) ? 1 : 0);
	*GlobalAlphaValuePtr = (u16)(((RegValue) & (XOSD_LXC_ALPHA_MASK)) >>
					(XOSD_LXC_ALPHA_SHIFT));
}

/*****************************************************************************/
/**
*
* This function sets the priority of an OSD layer.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	LayerIndex indicates which layer to be worked on. Valid value
*		range is from 0 to (the number of layers implemented in the
*		core - 1).
* @param	Priority indicates the priority to be applied on the layer.
*		Use one of XOSD_LAYER_PRIORITY_0 (the lowest priority) through
*		XOSD_LAYER_PRIORITY_7 (the highest priority) defined in
*		xosd_hw.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_SetLayerPriority(XOsd *InstancePtr, u8 LayerIndex, u8 Priority)
{
	u32 LayerBaseRegAddr;
	u32 RegValue;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid((u16)LayerIndex < InstancePtr->Config.LayerNum);
	Xil_AssertVoid(InstancePtr->Layers[LayerIndex].LayerType !=
					(XOSD_LAYER_TYPE_DISABLE));
	Xil_AssertVoid(Priority <= (XOSD_LAYER_PRIORITY_7));

	/* Calculate the base register address of the layer to work on */
	LayerBaseRegAddr = ((u32)(XOSD_L0C_OFFSET)) + ((((u32)LayerIndex) *
						((u32)(XOSD_LAYER_SIZE))));

	/* Read the current layer control register value */
	RegValue = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
			((LayerBaseRegAddr) + (XOSD_LXC)));

	/* Update the priority field */
	RegValue &= (u32)(~(XOSD_LXC_PRIORITY_MASK));
	RegValue |= (((u32)Priority) << (XOSD_LXC_PRIORITY_SHIFT))
					& (XOSD_LXC_PRIORITY_MASK);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress,
			((LayerBaseRegAddr) + (XOSD_LXC)), RegValue);
}

/*****************************************************************************/
/**
*
* This function gets the priority of an OSD layer.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	LayerIndex indicates which layer to be worked on. Valid value
*		range is from 0 to (the number of layers implemented in the
*		core - 1).
* @param	PriorityPtr will point to the priority used on the layer
*		after this function returns. Use one of XOSD_LAYER_PRIORITY_0
*		(the lowest priority) through XOSD_LAYER_PRIORITY_7
*		(the highest priority) defined in xosd_hw.h to interpret
*		the value.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_GetLayerPriority(XOsd *InstancePtr, u8 LayerIndex, u8 *PriorityPtr)
{
	u32 LayerBaseRegAddr;
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid((u16)LayerIndex < InstancePtr->Config.LayerNum);
	Xil_AssertVoid(InstancePtr->Layers[LayerIndex].LayerType !=
					(XOSD_LAYER_TYPE_DISABLE));
	Xil_AssertVoid(PriorityPtr != NULL);

	/* Calculate the base register address of the layer to work on */
	LayerBaseRegAddr = ((u32)(XOSD_L0C_OFFSET)) + ((((u32)LayerIndex) *
				((u32)(XOSD_LAYER_SIZE))));

	/* Read the current layer control register value */
	RegValue = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
			((LayerBaseRegAddr) + (XOSD_LXC)));

	/* Get the priority field */
	*PriorityPtr = (u8)(((RegValue) & (XOSD_LXC_PRIORITY_MASK)) >>
				(XOSD_LXC_PRIORITY_SHIFT));
}

/*****************************************************************************/
/**
*
* This function enables an OSD layer.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	LayerIndex indicates which layer to be worked on. Valid
*		value range is from 0 to (the number of layers implemented
*		in the core - 1).
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_EnableLayer(XOsd *InstancePtr, u8 LayerIndex)
{
	u32 LayerBaseRegAddr;
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid((u16)LayerIndex < InstancePtr->Config.LayerNum);
	Xil_AssertVoid(InstancePtr->Layers[LayerIndex].LayerType !=
					(XOSD_LAYER_TYPE_DISABLE));

	/* Calculate the base register address of the layer to work on */
	LayerBaseRegAddr = ((u32)(XOSD_L0C_OFFSET)) + ((((u32)LayerIndex) *
				((u32)(XOSD_LAYER_SIZE))));

	/* Read the current layer control register value */
	RegValue = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
			((LayerBaseRegAddr) + (XOSD_LXC)));

	/* Set the layer enable field */
	RegValue |= (XOSD_LXC_EN_MASK);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress,
			((LayerBaseRegAddr) + (XOSD_LXC)), RegValue);
}

/*****************************************************************************/
/**
*
* This function disables an OSD layer.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	LayerIndex indicates which layer to be worked on. Valid
*		value range is from 0 to (the number of layers implemented
*		in the core - 1).
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_DisableLayer(XOsd *InstancePtr, u8 LayerIndex)
{
	u32 LayerBaseRegAddr;
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid((u16)LayerIndex < InstancePtr->Config.LayerNum);
	Xil_AssertVoid(InstancePtr->Layers[LayerIndex].LayerType !=
					(XOSD_LAYER_TYPE_DISABLE));

	/* Calculate the base register address of the layer to work on */
	LayerBaseRegAddr = ((u32)(XOSD_L0C_OFFSET)) +
			((((u32)LayerIndex) * ((u32)(XOSD_LAYER_SIZE))));

	/* Read the current layer control register value */
	RegValue = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
			((LayerBaseRegAddr) + (XOSD_LXC)));

	/* Clear the layer enable field */
	RegValue &= (u32)(~(XOSD_LXC_EN_MASK));
	XOsd_WriteReg(InstancePtr->Config.BaseAddress,
			((LayerBaseRegAddr) + (XOSD_LXC)), RegValue);
}

/*****************************************************************************/
/**
*
* This function loads color look up table data into an OSD Graphics Controller
* bank.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	GcIndex indicates which Graphics Controller to work on.
*		Valid value range is from 0 to (The Number of Layers) - 1.
*		The layer's type must be set to XOSD_LAYER_TYPE_GPU
*		(defined in xosd_hw.h) for this function to work properly.
* @param	BankIndex indicates which GC Bank to be worked on. Valid
*		value range is from 0 to XOSD_GC_BANK_NUM - 1.
* @param	ColorData points to the color LUT data to be loaded.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_LoadColorLUTBank(XOsd *InstancePtr, u8 GcIndex, u8 BankIndex,
							u32 ColorData[])
{
	u32 Index;
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid((u16)GcIndex < InstancePtr->Config.LayerNum);
	Xil_AssertVoid(InstancePtr->Layers[GcIndex].LayerType ==
					(XOSD_LAYER_TYPE_GPU));
	Xil_AssertVoid(BankIndex < (XOSD_GC_BANK_NUM));
	Xil_AssertVoid(ColorData != NULL);

	/* Choose which bank to be loaded */
	RegValue = (((u32)BankIndex) + (XOSD_GCWBA_COL0)) &
			(XOSD_GCWBA_BANK_MASK);
	RegValue |= (((u32)GcIndex) << (XOSD_GCWBA_GCNUM_SHIFT))
					& (XOSD_GCWBA_GCNUM_MASK);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress, (XOSD_GCWBA_OFFSET),
			RegValue);

	/* Load color data */
	if(InstancePtr->Config.SlaveAxisVideoDataWidth == (u16)(XOSD_DATA_8)) {
		for(Index = 0x0;
		Index < ((InstancePtr->Layers[GcIndex].ColorLutSize *
				(XOSD_COLOR_ENTRY_SIZE)) / sizeof(u32));
		Index++) {
			XOsd_WriteReg(InstancePtr->Config.BaseAddress,
				(XOSD_GCD_OFFSET), ColorData[Index]);
		}
	}
	/* For video channel size of 10 or 12, the color size is 64 bits */
	else {
		for (Index = 0;
			Index < (((InstancePtr->Layers[GcIndex].ColorLutSize) *
				((XOSD_DATA_2) * (XOSD_COLOR_ENTRY_SIZE))) /
					(sizeof(u32)));
			Index++) {
				XOsd_WriteReg(InstancePtr->Config.BaseAddress,
					(XOSD_GCD_OFFSET), ColorData[Index]);
		}
	}
	/* Set the active color LUT bank */
	RegValue = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
				(XOSD_GCABA_OFFSET));
	RegValue &= ~((u32)1 << ((XOSD_GCABA_COL_SHIFT) + GcIndex));

	RegValue |= ((u32)BankIndex) << ((XOSD_GCABA_COL_SHIFT) +
			((u32)GcIndex));
	XOsd_WriteReg(InstancePtr->Config.BaseAddress, (XOSD_GCABA_OFFSET),
			RegValue);
}

/*****************************************************************************/
/**
*
* This function loads character set data (font) into an OSD Graphics
* Controller bank.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	GcIndex indicates which Graphics Controller to work on. Valid
*		value range is from 0 to (The Number of Layers) - 1. The
*		layer's type must be set to XOSD_LAYER_TYPE_GPU
*		(defined in xosd_hw.h) for this function to work properly.
* @param	BankIndex indicates which GC bank to be worked on. Valid
*		value range is from 0 to XOSD_GC_BANK_NUM - 1.
* @param	CharSetData points to the character set data to be loaded.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_LoadCharacterSetBank(XOsd *InstancePtr, u8 GcIndex, u8 BankIndex,
				u32 CharSetData[])
{
	u32 RegValue;
	u32 FontWriteNum;
	u32 Index;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid((u16)GcIndex < InstancePtr->Config.LayerNum);
	Xil_AssertVoid(InstancePtr->Layers[GcIndex].LayerType ==
					(XOSD_LAYER_TYPE_GPU));
	Xil_AssertVoid(BankIndex < (XOSD_GC_BANK_NUM));
	Xil_AssertVoid(CharSetData != NULL);

	/* Choose which bank to be loaded */
	RegValue = (((u32)BankIndex) + (XOSD_GCWBA_CHR0)) &
			(XOSD_GCWBA_BANK_MASK);
	RegValue |= (((u32)GcIndex) << (XOSD_GCWBA_GCNUM_SHIFT)) &
					(XOSD_GCWBA_GCNUM_MASK);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress, (XOSD_GCWBA_OFFSET),
			RegValue);

	/* Calculate the number of write to load the whole font data set */
	FontWriteNum = ((u32)(InstancePtr->Layers[GcIndex].FontWidth) *
			(u32)(InstancePtr->Layers[GcIndex].FontHeight) *
			(u32)(InstancePtr->Layers[GcIndex].FontBitsPerPixel));
	FontWriteNum /= (u32)(XOSD_FONT_BIT_TO_BYTE);
	FontWriteNum *= (u32)(InstancePtr->Layers[GcIndex].FontNumChars);
	FontWriteNum /= (u32)sizeof(u32);

	/* Load the font data */
	for (Index = 0x0; Index < FontWriteNum; Index++) {
		XOsd_WriteReg(InstancePtr->Config.BaseAddress, (XOSD_GCD_OFFSET),
						CharSetData[Index]);
	}

	/* Set the bank to be active so the font is used by the core */
	RegValue = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
					(XOSD_GCABA_OFFSET));
	RegValue &= ~((u32)1 << ((XOSD_GCABA_CHR_SHIFT) + GcIndex));
	RegValue |= ((u32)BankIndex) << ((XOSD_GCABA_CHR_SHIFT) +
			((u32)GcIndex));
	XOsd_WriteReg(InstancePtr->Config.BaseAddress, (XOSD_GCABA_OFFSET),
			RegValue);
}

/*****************************************************************************/
/**
*
* This function loads text data into an OSD Graphics Controller bank.
*
* @param	InstancePtr is a pointer to the XOsd instance to be
*		worked on.
* @param	GcIndex indicates which Graphics Controller to work on.
*		Valid value range is from 0 to (The Number of Layers) - 1.
*		The layer's type must be set to XOSD_LAYER_TYPE_GPU
*		(defined in xosd_hw.h) for this	function to work properly.
* @param	BankIndex indicates which GC bank to be worked on. Valid value
*		range is from 0 to XOSD_GC_BANK_NUM - 1.
* @param	TextData points to the text data to be loaded.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_LoadTextBank(XOsd *InstancePtr, u8 GcIndex, u8 BankIndex,
			u32 TextData[])
{
	u32 Index;
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid((u16)GcIndex < InstancePtr->Config.LayerNum);
	Xil_AssertVoid(InstancePtr->Layers[GcIndex].LayerType ==
					(XOSD_LAYER_TYPE_GPU));
	Xil_AssertVoid(BankIndex < (XOSD_GC_BANK_NUM));
	Xil_AssertVoid(TextData != NULL);

	/* Choose which bank to be loaded */
	RegValue = (((u32)BankIndex) + (XOSD_GCWBA_TXT0)) &
			(XOSD_GCWBA_BANK_MASK);
	RegValue |= (((u32)GcIndex) << (XOSD_GCWBA_GCNUM_SHIFT)) &
			(XOSD_GCWBA_GCNUM_MASK);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress, (XOSD_GCWBA_OFFSET),
			RegValue);

	/* Load text data */
	for (Index = 0x0;
		Index < (u32)((((u32)(InstancePtr->
				Layers[GcIndex].TextNumStrings) *
		(u32)(InstancePtr->Layers[GcIndex].TextMaxStringLength)) /
				sizeof(u32)));
		Index++) {
			XOsd_WriteReg(InstancePtr->Config.BaseAddress,
				(XOSD_GCD_OFFSET), TextData[Index]);
	}

	/* Set the active text bank */
	RegValue = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
					(XOSD_GCABA_OFFSET));
	RegValue &= ~(1 << ((XOSD_GCABA_TXT_SHIFT) + GcIndex));

	RegValue |= ((u32)BankIndex) << ((XOSD_GCABA_TXT_SHIFT) +
			((u32)GcIndex));
	XOsd_WriteReg(InstancePtr->Config.BaseAddress, (XOSD_GCABA_OFFSET),
			RegValue);
}

/*****************************************************************************/
/**
*
* This function chooses active banks for a GC in the OSD core.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	GcIndex indicates which Graphics Controller to work on.
*		Valid value range is from 0 to (The Number of Layers) - 1.
*		The layer's type must be set to XOSD_LAYER_TYPE_GPU
*		(defined in xosd_hw.h) for this function to work properly.
* @param	ColorBankIndex indicates the color LUT bank to be choose as
*		active.
* @param	CharBankIndex indicates the character set bank to be chosen as
*		active.
* @param	TextBankIndex indicates the text data bank to be chosen as
*		active.
* @param	InstructionBankIndex indicates the instruction bank to be
*		chosen as active.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_SetActiveBank(XOsd *InstancePtr, u8 GcIndex, u8 ColorBankIndex,
	u8 CharBankIndex, u8 TextBankIndex, u8 InstructionBankIndex)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid((u16)GcIndex < InstancePtr->Config.LayerNum);
	Xil_AssertVoid(InstancePtr->Layers[GcIndex].LayerType ==
					(XOSD_LAYER_TYPE_GPU));
	Xil_AssertVoid(ColorBankIndex < (XOSD_GC_BANK_NUM));
	Xil_AssertVoid(CharBankIndex < (XOSD_GC_BANK_NUM));
	Xil_AssertVoid(TextBankIndex < (XOSD_GC_BANK_NUM));
	Xil_AssertVoid(InstructionBankIndex < (XOSD_GC_BANK_NUM));

	/* Clear the current active bank setting first */
	RegValue = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
					(XOSD_GCABA_OFFSET));
	RegValue &= ~(1 << GcIndex) & (XOSD_GCABA_INS_MASK);
	RegValue &= ~(1 << ((XOSD_GCABA_COL_SHIFT) + (GcIndex))) &
				(XOSD_GCABA_COL_MASK);
	RegValue &= ~(1 << ((XOSD_GCABA_TXT_SHIFT) + (GcIndex))) &
				(XOSD_GCABA_TXT_MASK);
	RegValue &= ~(1 << ((XOSD_GCABA_CHR_SHIFT) + (GcIndex))) &
				(XOSD_GCABA_CHR_MASK);

	/* Choose the active banks */
	RegValue |= (((u32)InstructionBankIndex) << ((u32)GcIndex)) &
				(XOSD_GCABA_INS_MASK);
	RegValue |= (((u32)ColorBankIndex) << ((XOSD_GCABA_COL_SHIFT) +
			(GcIndex))) & (XOSD_GCABA_COL_MASK);
	RegValue |= (((u32)TextBankIndex) << ((XOSD_GCABA_TXT_SHIFT) +
			(GcIndex))) & (XOSD_GCABA_TXT_MASK);
	RegValue |= (((u32)CharBankIndex) << ((XOSD_GCABA_CHR_SHIFT) +
			(GcIndex))) & (XOSD_GCABA_CHR_MASK);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress, XOSD_GCABA_OFFSET,
			RegValue);
}

/*****************************************************************************/
/**
*
* This function creates an instruction for the OSD core.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	InstructionPtr is a pointer to the instruction buffer to be
*		populated with the instruction to be created. The upper level
*		application is responsible for allocating this instruction
*		buffer.
* @param	GcIndex indicates the Graphics Controller that will consume the
*		instruction. Valid value range is from 0 to
*		(The Number of Layers) - 1. The layer's type must be set to
*		XOSD_LAYER_TYPE_GPU (defined in xosd_hw.h) for this function to
*		work properly.
* @param	ObjType indicates the type of object to draw. Use one of
*		XOSD_INS_OPCODE_* constants defined in xosd_hw.h.
* @param	ObjSize indicates line width of boxes and lines and the text
*		scale factor for text boxes.
* @param	XStart indicates the horizontal start pixel of the Object.
* @param	YStart indicates the vertical start line of the Object.
* @param	XEnd indicates the horizontal end pixel of the Object.
* @param	YEnd indicates the vertical end line of the Object.
* @param	TextIndex indicates the string index.
* @param	ColorIndex indicates the color index.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_CreateInstruction(XOsd *InstancePtr, u32 InstructionPtr[],
		u8 GcIndex, u16 ObjType, u8 ObjSize, u16 XStart, u16 YStart,
		u16 XEnd, u16 YEnd, u8 TextIndex, u8 ColorIndex)
{
	u32 Value;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(InstructionPtr != NULL);
	Xil_AssertVoid((u16)GcIndex < InstancePtr->Config.LayerNum);
	Xil_AssertVoid(InstancePtr->Layers[GcIndex].LayerType ==
					(XOSD_LAYER_TYPE_GPU));
	Xil_AssertVoid((ObjType == (XOSD_INS_OPCODE_END)) ||
					(ObjType == (XOSD_INS_OPCODE_NOP)) ||
					(ObjType == (XOSD_INS_OPCODE_BOXTXT)) ||
					(ObjType == (XOSD_INS_OPCODE_LINE)) ||
					(ObjType == (XOSD_INS_OPCODE_TXT)) ||
					(ObjType == (XOSD_INS_OPCODE_BOX)));
	Xil_AssertVoid((u32)XEnd < InstancePtr->ScreenWidth);
	Xil_AssertVoid((u32)YEnd < InstancePtr->ScreenHeight);
	Xil_AssertVoid(TextIndex <
				InstancePtr->Layers[GcIndex].TextNumStrings);
	Xil_AssertVoid(ColorIndex < InstancePtr->Layers[GcIndex].ColorLutSize);

	/* Clear the whole instruction first */
	(void)memset((void *)InstructionPtr, 0, (XOSD_INS_SIZE) *
			(sizeof(u32)));

	/* Populate instruction word 0 fields */
	Value = ((u32)XStart) & (XOSD_INS0_XSTART_MASK);
	Value |= (((u32)XEnd) << (XOSD_INS0_XEND_SHIFT)) &
					(XOSD_INS0_XEND_MASK);
	Value |= (((u32)GcIndex) << (XOSD_INS0_GCNUM_SHIFT)) &
					(XOSD_INS0_GCNUM_MASK);
	Value |= (((u32)ObjType) << (XOSD_INS0_OPCODE_SHIFT)) &
					(XOSD_INS0_OPCODE_MASK);
	InstructionPtr[XOSD_INS0] = Value;

	/* Populate instruction word 1 fields */
	Value = ((u32)TextIndex) & (XOSD_INS1_TXTINDEX_MASK);
	InstructionPtr[XOSD_INS1] = Value;

	/* Populate instruction word 2 fields */
	Value = ((u32)YStart) & (XOSD_INS2_YSTART_MASK);
	Value |= (((u32)YEnd) << (XOSD_INS2_YEND_SHIFT)) &
				(XOSD_INS2_YEND_MASK);
	Value |= (((u32)ObjSize) << (XOSD_INS2_OBJSIZE_SHIFT)) &
				(XOSD_INS2_OBJSIZE_MASK);
	InstructionPtr[XOSD_INS2] = Value;

	/* Populate instruction word 3 fields */
	Value = ((u32)ColorIndex) & (XOSD_INS3_COL_MASK);
	InstructionPtr[XOSD_INS3] = Value;
}

/*****************************************************************************/
/**
*
* This function load an instruction list to be used by an Graphic Controller
* in the OSD core.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	GcIndex indicates which Graphics Controller to work on. Valid
*		value range is from 0 to (The Number of Layers) - 1. The
*		layer's type must be set to XOSD_LAYER_TYPE_GPU
*		(defined in xosd_hw.h) for this function to work properly.
* @param	BankIndex indicates which GC Bank to be worked on. Valid
*		value range is from 0 to XOSD_GC_BANK_NUM.
* @param	InstSetPtr is a pointer to the start of the instruction list
*		to load into the OSD core. The last instruction in the list
*		must has XOSD_INS_OPCODE_END type.
* @param	InstNum indicates the number of the instructions in the list to
*		load. Valid value range is from 1 to the half of the size of
*		the instruction memory created for the Graphic Controller.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XOsd_LoadInstructionList(XOsd *InstancePtr, u8 GcIndex, u8 BankIndex,
				u32 InstSetPtr[], u32 InstNum)
{
	u32 RegValue;
	u32 Index;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid((u16)GcIndex < InstancePtr->Config.LayerNum);
	Xil_AssertVoid(InstancePtr->Layers[GcIndex].LayerType ==
					(XOSD_LAYER_TYPE_GPU));
	Xil_AssertVoid(BankIndex < (XOSD_GC_BANK_NUM));
	Xil_AssertVoid(InstSetPtr != NULL);
	Xil_AssertVoid(InstNum <= InstancePtr->Layers[GcIndex].InstructionNum);

	/* Choose which bank to be loaded */
	RegValue = (((u32)BankIndex) + (XOSD_GCWBA_INS0)) &
				(XOSD_GCWBA_BANK_MASK);
	RegValue |= (((u32)GcIndex) << (XOSD_GCWBA_GCNUM_SHIFT)) &
					(XOSD_GCWBA_GCNUM_MASK);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress, (XOSD_GCWBA_OFFSET),
			RegValue);
	for (Index = 0; Index < (InstNum * (XOSD_INS_SIZE)); Index++) {
		XOsd_WriteReg(InstancePtr->Config.BaseAddress,
				(XOSD_GCD_OFFSET), InstSetPtr[Index]);
	}

	/* Notify OSD this is the end of the instruction list by adding an END
	 * instruction
	 */
	if (InstNum < InstancePtr->Layers[GcIndex].InstructionNum) {
		for (Index = 0x0; Index < (XOSD_INS_SIZE); Index++) {
			XOsd_WriteReg(InstancePtr->Config.BaseAddress,
						(XOSD_GCD_OFFSET), 0x0);
		}
	}

	/* Set the active instruction bank */
	RegValue = XOsd_ReadReg(InstancePtr->Config.BaseAddress,
					(XOSD_GCABA_OFFSET));
	RegValue &= ~(1 << GcIndex);
	RegValue |= ((u32)BankIndex << (u32)GcIndex);
	XOsd_WriteReg(InstancePtr->Config.BaseAddress,
			(XOSD_GCABA_OFFSET), RegValue);
}

/*****************************************************************************/
/*
*
* This function populates the layer array in the XOsd instance with
* the properties of all layers. This is to help fetch the information faster
* later.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
* @param	CfgPtr is a to the configuration structure associated with
*		the OSD core.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void PopulateLayerProperty(XOsd *InstancePtr, XOsd_Config *CfgPtr)
{
	XOsd *IpTemp = NULL;
	XOsd_Config *Cfg = NULL;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CfgPtr != NULL);

	/* Use short variable names to keep the lines in this function
	 * shorter
	 */
	IpTemp = InstancePtr;
	Cfg = CfgPtr;

	/* Layer #0 */
	IpTemp->Layers[0].LayerType	= Cfg->Layer0Type;
	IpTemp->Layers[0].InstructionNum = Cfg->Layer0InstructionMemSize;
	IpTemp->Layers[0].InstructionBoxEnable = Cfg->Layer0InstructionBoxEnable;
	IpTemp->Layers[0].InstructionLineEnable = Cfg->Layer0InstructionLineEnable;
	IpTemp->Layers[0].InstructionTextEnable = Cfg->Layer0InstructionTextEnable;
	IpTemp->Layers[0].ColorLutSize = Cfg->Layer0ColorLutSize;
	IpTemp->Layers[0].ColorLutMemoryType = Cfg->Layer0ColorLutMemoryType;
	IpTemp->Layers[0].FontNumChars = Cfg->Layer0FontNumChars;
	IpTemp->Layers[0].FontWidth = Cfg->Layer0FontWidth;
	IpTemp->Layers[0].FontHeight = Cfg->Layer0FontHeight;
	IpTemp->Layers[0].FontBitsPerPixel = Cfg->Layer0FontBitsPerPixel;
	IpTemp->Layers[0].FontAsciiOffset = Cfg->Layer0FontAsciiOffset;
	IpTemp->Layers[0].TextNumStrings = Cfg->Layer0TextNumStrings;
	IpTemp->Layers[0].TextMaxStringLength = Cfg->Layer0TextMaxStringLength;

	/* Layer #1 */
	IpTemp->Layers[1].LayerType = Cfg->Layer1Type;
	IpTemp->Layers[1].InstructionNum = Cfg->Layer1InstructionMemSize;
	IpTemp->Layers[1].InstructionBoxEnable = Cfg->Layer1InstructionBoxEnable;
	IpTemp->Layers[1].InstructionLineEnable = Cfg->Layer1InstructionLineEnable;
	IpTemp->Layers[1].InstructionTextEnable = Cfg->Layer1InstructionTextEnable;
	IpTemp->Layers[1].ColorLutSize = Cfg->Layer1ColorLutSize;
	IpTemp->Layers[1].ColorLutMemoryType = Cfg->Layer1ColorLutMemoryType;
	IpTemp->Layers[1].FontNumChars = Cfg->Layer1FontNumChars;
	IpTemp->Layers[1].FontWidth	= Cfg->Layer1FontWidth;
	IpTemp->Layers[1].FontHeight = Cfg->Layer1FontHeight;
	IpTemp->Layers[1].FontBitsPerPixel = Cfg->Layer1FontBitsPerPixel;
	IpTemp->Layers[1].FontAsciiOffset = Cfg->Layer1FontAsciiOffset;
	IpTemp->Layers[1].TextNumStrings = Cfg->Layer1TextNumStrings;
	IpTemp->Layers[1].TextMaxStringLength = Cfg->Layer1TextMaxStringLength;

	/* Layer #2 */
	IpTemp->Layers[2].LayerType = Cfg->Layer2Type;
	IpTemp->Layers[2].InstructionNum = Cfg->Layer2InstructionMemSize;
	IpTemp->Layers[2].InstructionBoxEnable = Cfg->Layer2InstructionBoxEnable;
	IpTemp->Layers[2].InstructionLineEnable = Cfg->Layer2InstructionLineEnable;
	IpTemp->Layers[2].InstructionTextEnable = Cfg->Layer2InstructionTextEnable;
	IpTemp->Layers[2].ColorLutSize = Cfg->Layer2ColorLutSize;
	IpTemp->Layers[2].ColorLutMemoryType = Cfg->Layer2ColorLutMemoryType;
	IpTemp->Layers[2].FontNumChars = Cfg->Layer2FontNumChars;
	IpTemp->Layers[2].FontWidth = Cfg->Layer2FontWidth;
	IpTemp->Layers[2].FontHeight = Cfg->Layer2FontHeight;
	IpTemp->Layers[2].FontBitsPerPixel = Cfg->Layer2FontBitsPerPixel;
	IpTemp->Layers[2].FontAsciiOffset = Cfg->Layer2FontAsciiOffset;
	IpTemp->Layers[2].TextNumStrings = Cfg->Layer2TextNumStrings;
	IpTemp->Layers[2].TextMaxStringLength = Cfg->Layer2TextMaxStringLength;

	/* Layer #3 */
	IpTemp->Layers[3].LayerType = Cfg->Layer3Type;
	IpTemp->Layers[3].InstructionNum = Cfg->Layer3InstructionMemSize;
	IpTemp->Layers[3].InstructionBoxEnable = Cfg->Layer3InstructionBoxEnable;
	IpTemp->Layers[3].InstructionLineEnable = Cfg->Layer3InstructionLineEnable;
	IpTemp->Layers[3].InstructionTextEnable = Cfg->Layer3InstructionTextEnable;
	IpTemp->Layers[3].ColorLutSize = Cfg->Layer3ColorLutSize;
	IpTemp->Layers[3].ColorLutMemoryType = Cfg->Layer3ColorLutMemoryType;
	IpTemp->Layers[3].FontNumChars = Cfg->Layer3FontNumChars;
	IpTemp->Layers[3].FontWidth = Cfg->Layer3FontWidth;
	IpTemp->Layers[3].FontHeight = Cfg->Layer3FontHeight;
	IpTemp->Layers[3].FontBitsPerPixel = Cfg->Layer3FontBitsPerPixel;
	IpTemp->Layers[3].FontAsciiOffset = Cfg->Layer3FontAsciiOffset;
	IpTemp->Layers[3].TextNumStrings = Cfg->Layer3TextNumStrings;
	IpTemp->Layers[3].TextMaxStringLength = Cfg->Layer3TextMaxStringLength;

	/* Layer #4 */
	IpTemp->Layers[4].LayerType	= Cfg->Layer4Type;
	IpTemp->Layers[4].InstructionNum = Cfg->Layer4InstructionMemSize;
	IpTemp->Layers[4].InstructionBoxEnable = Cfg->Layer4InstructionBoxEnable;
	IpTemp->Layers[4].InstructionLineEnable = Cfg->Layer4InstructionLineEnable;
	IpTemp->Layers[4].InstructionTextEnable = Cfg->Layer4InstructionTextEnable;
	IpTemp->Layers[4].ColorLutSize = Cfg->Layer4ColorLutSize;
	IpTemp->Layers[4].ColorLutMemoryType = Cfg->Layer4ColorLutMemoryType;
	IpTemp->Layers[4].FontNumChars = Cfg->Layer4FontNumChars;
	IpTemp->Layers[4].FontWidth	= Cfg->Layer4FontWidth;
	IpTemp->Layers[4].FontHeight = Cfg->Layer4FontHeight;
	IpTemp->Layers[4].FontBitsPerPixel = Cfg->Layer4FontBitsPerPixel;
	IpTemp->Layers[4].FontAsciiOffset = Cfg->Layer4FontAsciiOffset;
	IpTemp->Layers[4].TextNumStrings = Cfg->Layer4TextNumStrings;
	IpTemp->Layers[4].TextMaxStringLength = Cfg->Layer4TextMaxStringLength;

	/* Layer #5 */
	IpTemp->Layers[5].LayerType = Cfg->Layer5Type;
	IpTemp->Layers[5].InstructionNum = Cfg->Layer5InstructionMemSize;
	IpTemp->Layers[5].InstructionBoxEnable = Cfg->Layer5InstructionBoxEnable;
	IpTemp->Layers[5].InstructionLineEnable = Cfg->Layer5InstructionLineEnable;
	IpTemp->Layers[5].InstructionTextEnable = Cfg->Layer5InstructionTextEnable;
	IpTemp->Layers[5].ColorLutSize = Cfg->Layer5ColorLutSize;
	IpTemp->Layers[5].ColorLutMemoryType = Cfg->Layer5ColorLutMemoryType;
	IpTemp->Layers[5].FontNumChars = Cfg->Layer5FontNumChars;
	IpTemp->Layers[5].FontWidth = Cfg->Layer5FontWidth;
	IpTemp->Layers[5].FontHeight = Cfg->Layer5FontHeight;
	IpTemp->Layers[5].FontBitsPerPixel = Cfg->Layer5FontBitsPerPixel;
	IpTemp->Layers[5].FontAsciiOffset = Cfg->Layer5FontAsciiOffset;
	IpTemp->Layers[5].TextNumStrings = Cfg->Layer5TextNumStrings;
	IpTemp->Layers[5].TextMaxStringLength = Cfg->Layer5TextMaxStringLength;

	/* Layer #6 */
	IpTemp->Layers[6].LayerType	= Cfg->Layer6Type;
	IpTemp->Layers[6].InstructionNum = Cfg->Layer6InstructionMemSize;
	IpTemp->Layers[6].InstructionBoxEnable = Cfg->Layer6InstructionBoxEnable;
	IpTemp->Layers[6].InstructionLineEnable = Cfg->Layer6InstructionLineEnable;
	IpTemp->Layers[6].InstructionTextEnable = Cfg->Layer6InstructionTextEnable;
	IpTemp->Layers[6].ColorLutSize = Cfg->Layer6ColorLutSize;
	IpTemp->Layers[6].ColorLutMemoryType = Cfg->Layer6ColorLutMemoryType;
	IpTemp->Layers[6].FontNumChars = Cfg->Layer6FontNumChars;
	IpTemp->Layers[6].FontWidth = Cfg->Layer6FontWidth;
	IpTemp->Layers[6].FontHeight = Cfg->Layer6FontHeight;
	IpTemp->Layers[6].FontBitsPerPixel = Cfg->Layer6FontBitsPerPixel;
	IpTemp->Layers[6].FontAsciiOffset = Cfg->Layer6FontAsciiOffset;
	IpTemp->Layers[6].TextNumStrings = Cfg->Layer6TextNumStrings;
	IpTemp->Layers[6].TextMaxStringLength = Cfg->Layer6TextMaxStringLength;

	/* Layer #7 */
	IpTemp->Layers[7].LayerType = Cfg->Layer7Type;
	IpTemp->Layers[7].InstructionNum = Cfg->Layer7InstructionMemSize;
	IpTemp->Layers[7].InstructionBoxEnable = Cfg->Layer7InstructionBoxEnable;
	IpTemp->Layers[7].InstructionLineEnable = Cfg->Layer7InstructionLineEnable;
	IpTemp->Layers[7].InstructionTextEnable = Cfg->Layer7InstructionTextEnable;
	IpTemp->Layers[7].ColorLutSize = Cfg->Layer7ColorLutSize;
	IpTemp->Layers[7].ColorLutMemoryType = Cfg->Layer7ColorLutMemoryType;
	IpTemp->Layers[7].FontNumChars = Cfg->Layer7FontNumChars;
	IpTemp->Layers[7].FontWidth	= Cfg->Layer7FontWidth;
	IpTemp->Layers[7].FontHeight = Cfg->Layer7FontHeight;
	IpTemp->Layers[7].FontBitsPerPixel = Cfg->Layer7FontBitsPerPixel;
	IpTemp->Layers[7].FontAsciiOffset = Cfg->Layer7FontAsciiOffset;
	IpTemp->Layers[7].TextNumStrings = Cfg->Layer7TextNumStrings;
	IpTemp->Layers[7].TextMaxStringLength = Cfg->Layer7TextMaxStringLength;

	/* Convert instruction memory sizes into the maximum numbers of
	 * instructions supported
	 */
	IpTemp->Layers[0].InstructionNum /= (XOSD_INS_MEM_SIZE_TO_NUM);
	IpTemp->Layers[1].InstructionNum /= (XOSD_INS_MEM_SIZE_TO_NUM);
	IpTemp->Layers[2].InstructionNum /= (XOSD_INS_MEM_SIZE_TO_NUM);
	IpTemp->Layers[3].InstructionNum /= (XOSD_INS_MEM_SIZE_TO_NUM);
	IpTemp->Layers[4].InstructionNum /= (XOSD_INS_MEM_SIZE_TO_NUM);
	IpTemp->Layers[5].InstructionNum /= (XOSD_INS_MEM_SIZE_TO_NUM);
	IpTemp->Layers[6].InstructionNum /= (XOSD_INS_MEM_SIZE_TO_NUM);
	IpTemp->Layers[7].InstructionNum /= (XOSD_INS_MEM_SIZE_TO_NUM);
}

/*****************************************************************************/
/**
*
* This function returns the Version of the OSD core.
*
* @param	InstancePtr is a pointer to the XOsd instance to be worked on.
*
* @return	Contents of the Version register.
*
* @note		None.
*
******************************************************************************/
u32 XOsd_GetVersion(XOsd *InstancePtr)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read core version */
	Data = XOsd_ReadReg(InstancePtr->Config.BaseAddress, XOSD_VER_OFFSET);

	return Data;
}

/*****************************************************************************/
/**
*
* This function is a stub for the asynchronous callbacks. The stub is here in
* case the upper layer forgot to set the handlers. On initialization, All
* handlers except error handler are set to this callback. It is considered an
* error for this handler to be invoked.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions and passed back
*		to the upper layer when the callback is invoked.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubCallBack(void *CallBackRef)
{
	(void)CallBackRef;
	Xil_AssertVoidAlways();
}

/*****************************************************************************/
/**
*
* This routine is a stub for the asynchronous error interrupt callback. The
* stub is here in case the upper layer forgot to set the handler. On
* initialization, error interrupt handler is set to this callback. It is
* considered an error for this handler to be invoked.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions and passed back
*		to the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
*		value equals 'OR'ing one or more XOSD_IXR_* values defined
*		in xosd_hw.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask)
{
	(void)CallBackRef;
	(void)ErrorMask;
	Xil_AssertVoidAlways();
}
/** @} */
