/******************************************************************************
* Copyright (c) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrgb2ycrcb.h
* @addtogroup rgb2ycrcb_v7_2
* @{
* @details
*
* This header file contains identifiers and register-level driver functions (or
* macros), range macros, structure typedefs that can be used to access the
* Xilinx Rgb2YCrCb core instance.
*
* The RGB to YCrCb Color-Space Convertor core is a simplified 3x3 matrix
* multiplier converts three input color samples to three output samples in a
* single clock cycle. The core supports four common format conversions as well
* as a custom mode that allows for a user-defined transform. The core is
* capable of a maximum resolution of 7680 columns by 7680 rows with 8, 10, 12,
* or 16 bits per pixel.
*
* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the RGB2YCRCB core.
*
* XRgb2YCrCb_CfgInitialize() API is used to initialize the RGB2YCRCB core.
* The user needs to first call the XRgb2YCrCb_LookupConfig() API which returns
* the Configuration structure pointer which is passed as a parameter to the
* XRgb2YCrCb_CfgInitialize() API.
*
* <b> Interrupts </b>
*
* The driver provides an interrupt handler XRgb2YCrCb_IntrHandler for handling
* the interrupt from the RGB2YCRCB core. The users of this driver have to
* register this handler with the interrupt system and provide the callback
* functions by using XRgb2YCrCb_SetCallBack API.
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
* The XRgb2YCrCb driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 5.00a tb     02/27/12 Updated for RGB2YCRCB V5.00.a
* 5.01a bao    12/28/12 Converted from xio.h to xil_io.h, translating basic
*                       types, MB cache functions, exceptions and assertions
*                       to xil_io format.
* 6.0   adk    19/12/13 Updated as per the New Tcl API's.
* 7.0   adk    01/07/14 Changed the file name from "rgb2ycrcb.h" to
*                       "xrgb2ycrcb.h".
*                       Moved register offsets and bit definitions from
*                       rgb2ycrcb.h file to xrgb2ycrcb_hw.h file.
*
*                       Removed RGB_TIMING_STATUS register offset because this
*                       register is not present in latest product guide.
*
*                       Removed following function macros:
*                       RGB_Enable, RGB_Disable, RGB_RegUpdateEnable,
*                       RGB_RegUpdateDisable, RGB_Reset, RGB_ClearReset,
*                       RGB_AutoSyncReset.

*                       Removed following functions:
*                       RGB_select_standard, RGB_coefficient_translation,
*                       RGB_set_coefficients, RGB_get_coefficients.
*
*                       Added following handler types as enum:
*                       XRGB_HANDLER_PROCSTART, XRGB_HANDLER_FRAMEDONE,
*                       XRGB_HANDLER_ERROR.
*
*                       Added following standard types as enum:
*                       XRGB_STANDARD_ITU_601_SD, XRGB_STANDARD_ITU_709_NTSC,
*                       XRGB_STANDARD_ITU_709_PAL, XRGB_STANDARD_YUV,
*                       XRGB_STANDARD_CUSTOM.
*
*                       Added following output ranges as enum:
*                       XRGB_TV_16_TO_240, XRGB_STUDIO_16_TO_235,
*                       XRGB_GRAPHICS_0_TO_255.
*
*                       Added range macros for ActiveSize, YMAX, YMIN, YOFFSET,
*                       CBMAX, CBMIN, CBOFFSET, CRMAX, CRMIN, CROFFSET
*                       registers.
*
*                       Added following function macros:
*                       XRgb2YCrCb_IntrEnable, XRgb2YCrCb_SyncReset,
*                       XRgb2YCrCb_IntrDisable, XRgb2YCrCb_StatusGetPending,
*                       XRgb2YCrCb_IntrGetPending, XRgb2YCrCb_IntrClear,
*                       XRgb2YCrCb_RegUpdateEnable,
*                       XRgb2YCrCb_RegUpdateDisable, XRgb2YCrCb_Reset,
*                       XRgb2YCrCb_Enable, XRgb2YCrCb_Disable,
*                       XRgb2YCrCb_Start, XRgb2YCrCb_Stop.
*
*                       Added core and configuration structure.
*                       Renamed rgb_coef_inputs -> XRgb2YCrCb_Coef_Inputs and
*                       rgb_coef_outputs - > XRgb2YCrCb_Coef_Outputs.
*
*                       Added callback functions typedef.
*
*                       Implemented XRgb2YCrCb_LookupConfig in
*                       xrgb2ycrcb_sinit.c
*                       Implemented XRgb2YCrCb_SelfTest in
*                       xrgb2ycrcb_selftest.c
*                       Implemented XRgb2YCrCb_IntrHandler,
*                       XRgb2YCrCb_SetCallBack in xrgb2ycrcb_intr.c.
*
*                       Implemented following functions in xrgb2ycrcb.c:
*                       XRgb2YCrCb_CfgInitialize, XRgb2YCrCb_EnableDbgByPass,
*                       XRgb2YCrCb_IsDbgByPassEnabled,
*                       XRgb2YCrCb_DisableDbgBypass,
*                       XRgb2YCrCb_EnableDbgTestPattern,
*                       XRgb2YCrCb_IsDbgTestPatternEnabled,
*                       XRgb2YCrCb_DisableDbgTestPattern,
*                       XRgb2YCrCb_GetVersion, XRgb2YCrCb_GetDbgFrameCount,
*                       XRgb2YCrCb_GetDbgLineCount,
*                       XRgb2YCrCb_GetDbgPixelCount, XRgb2YCrCb_Setup,
*                       XRgb2YCrCb_SetActiveSize, XRgb2YCrCb_GetActiveSize,
*                       XRgb2YCrCb_SetYMax, XRgb2YCrCb_GetYMax,
*                       XRgb2YCrCb_SetYMin, XRgb2YCrCb_GetYMin,
*                       XRgb2YCrCb_SetCbMax, XRgb2YCrCb_GetCbMax,
*                       XRgb2YCrCb_SetCbMin, XRgb2YCrCb_GetCbMin,
*                       XRgb2YCrCb_SetCrMax, XRgb2YCrCb_GetCrMax,
*                       XRgb2YCrCb_SetCrMin, XRgb2YCrCb_GetCrMin,
*                       XRgb2YCrCb_SetYOffset, XRgb2YCrCb_GetYOffset,
*                       XRgb2YCrCb_SetCbOffset, XRgb2YCrCb_GetCbOffset,
*                       XRgb2YCrCb_SetCrOffset, XRgb2YCrCb_GetCrOffset,
*                       XRgb2YCrCb_SetCoefs, XRgb2YCrCb_GetCoefs,
*                       XRgb2YCrCb_Select_Standard,
*                       XRgb2YCrCb_Coefficient_Translation,
*                       XRgb2YCrCb_Select_OutputRange.
* 7.1   ms     01/16/17 Updated the parameter naming from
*                       XPAR_RGB2YCRCB_NUM_INSTANCES  to
*                       XPAR_XRGB2YCRCB_NUM_INSTANCES to avoid compilation
*                       failure for XPAR_RGB2YCRCB_NUM_INSTANCES as the
*                       tools are generating XPAR_XRGB2YCRCB_NUM_INSTANCES
*                       in the generated xrgb2ycrcb_g.c for fixing MISRA-C
*                       files. This is a fix for CR-966099 based on the
*                       update in the tools.
*       ms     03/17/17 Added readme.txt file in examples folder for doxygen
*                       generation.
*</pre>
*
******************************************************************************/

