/******************************************************************************
* Copyright (c) 2001 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtpg.c
* @addtogroup tpg Overview
* @{
*
* This file contains the implementation of the interface functions for
* Test Pattern Generator (TPG) driver. Refer to the header file xtpg.h for more
* detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00a se     10/01/12 Initial creation.
* 2.0   se     01/24/13 Cleaned up comments.
* 3.0   adk    02/19/14 Changed the file name form "tpg.c" to "xtpg.c".
*                       Implemented the following functions:
*                       XTpg_CfgInitialize, XTpg_Setup, XTpg_GetVersion,
*                       XTpg_SetActiveSize, XTpg_GetActiveSize,
*                       XTpg_SetBackground, XTpg_GetBackground,
*                       XTpg_EnableCrossHair, XTpg_DisableCrossHair,
*                       XTpg_EnableBox, XTpg_DisableBox, XTpg_SetComponentMask,
*                       XTpg_GetComponentMask, XTpg_EnableStuckPixel,
*                       XTpg_DisableStuckPixel, XTPg_EnableNoise,
*                       XTPg_DisableNoise, XTpg_EnableMotion,
*                       XTpg_DisableMotion, XTpg_SetMotionSpeed,
*                       XTpg_GetMotionSpeed, XTpg_SetCrosshairPosition,
*                       XTpg_GetCrosshairPosition, XTpg_SetZPlateHStart,
*                       XTpg_GetZPlateHStart, XTpg_SetZPlateHSpeed,
*                       XTpg_GetZPlateHSpeed, XTpg_SetZPlateVStart,
*                       XTpg_GetZPlateVStart, XTpg_SetZPlateVSpeed,
*                       XTpg_GetZPlateVSpeed, XTpg_SetBoxSize, XTpg_GetBoxSize,
*                       XTpg_SetBoxColor, XTpg_GetBoxColor,
*                       XTpg_SetStuckPixelThreshold,
*                       XTpg_GetStuckPixelThreshold, XTpg_SetNoiseGain,
*                       XTpg_GetNoiseGain, XTpg_SetBayerPhase,
*                       XTpg_GetBayerPhase, XTpg_SetPattern, XTpg_GetPattern.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xtpg.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void StubCallBack(void *CallBackRef);
static void StubErrCallBack(void *CallBackRef, u32 ErrorMask);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes the TPG core. This function must be called
* prior to using the TPG core. Initialization of the TPG includes
* setting up the instance data and ensuring the hardware is in a quiescent
* state.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	CfgPtr is a reference to a structure containing information
*		about a specific XTpg instace.
* @param	EffectiveAddr is the core base address in the virtual memory
*		address space. The caller is responsible for keeping the
*		address mapping from EffectiveAddr to the core physical base
*		address unchanged once this function is invoked. Unexpected
*		errors may occur if the address mapping changes after this
*		function is called. If address translation is not used, pass in
*		the physical address instead.
*
* @return	- XST_SUCCESS if XTpg instance initialization was successful.
*
* @note		None.
*
******************************************************************************/
int XTpg_CfgInitialize(XTpg *InstancePtr, XTpg_Config *CfgPtr,
			u32 EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (u32)0x0);

	/* Setup the instance. */
	(void)memset((void *)InstancePtr, 0, sizeof(XTpg));

	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
			sizeof(XTpg_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/*
	 * Set all handlers to stub values, let user configure this data later.
	 */
	InstancePtr->ProcStartCallBack =
				(XTpg_CallBack)((void *)StubCallBack);
	InstancePtr->FrameDoneCallBack =
				(XTpg_CallBack)((void *)StubCallBack);
	InstancePtr->ErrCallBack =
				(XTpg_ErrorCallBack)((void *)StubErrCallBack);

	/*
	 * Reset the hardware and set the flag to indicate the driver is
	 * ready.
	 */
	XTpg_Reset(InstancePtr);
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function sets the input/output frame size in Active Size register and
* enables the register update.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_Setup(XTpg *InstancePtr)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XTpg_RegUpdateDisable(InstancePtr);

	/* Write Active Size Register. */
	Data = (((InstancePtr->VSize) << (XTPG_ACTSIZE_NUM_LINE_SHIFT)) &
		(XTPG_ACTSIZE_NUM_LINE_MASK)) | (((InstancePtr) -> HSize) &
			(XTPG_ACTSIZE_NUM_PIXEL_MASK));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_ACTIVE_SIZE_OFFSET), Data);
	XTpg_RegUpdateEnable(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function returns the contents of the Version register of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
*
* @return	Content of Version register is returned.
*
* @note		None.
*
******************************************************************************/
u32 XTpg_GetVersion(XTpg *InstancePtr)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read version register of the TPG core. */
	Data = XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_VERSION_OFFSET));

	return Data;
}

