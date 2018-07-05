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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xcresample.h
* @addtogroup cresample_v4_1
* @{
* @details
*
* This header file contains identifiers and register-level driver functions
* (or macros), that can be used to access the Xilinx Chroma Resampler
* (Cresample) core instance.
*
* The Chroma Resampler core converts between chroma sub-sampling formats of
* 4:4:4, 4:2:2,and 4:2:0. There are a total of six conversions available for
* the three supported sub-sampling formats.
* Conversion is achieved using a FIR filter approach. Some conversions require
* filtering in only the horizontal dimension, only the vertical dimension,
* or both.
* Interpolation operations are implemented using a two-phase polyphase
* FIR filter. Decimation operations are implemented using a low-pass FIR filter
* to suppress chroma aliasing.
*
* Features of Chroma Resampler core are
*	- Configurable filters sizes with programmable filter coefficients for
*	  high performance applications.
*	- Replicate or drop pixels.
*	- Static, predefined, powers-of-two coefficients for low-footprint
*	  applications
*	- Converts between YCbCr: 4:4:4, 4:2:2,and 4:2:0.
*	- Supports both progressive and interlaced video.
*
* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the Cresample core.
*
* XCresample_CfgInitialize() API is used to initialize the Cresample core.
* The user needs to first call the XCresample_LookupConfig() API which returns
* the Configuration structure pointer which is passed as a parameter to the
* XCresample_CfgInitialize() API.
*
* <b> Interrupts </b>
*
* The driver provides an interrupt handler XCresample_IntrHandler for handling
* the interrupt from the Cresample core. The users of this driver have to
* register this handler with the interrupt system and provide the callback
* functions by using XCresample_SetCallBack API.
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
* The Cresample driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- -------------------------------------------------------
* 2.00a vy      04/24/12 Updated for version 2.00.a
*                        Converted from xio.h to xil_io.h, translating
*                        basic type, MB cache functions, exceptions and
*                        assertion to xil_io format.
* 1.00a vy      10/22/10 Initial version
* 3.0   adk     19/12/13 Updated as per the New Tcl API's.
* 4.0   adk     03/12/14 Changed the file name cresample.h to xcresample.h.
*                        Macros of register offsets, bit definitions,
*                        ReadReg and WriteReg macros which are the part of
*                        xcresample_hw.h.were moved to xcresample_hw.h.
*                        Removed file inclusion of Xil_io.h.
*
*                        Removed the following functional macros:
*                        CRESAMPLE_Enable, CRESAMPLE_Disable,
*                        CRESAMPLE_RegUpdateEnable, CRESAMPLE_RegUpdateDisable,
*                        CRESAMPLE_Reset, CRESAMPLE_ClearReset,
*                        CRESAMPLe_AutoSyncReset.
*                        Defined the following functional macros:
*                        XCresample_Enable, XCresample_Disable,
*                        XCresample_RegUpdateEnable,XCresample_RegUpdateDisable
*                        XCresample_Reset, XCresample_ClearReset,
*                        XCresample_SyncReset, XCresample_IntrDisable,
*                        XCresample_IntrEnable, XCresample_StatusGetPending
*                        XCresample_IntrGetPending, XCresample_IntrClear.
*                        Declared following structures:
*                        XCresample_Config and XCresample of Chroma Resampler.
*
*                        Defined the following range macros:
*                        XCRE_VSIZE_FIRST, XCRE_VSIZE_LAST, XCRE_HSIZE_FIRST,
*                        XCRE_HSIZE_FIRST, XCRE_HSIZE_LAST, XCRE_PARITY_ODD,
*                        XCRE_PARITY_EVEN, XCRE_COEF_FIRST, XCRE_COEF_LAST,
*                        XCRE_OFFSET_DIFF, XCRE_NUM_OF_PHASES, XCRE_NUM_HCOEFS,
*                        and XCRE_NUM_VCOEFS.
*
*                        Modifications in the file xcresample_hw.h are:
*                        Added the register offsets and bit masks for the
*                        registers and added backward compatibility for macros.
*
*                        Modifications in the file xcresample.c are:
*                        Changed the filename cresample.c to xcresample.c.
*                        The Following functions are removed:
*                        clear_coef_values, configure_444_to_422,
*                        configure_422_to_444, configure_422_to_420,
*                        configure_420_to_422, configure_444_to_420 and
*                        configure_420_to_444.
*                        Implemented the following functions :
*                        StubCallBack, StubErrorCallBack,
*                        XCresample_CfgInitialize,
*                        XCresample_GetVersion, XCresample_EnableDbgByPass,
*                        XCresample_IsDbgByPassEnabled,
*                        XCresample_DisableDbgByPass,
*                        XCresample_SetDbgTestPattern,
*                        XCresample_IsDbgTestPatternEnabled,
*                        XCresample_DisableDbgTestPattern
*                        XCresample_GetDbgFrameCount,
*                        XCresample_GetDbgLineCount,
*                        XCresample_GetDbgPixelCount,
*                        XCresample_SetActiveSize, XCresample_GetActiveSize,
*                        XCresample_SetFieldParity, XCresample_GetFieldParity,
*                        XCresample_SetChromaParity,
*                        XCresample_GetChromaParity
*                        XCresample_SetHCoefs, XCresample_GetHCoefs,
*                        XCresample_SetVCoefs, XCresample_GetVCoefs,
*                        XCresample_Clear_HCoef_Values, and
*                        XCresample_Clear_VCoef_Values.
*
*                        Modifications in the file xcresample_intr.c are:
*                        Implemented XCresample_IntrHandler and
*                        XCresample_SetCallBack functions.
*
*                        Modifications in the file xcresample_selftest.c are:
*                        Implemented XCresample_SelfTest function.
*
*                        Modifications in the file xcresample_sinit.c are:
*                        Implemented XCresample_LookupConfig function.
* 4.1   ms      01/16/17 Updated the parameter naming from
*                        XPAR_CRESAMPLE_NUM_INSTANCES to
*                        XPAR_XCRESAMPLE_NUM_INSTANCES to avoid compilation
*                        failure for XPAR_CRESAMPLE_NUM_INSTANCES as the
*                        tools are generating XPAR_XCRESAMPLE_NUM_INSTANCES
*                        in the generated xcresample_g.c for fixing MISRA-C
*                        files. This is a fix for CR-966099 based on the
*                        update in the tools.
*       ms      03/17/17 Added readme.txt file in examples folder for doxygen
*                        generation.
* </pre>
*
******************************************************************************/

#ifndef XCRESAMPLE_H_
#define XCRESAMPLE_H_	/**< Prevent circular inclusions by using
			  *  protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_assert.h"
#include "xstatus.h"
#include "xcresample_hw.h"

/************************** Constant Definitions *****************************/

/** @name Interrupt types for setting up callbacks
* @{
*/
/**
 * These constants specify different types of handler and used to differentiate
 * interrupt requests from core.
 *
 */
enum {
	XCRE_HANDLER_PROCSTART = 1,	/**< A processing start event interrupt
					  *  type */
	XCRE_HANDLER_FRAMEDONE,		/**< A frame done event interrupt
					  *  type */
	XCRE_HANDLER_ERROR		/**< An error condition interrupt
					  *  type */
} ;
/*@}*/

/** @name Active size ranges
* @{
*/
#define XCRE_ACT_SIZE_FIRST	32	/**< Active Size starting value */
#define XCRE_ACT_SIZE_LAST	7680	/**< Active Size ending value */
/*@}*/

/** @name Parity valid values
* @{
*/
#define XCRE_PARITY_ODD		1	/**< For odd (or top)
						  *  field it is 1 */
#define XCRE_PARITY_EVEN	0	/**< For even (or bottom)
						  *  field it is 0 */
/*@}*/

/** @name Coefficient ranges
* @{
*/
#define XCRE_COEF_FIRST		-2.0	/**< Coefficient start value */
#define XCRE_COEF_LAST		2.0	/**< Coefficient end value */
#define XCRE_OFFSET_DIFF	4	/**< Coefficient offset
					  *  difference	 */
/*@}*/

/** @name Number of phases
* @{
*/
#define XCRE_NUM_OF_PHASES	2	/**< Number of phases */
/*@}*/

/** @name Number of coefficients
* @{
*/
#define XCRE_NUM_HCOEFS		24	/**< Number of horizontal
					  *  coefficients */
#define XCRE_NUM_VCOEFS		8	/**< Number of vertical
					  *  coefficients */
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro enables the Chroma Resampler core.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCresample_Enable(XCresample *InstancePtr)
*
******************************************************************************/
#define XCresample_Enable(InstancePtr) \
	XCresample_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCRE_CONTROL_OFFSET), \
		((XCresample_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCRE_CONTROL_OFFSET))) | (XCRE_CTL_SW_EN_MASK)))