#ifndef XRGB2YCRCB_H_
#define XRGB2YCRCB_H_	/**< Prevent circular inclusions by using protection
			  *  macros
			  */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xrgb2ycrcb_hw.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/** @name Handler Types
 * @{
 */
/**
*
* These constants specify different types of handler and used to differentiate
* interrupt requests from core.
*/
enum {
	XRGB_HANDLER_PROCSTART = 1, /**< A processing start event interrupt
				      * type */
	XRGB_HANDLER_FRAMEDONE,	    /**< A frame done event
				      *  interrupt type */
	XRGB_HANDLER_ERROR	    /**< An error condition interrupt
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
enum XRgb_Standards {
	XRGB_STANDARD_ITU_601_SD = 0,	 /**< Standard ITU 601 SD. */
	XRGB_STANDARD_ITU_709_NTSC,	 /**< Standard ITU 709 NTSC. */
	XRGB_STANDARD_ITU_709_PAL,	 /**< Standard ITU 709 PAL. */
	XRGB_STANDARD_YUV,		 /**< Standard YUV. */
	XRGB_STANDARD_CUSTOM		 /**< Standard Custom. */
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
enum XRgb_OutputRanges {
	XRGB_TV_16_TO_240 = 0,  /**< 16 to 240, for Television. */
	XRGB_STUDIO_16_TO_235, /**< 16 to 235, for Studio Equipment. */
	XRGB_GRAPHICS_0_TO_255	  /**< 0 to 255, for Computer Graphics. */
} ;
/*@}*/

/** @name Active size range macros
 * @{
 */
#define XRGB_VSIZE_FIRST	32	/**< VSize starting value */
#define XRGB_VSIZE_LAST		7680	/**< VSize ending value */
#define XRGB_HSIZE_FIRST	32	/**< HSize starting value */
#define XRGB_HSIZE_LAST		7680	/**< HSize ending value */
/*@}*/

/** @name Ymax range macros
 * @{
 */
#define XRGB_YMAX_FIRST		0	/**< Ymax starting value */
#define XRGB_YMAX_LAST		255	/**< Ymax ending value */
/*@}*/

/** @name Ymin range macros
 * @{
 */
#define XRGB_YMIN_FIRST		0	/**< Ymin starting value */
#define XRGB_YMIN_LAST		255	/**< Ymax ending value */
/*@}*/

/** @name Yoffset range macros
 * @{
 */
#define XRGB_YOFFSET_FIRST	0	/**< Yoffset starting value */
#define XRGB_YOFFSET_LAST	255	/**< Yoffset ending value */
/*@}*/

/** @name Cbmax range macros
 * @{
 */
#define	XRGB_CBMAX_FIRST	0	/**< Cbmax starting value */
#define XRGB_CBMAX_LAST		255	/**< Cbmax ending value */
/*@}*/

/** @name Cbmin range macros
 * @{
 */
#define	XRGB_CBMIN_FIRST	0	/**< Cbmin starting value */
#define XRGB_CBMIN_LAST		255	/**< Cbmin ending value */
/*@}*/

/** @name Cboffset range macros
 * @{
 */
#define XRGB_CBOFFSET_FIRST	0	/**< Cboffset starting value */
#define XRGB_CBOFFSET_LAST	255	/**< Cboffset ending value */
/*@}*/

/** @name Crmax range macros
 * @{
 */
#define XRGB_CRMAX_FIRST	0	/**< Crmax starting value */
#define XRGB_CRMAX_LAST		255	/**< Crmax ending value */
/*@}*/

/** @name Crmin range macros
 * @{
 */
#define XRGB_CRMIN_FIRST	0	/**< Crmin starting value */
#define XRGB_CRMIN_LAST		255	/**< Crmin ending value */
/*@}*/

/** @name Croffset range macros
 * @{
 */
#define XRGB_CROFFSET_FIRST	0	/**< Croffset starting value */
#define XRGB_CROFFSET_LAST	255	/**< Croffset ending value */
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro enables the Rgb2YCrCb device/core.
*
* @param	InstancePtr is a pointer to the Rgb2YCrCb core instance to be
* 		worked on.
* @return	None.
*
* @note		C-style signature:
*		void XRgb2YCrCb_Enable(XRgb2YCrCb *InstancePtr)
*
******************************************************************************/
#define XRgb2YCrCb_Enable(InstancePtr) \
	XRgb2YCrCb_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XRGB_CONTROL_OFFSET), \
		((XRgb2YCrCb_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XRGB_CONTROL_OFFSET))) | (XRGB_CTL_SW_EN_MASK)))

