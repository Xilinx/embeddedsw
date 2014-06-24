/******************************************************************************
*
* Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file XDeint.c
*
* This is main code of Xilinx Vide Deinterlacer
* device driver. Please see xdeint.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who     Date      Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rjh 07/10/11 First release
* 2.00a rjh 18/01/12 Updated for v_deinterlacer 2.00
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdeint.h"
#include "xenv.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

static void StubCallBack(void *CallBackRef);

/************************** Function Definition ******************************/

/*****************************************************************************/
/**
 * This function initializes an Deinterlacer device.  This function must be called
 * prior to using a Deinterlacer device. Initialization of an Deinterlacer includes
 * setting up the instance data, and ensuring the hardware is in a quiescent
 * state.
 *
 * @param  InstancePtr is a pointer to the Deinterlacer device instance to be
 *       worked on.
 * @param  CfgPtr points to the configuration structure associated with the
 *       Deinterlacer device.
 * @param  EffectiveAddr is the base address of the device. If address
 *       translation is being used, then this parameter must reflect the
 *       virtual base address. Otherwise, the physical address should be
 *       used.
 * @return XST_SUCCESS
 *
 *****************************************************************************/
int XDeint_ConfigInitialize(XDeint *InstancePtr, XDeint_Config *CfgPtr,
                            u32 EffectiveAddr)
{
    /* Verify arguments */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(CfgPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (u32)NULL);

    /* Setup the instance */
    memset((void *)InstancePtr, 0, sizeof(XDeint));
    memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
               sizeof(XDeint_Config));
    InstancePtr->Config.BaseAddress = EffectiveAddr;

    /* Set all handlers to stub values, let user configure this data later
     */
    InstancePtr->IntCallBack = (XDeint_CallBack) StubCallBack;
    /* Reset the hardware and set the flag to indicate the driver is ready
     */
    XDeint_Disable(InstancePtr);
    /* Reset the deinterlacer
    */
    XDeint_Reset(InstancePtr);
    /* Wait for Soft reset to complete
    */
    while(XDeint_InReset(InstancePtr));
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function sets up input field buffer addresses of an Deinterlacer device.
 *
 * @param  InstancePtr is a pointer to the DEINT device instance to be
 *       worked on.
 * @param  FieldAddr1 is the address of the 1st input field buffer.
 * @param  FieldAddr2 is the address of the 2nd input field buffer.
 * @param  FieldAddr3 is the address of the 3rd input field buffer.
 * @param  FrameSize  is the size in 32bit words of a single field buffer
 * @return None.
 *
 *****************************************************************************/
void XDeint_SetFramestore(XDeint *InstancePtr,
                u32 FieldAddr1, u32 FieldAddr2,
                u32 FieldAddr3, u32 FrameSize)
{
    /* Verify arguments */
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertVoid(FrameSize  != (u32)NULL);

    /* Set the input buffer addresses amd size of all fieldstores */
    XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_FS_BASE0,FieldAddr1);
    XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_FS_BASE1,FieldAddr2);
    XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_FS_BASE2,FieldAddr3);
    XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_FS_WORDS,FrameSize);

    return;
}


/*****************************************************************************/
/**
 * This function sets up the video format
 *
 * @param  Packing selects the XSVI video packing mode.
 * @param  Colour  selects what colourspace to use
 * @param  Order   selects which field ordering is being used
 * @param  psf     enables psf (progressive segmented frame mode)
 * @return None.
 *
 *****************************************************************************/
void XDeint_SetVideo(XDeint *InstancePtr,
                u32 Packing, u32 Colour, u32 Order, u32 PSF)
{
u32 mode_reg;

    /* Verify arguments */
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Read modify write the mode register.
    mode_reg = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,XDEINT_MODE);
    mode_reg &= ~(XDEINT_MODE_PACKING_MASK |
                  XDEINT_MODE_COL_MASK |
                  XDEINT_MODE_FIELD_ORDER_MASK |
                  XDEINT_MODE_PSF_ENABLE_MASK);
    mode_reg |= Packing | Colour | Order | PSF;

    XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_MODE,mode_reg);

    return;
}

/*****************************************************************************/
/**
 * This function sets up the threshold used by the motion adaptive kernel
 *
 * @param  InstancePtr is a pointer to the DEINT device instance to be
 *       worked on.
 * @param  T1 is the lower threshold of the motion kernel
 * @param  T2 is the upper threshold of the motion kernel
 * @return None.
 *
 *****************************************************************************/