/*****************************************************************************/
/**
*
* This macro disables the Chroma Resampler core.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCresample_Disable(XCresample *InstancePtr)
*
******************************************************************************/
#define XCresample_Disable(InstancePtr) \
	XCresample_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCRE_CONTROL_OFFSET), \
		((XCresample_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCRE_CONTROL_OFFSET))) & (~(XCRE_CTL_SW_EN_MASK))))

/*****************************************************************************/
/**
*
* This macro enables/starts the Chroma Resampler core.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCresample_Start(XCresample *InstancePtr)
*
******************************************************************************/
#define XCresample_Start	XCresample_Enable

/*****************************************************************************/
/**
*
* This macro disables/stops the Chroma Resampler core.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCresample_Stop(XCresample *InstancePtr)
*
******************************************************************************/
#define XCresample_Stop		XCresample_Disable

/*****************************************************************************/
/**
*
* This macro commits all the register value changes made so far
* by the software to the Chroma Resampler core.
*
* This macro only works when the Chroma Resampler core is enabled.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	None.
*
* @note 	C-style signature:
* 		void XCresample_RegUpdateEnable(XCresample *InstancePtr)
*
******************************************************************************/
#define XCresample_RegUpdateEnable(InstancePtr) \
	XCresample_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCRE_CONTROL_OFFSET), \
		((XCresample_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCRE_CONTROL_OFFSET))) | (XCRE_CTL_RUE_MASK)))