/*****************************************************************************/
/**
*
* This macro disables the Rgb2YCrCb device/core.
*
* @param	InstancePtr is a pointer to the Rgb2YCrCb core instance to be
* 		worked on.
* @return	None.
*
* @note		C-style signature:
* 		void XRgb2YCrCb_Disable(XRgb2YCrCb *InstancePtr)
*
******************************************************************************/
#define XRgb2YCrCb_Disable(InstancePtr) \
	XRgb2YCrCb_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XRGB_CONTROL_OFFSET), \
		((XRgb2YCrCb_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XRGB_CONTROL_OFFSET))) & (~(XRGB_CTL_SW_EN_MASK))))

/*****************************************************************************/
/**
*
* This function macro enables/starts the Rgb2YCrCb core.
*
* @param	InstancePtr is a pointer to the Rgb2YCrCb instance to be
*		worked on
*
* @return	None.
*
* @note		C-style signature:
*		void XRgb2YCrCb_Start(XRgb2YCrCb *InstancePtr)
*
******************************************************************************/
#define XRgb2YCrCb_Start	XRgb2YCrCb_Enable

/*****************************************************************************/
/**
*
* This function macro disables/stops the Rgb2YCrCb core.
*
* @param	InstancePtr is a pointer to the Rgb2YCrCb instance to be
*		worked on
*
* @return	None.
*
* @note		C-style signature:
*		void XRgb2YCrCb_Stop(XRgb2YCrCb *InstancePtr)
*
******************************************************************************/
#define XRgb2YCrCb_Stop		XRgb2YCrCb_Disable

