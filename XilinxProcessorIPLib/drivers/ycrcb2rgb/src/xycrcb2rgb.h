/******************************************************************************
* Copyright (c) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xycrcb2rgb.h
* @addtogroup ycrcb2rgb_v7_2
* @{
* @details
*
* This header file contains identifiers and register-level driver functions
* (or macros), range macros, structure typedefs that can be used to access the
* Xilinx YCRCB2RGB core.
*
* The YCrCb to RGB Color-Space Convertor core is a simplified 3x3 matrix
* multiplier converts three input color samples to three output samples in a
* single clock cycle. The core supports four common format conversions as well
* as a custom mode that allows for a user-defined transform. The core is
* capable of a maximum resolution of 7680 columns by 7680 rows with 8, 10, 12,
* or 16 bits per pixel.
*
* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the YCRCB2RGB core.
*
* XYCrCb2Rgb_CfgInitialize() API is used to initialize the YCRCB2RGB core.
* The user needs to first call the XYCrCb2Rgb_LookupConfig() API which returns
* the Configuration structure pointer which is passed as a parameter to the
* XYCrCb2Rgb_CfgInitialize() API.
*
* <b> Interrupts </b>
*
* The driver provides an interrupt handler XYCrCb2Rgb_IntrHandler for handling
* the interrupt from the YCRCB2RGB core. The users of this driver have to
* register this handler with the interrupt system and provide the callback
* functions by using XYCrCb2Rgb_SetCallBack API.
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
* The XYCrCb2Rgb driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 5.00a tb     02/28/12 Updated for YCRCB2RGB V5.00.a
* 5.01a bao    12/28/12 Converted from xio.h to xil_io.h, translating basic
*                       types, MB cache functions, exceptions and assertions
*                       to xil_io format.
* 6.0   adk    19/12/13 Updated as per the New Tcl API's
* 7.0   adk    01/31/14 Changed the file name from "ycrcb2rgb.h" to
*                       "xycrcb2rgb.h".
*
*                       Moved register offsets and bit definitions from
*                       ycrcb2rgb.h file to xycrcb2rgb_hw.h file.
*
*                       Removed YCC_TIMING_STATUS register offset because this
*                       register is not present in latest product guide.
*
*                       Removed following function macros:
*                       YCC_Enable, YCC_Disable, YCC_RegUpdateEnable,
*                       YCC_RegUpdateDisable, YCC_Reset, YCC_ClearReset,
*                       YCC_AutoSyncReset, ycc_max..
*
*                       Removed following functions:
*                       YCC_select_standard, YCC_coefficient_translation,
*                       YCC_set_coefficients, YCC_get_coefficients.
*
*                       Added following handler types as enum:
*                       XYCC_HANDLER_PROCSTART, XYCC_HANDLER_FRAMEDONE,
*                       XYCC_HANDLER_ERROR.
*
*                       Added following standard types as enum:
*                       XYCC_STANDARD_ITU_601_SD, XYCC_STANDARD_ITU_709_NTSC,
*                       XYCC_STANDARD_ITU_709_PAL, XYCC_STANDARD_YUV,
*                       XYCC_STANDARD_CUSTOM.
*
*                       Added following output ranges as enum:
*                       XYCC_TV_16_TO_240, XYCC_STUDIO_16_TO_235,
*                       XYCC_GRAPHICS_0_TO_255.
*
*                       Added range macros for ActiveSize, RGBMAX, RGBMIN,
*                       ROFFSET, GOFFSET, BOFFSET registers.
*
*                       Added following function macros:
*                       XYCrCb2Rgb_IntrEnable, XYCrCb2Rgb_IntrDisable,
*                       XYCrCb2Rgb_StatusGetPending, XYCrCb2Rgb_IntrGetPending,
*                       XYCrCb2Rgb_IntrClear, XYCrCb2Rgb_Reset,
*                       XYCrCb2Rgb_Enable, XYCrCb2Rgb_Disable,
*                       XYCrCb2Rgb_Start, XYCrCb2Rgb_Stop,
*                       XYCrCb2Rgb_RegUpdateEnable,
*                       XYCrCb2Rgb_RegUpdateDisable,
*                       XYCrCb2Rgb_SyncReset, XYCrCb2Rgb_Max
*
*                       Added core, configuration and coefficient structure.
*                       Renamed ycc_coef_inputs -> XYCrCb2Rgb_Coef_Inputs and
*                       ycc_coef_outputs - > XYCrCb2Rgb_Coef_Outputs.
*
*                       Added callback functions typedef.
*
*                       Implemented XYCrCb2Rgb_LookupConfig in
*                       xycrcb2rgb_sinit.c
*                       Implemented XYCrCb2Rgb_SelfTest in
*                       xycrcb2rgb_selftest.c
*                       Implemented XYCrCb2Rgb_IntrHandler,
*                       XYCrCb2Rgb_SetCallBack in xycrcb2rgb_intr.c.
*
*                       Implemented following functions in xycrcb2rgb.c:
*                       XYCrCb2Rgb_CfgInitialize, XYCrCb2Rgb_EnableDbgByPass,
*                       XYCrCb2Rgb_IsDbgByPassEnabled,
*                       XYCrCb2Rgb_DisableDbgBypass,
*                       XYCrCb2Rgb_EnableDbgTestPattern,
*                       XYCrCb2Rgb_IsDbgTestPatternEnabled,
*                       XYCrCb2Rgb_DisableDbgTestPattern,
*                       XYCrCb2Rgb_GetVersion, XYCrCb2Rgb_GetDbgFrameCount,
*                       XYCrCb2Rgb_GetDbgLineCount,
*                       XYCrCb2Rgb_GetDbgPixelCount, XYCrCb2Rgb_Setup,
*                       XYCrCb2Rgb_SetActiveSize, XYCrCb2Rgb_GetActiveSize,
*                       XYCrCb2Rgb_SetRGBMax, XYCrCb2Rgb_GetRGBMax,
*                       XYCrCb2Rgb_SetRGBMin, XYCrCb2Rgb_GetRGBMin,
*                       XYCrCb2Rgb_SetROffset, XYCrCb2Rgb_GetROffset,
*                       XYCrCb2Rgb_SetGOffset, XYCrCb2Rgb_GetGOffset,
*                       XYCrCb2Rgb_SetBOffset, XYCrCb2Rgb_GetBOffset,
*                       XYCrCb2Rgb_SetCoefs, XYCrCb2Rgb_GetCoefs,
*                       XYCrCb2Rgb_Select_Standard,
*                       XYCrCb2Rgb_Coefficient_Translation,
*                       XYCrCb2Rgb_Select_OutputRange.
* 7.1   ms     01/31/17 Updated the parameter naming from
*                       XPAR_YCRCB2RGB_NUM_INSTANCES to
*                       XPAR_XYCRCB2RGB_NUM_INSTANCES to avoid compilation
*                       failure for XPAR_YCRCB2RGB_NUM_INSTANCES
*                       as the tools are generating
*                       XPAR_XYCRCB2RGB_NUM_INSTANCES in the generated
*                       xycrcb2rgb_g.c for fixing MISRA-C files. This is a
*                       fix for CR-967548 based on the update in the tools.
*       ms     03/17/17 Added readme.txt file in examples folder for doxygen
*                       generation.
*</pre>
*
******************************************************************************/

