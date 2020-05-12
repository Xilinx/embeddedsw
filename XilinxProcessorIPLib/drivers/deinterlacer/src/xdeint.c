/******************************************************************************
* Copyright (c) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdeint.c
* @addtogroup deinterlacer_v3_3
* @{
*
* This is main code of Xilinx Vide Deinterlacer core.
* Please see xdeint.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------
* 1.00a rjh  07/10/11 First release
* 2.00a rjh  18/01/12 Updated for v_deinterlacer 2.00
* 3.2   adk  02/13/14 Changed the prototype of XDeint_GetVersion
*                     Implemented the following functions:
*                     XDeint_GetFramestore
*                     XDeint_GetVideo
*                     XDeint_GetThresholds
*                     XDeint_GetPulldown
*                     XDeint_GetSize
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdeint.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void StubCallBack(void *CallBackRef);

/************************** Function Definition ******************************/

/*****************************************************************************/
/**
*
* This function initializes the Deinterlacer core. This function must be
* called prior to using a Deinterlacer core. Initialization of the
* Deinterlacer includes setting up the instance data, and ensuring the
* hardware is in a quiescent state.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	CfgPtr points to the configuration structure associated with
* 		the Deinterlacer core.
* @param	EffectiveAddr is the base address of the core. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*
* @note		None.
*
******************************************************************************/
int XDeint_ConfigInitialize(XDeint *InstancePtr, XDeint_Config *CfgPtr,
							u32 EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (u32)0x0);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XDeint));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
		sizeof(XDeint_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/*
	 * Set all handlers to stub values, let user configure this data later
	 */
	InstancePtr->IntCallBack = (XDeint_CallBack)((void *)StubCallBack);

	/*
	 * Reset the hardware and set the flag to indicate the driver is ready
	 */
	XDeint_Disable(InstancePtr);

	/* Reset the Deinterlacer */
	XDeint_Reset(InstancePtr);

	/* Wait for Soft reset to complete */
	while(XDeint_InReset(InstancePtr));

	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function sets the input field buffer addresses of the Deinterlacer core.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	FieldAddr1 is the address of the 1st input field buffer.
* @param	FieldAddr2 is the address of the 2nd input field buffer.
* @param	FieldAddr3 is the address of the 3rd input field buffer.
* @param	FrameSize  is the size in 32bit words of a single field buffer.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDeint_SetFramestore(XDeint *InstancePtr,
		u32 FieldAddr1, u32 FieldAddr2,
		u32 FieldAddr3, u32 FrameSize)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(FrameSize != (u32)0x0);
	Xil_AssertVoid(FieldAddr1 != (u32)0x0);
	Xil_AssertVoid(FieldAddr2 != (u32)0x0);
	Xil_AssertVoid(FieldAddr3 != (u32)0x0);

	/* Set the input buffer addresses and size of all field stores. */
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress,
					(XDEINT_BUFFER0_OFFSET), FieldAddr1);
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress,
					(XDEINT_BUFFER1_OFFSET), FieldAddr2);
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress,
					(XDEINT_BUFFER2_OFFSET), FieldAddr3);
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress,
					(XDEINT_BUFSIZE_OFFSET), FrameSize);
}

/*****************************************************************************/
/**
*
* This function sets the video format of the Deinterlacer core.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	Packing selects the XSVI video packing mode.
* 		- 0 4:2:0 Packging is used.
* 		- XDEINT_MODE_PACKING_0 4:2:2 Packging is used.
* 		- XDEINT_MODE_PACKING_1 4:4:4 Packging is used.
* @param	Color selects what color space to use.
* 		- 0 YUV color space is used.
* 		- XDEINT_MODE_COL RGB color space is used.
* @param	Order selects which field ordering is being used.
* 		- 0 the field order maps to PAL/HD/3G.
* 		- XDEINT_MODE_FIELD_ORDER the field order maps to NTSC/480i
* @param	PSF enables psf (progressive segmented frame mode).
* 		- XDEINT_MODE_PSF_ENABLE for psf enable.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDeint_SetVideo(XDeint *InstancePtr,
			u32 Packing, u32 Color, u32 Order, u32 PSF)
{
	u32 ModeReg;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));

	/* Read modify write the mode register. */
	ModeReg = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,
					(XDEINT_MODE_OFFSET));
	ModeReg &= ~((XDEINT_MODE_PACKING_MASK) | (XDEINT_MODE_COL_MASK) |
				(XDEINT_MODE_FIELD_ORDER_MASK) |
					(XDEINT_MODE_PSF_ENABLE_MASK));
	ModeReg |= Packing | Color | Order | PSF;
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress,
					(XDEINT_MODE_OFFSET), ModeReg);
}