/*****************************************************************************/
/**
*
* This macro enables copying from updated processor register values to
* the active set at the end of each AXI-Stream frame.
*
* @param	InstancePtr is a pointer to the Rgb2YCrCb core instance to be
* 		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XRgb2YCrCb_RegUpdateEnable(XRgb2YCrCb *InstancePtr)
*
******************************************************************************/
#define XRgb2YCrCb_RegUpdateEnable(InstancePtr)\
	XRgb2YCrCb_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XRGB_CONTROL_OFFSET), \
		((XRgb2YCrCb_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XRGB_CONTROL_OFFSET))) | (XRGB_CTL_RUE_MASK)))

/*****************************************************************************/
/**
*
* This macro disables copying from updated processor register values to
* the active set at the end of each AXI-Stream frame.
*
* @param	InstancePtr is a pointer to the Rgb2YCrCb core instance to be
* 		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XRgb2YCrCb_RegUpdateDisable(XRgb2YCrCb *InstancePtr)
*
******************************************************************************/
#define XRgb2YCrCb_RegUpdateDisable(InstancePtr) \
	XRgb2YCrCb_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XRGB_CONTROL_OFFSET), \
		((XRgb2YCrCb_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XRGB_CONTROL_OFFSET)))) & (~(XRGB_CTL_RUE_MASK)))

/*****************************************************************************/
/**
*
* This macro resets the Rgb2YCrCb device, but differs from XRgb_Reset() in that
* it automatically synchronizes to the VBlank_in input of the core to prevent
* tearing.
*
* On the next rising-edge of VBlank_in following a call to
* XRgb2YCrCb_SyncReset(), all of the core's configuration registers and outputs
* will be reset, then the reset flag will be immediately released, allowing the
* core to immediately resume default operation.
*
* @param	InstancePtr is a pointer to the Rgb2YCrCb core instance to be
* 		worked on.
*
* @return	None.
*
* @note		C-style signature:
* 		void XRgb2YCrCb_SyncReset(XRgb2YCrCb *InstancePtr)
*
******************************************************************************/
#define XRgb2YCrCb_SyncReset(InstancePtr) \
		XRgb2YCrCb_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XRGB_CONTROL_OFFSET), (XRGB_CTL_AUTORESET_MASK))

/*****************************************************************************/
/**
*
* This macro resets the Rgb2YCrCb core. This reset effects the core
* immediately, and may cause image tearing.
*
* @param	InstancePtr is a pointer to the Rgb2YCrCb core instance to be
* 		worked on.
* @return	None.
*
* @note		C-style signature:
*		void XRgb2YCrCb_Reset(XRgb2YCrCb *InstancePtr)
*
******************************************************************************/
#define XRgb2YCrCb_Reset(InstancePtr) \
		XRgb2YCrCb_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XRGB_CONTROL_OFFSET), (XRGB_CTL_RESET_MASK))

