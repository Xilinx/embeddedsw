/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdptxss.h
* @addtogroup dptxss_v6_7
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
* The DisplayPort TX Subsystem driver provides the interrupt handlers
* - XDpTxSs_DpIntrHandler
* - XDpTxSs_HdcpIntrHandler
* - XDpTxSs_TmrCtrIntrHandler, for handling the interrupt from the DisplayPort,
* optional HDCP and Timer Counter sub-cores respectively. The users of this
* driver have to register this handler with the interrupt system and provide
* the callback functions by using XDpTxSs_SetCallBack API.
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
* ---- --- -------- ---------------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* 1.00 sha 07/21/15 Included renamed sub-cores header files.
* 2.00 sha 08/07/15 Added new handler types: lane count, link rate,
*                   pre-emphasis voltage swing adjust and set MSA.
*                   Added support for customized main stream attributes.
*                   Added function: XDpTxSs_SetHasRedriverInPath.
*                   Added HDCP support data structure.
* 2.00 sha 09/28/15 Added HDCP and Timer Counter functions.
* 3.0  sha 02/19/16 Added handler type as enums for HDCP:
*                   XDPTXSS_HANDLER_HDCP_RPTR_DWN_STRM_RDY,
*                   XDPTXSS_HANDLER_HDCP_RPTR_EXCHG.
*                   Added function: XDpTxSs_ReadDownstream,
*                   XDpTxSs_HandleTimeout.
* 4.0  aad 05/13/16 Expose API to set (a)synchronous clock mode from DP driver.
* 4.1  als 08/08/16 Synchronize with new HDCP APIs.
*      aad 09/06/16 Updates to support 64-bit base addresses.
*      ms  01/23/17 Modified xil_printf statement in main function for all
*                   examples to ensure that "Successfully ran" and "Failed"
*                   strings are available in all examples. This is a fix
*                   for CR-965028.
*      ms  03/17/17 Modified readme.txt file in examples folder for doxygen
*                   generation.
* 5.0  tu  08/10/17 Adjusted BS symbol for equal timing
* 5.0  tu  09/08/17 Added two interrupt handler that addresses driver's
*                   internal callback function of application
*                   DrvHpdEventHandler and DrvHpdPulseHandler
*                   Added HPD user data stucture XDpTxSs_UsrHpdPulseData
*                   and XDpTxSs_UsrHpdEventData
* 5.0  jb  02/21/19 Added HDCP22 support.
* 					Made the Timer counter available for both HDCP1x and 22.
* 6.4  rg  09/01/20 Added handler type as enum for extended packet transmit
*                   done interrupt.
* 6.4  rg  09/26/20 Added support for YUV420 color format.
*
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
#include "xdptxss_hdcp1x.h"
#include "xdptxss_vtc.h"
#include "xdptxss_hdcp22.h"

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
	XDPTXSS_HANDLER_DP_HPD_PULSE,		/**< A HPD pulse interrupt
						  *  type for DisplayPort
						  *  core */
	XDPTXSS_HANDLER_DP_LANE_COUNT_CHG,	/**< Lane count change
						  *  interrupt type for
						  *  DisplayPort core */
	XDPTXSS_HANDLER_DP_LINK_RATE_CHG,	/**< Link rate change
						  *  interrupt type for
						  *  DisplayPort core */
	XDPTXSS_HANDLER_DP_PE_VS_ADJUST,	/**< Pre-emphasis and voltage
						  *  swing change interrupt
						  *  type for DisplayPort
						  *  core */
	XDPTXSS_HANDLER_HDCP_RPTR_EXCHG,	/**< Repeater Exchange
						  *  interrupt type for
						  *  HDCP core */
	XDPTXSS_HANDLER_HDCP22_AUTHENTICATED, /**< Handler for
					       * HDCP22 unauthenticated
					       * event */
	XDPTXSS_HANDLER_HDCP22_UNAUTHENTICATED, /**< Handler for
						 * HDCP22 unauthenticated
						 * event */
	XDPTXSS_HANDLER_HDCP22_UPDATE_DOWNSTREAM_TOPOLOGY, /**< Handler for
						 * HDCP22 downstream topology available */
	XDPTXSS_HANDLER_DP_SET_MSA,		/**< Set MSA immediate change
						  *  change interrupt type for
						  *  DisplayPort core */
	XDPTXSS_DRV_HANDLER_DP_HPD_EVENT,	/**< Driver's internal HPD
						  *  event interrupt type for
						  *  DisplayPort core */
	XDPTXSS_DRV_HANDLER_DP_HPD_PULSE,	/**< Driver's HPD pulse
						  *  interrupt type for
						  *  DisplayPort core */
	XDPTXSS_HANDLER_DP_EXT_PKT_EVENT,	/**< Driver's extended
						  *  packet transmit done
						  *  interrupt type for
						  *  DisplayPort core */
	XDPTXSS_DRV_HANDLER_DP_EXT_PKT_EVENT,	/**< Driver's extended
						  *	 packet transmit done
						  *	 interrupt type for
						  *	 DisplayPort core */
	XDPTXSS_HANDLER_DP_VSYNC	/**< A Vsync interrupt
						  *  type for DisplayPort
						  *  core */
} XDpTxSs_HandlerType;

