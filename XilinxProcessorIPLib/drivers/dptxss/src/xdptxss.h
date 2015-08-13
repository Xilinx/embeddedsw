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
* @file xdptxss.h
* @addtogroup dptxss_v1_0
* @{
* @details
*
* This is the main header file for Xilinx DisplayPort Transmitter Subsystem
* core. It abstracts Subsystem cores and provides high level API's to
* application developer.
*
* <b>Core Features</b>
*
* For a full description of DisplayPort Transmitter Subsystem core, please
* see the hardware specification.
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* DisplayPort Transmitter Subsystem core to be ready.
*
* - Call XDpTxSs_LookupConfig using a device ID to find the core
*   configuration.
* - Call XDpTxSs_CfgInitialize to initialize the device and the driver
*   instance associated with it.
*
* <b>Interrupts</b>
*
* The DisplayPort TX Subsystem driver provides an interrupt handler
* XDpTxSs_DpIntrHandler for handling the interrupt from the DisplayPort
* sub-core. The users of this driver have to register this handler with
* the interrupt system and provide the callback functions by using
* XDpTxSs_SetCallBack API.
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
* The DisplayPort Transmitter Subsystem driver is composed of several source
* files. This allows the user to build and link only those parts of the driver
* that are necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* 1.00 sha 07/21/15 Included renamed sub-cores header files.
* </pre>
*
******************************************************************************/
#ifndef XDPTXSS_H_
#define XDPTXSS_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xdptxss_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xvidc.h"
#include "xdebug.h"

/* Subsystem sub-cores header files */
#include "xdptxss_dptx.h"
#include "xdptxss_dualsplitter.h"
#include "xdptxss_vtc.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from sub-cores.
*/
typedef enum {
	XDPTXSS_HANDLER_DP_HPD_EVENT = 1,	/**< A HPD event interrupt
						  *  type for DisplayPort
						  *  core */
	XDPTXSS_HANDLER_DP_HPD_PULSE		/**< A HPD pulse interrupt
						  *  type for DisplayPort
						  *  core */
} XDpTxSs_HandlerType;

/**
* User input structure
*/
typedef struct {
	XVidC_VideoMode VmId;	/**< Video Mode ID */
	u8 Bpc;			/**< Bits per color */
	u8 MstSupport;		/**< Multi-stream transport (MST) support */
	u8 NumOfStreams;	/**< The total number of MST streams */
} XDpTxSs_UsrOpt;

/**
* VTC Sub-core structure.
*/
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of VTC core. */
	XVtc_Config VtcConfig;	/**< Video Timing Controller (VTC) core
				  * configuration information */
} XDpTxSs_VtcSubCore;

#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
/**
* Dual Splitter Sub-core structure.
*/
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of Dual
				  *  Splitter core. */
	XDualSplitter_Config DsConfig;	/**< Dual Splitter core configuration
					  *  information */
} XDpTxSs_DsSubCore;
#endif

/**
* DisplayPort Sub-core structure.
*/
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of DisplayPort
				  *  Transmitter core. */
	XDp_Config DpConfig;	/**< DisplayPort core configuration
				  *  information */
} XDpTxSs_DpSubCore;

/**
* This typedef contains configuration information for the DisplayPort
* Transmitter Subsystem core. Each DisplayPort TX Subsystem core should have
* a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID of the
				  *  DisplayPort TX Subsystem core */
	u32 BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the core's registers */
	u8 SecondaryChEn;	/**< This Subsystem core supports audio packets
				  *  being sent by the secondary channel. */
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
	XDpTxSs_DpSubCore DpSubCore;	/**< DisplayPort Configuration */
#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	XDpTxSs_DsSubCore DsSubCore;	/**< Dual Splitter Configuration */
#endif
	XDpTxSs_VtcSubCore VtcSubCore[XDPTXSS_NUM_STREAMS]; /**< VTC
							      *  Configura-
							      *  tion */
} XDpTxSs_Config;

