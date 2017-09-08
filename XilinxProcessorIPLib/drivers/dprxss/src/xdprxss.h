/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
* @file xdprxss.h
* @addtogroup dprxss_v4_1
* @{
* @details
*
* This is the main header file for Xilinx DisplayPort Receiver Subsystem core.
* It abstracts Subsystem cores and provides high level API's to application
* developer.
*
* <b>Core Features</b>
*
* For a full description of DisplayPort Receiver Subsystem core, please
* see the hardware specification.
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* DisplayPort Receiver Subsystem core to be ready.
*
* - Call XDpRxSs_LookupConfig using a device ID to find the core
*   configuration.
* - Call XDpRxSs_CfgInitialize to initialize the device and the driver
*   instance associated with it.
*
* <b>Interrupts</b>
*
* The DisplayPort RX Subsystem driver provides the interrupt handlers
* - XDpRxSs_DpIntrHandler
* - XDpRxSs_HdcpIntrHandler
* - XDpRxSs_TmrCtrIntrHandler, for handling the interrupt from the DisplayPort,
* optional HDCP and Timer Counter sub-cores respectively. The users of this
* driver have to register this handler with the interrupt system and provide
* the callback functions by using XDpRxSs_SetCallBack API.
*
* <b>Virtual Memory</b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b>Threads</b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b>Asserts</b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b>Building the driver</b>
*
* The DisplayPort Receiver Subsystem driver is composed of several source
* files. This allows the user to build and link only those parts of the driver
* that are necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- ----------------------------------------------------------
* 1.00 sha 05/18/15 Initial release.
* 2.00 sha 10/05/15 Removed HDCP interrupt handler types.
*                   Added HDCP and Timer Counter support.
* 3.0  sha 02/19/16 Removed indexing from enum XDpRxSs_HandlerType.
*                   Added handler type as enum for HDCP:
*                   XDPRXSS_HANDLER_HDCP_RPTR_TDSA_EVENT.
*                   Added function: XDpRxSs_DownstreamReady.
* 3.1  als 08/08/16 Added HDCP timeout functionality.
* 3.1  aad 09/07/16 Updates to support 64-bit base addresses.
* 4.0  aad 12/01/16 Added interrupt handler for HDCP authentication
*      ms  01/23/17 Modified xil_printf statement in main function for all
*                   examples to ensure that "Successfully ran" and "Failed"
*                   strings are available in all examples. This is a fix
*                   for CR-965028.
*      ms  03/17/17 Modified readme.txt file in examples folder for doxygen
*                   generation.
* 4.1  tu  09/08/17 Added three driver side interrupt handler for Video,
*                   NoVideo and PowerChange events
* </pre>
*
******************************************************************************/
#ifndef XDPRXSS_H_
#define XDPRXSS_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xdprxss_hw.h"
#include "xil_assert.h"
#include "xstatus.h"

/* Subsystem sub-cores header files */
#include "xdprxss_dprx.h"
#include "xdprxss_iic.h"
#include "xdprxss_hdcp1x.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from sub-cores.
*/
typedef enum {
	XDPRXSS_HANDLER_DP_VM_CHG_EVENT = 1,	/**< Video mode change event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_PWR_CHG_EVENT,	/**< Power state change
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_NO_VID_EVENT,	/**< No video event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_VBLANK_EVENT,	/**< Vertical blanking event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_TLOST_EVENT,		/**< Training lost event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_VID_EVENT,		/**< Valid video event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_INFO_PKT_EVENT,		/**< Info packet event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_EXT_PKT_EVENT,	/**< Extension packet event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_TDONE_EVENT,		/**< Training done event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_BW_CHG_EVENT,	/**< Bandwidth change event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_DWN_REQ_EVENT,	/**< Down request event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_DWN_REP_EVENT,	/**< Down reply event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_AUD_OVRFLW_EVENT,	 /**< Audio packet overflow
						    *  event interrupt type for
						    *  DisplayPort core */
	XDPRXSS_HANDLER_DP_PAYLOAD_ALLOC_EVENT,		/**< Payload allocation
							  *  event interrupt
							  *  type for
							  *  DisplayPort
							  *  core */
	XDPRXSS_HANDLER_DP_ACT_RX_EVENT,	/**< ACT sequence received
						  *  event interrupt type for
						  *  DisplayPort core */
	XDPRXSS_HANDLER_DP_CRC_TEST_EVENT,	/**< CRC test start event
						  *  interrupt type for
						  *  DisplayPort core */
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	XDPRXSS_HANDLER_HDCP_RPTR_TDSA_EVENT,	/**< Repeater Trigger
						  *  Downstream AUTH event
						  *  interrupt type for
						  *  HDCP core */
	XDPRXSS_HANDLER_HDCP_AUTHENTICATED,	/**< HDCP Authentication
						  *  completion interrupt type for  HDCP core */
#endif
	XDPRXSS_HANDLER_UNPLUG_EVENT,		/**< Unplug event type for
						  *  DisplayPort RX
						  *  Subsystem */
	XDPRXSS_HANDLER_LINKBW_EVENT,		/**< Link BW event type for
						  *  DisplayPort RX Subsystem
						  */
	XDPRXSS_HANDLER_PLL_RESET_EVENT,	/**< PLL reset event type for
						  *  DisplayPort RX Subsystem
						  */
	XDPRXSS_DRV_HANDLER_DP_PWR_CHG_EVENT,   /**< Drv power state change
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_DRV_HANDLER_DP_VID_EVENT,       /**< Drv Valid video event
						  *  interrupt type for
						  *  DisplayPort core */
	XDPRXSS_DRV_HANDLER_DP_NO_VID_EVENT     /**< Drv No video event
						  *  interrupt type for
						  *  DisplayPort core */
} XDpRxSs_HandlerType;

/**
* User input structure
*/
typedef struct {
	u8 Bpc;			/**< Bits per color */
	u8 LaneCount;		/**< Lane count */
	u8 LinkRate;		/**< Link rate */
	u8 MstSupport;		/**< Multi-stream transport (MST) support */
	u8 NumOfStreams;	/**< The total number of MST streams */
} XDpRxSs_UsrOpt;

/**
* DisplayPort Sub-core structure.
*/
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of DisplayPort
				  *  Receiver core. */
	XDp_Config DpConfig;	/**< DisplayPort core configuration
				  *  information */
} XDpRxSs_DpSubCore;