/*****************************************************************************/
/**
*
* This function macro enables individual interrupts of the RGB2YCRCB core by
* updating the IRQ_ENABLE register.
*
* @param	InstancePtr is a pointer to the Rgb2YCrCb core instance to be
* 		worked on.
* @param	IntrType is the bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XRGB_IXR_*_MASK bits defined in xrgb2ycrcb_hw.h.
*
* @return	None.
*
* @note		C-style signature:
*		void XRgb2YCrCb_IntrEnable(XRgb2YCrCb *InstancePtr,
*						u32 IntrType)
*
* 		The existing enabled interrupt(s) will remain enabled.
*
******************************************************************************/
#define XRgb2YCrCb_IntrEnable(InstancePtr, IntrType) \
		XRgb2YCrCb_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XRGB_IRQ_EN_OFFSET), (((IntrType) & \
				(XRGB_IXR_ALLINTR_MASK)) | \
		(XRgb2YCrCb_ReadReg((InstancePtr)->Config.BaseAddress, \
					(XRGB_IRQ_EN_OFFSET)))))

/*****************************************************************************/
/**
*
* This function macro disables individual interrupts of the RGB2YCRCB core by
* updating the IRQ_ENABLE register.
*
* @param	InstancePtr is a pointer to the Rgb2YCrCb core instance to be
* 		worked on.
* @param	IntrType is the bit-mask of the interrupts to be disabled.
*		Bit positions of 1 will be disabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XRGB_IXR_*_MASK bits defined in xrgb2ycrcb_hw.h.
*
* @return	None.
*
* @note		C-style signature:
*		void XRgb2YCrCb_IntrDisable(XRgb2YCrCb *InstancePtr,
*						u32 IntrType)
*
* 		Any other interrupt not covered by parameter IntrType, if
* 		enabled before this macro is called, will remain enabled.
*
******************************************************************************/
#define XRgb2YCrCb_IntrDisable(InstancePtr, IntrType) \
	XRgb2YCrCb_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XRGB_IRQ_EN_OFFSET), \
		 ((XRgb2YCrCb_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XRGB_IRQ_EN_OFFSET))) & ((~(IntrType)) & \
				(XRGB_IXR_ALLINTR_MASK))))

/*****************************************************************************/
/**
*
* This function macro returns the pending interrupt status of the RGB2YCRCB
* core read from the Status register.
*
* @param	InstancePtr is a pointer to the Rgb2YCrCb core instance to be
* 		worked on.
*
* @return	The status of pending interrupts of the Rgb2YCrCb core.
* 		Use XRGB_IXR_*_MASK constants defined in xrgb2ycrcb_hw.h to
*		interpret this value.
*
* @note		C-style signature:
*		u32 XRgb2YCrCb_StatusGePending(XRgb2YCrCb *InstancePtr)
*
******************************************************************************/
#define XRgb2YCrCb_StatusGetPending(InstancePtr) \
		XRgb2YCrCb_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XRGB_STATUS_OFFSET)) & (XRGB_IXR_ALLINTR_MASK)

/*****************************************************************************/
/**
*
* This function macro returns the pending interrupts of the RGB2YCRCB core for
* the interrupts that have been enabled.
*
* @param	InstancePtr is a pointer to the Rgb2YCrCb core instance to be
* 		worked on.
*
* @return	The pending interrupts of the Rgb2YCrCb core. Use
*		XRGB_IXR_*_MASK constants defined in xrgb2ycrcb_hw.h to
*		interpret this value. The returned value is a logical AND of
*		the contents of the STATUS Register and the IRQ_ENABLE
*		Register.
*
* @note		C-style signature:
*		u32 XRgb2YCrCb_IntrGetPending(XRgb2YCrCb *InstancePtr)
*
******************************************************************************/
#define XRgb2YCrCb_IntrGetPending(InstancePtr) \
	XRgb2YCrCb_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XRGB_IRQ_EN_OFFSET)) & \
		((XRgb2YCrCb_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XRGB_STATUS_OFFSET))) & ((u32)(XRGB_IXR_ALLINTR_MASK)))

