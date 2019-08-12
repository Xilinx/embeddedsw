/******************************************************************************
*
* Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
* @file xdualsplitter.h
* @addtogroup dual_splitter_v1_1
* @{
* @details
*
* This header file contains identifiers and register-level core functions (or
* macros), range macros, structure typedefs that can be used to access the
* Xilinx Dual Splitter core.
*
* <b>Core Features </b>
*
* For full description of Dual Splitter features, please see the hardware
* specification.
*
* <b>Initialization & Configuration</b>
*
* The application needs to do the following steps in order to use
* Dual Splitter core.
*
* - Call XDualSplitter_LookupConfig using a device ID to find the core
*   configuration.
* - Call XDualSplitter_CfgInitialize to initialize the device and the driver
*   instance associated with it.
*
* <b> Interrupts </b>
*
* The driver provides an interrupt handler XDualSplitter_IntrHandler for
* handling the interrupt from the Dual Splitter core. The users of this driver
* have to register this handler with the interrupt system and provide the
* callback functions by using XDualSplitter_SetCallBack API.
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
* The Dual Splitter driver is composed of several source files. This allows
* the user to build and link only those parts of the driver that are necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- --------------------------------------------------
* 1.00  sha 07/21/14 Initial release.
*       ms  03/17/17 Added readme.txt file in examples folder for doxygen
*                    generation.
* </pre>
*
******************************************************************************/

#ifndef XDUALSPLITTER_H_
#define XDUALSPLITTER_H_	/**< Prevent circular inclusions by using
				  *  protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xdualsplitter_hw.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

#define XDUSP_MAX_INPUT_SAMPLES		4	/**< Maximum input samples per
						  *  clock */
#define XDUSP_MAX_OUTPUT_SAMPLES	4	/**< Maximum output samples
						  *  per clock */

#define XDUSP_MAX_SEGMENTS		4	/**< Maximum number of segments
						  *  in an image */

#define XDUSP_MAX_IMG_WIDTH		3840	/**< Maximum image width */
#define XDUSP_MAX_IMG_HEIGHT		2160	/**< Maximum image height */

/**************************** Type Definitions *******************************/

/**
* This typedef contains configuration information for the Dual Splitter
* core. Each Dual Splitter device should have a configuration structure
* associated.
*/
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID of the Dual
				  *  Splitter core */
	u32 BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the core's registers */
	u32 ActiveCols;		/**< Active Maximum Image Width */
	u32 ActiveRows;		/**< Active Maximum Image Height */
	u8 MaxSegments;		/**< Maximum number of segments in an image */
	u8 MaxTDataWidth;	/**< Maximum Data bus Width. It is the
				  *  multiplication of maximum input sample
				  *  width and maximum input samples per
				  * clock */
	u8 MaxITDataSamples;	/**< Maximum Input Data Samples per clock */
	u8 MaxOTDataSamples;	/**< Maximum Output Data Samples per clock */
	u8 MaxOverlap;		/**< Maximum Overlap of the samples in the
				  *  segments */
	u8 MaxSampleWidth;	/**< Maximum number of bits in a Sample */
	u8 HasAxi4Lite;		/**< Axi4-Lite support */
	u8 HasIntrReq;		/**< IRQ support */
} XDualSplitter_Config;

/**
*
* Callback type for error interrupt.
*
* @param	CallbackRef is a callback reference passed in by the upper
*		layer when setting the callback functions and passed back
*		to the upper layer when the callback is invoked.
* @param	ErrorMask is a bit mask indicating the cause of the error.
*		Its value equals 'OR'ing one or more XDUSP_ERR_*_MASK values
*		defined in xdualsplitter_hw.h.
*
******************************************************************************/
typedef void (*XDualSplitter_ErrCallback)(void *CallbackRef, u32 ErrorMask);