/**
* The DisplayPort TX Subsystem driver instance data. An instance must be
* allocated for each core in use.
*/
typedef struct {
	XDpTxSs_Config Config;		/**< Hardware Configuration */
	u32 IsReady;			/**< Core and the driver instance are
					  *  initialized */
	/* Sub-core instances */
#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	XDualSplitter *DsPtr;		/**< Dual Splitter sub-core instance */
#endif
	XDp *DpPtr;			/**< DisplayPort sub-core instance */
	XVtc *VtcPtr[XDPTXSS_NUM_STREAMS];/**< Maximum number of VTC sub-core
					  *  instances */

	XDpTxSs_UsrOpt UsrOpt;		/**< User Options structure */
} XDpTxSs;

/***************** Macros (Inline Functions) Definitions *********************/

/**
* Callback type which represents a custom timer wait handler.
*/
#define XDpTxSs_TimerHandler		XDp_TimerHandler

/************************** Function Prototypes ******************************/

/* Initialization function in xdptxss_sinit.c */
XDpTxSs_Config* XDpTxSs_LookupConfig(u16 DeviceId);

/* Initialization and control functions in xaxi4s_switch.c */
u32 XDpTxSs_CfgInitialize(XDpTxSs *InstancePtr, XDpTxSs_Config *CfgPtr,
				u32 EffectiveAddr);
u32 XDpTxSs_Start(XDpTxSs *InstancePtr);
void XDpTxSs_Stop(XDpTxSs *InstancePtr);
void XDpTxSs_Reset(XDpTxSs *InstancePtr);
u32 XDpTxSs_SetBpc(XDpTxSs *InstancePtr, u8 Bpc);
u32 XDpTxSs_SetVidMode(XDpTxSs *InstancePtr, XVidC_VideoMode VidMode);
u32 XDpTxSs_SetLinkRate(XDpTxSs *InstancePtr, u8 LinkRate);
u32 XDpTxSs_SetLaneCount(XDpTxSs *InstancePtr, u8 LaneCount);
u32 XDpTxSs_SetTransportMode(XDpTxSs *InstancePtr, u8 Mode);
u32 XDpTxSs_IsConnected(XDpTxSs *InstancePtr);
u32 XDpTxSs_CheckLinkStatus(XDpTxSs *InstancePtr);
u32 XDpTxSs_IsMstCapable(XDpTxSs *InstancePtr);
u32 XDpTxSs_GetRxCapabilities(XDpTxSs *InstancePtr);
u32 XDpTxSs_GetEdid(XDpTxSs *InstancePtr, u8 *Edid);
u32 XDpTxSs_GetRemoteEdid(XDpTxSs *InstancePtr, u8 SinkNum, u8 *Edid);

void XDpTxSs_ReportCoreInfo(XDpTxSs *InstancePtr);
void XDpTxSs_ReportLinkInfo(XDpTxSs *InstancePtr);
void XDpTxSs_ReportMsaInfo(XDpTxSs *InstancePtr);
void XDpTxSs_ReportSinkCapInfo(XDpTxSs *InstancePtr);
void XDpTxSs_ReportSplitterInfo(XDpTxSs *InstancePtr);
void XDpTxSs_ReportVtcInfo(XDpTxSs *InstancePtr);

/* Self test function in xdptxss_selftest.c */
u32 XDpTxSs_SelfTest(XDpTxSs *InstancePtr);

/* Interrupt functions in xdptxss_intr.c */
void XDpTxSs_DpIntrHandler(void *InstancePtr);
u32 XDpTxSs_SetCallBack(XDpTxSs *InstancePtr, u32 HandlerType,
			void *CallbackFunc, void *CallbackRef);
void XDpTxSs_SetUserTimerHandler(XDpTxSs *InstancePtr,
		XDpTxSs_TimerHandler CallbackFunc, void *CallbackRef);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