/*****************************************************************************/
/**
*
* This macro prevents the Chroma Resampler core from committing
* recent changes made so far by the software. When disabled, changes to other
* configuration registers are stored, but do not effect the behavior of the
* core.
*
* This macro only works when the Chroma Resampler core is enabled.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XCresample_RegUpdateDisable(XCresample *InstancePtr)
*
******************************************************************************/
#define XCresample_RegUpdateDisable(InstancePtr) \
	XCresample_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCRE_CONTROL_OFFSET), \
		((XCresample_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCRE_CONTROL_OFFSET))) & (u32)(~(XCRE_CTL_RUE_MASK))))

/*****************************************************************************/
/**
*
* This macro resets a Chroma Resampler core instance, but differs from
* XCresample_Reset() in that it automatically synchronizes to the SOF input of
* the core to prevent tearing.
*
* On the next raising edge of SOF following a call to XCresample_SyncReset(),
* all of the core's configuration registers and outputs will be reset, then
* the reset flag will be immediately released, allowing the core to immediately
* resume default operation.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	None.
*
* @note 	C-style signature:
*		void XCresample_SyncReset(XCresample *InstancePtr)
*
******************************************************************************/
#define XCresample_SyncReset(InstancePtr) \
	XCresample_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XCRE_CONTROL_OFFSET), (XCRE_CTL_AUTORESET_MASK))

/*****************************************************************************/
/**
*
* This macro resets the Chroma Resampler core. This reset effects the core
* immediately, and may cause image tearing.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	None.
*
* @note 	C-style signature:
*		void XCresample_Reset(XCresample *InstancePtr)
*
******************************************************************************/
#define XCresample_Reset(InstancePtr) \
	XCresample_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XCRE_CONTROL_OFFSET), (XCRE_CTL_RESET_MASK))

/*****************************************************************************/
/**
*
* This macro enables individual interrupts of the Cresample core by
* updating the IRQ_ENABLE register.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
* @param	IntrType is the type of the interrupts to enable. Use OR'ing of
* 		XCRE_IXR_*_MASK constants defined in xcresample_hw.h to create
*		this parameter value.
*
* @return	None
*
* @note		The existing enabled interrupt(s) will remain enabled.
*		C-style signature:
*		void XCresample_IntrEnable(XCresample *InstancePtr,
								u32 IntrType)
*
******************************************************************************/
#define XCresample_IntrEnable(InstancePtr, IntrType) \
	XCresample_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCRE_IRQ_EN_OFFSET), \
			(((IntrType) & (XCRE_IXR_ALLINTR_MASK)) | \
		(XCresample_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCRE_IRQ_EN_OFFSET)))))

/*****************************************************************************/
/**
*
* This macro disables individual interrupts of the Cresample core by
* updating the IRQ_ENABLE register.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
* @param	IntrType is the type of the interrupts to disable. Use OR'ing
*		of XCRE_IXR_*_MASK constants defined in xcresample_hw.h to
*		create this parameter value.
*
* @return	None
*
* @note		Any other interrupt not covered by parameter IntrType,
*		if enabled before this macro is called, will remain enabled.
*		C-style signature:
* 		void XCresample_IntrDisable(XCresample *InstancePtr,
*								u32 IntrType)
*
******************************************************************************/
#define XCresample_IntrDisable(InstancePtr, IntrType) \
	XCresample_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XCRE_IRQ_EN_OFFSET), \
		((XCresample_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XCRE_IRQ_EN_OFFSET))) \
				& ((~(IntrType)) & (XCRE_IXR_ALLINTR_MASK))))

