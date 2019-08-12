/******************************************************************************
*
* Copyright (C) 2001 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xtpg.h
* @addtogroup tpg_v3_1
* @{
* @details
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx Test Pattern Generator
* (TPG) core instance.
*
* The TPG core provides a wide variety of tests patterns enabling you to debug
* and assess video system color, quality, edge, and motion performance.
*
* The Test Pattern Generator core produces the following patterns in RGB,
* YCbCr 444, or YCbCr 422 video format.
*	- Video input pass through
*	- Horizontal ramp
*	- Vertical ramp
*	- Temporal ramp
*	- Flat fields (red, green, blue, black and white)
*	- Combined vertical and horizontal ramp
*	- Color bars
*	- Tartan bars
*	- Zone plate
*	- Cross hairs
*	- Cross hatch
*	- Solid box
*	- Motion effect for ramps, zone plate, and solid box
*
* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the TPG core.
*
* XTpg_CfgInitialize() API is used to initialize the TPG core.
* The user needs to first call the XTpg_LookupConfig() API which returns
* the Configuration structure pointer which is passed as a parameter to the
* XTpg_CfgInitialize() API.
*
* <b> Interrupts </b>
*
* The driver provides an interrupt handler XTpg_IntrHandler for handling
* the interrupt from the TPG core. The users of this driver have to
* register this handler with the interrupt system and provide the callback
* functions by using XTpg_SetCallBack API.
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* The XTpg driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00a se     10/01/12 Initial creation.
* 2.0a  se     02/12/14 Cleaned up comments, updated masks and registers.
* 2.0   adk    19/12/13 Updated as per the New Tcl API's.
* 3.0   adk    02/19/14 Changed the file name from "tpg.h" to "xtpg.h"
*                       Register offsets, TPG_In32, TPG_In32, TPG_ReadReg,
*                       TPG_WriteReg and bit definitions of tpg.h file
*                       were moved to xtpg_hw.h file.
*
*                       Removed the following bit definitions :
*                       TPG_CTL_CS_MASK.
*
*                       Defined the following handler types as enum values:
*                       XTPG_HANDLER_PROCSTART ,XTPG_HANDLER_FRAMEDONE and
*                       XTPG_HANDLER_ERROR.
*                       Defined the following enums:
*                       XTpg_BackgroundPattern, XTpg_ComponentMask and
*                       XTpg_BayerPhaseCombination.
*
*                       Defined the following range macros
*                       XTPG_MOTION_SPEED_MIN, XTPG_MOTION_SPEED_MAX,
*                       XTPG_VSIZE_FIRST, XTPG_VSIZE_LAST, XTPG_HSIZE_FIRST,
*                       XTPG_HSIZE_LAST.
*
*                       Added the following function macros:
*                       XTpg_Enable, XTpg_Disable, XTpg_Start, XTpg_Stop,
*                       XTpg_RegUpdateEnable, XTpg_RegUpdateDisable,
*                       XTpg_Reset, XTpg_SyncReset, XTpg_IntrEnable,
*                       XTpg_IntrDisable, XTpg_StatusGetPending,
*                       XTpg_IntrGetPending, XTpg_IntrClear.
*
*                       Removed following function macros:
*                       TPG_Enable, TPG_Disable, TPG_RegUpdateEnable,
*                       TPG_RegUpdateDisable, TPG_Reset, TPG_FSyncReset
*                       TPG_ClearStatus, TPG_ClearReset.
*                       Added the following structures
*                       XTpg and XTpg_Config.
*
*                       Implemented XTpg_LookupConfig in xtpg_sinit.c
*                       Implemented XTpg_SelfTest in xtpg_selftest.c
*                       Implemented XTpg_IntrHandler, XTpg_SetCallBack in
*                       xtpg_intr.c.
*
*                       Added the register offsets and bit masks for the
*                       registers and added backward compatibility for
*                       macros.in xtpg_hw.h.
*
*                       Modification history from xtpg.c file:
*                       Changed the file name form "tpg.c" to "xtpg.c".
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
*       ms     03/17/17 Added readme.txt file in examples folder for doxygen
*                       generation.
*       ms     04/10/17 Modified filename tag in examples to include them in
*                       doxygen examples.
* 3.1   ms     05/22/17 Updated the parameter naming from
*                       XPAR_TPG_NUM_INSTANCES to XPAR_XTPG_NUM_INSTANCES
*                       to avoid  compilation failure as the tools are
*                       generating XPAR_XTPG_NUM_INSTANCES in the xtpg_g.c
*                       for fixing MISRA-C files.
* </pre>
*
******************************************************************************/
#ifndef XTPG_H_
#define XTPG_H_		/**< Prevent circular inclusions
			  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_assert.h"