#ifndef XYCRCB2RGB_H_
#define XYCRCB2RGB_H_	/**< Prevent circular inclusions by using protection
			  *  macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xycrcb2rgb_hw.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/** @name Handler Types
 * @{
 */
/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from core.
*/
enum {
	XYCC_HANDLER_PROCSTART = 1,	/**< A processing start event interrupt
					  *  type */
	XYCC_HANDLER_FRAMEDONE,		/**< A frame done event interrupt
					  *  type */
	XYCC_HANDLER_ERROR		/**< An error condition interrupt
					  *  type */
} ;
/*@}*/

/** @name Standard Types
 * @{
 */
/**
*
* These constants specify different types of standards used to represent
* standard color encoding.
*/
enum XYcc_Standards {
	XYCC_STANDARD_ITU_601_SD = 0,	 /**< Standard ITU 601 SD. */
	XYCC_STANDARD_ITU_709_NTSC,	 /**< Standard ITU 709 NTSC. */
	XYCC_STANDARD_ITU_709_PAL,	 /**< Standard ITU 709 PAL. */
	XYCC_STANDARD_YUV,		 /**< Standard YUV. */
	XYCC_STANDARD_CUSTOM		 /**< Standard Custom. */
} ;
/*@}*/

/** @name Output Ranges
 * @{
 */