/*****************************************************************************/
/**
*
* This macro returns the pending interrupt status of the Cresample
* core read from the Status register.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	The pending interrupts of the Chroma Resampler. Use
*		XCRE_IXR_*_MASK constants defined in xcresample_hw.h to
*		interpret this value.
*
* @note		C-style signature:
* 		u32 XCresample_StatusGePending(XCresample *InstancePtr)
*
******************************************************************************/
#define XCresample_StatusGetPending(InstancePtr) \
	XCresample_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XCRE_STATUS_OFFSET)) & (XCRE_IXR_ALLINTR_MASK)

/*****************************************************************************/
/**
*
* This macro returns the pending interrupts of the Cresample core for
* the interrupts that have been enabled.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
*
* @return	The pending interrupts of the Chroma Resampler. Use
* 		XCRE_IXR_*_MASK constants defined in xcresample_hw.h to
*		interpret this value.
*
* @note		C-style signature:
*		u32 XCresample_IntrGetPending(XCresample *InstancePtr)
*
******************************************************************************/
#define XCresample_IntrGetPending(InstancePtr) \
	XCresample_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XCRE_IRQ_EN_OFFSET)) & \
		((XCresample_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XCRE_STATUS_OFFSET))) & ((u32)(XCRE_IXR_ALLINTR_MASK)))

/*****************************************************************************/
/**
*
* This macro clears/acknowledges pending interrupts of the Cresample
* core in the Status register. Bit positions of 1 will be cleared.
*
* @param	InstancePtr is a pointer to the XCresample instance to be
*		worked on.
* @param	IntrType is the pending interrupts to clear/acknowledge.
*		Use OR'ing of XCRE_IXR_*_MASK constants defined in
*		xcresample_hw.h to create this parameter value.
*
* @return	None
* @note		C-style signature:
*		void XCresample_IntrClear(XCresample *InstancePtr,
*		u32 IntrType)
*
******************************************************************************/
#define XCresample_IntrClear(InstancePtr, IntrType) \
	XCresample_WriteReg((InstancePtr)->Config.BaseAddress, \
					(XCRE_STATUS_OFFSET), \
			((IntrType) & ((u32)(XCRE_IXR_ALLINTR_MASK))))

/**************************** Type Definitions *******************************/
/**
*
* This typedef contains configuration information for a Video device.
* Each Video device should have a configuration structure associated
*/
typedef struct {
	u16 DeviceId;			/**< DeviceId is the unique ID of
					  *  the device */
	u32 BaseAddress;		/**< BaseAddress is the physical
					  *  base address of the device's
					  *  registers */
	u32 SlaveAxisVideoFormat;	/**< Slave axis video format */
	u32 MasterAxisVideoFormat;	/**< Master axis video format */
	u32 SlaveAxiClkFreqHz;		/**< Slave clock frequency */
	u16 HasIntcIf;			/**< Has interrupt control */
	u16 HasDebug;			/**< Has debug GUI specified */
	u32 MaxColumns;			/**< Maximum columns */
	u32 ActiveRows;			/**< Active rows */
	u32 ActiveColumns;		/**< Active columns */
	u8 ChromaParity;		/**< Chroma parity */
	u8 FieldParity;			/**< Chroma parity */
	u32 Interlaced;			/**< Interlaced value */
	u32 NumHTaps;			/**< Horizontal taps */
	u32 NumVTaps;			/**< Vertical taps */
	u32 ConvertType;		/**< Convert type */
	u32 CoefWidth;			/**< Coefficient width */
} XCresample_Config;

/*****************************************************************************/
/**
*
* Callback type for all interrupts except error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
*/
typedef void (*XCresample_CallBack)(void *CallBackRef);

/*****************************************************************************/
/**
*
* Callback type for Error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back
*		to the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
*		value equals 'OR'ing one or more XCRE_IXR_* values
*		defined in xcresample_hw.h
*/
typedef void (*XCresample_ErrorCallBack)(void *CallBackRef, u32 ErrorMask);