#include "xstatus.h"
#include "xtpg_hw.h"

/************************** Constant Definitions *****************************/

/** @name Handler Types
 * @{
 */
/**
* These constants specify different types of handlers and used to differentiate
* interrupt requests from core.
*/
enum {
	XTPG_HANDLER_PROCSTART = 1,	/**< A processing start event interrupt
					  *  type */
	XTPG_HANDLER_FRAMEDONE,		/**< A frame done event interrupt
					  *  type */
	XTPG_HANDLER_ERROR		/**< An error condition event interrupt
					  *  type */
} ;
/*@}*/

/** @name BackgroundPattern
 * @{
 */
/**
* These constants specify different types of background patterns supported by
* the core.
*/
enum XTpg_BackgroundPattern {
	XTPG_PASS_THROUGH,		/**< Pass video input straight through
					  *  the video output */
	XTPG_H_RAMP,			/**< Horizontal ramp */
	XTPG_V_RAMP,			/**< Vertical ramp */
	XTPG_T_RAMP,			/**< Temporal ramp */
	XTPG_RED_PLANE,			/**< Solid red output */
	XTPG_GREEN_PLANE,		/**< Solid green output */
	XTPG_BLUE_PLANE,		/**< Solid blue output */
	XTPG_BLACK_PLANE,		/**< Solid black output */
	XTPG_WHITE_PLANE,		/**< Solid white output */
	XTPG_COLOR_BARS,		/**< Color bars */
	XTPG_ZONE_PLATE,		/**< Zone plate output (sinusoidal) */
	XTPG_TARAN_BARS,		/**< Tartan color bars */
	XTPG_CROSS_HATCH,		/**< Cross hatch pattern */
	XTPG_HV_RAMP = 0xE,		/**< Horizontal vertical ramp */
	XTPG_BLACK_AND_WHITE_CHECKERBOARD /**< Black and white checker board */
};
/*@}*/

/** @name ComponentMask
 * @{
 */
/**
* These constants specify mask outs for a particular color component.
*/
enum XTpg_ComponentMask {
	XTPG_NOMASK,		/**< No masking */
	XTPG_MASKOUT_RED,	/**< Mask out red,
				  *  Cr(for YCbCr mode) component */
	XTPG_MASKOUT_GREEN,	/**< Mask out green,
				  *  Y( for YCbCr mode) component */
	XTPG_MASKOUT_BLUE = 0x4	/**< Mask out blue,
				  *  Cr(for YCbCr mode) component */
};
/*@}*/

/** @name Bayer phase combinations
 * @{
 */
/**
* These constants specify Bayer phase combinations of the core.
*/
enum XTpg_BayerPhaseCombination {
	XTPG_BAYER_PHASE_RGRG,		/**< Red green combination */
	XTPG_BAYER_PHASE_GRGR,		/**< Green red combination */
	XTPG_BAYER_PHASE_GBGB,		/**< Green blue combination */
	XTPG_BAYER_PHASE_BGBG,		/**< Blue green combination */
	XTPG_BAYER_PHASE_TURNOFF	/**< Turn off the Bayer phase */
} ;
/*@}*/