/**
* These constants specify the HDCP protection schemes
*/
typedef enum
{
    XDPTXSS_HDCP_NONE,   /**< No content protection */
    XDPTXSS_HDCP_1X,     /**< HDCP 1X */
    XDPTXSS_HDCP_22,     /**< HDCP 2.2 */
    XDPTXSS_HDCP_BOTH    /**< Both HDCP 1.4 and 2.2 */
} XDpTxSs_HdcpProtocol;

/**
* These constants specify HDCP repeater content stream management type
*/
typedef enum
{
    XDPTXSS_HDCP_STREAMTYPE_0, /**< HDCP Stream Type 0 */
    XDPTXSS_HDCP_STREAMTYPE_1  /**< HDCP Stream Type 1 */
} XDpTxSs_HdcpContentStreamType;

/**
* User input structure
*/
typedef struct {
	XVidC_VideoMode VmId;	/**< Video Mode ID */
	u8 Bpc;			/**< Bits per color */
	u8 MstSupport;		/**< Multi-stream transport (MST) support */
	u8 NumOfStreams;	/**< The total number of MST streams */
	u8 VtcAdjustBs;		/**< Adjustment in Blanking symbol timing */
} XDpTxSs_UsrOpt;

/*
 * This typedef contains configuration information for the
 * DpTxSs subcore instances.
 */
typedef struct {
	u16 DeviceId;	/**< Device ID of the sub-core */
	UINTPTR AbsAddr;/**< Absolute Base Address of the Sub-cores*/
} XDpTxSs_SubCoreConfig;

/**
* VTC Sub-core structure.
*/
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of VTC core. */
	XVtc_Config VtcConfig;	/**< Video Timing Controller (VTC) core
				  * configuration information */
} XDpTxSs_VtcSubCore;

/**
* Dual Splitter Sub-core structure.
*/
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of Dual
				  *  Splitter core. */
	XDpTxSs_SubCoreConfig DsConfig;	/**< Dual Splitter core configuration
					 *  information */
} XDpTxSs_DsSubCore;

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
* High-Bandwidth Content Protection (HDCP) Sub-core structure.
*/
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of HDCP core */
	XDpTxSs_SubCoreConfig Hdcp1xConfig;	/**< HDCP core configuration
						 *  information */
} XDpTxSs_Hdcp1xSubCore;

/**
* Timer Counter Sub-core structure.
*/
typedef struct {
	u16 IsPresent;		/**< Flag to hold the presence of Timer
				  *  Counter core */
	XDpTxSs_SubCoreConfig TmrCtrConfig;	/**< Timer Counter core
						 * configuration information */
} XDpTxSs_TmrCtrSubCore;

/**
 * HDCP22 Sub-core structure.
 */
typedef struct
{
	u16 IsPresent;  /**< Flag to indicate if sub-core is present
			  in the design*/
	XDpTxSs_SubCoreConfig Hdcp22Config; /**< HDCP22 core configuration */
} XDpTxSs_Hdcp22SubCore;