/*****************************************************************************/
/**
*
* This function macro clears/acknowledges pending interrupts of the RGB2YCRCB
* core in the Status register. Bit positions of 1 will be cleared.
*
* @param	InstancePtr is a pointer to the Rgb2YCrCb core instance to be
* 		worked on.
* @param	IntrType is the pending interrupts to clear/acknowledge.
*		Use OR'ing of XRGB_IXR_*_MASK constants defined in
*		xrgb2ycrcb_hw.h to create this parameter value.
*
* @return	None.
*
* @note		C-style signature:
*		void XRgb2YCrCb_IntrClear(XRgb2YCrCb *InstancePtr,
*						u32 IntrType)
*
******************************************************************************/
#define XRgb2YCrCb_IntrClear(InstancePtr, IntrType) \
		XRgb2YCrCb_WriteReg((InstancePtr)->Config.BaseAddress, \
			(XRGB_STATUS_OFFSET), ((IntrType) & \
				(XRGB_IXR_ALLINTR_MASK)))

/**************************** Type Definitions *******************************/

/**
* This typedef contains configuration information for a Rgb2YCrCb core.
* Each Rgb2YCrCb core should have a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID of the
				  *  RGB2YCRCB core */
	u32 BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the RGB2YCRCB core registers */
	u32 SlaveAxisVideoFormat;	/**< Slave Axis Video Format */
	u32 MasterAxisVideoFormat;	/**< Master Axis Video Format */
	u16 HasDebug;			/**< To check debug support */
	u16 HasIntcIf;			/**< To check Interrupt controller
					  * support */
	u32 MaxCols;			/**< Maximum number of columns */
	u32 ActiveCols;			/**< Number of active columns */
	u32 ActiveRows;			/**< Number of active rows */
	u16 HasClip;			/**< To check support for clipping */
	u16 HasClamp;			/**< To check support for clamping */
	u32 ACoef;			/**< A coefficient */
	u32 BCoef;			/**< B coefficient */
	u32 CCoef;			/**< C coefficient */
	u32 DCoef;			/**< D coefficient */
	u32 YOffset;			/**< Y Offset value */
	u32 CbOffset;			/**< Cb Offset value */
	u32 CrOffset;			/**< Cr Offset value */
	u32 YMax;			/**< Y Min value */
	u32 YMin;			/**< Y Max value */
	u32 CbMax;			/**< Cb Max value */
	u32 CbMin;			/**< Cb Min value */
	u32 CrMax;			/**< Cr Max value */
	u32 CrMin;			/**< Cr Min value */
	u32 SlaveAxiClkFreqHz;		/**< Slave AXI Clock Frequency in Hz */
	u32 StandardSelection;		/**< To select standard */
	u32 OutputRange;		/**< To identify output range */
} XRgb2YCrCb_Config;

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
*****************************************************************************/
typedef void (*XRgb2YCrCb_CallBack)(void *CallBackRef);

/*****************************************************************************/
/**
*
* This data type defines a handler that an application defines to communicate
* with interrupt system to retrieve error information while processing video
* frame.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
*		value equals 'OR'ing one or more XRGB_IXR_*_MASK values defined
*		in xrgb2ycrcb_hw.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
typedef void (*XRgb2YCrCb_ErrorCallBack)(void *CallBackRef, u32 ErrorMask);

/**
* The XRgb2YCrCb driver instance data structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
typedef struct {
	XRgb2YCrCb_Config Config;	/**< Hardware configuration */
	u32 IsReady;	/**< Core and driver instance are initialized */
	u16 HSize;	/**< Active Video Horizontal Size */
	u16 VSize;	/**< Active Video Vertical Size */

	/* IRQ callbacks Here */
	XRgb2YCrCb_CallBack ProcStartCallBack;	/**< Callback for Processing
						  * Start interrupt */
	void *ProcStartRef;	/**< To be passed to Process start interrupt
				  *  callback */
	XRgb2YCrCb_CallBack FrameDoneCallBack;	/**< Callback for Frame Done
						  * interrupt */
	void *FrameDoneRef;	/**< To be passed to the Frame done interrupt
				  *  callback */
	XRgb2YCrCb_ErrorCallBack ErrCallBack;	/**< Callback for Error
						  *  interrupt */
	void *ErrRef;	/**< To be passed to the Error interrupt callback */
	u32 StandardSelection;		/**< Standard Selection */
	u32 OutputRange;		/**< Output Range */
} XRgb2YCrCb;