/** @name Motion Speed Ranges
 * @{
 */
#define XTPG_MOTION_SPEED_MIN	0	/**< Motion Speed minimum value */
#define XTPG_MOTION_SPEED_MAX	255	/**< Motion Speed maximum value */
/*@}*/

/** @name Active Size Ranges
 * @{
 */
#define XTPG_VSIZE_FIRST	32	/**< Vertical Size starting value */
#define XTPG_VSIZE_LAST		7680	/**< Vertical Size ending value */
#define XTPG_HSIZE_FIRST	32	/**< Horizontal Size starting
					  *  value */
#define XTPG_HSIZE_LAST		7680	/**< Horizontal Size ending value */
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Macros for operating a TPG core
 * @{
 */
/*****************************************************************************/
/**
*
* This function macro enables the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XTpg_Enable(XTpg *InstancePtr)
*
******************************************************************************/
#define XTpg_Enable(InstancePtr) \
	XTpg_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XTPG_CONTROL_OFFSET), \
			(XTpg_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XTPG_CONTROL_OFFSET)) | (XTPG_CTL_SW_EN_MASK)))

/*****************************************************************************/
/**
*
* This function macro disables the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XTpg_Disable(XTpg *InstancePtr)
*
******************************************************************************/
#define XTpg_Disable(InstancePtr) \
	XTpg_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XTPG_CONTROL_OFFSET), \
			(XTpg_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XTPG_CONTROL_OFFSET)) & \
					(~(XTPG_CTL_SW_EN_MASK))))

/*****************************************************************************/
/**
*
* This function macro enables/starts the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XTpg_Start(XTpg *InstancePtr)
*
******************************************************************************/
#define XTpg_Start	XTpg_Enable

/*****************************************************************************/
/**
*
* This function macro disables/stops the TPG core.
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XTpg_Stop(XTpg *InstancePtr)
*
******************************************************************************/
#define XTpg_Stop	XTpg_Disable

/*****************************************************************************/
/**
*
* This function macro commits all the register value changes made so far by
* the software to the TPG core.
*
* This function only works when the TPG core is enabled.
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
* 		void XTpg_RegUpdateEnable(XTpg *InstancePtr)
*
******************************************************************************/
#define XTpg_RegUpdateEnable(InstancePtr) \
	XTpg_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XTPG_CONTROL_OFFSET), \
			(XTpg_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XTPG_CONTROL_OFFSET)) | (XTPG_CTL_RUE_MASK)))

/*****************************************************************************/
/**
*
* This function macro prevents the TPG core from committing recent changes made
* so far by the software. When disabled, changes to other configuration
* registers are stored, but do not effect the behavior of the core.
*
* This function only works when the TPG core is enabled.
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
* 		void XTpg_RegUpdateDisable(XTpg *InstancePtr)
*
******************************************************************************/
#define XTpg_RegUpdateDisable(InstancePtr) \
	XTpg_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XTPG_CONTROL_OFFSET), \
			(XTpg_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XTPG_CONTROL_OFFSET)) & (~(XTPG_CTL_RUE_MASK))))

/*****************************************************************************/
/**
*
* This function macro resets a TPG core at the end of the frame being
* processed. It enables core automatically synchronizes to the SOF of the core
* to prevent image tearing. This function macro is differ from XTpg_Reset().
*
* On the next rising-edge of SOF following a call to XTpg_SyncReset(),
* all of the core's configuration registers and outputs will be reset, then the
* reset flag will be immediately released, allowing the core to immediately
* resume default operation.
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XTpg_SyncReset(XTpg *InstancePtr)
*
******************************************************************************/
#define XTpg_SyncReset(InstancePtr) \
	XTpg_WriteReg(((InstancePtr)->Config.BaseAddress), \
		(XTPG_CONTROL_OFFSET), (XTPG_CTL_AUTORESET_MASK))