/**
* This typedef contains configuration information for the DisplayPort
* Transmitter Subsystem core. Each DisplayPort TX Subsystem core should have
* a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID of the
				  *  DisplayPort TX Subsystem core */
	UINTPTR BaseAddress;	/**< BaseAddress is the physical base address
				  *  of the core's registers */
	u8 SecondaryChEn;	/**< This Subsystem core supports audio packets
				  *  being sent by the secondary channel. */
	u8 MaxBpc;		/**< The maximum bits/color supported by this
				  *  Subsystem core */
	u8 HdcpEnable;		/**< This Subsystem core supports digital
				  *  content protection. */
	u8 Hdcp22Enable;		/**< This Subsystem core supports digital
					  *  content protection(HDCP22). */
	u8 MaxLaneCount;	/**< The maximum lane count supported by this
				  *  core instance. */
	u8 MstSupport;		/**< Multi-stream transport (MST) mode is
				  *  enabled by this core instance. */
	u8 NumMstStreams;	/**< The total number of MST streams supported
				  *  by this core instance. */
	XDpTxSs_TmrCtrSubCore TmrCtrSubCore;	/**< Timer Counter
							  *  Configuration */
	XDpTxSs_DpSubCore DpSubCore;	/**< DisplayPort Configuration */
	XDpTxSs_Hdcp1xSubCore Hdcp1xSubCore;	/**< HDCP Configuration */
	XDpTxSs_Hdcp22SubCore Hdcp22SubCore;	/**< HDCP22 Configuration */
	XDpTxSs_DsSubCore DsSubCore;	/**< Dual Splitter Configuration */
	XDpTxSs_VtcSubCore VtcSubCore[XDPTXSS_NUM_STREAMS]; /**< VTC
							      *  Configura-
							      *  tion */
} XDpTxSs_Config;

/**
* HPD Pulse User Data structure
*/
typedef struct {
	u8 Edid[128];
	u8 AuxValues[9];
	u8 Lane0Sts;
	u8 Lane2Sts;
	u8 LaneAlignStatus;
	u8 BwSet;
	u8 LaneSet;
} XDpTxSs_UsrHpdPulseData;

/**
* HPD Event User Data structure
*/
typedef struct {
	u8 MaxCapNew;
	u8 MaxCapLanesNew;
	u8 Lane0Sts;
	u8 Lane2Sts;
	u8 Rd200;
	u8 EdidOrg[XDP_EDID_BLOCK_SIZE];
	u8 EdidOrg_1[XDP_EDID_BLOCK_SIZE];
	u8 EdidOrg_2[XDP_EDID_BLOCK_SIZE];
	u8 Dpcd[88];
	u8 Tmp[12];
} XDpTxSs_UsrHpdEventData;

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
	XHdcp1x *Hdcp1xPtr;		/**< HDCP sub-core instance */
	XTmrCtr *TmrCtrPtr;		/**< Timer Counter sub-core instance */
	XHdcp22_Tx_Dp  *Hdcp22Ptr;		/**< handle to sub-core driver
					  instance */
	XDp *DpPtr;			/**< DisplayPort sub-core instance */
	XVtc *VtcPtr[XDPTXSS_NUM_STREAMS];/**< Maximum number of VTC sub-core
					  *  instances */

	XDpTxSs_UsrOpt UsrOpt;		/**< User Options structure */
	XDpTxSs_UsrHpdPulseData UsrHpdPulseData; /**< User HPD Pulse data*/
	XDpTxSs_UsrHpdEventData UsrHpdEventData; /**< User HPD Event data*/
	u8 link_up_trigger;
	u8 no_video_trigger;
	XDpTxSs_HdcpProtocol    HdcpProtocol;    /**< HDCP protocol selected */
	u32 HdcpIsReady;     /**< HDCP ready flag */
	XDpTxSs_HdcpProtocol HdcpCapability;  /**< HDCP protocol desired */
	XDpTxSs_HdcpEventQueue HdcpEventQueue; /**< HDCP22 event queue */
	u8 *Hdcp22Lc128Ptr;			/**< Pointer to HDCP 2.2 LC128*/
	u8 *Hdcp22SrmPtr;			/**< Pointer to HDCP 2.2 SRM */
} XDpTxSs;

/***************** Macros (Inline Functions) Definitions *********************/

/**
* Callback type which represents a custom timer wait handler.
*/
#define XDpTxSs_TimerHandler		XDp_TimerHandler

