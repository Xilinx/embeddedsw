/******************************************************************************
* Copyright (c) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdeint.h
* @addtogroup deinterlacer Overview
* @{
* @details
*
* This is the main header file of Xilinx Video Deinterlacer core.
*
* <b>Interrupt Service </b>
*
* The Deinterlacer can generate 11 interrupt sources
* - Internal Register update done
* - Deinterlacer is locked to incoming video
* - Deinterlacer has lost lock to incoming video
* - Deinterlacer internal FIFO error
* - Pull down activated
* - Pull down cancelled
* - Frame Tick
* - Frame store Write setup error
* - Frame store Write FIFO overflow
* - Frame store Read Field under run
* - Frame store Read Frame under run
*
* This driver provides functions to install callbacks for the interrupts and
* enable/disable/clear any of them.
*
* <b> Examples </b>
*
* Example(s) are provided with this driver to demonstrate the self test.
*
* <b>Limitations</b>
*
* <b>BUS Interface</b>
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rjh    07/10/11 First release
* 2.00a rjh    18/01/12 Updated for v_deinterlacer 2.00
* 3.0   adk    19/12/13 Updated as per the New Tcl API's.
* 3.2   adk    02/13/14 Modified XDeint_IntrDisable macro replace ISR_OFFSET
*                       with IER OFFSET, XDeint_InReset.
*                       Added Doxygen support, adherence to Xilinx
*                       coding standards.
*
*                       Modification history of xdeint_sinit.c:
*                       Added Doxygen support, adherence to Xilinx
*                       coding standards.
*
*                       Modification history of xdeint_selftest.c:
*                       Added the XDeint_Selftest function.
*
*                       Modification history of xdeint_intr.c:
*                       Adherence to Xilinx coding, Doxygen guidelines.
*
*                       Modification history of xdeint_i.h:
*                       Added Doxygen support.
*
*                       Modification history of xdeint_hw.h:
*                       Suffixed "_OFFSET" to all register offset macros.
*                       Added bit masks for the registers and added
*                       backward compatibility for macros.
*
*                       Modification history of xdeint.c:
*                       Changed the prototype of XDeint_GetVersion
*                       Implemented the following functions:
*                       XDeint_GetFramestore
*                       XDeint_GetVideo
*                       XDeint_GetThresholds
*                       XDeint_GetPulldown
*                       XDeint_GetSize
*        ms    03/17/17 Added readme.txt file in examples folder for doxygen
*                       generation.
* </pre>
*
******************************************************************************/

#ifndef XDEINT_H
#define XDEINT_H	/**< Prevent circular inclusions
			  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xdeint_hw.h"
#include "xstatus.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/

/** @name Interrupt types for setting up callbacks
 *  @{
 */
#define XDEINT_HANDLER	/**< Internal Register update done */
/*@}*/

/**************************** Type Definitions *******************************/

/**
 * Deinterlacer core configuration structure.
 * Each Deinterlacer core should have a configuration structure associated.
 */
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID of the core */
	u32 BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the core's registers */
} XDeint_Config;

/**
 * Callback type for all interrupts
 */
typedef void (*XDeint_CallBack)(u32 status);

/**
 * The XDeint driver instance data. An instance must be allocated for each
 * Deintlerlacer core in use.
 */
typedef struct {
	XDeint_Config Config;	/**< Hardware configuration */
	u32 IsReady;		/**< Core and the driver instance
				  *  are initialized */
	u32 XSize;		/**< X Input Dimension */
	u32 YSize;		/**< Y Input Dimension */
	XDeint_CallBack IntCallBack;	/**< Call back for Status interrupt */
} XDeint;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro enables the Deinterlacer algorithms.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XDeint_Enable(XDeint *InstancePtr)
*
******************************************************************************/
#define XDeint_Enable(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XDEINT_CONTROL_OFFSET), \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XDEINT_CONTROL_OFFSET)) | (XDEINT_CTL_ENABLE))

/*****************************************************************************/
/**
*
* This macro disables the Deinterlacer algorithms.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XDeint_Disable(XDeint *InstancePtr)
*
******************************************************************************/
#define XDeint_Disable(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XDEINT_CONTROL_OFFSET), \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XDEINT_CONTROL_OFFSET)) & (~(XDEINT_CTL_ENABLE)))

/*****************************************************************************/
/**
*
* This macro enables the Deinterlacer core to accept video.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XDeint_Start(XDeint *InstancePtr)
*
******************************************************************************/
#define XDeint_Start(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XDEINT_CONTROL_OFFSET), \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XDEINT_CONTROL_OFFSET)) | (XDEINT_CTL_ACCEPT_VIDEO))