/*****************************************************************************/
/**
*
* This function sets the active H/V sizes in the Active Size register.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	HSize specifies the number of Active Pixels per Scan Line
*		that needs to be set (Range is 32 to 7680).
* @param	VSize specifies the number of Active Lines per Frame that
*		needs to be set (Range is 32 to 7680).
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_SetActiveSize(XTpg *InstancePtr, u16 HSize, u16 VSize)
{
	u32 Size;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((VSize >= (u16)(XTPG_VSIZE_FIRST)) &&
			(VSize <= (u16)XTPG_VSIZE_LAST));
	Xil_AssertVoid((HSize >= (u16)XTPG_HSIZE_FIRST) &&
			(HSize <= (u16)XTPG_HSIZE_LAST));

	Size = (((u32)VSize) << ((u32)(XTPG_ACTSIZE_NUM_LINE_SHIFT))) |
								(HSize);
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_ACTIVE_SIZE_OFFSET), Size);
}

/*****************************************************************************/
/**
*
* This function gets the number of Active Pixel per Scan line
* and number of Active Lines per Frame from the Active Size register.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	HSize is a pointer to 16-bit variable in which
*		the number of Active Pixels per Scan Line is returned.
*		(Range is 32 to 7680).
* @param	VSize is a pointer to 16-bit variable in which
*		the number of Active Lines per Frame is returned.
*		(Range is 32 to 7680).
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XTpg_GetActiveSize(XTpg *InstancePtr, u16 *HSize, u16 *VSize)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(HSize != NULL);
	Xil_AssertVoid(VSize != NULL);

	/*
	 * Read the number of active pixels per scan line.
	 */
	*HSize = (u16)(XTpg_ReadReg(InstancePtr->Config.BaseAddress,
			(XTPG_ACTIVE_SIZE_OFFSET))) &
				(XTPG_ACTSIZE_NUM_PIXEL_MASK);

	/*
	 * Reads number of active lines per Frame.
	 */
	*VSize = (u16)(XTpg_ReadReg(InstancePtr->Config.BaseAddress,
			(XTPG_ACTIVE_SIZE_OFFSET)) &
				(XTPG_ACTSIZE_NUM_LINE_MASK)) >>
					(XTPG_ACTSIZE_NUM_LINE_SHIFT);
}

/*****************************************************************************/
/**
*
* This function sets the background pattern in the Pattern Control Register.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	Pattern is the pattern to be generated.
* 		The enum values to be used for the patterns are :
*		- XTPG_PASS_THROUGH-	Pass video input straight through the
*					video output
*		- XTPG_H_RAMP	   -	Horizontal ramp
*		- XTPG_V_RAMP	   -	Vertical ramp
*		- XTPG_R_RAMP	   -	Temporal ramp
*		- XTPG_RED_PLANE   -	Solid red output
*		- XTPG_GREEN_PLANE -	Solid green output
*		- XTPG_BLUE_PLANE  -	Solid blue output
*		- XTPG_BLACK_PLANE -	Solid black output
*		- XTPG_WHITE_PLANE -	Solid white output
*		- XTPG_COLOR_BARS  -	Color bars
*		- XTPG_ZONE_PLATE  -	Zone plate output(sinusoidal)
*		- XTPG_TARAN_BARS  -	Tartan color bars
*		- XTPG_CROSS_HATCH -	Cross hatch pattern
*		- XTPG_HV_RAMP	   -	Horizontal vertical ramp
*		- XTPG_BLACK_AND_WHITE_CHECKERBOARD -
*					Black and white checker board
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_SetBackground(XTpg *InstancePtr, enum XTpg_BackgroundPattern Pattern)
{
	u32 Background;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid( (Pattern == XTPG_BLACK_AND_WHITE_CHECKERBOARD ) ||
		(Pattern <= XTPG_CROSS_HATCH) || ((Pattern == XTPG_HV_RAMP) ));

	Background = ((XTpg_ReadReg(InstancePtr->Config.BaseAddress,
			(XTPG_PATTERN_CONTROL_OFFSET))) &
				(~(XTPG_PTRN_CTL_SET_BG_MASK))) |
					((u32)(Pattern) &
						(XTPG_PTRN_CTL_SET_BG_MASK));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET), Background);
}

/*****************************************************************************/
/**
*
* This function gets the background pattern for TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	Background pattern is returned.
*		The enum values for corresponding Background pattern value is
*		- 0x0	XTPG_PASS_THROUGH-	Pass video input straight
*						video output
*		- 0x1	XTPG_H_RAMP	 -	Horizontal ramp
*		- 0x2	XTPG_V_RAMP	 -	Vertical ramp
*		- 0x3	XTPG_R_RAMP	 -	Temporal ramp
*		- 0x4	XTPG_RED_PLANE	 -	Solid red output
*		- 0x5	XTPG_GREEN_PLANE -	Solid green output
*		- 0x6	XTPG_BLUE_PLANE	 -	Solid blue output
*		- 0x7	XTPG_BLACK_PLANE -	Solid black output
*		- 0x8	XTPG_WHITE_PLANE -	Solid white output
*		- 0x9	XTPG_COLOR_BARS	 -	Color bars
*		- 0xA	XTPG_ZONE_PLATE	 -	Zone plate output (sinusoidal)
*		- 0xB	XTPG_TARAN_BARS	 -	Tartan color bars
*		- 0xC	XTPG_CROSS_HATCH -	Cross hatch pattern
*		- 0xE	XTPG_HV_RAMP	 -	Horizontal vertical ramp
*		- 0xF	XTPG_BLACK_AND_WHITE_CHECKERBOARD -
*						Black and white checker board
*
* @note		None.
*
******************************************************************************/
u32 XTpg_GetBackground(XTpg *InstancePtr)
{
	u32 Background;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Background = (XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET)) &
					(XTPG_PTRN_CTL_SET_BG_MASK));

	return Background;
}