/*****************************************************************************/
/**
*
* This function macro resets a TPG core. This reset effects the core
* immediately and may cause image tearing.
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
*
* @return	None.
*
* @note		This reset resets the TPG configuration registers.
*		C-style signature:
*		void XTpg_Reset(XTpg *InstancePtr)
*
******************************************************************************/
#define XTpg_Reset(InstancePtr) \
	XTpg_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XTPG_CONTROL_OFFSET), (XTPG_CTL_RESET_MASK))

/*****************************************************************************/
/**
*
* This function macro enables individual interrupts of the TPG core by updating
* the IRQ_ENABLE register.
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
* @param	IntrType is the bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XTPG_IXR_*_MASK bits defined in xtpg_hw.h.
*
* @return	None.
*
* @note		The existing enabled interrupt(s) will remain enabled.
* 		C-style signature:
* 		void XTpg_IntrEnable(XTpg *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XTpg_IntrEnable(InstancePtr, IntrType) \
	XTpg_WriteReg((InstancePtr)->Config.BaseAddress, (XTPG_IRQ_EN_OFFSET),\
		((IntrType) & (XTPG_IXR_ALLINTR_MASK)) | \
			(XTpg_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XTPG_IRQ_EN_OFFSET))))

/*****************************************************************************/
/**
*
* This function macro disables individual interrupts of the TPG core by
* updating the IRQ_ENABLE register.
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
* @param	IntrType is the bit-mask of the interrupts to be disabled.
*		Bit positions of 1 will be disabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XTPG_IXR_*_MASK bits defined in xtpg_hw.h.
*
* @return	None.
*
* @note		Any other interrupt not covered by parameter IntrType, if
* 		enabled before this macro is called, will remain enabled.
* 		C-style signature:
*		void XTpg_IntrDisable(XTpg *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XTpg_IntrDisable(InstancePtr, IntrType) \
	XTpg_WriteReg((InstancePtr)->Config.BaseAddress, (XTPG_IRQ_EN_OFFSET),\
		XTpg_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XTPG_IRQ_EN_OFFSET)) & ((~(IntrType)) & \
				(XTPG_IXR_ALLINTR_MASK)))

/*****************************************************************************/
/**
*
* This function macro returns the pending interrupt status of the TPG core read
* from the Status register.
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
*
* @return	The pending interrupts of the TPG. Use XTPG_IXR_*_MASK
*		constants defined in xtpg_hw.h to interpret this value.
*
* @note		C-style signature:
* 		u32 XTpg_StatusGePending(XTpg *InstancePtr)
*
******************************************************************************/
#define XTpg_StatusGetPending(InstancePtr) \
	XTpg_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XTPG_STATUS_OFFSET)) & \
					(XTPG_IXR_ALLINTR_MASK)

/*****************************************************************************/
/**
*
* This function macro returns the pending interrupts of the TPG core for the
* interrupts that have been enabled.
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
*
* @return	The pending interrupts of the TPG. Use XTPG_IXR_*_MASK
*		constants defined in xtpg_hw.h to interpret this value.
*		The returned value is a logical AND of the contents of the
*		Status Register and the IRQ_ENABLE Register.
*
* @note		C-style signature:
*		u32 XTpg_IntrGetPending(XTpg *InstancePtr)
*
******************************************************************************/
#define XTpg_IntrGetPending(InstancePtr) \
	XTpg_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XTPG_IRQ_EN_OFFSET)) &\
		(XTpg_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XTPG_STATUS_OFFSET)) & ((u32)XTPG_IXR_ALLINTR_MASK))

/*****************************************************************************/
/**
*
* This function macro clears/acknowledges pending interrupts of the TPG core
* in the Status register. Bit positions of 1 will be cleared.
*
* @param	InstancePtr is a pointer to the XTpg instance to be worked on.
* @param	IntrType is the pending interrupts to clear/acknowledge.
*		Use OR'ing of XTPG_IXR_*_MASK constants defined in xtpg_hw.h to
*		create this parameter value.
*
* @return	None.
*
* @note		C-style signature:
* 		void XTpg_IntrClear(XTpg *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XTpg_IntrClear(InstancePtr, IntrType) \
	XTpg_WriteReg((InstancePtr)->Config.BaseAddress, \
				(XTPG_STATUS_OFFSET),\
		(IntrType) & ((u32)(XTPG_IXR_ALLINTR_MASK)))
/*@}*/