/**
* Rgb2YCrCb input coefficient
*/
struct XRgb2YCrCb_Coef_Inputs {
	/* Pre-translated coefficient/offset data */
	double ACoef;	/**< [ 0.0 - 1.0 ]     0.0 < ACOEFF + BCOEFF < 1.0 */
	double BCoef;	/**< [ 0.0 - 1.0 ]     0.0 < ACOEFF + BCOEFF < 1.0 */
	double CCoef;	/**< [ 0.0 - 0.9 ] */
	double DCoef;	/**< [ 0.0 - 0.9 ] */
	u32 YOffset;	/**< Offset for the Luminance Channel */
	u32 CbOffset;	/**< Offset for the Chrominance Channels */
	u32 CrOffset;	/**< Offset for the Chrominance Channels */
	u32 YMax;	/**< Y Clipping */
	u32 YMin;	/**< Y Clamping */
	u32 CbMax;	/**< Cb Clipping */
	u32 CbMin;	/**< Cb Clamping */
	u32 CrMax;	/**< Cr Clipping */
	u32 CrMin;	/**< Cr Clamping */
} ;

/**
* Rgb2YCrCb output coefficient
*/
struct XRgb2YCrCb_Coef_Outputs {
	/* Translated coefficient/offset data */
	u32 ACoef;	/**< Translated ACoef */
	u32 BCoef;	/**< Translated BCoef */
	u32 CCoef;	/**< Translated CCoef */
	u32 DCoef;	/**< Translated DCoef */
	u32 YOffset;	/**< Translated Offset for the Luminance Channel */
	u32 CbOffset;	/**< Translated Offset for the Chrominance Channels */
	u32 CrOffset;	/**< Translated Offset for the Chrominance Channels */
	u32 YMax;	/**< Translated Y Clipping */
	u32 YMin;	/**< Translated Y Clamping */
	u32 CbMax;	/**< Translated Cb Clipping */
	u32 CbMin;	/**< Translated Cb Clamping */
	u32 CrMax;	/**< Translated Cr Clipping */
	u32 CrMin;	/**< Translated Cr Clamping */
} ;

/************************** Function Prototypes ******************************/

/* Static lookup function implemented in xrgb2ycrcb_sinit.c */
XRgb2YCrCb_Config *XRgb2YCrCb_LookupConfig(u16 DeviceId);

/* Following functions implemented in xrgb2ycrcb.c */
int XRgb2YCrCb_CfgInitialize(XRgb2YCrCb *InstancePtr,
			XRgb2YCrCb_Config *CfgPtr, u32 EffectiveAddr);

void XRgb2YCrCb_EnableDbgByPass(XRgb2YCrCb *InstancePtr);
int XRgb2YCrCb_IsDbgByPassEnabled(XRgb2YCrCb *InstancePtr);
void XRgb2YCrCb_DisableDbgBypass(XRgb2YCrCb *InstancePtr);

void XRgb2YCrCb_EnableDbgTestPattern(XRgb2YCrCb *InstancePtr);
int XRgb2YCrCb_IsDbgTestPatternEnabled(XRgb2YCrCb *InstancePtr);
void XRgb2YCrCb_DisableDbgTestPattern(XRgb2YCrCb *InstancePtr);

u32 XRgb2YCrCb_GetVersion(XRgb2YCrCb *InstancePtr);