/**
* The XDualSplitter driver instance data. An instance must be allocated for
* each Dual Splitter core in use.
*/
typedef struct {
	XDualSplitter_Config Config;	/**< Hardware Configuration */
	u32 IsReady;			/**< Core and the driver instance are
					  *  initialized */
	XDualSplitter_ErrCallback ErrCallback;	/**< Callback for error
						  *  interrupt */
	void *ErrRef;	/**< To be passed to the error
			  *  interrupt callback */
} XDualSplitter;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro commits all the register value changes made so far by the
* software to the Dual Splitter core.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XDualSplitter_RegUpdateEnable(XDualSplitter *InstancePtr)
*
******************************************************************************/
#define XDualSplitter_RegUpdateEnable(InstancePtr) \
	XDualSplitter_WriteReg((InstancePtr)->Config.BaseAddress, \
		XDUSP_GENR_CTL_OFFSET, \
		XDualSplitter_ReadReg((InstancePtr)->Config.BaseAddress, \
			XDUSP_GENR_CTL_OFFSET) | XDUSP_GENR_CTL_RUE_MASK)

/*****************************************************************************/
/**
*
* This macro prevents the Dual Splitter core from committing recent changes
* made so far by the software. When disabled, changes to other configuration
* registers are stored but do not effect the behavior of the core.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XDualSplitter_RegUpdateDisable(XDualSplitter *InstancePtr)
*
******************************************************************************/
#define XDualSplitter_RegUpdateDisable(InstancePtr) \
	XDualSplitter_WriteReg((InstancePtr)->Config.BaseAddress, \
		XDUSP_GENR_CTL_OFFSET, \
		XDualSplitter_ReadReg((InstancePtr)->Config.BaseAddress, \
			XDUSP_GENR_CTL_OFFSET) & (~XDUSP_GENR_CTL_RUE_MASK))