/*****************************************************************************/
/**
*
* This function enables the drawing of Cross Hairs feature in the Pattern
* Control Register of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	None.
*
* @note		To draw the Cross Hairs, the position of drawing must be set
*		by using XTpg_SetCrosshairPosition.
*
******************************************************************************/
void XTpg_EnableCrossHair(XTpg *InstancePtr)
{
	u32 EnableCrossHair;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	EnableCrossHair = ((XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET))) |
					(XTPG_PTRN_CTL_EN_CROSSHAIR_MASK));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET), EnableCrossHair);
}

/*****************************************************************************/
/**
*
* This function disables the drawing of Cross Hairs feature in the Pattern
* Control Register of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_DisableCrossHair(XTpg *InstancePtr)
{
	u32 DisableCrossHair;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	DisableCrossHair = ((XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET))) &
					(~(XTPG_PTRN_CTL_EN_CROSSHAIR_MASK)));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_PATTERN_CONTROL_OFFSET), DisableCrossHair);
}

/*****************************************************************************/
/**
*
* This function enables the Moving Box feature in the Pattern Control Register
* of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	None.
*
* @note
*		- XTpg_SetBoxSize API should be used for setting box size.
*		- XTpg_SetBoxColor API should be used for setting box color.
*
******************************************************************************/
void XTpg_EnableBox(XTpg *InstancePtr)
{
	u32 EnableBox;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	EnableBox = ((XTpg_ReadReg(InstancePtr->Config.BaseAddress,
			(XTPG_PATTERN_CONTROL_OFFSET))) |
				(XTPG_PTRN_CTL_EN_BOX_MASK));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET), EnableBox);
}