/**
*
* These constants specify different ranges used for studio equipment,
* television and computer graphics respectively.
*/
enum XYcc_OutputRanges {
	XYCC_TV_16_TO_240 = 0,	/**< 16 to 240, for Television. */
	XYCC_STUDIO_16_TO_235,	/**< 16 to 235, for Studio Equipment. */
	XYCC_GRAPHICS_0_TO_255	/**< 0 to 255, for Computer Graphics. */
} ;
/*@}*/

/**  Active Size (VxH) range macros
 * @{
 */
#define XYCC_ACT_SIZE_FIRST	32	/**< Starting value for H/V */
#define XYCC_ACT_SIZE_LAST	8192	/**< Ending value for H/V */
/*@}*/

/**  RGBMAX, RGBMIN range macros
 * @{
 */
#define XYCC_RGBMAX_MIN_FIRST	0	/**< Starting value for
					  *  RGBMAX/RGBMIN */
#define XYCC_RGBMAX_MIN_LAST	255	/**< Ending value for RGBMAX/RGBMIN */
/*@}*/


/** ROFFSET, GOFFSET, BOFFSET range macros
 * @{
 */
#define XYCC_RGBOFFSET_FIRST	0	/**< Starting value for ROFFSET/
					  *  GOFFSET/BOFFSET */
#define XYCC_RGBOFFSET_LAST	255	/**< Ending value for ROFFSET/
					  *  GOFFSET/BOFFSET */
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro enables the YCrCb2Rgb core.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XYCrCb2Rgb_Enable(*XYCrCb2Rgb InstancePtr)
*
******************************************************************************/
#define XYCrCb2Rgb_Enable(InstancePtr) \
	XYCrCb2Rgb_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XYCC_CONTROL_OFFSET), \
		XYCrCb2Rgb_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XYCC_CONTROL_OFFSET)) | (XYCC_CTL_SW_EN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the YCrCb2Rgb core.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XYCrCb2Rgb_Disable(*XYCrCb2Rgb InstancePtr)
*
******************************************************************************/
#define XYCrCb2Rgb_Disable(InstancePtr) \
	XYCrCb2Rgb_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XYCC_CONTROL_OFFSET), \
		(XYCrCb2Rgb_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XYCC_CONTROL_OFFSET)) & (~(XYCC_CTL_SW_EN_MASK))))

/*****************************************************************************/
/**
*
* This macro starts the YCrCb2Rgb core.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XYCrCb2Rgb_Start(*XYCrCb2Rgb InstancePtr)
*
******************************************************************************/
#define XYCrCb2Rgb_Start	XYCrCb2Rgb_Enable

/*****************************************************************************/
/**
*
* This macro stops the YCrCb2Rgb core.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XYCrCb2Rgb_Stop(*XYCrCb2Rgb InstancePtr)
*
******************************************************************************/
#define XYCrCb2Rgb_Stop		XYCrCb2Rgb_Disable

/*****************************************************************************/
/**
*
* This macro enables copying from updated processor register values to
* the active set at the end of each AXI-Stream frame.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XYCrCb2Rgb_RegUpdateEnable(*XYCrCb2Rgb InstancePtr)
*
******************************************************************************/
#define XYCrCb2Rgb_RegUpdateEnable(InstancePtr) \
	XYCrCb2Rgb_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XYCC_CONTROL_OFFSET), \
		(XYCrCb2Rgb_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XYCC_CONTROL_OFFSET)) | (XYCC_CTL_RUE_MASK)))

/*****************************************************************************/
/**
*
* This macro disables copying from updated processor register values to
* the active set at the end of each AXI-Stream frame.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XYCrCb2Rgb_RegUpdateDisable(*XYCrCb2Rgb InstancePtr)
*
******************************************************************************/
#define XYCrCb2Rgb_RegUpdateDisable(InstancePtr) \
	XYCrCb2Rgb_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XYCC_CONTROL_OFFSET), \
		(XYCrCb2Rgb_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XYCC_CONTROL_OFFSET)) & (~(XYCC_CTL_RUE_MASK))))