/**
* Main-Stream attributes.
*/
#define XDpTxSs_MainStreamAttributes	XDp_TxMainStreamAttributes

#define XDpTxSs_Printf		XHdcp1x_Printf	/**< Debug printf */
#define XDpTxSs_LogMsg		XHdcp1x_LogMsg	/**< Debug log message */

#define XDpTxSs_HdcpIsReady(InstancePtr) \
	(InstancePtr)->HdcpIsReady

/************************** Function Prototypes ******************************/

/* Initialization function in xdptxss_sinit.c */
XDpTxSs_Config* XDpTxSs_LookupConfig(u16 DeviceId);

/* Initialization and control functions in xaxi4s_switch.c */
u32 XDpTxSs_CfgInitialize(XDpTxSs *InstancePtr, XDpTxSs_Config *CfgPtr,
				UINTPTR EffectiveAddr);
u32 XDpTxSs_Start(XDpTxSs *InstancePtr);
u32 XDpTxSs_StartCustomMsa(XDpTxSs *InstancePtr,
		XDpTxSs_MainStreamAttributes *MsaConfigCustom);
void XDpTxSs_Stop(XDpTxSs *InstancePtr);
void XDpTxSs_Reset(XDpTxSs *InstancePtr);
void XDpTxSs_VtcAdjustBSTimingEnable(XDpTxSs *InstancePtr);
void XDpTxSs_VtcAdjustBSTimingDisable(XDpTxSs *InstancePtr);
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
void XDpTxSs_SetHasRedriverInPath(XDpTxSs *InstancePtr, u8 Set);
void XDpTxSs_SetUserPixelWidth(XDpTxSs *InstancePtr, u8 UserPixelWidth,
				u8 StreamId);

/* Optional HDCP related functions */
u32 XDpTxSs_Authenticate(XDpTxSs *InstancePtr);
u32 XDpTxSs_IsAuthenticated(XDpTxSs *InstancePtr);
u32 XDpTxSs_HdcpEnable(XDpTxSs *InstancePtr);
u32 XDpTxSs_HdcpDisable(XDpTxSs *InstancePtr);
int XDpTxSs_HdcpSetCapability(XDpTxSs *InstancePtr,
		XDpTxSs_HdcpProtocol Protocol);
int XDpTxSs_HdcpReset(XDpTxSs *InstancePtr);
int XDpTxSs_HdcpSetProtocol(XDpTxSs *InstancePtr,
		XDpTxSs_HdcpProtocol Protocol);
u32 XDpTxSs_EnableEncryption(XDpTxSs *InstancePtr, u64 StreamMap);
u32 XDpTxSs_DisableEncryption(XDpTxSs *InstancePtr, u64 StreamMap);
u32 XDpTxSs_Poll(XDpTxSs *InstancePtr);
u32 XDpTxSs_IsHdcpCapable(XDpTxSs *InstancePtr);
u64 XDpTxSs_GetEncryption(XDpTxSs *InstancePtr);
u32 XDpTxSs_SetPhysicalState(XDpTxSs *InstancePtr, u32 PhyState);
u32 XDpTxSs_SetLane(XDpTxSs *InstancePtr, u32 Lane);
void XDpTxSs_SetDebugPrintf(XDpTxSs *InstancePtr, XDpTxSs_Printf PrintfFunc);
void XDpTxSs_SetDebugLogMsg(XDpTxSs *InstancePtr, XDpTxSs_LogMsg LogFunc);
u32 XDpTxSs_ReadDownstream(XDpTxSs *InstancePtr);
void XDpTxSs_HandleTimeout(XDpTxSs *InstancePtr);

void XDpTxSs_ReportCoreInfo(XDpTxSs *InstancePtr);
void XDpTxSs_ReportLinkInfo(XDpTxSs *InstancePtr);
void XDpTxSs_ReportMsaInfo(XDpTxSs *InstancePtr);
void XDpTxSs_ReportSinkCapInfo(XDpTxSs *InstancePtr);
void XDpTxSs_ReportSplitterInfo(XDpTxSs *InstancePtr);
void XDpTxSs_ReportVtcInfo(XDpTxSs *InstancePtr);
void XDpTxSs_ReportHdcpInfo(XDpTxSs *InstancePtr);

/* Self test function in xdptxss_selftest.c */
u32 XDpTxSs_SelfTest(XDpTxSs *InstancePtr);