u32 XRgb2YCrCb_GetDbgFrameCount(XRgb2YCrCb *InstancePtr);
u32 XRgb2YCrCb_GetDbgLineCount(XRgb2YCrCb *InstancePtr);
u32 XRgb2YCrCb_GetDbgPixelCount(XRgb2YCrCb *InstancePtr);

void XRgb2YCrCb_Setup(XRgb2YCrCb *InstancePtr);

void XRgb2YCrCb_SetActiveSize(XRgb2YCrCb *InstancePtr, u16 HSize, u16 VSize);
void XRgb2YCrCb_GetActiveSize(XRgb2YCrCb *InstancePtr, u16 *HSize, u16 *VSize);

void XRgb2YCrCb_SetYMax(XRgb2YCrCb *InstancePtr, u32 YMax);
u32 XRgb2YCrCb_GetYMax(XRgb2YCrCb *InstancePtr);

void XRgb2YCrCb_SetYMin(XRgb2YCrCb *InstancePtr, u32 YMin);
u32 XRgb2YCrCb_GetYMin(XRgb2YCrCb *InstancePtr);

void XRgb2YCrCb_SetCbMax(XRgb2YCrCb *InstancePtr, u32 CbMax);
u32 XRgb2YCrCb_GetCbMax(XRgb2YCrCb *InstancePtr);

void XRgb2YCrCb_SetCbMin(XRgb2YCrCb *InstancePtr, u32 CbMin);
u32 XRgb2YCrCb_GetCbMin(XRgb2YCrCb *InstancePtr);

void XRgb2YCrCb_SetCrMax(XRgb2YCrCb *InstancePtr, u32 CrMax);
u32 XRgb2YCrCb_GetCrMax(XRgb2YCrCb *InstancePtr);

void XRgb2YCrCb_SetCrMin(XRgb2YCrCb *InstancePtr, u32 CrMin);
u32 XRgb2YCrCb_GetCrMin(XRgb2YCrCb *InstancePtr);


void XRgb2YCrCb_SetYOffset(XRgb2YCrCb *InstancePtr, u32 YOffset);
u32 XRgb2YCrCb_GetYOffset(XRgb2YCrCb *InstancePtr);

void XRgb2YCrCb_SetCbOffset(XRgb2YCrCb *InstancePtr, u32 CbOffset);
u32 XRgb2YCrCb_GetCbOffset(XRgb2YCrCb *InstancePtr);

void XRgb2YCrCb_SetCrOffset(XRgb2YCrCb *InstancePtr, u32 CrOffset);
u32 XRgb2YCrCb_GetCrOffset(XRgb2YCrCb *InstancePtr);

void XRgb2YCrCb_SetCoefs(XRgb2YCrCb *InstancePtr, double ACoef, double BCoef,
				double CCoef, double DCoef);
void XRgb2YCrCb_GetCoefs(XRgb2YCrCb *InstancePtr, double *ACoef, double *BCoef,
				double *CCoef, double *DCoef);

void XRgb2YCrCb_Select_Standard(XRgb2YCrCb *InstancePtr,
				enum XRgb_Standards StandardSel,
			enum XRgb_OutputRanges InputRange, u32 DataWidth,
				struct XRgb2YCrCb_Coef_Inputs *CoefIn);

u32 XRgb2YCrCb_Coefficient_Translation(XRgb2YCrCb *InstancePtr,
				struct XRgb2YCrCb_Coef_Inputs *CoefIn,
				struct XRgb2YCrCb_Coef_Outputs *CoefOut,
				u32 Data_Width);

void XRgb2YCrCb_Select_OutputRange(XRgb2YCrCb *InstancePtr,
					enum XRgb_OutputRanges Range);

/* Self-test function implemented in xrgb2ycrcb_selftest.c */
int XRgb2YCrCb_SelfTest(XRgb2YCrCb *InstancePtr);

/* Interrupt related functions implemented in xrgbycrcb_intr.c */
void XRgb2YCrCb_IntrHandler(void *InstancePtr);

int XRgb2YCrCb_SetCallBack(XRgb2YCrCb *InstancePtr, u32 HandlerType,
				void *CallBackFunc,
				void *CallBackRef);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