/*****************************************************************************/
/**
*
* This macro resets a the YCrCb2Rgb core, but differs from XYCrCb2Rgb_Reset()
* in that it automatically synchronizes to the VBlank_in input of the core to
* prevent tearing.
*
* On the next rising-edge of VBlank_in following a call to
* XYCrCb2Rgb_SyncReset(), all of the core's configuration registers and
* outputs will be reset, then the reset flag will be immediately released,
* allowing the core to immediately resume default operation.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XYCrCb2Rgb_SyncReset(*XYCrCb2Rgb InstancePtr)
*
******************************************************************************/
#define XYCrCb2Rgb_SyncReset(InstancePtr) \
	XYCrCb2Rgb_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XYCC_CONTROL_OFFSET), (XYCC_CTL_AUTORESET_MASK))

/*****************************************************************************/
/**
*
* This macro resets the YCrCb2Rgb core.. This reset effects the core
* immediately and may cause image tearing.
*
* @param	InstancePtr is a pointer to the XYCrCb2Rgb instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XYCrCb2Rgb_Reset(XYCrCb2Rgb *InstancePtr)
*
******************************************************************************/
#define XYCrCb2Rgb_Reset(InstancePtr) \
	XYCrCb2Rgb_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XYCC_CONTROL_OFFSET), (XYCC_CTL_RESET_MASK))

/*****************************************************************************/
/**
*
* This function macro enables individual interrupts of the YCRCB2RGB core by
* updating the IRQ_ENABLE register.
*
* @param	InstancePtr is a pointer to the YCrCb2Rgb core instance
*		to be worked on.
* @param	IntrType is the bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XYCC_IXR_*_MASK bits defined in xycrcb2rgb_hw.h.
*
* @return	None
*
* @note		C-style signature:
*		void XYCrCb2Rgb_IntrEnable(XYCrCb2Rgb *InstancePtr,
*						u32 IntrType)
*
*		The existing enabled interrupt(s) will remain enabled.
*
******************************************************************************/
#define XYCrCb2Rgb_IntrEnable(InstancePtr, IntrType) \
	XYCrCb2Rgb_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XYCC_IRQ_EN_OFFSET), (((IntrType) & \
			(XYCC_IXR_ALLINTR_MASK)) | \
		(XYCrCb2Rgb_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XYCC_IRQ_EN_OFFSET)))))

/*****************************************************************************/
/**
*
* This function macro disables individual interrupts of the YCRCB2RGB core by
* updating the IRQ_ENABLE register.
*
* @param	InstancePtr is a pointer to the YCrCb2Rgb core instance
*		to be worked on.
* @param	IntrType is the bit-mask of the interrupts to be disabled.
*		Bit positions of 1 will be disabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XYCC_IXR_*_MASK bits defined in xycrcb2rgb_hw.h.
*
* @return	None
*
* @note		C-style signature:
*		void XYCrCb2Rgb_IntrDisable(XYCrCb2Rgb *InstancePtr,
*						u32 IntrType)
*
*		Any other interrupt not covered by parameter IntrType, if
*		enabled before this macro is called, will remain enabled.
*
******************************************************************************/
#define XYCrCb2Rgb_IntrDisable(InstancePtr, IntrType) \
	XYCrCb2Rgb_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XYCC_IRQ_EN_OFFSET),\
			XYCrCb2Rgb_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XYCC_IRQ_EN_OFFSET)) & \
				((~(IntrType)) & (XYCC_IXR_ALLINTR_MASK)))

/*****************************************************************************/
/**
*
* This function macro returns the pending interrupt status of the YCRCB2RGB
* core read from the Status register.
*
* @param	InstancePtr is a pointer to the YCrCb2Rgb core instance
*		to be worked on.
*
* @return	The status of pending interrupts of the YCRCB2RGB core.
* 		Use XYCC_IXR_*_MASK constants defined in xycrcb2rgb_hw.h to
*		interpret this value.
*
* @note		C-style signature:
*		XYCrCb2Rgb_StatusGePending(XYCC *InstancePtr)
*
******************************************************************************/
#define XYCrCb2Rgb_StatusGetPending(InstancePtr) \
	XYCrCb2Rgb_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XYCC_STATUS_OFFSET)) & (XYCC_IXR_ALLINTR_MASK)

