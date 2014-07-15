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
* @file xdeint.h
*
* This is the main header file of Xilinx Video Deinterlacer device driver
*
* <b>Interrupt Service </b>
*
* The Deinterlacer can generate 11 interrupt sources
* - Internal Register update done
* - Deinterlacer is locked to incoming video
* - Deinterlacer has lost lock to incoming video
* - Deinterlacer internal fifo error
* - Pulldown activated
* - Pulldown cancelled
* - Frame Tick
* - Framestore Write setup error
* - Framestore Write fifo overflow
* - Framestore Read Field underrun
* - Framestore Read Frame underrun
*
* This driver provides functions to install callbacks for the interrupts and
* enable/disable/clear any of them.
*
* <b> Examples </b>
*
* Example(s) are provided with this driver to demonstrate the driver usage.
*
* <b>Limitations</b>
*
* <b>BUS Interface</b>
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rjh 07/10/11 First release
* 2.00a rjh 18/01/12 Updated for v_deinterlacer 2.00
* 3.0   adk  19/12/13 Updated as per the New Tcl API's
* 3.1   adk 09/05/14 Fixed the CR:798337 driver doesn't support the IP in the
*		     build.Changes are made in the driver mdd file.
* </pre>
*
******************************************************************************/

#ifndef XDeint_H		  /* prevent circular inclusions */
#define XDeint_H		  /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xdeint_hw.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/** @name Interrupt types for setting up callbacks
 *  @{
 */
#define XDeint_HANDLER               /**< Internal Register update done  */
/*@}*/

/**************************** Type Definitions *******************************/

/**
 * Deinterlacer device configuration structure.
 * Each Deinterlacer device should have a configuration structure associated.
 */
typedef struct {
	u16 DeviceId;	   /**< DeviceId is the unique ID of the device */
	u32 BaseAddress;   /**< BaseAddress is the physical base address of the
			     *  device's registers */
} XDeint_Config;

/**
 * Callback type for all interrupts
 *
 * @param	none
 */
typedef void (*XDeint_CallBack) (u32 status);

/**
 * The XDeint driver instance data. An instance must be allocated for each
 * DEINT device in use.
 */
typedef struct {
	XDeint_Config Config;	      /**< hardware configuration */
	u32 IsReady;		          /**< Device and the driver instance are initialized */
    u32 xsize;                    /**< X Input Dimension */
    u32 ysize;                    /**< Y Input Dimension */

	XDeint_CallBack IntCallBack;   /**< Call back for Status  interrupt */

} XDeint;

/*****************************************************************************/
/**
*
* This macro enables the deinterlacer algorithms
*
* @param  InstancePtr is a pointer to the Deinterlacer device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XDeint_Enable(XDeint *InstancePtr);
*
******************************************************************************/
#define XDeint_Enable(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_CONTROL, \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, XDEINT_CONTROL) | \
		XDEINT_CTL_ENABLE)

/*****************************************************************************/
/**
*
* This macro disables the deinterlacer algorithms
*
* @param  InstancePtr is a pointer to the DEINT device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XDeint_Disable(XDeint *InstancePtr);
*
******************************************************************************/
#define XDeint_Disable(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_CONTROL, \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, XDEINT_CONTROL) & \
		~XDEINT_CTL_ENABLE)

/*****************************************************************************/
/**
*
* This macro enables the deinterlacer to accept video
*
* @param  InstancePtr is a pointer to the Deinterlacer device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XDeint_Start(XDeint *InstancePtr);
*
******************************************************************************/
#define XDeint_Start(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_CONTROL, \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, XDEINT_CONTROL) | \
		XDEINT_CTL_ACCEPT_VIDEO)

/*****************************************************************************/
/**
*
* This macro halts the deinterlacer on the next frame boundary
*
* @param  InstancePtr is a pointer to the DEINT device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XDeint_Stop(XDeint *InstancePtr);
*
******************************************************************************/
#define XDeint_Stop(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_CONTROL, \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, XDEINT_CONTROL) & \
		~XDEINT_CTL_ACCEPT_VIDEO)

/*****************************************************************************/
/**
*
* This macro soft resets the deinterlacer to its default mode. This register will clear
* once the reset is complete. Software should poll here until the reset has completed
* NOTE : Attempting to alter CPU registers during a soft reset will result in no register
* changes due to the CPU interface being reset...
*
* @param  InstancePtr is a pointer to the DEINT device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XDeint_Reset(XDeint *InstancePtr);
*
******************************************************************************/
#define XDeint_Reset(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_RESET,XDEINT_RESET_RESET_MASK)

/*****************************************************************************/
/**
*
* This macro returns the current soft-reset state
*
* @param  InstancePtr is a pointer to the DEINT device instance to be worked on.
*
* @return 1=In Reset, 0=Ready.
*
* @note
* C-style signature:
*	 void XDeint_InReset(XDeint *InstancePtr);
*
******************************************************************************/
#define XDeint_InReset(InstancePtr) \
	XDeint_ReadReg((InstancePtr)->Config.BaseAddress, XDEINT_RESET)


/*****************************************************************************/
/**
*
* This macro tells an DEINT device to pick up all the register value changes
* made so far by the software at the next frame boundary.
*
* @param  InstancePtr is a pointer to the DEINT device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XDeint_RegUpdateReq(XDeint *InstancePtr);
*
******************************************************************************/
#define XDeint_RegUpdateReq(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_CONTROL, \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, XDEINT_CONTROL) | \
		0x00000001)

