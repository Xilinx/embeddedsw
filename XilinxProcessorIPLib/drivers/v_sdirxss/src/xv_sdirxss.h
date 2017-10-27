/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc. All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xv_sdirxss.h
*
* This is main header file of the Xilinx SDI RX Subsystem driver
*
* <b>SDI RX Subsystem Overview</b>
*
* Video Subsystem is a collection of IP cores bounded together by software
* to provide an abstract view of the processing pipe. It hides all the
* complexities of programming the underlying cores from end user.
*
* <b>Subsystem Driver Features</b>
*
* Video Subsystem supports following features
*	- AXI Stream Input/Output interface
*	- 2 pixel-wide video interface
*	- 10 bits per component
*	- YCbCr 4:2:2 color space
*	- Up to 4k2k 60Hz resolution (12G) at both Input and Output interface
*	- Interlaced input support (1080i 50Hz/60Hz)
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  jsr    07/17/17 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XV_SDIRXSS_H_
#define XV_SDIRXSS_H_        /**< Prevent circular inclusions
			      * by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xv_sdivid.h"
#include "xvidc.h"
#include "xv_sdirx.h"

/************************** Constant Definitions *****************************/
/** @name Bitmasks for interrupt callbacks
 *
 * Please refer to SDI Rx driver for details of the bitmasks.
 * The application should use the XV_SDIRXSS_IER* masks in the call back functions
 * to decode the exact cause of interrupt and handle it accordingly.
 * @{
 */
#define XV_SDIRXSS_IER_VIDEO_LOCK_MASK		XV_SDIRX_IER_VIDEO_LOCK_MASK
#define XV_SDIRXSS_IER_VIDEO_UNLOCK_MASK	XV_SDIRX_IER_VIDEO_UNLOCK_MASK
#define XV_SDIRXSS_IER_OVERFLOW_MASK		XV_SDIRX_IER_OVERFLOW_MASK
#define XV_SDIRXSS_IER_UNDERFLOW_MASK		XV_SDIRX_IER_UNDERFLOW_MASK
#define XV_SDIRXSS_IER_ALLINTR_MASK		XV_SDIRX_IER_ALLINTR_MASK

/**************************** Type Definitions *******************************/
/**
 * This typedef contains the enum for various logging events.
 */

typedef enum {
	XV_SDIRXSS_LOG_EVT_NONE = 1,	/**< Log event none. */
	XV_SDIRXSS_LOG_EVT_SDIRX_INIT,	/**< Log event SDIRX Init. */
	XV_SDIRXSS_LOG_EVT_START,	/**< Log event SDIRXSS Start. */
	XV_SDIRXSS_LOG_EVT_STOP,	/**< Log event SDIRXSS Stop. */
	XV_SDIRXSS_LOG_EVT_RESET,	/**< Log event SDIRXSS Reset. */
	XV_SDIRXSS_LOG_EVT_STREAMUP,	/**< Log event Stream Up. */
	XV_SDIRXSS_LOG_EVT_STREAMDOWN,	/**< Log event Stream Down. */
	XV_SDIRXSS_LOG_EVT_OVERFLOW,	/**< Log event Over flow. */
	XV_SDIRXSS_LOG_EVT_UNDERFLOW,	/**< Log event Under flow. */
	XV_SDIRXSS_LOG_EVT_STREAMSTART, /**< Log event Stream Start. */
	XV_SDIRXSS_LOG_EVT_SETSTREAM,	/**< Log event SDIRXSS Setstream. */
	XV_SDIRXSS_LOG_EVT_DUMMY,	/**< Dummy Event should be last */
} XV_SdiRxSs_LogEvent;

/**
 * This typedef contains the logging mechanism for debug.
 */
typedef struct {
	u16 DataBuffer[256];		/**< Log buffer with event data. */
	u8 HeadIndex;			/**< Index of the head entry of the
					  Event/DataBuffer. */
	u8 TailIndex;			/**< Index of the tail entry of the
					  Event/DataBuffer. */
} XV_SdiRxSs_Log;


/**
 * These constants specify different types of handler and used to differentiate
 * interrupt requests from peripheral.
 */
typedef enum {
	XV_SDIRXSS_HANDLER_STREAM_DOWN = 1,	/**< Handler for stream down event */
	XV_SDIRXSS_HANDLER_STREAM_UP,		/**< Handler for stream up event */
	XV_SDIRXSS_HANDLER_OVERFLOW,		/**< Handler for over flow event */
	XV_SDIRXSS_HANDLER_UNDERFLOW		/**< Handler for under flow event */
} XV_SdiRxSs_HandlerType;
/*@}*/

/**
 * Sub-Core Configuration Table
 */
typedef struct {
	u16 IsPresent;  /**< Flag to indicate if sub-core is present in the design*/
	u16 DeviceId;   /**< Device ID of the sub-core */
	UINTPTR AbsAddr; /**< sub-core offset from subsystem base address */
} XV_SdiRxSs_SubCore;

/**
 * This typedef contains configuration information for the SDI RX core.
 * Each SDI RX device should have a configuration structure associated.
 */
typedef struct {
	u16 DeviceId;       /**< DeviceId is the unique ID of the SDI RX core */
	UINTPTR BaseAddress;      /**< BaseAddress is the physical base address of the
				    subsystem address range */
	XVidC_PixelsPerClock Ppc;
	u8 IsEdhIncluded;
	u8 MaxRateSupported;
	XV_SdiRxSs_SubCore SdiRx;       /**< Sub-core instance configuration */
} XV_SdiRxSs_Config;