/*****************************************************************************/
/**
*
* This function macro returns the pending interrupts of the YCRCB2RGB core for
* the interrupts that have been enabled.
*
* @param	InstancePtr is a pointer to the YCrCb2Rgb core instance
*		to be worked on.
*
* @return	The pending interrupts of the YCRCB2RGB core. Use
*		XYCC_IXR_*_MASK constants defined in xycrcb2rgb_hw.h to
*		interpret this value. The returned value is a logical AND of
*		the contents of the STATUS register and the IRQ_ENABLE
*		register.
*
* @note		C-style signature:
*		u32 XYCC_IntrGetPending(XYCC *InstancePtr)
*
******************************************************************************/
#define XYCrCb2Rgb_IntrGetPending(InstancePtr) \
	XYCrCb2Rgb_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XYCC_IRQ_EN_OFFSET)) & \
		(XYCrCb2Rgb_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XYCC_STATUS_OFFSET)) & \
					((u32)(XYCC_IXR_ALLINTR_MASK)))

/*****************************************************************************/
/**
*
* This function macro clears/acknowledges pending interrupts of the YCRCB2RGB
* core in the Status register. Bit positions of 1 will be cleared.
*
* @param	InstancePtr is a pointer to the YCRCB2RGB core instance
*		to be worked on.
*
* @param	IntrType is the pending interrupts to clear/acknowledge.
*		Use OR'ing of XYCC_IXR_*_MASK constants defined in
*		xycrcb2rgb_hw.h to create this parameter value.
*
* @return	None.
*
* @note		C-style signature:
*		void XYCrCb2Rgb_IntrClear(XYCC *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XYCrCb2Rgb_IntrClear(InstancePtr, IntrType) \
	XYCrCb2Rgb_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XYCC_STATUS_OFFSET), \
				(IntrType) & (XYCC_IXR_ALLINTR_MASK))

/*****************************************************************************/
/**
*
* This macro identifies maximum number from given numbers.
*
* @param	Num1 is a a u32 number.
* @param	Num2 is a a u32 number.
*
* @return	Maximum number from given two numbers.
*
* @note		C-style signature:
*		u32 XYCrCb2Rgb_Max(u32 Num1, u32 Num2)
*
******************************************************************************/
#define XYCrCb2Rgb_Max(Num1, Num2) (((Num1) > (Num2)) ? (Num1) : (Num2))

/*****************************************************************************/
/**************************** Type Definitions *******************************/

/**
* This typedef contains configuration information for a YCrCb2Rgb core.
* Each YCrCb2Rgb core should have a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;			/**< DeviceId is the unique ID
					  *  of the YCRCB2RGB core */
	u32 BaseAddress;		/**< BaseAddress is the physical base
					  *  address of the YCRCB2RGB core
					  *  registers */
	u32 SlaveAxisVideoFormat;	/**< Slave Axis Video Format */
	u32 MasterAxisVideoFormat;	/**< Master Axis Video Format */
	u16 HasDebug;			/**< Debugging support */
	u16 HasIntcIf;			/**< Interrupt controller support */
	u32 MaxCols;			/**< Maximum number of columns */
	u32 ActiveCols;			/**< Active number of columns */
	u32 ActiveRows;			/**< Active number of rows */
	u32 MWidth;			/**< MWidht */
	u32 CoefRange;			/**< Coefficient range */
	u32 ACoef;			/**< A Coefficient */
	u32 BCoef;			/**< B Coefficient */
	u32 CCoef;			/**< C Coefficient */
	u32 DCoef;			/**< D Coefficient */
	u32 ROffset;			/**< R Offset */
	u32 GOffset;			/**< G Offset */
	u32 BOffset;			/**< B Offset */
	u16 HasClip;			/**< Clipping support */
	u16 HasClamp;			/**< Clamping support  */
	u16 RgbMax;			/**< RGB Max value */
	u16 RgbMin;			/**< RGB Min value */
	u32 SlaveAxiClkFreqHz;		/**< Slave Axis Clock frequency
					  *  in Hz */
	u32 StandardSelection;		/**< Standard Selection */
	u32 OutputRange;		/**< Output Range */
} XYCrCb2Rgb_Config;

