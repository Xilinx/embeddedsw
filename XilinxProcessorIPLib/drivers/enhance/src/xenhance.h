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
* @file xenhance.h
* @addtogroup enhance_v7_1
* @{
* @details
*
* This header file contains identifiers and register-level driver functions (or
* macros), range macros, structure typedefs that can be used to access the
* Image Statistic core instance.
*
* The Image Enhancement core offers noise reduction and/or edge enhancement.
* For edge enhancement, optional anti-halo and anti-alias post-processing
* modules are available to reduce image artifacts that can appear from the
* high-pass filtering of the edge enhancement filters. The amount of noise
* reduction and edge enhancement is controlled through user parameters.
* There are two variations of the algorithm offered to choose between high
* performance and minimal resource usage. This core works on YCbCr 4:4:4 and
* 4:2:2 data. The core is capable of a maximum resolution of 7680 columns by
* 7680 rows with 8, 10, 12, or 16 bits per pixel and supports the bandwidth
* necessary for High-definition (1080p60) resolutions in all Xilinx FPGA device
* families. Higher resolutions can be supported in Xilinx high-performance
* device families.
*
* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the Enhance core.
*
* XEnhance_CfgInitialize() API is used to initialize the Enhance core.
* The user needs to first call the XEnhance_LookupConfig() API which returns
* the Configuration structure pointer which is passed as a parameter to the
* XEnhance_CfgInitialize() API.
*
* <b> Interrupts </b>
*
* The driver provides an interrupt handler XEnhance_IntrHandler for handling
* the interrupt from the Enhance core. The users of this driver have to
* register this handler with the interrupt system and provide the callback
* functions by using XEnhance_SetCallBack API.
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
* The XEnhance driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- ---- -------- -----------------------------------------------------
* 2.00a vc  12/14/10 Updated for ENHANCE V2.0
* 3.00a rc  09/11/11 Updated for ENHANCE V3.0
* 4.00a vyc 04/24/12 Updated for ENHANCE V4.00.a
*                    Converted from xio.h to xil_io.h, translating
*                    basic type, MB cache functions, exceptions and
*                    assertion to xil_io format.
* 5.00a vyc 06/19/13 Updated for ENHANCE V8.0
*                    New edge enhancement algorithm and registers
*                    Noise reduction support added
* 6.0   adk 19/12/13 Updated as per the New Tcl API's
* 7.0   adk 02/19/14 Changed the filename from enhance.h to xenhance.h.
*                    Changes in xenhance.h:
*                    Removed the following macros
*                    ENHANCE_Enable, ENHANCE_Disable, ENHANCE_RegUpdateEnable,
*                    ENHANCE_RegUpdateDisable, ENHANCE_Reset,
*                    ENHANCE_AutoSyncReset
*
*                    Added the following function macros
*                    XEnhance_Enable, XEnhance_Disable,
*                    XEnhance_RegUpdateDisable, XEnhance_RegUpdateDisable
*                    XEnhance_Reset, XEnhance_SyncReset, XEnhance_IntrEnable
*                    XEnhance_IntrDisable, XEnhance_StatusGetPending
*                    XEnhance_IntrGetPending, XEnhance_IntrClear
*
*                    Added the following type definitions:
*                    XEnhance_Config and XEnhance structures.
*                    XEnhance_CallBack and XEnhance_ErrorCallBack.
*
*                    Changes in xenhance.c:
*                    Modified the following functions
*                    XENHANCE_CfgInitialize -> XEnhance_CfgInitialize
*                    XENHANCE_Setup -> XEnhance_Setup
*
*                    Implemented the following functions:
*                    XEnhance_CfgInitialize, XEnhance_Setup,
*                    XEnhance_GetVersion, XEnhance_EnableDbgByPass,
*                    XEnhance_IsDbgByPassEnabled, XEnhance_DisableDbgBypass
*                    XEnhance_EnableDbgTestPattern,
*                    XEnhance_IsDbgTestPatternEnabled
*                    XEnhance_DisableDbgTestPattern
*                    XEnhance_GetDbgFrameCount, XEnhance_GetDbgLineCount,
*                    XEnhance_GetDbgPixelCount, XEnhance_SetActiveSize,
*                    XEnhance_GetActiveSize, XEnhance_SetNoiseThreshold,
*                    XEnhance_GetNoiseThreshold, XEnhance_SetEdgeStrength,
*                    XEnhance_GetEdgeStrength, XEnhance_SetHaloSuppress
*                    XEnhance_GetHaloSuppress.
*
*                    Changes in xenhance_hw.h:
*                    Added the register offsets and bit masks for the
*                    registers and added backward compatibility for macros.
*
*                    Changes in xenhance_intr.c:
*                    Implemented the following functions
*                    XEnhance_IntrHandler
*                    XEnhance_SetCallBack
*
*                    Changes in xenhance_sinit.c:
*                    Implemented the following function
*                    XEnhance_LookupConfig
*
*                    Changes in xenhance_selftest.c:
*                    Implemented the following function
*                    XEnhance_SelfTest
* 7.1   ms  01/31/17 Updated the parameter naming from
*                    XPAR_ENHANCE_NUM_INSTANCES to
*                    XPAR_XENHANCE_NUM_INSTANCES to avoid compilation
*                    failure for XPAR_ENHANCE_NUM_INSTANCES
*                    as the tools are generating
*                    XPAR_XENHANCE_NUM_INSTANCES in the generated
*                    xenhance_g.c for fixing MISRA-C files. This is a
*                    fix for CR-967548 based on the update in the tools.
*       ms  03/17/17 Added readme.txt file in examples folder for doxygen
*                    generation.
* </pre>
*
******************************************************************************/

#ifndef XENHANCE_H_
#define XENHANCE_H_	/**< Prevent circular inclusions by using
			  *  protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_assert.h"
#include "xstatus.h"
#include "xenhance_hw.h"

/************************** Constant Definitions *****************************/

/** @name Handler Types
* @{
*/
/**
* These constants specify different types of handlers and used to
* differentiate interrupt requests from core.
*
*/
enum {
	XENH_HANDLER_PROCSTART = 1,	/**< A processing start event
					  *  interrupt type */
	XENH_HANDLER_FRAMEDONE,		/**< A frame done event
					  *  interrupt type */
	XENH_HANDLER_ERROR		/**< An error condition event
					  *  interrupt type */
} ;
/*@}*/

/** @name Active size range macros
* @{
*/
#define XENH_VSIZE_FIRST	32		/**< Vertical Size starting
						  *  value */
#define XENH_VSIZE_LAST		7680		/**< Vertical Size ending
						  *  value */
#define XENH_HSIZE_FIRST	32		/**< Horizontal Size starting
						  *  value */
#define XENH_HSIZE_LAST		7680		/**< Horizontal Size ending
						  *  value */
/*@}*/

/** @name Noise threshold range macros
* @{
*/
#define XENH_NOISETHRES_MIN	0		/**< Noise threshold starting
						  *  value */
#define XENH_NOISETHRES_MAX	255		/**< Noise threshold ending
						  *  value */
/*@}*/

/** @name Strength range macros
* @{
*/
#define XENH_ENHSTRENGTH_MIN	0		/**< Strength starting value */
#define XENH_ENHSTRENGTH_MAX	32768		/**< Strength ending value */
/*@}*/

/** @name Halo Suppress range macros
* @{
*/
#define XENH_HALOSUPPRESS_MIN	0		/**< Halo Suppress starting
						  *  value */
#define XENH_HALOSUPPRESS_MAX	32768		/**< Halo Suppress ending
						  *  value */
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This function macro enables the Image Enhancement core instance.
*
* @param	InstancePtr is a pointer to the Enhance core instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XEnhance_Enable(XEnhance *InstancePtr)
*
******************************************************************************/
#define XEnhance_Enable(InstancePtr) \
		XEnhance_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XENH_CONTROL_OFFSET), \
			XEnhance_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XENH_CONTROL_OFFSET)) | (XENH_CTL_SW_EN_MASK))

