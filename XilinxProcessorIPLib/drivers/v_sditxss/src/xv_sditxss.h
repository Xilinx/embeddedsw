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
* @file xv_sditxss.h
*
* This is main header file of the Xilinx SDI TX Subsystem driver
*
* <b>SDI Transmitter Subsystem Overview</b>
*
* SDI TX Subsystem is a collection of IP cores bounded together by software
* to provide an abstract view of the processing pipe. It hides all the
* complexities of programming the underlying cores from end user.
*
* <b>Subsystem Driver Features</b>
*
* SDI Subsystem supports following features
*   - AXI Stream Input/Output interface
*   - 2 pixel-wide video interface
*   - 10 bits per component
*   - YCbCr color space
*   - Up to 4k2k 60Hz resolution at both Input and Output interface
*   - Interlaced output support (1080i 50Hz/60Hz)

* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  jsr  07/17/17 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XV_SDITXSS_H_
#define XV_SDITXSS_H_        /**< Prevent circular inclusions
						   *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xv_sdivid.h"
#include "xvidc.h"
#include "xv_sditx.h"
#include "xvtc.h"

/************************** Constant Definitions *****************************/
/** @name Bitmasks for interrupt callbacks
 *
 * Please refer to SDI Tx driver for details of the bitmasks.
 * The application should use the XV_SDITXSS_IER* masks in the call back functions
 * to decode the exact cause of interrupt and handle it accordingly.
 * @{
 */
#define XV_SDITXSS_IER_GTTX_RSTDONE_MASK	XV_SDITX_IER_GTTX_RSTDONE_MASK
#define XV_SDITXSS_IER_TX_CE_ALIGN_ERR_MASK	XV_SDITX_IER_TX_CE_ALIGN_ERR_MASK
#define XV_SDITXSS_IER_AXI4S_VID_LOCK_MASK	XV_SDITX_IER_AXI4S_VID_LOCK_MASK
#define XV_SDITXSS_IER_OVERFLOW_MASK		XV_SDITX_IER_OVERFLOW_MASK
#define XV_SDITXSS_IER_UNDERFLOW_MASK		XV_SDITX_IER_UNDERFLOW_MASK
#define XV_SDITXSS_IER_ALLINTR_MASK		XV_SDITX_IER_ALLINTR_MASK

/**************************** Type Definitions *******************************/
/**
* This typedef contains the enum for various logging events.
*/

typedef enum {
	XV_SDITXSS_LOG_EVT_NONE = 1,		/**< Log event none. */
	XV_SDITXSS_LOG_EVT_SDITX_INIT,	/**< Log event SDITX Init. */
	XV_SDITXSS_LOG_EVT_VTC_INIT,	/**< Log event VTC Init. */
	XV_SDITXSS_LOG_EVT_START,	/**< Log event SDITXSS Start. */
	XV_SDITXSS_LOG_EVT_STOP,	/**< Log event SDITXSS Stop. */
	XV_SDITXSS_LOG_EVT_RESET,	/**< Log event SDITXSS Reset. */
	XV_SDITXSS_LOG_EVT_STREAMUP,	/**< Log event Stream Up. */
	XV_SDITXSS_LOG_EVT_STREAMDOWN,	/**< Log event Stream Down. */
	XV_SDITXSS_LOG_EVT_OVERFLOW,	/**< Log event Over flow. */
	XV_SDITXSS_LOG_EVT_UNDERFLOW,	/**< Log event Under Flow. */
	XV_SDITXSS_LOG_EVT_STREAMSTART, /**< Log event Stream Start. */
	XV_SDITXSS_LOG_EVT_STREAMCFG,
	XV_SDITXSS_LOG_EVT_DUMMY,		/**< Dummy Event should be last */
} XV_SdiTxSs_LogEvent;

/**
 * This typedef contains the logging mechanism for debug.
 */
typedef struct {
	u16 DataBuffer[256];		/**< Log buffer with event data. */
	u8 HeadIndex;				/**< Index of the head entry of the
									 Event/DataBuffer. */
	u8 TailIndex;				/**< Index of the tail entry of the
									 Event/DataBuffer. */
} XV_SdiTxSs_Log;


/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from peripheral.
*/
typedef enum {
    XV_SDITXSS_HANDLER_GTREADY = 1,		/**< Handler for connect event */
    XV_SDITXSS_HANDLER_OVERFLOW,		/**< Handler for overflow event */
    XV_SDITXSS_HANDLER_UNDERFLOW		/**< Handler for underflow event */
} XV_SdiTxSs_HandlerType;
/*@}*/

/**
 * Sub-Core Configuration Table
 */
typedef struct {
  u16 IsPresent;  /**< Flag to indicate if sub-core is present in the design*/
  u16 DeviceId;   /**< Device ID of the sub-core */
  UINTPTR AbsAddr; /**< sub-core offset from subsystem base address */
} XV_SdiTxSs_SubCore;

/**
* This typedef contains configuration information for the SDI TX core.
* Each SDI TX device should have a configuration structure associated.
*/
typedef struct {
    u16 DeviceId;		/**< DeviceId is the unique ID of the SDI TX core */
    UINTPTR BaseAddress;	/**< BaseAddress is the physical base address of the					subsystem address range */
    XVidC_PixelsPerClock Ppc;	/**< Supported Pixel per Clock */
    u8 IsEdhIncluded;
    u8 MaxRateSupported;
    XV_SdiTxSs_SubCore SdiTx;	/**< Sub-core instance configuration */
	XV_SdiTxSs_SubCore Vtc;	/**< Sub-core instance configuration */
} XV_SdiTxSs_Config;