/*****************************************************************************/
/**
*
* Callback type for all interrupts except error interrupt.
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
typedef void (*XYCrCb2Rgb_CallBack)(void *CallBackRef);

/*****************************************************************************/
/**
*
* Callback type for error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
* 		value equals 'OR'ing one or more XYCC_IXR_*_MASK values defined
*		in xycrcb2rgb_hw.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
typedef void (*XYCrCb2Rgb_ErrorCallBack)(void *CallBackRef, u32 ErrorMask);

/*****************************************************************************/

/**
* The XYCrCb2Rgb driver instance data structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
typedef struct {
	XYCrCb2Rgb_Config Config;	/**< Hardware configuration */
	u32 IsReady;	/**< Device and the driver instance are initialized */
	u16 HSize;	/**< Active Video Horizontal Size */
	u16 VSize;	/**< Active Video Vertical Size */

	/* IRQ Callbacks Here */
	XYCrCb2Rgb_CallBack ProcStartCallBack;	/**< Callback for Processing
						  *  start interrupt */
	void *ProcStartRef;	/**< To be passed to Process start interrupt
				  *  callback */
	XYCrCb2Rgb_CallBack FrameDoneCallBack;	/**< Callback for Frame Done
						  *  interrupt */
	void *FrameDoneRef;	/**< To be passed to the Frame Done interrupt
				  *  callback */
	XYCrCb2Rgb_ErrorCallBack ErrCallBack;	/**< Callback for Error
						  *  interrupt */
	void *ErrRef;	/**< To be passed to the Error interrupt callback */
	u32 StandardSelection;	/**< Standard Selection */
	u32 OutputRange;	/**< Output Range */
} XYCrCb2Rgb;

/**
*
* YCrCb2Rgb input coefficient
*/
struct XYCrCb2Rgb_Coef_Inputs {
	/* Pre-translated coefficient/offset data */
	double ACoef;	/**< [ 0.0 - 1.0 ] 0.0 < ACOEFF + BCOEFF < 1.0 */
	double BCoef;	/**< [ 0.0 - 1.0 ] 0.0 < ACOEFF + BCOEFF < 1.0 */
	double CCoef;	/**< [ 0.0 - 1.0 ] */
	double DCoef;	/**< [ 0.0 - 1.0 ] */
	u32 YOffset;	/**< Offset for the Luminance Channel */
	u32 CbOffset;	/**< Offset for the Chrominance Channels */
	u32 CrOffset;	/**< Offset for the Chrominance Channels */
	u32 RgbMax;	/**< RGB Clipping */
	u32 RgbMin;	/**< RGB Clamping */
} ;

/**
*
* YCrCb2Rgb output coefficient
*/
struct XYCrCb2Rgb_Coef_Outputs {
	/* Translated coefficient/offset data */
	u32 ACoef;	/**< Translated ACoef */
	u32 BCoef;	/**< Translated BCoef */
	u32 CCoef;	/**< Translated CCoef */
	u32 DCoef;	/**< Translated DCoef */
	u32 ROffset;	/**< Translated Offset for the R Channel */
	u32 GOffset;	/**< Translated Offset for the G Channel */
	u32 BOffset;	/**< Translated Offset for the B Channel */
	u32 RgbMax;	/**< Translated RGB Clipping */
	u32 RgbMin;	/**< Translated RGB Clamping */
} ;

/**
*
* YCrCb2Rgb coefficients
*/
struct XYCrCb2Rgb_Coefficients {
	double ACoef;	/**< [ 0.0 - 1.0 ] 0.0 < ACOEFF + BCOEFF < 1.0 */
	double BCoef;	/**< [ 0.0 - 1.0 ] 0.0 < ACOEFF + BCOEFF < 1.0 */
	double CCoef;	/**< [ 0.0 - 1.0 ] */
	double DCoef;	/**< [ 0.0 - 1.0 ] */
} ;

/************************** Function Prototypes ******************************/

/*
 * Initialization function implemented in xycrcb2rgb_sinit.c
 */
XYCrCb2Rgb_Config *XYCrCb2Rgb_LookupConfig(u16 DeviceId);

/* Following functions implemented in xycrcb2rgb.c */
int XYCrCb2Rgb_CfgInitialize(XYCrCb2Rgb *InstancePtr,
		XYCrCb2Rgb_Config *CfgPtr, u32 EffectiveAddr);