/**************************** Type Definitions *******************************/

/**
* This typedef contains configuration information for a TPG core.
* Each TPG core should have a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;			/**< DeviceId is the unique ID
					  *  of the TPG core */
	u32 BaseAddress;		/**< BaseAddress is the physical
					  *  base address of the
					  *  TPG core registers */
	u32 SlaveAxisVideoFormat;	/**< Slave Axis Video Format */
	u32 MasterAxisVideoFormat;	/**< Master Axis Video Format */
	u32 SlaveAxiClkFreqHz;		/**< Slave Clock Frequency */
	u32 ActiveRows;			/**< Active Rows */
	u32 ActiveColumns;		/**< Active Columns */
	u32 PatternControl;		/**< Pattern Control */
	u8  MotionSpeed;		/**< Motion Speed */
	u32 CrossHairs;			/**< Cross Hair */
	u32 ZPlateHorizontalControl;	/**< ZPlate Horizontal Control */
	u32 ZPlateVerticalControl;	/**< ZPlate Vertical Control */
	u16 BoxSize;			/**< Box Size */
	u32 BoxColor;			/**< Box Color */
	u16 StuckPixelThreshold;	/**< Stuck Pixel Threshold */
	u8  NoiseGain;			/**< Noise Gain */
	u8  BayerPhase;			/**< Bayer Phase */
	u16 HasIntcIf;			/**< Has Interrupt Control Flag */
	u16 EnableMotion;		/**< Enable Motion */
} XTpg_Config;

/**
*
* Callback type for all interrupts except error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
*/
typedef void (*XTpg_CallBack)(void *CallBackRef);

/**
*
* Callback type for Error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
*		value equals 'OR'ing one or more XTPG_IXR_*_MASK values defined
*		in xtpg_hw.h.
*/
typedef void (*XTpg_ErrorCallBack)(void *CallBackRef, u32 ErrorMask);

/**
*
* The XTpg driver instance data structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
typedef struct {
	XTpg_Config Config;		/**< Hardware configuration */
	u32 IsReady;			/**< Core and the driver instance
					  *  are initialized */
	u16 HSize;			/**< Active Video Horizontal Size */
	u16 VSize;			/**< Active Video Vertical Size */

	/* IRQ Callbacks Here */
	XTpg_CallBack ProcStartCallBack;/**< Callback for Processing Start
					  *  interrupt */
	void *ProcStartRef;		/**< To be passed to the Process Start
					  *  interrupt callback */

	XTpg_CallBack FrameDoneCallBack;/**< Callback for Frame Done
					  *  interrupt */
	void *FrameDoneRef;		/**< To be passed to the Frame Done
					  *  interrupt callback */
	XTpg_ErrorCallBack ErrCallBack;	/**< Callback for Error interrupt */
	void *ErrRef;			/**< To be passed to the Error
					  *  interrupt callback */
} XTpg;

/************************** Function Prototypes ******************************/

/*
 * Static lookup function implemented in xtpg_sinit.c
 */
XTpg_Config *XTpg_LookupConfig(u16 DeviceId);

/*
 * Initialization and control functions implemented in xtpg.c
 */
int XTpg_CfgInitialize(XTpg *InstancePtr, XTpg_Config *CfgPtr,
			u32 EffectiveAddr);