/*****************************************************************************/
/**
*
* This function macro disables the Image Enhancement core instance.
*
* @param	InstancePtr is a pointer to the Enhance core instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XEnhance_Disable(XEnhance *InstancePtr)
*
******************************************************************************/
#define XEnhance_Disable(InstancePtr) \
	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XENH_CONTROL_OFFSET), \
			XEnhance_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XENH_CONTROL_OFFSET)) & (~(XENH_CTL_SW_EN_MASK)))

/*****************************************************************************/
/**
*
* This function macro starts the Image Enhancement core instance.
*
* @param	InstancePtr is a pointer to the Enhance core instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XEnhance_Start(XEnhance *InstancePtr)
*
******************************************************************************/
#define XEnhance_Start		XEnhance_Enable

/*****************************************************************************/
/**
*
* This function macro stops the Image Enhancement core instance.
*
* @param	InstancePtr is a pointer to the Enhance core instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XEnhance_Stop(XEnhance *InstancePtr)
*
******************************************************************************/
#define XEnhance_Stop		XEnhance_Disable

/*****************************************************************************/
/**
*
* This function macro commits all the register value changes made so far by the
* software to the Image Enhancement core instance.
*
* @param	InstancePtr is a pointer to the Enhance core instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XEnhance_RegUpdateEnable(XEnhance *InstancePtr)
*
******************************************************************************/
#define XEnhance_RegUpdateEnable(InstancePtr) \
	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XENH_CONTROL_OFFSET), \
			(XEnhance_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XENH_CONTROL_OFFSET))) | (XENH_CTL_RUE_MASK))

