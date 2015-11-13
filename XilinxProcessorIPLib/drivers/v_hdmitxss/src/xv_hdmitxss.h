/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
* @file xv_hdmitxss.h
*
* This is main header file of the Xilinx HDMI TX Subsystem driver
*
* <b>HDMI Transmitter Subsystem Overview</b>
*
* HDMI TX Subsystem is a collection of IP cores bounded together by software
* to provide an abstract view of the processing pipe. It hides all the
* complexities of programming the underlying cores from end user.
*
* <b>Subsystem Driver Features</b>
*
* HDMI Subsystem supports following features
* 	- AXI Stream Input/Output interface
* 	- 1, 2 or 4 pixel-wide video interface
* 	- 8/10/12/16 bits per component
* 	- RGB & YCbCr color space
* 	- Up to 4k2k 60Hz resolution at both Input and Output interface
* 	- Interlaced input support (1080i 50Hz/60Hz)

* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00         10/07/15 Initial release.

* </pre>
*
******************************************************************************/

#ifndef HDMITXSS_H /**< prevent circular inclusions by using protection macros*/
#define HDMITXSS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xintc.h"
#include "xvidc.h"
#include "xvidc_edid.h"
#include "xv_hdmitx.h"
#include "xvtc.h"
#include "xtmrctr.h"
#include "xhdcp1x.h"

#define XV_HDMITXSS_HDCP_KEYSEL 0x00u


/****************************** Type Definitions ******************************/
/** @name Handler Types
* @{
*/
/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from peripheral.
*/
typedef enum {
	XV_HDMITXSS_HANDLER_CONNECT = 1,	    /**< Handler for connect     */
	XV_HDMITXSS_HANDLER_VS,			        /**< Handler for vsync       */
	XV_HDMITXSS_HANDLER_STREAM_DOWN,		/**< Handler for stream down */
	XV_HDMITXSS_HANDLER_STREAM_UP			/**< Handler for stream up   */
} XV_HdmiTxSs_HandlerType;
/*@}*/

/**
 * Sub-Core Configuration Table
 */
typedef struct
{
  u16 IsPresent;  /**< Flag to indicate if sub-core is present in the design*/
  u16 DeviceId;   /**< Device ID of the sub-core */
  u32 AddrOffset; /**< sub-core offset from subsystem base address */
}XV_HdmiTxSs_SubCore;

/**
 * Video Processing Subsystem configuration structure.
 * Each subsystem device should have a configuration structure associated
 * that defines the MAX supported sub-cores within subsystem
 */

typedef struct
{
	u16 DeviceId;	            /**< DeviceId is the unique ID  of the device */
	u32 BaseAddress;          /**< BaseAddress is the physical base address of the
								subsystem address range */
	u32 HighAddress;          /**< HighAddress is the physical MAX address of the
								subsystem address range */
	XVidC_PixelsPerClock Ppc;         /**< Supported Pixel per Clock */
	u8 MaxBitsPerPixel;               /**< Maximum  Supported Color Depth */
	XV_HdmiTxSs_SubCore HdcpTimer;    /**< Sub-core instance configuration */
	XV_HdmiTxSs_SubCore Hdcp;         /**< Sub-core instance configuration */
	XV_HdmiTxSs_SubCore HdmiTx;       /**< Sub-core instance configuration */
	XV_HdmiTxSs_SubCore Vtc;          /**< Sub-core instance configuration */
} XV_HdmiTxSs_Config;

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
typedef void (*XV_HdmiTxSs_Callback)(void *CallbackRef);

/**
* The XVprocss driver instance data. The user is required to allocate a variable
* of this type for every XVprocss device in the system. A pointer to a variable
* of this type is then passed to the driver API functions.
*/
typedef struct
{
	XV_HdmiTxSs_Config Config;	/**< Hardware configuration */
	u32 IsReady;		 /**< Device and the driver instance are initialized */

	XTmrCtr *HdcpTimerPtr;    	/**< handle to sub-core driver instance */
	XHdcp1x *HdcpPtr;         	/**< handle to sub-core driver instance */
	XV_HdmiTx *HdmiTxPtr;     	/**< handle to sub-core driver instance */
	XVtc *VtcPtr;             	/**< handle to sub-core driver instance */

	/* Callbacks */
	XV_HdmiTxSs_Callback ConnectCallback;	/**< Callback for connect
										* event interrupt */
	void *ConnectRef;	/**< To be passed to the connect
										* interrupt callback */

	XV_HdmiTxSs_Callback VsCallback; /**< Callback for Vsync event interrupt */
	void *VsRef; /**< To be passed to the Vsync interrupt callback */

	XV_HdmiTxSs_Callback StreamDownCallback; /**< Callback for stream
											* down callback */
	void *StreamDownRef; /**< To be passed to the stream down callback */

	XV_HdmiTxSs_Callback StreamUpCallback; /**< Callback for stream up
											* callback */
	void *StreamUpRef;	/**< To be passed to the stream up callback */

	/**< Scratch pad */
	u8 SamplingRate;              /**< HDMI TX Sampling rate */
	u8 IsStreamConnected;         /**< HDMI TX Stream Connected */
	u8 AudioEnabled;              /**< HDMI TX Audio Enabled */
	u8 AudioMute;                 /**< HDMI TX Audio Mute */
	u8 AudioChannels;             /**< Number of Audio Channels */

	XVidC_DelayHandler UserTimerWaitUs;	/**< Custom user function for
							delay/sleep. */
	void *UserTimerPtr;           /**< Pointer to a timer instance
							used by the custom user
							delay/sleep function. */
} XV_HdmiTxSs;