void XDeint_SetThresholds(XDeint *InstancePtr,
                u32 t1, u32 t2)
{
u32 xfade;
    /* Verify arguments */
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertVoid(t1 != (u32)NULL);
    Xil_AssertVoid(t2 != (u32)NULL);

    // Determine the T1->T2 cross fade setting.
    xfade = (256*4096)/(t2-t1);

    XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_T1,t1);
    XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_T2,t2);
    XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_XFADE,xfade);

    return;
}

/*****************************************************************************/
/**
 * This function sets up the pulldown controller
 *
 * @param  InstancePtr is a pointer to the DEINT device instance to be
 *       worked on.
 * @param  enable
 * @return None.
 *
 *****************************************************************************/
void XDeint_SetPulldown(XDeint *InstancePtr,
                u32 enable_32,
                u32 enable_22)
{
u32 mode_reg;

    /* Verify arguments */
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Read modify write the mode register.
    mode_reg = XDeint_ReadReg((InstancePtr)->Config.BaseAddress,XDEINT_MODE);
    mode_reg &= ~(XDEINT_MODE_PULL_22_ENABLE | XDEINT_MODE_PULL_32_ENABLE);
    if (enable_32)
      mode_reg |= XDEINT_MODE_PULL_32_ENABLE;
    if (enable_22)
      mode_reg |= XDEINT_MODE_PULL_22_ENABLE;

    XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_MODE,mode_reg);

    return;
}

/*****************************************************************************/
/**
*
* This function returns the version of a Deinterlacer device.
*
* @param  InstancePtr is a pointer to the Deinterlacer device instance to be
*	  worked on.
* @param  Major points to an unsigned 16-bit variable that will be assigned
*	  with the major version number after this function returns. Value
*	  range is from 0x0 to 0xF.
* @param  Minor points to an unsigned 16-bit variable that will be assigned
*	  with the minor version number after this function returns. Value
*	  range is from 0x00 to 0xFF.
* @param  Revision points to an unsigned 16-bit variable that will be assigned
*	  with the revision version number after this function returns. Value
*	  range is from 0xA to 0xF.
* @return None.
* @note	  Example: Device version should read v2.01.c if major version number
*	  is 0x2, minor version number is 0x1, and revision version number is
*	  0xC.
*
******************************************************************************/
void XDeint_GetVersion(XDeint *InstancePtr, u16 *Major, u16 *Minor,
				u16 *Revision)
{
	u32 Version;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Major != NULL);
	Xil_AssertVoid(Minor != NULL);
	Xil_AssertVoid(Revision != NULL);

	/* Read device version */
	Version = XDeint_ReadReg(InstancePtr->Config.BaseAddress, XDEINT_VER);

	/* Parse the version and pass the info to the caller via output
	 * parameter
	 */
	*Major = (u16)
		((Version & XDEINT_VER_MAJOR_MASK) >> XDEINT_VER_MAJOR_SHIFT);

	*Minor = (u16)
		((Version & XDEINT_VER_MINOR_MASK) >> XDEINT_VER_MINOR_SHIFT);

	*Revision = (u16)
		((Version & XDEINT_VER_REV_MASK) >> XDEINT_VER_REV_SHIFT);

	return;
}

/*****************************************************************************/
/**
 * This function sets up the input frame size of the deinterlacer
 *
 * @param  InstancePtr is a pointer to the DEINT device instance to be
 *       worked on.
 * @param  Width
 * @param  Height
 * @return None.
 *
 *****************************************************************************/
void XDeint_SetSize(XDeint *InstancePtr,
                u32 Width, u32 Height)
{
    /* Verify arguments */
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertVoid(Width != (u32)NULL);
    Xil_AssertVoid(Height != (u32)NULL);

    XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_HEIGHT,Height);
    XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_WIDTH,Width);

    return;
}


/*****************************************************************************/
/*
 * This routine is a stub for the frame done interrupt callback. The stub is
 * here in case the upper layer forgot to set the handler. On initialization,
 * the frame done interrupt handler is set to this callback. It is considered
 * an error for this handler to be invoked.
 *
 *****************************************************************************/
static void StubCallBack(void *CallBackRef)
{
    Xil_AssertVoidAlways();
}