/**
 * Callback type for interrupt.
 *
 * @param	CallbackRef is a callback reference passed in by the upper
 *		layer when setting the callback functions, and passed back to
 *		the upper layer when the callback is invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
 */
typedef void (*XV_SdiRxSs_Callback)(void *CallbackRef);

/**
 * The XV_SdiRxSs driver instance data. An instance must be allocated for each
 * SDI RX core in use.
 */
typedef struct {
	XV_SdiRxSs_Config Config;	/**< Hardware Configuration */
	u32 IsReady;			/**< Core and the driver instance are initialized */
	XV_SdiRxSs_Log Log;		/**< A log of events. */
	XV_SdiRx *SdiRxPtr;		/**< handle to sub-core driver instance */

	XV_SdiRxSs_Callback StreamDownCallback; /**< Callback for stream down event */
	void *StreamDownRef;		/**< To be passed to the stream down callback */

	XV_SdiRxSs_Callback StreamUpCallback; /**< Callback for stream up event */
	void *StreamUpRef;		/**< To be passed to the stream up callback */

	XV_SdiRxSs_Callback OverFlowCallback; /**< Callback for Over flow event */
	void *OverFlowRef;		/**< To be passed to the Over flow callback */

	XV_SdiRxSs_Callback UnderFlowCallback; /**< Callback for Under Flow event */
	void *UnderFlowRef;		/**< To be passed to the Under Flow callback */

	u8 IsStreamUp;			/**< SDI RX Stream Up */
} XV_SdiRxSs;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 *
 * This macro reads the RX version
 *
 * @param	InstancePtr is a pointer to the XV_SdiRX core instance.
 *
 * @return	None.
 *
 *
 ******************************************************************************/
#define XV_SdiRxSs_GetVersion(InstancePtr) \
	XV_SdiRxSs_ReadReg((InstancePtr)->Config.BaseAddress, \
			(XV_SDIRXSS_VER_OFFSET))

/************************** Function Prototypes ******************************/

/* Initialization function in xv_sdirxss_sinit.c */
XV_SdiRxSs_Config *XV_SdiRxSs_LookupConfig(u32 DeviceId);

/* Initialization and control functions in xv_sdirxss.c */
int XV_SdiRxSs_CfgInitialize(XV_SdiRxSs *InstancePtr,
		XV_SdiRxSs_Config *CfgPtr,
		UINTPTR EffectiveAddr);
void XV_SdiRxSS_SdiRxIntrHandler(XV_SdiRxSs *InstancePtr);
void XV_SdiRxSs_StreamFlowEnable(XV_SdiRxSs *InstancePtr);
void XV_SdiRxSs_StreamFlowDisable(XV_SdiRxSs *InstancePtr);
void XV_SdiRxSs_Start(XV_SdiRxSs *InstancePtr, XV_SdiRx_SearchMode Mode);
void XV_SdiRxSs_Stop(XV_SdiRxSs *InstancePtr);
void XV_SdiRxSs_ReportDetectedError(XV_SdiRxSs *InstancePtr);
int XV_SdiRxSs_SetCallback(XV_SdiRxSs *InstancePtr, u32 HandlerType,
		void *CallbackFunc, void *CallbackRef);
void XV_SdiRxSs_ReportCoreInfo(XV_SdiRxSs *InstancePtr);
void XV_SdiRxSs_ReportInfo(XV_SdiRxSs *InstancePtr);
void XV_SdiRxSs_ReportDebugInfo(XV_SdiRxSs *InstancePtr);
u32 *XV_SdiRxSs_GetPayloadId(XV_SdiRxSs *InstancePtr, u8 StreamId);
XSdiVid_Transport *XV_SdiRxSs_GetTransport(XV_SdiRxSs *InstancePtr);
XVidC_VideoStream *XV_SdiRxSs_GetVideoStream(XV_SdiRxSs *InstancePtr,
		u8 StreamId);
XSdiVid_TransMode XV_SdiRxSs_GetTransportMode(XV_SdiRxSs *InstancePtr);
u8 XV_SdiRxSs_GetTransportBitRate(XV_SdiRxSs *InstancePtr);
int XV_SdiRxSs_IsStreamUp(XV_SdiRxSs *InstancePtr);
void XV_SdiRxSs_IntrEnable(XV_SdiRxSs *InstancePtr, u32 IntrMask);
void XV_SdiRxSs_IntrDisable(XV_SdiRxSs *InstancePtr, u32 IntrMask);

/* Self test function in xv_sdirxss_selftest.c */
u32 XV_SdiRxSs_SelfTest(XV_SdiRxSs *InstancePtr);

/* XV_SdiRxSs_log.c: Logging functions. */
void XV_SdiRxSs_LogReset(XV_SdiRxSs *InstancePtr);
void XV_SdiRxSs_LogWrite(XV_SdiRxSs *InstancePtr, XV_SdiRxSs_LogEvent Evt,
		u8 Data);
u16 XV_SdiRxSs_LogRead(XV_SdiRxSs *InstancePtr);
void XV_SdiRxSs_LogDisplay(XV_SdiRxSs *InstancePtr);


/************************** Variable Declarations ****************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