/* Interrupt functions in xdptxss_intr.c */
void XDpTxSs_HdcpIntrHandler(void *InstancePtr);
void XDpTxSs_TmrCtrIntrHandler(void *InstancePtr);
void XDpTxSs_Hdcp22SetKey(XDpTxSs *InstancePtr,
		XDpTxSs_Hdcp22KeyType KeyType, u8 *KeyPtr);
void XDpTxSs_DpIntrHandler(void *InstancePtr);
u32 XDpTxSs_SetCallBack(XDpTxSs *InstancePtr, u32 HandlerType,
			void *CallbackFunc, void *CallbackRef);
void XDpTxSs_SetUserTimerHandler(XDpTxSs *InstancePtr,
		XDpTxSs_TimerHandler CallbackFunc, void *CallbackRef);
u32 XDpTxSs_CheckVscColorimetrySupport(XDpTxSs *InstancePtr);
u32 XDpTxSs_SetVscExtendedPacket(XDpTxSs *InstancePtr, XDp_TxVscExtPacket VscPkt);
void XDpTxss_EnableVscColorimetry(XDpTxSs *InstancePtr, u8 Enable);

/* DpTxSs Interrupt Related Internal Functions */
void XDpTxSs_HpdEventProcess(void *InstancePtr);
void XDpTxSs_HpdPulseProcess(void *InstancePtr);
void XDpTxSs_WriteVscExtPktProcess(void * InstancePtr);

/******************* Macros (Inline Functions) Definitions *******************/

/******************************************************************************/
/**
 * This function enables or disables synchronous clock mode for a video stream.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Stream is the stream number for which to enable or disable
 *		synchronous clock mode.
 * @param	Enable if set to 1, will enable synchronous clock mode.
 *		Otherwise, if set to 0, synchronous clock mode will be disabled.
 *
 * @return	None.
 *
 * @note	C-style signature:
 *		void XDp_TxCfgMsaEnSynchClkMode(XDpTxSs *InstancePtr,
 *							u8 Stream, u8 Enable)
 *
*******************************************************************************/
#define XDpTxSs_CfgMsaEnSynchClkMode(InstancePtr, Stream, Enable) \
	XDp_TxCfgMsaEnSynchClkMode((InstancePtr)->DpPtr, (Sream), (Enable))

/*****************************************************************************/
/**
 * This function macro enables MST-TX audio on a given stream on the main link.
 *
 * @param      InstancePtr is a pointer to the XDpTxSs core instance.
 * @param      Stream Id to be enabled audio
 *
 * @return     None.
 *
 * @note       C-style signature:
 *             void XDpTxSs_Mst_AudioEnable(XDpTxSs *InstancePtr, u8 StreamId)
 *
 *******************************************************************************/
#define XDpTxSs_Mst_AudioEnable(InstancePtr, StreamId) \
        XDp_Tx_Mst_AudioEn((InstancePtr)->DpPtr, StreamId)

/*****************************************************************************/
/**
 *
 * This function macro disables audio on a given stream on the main link.
 *
 * @param        InstancePtr is a pointer to the XDpTxSs core instance.
 *
 * @return       None.
 *
 * @note         C-style signature:
 *               void XDpTxSs_Mst_AudioDisable(XDpTxSs *InstancePtr)
 *
 ******************************************************************************/
#define XDpTxSs_Mst_AudioDisable(InstancePtr) \
        XDp_TxAudioDis((InstancePtr)->DpPtr)

/*****************************************************************************/
/**
 *
 * This function macro sends audio infoframe packets on the main link.
 *
 * @param        InstancePtr is a pointer to the XDpTxSs core instance.
 * @param		xilInfoFrame is a pointer to the InfoFrame buffer.
 *
 * @return       None.
 *
 * @note         C-style signature:
 *               void XDpTxSs_SendAudioInfoFrame(XDpTxSs *InstancePtr,
 *               			XDp_TxAudioInfoFrame *xilInfoFrame)
 *
 ******************************************************************************/
#define XDpTxSs_SendAudioInfoFrame(InstancePtr, xilInfoFrame) \
	XDp_TxSendAudioInfoFrame((InstancePtr)->DpPtr, xilInfoFrame)

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