/************************** Macros Definitions *******************************/

/************************** Function Prototypes ******************************/
XV_HdmiTxSs_Config *XV_HdmiTxSs_LookupConfig(u32 DeviceId);
void XV_HdmiTxSs_ReportCoreInfo(XV_HdmiTxSs *InstancePtr);
void XV_HdmiTxSs_SetUserTimerHandler(XV_HdmiTxSs *InstancePtr,
		XVidC_DelayHandler CallbackFunc, void *CallbackRef);
void XV_HdmiTxSS_HdmiTxIntrHandler(XV_HdmiTxSs *InstancePtr);
void XV_HdmiTxSS_HdcpIntrHandler(XV_HdmiTxSs *InstancePtr);
void XV_HdmiTxSS_HdcpTimerIntrHandler(XV_HdmiTxSs *InstancePtr);
int XV_HdmiTxSs_CfgInitialize(XV_HdmiTxSs *InstancePtr,
	XV_HdmiTxSs_Config *CfgPtr,
	u32 EffectiveAddr);
void XV_HdmiTxSs_Start(XV_HdmiTxSs *InstancePtr);
void XV_HdmiTxSs_Stop(XV_HdmiTxSs *InstancePtr);
void XV_HdmiTxSs_Reset(XV_HdmiTxSs *InstancePtr);
int XV_HdmiTxSs_SetCallback(XV_HdmiTxSs *InstancePtr,
	u32 HandlerType,
	void *CallbackFuncPtr,
	void *CallbackRef);
int XV_HdmiTxSs_ReadEdid(XV_HdmiTxSs *InstancePtr, u8 *BufferPtr);
void XV_HdmiTxSs_ShowEdid(XV_HdmiTxSs *InstancePtr);
void XV_HdmiTxSs_StreamStart(XV_HdmiTxSs *InstancePtr);
void XV_HdmiTxSs_SendAuxInfoframe(XV_HdmiTxSs *InstancePtr, void *AuxPtr);
void XV_HdmiTxSs_SetAudioChannels(XV_HdmiTxSs *InstancePtr, u8 AudioChannels);
void XV_HdmiTxSs_AudioMute(XV_HdmiTxSs *InstancePtr, u8 Enable);
u32 XV_HdmiTxSs_SetStream(XV_HdmiTxSs *InstancePtr,
	XVidC_VideoMode VideoMode,
	XVidC_ColorFormat ColorFormat,
	XVidC_ColorDepth Bpc);
u32 XV_HdmiTxSs_SetStreamReducedBlanking(XV_HdmiTxSs *InstancePtr);
XVidC_VideoStream *XV_HdmiTxSs_GetVideoStream(XV_HdmiTxSs *InstancePtr);
void XV_HdmiTxSs_SetVideoStream(XV_HdmiTxSs *InstancePtr,
									XVidC_VideoStream VidStream);
void XV_HdmiTxSs_SetSamplingRate(XV_HdmiTxSs *InstancePtr, u8 SamplingRate);
void XV_HdmiTxSs_SetVideoIDCode(XV_HdmiTxSs *InstancePtr, u8 Vic);
void XV_HdmiTxSs_SetVideoStreamType(XV_HdmiTxSs *InstancePtr, u8 StreamType);
void XV_HdmiTxSs_SetVideoStreamScramblingFlag(XV_HdmiTxSs *InstancePtr,
															u8 IsScrambled);
void XV_HdmiTxSs_SetTmdsClockRatio(XV_HdmiTxSs *InstancePtr, u8 Ratio);
u32 XV_HdmiTxSs_GetTmdsClockFreqHz(XV_HdmiTxSs *InstancePtr);
int XV_HdmiTxSs_DetectHdmi20(XV_HdmiTxSs *InstancePtr);
void XV_HdmiTxSs_RefClockChangeInit(XV_HdmiTxSs *InstancePtr);
void XV_HdmiTxSs_ReportTiming(XV_HdmiTxSs *InstancePtr);
void XV_HdmiTxSs_ReportSubcoreVersion(XV_HdmiTxSs *InstancePtr);
void XV_HdmiTxSs_HdcpEnable(XV_HdmiTxSs *InstancePtr, u8 Enable);
u8 XV_HdmiTxSs_HdcpPoll(XV_HdmiTxSs *InstancePtr);
u8 XV_HdmiTxSs_HdcpStart(XV_HdmiTxSs *InstancePtr);
void XV_HdmiTxSs_HdcpTimerCallback(void *CallBackRef, u8 TimerChannel);
int XV_HdmiTxSs_HdcpTimerStart(const XHdcp1x *InstancePtr, u16 TimeoutInMs);
int XV_HdmiTxSs_HdcpTimerStop(const XHdcp1x *InstancePtr);
int XV_HdmiTxSs_HdcpTimerBusyDelay(const XHdcp1x *InstancePtr, u16 DelayInMs);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