/**
* IIC Sub-core structure.
*/
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of DisplayPort
				  *  Receiver core. */
	XIic_Config IicConfig;	/**< IIC core configuration
				  *  information */
} XDpRxSs_IicSubCore;

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
/**
* High-Bandwidth Content Protection (HDCP) Sub-core structure.
*/
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of HDCP core. */
	XHdcp1x_Config Hdcp1xConfig;	/**< HDCP core configuration
					  *  information */
} XDpRxSs_Hdcp1xSubCore;

/**
* Timer Counter Sub-core structure.
*/
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of Timer
				  *  Counter core */
	XTmrCtr_Config TmrCtrConfig;	/**< Timer Counter core
					  * configuration information */
} XDpRxSs_TmrCtrSubCore;
#endif

/**
* This typedef contains configuration information for the DisplayPort
* Receiver Subsystem core. Each DisplayPort RX Subsystem core should have
* a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID of the
				  *  DisplayPort RX Subsystem core */
	UINTPTR BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the core's registers */
	u8 SecondaryChEn;	/**< This Subsystem core supports audio packets
				  *  being sent by the secondary channel. */
	u8 MaxNumAudioCh;	/**< The total number of Audio channels
				  *  supported by this core instance. */
	u8 MaxBpc;		/**< The maximum bits/color supported by this
				  *  Subsystem core */
	u8 HdcpEnable;		/**< This Subsystem core supports digital
				  *  content protection. */
	u8 MaxLaneCount;	/**< The maximum lane count supported by this
				  *  core instance. */
	u8 MstSupport;		/**< Multi-stream transport (MST) mode is
				  *  enabled by this core instance. */
	u8 NumMstStreams;	/**< The total number of MST streams supported
				  *  by this core instance. */
	u8 ColorFormat;		/**< Type of color format supported by this
				  *  core instance. */
	XDpRxSs_DpSubCore DpSubCore;	/**< DisplayPort Configuration */
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	XDpRxSs_Hdcp1xSubCore Hdcp1xSubCore;	/**< HDCP Configuration */
#endif
	XDpRxSs_IicSubCore IicSubCore;	/**< IIC Configuration */
#if ((XPAR_XHDCP_NUM_INSTANCES > 0) && (XPAR_XTMRCTR_NUM_INSTANCES > 0))
	XDpRxSs_TmrCtrSubCore TmrCtrSubCore;	/**< Timer Counter
						  *  Configuration */