/*****************************************************************************/
/**
*
* This function disables the Moving Box feature in the Pattern Control Register
* of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_DisableBox(XTpg *InstancePtr)
{
	u32 DisableBox;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	DisableBox = ((XTpg_ReadReg(InstancePtr->Config.BaseAddress,
			(XTPG_PATTERN_CONTROL_OFFSET))) &
				(~(XTPG_PTRN_CTL_EN_BOX_MASK)));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET), DisableBox);
}

/*****************************************************************************/
/**
*
* This function masks out the specified color component by setting the
* Pattern Control Register of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	Mask specifies the ComponentMask that needs to be set.
*		the enum values as
*		- XTPG_NOMASK,		- No masking.
*		- XTPG_MASKOUT_RED,	- Mask out red,
*					  Cr(for YCbCr mode) component.
*		- XTPG_MASKOUT_GREEN,	- Mask out green,
*					  Y(for YCbCr mode) component.
*		- XTPG_MASKOUT_BLUE	- Mask out blue,
*					  Cb(for YCbCr mode) component.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_SetComponentMask(XTpg *InstancePtr, enum XTpg_ComponentMask Mask)
{
	u32 ColorComponent;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Mask == XTPG_NOMASK) || (Mask == XTPG_MASKOUT_GREEN) ||
		(Mask == XTPG_MASKOUT_GREEN) || (Mask == XTPG_MASKOUT_BLUE));

	ColorComponent = ((XTpg_ReadReg(InstancePtr->Config.BaseAddress,
		(XTPG_PATTERN_CONTROL_OFFSET))) &
			(~(XTPG_PTRN_CTL_MASK_COLR_COMP_MASK))) |
			(((u32)(Mask) <<
				(XTPG_PTRN_CTL_MASK_COLR_COMP_SHIFT)) &
				(XTPG_PTRN_CTL_MASK_COLR_COMP_MASK));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET), ColorComponent);
}

/*****************************************************************************/
/**
*
* This function returns the color component mask status in the Pattern Control
* Register.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	Color component value is returned.
* 		Corresponding enum values for Color component value is
*		- 0x0	XTPG_NOMASK	 - No masking.
*		- 0x1	XTPG_MASKOUT_RED - Mask out red,
*					   Cr(for YCbCr mode) component.
*		- 0x2	XTPG_MASKOUT_GREE- Mask out green,N
*					   Y(for YCbCr mode) component.
*		- 0x4	XTPG_MASKOUT_BLUE- Mask out blue,
*					   Cb(for YCbCr mode) component.
*
* @note		None.
*
******************************************************************************/
u32 XTpg_GetComponentMask(XTpg *InstancePtr)
{
	u32 ColorComponent;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	ColorComponent = (XTpg_ReadReg(InstancePtr->Config.BaseAddress,
					(XTPG_PATTERN_CONTROL_OFFSET)) &
					(XTPG_PTRN_CTL_MASK_COLR_COMP_MASK)) >>
					(XTPG_PTRN_CTL_MASK_COLR_COMP_SHIFT);

	return ColorComponent;
}

/*****************************************************************************/
/**
*
* This function enables the Stuck Pixel Feature in the Pattern Control Register
* of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	None.
*
* @note		XTpg_SetStuckPixelThreshold API should be used to set Stuck
*		Pixel threshold value.
*
******************************************************************************/
void XTpg_EnableStuckPixel(XTpg *InstancePtr)
{
	u32 EnableStuckPixel;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	EnableStuckPixel = ((XTpg_ReadReg(InstancePtr->Config.BaseAddress,
			(XTPG_PATTERN_CONTROL_OFFSET))) |
				(XTPG_PTRN_CTL_EN_STUCK_MASK));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET),
					EnableStuckPixel);
}

/*****************************************************************************/
/**
*
* This function disables the Stuck Pixel feature in the Pattern Control
* register of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_DisableStuckPixel(XTpg *InstancePtr)
{
	u32 DisableStuckPixel;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	DisableStuckPixel = ((XTpg_ReadReg(InstancePtr->Config.BaseAddress,
			(XTPG_PATTERN_CONTROL_OFFSET))) &
				(~(XTPG_PTRN_CTL_EN_STUCK_MASK)));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_PATTERN_CONTROL_OFFSET), DisableStuckPixel);
}

/*****************************************************************************/
/**
*
* This function enables the Noise on the output by setting value in Pattern
* Control register of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	None.
*
* @note		XTpg_SetNoiseGain API should be used to increase or decrease
*		the noise produced on the output.
*
******************************************************************************/
void XTPg_EnableNoise(XTpg *InstancePtr)
{
	u32 EnableNoise;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	EnableNoise = ((XTpg_ReadReg(InstancePtr->Config.BaseAddress,
			(XTPG_PATTERN_CONTROL_OFFSET))) |
				(XTPG_PTRN_CTL_EN_NOISE_MASK));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET), EnableNoise);
}

/*****************************************************************************/
/**
*
* This function disables the Noise on the output by disabling this feature in
* Pattern Control register of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTPg_DisableNoise(XTpg *InstancePtr)
{
	u32 DisableNoise;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	DisableNoise = ((XTpg_ReadReg(InstancePtr->Config.BaseAddress,
			(XTPG_PATTERN_CONTROL_OFFSET))) &
				(~(XTPG_PTRN_CTL_EN_NOISE_MASK)));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET), DisableNoise);
}

/*****************************************************************************/
/**
*
* This function enables the Motion Feature in the Pattern Control register
* of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_EnableMotion(XTpg *InstancePtr)
{
	u32 EnableMotion;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	EnableMotion = ((XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET))) |
					(XTPG_PTRN_CTL_EN_MOTION_MASK));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET), EnableMotion);
}

/*****************************************************************************/
/**
*
* This function disables the Motion feature in the Pattern Control register
* of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_DisableMotion(XTpg *InstancePtr)
{
	u32 DisableMotion;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	DisableMotion = ((XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET))) &
					(~(XTPG_PTRN_CTL_EN_MOTION_MASK)));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET), DisableMotion);
}

/*****************************************************************************/
/**
*
* This function sets the Motion Speed.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	MotionSpeed is how fast the temporal features of supported
*		test pattern changes from frame to frame which need to be set.
*		Range is 0 to 255.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_SetMotionSpeed(XTpg *InstancePtr, u32 MotionSpeed)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((MotionSpeed) <= (u32)(XTPG_MOTION_SPEED_MAX));

	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_MOTION_SPEED_OFFSET), MotionSpeed);
}

/*****************************************************************************/
/**
*
* This function gets the Motion Speed.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	MotionSpeed, which indicates the speed that affects on test
*		pattern changes from frame to frame is returned.
*
* @note		None.
*
******************************************************************************/
u32 XTpg_GetMotionSpeed(XTpg *InstancePtr)
{
	u32 MotionSpeed;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	MotionSpeed = (XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_MOTION_SPEED_OFFSET)) &
					(XTPG_MOTION_SPEED_MASK));

	return MotionSpeed;
}