/*****************************************************************************/
/**
*
* This macro enables interrupts of an DEINT device.
*
* @param  InstancePtr is a pointer to the DEINT device instance to be worked on.
*
* @param  IntrType is the type of the interrupts to enable. Use OR'ing of
*	  XDeint_IXR_* constants defined in xdeint_hw.h to create this parameter
*	  value.
*
* @return None
*
* @note
*
* The existing enabled interrupt(s) will remain enabled.
*
* C-style signature:
*	 void XDeint_IntrEnable(XDeint *InstancePtr, u32 IntrType);
*
******************************************************************************/
#define XDeint_IntrEnable(InstancePtr, IntrType) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_IER, \
		((IntrType) & XDEINT_IXR_ALLINTR_MASK) | \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, XDEINT_IER))

/*****************************************************************************/
/**
*
* This macro disables interrupts of an DEINT device.
*
* @param  InstancePtr is a pointer to the DEINT device instance to be worked on.
*
* @param  IntrType is the type of the interrupts to disable. Use OR'ing of
*	  XDeint_IXR_* constants defined in xdeint_hw.h to create this parameter
*	  value.
*
* @return None
*
* @note
*
* Any other interrupt not covered by parameter IntrType, if enabled before
* this macro is called, will remain enabled.
*
* C-style signature:
*	 void XDeint_IntrDisable(XDeint *InstancePtr, u32 IntrType);
*
******************************************************************************/
#define XDeint_IntrDisable(InstancePtr, IntrType) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_IER, \
		((~(IntrType)) & XDEINT_IXR_ALLINTR_MASK) & \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, XDEINT_ISR))

/*****************************************************************************/
/**
*
* This macro returns the pending interrupts of an DEINT device.
*
* @param  InstancePtr is a pointer to the DEINT device instance to be worked on.
*
* @return The pending interrupts of the DEINT. Use XDEINT_IXR_* constants
*	  defined in xdeint_hw.h to interpret this value.
*
* @note
*
* C-style signature:
*	 u32 XDeint_IntrGetPending(XDeint *InstancePtr);
*
******************************************************************************/
#define XDeint_IntrGetPending(InstancePtr) \
	(XDeint_ReadReg((InstancePtr)->Config.BaseAddress, XDEINT_IER) & \
		 XDeint_ReadReg((InstancePtr)->Config.BaseAddress, XDEINT_ISR) \
		 & XDEINT_IXR_ALLINTR_MASK)

/*****************************************************************************/
/**
*
* This macro clears/acknowledges pending interrupts of an DEINT device.
*
* @param  InstancePtr is a pointer to the DEINT device instance to be worked on.
*
* @param  IntrType is the pending interrupts to clear/acknowledge. Use OR'ing
*	  of XDeint_IXR_* constants defined in xdeint_hw.h to create this
*	  parameter value.
*
* @return None
*
* @note
*
* C-style signature:
*	 void XDeint_IntrClear(XDeint *InstancePtr, u32 IntrType);
*
******************************************************************************/
#define XDeint_IntrClear(InstancePtr, IntrType) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_ISR, \
		(IntrType) & XDeint_IntrGetPending(InstancePtr))

/*****************************************************************************/
/**
*
* This macro set the deinterlacers colour space RGB.
*
* @param  InstancePtr is a pointer to the Deinterlacer device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XDeint_Enable(XDeint *InstancePtr);
*
******************************************************************************/
#define XDeint_SetRGB(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_MODE, \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, XDEINT_MODE) \
		 | XDEINT_MODE_COLOUR_RGB)

/*****************************************************************************/
/**
*
* This macro set the deinterlacers colour space to YUV.
*
* @param  InstancePtr is a pointer to the Deinterlacer device instance to be worked on.
*
* @return None.
*
* @note
* C-style signature:
*	 void XDeint_Enable(XDeint *InstancePtr);
*
******************************************************************************/
#define XDeint_SetYUV(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_MODE, \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, XDEINT_MODE) \
		 & ~XDEINT_MODE_COLOUR_RGB)

/*****************************************************************************/
/**
*
* This macro set the deinterlacers processing algorithm
*
* @param  InstancePtr is a pointer to the Deinterlacer device instance to be worked on.
* @param  Alg is a algorthim setting from XDEINT_MODE_ALGORITHM_????
*
* @return None.
*
* @note
* C-style signature:
*	 void XDeint_Enable(XDeint *InstancePtr);
*
******************************************************************************/
#define XDeint_SetAlgorithm(InstancePtr,Alg) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, XDEINT_MODE, \
		(XDeint_ReadReg((InstancePtr)->Config.BaseAddress, XDEINT_MODE) \
		 & ~XDEINT_MODE_ALGORITHM_FULL) | Alg)


/************************** Function Prototypes ******************************/

/*
 * Initialization and control functions in xdeint.c
 */

/* Initialization */
int XDeint_ConfigInitialize(XDeint *InstancePtr, XDeint_Config *CfgPtr,
				u32 EffectiveAddr);

/*
 * Initialization functions in xdeint_sinit.c
 */
XDeint_Config *XDeint_LookupConfig(u16 DeviceId);

// Framestore management
void XDeint_SetFramestore(XDeint *InstancePtr,
                u32 FieldAddr1, u32 FieldAddr2,
                u32 FieldAddr3, u32 FrameSize);

// Deinterlacer Management
void XDeint_SetSize(XDeint *InstancePtr,
                u32 Width, u32 Height);

void XDeint_SetPulldown(XDeint *InstancePtr,
                u32 enable_32,
                u32 enable_22);
void XDeint_SetThresholds(XDeint *InstancePtr,
                u32 t1, u32 t2);
void XDeint_SetVideo(XDeint *InstancePtr,
                u32 Packing, u32 Colour, u32 Order, u32 PSF);


/*
 * Interrupt related functions in xdeint_intr.c
 */
void XDeint_IntrHandler(void *InstancePtr);
int XDeint_SetCallBack(XDeint *InstancePtr, void *CallBackFunc);


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