/**
* Callback type for interrupt.
*
* @param  CallbackRef is a callback reference passed in by the upper
*   layer when setting the callback functions, and passed back to
*   the upper layer when the callback is invoked.
*
* @return None.
*
* @note   None.
*
*/
typedef void (*XV_SdiTxSs_Callback)(void *CallbackRef);

/**
* The XV_SdiTxSs driver instance data. An instance must be allocated for each
* SDI TX core in use.
*/
typedef struct {
    XV_SdiTxSs_Config Config;		/**< Hardware Configuration */
    u32 IsReady;				/**< Core and the driver instance are initialized */
    XV_SdiTxSs_Log Log;				/**< A log of events. */
    XV_SdiTx *SdiTxPtr;           /**< handle to sub-core driver instance */
	XVtc *VtcPtr;                   /**< handle to sub-core driver instance */

	XV_SdiTxSs_Callback GtReadyCallback; /**< Callback for stream down event */
	void *GtReadyRef;  /**< To be passed to the stream down callback */

	XV_SdiTxSs_Callback OverFlowCallback; /**< Callback for Over flow event */
	void *OverFlowRef;  /**< To be passed to the Over flow callback */

	XV_SdiTxSs_Callback UnderFlowCallback; /**< Callback for Under flow event */
	void *UnderFlowRef;  /**< To be passed to the Under flow callback */

	u8 IsStreamUp;                /**< SDI TX Stream Up */
} XV_SdiTxSs;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro reads the TX version
*
* @param  InstancePtr is a pointer to the XV_SdiTX core instance.
*
* @return None.
*
*
******************************************************************************/
#define XV_SdiTxSs_GetVersion(InstancePtr) \
  XV_SdiTxSs_ReadReg((InstancePtr)->Config.BaseAddress, \
  (XV_SDITXSS_VER_OFFSET))

/************************** Function Prototypes ******************************/

/* Initialization function in xv_sditxss_sinit.c */
XV_SdiTxSs_Config *XV_SdiTxSs_LookupConfig(u32 DeviceId);

/* Initialization and control functions in xv_sditxss.c */
int XV_SdiTxSs_CfgInitialize(XV_SdiTxSs *InstancePtr,
    XV_SdiTxSs_Config *CfgPtr,
    UINTPTR EffectiveAddr);
void XV_SdiTxSS_SdiTxIntrHandler(XV_SdiTxSs *InstancePtr);

int XV_SdiTxSs_SetCallback(XV_SdiTxSs *InstancePtr,
		u32 HandlerType,
		void *CallbackFunc,
		void *CallbackRef);

void XV_SdiTxSs_StreamStart(XV_SdiTxSs *InstancePtr);
void XV_SdiTxSs_StreamConfig(XV_SdiTxSs *InstancePtr);
void XV_SdiTxSs_Stop(XV_SdiTxSs *InstancePtr);
u32 *XV_SdiTxSs_GetPayloadId(XV_SdiTxSs *InstancePtr, u8 StreamId);
XSdiVid_Transport *XV_SdiTxSs_GetTransport(XV_SdiTxSs *InstancePtr);
XVidC_VideoStream *XV_SdiTxSs_GetVideoStream(XV_SdiTxSs *InstancePtr,
		u8 StreamId);
void XV_SdiTxSs_SetVideoStream(XV_SdiTxSs *InstancePtr,
		XVidC_VideoStream VidStream);

void XV_SdiTxSs_ReportInfo(XV_SdiTxSs *InstancePtr);
void XV_SdiTxSs_ReportDebugInfo(XV_SdiTxSs *InstancePtr, u32 VtcFlag);
void XV_SdiTxSs_ReportStreamInfo(XV_SdiTxSs *InstancePtr);
void XV_SdiTxSs_ReportDetectedError(XV_SdiTxSs *InstancePtr);
int XV_SdiTxSs_IsStreamUp(XV_SdiTxSs *InstancePtr);
void XV_SdiTxSs_IntrEnable(XV_SdiTxSs *InstancePtr, u32 IntrMask);
void XV_SdiTxSs_IntrDisable(XV_SdiTxSs *InstancePtr, u32 IntrMask);
u32 XV_SdiTxSs_GetPayload(XV_SdiTxSs *InstancePtr, XVidC_VideoMode VideoMode,
				XSdiVid_TransMode SdiMode, u8 DataStream);
u32 XV_SdiTxSs_SetStream(XV_SdiTxSs *InstancePtr, XV_SdiTx_StreamSelId SelId,
				u32 Data, u8 StreamId);
/* Self test function in xv_sditxss_selftest.c */
u32 XV_SdiTxSs_SelfTest(XV_SdiTxSs *InstancePtr);

/* XV_SdiTxSs_log.c: Logging functions. */
void XV_SdiTxSs_LogReset(XV_SdiTxSs *InstancePtr);
void XV_SdiTxSs_LogWrite(XV_SdiTxSs *InstancePtr,
		XV_SdiTxSs_LogEvent Evt,
		u8 Data);
u16 XV_SdiTxSs_LogRead(XV_SdiTxSs *InstancePtr);
void XV_SdiTxSs_LogDisplay(XV_SdiTxSs *InstancePtr);


/************************** Variable Declarations ****************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