/*****************************************************************************/
/**
*
* This function sets the Cross Hairs Positions.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	HPos is the row of the frame that will have horizontal line of
* 		Cross Hairs that need to be set.
* @param	VPos is the column of the frame that will have vertical line
* 		of Cross Hairs that need to be set.
*
* @return	None.
*
* @note		Cross hair feature should be enabled by using the API
*		XTpg_EnableCrossHair
*
******************************************************************************/
void XTpg_SetCrosshairPosition(XTpg *InstancePtr, u16 HPos, u16 VPos)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	Data = (VPos << (u32)(XTPG_CROSSHAIR_SHIFT)) | ((u32)HPos);
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_CROSS_HAIRS_OFFSET), Data);
}

/*****************************************************************************/
/**
*
* This function gets the cross hair positions.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	HPos is the row of the frame that will have horizontal line of
* 		Cross Hairs is returned.
* @param	VPos is the column of the frame that will have vertical line
* 		of Cross Hairs is returned.
*
* @return	None.
*
******************************************************************************/
void XTpg_GetCrosshairPosition(XTpg *InstancePtr, u16 *HPos, u16 *VPos)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(HPos != NULL);
	Xil_AssertVoid(VPos != NULL);

	Data = XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_CROSS_HAIRS_OFFSET));
	*HPos = (u16)((Data) & ((u32)(XTPG_CROSSHAIR_HPOS_MASK)));
	*VPos = (u16)((((Data) & ((u32)(XTPG_CROSSHAIR_VPOS_MASK)))) >>
					(XTPG_CROSSHAIR_SHIFT));
}

/*****************************************************************************/
/**
*
* This function sets a starting point in the ROM based sinusoidal values for
* the horizontal component.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	ZPlateHStart is starting point in the ROM based sinusoidal
*		value for horizontal component.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_SetZPlateHStart(XTpg *InstancePtr, u16 ZPlateHStart)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_ZPLATE_HOR_CONTROL_OFFSET));
	Data = (Data) &	(~(XTPG_ZPLATEHOR_START_MASK));
	Data = (Data) | (u32)(ZPlateHStart);
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_ZPLATE_HOR_CONTROL_OFFSET), Data);
}

/*****************************************************************************/
/**
*
* This function gets a starting point in the ROM based sinusoidal values
* for the horizontal component.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	ZPlateHStart is starting point in the ROM based sinusoidal
*		value for horizontal component is returned.
*
* @note		None.
*
******************************************************************************/
u16 XTpg_GetZPlateHStart (XTpg *InstancePtr)
{
	u32 ZPlateHStart;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	ZPlateHStart = (XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_ZPLATE_HOR_CONTROL_OFFSET)) &
					(XTPG_ZPLATEHOR_START_MASK));

	return (u16)ZPlateHStart;
}

/*****************************************************************************/
/**
*
* This function sets how fast (the speed of) the horizontal component changes.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	ZPlateHSpeed is the speed of the horizontal component changes
* 		which need to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_SetZPlateHSpeed(XTpg *InstancePtr, u16 ZPlateHSpeed)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* MSB indicates the speed of the horizontal component changes. */
	Data = XTpg_ReadReg(InstancePtr->Config.BaseAddress,
			(XTPG_ZPLATE_HOR_CONTROL_OFFSET));
	Data = (Data) & (~(XTPG_ZPLATEHOR_SPEED_MASK));
	Data = (Data) | (((u32)ZPlateHSpeed) << (XTPG_ZPLATEHOR_SPEED_SHIFT));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_ZPLATE_HOR_CONTROL_OFFSET), Data);
}