/*****************************************************************************/
/**
*
* This macro halts the Deinterlacer core on the next frame boundary.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XDeint_Stop(XDeint *InstancePtr)
*
******************************************************************************/
#define XDeint_Stop(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XDEINT_CONTROL_OFFSET), \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XDEINT_CONTROL_OFFSET)) & \
				(~(XDEINT_CTL_ACCEPT_VIDEO)))

/*****************************************************************************/
/**
*
* This macro soft resets the Deinterlacer to its default mode. This register
* will clear once the reset is complete. Software should poll here until the
* reset has completed.
* NOTE : Attempting to alter CPU registers during a soft reset will result
* in no register changes due to the CPU interface being reset.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XDeint_Reset(XDeint *InstancePtr)
*
******************************************************************************/
#define XDeint_Reset(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XDEINT_RESET_OFFSET), (u32)(XDEINT_RESET_RESET_MASK))

/*****************************************************************************/
/**
*
* This macro returns the current soft-reset state.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return	1=In Reset, 0=Ready.
*
* @note		C-style signature:
*		u32 XDeint_InReset(XDeint *InstancePtr)
*
******************************************************************************/
#define XDeint_InReset(InstancePtr) \
	(XDeint_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XDEINT_RESET_OFFSET)) & (XDEINT_RESET_RESET_MASK))

/*****************************************************************************/
/**
*
* This macro tells the Deinterlacer core to pick up all the register value
* changes made so far by the software at the next frame boundary.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XDeint_RegUpdateReq(XDeint *InstancePtr)
*
******************************************************************************/
#define XDeint_RegUpdateReq(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XDEINT_CONTROL_OFFSET), \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XDEINT_CONTROL_OFFSET)) | (XDEINT_CTL_UPDATE_REQ))

/*****************************************************************************/
/**
*
* This macro enables the given individual interrupt(s) on the
* Deinterlacer core.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	IntrType is the bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit post ions of 0 will keep
*		the previous setting.This mask is formed by OR'ing
*		XDEINT_IXR_*_MASK constants defined in xdeint_hw.h to create
*		this parameter value.
*
* @return	None.
*
* @note		The existing enabled interrupt(s) will remain enabled.
*
*		C-style signature:
*		void XDeint_IntrEnable(XDeint *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XDeint_IntrEnable(InstancePtr, IntrType) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XDEINT_IER_OFFSET), ((IntrType) & \
				(XDEINT_IXR_ALLINTR_MASK)) | \
	XDeint_ReadReg((InstancePtr)->Config.BaseAddress, (XDEINT_IER_OFFSET)))

/*****************************************************************************/
/**
*
* This macro disables the given individual interrupt(s) on the
* Deinterlacer core.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	IntrType is the bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be disabled. Bit post ions of 0 will keep
*		the previous setting.This mask is formed by OR'ing
*		XDEINT_IXR_*_MASK constants defined in xdeint_hw.h to create
*		this parameter value.
*
* @return	None.
*
* @note		Any other interrupt not covered by parameter IntrType, if
*		enabled before this macro is called, will remain enabled.
*
*		C-style signature:
*		void XDeint_IntrDisable(XDeint *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XDeint_IntrDisable(InstancePtr, IntrType) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XDEINT_IER_OFFSET), ((~(IntrType)) & \
			(XDEINT_IXR_ALLINTR_MASK)) & \
	XDeint_ReadReg((InstancePtr)->Config.BaseAddress, (XDEINT_IER_OFFSET)))

/*****************************************************************************/
/**
*
* This macro returns the pending interrupts of the Deinterlacer core.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return	The pending interrupts of the Deinterlacer core. Use
*		XDEINT_IXR_*_MASK constants defined in xdeint_hw.h to interpret
*		this value.
*
* @note		C-style signature:
*		u32 XDeint_IntrGetPending(XDeint *InstancePtr)
*
******************************************************************************/
#define XDeint_IntrGetPending(InstancePtr) \
	XDeint_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XDEINT_IER_OFFSET)) & \
		(XDeint_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XDEINT_ISR_OFFSET)) & (XDEINT_IXR_ALLINTR_MASK))

/*****************************************************************************/
/**
*
* This macro clears/acknowledges pending interrupts of the
* Deinterlacer core.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	IntrType is the pending interrupts to clear/acknowledge.
*		Use OR'ing of XDEINT_IXR_*_MASK constants defined in
*		xdeint_hw.h to create this parameter value.
*
* @return	None.
*
* @note		C-style signature:
*		void XDeint_IntrClear(XDeint *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XDeint_IntrClear(InstancePtr, IntrType) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, \
				(XDEINT_ISR_OFFSET), (IntrType) & \
					(XDeint_IntrGetPending(InstancePtr)))

/*****************************************************************************/
/**
*
* This macro sets the Deinterlacer's color space to RGB.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XDeint_SetRGB(XDeint *InstancePtr)
*
******************************************************************************/
#define XDeint_SetRGB(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XDEINT_MODE_OFFSET), \
			XDeint_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XDEINT_MODE_OFFSET)) | (XDEINT_MODE_COLOUR_RGB))