#endif
} XDpRxSs_Config;

/*****************************************************************************/
/**
*
* Callback type which represents the handler for events.
*
* @param	InstancePtr is a pointer to the XDpRxSs instance.
*
* @note		None.
*
******************************************************************************/
typedef void (*XDpRxSs_Callback)(void *InstancePtr);

/**
* The DisplayPort RX Subsystem driver instance data. An instance must be
* allocated for each core in use.
*/
typedef struct {
	XDpRxSs_Config Config;		/**< Hardware Configuration */
	u32 IsReady;			/**< Core and the driver instance are
					  *  initialized */
	/* Sub-core instances */
	XDp *DpPtr;			/**< DisplayPort sub-core instance */
	XIic *IicPtr;			/**< IIC sub-core instance */
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	XHdcp1x *Hdcp1xPtr;		/**< HDCP sub-core instance */
	XTmrCtr *TmrCtrPtr;		/**< Timer Counter sub-core instance */
	u8 TmrCtrResetDone;		/**< Timer reset done. This is used for
					  *  MacBook which authenticates just
					  *  after training is done. This
					  *  ensures that system does not do
					  *  anything until this variable set
					  *  to one.*/
#endif

	/* Callback */
	XDpRxSs_Callback PllResetCallback;	/**< Callback function for PLL
						  *  reset */
	void *PllResetRef;		/**< A pointer to the user data passed
					  *  to the PLL reset callback
					  *  function */
	XDpRxSs_Callback LinkBwCallback;	/**< Callback function for
						  *  link bandwidth */
	void *LinkBwRef;		/**< A pointer to the user data passed
					  *  to the link bandwidth callback
					  *  function */
	XDpRxSs_Callback UnplugCallback;	/**< Callback function for
						  *  unplug event */
	void *UnplugRef;		/**< A pointer to the user data passed
					  *  to the unplug event callback
					  *  function */

	/* Vertical blank */
	u8 VBlankEnable;		/**< Vertical Blank Enable */
	u8 VBlankCount;			/**< Vertical Blank Count */

	/* User options */
	XDpRxSs_UsrOpt UsrOpt;		/**< User Options structure */
} XDpRxSs;

/***************** Macros (Inline Functions) Definitions *********************/

/**
* Callback type which represents a custom timer wait handler.
*/
#define XDpRxSs_TimerHandler		XDp_TimerHandler

/*****************************************************************************/
/**
*
* This function macro enables the display timing generator (DTG).
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XDpRxSs_DtgEnable(XDpRxSs *InstancePtr)
*
******************************************************************************/
#define XDpRxSs_DtgEnable(InstancePtr)	XDp_RxDtgEn((InstancePtr)->DpPtr)

/*****************************************************************************/
/**
*
* This function macro disables the display timing generator (DTG).
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XDpRxSs_DtgDisable(XDpRxSs *InstancePtr)
*
******************************************************************************/
#define XDpRxSs_DtgDisable(InstancePtr)	XDp_RxDtgDis((InstancePtr)->DpPtr)

/*****************************************************************************/
/**
*
* This function macro enables audio stream packets on the main link.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XDpRxSs_AudioEnable(XDpRxSs *InstancePtr)
*
******************************************************************************/
#define XDpRxSs_AudioEnable(InstancePtr) \
	XDp_RxAudioEn((InstancePtr)->DpPtr)

/*****************************************************************************/
/**
*
* This function macro disables audio stream packets on the main link.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XDpRxSs_AudioDisable(XDpRxSs *InstancePtr)
*
******************************************************************************/
#define XDpRxSs_AudioDisable(InstancePtr) \
	XDp_RxAudioDis((InstancePtr)->DpPtr)

/*****************************************************************************/
/**
*
* This function macro resets the reception of audio stream packets on the
* main link.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XDpRxSs_AudioReset(XDpRxSs *InstancePtr)
*
******************************************************************************/
#define XDpRxSs_AudioReset(InstancePtr) \
	XDp_RxAudioReset((InstancePtr)->DpPtr)

/******************************************************************************/
/**
*
* This function macro is the delay/sleep function for the XDpRxSs driver.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
* @param	MicroSeconds is the number of microseconds to delay/sleep for.
*
* @return	None.
*
* @note		C-style signature:
*		void XDpRxSs_WaitUs(XDpRxSs *InstancePtr)
*
*******************************************************************************/
#define XDpRxSs_WaitUs(InstancePtr, MicroSeconds) \
	XDp_WaitUs((InstancePtr)->DpPtr, MicroSeconds)

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#define XDpRxSs_Printf		XHdcp1x_Printf	/**< Debug printf */
#define XDpRxSs_LogMsg		XHdcp1x_LogMsg	/**< Debug log message */
#endif