/*****************************************************************************/
/**
*
* This function gets how fast (the speed of) horizontal component changes.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	ZPlateHSpeed is speed of the horizontal component changes is
*		returned.
*
* @note		None.
*
******************************************************************************/
u16 XTpg_GetZPlateHSpeed(XTpg *InstancePtr)
{
	u32 ZPlateHSpeed;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	ZPlateHSpeed = XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_ZPLATE_HOR_CONTROL_OFFSET));
	ZPlateHSpeed = ((ZPlateHSpeed) & ((XTPG_ZPLATEHOR_SPEED_MASK))) >>
				(XTPG_ZPLATEHOR_SPEED_SHIFT);

	return (u16)ZPlateHSpeed;
}

/*****************************************************************************/
/**
*
* This function sets a starting point in the ROM based sinusoidal values for
* the vertical component.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	ZPlateVStart is starting point in the ROM based sinusoidal
*		value for vertical component which need to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_SetZPlateVStart(XTpg *InstancePtr, u16 ZPlateVStart)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_ZPLATE_VER_CONTROL_OFFSET));
	Data = (Data) &	(~(XTPG_ZPLATEVER_START_MASK));
	Data = (Data) | (ZPlateVStart);

	/*
	 * Sets a starting point in the ROM based sinusoidal values for
	 * the vertical component
	 */
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_ZPLATE_VER_CONTROL_OFFSET), Data);
}

/*****************************************************************************/
/**
*
* This function gets a starting point in the ROM based sinusoidal values for
* the vertical component.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	ZPlateVStart is starting point in the ROM based sinusoidal
*		value for vertical component is returned.
*
* @note		None.
*
******************************************************************************/
u16 XTpg_GetZPlateVStart (XTpg *InstancePtr)
{
	u32 ZPlateVStart;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	ZPlateVStart = (XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_ZPLATE_VER_CONTROL_OFFSET)) &
					(XTPG_ZPLATEVER_START_MASK));

	return (u16)ZPlateVStart;
}

/*****************************************************************************/
/**
*
* This function sets how fast (the speed of) the vertical component changes.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	ZPlateVSpeed is the speed of the vertical component changes
*		which need to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_SetZPlateVSpeed(XTpg *InstancePtr, u16 ZPlateVSpeed)
{
	u32 Data;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Manipulates the speed of the vertical component changes. */
	Data = XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_ZPLATE_VER_CONTROL_OFFSET));
	Data = (Data) &	(~(XTPG_ZPLATEVER_START_MASK));
	Data = (Data) | (ZPlateVSpeed);
	Data = (Data) | (((u32)ZPlateVSpeed) << (XTPG_ZPLATEVER_SPEED_SHIFT));
	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_ZPLATE_VER_CONTROL_OFFSET), Data);
}

/*****************************************************************************/
/**
*
* This function gets how fast (the speed of) the vertical component changes.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	ZPlateVSpeed is a 16 bit variable the speed of the vertical
*		component changes is returned.
*
* @note		None.
*
******************************************************************************/
u16 XTpg_GetZPlateVSpeed(XTpg *InstancePtr)
{
	u32 ZPlateVSpeed;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	ZPlateVSpeed = XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_ZPLATE_VER_CONTROL_OFFSET));
	ZPlateVSpeed = ((ZPlateVSpeed) & ((XTPG_ZPLATEVER_SPEED_MASK))) >>
					(XTPG_ZPLATEVER_SPEED_SHIFT);

	return (u16)ZPlateVSpeed;
}

/*****************************************************************************/
/**
*
* This function sets the Box Size.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	BoxSize is size of the box in pixel.
*
* @return
*		-XST_SUCCESS if BoxSize is less than active size.
*		-XST_FAILURE if BoxSize is greater than active size.
*
* @note
*		- The size of the box has to be set smaller than the frame size
*		  that is set in the ACTIVE_SIZE register.
*		- XTpg_EnableBox API should be used to enable the Box feature.
*
******************************************************************************/
int XTpg_SetBoxSize(XTpg *InstancePtr, u32 BoxSize)
{
	u32 Data;
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * The size of the box has to be set smaller than the frame size that
	 * is set in the ACTIVE_SIZE register.
	 */
	Data = XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_ACTIVE_SIZE_OFFSET));

	/* Input Size always less than active size (Data) */
	if (BoxSize < Data) {
		XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_BOX_SIZE_OFFSET), BoxSize);
		Status = (XST_SUCCESS);
	}
	else {
		Status = (XST_FAILURE);
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function gets the Box Size in pixel.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	BoxSize is the size of a box in pixel.
*
* @note		None.
*
******************************************************************************/
u32 XTpg_GetBoxSize(XTpg *InstancePtr)
{
	u32 BoxSize;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	BoxSize = (XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_BOX_SIZE_OFFSET)) &
					(XTPG_BOX_SIZE_MASK));

	return BoxSize;
}