/*****************************************************************************/
/**
*
* This function macro prevents the Image Enhancement core instance from
* committing recent changes made so far by the software. When disabled, changes
* to other configuration registers are stored, but do not effect the behavior
* of the core.
*
* @param	InstancePtr is a pointer to the Enhance core instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XEnhance_RegUpdateDisable(XEnhance *InstancePtr)
*
******************************************************************************/
#define XEnhance_RegUpdateDisable(InstancePtr) \
	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XENH_CONTROL_OFFSET), \
			((XEnhance_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XENH_CONTROL_OFFSET))) & \
					(~(XENH_CTL_RUE_MASK))))

/*****************************************************************************/
/**
*
* This function macro resets a Image Enhancement core instance, but differs from
* XEnhance_Reset() in that it automatically synchronizes to the SOF input of
* the core to prevent tearing.
*
* On the next SOF following a call to XEnhance_SyncReset(),
* all of the core's configuration registers and outputs will be reset, then the
* reset flag will be immediately released, allowing the core to immediately
* resume default operation.
*
* @param	InstancePtr is a pointer to the Enhance core instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void Enhance_SyncReset(XEnhance *InstancePtr)
*
******************************************************************************/
#define XEnhance_SyncReset(InstancePtr) \
	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XENH_CONTROL_OFFSET), (XENH_CTL_AUTORESET_MASK))

/*****************************************************************************/
/**
*
* This function macro resets a Image Enhancement core instance. This reset
* effects the core immediately, and may cause image tearing.
*
* @param	InstancePtr is a pointer to the Enhance core instance to be
*		worked on.
*
* @return	None.
*
* @note		C-style signature:
*		void XEnhance_Reset(XEnhance *InstancePtr)
*
******************************************************************************/
#define XEnhance_Reset(InstancePtr) \
	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XENH_CONTROL_OFFSET), (XENH_CTL_RESET_MASK))

/*****************************************************************************/
/**
*
* This function macro returns the pending status of a Enhance core.
*
* @param	InstancePtr is a pointer to the Enhance core instance to be
*		worked on.
*
* @return	The pending interrupts of the Enhance core. Use XENH_IXR_*_MASK
*		constants defined in xenhance_hw.h to interpret this value.
*
* @note		C-style signature:
*		u32 XEnhance_StatusGePending(XEnhance *InstancePtr).
*
******************************************************************************/
#define XEnhance_StatusGetPending(InstancePtr) \
	XEnhance_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XENH_STATUS_OFFSET)) & (XENH_IXR_ALLINTR_MASK)