/*****************************************************************************/
/**
*
* This macro sets the Deinterlacer's color space to YUV.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XDeint_SetYUV(XDeint *InstancePtr)
*
******************************************************************************/
#define XDeint_SetYUV(InstancePtr) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XDEINT_MODE_OFFSET), \
		XDeint_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XDEINT_MODE_OFFSET)) & (~(XDEINT_MODE_COLOUR_RGB)))

/*****************************************************************************/
/**
*
* This macro sets the Deinterlacer's processing algorithm.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
* @param	Alg is a algorithm setting from XDEINT_MODE_ALGORITHM_????
*
* @return	None.
*
*		@note C-style signature:
*		void XDeint_SetAlgorithm(XDeint *InstancePtr, u32 Alg)
*
******************************************************************************/
#define XDeint_SetAlgorithm(InstancePtr, Alg) \
	XDeint_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XDEINT_MODE_OFFSET), \
			(XDeint_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XDEINT_MODE_OFFSET)) & \
					(~(XDEINT_MODE_ALGORITHM_FULL))) | \
						(Alg))

/*****************************************************************************/
/**
*
* This macro gets the Deinterlacer's color space.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return	Returns the color space.
*		0x00000004 - RGB color space,
*		0x00000000 - YUV color space.
*
* @note		C-style signature:
*		u32 XDeint_GetColorSpace(XDeint *InstancePtr)
*
******************************************************************************/
#define XDeint_GetColorSpace(InstancePtr) \
	XDeint_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XDEINT_MODE_OFFSET)) & (XDEINT_MODE_COL)

/*****************************************************************************/
/**
*
* This macro gets the Deinterlacer's processing algorithm.
*
* @param	InstancePtr is a pointer to XDeint instance to be worked on.
*
* @return	Returns algorithm for deinterlacing method.
*		- 0x00000000 = Pure field interpolating technique is used.
*		- 0x00000001 = Diagonal engine is used.
*		- 0x00000002 = Motion adaptive engine is used.
*		- 0x00000003 = Motion and Diagonal engines are used.
*
* @note		C-style signature:
*		u32 XDeint_GetAlgorithm(XDeint *InstancePtr)
*
******************************************************************************/
#define XDeint_GetAlgorithm(InstancePtr) \
	XDeint_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XDEINT_MODE_OFFSET)) & (XDEINT_MODE_ALGORITHM_FULL)

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

/* Frame store management*/
void XDeint_SetFramestore(XDeint *InstancePtr, u32 FieldAddr1, u32 FieldAddr2,
						u32 FieldAddr3, u32 FrameSize);
/* Deinterlacer Management */
void XDeint_SetSize(XDeint *InstancePtr, u32 Width, u32 Height);
void XDeint_SetPulldown(XDeint *InstancePtr, u32 Enable_32, u32 Enable_22);
void XDeint_SetThresholds(XDeint *InstancePtr, u32 ThresholdT1,
						u32 ThresholdT2);
void XDeint_SetVideo(XDeint *InstancePtr, u32 Packing, u32 Color, u32 Order,
								u32 PSF);
/*
 * Interrupt related functions in xdeint_intr.c
 */
void XDeint_IntrHandler(void *InstancePtr);
int XDeint_SetCallBack(XDeint *InstancePtr, void *CallBackFunc);

void XDeint_GetVideo(XDeint *InstancePtr, u32 *Packing, u32 *Color,
		u32 *Order, u32 *PSF);
void XDeint_GetPulldown(XDeint *InstancePtr, u32 *Enable_32, u32 *Enable_22);
void XDeint_GetSize(XDeint *InstancePtr, u32 *Width, u32 *Height);
void XDeint_GetThresholds(XDeint *InstancePtr, u32 *ThresholdT1,
						u32 *ThresholdT2);

void XDeint_GetFramestore(XDeint *InstancePtr,
				u32 *FieldAddr1, u32 *FieldAddr2,
				u32 *FieldAddr3, u32 *FrameSize);
u32 XDeint_GetVersion(XDeint *InstancePtr);
int XDeint_Selftest(XDeint *InstancePtr);

#ifdef __cplusplus
}

#endif

#endif	/* End of protection macro */
/** @} */