/************************** Function Prototypes ******************************/

/* Initialization function in xdprxss_sinit.c */
XDpRxSs_Config* XDpRxSs_LookupConfig(u16 DeviceId);

/* Initialization and control functions in xdprxss.c */
u32 XDpRxSs_CfgInitialize(XDpRxSs *InstancePtr, XDpRxSs_Config *CfgPtr,
				UINTPTR EffectiveAddr);
u32 XDpRxSs_Start(XDpRxSs *InstancePtr);
void XDpRxSs_Reset(XDpRxSs *InstancePtr);
u32 XDpRxSs_SetLinkRate(XDpRxSs *InstancePtr, u8 LinkRate);
u32 XDpRxSs_SetLaneCount(XDpRxSs *InstancePtr, u8 LaneCount);
u32 XDpRxSs_ExposePort(XDpRxSs *InstancePtr, u8 Port);
u32 XDpRxSs_CheckLinkStatus(XDpRxSs *InstancePtr);
u32 XDpRxSs_HandleDownReq(XDpRxSs *InstancePtr);
void XDpRxSs_SetUserPixelWidth(XDpRxSs *InstancePtr, u8 UserPixelWidth);

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
/* Optional HDCP related functions */
u32 XDpRxSs_HdcpEnable(XDpRxSs *InstancePtr);
u32 XDpRxSs_HdcpDisable(XDpRxSs *InstancePtr);
u32 XDpRxSs_Poll(XDpRxSs *InstancePtr);
u32 XDpRxSs_SetPhysicalState(XDpRxSs *InstancePtr, u32 PhyState);
u32 XDpRxSs_SetLane(XDpRxSs *InstancePtr, u32 Lane);
u32 XDpRxSs_Authenticate(XDpRxSs *InstancePtr);
u32 XDpRxSs_IsAuthenticated(XDpRxSs *InstancePtr);
u64 XDpRxSs_GetEncryption(XDpRxSs *InstancePtr);
void XDpRxSs_SetDebugPrintf(XDpRxSs *InstancePtr, XDpRxSs_Printf PrintfFunc);
void XDpRxSs_SetDebugLogMsg(XDpRxSs *InstancePtr, XDpRxSs_LogMsg LogFunc);
void XDpRxSs_StartTimer(XDpRxSs *InstancePtr);
void XDpRxSs_StopTimer(XDpRxSs *InstancePtr);
u32 XDpRxSs_DownstreamReady(XDpRxSs *InstancePtr);
void XDpRxSs_HandleTimeout(XDpRxSs *InstancePtr);
#endif

void XDpRxSs_ReportCoreInfo(XDpRxSs *InstancePtr);
void XDpRxSs_ReportLinkInfo(XDpRxSs *InstancePtr);
void XDpRxSs_ReportMsaInfo(XDpRxSs *InstancePtr);
void XDpRxSs_ReportDp159BitErrCount(XDpRxSs *InstancePtr);
void XDpRxSs_ReportHdcpInfo(XDpRxSs *InstancePtr);

/* Self test function in xdprxss_selftest.c */
u32 XDpRxSs_SelfTest(XDpRxSs *InstancePtr);

/* Interrupt functions in xdprxss_intr.c */
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
void XDpRxSs_HdcpIntrHandler(void *InstancePtr);
void XDpRxSs_TmrCtrIntrHandler(void *InstancePtr);
#endif
void XDpRxSs_DpIntrHandler(void *InstancePtr);
u32 XDpRxSs_SetCallBack(XDpRxSs *InstancePtr, u32 HandlerType,
			void *CallbackFunc, void *CallbackRef);
void XDpRxSs_SetUserTimerHandler(XDpRxSs *InstancePtr,
		XDpRxSs_TimerHandler CallbackFunc, void *CallbackRef);

/* DpRxSs Interrupt Related Function */
void XDpRxSs_DrvNoVideoHandler(void *InstancePtr);
void XDpRxSs_DrvVideoHandler(void *InstancePtr);
void XDpRxSs_DrvPowerChangeHandler(void *InstancePtr);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