/*****************************************************************************/
/**
*
* This function macro clears/acknowledges pending interrupts of a Enhance core.
*
* @param	InstancePtr is a pointer to the Enhance core instance to be
*		worked on.
* @param	IntrType is the pending interrupts to clear/acknowledge. Use
*		OR'ing of XENH_IXR_*_MASK constants defined in xEnhance_hw.h
*		to create this parameter value.
*
* @return	None.
*
* @note		C-style signature:
*		void XEnhance_IntrClear(XEnhance *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XEnhance_IntrClear(InstancePtr, IntrType) \
	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XENH_STATUS_OFFSET), ((IntrType) & (XENH_IXR_ALLINTR_MASK)))

/*****************************************************************************/
/**
*
* This function macro enables the given individual interrupt(s) on the Enhance
* core.
*
* @param	InstancePtr is a pointer to the Enhance core instance to be
*		worked on.
* @param	IntrType is the bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XENH_IXR_*_MASK bits defined in xenhance_hw.h.
*
* @return	None.
*
* @note		The existing enabled interrupt(s) will remain enabled.
*		C-style signature:
*		void XEnhance_IntrEnable(XEnhance *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XEnhance_IntrEnable(InstancePtr, IntrType) \
	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XENH_IRQ_EN_OFFSET), \
		((IntrType) & (XENH_IXR_ALLINTR_MASK)) | \
			(XEnhance_ReadReg((InstancePtr)->Config.BaseAddress, \
				(XENH_IRQ_EN_OFFSET))))

/*****************************************************************************/
/**
*
* This function macro disables the given individual interrupt(s) on the
* Enhance core by updating Irq_Enable register.
*
* @param	InstancePtr is a pointer to the Enhance core instance to be
*		worked on.
* @param	IntrType is the bit-mask of the interrupts to be disabled.
*		Bit positions of 1 will be disabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XENH_IXR_*_MASK bits defined in xenhance_hw.h.
*
* @return	None.
*
* @note		Any other interrupt not covered by parameter IntrType, if
*		enabled before this macro is called, will remain enabled.
*		C-style signature:
*		void XEnhance_IntrDisable(XEnhance *InstancePtr, u32 IntrType)
*
******************************************************************************/
#define XEnhance_IntrDisable(InstancePtr, IntrType) \
	XEnhance_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XENH_IRQ_EN_OFFSET), \
		(XEnhance_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XENH_IRQ_EN_OFFSET)) & ((~(IntrType)) & \
				(XENH_IXR_ALLINTR_MASK))))

/*****************************************************************************/
/**
*
* This function macro returns the pending interrupts of a Enhance core.
*
* @param	InstancePtr is a pointer to the Enhance core instance to be
*		worked on.
*
* @return	The pending interrupts of the Enhance core. Use XENH_IXR_*_MASK
*		constants defined in xenhance_hw.h to interpret this value.
*
* @note		C-style signature:
*		u32 XEnhance_IntrGetPending(XEnhance *InstancePtr)
*
******************************************************************************/
#define XEnhance_IntrGetPending(InstancePtr) \
	XEnhance_ReadReg((InstancePtr)->Config.BaseAddress, \
		(XENH_IRQ_EN_OFFSET)) & \
		(XEnhance_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XENH_STATUS_OFFSET))) & ((u32)(XENH_IXR_ALLINTR_MASK))

/**************************** Type Definitions *******************************/

/**
* This typedef contains configuration information for a Video Enhance core.
* Each Video Enhance core should have a configuration structure associated.
*
******************************************************************************/
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID of the
				  *  device */
	u32 BaseAddress;	/**< BaseAddress is the physical base
				  *  address of the core registers */
	u32 SlaveAxisVideoFormat;	/**< Slave axis video format */
	u32 MasteAxisVideoFoemat;	/**< Master axis video format */
	u32 SlaveAxiClkFreqHz;		/**< Slave axi clock */
	u16 HasIntcIf;			/**< Has Intc IF */
	u16 HasDebug;			/**< Has Debug */
	u32 MaxColumns;			/**< Maximum columns */
	u32 ActiveColumns;		/**< Active columns */
	u32 ActiveRows;			/**< Active Rows */
	u16 HasNoise;			/**< Has noise */
	u16 HasEnhance;			/**< Has enhance */
	u16 HasHalo;			/**< Has halo suppression */
	u16 HasAlias;			/**< Has Alias */
	u32 OptSize;			/**< Optional size */
	u16 NoiseThreshold;		/**< Noise Threshold */
	u32 EnhanceStrength;		/**< Enhance strength */
	u16 HaloSuppress;		/**< Halo suppression */
} XEnhance_Config;