/*****************************************************************************/
/**
*
* This function sets the threshold used by the motion adaptive kernel of the
* Deinterlacer core.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	ThresholdT1 is the lower threshold of the motion kernel.
* @param	ThresholdT2 is the upper threshold of the motion kernel.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDeint_SetThresholds(XDeint *InstancePtr, u32 ThresholdT1,
						u32 ThresholdT2)
{
	u32 XFade;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(ThresholdT1 != (u32)0x0);
	Xil_AssertVoid(ThresholdT2 != (u32)0x0);

	/* Determine the T1->T2 cross fade setting. */
	XFade = (u32)((XDEINT_FADE_RATIO)/(ThresholdT2 - ThresholdT1));
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress,
					(XDEINT_THRESH1_OFFSET), ThresholdT1);
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress,
					(XDEINT_THRESH2_OFFSET), ThresholdT2);
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress,
					(XDEINT_XFADE_OFFSET), XFade);
}

/*****************************************************************************/
/**
*
* This function sets the pull down controller of the Deinterlacer core.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	Enable_32 allows detectors to automatically control
*		Deinterlacer core.
*		- 1 Pull down enable 3:2
* @param	Enable_22 allows detectors to automatically control
*		Deinterlacer core.
*		- 1 Pull down enable 2:2
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDeint_SetPulldown(XDeint *InstancePtr, u32 Enable_32,
						u32 Enable_22)
{
	u32 ModeReg;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady ==
					(u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid((Enable_32 == (u32)0x00) || (Enable_32 == (u32)0x01));
	Xil_AssertVoid((Enable_22 == (u32)0x00) || (Enable_22 == (u32)0x01));

	/* Read modify write the mode register.*/
	ModeReg = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,
						(XDEINT_MODE_OFFSET));
	ModeReg &= ~(((u32)(XDEINT_MODE_PULL_22_ENABLE)) |
				((u32)(XDEINT_MODE_PULL_32_ENABLE)));
	/* Checking for Enable_32 */
	if (Enable_32 == 1) {
		ModeReg |= ((u32)(XDEINT_MODE_PULL_32_ENABLE));
	}
	/* Checking for Enable_22 */
	if (Enable_22 == 1) {
		ModeReg |= ((u32)(XDEINT_MODE_PULL_22_ENABLE));
	}

	XDeint_WriteReg((InstancePtr)->Config.BaseAddress,
						(XDEINT_MODE_OFFSET), ModeReg);
}

/*****************************************************************************/
/**
*
* This function returns the contents of Version register.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return	Returns the contents of the version register.
*
* @note		None.
*
******************************************************************************/
u32 XDeint_GetVersion(XDeint *InstancePtr)
{
	u32 Version;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read core version. */
	Version = XDeint_ReadReg(InstancePtr->Config.BaseAddress,
						(XDEINT_VER_OFFSET));

	return Version;
}

/*****************************************************************************/
/**
*
* This function sets the input frame size of the Deinterlacer core.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	Width is the width of the frame.
* @param 	Height is the height of the frame.
*
* @return 	None.
*
* @note		None.
*
******************************************************************************/
void XDeint_SetSize(XDeint *InstancePtr, u32 Width, u32 Height)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(Width != (u32)0x00);
	Xil_AssertVoid(Height != (u32)0x00);

	XDeint_WriteReg((InstancePtr)->Config.BaseAddress,
					(XDEINT_HEIGHT_OFFSET), Height);
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress,
					(XDEINT_WIDTH_OFFSET), Width);
}

/*****************************************************************************/
/**
*
* This routine is a stub for the interrupt callback. The stub is here in case
* the upper layer forgot to set the handler. On initialization,
* interrupt handler is set to this callback. It is considered an error for
* this handler to be invoked.
*
*****************************************************************************/
static void StubCallBack(void *CallBackRef)
{
	(void)CallBackRef;
	Xil_AssertVoidAlways();
}

/*****************************************************************************/
/**
*
* This function gets the video format of the Deinterlacer core.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	Packing is a pointer to 32 bit variable which holds Packaging
*		format.
*		- 0 4:2:0 Packging is used.
* 		- XDEINT_MODE_PACKING_0 4:2:2 Packging is used.
* 		- XDEINT_MODE_PACKING_1 4:4:4 Packging is used.
* @param	Color is a pointer to 32 bit varaible which holds color value.
*		- 0 YUV color space is used.
* 		- XDEINT_MODE_COL RGB color space is used.
* @param	Order is a pointer to 32 bit varaible which holds Field order.
*		- 0 the field order maps to PAL/HD/3G.
* 		- XDEINT_MODE_FIELD_ORDER the field order maps to NTSC/480i
* @param	PSF is a pointer to 32 bit variable which says whether Psf
*		enabled or disabled.
*		- XDEINT_MODE_PSF_ENABLE for psf enable.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDeint_GetVideo(XDeint *InstancePtr, u32 *Packing, u32 *Color,
		u32 *Order, u32 *PSF)
{
	u32 ModeReg;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady ==
			(u32)(XIL_COMPONENT_IS_READY));

	/* Read modify write the mode register. */
	ModeReg = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,
						(XDEINT_MODE_OFFSET));
	*Packing = ModeReg & (XDEINT_MODE_PACKING_MASK);
	*Color = ModeReg & (XDEINT_MODE_COL_MASK);
	*Order = ModeReg & (XDEINT_MODE_FIELD_ORDER_MASK);
	*PSF = ModeReg & (XDEINT_MODE_PSF_ENABLE_MASK);
}