void XYCrCb2Rgb_EnableDbgByPass(XYCrCb2Rgb *InstancePtr);
int XYCrCb2Rgb_IsDbgByPassEnabled(XYCrCb2Rgb *InstancePtr);
void XYCrCb2Rgb_DisableDbgBypass(XYCrCb2Rgb *InstancePtr);

void XYCrCb2Rgb_EnableDbgTestPattern(XYCrCb2Rgb *InstancePtr);
int XYCrCb2Rgb_IsDbgTestPatternEnabled(XYCrCb2Rgb *InstancePtr);
void XYCrCb2Rgb_DisableDbgTestPattern(XYCrCb2Rgb *InstancePtr);

u32 XYCrCb2Rgb_GetVersion(XYCrCb2Rgb *InstancePtr);

u32 XYCrCb2Rgb_GetDbgFrameCount(XYCrCb2Rgb *InstancePtr);
u32 XYCrCb2Rgb_GetDbgLineCount(XYCrCb2Rgb *InstancePtr);
u32 XYCrCb2Rgb_GetDbgPixelCount(XYCrCb2Rgb *InstancePtr);

void XYCrCb2Rgb_Setup(XYCrCb2Rgb *InstancePtr);

void XYCrCb2Rgb_SetActiveSize(XYCrCb2Rgb *InstancePtr, u16 HSize, u16 VSize);
void XYCrCb2Rgb_GetActiveSize(XYCrCb2Rgb *InstancePtr, u16 *HSize, u16 *VSize);

void XYCrCb2Rgb_SetRGBMax(XYCrCb2Rgb *InstancePtr, u32 RGBMax);
u32 XYCrCb2Rgb_GetRGBMax(XYCrCb2Rgb *InstancePtr);

void XYCrCb2Rgb_SetRGBMin(XYCrCb2Rgb *InstancePtr, u32 RGBMin);
u32 XYCrCb2Rgb_GetRGBMin(XYCrCb2Rgb *InstancePtr);

void XYCrCb2Rgb_SetROffset(XYCrCb2Rgb *InstancePtr, u32 ROffset);
u32 XYCrCb2Rgb_GetROffset(XYCrCb2Rgb *InstancePtr);

void XYCrCb2Rgb_SetGOffset(XYCrCb2Rgb *InstancePtr, u32 GOffset);
u32 XYCrCb2Rgb_GetGOffset(XYCrCb2Rgb *InstancePtr);

void XYCrCb2Rgb_SetBOffset(XYCrCb2Rgb *InstancePtr, u32 BOffset);
u32 XYCrCb2Rgb_GetBOffset(XYCrCb2Rgb *InstancePtr);

void XYCrCb2Rgb_SetCoefs(XYCrCb2Rgb *InstancePtr,
				struct XYCrCb2Rgb_Coefficients *Coef);
void XYCrCb2Rgb_GetCoefs(XYCrCb2Rgb *InstancePtr,
				struct XYCrCb2Rgb_Coefficients *Coef);

void XYCrCb2Rgb_Select_Standard(XYCrCb2Rgb *InstancePtr,
				enum XYcc_Standards StandardSel,
				enum XYcc_OutputRanges InputRange, u32 DataWidth,
				struct XYCrCb2Rgb_Coef_Inputs *CoefIn);

u32 XYCrCb2Rgb_Coefficient_Translation(XYCrCb2Rgb *InstancePtr,
			struct XYCrCb2Rgb_Coef_Inputs *CoefIn,
			struct XYCrCb2Rgb_Coef_Outputs *CoefOut,
			u32 DataWidth, u32 MWidth);

void XYCrCb2Rgb_Select_OutputRange(XYCrCb2Rgb *InstancePtr,
			enum XYcc_OutputRanges Range);

/* Self-test function implemented in xycrcb2rgb_selftest.c */
int XYCrCb2Rgb_SelfTest(XYCrCb2Rgb *InstancePtr);

/* Interrupt related functions implemented in xycrcb2rgb_intr.c */
void XYCrCb2Rgb_IntrHandler(void *InstancePtr);
int XYCrCb2Rgb_SetCallBack(XYCrCb2Rgb *InstancePtr, u32 HandlerType,
				void *CallBackFunc, void *CallBackRef);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