/*****************************************************************************/
/**
*
* Callback type for all interrupts except error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
*
******************************************************************************/
typedef void (*XEnhance_CallBack)(void *CallBackRef);

/*****************************************************************************/
/**
*
* Callback type for Error interrupt.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error. Its
*		value equals 'OR'ing one or more XENH_IXR_*_MASK values defined in
*		xenhance_hw.h
*
******************************************************************************/
typedef void (*XEnhance_ErrorCallBack)(void *CallBackRef, u32 ErrorMask);

/**
* The Enhance driver instance data structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
typedef struct {
	XEnhance_Config Config;	/**< Hardware configuration */
	u32 IsReady;		/**< Device and the driver instantly
				  *  are initialized */
	u16 HSize;		/**< Active Video Horizontal Size */
	u16 VSize;		/**< Active Video Vertical Size */

	/* IRQ Callbacks */
	XEnhance_CallBack ProcStartCallBack;	/**< Callback for Processing
						  *  Start interrupt */
	void *ProcStartRef;			/**< To be passed to Process
						  *  Start interrupt
						  *  callback */
	XEnhance_CallBack FrameDoneCallBack;	/**< Callback for Frame Done
						  *  interrupt */
	void *FrameDoneRef;			/**< To be passed to the Frame
						  *  Done interrupt callback */
	XEnhance_ErrorCallBack ErrCallBack;	/**< Callback for Error
						  *  interrupt */
	void *ErrRef;				/**< To be passed to the Error
						  *  interrupt callback */
} XEnhance;

/************************** Function Prototypes ******************************/

int XEnhance_CfgInitialize(XEnhance *InstancePtr, XEnhance_Config *CfgPtr,
				u32 EffectiveAddr);
void XEnhance_Setup(XEnhance *InstancePtr);
void XEnhance_EnableDbgByPass(XEnhance *InstancePtr);
int XEnhance_IsDbgByPassEnabled(XEnhance *InstancePtr);
void XEnhance_DisableDbgBypass(XEnhance *InstancePtr);
void XEnhance_EnableDbgTestPattern(XEnhance *InstancePtr);
int XEnhance_IsDbgTestPatternEnabled(XEnhance *InstancePtr);
void XEnhance_DisableDbgTestPattern(XEnhance *InstancePtr);
u32 XEnhance_GetVersion(XEnhance *InstancePtr);
u32 XEnhance_GetDbgFrameCount(XEnhance *InstancePtr);
u32 XEnhance_GetDbgLineCount(XEnhance *InstancePtr);
u32 XEnhance_GetDbgPixelCount(XEnhance *InstancePtr);
void XEnhance_SetActiveSize(XEnhance *InstancePtr, u16 HSize, u16 VSize);
void XEnhance_GetActiveSize(XEnhance *InstancePtr, u16 *HSize, u16 *VSize);
void XEnhance_SetNoiseThreshold(XEnhance *InstancePtr, u32 Threshold);
u32 XEnhance_GetNoiseThreshold(XEnhance *InstancePtr);
void XEnhance_SetEdgeStrength(XEnhance *InstancePtr, u32 Strength);
u32 XEnhance_GetEdgeStrength(XEnhance *InstancePtr);
void XEnhance_SetHaloSuppress(XEnhance *InstancePtr, u32 Suppress);
u32 XEnhance_GetHaloSuppress(XEnhance *InstancePtr);

/* Initialization functions in xEnhance_sinit.c */
XEnhance_Config *XEnhance_LookupConfig(u16 DeviceId);

/* Self test functions in xenhance_selftest.c */
int XEnhance_SelfTest(XEnhance*InstancePtr);

/* Interrupt related functions in xEnhance_intr.c */
void XEnhance_IntrHandler(void *InstancePtr);
int XEnhance_SetCallBack(XEnhance *InstancePtr, u32 HandlerType,
			void *CallBackFunc, void *CallBackRef);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