/*****************************************************************************/
/**
*
* This function gets the pull down controller of the Deinterlacer core.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	Enable_32 is a pointer which holds status of Enable_32 mode.
*		- FALSE = Disabled.
*		- TRUE = Enabled.
* @param	Enable_22 is a pointer which holds status of Enable_22 mode.
*		- FALSE = Disabled.
*		- TRUE = Enabled.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDeint_GetPulldown(XDeint *InstancePtr, u32 *Enable_32,
			u32 *Enable_22)
{
	u32 ModeReg;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady ==
				(u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(Enable_32 != NULL);
	Xil_AssertVoid(Enable_22 != NULL);

	/* Read modify write the mode register.*/
	ModeReg = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,
						(XDEINT_MODE_OFFSET));
	*Enable_32 = FALSE;
	*Enable_22 = FALSE;
	if (((ModeReg) & (XDEINT_MODE_PULL_32_ENABLE)) ==
				XDEINT_MODE_PULL_32_ENABLE) {
		*Enable_32 = TRUE;
	}
	if (((ModeReg) & (XDEINT_MODE_PULL_22_ENABLE)) ==
				XDEINT_MODE_PULL_22_ENABLE) {
		*Enable_22 = TRUE;
	}
}

/*****************************************************************************/
/**
*
* This function gets the input frame size of the Deinterlacer core.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	Width is pointer to the width of the frame.
* @param	Height is pointer to the height of the frame.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDeint_GetSize(XDeint *InstancePtr, u32 *Width, u32 *Height)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(Width != NULL);
	Xil_AssertVoid(Height != NULL);

	*Height = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,
			(XDEINT_HEIGHT_OFFSET)) & (XDEINT_HEIGHT_MASK);
	*Width = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,
			(XDEINT_WIDTH_OFFSET)) & (XDEINT_WIDTH_MASK);
}

/*****************************************************************************/
/**
*
* This function gets the threshold used by the motion adaptive kernel.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	ThresholdT1 is the pointer to lower threshold of the
*		motion kernel.
* @param	ThresholdT2 is the pointer to upper threshold of the
*		motion kernel.
*
* @return	None.
*
******************************************************************************/
void XDeint_GetThresholds(XDeint *InstancePtr, u32 *ThresholdT1,
						u32 *ThresholdT2)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady ==
				(u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(ThresholdT1 != NULL);
	Xil_AssertVoid(ThresholdT2 != NULL);

	/* Determine the T1->T2 cross fade setting.*/
	*ThresholdT1 = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,
			(XDEINT_THRESH1_OFFSET)) & (XDEINT_THRESHOLD_MASK);
	*ThresholdT2 = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,
			(XDEINT_THRESH2_OFFSET)) & (XDEINT_THRESHOLD_MASK);
}

/*****************************************************************************/
/**
*
* This function gets input field buffer addresses of an Deinterlacer core.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	FieldAddr1 is the pointer to the 1st input field buffer.
* @param	FieldAddr2 is the pointer to the 2nd input field buffer.
* @param	FieldAddr3 is the pointer to the 3rd input field buffer.
* @param	FrameSize  is the pointer to size in 32bit words of a single
*		field buffer.
*
* @return 	None.
*
* @note		None.
*
******************************************************************************/
void XDeint_GetFramestore(XDeint *InstancePtr,
			u32 *FieldAddr1, u32 *FieldAddr2,
			u32 *FieldAddr3, u32 *FrameSize)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)(XIL_COMPONENT_IS_READY));
	Xil_AssertVoid(FrameSize != NULL);
	Xil_AssertVoid(FieldAddr1 != NULL);
	Xil_AssertVoid(FieldAddr2 != NULL);
	Xil_AssertVoid(FieldAddr3 != NULL);

	/* Get the input buffer addresses and size of all field stores. */
	*FieldAddr1 = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,
						(XDEINT_BUFFER0_OFFSET));
	*FieldAddr2 = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,
						(XDEINT_BUFFER1_OFFSET));
	*FieldAddr3 = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,
						(XDEINT_BUFFER2_OFFSET));
	*FrameSize = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,
						(XDEINT_BUFSIZE_OFFSET));
}

/** @} */