/*****************************************************************************/
/**
*
* This function sets the color components of the box in the Box Color register
* of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	Blue is the color component of the box which needs to be set.
* @param	Green is the color component of the box which needs to be set.
* @param	Red is the color component of the box which needs to be set.
*
* @return	None.
*
* @note		XTpg_EnableBox API should be used to enable the Box feature.
*
******************************************************************************/
void XTpg_SetBoxColor(XTpg *InstancePtr, u16 Blue, u16 Green, u16 Red)
{
	u32 Data;
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	Data = ((Blue & XTPG_BOXCOL_BLUE_MASK) |
		((Green << XTPG_BOXCOL_GREEN_SHIFT) & XTPG_BOXCOL_GREEN_MASK)) |
		((Red << XTPG_BOXCOL_RED_SHIFT) & XTPG_BOXCOL_RED_MASK);
	XTpg_WriteReg(InstancePtr->Config.BaseAddress, (XTPG_BOX_COLOR_OFFSET),
						Data);
}

/*****************************************************************************/
/**
*
* This function gets the color components of the box in the Box Color register
* of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	Blue is a pointer to 16 bit color component of the box in
*		which blue or Y (for YCbCr mode) is returned.
* @param	Green is a pointer to 16 bit color component of the box in
*		which Green or Cr (for YCbCr mode) is returned.
* @param	Red is a pointer to 16 bit color component of the box in
*		which Red or Cb (for YCbCr mode) is returned.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_GetBoxColor(XTpg *InstancePtr, u16 *Blue, u16 *Green, u16 *Red)
{
	u32 Data;
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	Data = XTpg_ReadReg(InstancePtr->Config.BaseAddress,
					(XTPG_BOX_COLOR_OFFSET));
	*Blue = (Data & (XTPG_BOXCOL_BLUE_MASK));
	*Green = (Data & (XTPG_BOXCOL_GREEN_MASK)) >> XTPG_BOXCOL_GREEN_SHIFT;
	*Red = (Data & (XTPG_BOXCOL_RED_MASK)) >> XTPG_BOXCOL_RED_SHIFT;

}

/*****************************************************************************/
/**
*
* This function sets the stuck pixel threshold in STUCK_PIXEL_THRESH register
* of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	PixelThreshold is an upper limit for PRNG for insertion of
*		stuck pixel which needs to be set.
*
* @return	None.
*
* @note		- XTpg_EnableStuckPixel API should be used to enable Stuck
*		Pixel threshold value.
*
******************************************************************************/
void XTpg_SetStuckPixelThreshold(XTpg *InstancePtr, u32 PixelThreshold)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_STUCK_PIXEL_THRESH_OFFSET),PixelThreshold);
}

/*****************************************************************************/
/**
*
* This function gets the stuck pixel threshold in the STUCK_PIXEL_THRESH
* register of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	Stuck pixel threshold is an upper limit for PRNG for insertion
*		of stuck pixel is returned.
*
* @note		None.
*
******************************************************************************/
u32 XTpg_GetStuckPixelThreshold(XTpg *InstancePtr)
{
	u32 PixelThreshold;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	PixelThreshold = (XTpg_ReadReg(InstancePtr->Config.BaseAddress,
					(XTPG_STUCK_PIXEL_THRESH_OFFSET)) &
						(XTPG_STUCKPIX_THRESH_MASK));

	return PixelThreshold;
}

/*****************************************************************************/
/**
*
* This function sets the noise gain in Noise Gain register of the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	NoiseGain is a value to increase the noise added to
*		each component which needs to be set.
*
* @return	None.
*
* @note		- XTPg_EnableNoise API should be used to enable Noise feature.
*
******************************************************************************/
void XTpg_SetNoiseGain(XTpg *InstancePtr, u32 NoiseGain)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_NOISE_GAIN_OFFSET), NoiseGain);
}

/*****************************************************************************/
/**
*
* This function gets the noise gain of each component of TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	NoiseGain is a value to increase the noise added to each
*		component.
*
* @note		None.
*
******************************************************************************/
u32 XTpg_GetNoiseGain(XTpg *InstancePtr)
{
	u32 NoiseGain;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	NoiseGain = XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_NOISE_GAIN_OFFSET));
	NoiseGain = (NoiseGain) & (XTPG_NOISE_GAIN_MASK);

	return NoiseGain;
}