/*****************************************************************************/
/**
*
* The XCresample driver instance data structure. A pointer to an instance
* data structure is passed around by functions to refer to a specific driver
* instance.
*/
typedef struct {
	XCresample_Config Config;	/**< Hardware configuration */
	u32 IsReady;			/**< Device and the driver instance
					  *  are initialized */
	u16 HSize;			/**< Active video horizontal size */
	u16 VSize;			/**< Active video vertical size */

	/* IRQ Callbacks Here */
 	XCresample_CallBack ProcStartCallBack; /**< Call back for processing
						 *  start interrupt */
	void *ProcStartRef;		/**< To be passed to the process start
					  *  interrupt callback */

	XCresample_CallBack FrameDoneCallBack; /**< Call back for frame done
						 *  interrupt */

	void *FrameDoneRef;	/**< To be passed to the frame done
				  *  interrupt callback */

	XCresample_ErrorCallBack ErrCallBack; /**< Call back for error
						*  interrupt */
	void *ErrRef;		/**< To be passed to the error
				  *  interrupt callback */
	u32 CoefH[XCRE_NUM_OF_PHASES][XCRE_NUM_HCOEFS];	/**< Horizontal filter
							  *  coefficients */
	u32 CoefV[XCRE_NUM_OF_PHASES][XCRE_NUM_VCOEFS];	/**< Vertical filter
							  *  coefficients */
} XCresample;

/*****************************************************************************/
/**
*
* The XCoeffs is a structure contains predefined fixed coefficient values for
* Horizontal filter of two phases (phase 0 and phase 1).
* This can be used for setting or getting coefficient values from
* XCresample_SetHCoefs and XCresample_GetHCoefs APIs.
*/
typedef struct {
	float HCoeff[XCRE_NUM_OF_PHASES][XCRE_NUM_HCOEFS];/**< Matrix for
							    *  Horizontal
							    *  Coefficients */
} XHorizontal_Coeffs;
/*****************************************************************************/
/**
*
* The XCoeffs is a structure contains predefined fixed coefficient values for
* Vertical filter of two phases (phase 0 and phase 1).
* This can be used for setting or getting coefficient values from
* XCresample_SetVCoefs and XCresample_GetVCoefs APIs.
*/
typedef struct {
	float VCoeff[XCRE_NUM_OF_PHASES][XCRE_NUM_VCOEFS];/**< Matrix for
							    *  Vertical
							    *  Coefficients */
} XVertical_Coeffs;

/************************** Function Prototypes ******************************/

/* Initialization and control functions implemented in xcresample.c */
int XCresample_CfgInitialize(XCresample *InstancePtr,
				XCresample_Config *CfgPtr, u32 EffectiveAddr);
u32 XCresample_GetVersion(XCresample *InstancePtr);
void XCresample_EnableDbgByPass(XCresample *InstancePtr);
int XCresample_IsDbgByPassEnabled(XCresample *InstancePtr);
void XCresample_DisableDbgBypass(XCresample *InstancePtr);
void XCresample_EnableDbgTestPattern(XCresample *InstancePtr);
int XCresample_IsDbgTestPatternEnabled(XCresample *InstancePtr);
void XCresample_DisableDbgTestPattern(XCresample *InstancePtr);
u32 XCresample_GetDbgFrameCount(XCresample *InstancePtr);
u32 XCresample_GetDbgLineCount(XCresample *InstancePtr);
u32 XCresample_GetDbgPixelCount(XCresample *InstancePtr);
void XCresample_SetActiveSize(XCresample *InstancePtr, u16 HSize, u16 VSize);
void XCresample_GetActiveSize(XCresample *InstancePtr, u16 *HSize, u16 *VSize);
void XCresample_SetFieldParity(XCresample *InstancePtr, u8 FieldParity);
void XCresample_SetChromaParity(XCresample *InstancePtr, u8 ChromaParity);
u8 XCresample_GetFieldParity(XCresample *InstancePtr);
u8 XCresample_GetChromaParity(XCresample *InstancePtr);
void XCresample_SetHCoefs(XCresample *InstancePtr, XHorizontal_Coeffs *Coeff,
							u32 Phases);
void XCresample_SetVCoefs(XCresample *InstancePtr, XVertical_Coeffs *Coeff,
							u32 Phases);
void XCresample_GetHCoefs(XCresample *InstancePtr, XHorizontal_Coeffs *Coeff);
void XCresample_GetVCoefs(XCresample *InstancePtr, XVertical_Coeffs *Coeff);

/* Static lookup function implemented in xcresample_sinit.c */
XCresample_Config *XCresample_LookupConfig(u16 DeviceId);

/* Self - Test functions in xcresample_selftest.c */
int XCresample_SelfTest(XCresample *InstancePtr);

/* Interrupt related functions in xcresample_intr.c */
int XCresample_SetCallBack(XCresample *InstancePtr, u32 HandlerType,
				void *CallBackFunc, void *CallBackRef);
void XCresample_IntrHandler(void *InstancePtr);

/* Clear coefficients function */
void XCresample_Clear_HCoef_Values(XCresample *InstancePtr);
void XCresample_Clear_VCoef_Values (XCresample *InstancePtr);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}

#endif

#endif /* end of protection macro */
/** @} */