void XTpg_Setup(XTpg *InstancePtr);
u32 XTpg_GetVersion(XTpg *InstancePtr);
void XTpg_SetActiveSize(XTpg *InstancePtr, u16 HSize, u16 VSize);
void XTpg_GetActiveSize(XTpg *InstancePtr, u16 *HSize, u16 *VSize);
void XTpg_SetPattern(XTpg *InstancePtr, u32 Pattern);
u32 XTpg_GetPattern(XTpg *InstancePtr);
void XTpg_SetBackground(XTpg *InstancePtr, enum XTpg_BackgroundPattern Pattern);
u32 XTpg_GetBackground(XTpg *InstancePtr);
void XTpg_EnableCrossHair(XTpg *InstancePtr);
void XTpg_DisableCrossHair(XTpg *InstancePtr);
void XTpg_EnableMotion(XTpg *InstancePtr);
void XTpg_DisableMotion(XTpg *InstancePtr);
void XTpg_EnableBox(XTpg *InstancePtr);
void XTpg_DisableBox(XTpg *InstancePtr);
void XTpg_SetComponentMask(XTpg *InstancePtr, enum XTpg_ComponentMask Mask);
u32 XTpg_GetComponentMask(XTpg *InstancePtr);
void XTpg_EnableStuckPixel(XTpg *InstancePtr);
void XTpg_DisableStuckPixel(XTpg *InstancePtr);
void XTPg_EnableNoise(XTpg *InstancePtr);
void XTPg_DisableNoise(XTpg *InstancePtr);
void XTpg_SetMotionSpeed(XTpg *InstancePtr, u32 MotionSpeed);
u32 XTpg_GetMotionSpeed(XTpg *InstancePtr);
void XTpg_SetCrosshairPosition(XTpg *InstancePtr, u16 HPos, u16 VPos);
void XTpg_GetCrosshairPosition(XTpg *InstancePtr, u16 *HPos, u16 *VPos);
void XTpg_SetZPlateHStart(XTpg *InstancePtr, u16 ZPlateHStart);
u16 XTpg_GetZPlateHStart (XTpg *InstancePtr);
void XTpg_SetZPlateHSpeed(XTpg *InstancePtr, u16 ZPlateHSpeed);
u16 XTpg_GetZPlateHSpeed(XTpg *InstancePtr);
void XTpg_SetZPlateVStart(XTpg *InstancePtr, u16 ZPlateVStart);
u16 XTpg_GetZPlateVStart (XTpg *InstancePtr);
void XTpg_SetZPlateVSpeed(XTpg *InstancePtr, u16 ZPlateVSpeed);
u16 XTpg_GetZPlateVSpeed(XTpg *InstancePtr);
int XTpg_SetBoxSize(XTpg *InstancePtr, u32 BoxSize);
u32 XTpg_GetBoxSize(XTpg *InstancePtr);
void XTpg_SetBoxColor(XTpg *InstancePtr, u16 Blue, u16 Green, u16 Red);
void XTpg_GetBoxColor(XTpg *InstancePtr, u16 *Blue, u16 *Green, u16 *Red);
void XTpg_SetStuckPixelThreshold(XTpg *InstancePtr, u32 PixelThreshold);
u32 XTpg_GetStuckPixelThreshold (XTpg *InstancePtr);
void XTpg_SetNoiseGain(XTpg *InstancePtr, u32 NoiseGain);
u32 XTpg_GetNoiseGain(XTpg *InstancePtr);
void XTpg_SetBayerPhase(XTpg *InstancePtr,
		enum XTpg_BayerPhaseCombination BayerPhaseComb);
u32 XTpg_GetBayerPhase(XTpg *InstancePtr);

/*
 * SelfTest function implemented in xtpg_selftest.c
 */
int XTpg_SelfTest(XTpg *InstancePtr);

/*
 * Interrupt related functions implemented in xtpg_intr.c
 */
void XTpg_IntrHandler(void *InstancePtr);
int XTpg_SetCallBack(XTpg *InstancePtr, u32 HandlerType, void *CallBackFunc,
			void *CallBackRef);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}

#endif

#endif /* End of protection macro */
/** @} */