/*****************************************************************************/
/**
*
* This macro enables the Dual Splitter core.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XDualSplitter_Enable(XDualSplitter *InstancePtr)
*
******************************************************************************/
#define XDualSplitter_Enable(InstancePtr) \
	XDualSplitter_WriteReg((InstancePtr)->Config.BaseAddress, \
		XDUSP_GENR_CTL_OFFSET, \
		XDualSplitter_ReadReg((InstancePtr)->Config.BaseAddress, \
			XDUSP_GENR_CTL_OFFSET) | XDUSP_GENR_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro disables the Dual Splitter core.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XDualSplitter_Disable(XDualSplitter *InstancePtr)
*
******************************************************************************/
#define XDualSplitter_Disable(InstancePtr) \
	XDualSplitter_WriteReg((InstancePtr)->Config.BaseAddress, \
		XDUSP_GENR_CTL_OFFSET, \
		XDualSplitter_ReadReg((InstancePtr)->Config.BaseAddress, \
			XDUSP_GENR_CTL_OFFSET) & \
				~(XDUSP_GENR_CTL_EN_MASK))

/*****************************************************************************/
/**
*
* This macro enables the given individual interrupt(s) on the Dual Splitter
* core.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
* @param	IntrType is the bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XDUSP_ERR_*_MASK bits defined in xdualsplitter_hw.h.
*
* @return	None.
*
* @note		The existing enabled interrupt(s) will remain enabled.
*		C-style signature:
*		void XDualSplitter_IntrEnable(XDualSplitter *InstancePtr,
*						u32 IntrType)
*
******************************************************************************/
#define XDualSplitter_IntrEnable(InstancePtr, IntrType) \
	XDualSplitter_WriteReg((InstancePtr)->Config.BaseAddress, \
		(XDUSP_IRQ_EN_OFFSET), (IntrType & \
			XDUSP_ALL_ERR_MASK) | \
		XDualSplitter_ReadReg((InstancePtr)->Config.BaseAddress, \
			XDUSP_IRQ_EN_OFFSET))

/*****************************************************************************/
/**
*
* This macro disables the given individual interrupt(s) on the Dual Splitter
* core.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
* @param	IntrType is the bit-mask of the interrupts to be disabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XDUSP_ERR_*_MASK bits defined in xdualsplitter_hw.h.
*
* @return	None.
*
* @note		Any other interrupt not covered by parameter IntrType,
*		if enabled before this macro is called, will remain enabled.
*		C-style signature:
*		void XDualSplitter_IntrDisable(XDualSplitter *InstancePtr,
*						u32 IntrType)
*
******************************************************************************/
#define XDualSplitter_IntrDisable(InstancePtr, IntrType) \
	XDualSplitter_WriteReg((InstancePtr)->Config.BaseAddress, \
			XDUSP_IRQ_EN_OFFSET, \
		XDualSplitter_ReadReg((InstancePtr)->Config.BaseAddress, \
			XDUSP_IRQ_EN_OFFSET) & ((~IntrType) & \
				XDUSP_ALL_ERR_MASK))

/*****************************************************************************/
/**
*
* This macro returns the pending interrupts of the Dual Splitter core.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
*
* @return	The pending interrupts of the Dual Splitter. Use
*		XDUSP_ERR_*_MASK constants defined in xdualsplitter_hw.h to
*		interpret this value. The returned value is a logical AND of
*		the contents of the GENR_ERROR Register and the IRQ_ENABLE
*		Register.
*
* @note		C-style signature:
*		u32 XDualSplitter_IntrGetPending(XDualSplitter *InstancePtr)
*
******************************************************************************/
#define XDualSplitter_IntrGetPending(InstancePtr) \
	XDualSplitter_ReadReg((InstancePtr)->Config.BaseAddress, \
			XDUSP_IRQ_EN_OFFSET) & \
		XDualSplitter_ReadReg((InstancePtr)->Config.BaseAddress, \
			XDUSP_GENR_ERR_OFFSET) & \
				XDUSP_ALL_ERR_MASK

/*****************************************************************************/
/**
*
* This macro clears/acknowledges pending interrupts of the Dual Splitter core.
* in the General error register. Bit positions of 1 will be cleared.
*
* @param	InstancePtr is a pointer to the XDualSplitter core instance.
* @param	IntrType is the pending interrupts to clear/acknowledge.
*		Use OR'ing of XDUSP_ERR_*_MASK constants defined in
*		xdualsplitter_hw.h to create this parameter value.
*
* @return	None.
*
* @note		C-style signature:
*		void XDualSplitter_IntrClear(XDualSplitter *InstancePtr,
*						u32 IntrType)
*
******************************************************************************/
#define XDualSplitter_IntrClear(InstancePtr, IntrType) \
	XDualSplitter_WriteReg((InstancePtr)->Config.BaseAddress, \
		XDUSP_GENR_ERR_OFFSET, IntrType & XDUSP_ALL_ERR_MASK)

/************************** Function Prototypes ******************************/

/* Initialization function in xdualsplitter_sinit.c */
XDualSplitter_Config *XDualSplitter_LookupConfig(u16 DeviceId);

/* Initialization and control functions in xdualsplitter.c */
s32 XDualSplitter_CfgInitialize(XDualSplitter *InstancePtr,
				XDualSplitter_Config *CfgPtr,
				u32 EffectiveAddr);

void XDualSplitter_Reset(XDualSplitter *InstancePtr);

void XDualSplitter_SetImageSize(XDualSplitter *InstancePtr, u16 Height,
				u16 Width);
void XDualSplitter_GetImageSize(XDualSplitter *InstancePtr, u16 *Height,
				u16 *Width);

void XDualSplitter_SetImgParam(XDualSplitter *InstancePtr,
				u8 InputSamples, u8 OutputSamples,
				u8 ImageSegments, u8 Overlap);
void XDualSplitter_GetImgParam(XDualSplitter *InstancePtr,
				u8 *InputSamples, u8 *OutputSamples,
				u8 *ImageSegments, u8 *Overlap);

/* Self test function in xdualsplitter_selftest.c */
s32 XDualSplitter_SelfTest(XDualSplitter *InstancePtr);

/* Interrupt related functions in xdualsplitter_intr.c */
void XDualSplitter_IntrHandler(void *InstancePtr);
void XDualSplitter_SetCallback(XDualSplitter *InstancePtr,
				void *CallbackFunc, void *CallbackRef);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