/*****************************************************************************/
/**
*
* This function specifies or sets whether the starting position pixel(0,0) of
* the Bayer sampling grid is on a red-green or blue-green line and whether the
* first pixel is green or not.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	BayerPhaseComb is Bayer phase value to be set to identify
*		starting pixel position.
*		The enum values of Bayer phase are
*		- Bayer Phase 0 - XTPG_BAYER_PHASE_RGRG.
*		- Bayer Phase 1 - XTPG_BAYER_PHASE_GRGR.
*		- Bayer Phase 2 - XTPG_BAYER_PHASE_GBGB.
*		- Bayer Phase 3 - XTPG_BAYER_PHASE_BGBG.
*		- Bayer Phase 4 - XTPG_BAYER_PHASE_TURNOFF.
*		- It is a double buffered register.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XTpg_SetBayerPhase(XTpg *InstancePtr,
		enum XTpg_BayerPhaseCombination BayerPhaseComb)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(BayerPhaseComb <= XTPG_BAYER_PHASE_TURNOFF);

	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_BAYER_PHASE_OFFSET), (u32)BayerPhaseComb);
}

/*****************************************************************************/
/**
*
* This function returns the Bayer phase value which indicates starting pixel
* position.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	BayerPhase is the starting pixel indicator is returned.
*		Corresponding enum values for Bayer Phase are:
*		- Bayer Phase 0 - XTPG_BAYER_PHASE_RGRG.
*		- Bayer Phase 1 - XTPG_BAYER_PHASE_GRGR.
*		- Bayer Phase 2 - XTPG_BAYER_PHASE_GBGB.
*		- Bayer Phase 3 - XTPG_BAYER_PHASE_BGBG.
*		- Bayer Phase 4 - XTPG_BAYER_PHASE_TURNOFF.
*		- It is a double buffered register.
*
* @note		None.
*
******************************************************************************/
u32 XTpg_GetBayerPhase(XTpg *InstancePtr)
{
	u32 BayerPhaseTemp;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	BayerPhaseTemp = (XTpg_ReadReg(InstancePtr->Config.BaseAddress,
					(XTPG_BAYER_PHASE_OFFSET)) &
						(XTPG_BAYER_PHASE_MASK));

	return BayerPhaseTemp;
}


/*****************************************************************************/
/**
*
* This function sets the Pattern Control Register of TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance.
* @param	Pattern is the 32 bit value to be written to the Pattern
*		Control Register.
*
* @return	None.
*
* @note		This API is used for writing the complete Pattern Control
*		Register.
*		Use the following APIs for the individual patterns separately
*		- XTpg_SetBackground
*		- XTpg_EnableCrossHair
*		- XTpg_DisableCrossHair
*		- XTpg_EnableBox
*		- XTpg_DisableBox
*		- XTpg_SetComponentMask
*		- XTpg_EnableStuckPixel
*		- XTpg_DisableStuckPixel
*		- XTPg_EnableNoise
*		- XTPg_DisableNoise
*		- XTpg_EnableMotion
*		- XTpg_DisableMotion
*
******************************************************************************/
void XTpg_SetPattern(XTpg *InstancePtr, u32 Pattern)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XTpg_WriteReg(InstancePtr->Config.BaseAddress,
			(XTPG_PATTERN_CONTROL_OFFSET), Pattern);
}

/*****************************************************************************/
/**
*
* This function returns the contents of the Pattern Control Register.
*
* @param	InstancePtr is a pointer to the XTpg instance.
*
* @return	Contents of the Pattern Control Register is returned.
*
* @note		None.
*
******************************************************************************/
u32 XTpg_GetPattern(XTpg *InstancePtr)
{
	u32 Pattern;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Pattern = XTpg_ReadReg(InstancePtr->Config.BaseAddress,
				(XTPG_PATTERN_CONTROL_OFFSET));

	return Pattern;
}


/*****************************************************************************/
/**
*
* This routine is a stub for the asynchronous callbacks. The stub is here in
* case the upper layer forgot to set the handlers. On initialization, All
* handlers except error handler are set to this callback. It is considered an
* error for this handler to be invoked.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubCallBack(void *CallBackRef)
{
	(void)CallBackRef;
	/* Verify arguments. */
	Xil_AssertVoidAlways();
}

/*****************************************************************************/
/**
*
* This routine is a stub for the asynchronous error interrupt callback. The
* stub is here in case the upper layer forgot to set the handler. On
* initialization, Error interrupt handler is set to this callback. It is
* considered an error for this handler to be invoked.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
* 		value equals 'OR'ing one or more XTPG_IXR_*_MASK values defined
* 		in xtpg_hw.h.
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
	/* Verify arguments. */
	Xil_AssertVoidAlways();
}

/** @} */
