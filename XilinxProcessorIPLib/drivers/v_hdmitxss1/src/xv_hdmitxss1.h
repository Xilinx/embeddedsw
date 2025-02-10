/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitxss1.h
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
*   - AXI Stream Input/Output interface
*   - 1, 2 or 4 pixel-wide video interface
*   - 8/10/12/16 bits per component
*   - RGB & YCbCr color space
*   - Up to 4k2k 60Hz resolution at both Input and Output interface
*   - Interlaced input support (1080i 50Hz/60Hz)

* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  EB   22/05/18 Initial release.
* </pre>
*
******************************************************************************/
#ifndef HDMITXSS1_H /**< prevent circular inclusions by using protection macros*/
#define HDMITXSS1_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xstatus.h"
#include "xvidc.h"
#include "xv_hdmic.h"
#include "xv_hdmic_vsif.h"
#include "xvidc_edid.h"
#include "xvidc_edid_ext.h"
#include "xv_hdmitx1.h"
#include "xvtc.h"
#include "xv_hdmitxss1_frl.h"

#if !defined(XV_CONFIG_LOG_VHDMITXSS1_DISABLE) && \
                                             !defined(XV_CONFIG_LOG_DISABLE_ALL)
#define XV_HDMITXSS1_LOG_ENABLE
#endif

#ifdef SDT
#if XPAR_XHDCP1X_NUM_INSTANCES
#define XPAR_XHDCP_NUM_INSTANCES XPAR_XHDCP1X_NUM_INSTANCES
#endif
#endif

#if defined(XPAR_XHDCP_NUM_INSTANCES) || defined(XPAR_XHDCP22_TX_NUM_INSTANCES)
#define USE_HDCP_TX
#define XV_HDMITXSS1_HDCP_KEYSEL 0x00u
#define XV_HDMITXSS1_HDCP_MAX_QUEUE_SIZE 16
#endif

#ifdef XPAR_XHDCP_NUM_INSTANCES
#include "xtmrctr.h"
#include "xhdcp1x.h"
#endif

#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
#include "xhdcp22_tx.h"
#endif


#define XV_HDMITXSS1_DDC_EDID_LENGTH	256

/****************************** Type Definitions ******************************/
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/** @name Handler Types
* @{
*/
#ifdef XV_HDMITXSS1_LOG_ENABLE
typedef enum {
	XV_HDMITXSS1_LOG_EVT_NONE = 1,		  /**< Log event none. */
	XV_HDMITXSS1_LOG_EVT_HDMITX1_INIT,	  /**< Log event HDMITX1 Init. */
	XV_HDMITXSS1_LOG_EVT_VTC_INIT,	      /**< Log event VTC Init. */
	XV_HDMITXSS1_LOG_EVT_HDCPTIMER_INIT,	  /**< Log event HDCP Timer Init */
	XV_HDMITXSS1_LOG_EVT_HDCP14_INIT,	  /**< Log event HDCP 14 Init. */
	XV_HDMITXSS1_LOG_EVT_HDCP22_INIT,	  /**< Log event HDCP 22 Init. */
	XV_HDMITXSS1_LOG_EVT_REMAP_HWRESET_INIT,	/**< Log event Remap reset Init. */
	XV_HDMITXSS1_LOG_EVT_REMAP_INIT,		/**< Log event Remapper Init. */
	XV_HDMITXSS1_LOG_EVT_START,	/**< Log event HDMITXSS1 Start. */
	XV_HDMITXSS1_LOG_EVT_STOP,	/**< Log event HDMITXSS1 Stop. */
	XV_HDMITXSS1_LOG_EVT_RESET,	/**< Log event HDMITXSS1 Reset. */
	XV_HDMITXSS1_LOG_EVT_CONNECT, /**< Log event Cable connect. */
	XV_HDMITXSS1_LOG_EVT_TOGGLE, /**< Log event HPD toggle. */
	XV_HDMITXSS1_LOG_EVT_DISCONNECT,	/**< Log event Cable disconnect. */
	XV_HDMITXSS1_LOG_EVT_STREAMUP,	/**< Log event Stream Up. */
	XV_HDMITXSS1_LOG_EVT_STREAMDOWN,	/**< Log event Stream Down. */
	XV_HDMITXSS1_LOG_EVT_STREAMSTART, /**< Log event Stream Start. */
	XV_HDMITXSS1_LOG_EVT_SETAUDIOCHANNELS, /**< Log event Set Audio Channels. */
	XV_HDMITXSS1_LOG_EVT_AUDIOMUTE,		/**< Log event Audio Mute */
	XV_HDMITXSS1_LOG_EVT_AUDIOUNMUTE,	/**< Log event Audio Unmute. */
	XV_HDMITXSS1_LOG_EVT_AUDIOINVALIDSAMPRATE, /**< Log event Audio Invalid
							Audio Sampling Rate. */
	XV_HDMITXSS1_LOG_EVT_SETSTREAM,   /**< Log event HDMITXSS1 Setstream. */
	XV_HDMITXSS1_LOG_EVT_HDCP14_AUTHREQ,   /**< Log event HDCP 1.4 AuthReq. */
	XV_HDMITXSS1_LOG_EVT_HDCP22_AUTHREQ,   /**< Log event HDCP 2.2 AuthReq. */
	XV_HDMITXSS1_LOG_EVT_PIX_REPEAT_ERR,	/**< Log event Unsupported Pixel
						     Repetition. */
	XV_HDMITXSS1_LOG_EVT_VTC_RES_ERR,	/**< Log event Resolution Unsupported
						     by VTC. */
	XV_HDMITXSS1_LOG_EVT_BRDG_LOCKED,	/**< VID-OUT bridge locked. */
	XV_HDMITXSS1_LOG_EVT_BRDG_UNLOCKED,	/**< VID-OUT bridge unlocked. */
	XV_HDMITXSS1_LOG_EVT_FRL_START,	/**< Log event HDMITXSS1 FRL Start. */
	XV_HDMITXSS1_LOG_EVT_FRL_LT_PASS,	/**< Log event HDMITXSS1 FRL LT Pass. */
	XV_HDMITXSS1_LOG_EVT_FRL_CFG,		/**< Log event HDMITXSS1 FRL Config. */
	XV_HDMITXSS1_LOG_EVT_TMDS_START,	/**< Log event HDMITXSS1 TMDS Start. */
	XV_HDMITXSS1_LOG_EVT_FRL_LTS1,
	XV_HDMITXSS1_LOG_EVT_FRL_LTS2,
	XV_HDMITXSS1_LOG_EVT_FRL_LTS3,
	XV_HDMITXSS1_LOG_EVT_FRL_LTS4,
	XV_HDMITXSS1_LOG_EVT_FRL_LTSP,
	XV_HDMITXSS1_LOG_EVT_FRL_LTSL,
	XV_HDMITXSS1_LOG_EVT_DUMMY		/**< Dummy Event should be last */
} XV_HdmiTxSs1_LogEvent;

/**
 * This typedef contains the logging mechanism for debug.
 */
typedef struct {
	u16 DataBuffer[128];		/**< Log buffer with event data. */
	u64 TimeRecord[128];		/**< Log time for the event */
    u8 HeadIndex;               /**< Index of the head entry of the
                                     Event/DataBuffer. */
    u8 TailIndex;               /**< Index of the tail entry of the
                                     Event/DataBuffer. */
} XV_HdmiTxSs1_Log;
#endif

typedef enum {
	XV_HDMITXSS1_LEADING_TOLERANCE,
	XV_HDMITXSS1_LAGGING_TOLERANCE,
	XV_HDMITXSS1_UNKNOWN
} XV_HdmiTxSs1_HpdToleranceType;

/**
* These constants specify the HDCP protection schemes
*/
typedef enum
{
    XV_HDMITXSS1_HDCP_NONE,   /**< No content protection */
    XV_HDMITXSS1_HDCP_14,     /**< HDCP 1.4 */
    XV_HDMITXSS1_HDCP_22,     /**< HDCP 2.2 */
    XV_HDMITXSS1_HDCP_BOTH    /**< Both HDCP 1.4 and 2.2 */
} XV_HdmiTxSs1_HdcpProtocol;

#ifdef USE_HDCP_TX
/**
* These constants specify the HDCP key types
*/
typedef enum
{
    XV_HDMITXSS1_KEY_HDCP22_LC128,   /**< HDCP 2.2 LC128 */
    XV_HDMITXSS1_KEY_HDCP22_SRM,     /**< HDCP 2.2 SRM */
    XV_HDMITXSS1_KEY_HDCP14,         /**< HDCP 1.4 Key */
    XV_HDMITXSS1_KEY_HDCP14_SRM,     /**< HDCP 1.4 SRM */
    XV_HDMITXSS1_KEY_INVALID         /**< Invalid Key */
} XV_HdmiTxSs1_HdcpKeyType;

/**
* These constants specify HDCP repeater content stream management type
*/
typedef enum
{
    XV_HDMITXSS1_HDCP_STREAMTYPE_0, /**< HDCP Stream Type 0 */
    XV_HDMITXSS1_HDCP_STREAMTYPE_1  /**< HDCP Stream Type 1 */
} XV_HdmiTxSs1_HdcpContentStreamType;

typedef enum
{
    XV_HDMITXSS1_HDCP_NO_EVT,
    XV_HDMITXSS1_HDCP_STREAMUP_EVT,
    XV_HDMITXSS1_HDCP_STREAMDOWN_EVT,
    XV_HDMITXSS1_HDCP_CONNECT_EVT,
    XV_HDMITXSS1_HDCP_DISCONNECT_EVT,
    XV_HDMITXSS1_HDCP_AUTHENTICATE_EVT,
    XV_HDMITXSS1_HDCP_INVALID_EVT
} XV_HdmiTxSs1_HdcpEvent;

/**
* These constants are used to identify fields inside the topology structure
*/
typedef enum {
    XV_HDMITXSS1_HDCP_TOPOLOGY_DEPTH,
    XV_HDMITXSS1_HDCP_TOPOLOGY_DEVICECNT,
    XV_HDMITXSS1_HDCP_TOPOLOGY_MAXDEVSEXCEEDED,
    XV_HDMITXSS1_HDCP_TOPOLOGY_MAXCASCADEEXCEEDED,
    XV_HDMITXSS1_HDCP_TOPOLOGY_HDCP2LEGACYDEVICEDOWNSTREAM,
    XV_HDMITXSS1_HDCP_TOPOLOGY_HDCP1DEVICEDOWNSTREAM,
    XV_HDMITXSS1_HDCP_TOPOLOGY_INVALID
} XV_HdmiTxSs1_HdcpTopologyField;

typedef struct
{
    XV_HdmiTxSs1_HdcpEvent   Queue[XV_HDMITXSS1_HDCP_MAX_QUEUE_SIZE]; /**< Data */
    u8                      Tail;      /**< Tail pointer */
    u8                      Head;      /**< Head pointer */
} XV_HdmiTxSs1_HdcpEventQueue;
#endif

/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from peripheral.
*/
typedef enum {
    XV_HDMITXSS1_HANDLER_CONNECT = 1,                       /**< Handler for
                                                            connect event */
    XV_HDMITXSS1_HANDLER_TOGGLE,                            /**< Handler for
                                                            toggle event */
    XV_HDMITXSS1_HANDLER_BRDGLOCK,                          /**< Handler for
                                                            bridge locked
															event */
    XV_HDMITXSS1_HANDLER_BRDGUNLOCK,                        /**< Handler for
                                                            bridge unlocked
                                                            event */
    XV_HDMITXSS1_HANDLER_BRDGOVERFLOW,                      /**< Handler for
                                                            bridge overflow
                                                            event */
    XV_HDMITXSS1_HANDLER_BRDGUNDERFLOW,                     /**< Handler for
                                                            bridge underflow
                                                            event */
    XV_HDMITXSS1_HANDLER_VS,                                /**< Handler for
                                                            vsync event */
    XV_HDMITXSS1_HANDLER_STREAM_DOWN,                       /**< Handler for
                                                            stream down event */
    XV_HDMITXSS1_HANDLER_STREAM_UP,                         /**< Handler for
                                                            stream up event */
    XV_HDMITXSS1_HANDLER_ERROR,                             /**< Handler for
                                                            error event */
    XV_HDMITXSS1_HANDLER_FRL_CONFIG,                        /**< Handler for FRL
                                                            Config */
    XV_HDMITXSS1_HANDLER_FRL_FFE,                           /**< Handler for FRL
                                                            FFE */
    XV_HDMITXSS1_HANDLER_FRL_START,                         /**< Handler for FRL
                                                            Start */
    XV_HDMITXSS1_HANDLER_FRL_STOP,                          /**< Handler for FRL
                                                            Stop */
    XV_HDMITXSS1_HANDLER_TMDS_CONFIG,                       /**< Handler for TMDS
                                                            Config */
    XV_HDMITXSS1_HANDLER_HDCP_AUTHENTICATED,                /**< Handler for
                                                            HDCP authenticated
                                                            event */
    XV_HDMITXSS1_HANDLER_HDCP_DOWNSTREAM_TOPOLOGY_AVAILABLE,/**< Handler for
                                                            HDCP downstream
                                                            topology available
                                                            event */
    XV_HDMITXSS1_HANDLER_HDCP_UNAUTHENTICATED,              /**< Handler for
                                                            HDCP unauthenticated
                                                            event */
    XV_HDMITXSS1_HANDLER_DYNHDR_MWT,		/**< Handler for MTW Event */
	XV_HDMITXSS1_HANDLER_DSCDECODE_FAIL,		/**< Dsc Decode fail Event */
} XV_HdmiTxSs1_HandlerType;
/*@}*/

/**
 * Sub-Core Configuration Table
 */
typedef struct
{
  u16 IsPresent;  /**< Flag to indicate if sub-core is present in the design*/
#ifndef SDT
  u16 DeviceId;   /**< Device ID of the sub-core */
  UINTPTR AbsAddr; /**< Sub-core Absolute Base Address */
#else
  UINTPTR AbsAddr;
#endif
}XV_HdmiTxSs1_SubCore;

/**
 * Video Processing Subsystem configuration structure.
 * Each subsystem device should have a configuration structure associated
 * that defines the MAX supported sub-cores within subsystem
 */

typedef struct
{
#ifndef SDT
    u16 DeviceId;                     /**< DeviceId is the unique ID  of the
                                           device */
#else
   char *Name;
#endif
    UINTPTR BaseAddress;              /**< BaseAddress is the physical base
                                           address of the subsystem address
                                           range */
    UINTPTR HighAddress;              /**< HighAddress is the physical MAX
                                           address of the subsystem address
                                           range */
    XVidC_PixelsPerClock Ppc;         /**< Supported Pixel per Clock */
    u8 MaxBitsPerPixel;               /**< Maximum  Supported Color Depth */
	u8 LowResolutionSupp;
	u8 YUV420Supp;
	u32 MaxFrlRate;                   /** < Maximum FRL Rate Supporte */
	u32 DynHdr;			/**< Supports Dynamic HDR */
    u32 DSC; /**< DSC Supported */
    u32 AxiLiteClkFreq;               /**< AXI Lite Clock Frequency in Hz */
    u8 VideoInterface;	/**< 0 - AXI4S 1 - Native 2 - Native DE video interface */
    XV_HdmiTxSs1_SubCore HdcpTimer;    /**< Sub-core instance configuration */
    XV_HdmiTxSs1_SubCore Hdcp14;       /**< Sub-core instance configuration */
    XV_HdmiTxSs1_SubCore Hdcp22;       /**< Sub-core instance configuration */
    XV_HdmiTxSs1_SubCore HdmiTx1;       /**< Sub-core instance configuration */
    XV_HdmiTxSs1_SubCore Vtc;          /**< Sub-core instance configuration */
#ifdef SDT
	u16 IntrId[5];	/**< Interrupt ID */
	UINTPTR IntrParent;
	/**< Bit[0] Interrupt parent type Bit[64/32:1] Parent base address */
#endif
} XV_HdmiTxSs1_Config;

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
typedef void (*XV_HdmiTxSs1_Callback)(void *CallbackRef);

/**
* Callback type for interrupt.
*
* @param  CallbackRef is a callback reference passed in by the upper
*   layer when setting the callback functions, and passed back to
*   the upper layer when the callback is invoked.
*
* @return u8 value.
*
* @note   None.
*
*/
typedef u64 (*XV_HdmiTxSs1_LogCallback)(void *CallbackRef);


/**
 * Dynamic HDR configuration structure.
 * This contains
 * 1 - buffer address which contains the Dynamic HDR packet,
 * 2 - the packet type and length,
 * 3 - Graphics Overlay Flag value
 * 4 - FAPA Location
 */
typedef struct
{
	u64 Address; /**< Dynamic HDR packet buffer address */
	u16 PktType; /**< 16 bit Packet Type */
	u16 PktLength; /**< 16 bit Packet Length */
	u8 GOF;	/**< GOF Value 0 or 1 only */
	u8 FAPA; /**<FAPA Location 0 or 1 only */
} XV_HdmiTxSs1_DynHdr_Config;

/**
* The XVprocss driver instance data. The user is required to allocate a variable
* of this type for every XVprocss device in the system. A pointer to a variable
* of this type is then passed to the driver API functions.
*/
typedef struct
{
    XV_HdmiTxSs1_Config Config;  /**< Hardware configuration */
    u32 IsReady;         /**< Device and the driver instance are initialized */
    u8 AppMajVer;       /**< Major Version of application used by the driver */
    u8 AppMinVer;       /**< Minor Version of application used by the driver */

#ifdef XV_HDMITXSS1_LOG_ENABLE
    XV_HdmiTxSs1_Log Log;                /**< A log of events. */
#endif
#ifdef XPAR_XHDCP_NUM_INSTANCES
    XTmrCtr *HdcpTimerPtr;          /**< handle to sub-core driver instance */
    XHdcp1x *Hdcp14Ptr;             /**< handle to sub-core driver instance */
#endif
#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
    XHdcp22_Tx  *Hdcp22Ptr;         /**< handle to sub-core driver instance */
#endif
    XV_HdmiTx1 *HdmiTx1Ptr;           /**< handle to sub-core driver instance */
    XVtc *VtcPtr;                   /**< handle to sub-core driver instance */

    /* Callbacks */
    XV_HdmiTxSs1_Callback ConnectCallback; /**< Callback for connect event */
    void *ConnectRef;                     /**< To be passed to the connect
                                               callback */

    XV_HdmiTxSs1_Callback ToggleCallback; /**< Callback for toggle event */
    void *ToggleRef;                     /**< To be passed to the toggle
                                              callback */

    XV_HdmiTxSs1_Callback BrdgLockedCallback; /**< Callback for Bridge Locked
                                                  event interrupt */
    void *BrdgLockedRef;                  /**< To be passed to the Bridge
                                              Unlocked interrupt callback */

    XV_HdmiTxSs1_Callback BrdgUnlockedCallback; /**< Callback for Bridge UnLocked
                                                  event interrupt */
    void *BrdgUnlockedRef;                  /**< To be passed to the Bridge
                                              Unlocked interrupt callback */

    XV_HdmiTxSs1_Callback BrdgOverflowCallback; /**< Callback for Bridge
                                                 * Overflow event interrupt */
    void *BrdgOverflowRef;                  /**< To be passed to the Bridge
                                              Overflow interrupt callback */

    XV_HdmiTxSs1_Callback BrdgUnderflowCallback; /**< Callback for Bridge
                                                 * Underflow event interrupt */
    void *BrdgUnderflowRef;                  /**< To be passed to the Bridge
                                              Underflow interrupt callback */

    XV_HdmiTxSs1_Callback VsCallback; /**< Callback for Vsync event */
    void *VsRef;                   /**< To be passed to the Vsync callback */

    XV_HdmiTxSs1_Callback StreamDownCallback; /**< Callback for stream down */
    void *StreamDownRef; /**< To be passed to the stream down callback */

    XV_HdmiTxSs1_Callback StreamUpCallback; /**< Callback for stream up */
    void *StreamUpRef;  /**< To be passed to the stream up callback */

    XV_HdmiTxSs1_Callback ErrorCallback; /**< Callback for stream up */
    void *ErrorRef;  /**< To be passed to the stream up callback */

    XV_HdmiTxSs1_LogCallback LogWriteCallback; /**< Callback for log write */
    u32 *LogWriteRef;  /**< To be passed to the log write callback */

    XV_HdmiTxSs1_Callback FrlConfigCallback; /**< Callback for stream up */
    void *FrlConfigRef;  /**< To be passed to the stream up callback */

    XV_HdmiTxSs1_Callback FrlFfeCallback; /**< Callback for stream up */
    void *FrlFfeRef;  /**< To be passed to the stream up callback */

    XV_HdmiTxSs1_Callback FrlStartCallback; /**< Callback for stream up */
    void *FrlStartRef;  /**< To be passed to the stream up callback */

    XV_HdmiTxSs1_Callback FrlStopCallback; /**< Callback for stream up */
    void *FrlStopRef;  /**< To be passed to the stream up callback */

    XV_HdmiTxSs1_Callback TmdsConfigCallback; /**< Callback for stream up */
    void *TmdsConfigRef;  /**< To be passed to the stream up callback */

    XV_HdmiTxSs1_Callback CedUpdateCallback; /**< Callback for FRL LTS:P */
    void *CedUpdatePRef;  /**< To be passed to FRL LTS:P callback */

    XV_HdmiTxSs1_Callback DynHdrMtwCallback;	/**< Callback for Dynamic HDR
						 *  MTW Start */
    void *DynHdrMtwRef;			/**< To be passed to the
					 *  Dynamic HDR callback */
    XV_HdmiTxSs1_Callback DscDecodeFailCallback; /**< Callback for DSC decode fail */
    void *DscDecodeFailRef;  /**< To be passed to DSC decode fail callback */

    /**< Scratch pad */
    u8 SamplingRate;              /**< HDMI TX Sampling rate */
    u8 IsStreamConnected;         /**< HDMI TX Stream Connected */
    u8 IsStreamUp;                /**< HDMI TX Stream Up */
    u8 IsStreamToggled;           /**< HDMI TX Stream Toggled */
    u8 AudioEnabled;              /**< HDMI TX Audio Enabled */
    u8 AudioMute;                 /**< HDMI TX Audio Mute */
    u8 AudioChannels;             /**< Number of Audio Channels */

    u8 EnableHDCPLogging;         /**< HDCP Logging Enabling */
    u8 EnableHDMILogging;         /**< HDMI Logging Enabling */

	XHdmiC_AVI_InfoFrame AVIInfoframe;		/**< AVI InfoFrame */
	XHdmiC_AudioInfoFrame AudioInfoframe;	/**< Audio InfoFrame */
	XHdmiC_VSIF VSIF;						/**< Vendor Specific InfoFrame */
	XHdmiC_DRMInfoFrame DrmInfoframe;	/**< DRM Infoframe */

    u8 VrrEnabled;	/* VRR set by user */
    u8 FvaFactor;	/* FVA factor set by user */
    u8 CnmvrrEnabled;	/* Cnmvrr enabled by user */
    u8 VrrMode;

    XV_HdmiTxSs1_HdcpProtocol    HdcpProtocol;    /**< HDCP protocol selected */
#ifdef USE_HDCP_TX
    /**< HDCP specific */
    XV_HdmiTxSs1_HdcpProtocol    HdcpCapability;  /**< HDCP protocol desired */
    u32                         HdcpIsReady;     /**< HDCP ready flag */
    XV_HdmiTxSs1_HdcpEventQueue  HdcpEventQueue;  /**< HDCP event queue */
#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
    u8                          *Hdcp22Lc128Ptr; /**< Pointer to HDCP 2.2
                                                      LC128 */
    u8                          *Hdcp22SrmPtr;   /**< Pointer to HDCP 2.2 SRM */
#endif
#ifdef XPAR_XHDCP_NUM_INSTANCES
    u8                          *Hdcp14KeyPtr;   /**< Pointer to HDCP 1.4 key */
    u8                          *Hdcp14SrmPtr;   /**< Pointer to HDCP 1.4 SRM */
#endif
#endif
} XV_HdmiTxSs1;

/************************** Macros Definitions *******************************/
#ifdef USE_HDCP_TX
#define XV_HdmiTxSs1_HdcpIsReady(InstancePtr) \
  (InstancePtr)->HdcpIsReady
#endif
/************************** Function Prototypes ******************************/
#ifndef SDT
XV_HdmiTxSs1_Config *XV_HdmiTxSs1_LookupConfig(u32 DeviceId);
#else
XV_HdmiTxSs1_Config *XV_HdmiTxSs1_LookupConfig(UINTPTR BaseAddress);
u32 XV_HdmiTxSs1_GetDrvIndex(XV_HdmiTxSs1 *InstancePtr, UINTPTR BaseAddress);
#endif
void XV_HdmiTxSS1_SetHdmiFrlMode(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSS1_SetHdmiTmdsMode(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSS1_SetDviMode(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSS1_HdmiTx1IntrHandler(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_CfgInitialize(XV_HdmiTxSs1 *InstancePtr,
	XV_HdmiTxSs1_Config *CfgPtr,
	UINTPTR EffectiveAddr);
void XV_HdmiTxSS1_SetAppVersion(XV_HdmiTxSs1 *InstancePtr, u8 maj, u8 min);
void XV_HdmiTxSs1_Start(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_Stop(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_Reset(XV_HdmiTxSs1 *InstancePtr);

void XV_HdmiTxSs1_TXCore_VRST(XV_HdmiTxSs1 *InstancePtr, u8 Reset);
void XV_HdmiTxSs1_TXCore_LRST(XV_HdmiTxSs1 *InstancePtr, u8 Reset);
void XV_HdmiTxSs1_VRST(XV_HdmiTxSs1 *InstancePtr, u8 Reset);
void XV_HdmiTxSs1_SYSRST(XV_HdmiTxSs1 *InstancePtr, u8 Reset);
void XV_HdmiTxSs1_SetGcpAvmuteBit(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_ClearGcpAvmuteBit(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_SetGcpClearAvmuteBit(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_ClearGcpClearAvmuteBit(XV_HdmiTxSs1 *InstancePtr);

int XV_HdmiTxSs1_SetCallback(XV_HdmiTxSs1 *InstancePtr,
	XV_HdmiTxSs1_HandlerType HandlerType,
	void *CallbackFuncPtr,
	void *CallbackRef);
int XV_HdmiTxSs1_SetLogCallback(XV_HdmiTxSs1 *InstancePtr,
	u64 *CallbackFunc,
	void *CallbackRef);
int XV_HdmiTxSs1_SendCvtemAuxPackets(XV_HdmiTxSs1 *InstancePtr, XHdmiC_Aux *DscAuxFifo);
int XV_HdmiTxSs1_ReadEdid(XV_HdmiTxSs1 *InstancePtr, u8 *BufferPtr, u32 BufferSize);
int XV_HdmiTxSs1_ReadEdid_extension(XV_HdmiTxSs1 *InstancePtr, XV_VidC_EdidCntrlParam *EdidCtrlParam);
int XV_HdmiTxSs1_ReadEdidSegment(XV_HdmiTxSs1 *InstancePtr, u8 *Buffer, u8 segment);
void XV_HdmiTxSs1_ShowEdid(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_ShowEdid_extension(XV_HdmiTxSs1 *InstancePtr, XV_VidC_EdidCntrlParam *EdidCtrlParam);
void XV_HdmiTxSs1_SetScrambler(XV_HdmiTxSs1 *InstancePtr, u8 Enable);
void XV_HdmiTxSs1_StreamStart(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_SendAuxInfoframe(XV_HdmiTxSs1 *InstancePtr, void *AuxPtr);
u32 XV_HdmiTxSs1_SendGenericAuxInfoframe(XV_HdmiTxSs1 *InstancePtr, void *AuxPtr);
void XV_HdmiTxSs1_SetAudioChannels(XV_HdmiTxSs1 *InstancePtr, u8 AudioChannels);
void XV_HdmiTxSs1_AudioMute(XV_HdmiTxSs1 *InstancePtr, u8 Enable);
void XV_HdmiTxSs1_SetAudioFormat(XV_HdmiTxSs1 *InstancePtr,
    XV_HdmiTx1_AudioFormatType format);
void XV_HdmiTxSs1_SetIntACR(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_SetExtACR(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_SetSampleFrequency(XV_HdmiTxSs1 *InstancePtr,
		XHdmiC_SamplingFrequencyVal AudSampleFreqVal);
XV_HdmiTx1_AudioFormatType XV_HdmiTxSs1_GetAudioFormat(XV_HdmiTxSs1 *InstancePtr);
XHdmiC_Aux *XV_HdmiTxSs1_GetAuxiliary(XV_HdmiTxSs1 *InstancePtr);
XHdmiC_AVI_InfoFrame *XV_HdmiTxSs1_GetAviInfoframe(XV_HdmiTxSs1 *InstancePtr);
XHdmiC_AudioInfoFrame *XV_HdmiTxSs1_GetAudioInfoframe(XV_HdmiTxSs1 *InstancePtr);
XHdmiC_VSIF *XV_HdmiTxSs1_GetVSIF(XV_HdmiTxSs1 *InstancePtr);
XHdmiC_DRMInfoFrame *XV_HdmiTxSs1_GetDrmInfoframe(XV_HdmiTxSs1 *InstancePtr);
u32 XV_HdmiTxSs1_SetStream(XV_HdmiTxSs1 *InstancePtr,
		XVidC_VideoTiming VideoTiming,
		XVidC_FrameRate FrameRate,
		XVidC_ColorFormat ColorFormat,
		XVidC_ColorDepth Bpc,
		u8 IsDSCompressed,
		XVidC_3DInfo *Info3D,
		u64 *TmdsClk);
XVidC_VideoStream *XV_HdmiTxSs1_GetVideoStream(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_SetVideoStream(XV_HdmiTxSs1 *InstancePtr,
				XVidC_VideoStream VidStream);
void XV_HdmiTxSs1_SetSamplingRate(XV_HdmiTxSs1 *InstancePtr, u8 SamplingRate);
void XV_HdmiTxSs1_SetVideoIDCode(XV_HdmiTxSs1 *InstancePtr, u8 Vic);
u8 XV_HdmiTxSs1_GetVideoStreamType(XV_HdmiTxSs1 *InstancePtr);
u8 XV_HdmiTxSs1_GetTransportMode(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_SetVideoStreamScramblingFlag(XV_HdmiTxSs1 *InstancePtr,
						u8 IsScrambled);
void XV_HdmiTxSs1_SetVideoStreamScramblingOverrideFlag(XV_HdmiTxSs1 *InstancePtr,
							u8 OverrideScramble);
void XV_HdmiTxSs1_SetTmdsClockRatio(XV_HdmiTxSs1 *InstancePtr, u8 Ratio);
u32 XV_HdmiTxSs1_GetTmdsClockFreqHz(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_DetectHdmi20(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_RefClockChangeInit(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_ReportInfo(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_DebugInfo(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_RegisterDebug(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_IsStreamUp(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_IsStreamConnected(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_IsStreamToggled(XV_HdmiTxSs1 *InstancePtr);
u8 XV_HdmiTxSs1_IsSinkHdcp14Capable(XV_HdmiTxSs1 *InstancePtr);
u8 XV_HdmiTxSs1_IsSinkHdcp22Capable(XV_HdmiTxSs1 *InstancePtr);

void XV_HdmiTxSs1_SetDefaultPpc(XV_HdmiTxSs1 *InstancePtr, u8 Id);
void XV_HdmiTxSs1_SetPpc(XV_HdmiTxSs1 *InstancePtr, u8 Id, u8 Ppc);
XVidC_PixelsPerClock XV_HdmiTxSS1_GetCorePpc(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_VrrControl(XV_HdmiTxSs1 *InstancePtr, u8 Enable);
void XV_HdmiTxSs1_FSyncControl(XV_HdmiTxSs1 *InstancePtr, u8 Enable);

void XV_HdmiTxSS1_SetVrrMode(XV_HdmiTxSs1 *InstancePtr,
			u8 mode, u8 VrrEn, u8 FvaFactor, u8 CnmvrrEn);
void XV_HdmiTxSS1_SetVrrVfpStretch(XV_HdmiTxSs1 *InstancePtr,
					u16 StretchValue);
void XV_HdmiTxSS1_DisableVrr(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_SetVrrIf(XV_HdmiTxSs1 *InstancePtr,
			XV_HdmiC_VrrInfoFrame *VrrIF);
void XV_HdmiTxSs1_SetCustomVrrIf(XV_HdmiTxSs1 *InstancePtr,
				 XV_HdmiC_VrrInfoFrame *VrrIF, u16 Sync,
				 u16 DataSetLen);

/* Dynamic HDR related APIs */
void XV_HdmiTxSs1_DynHdr_Control(XV_HdmiTxSs1 *InstancePtr, u8 Flag);
void XV_HdmiTxSs1_DynHdr_GOF_Control(XV_HdmiTxSs1 *InstancePtr, u8 Flag);
void XV_HdmiTxSs1_DynHdr_Cfg(XV_HdmiTxSs1 *InstancePtr,
			     XV_HdmiTxSs1_DynHdr_Config *Cfg);
void XV_HdmiTxSs1_DynHdr_DM_Control(XV_HdmiTxSs1 *InstancePtr, u8 Flag);
u32 XV_HdmiTxSs1_DynHdr_GetErr(XV_HdmiTxSs1 *InstancePtr);

#ifdef XV_HDMITXSS1_LOG_ENABLE
void XV_HdmiTxSs1_LogReset(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_LogWrite(XV_HdmiTxSs1 *InstancePtr, XV_HdmiTxSs1_LogEvent Evt, u8 Data);
u16 XV_HdmiTxSs1_LogRead(XV_HdmiTxSs1 *InstancePtr);
#endif
void XV_HdmiTxSs1_LogDisplay(XV_HdmiTxSs1 *InstancePtr);

void XV_HdmiTxSs1_ReportCoreInfo(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_ReportTiming(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_ReportDRMInfo(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_ReportAudio(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_ReportSubcoreVersion(XV_HdmiTxSs1 *InstancePtr);


#ifdef USE_HDCP_TX
void XV_HdmiTxSs1_HdcpSetKey(XV_HdmiTxSs1 *InstancePtr, XV_HdmiTxSs1_HdcpKeyType KeyType, u8 *KeyPtr);
int XV_HdmiTxSs1_HdcpPoll(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpSetProtocol(XV_HdmiTxSs1 *InstancePtr, XV_HdmiTxSs1_HdcpProtocol Protocol);
int XV_HdmiTxSs1_HdcpSetCapability(XV_HdmiTxSs1 *InstancePtr, XV_HdmiTxSs1_HdcpProtocol Protocol);
XV_HdmiTxSs1_HdcpProtocol XV_HdmiTxSs1_HdcpGetProtocol(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpClearEvents(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpPushEvent(XV_HdmiTxSs1 *InstancePtr, XV_HdmiTxSs1_HdcpEvent Event);
int XV_HdmiTxSs1_HdcpEnable(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpDisable(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpAuthRequest(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpEnableEncryption(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpDisableEncryption(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpEnableBlank(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpDisableBlank(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpIsEnabled(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpIsAuthenticated(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpIsEncrypted(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpIsInProgress(XV_HdmiTxSs1 *InstancePtr);

void XV_HdmiTxSs1_HdcpInfo(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_HdcpSetInfoDetail(XV_HdmiTxSs1 *InstancePtr, u8 Verbose);

void *XV_HdmiTxSs1_HdcpGetTopology(XV_HdmiTxSs1 *InstancePtr);
u8 *XV_HdmiTxSs1_HdcpGetTopologyReceiverIdList(XV_HdmiTxSs1 *InstancePtr);
u32 XV_HdmiTxSs1_HdcpGetTopologyField(XV_HdmiTxSs1 *InstancePtr, XV_HdmiTxSs1_HdcpTopologyField Field);

void XV_HdmiTxSs1_HdcpSetContentStreamType(XV_HdmiTxSs1 *InstancePtr,
       XV_HdmiTxSs1_HdcpContentStreamType StreamType);
int XV_HdmiTxSs1_HdcpIsRepeater(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpSetRepeater(XV_HdmiTxSs1 *InstancePtr, u8 Set);
int XV_HdmiTxSs1_HdcpIsInComputations(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_HdcpIsInWaitforready(XV_HdmiTxSs1 *InstancePtr);

#endif

#ifdef XPAR_XHDCP_NUM_INSTANCES
int XV_HdmiTxSs1_HdcpTimerStart(void *InstancePtr, u16 TimeoutInMs);
int XV_HdmiTxSs1_HdcpTimerStop(void *InstancePtr);
int XV_HdmiTxSs1_HdcpTimerBusyDelay(void *InstancePtr, u16 DelayInMs);

void XV_HdmiTxSS1_HdcpIntrHandler(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSS1_HdcpTimerIntrHandler(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_HdcpTimerCallback(void *CallBackRef, u8 TimerChannel);
#endif

#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
void XV_HdmiTxSS1_Hdcp22TimerIntrHandler(XV_HdmiTxSs1 *InstancePtr);
#endif

void XV_HdmiTxSS1_MaskEnable(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSS1_MaskDisable(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSS1_MaskNoise(XV_HdmiTxSs1 *InstancePtr, u8 Enable);
void XV_HdmiTxSS1_MaskSetRed(XV_HdmiTxSs1 *InstancePtr, u16 Value);
void XV_HdmiTxSS1_MaskSetGreen(XV_HdmiTxSs1 *InstancePtr, u16 Value);
void XV_HdmiTxSS1_MaskSetBlue(XV_HdmiTxSs1 *InstancePtr, u16 Value);
void XV_HdmiTxSS1_SetBackgroundColor(XV_HdmiTxSs1 *InstancePtr,
					XVMaskColorId  ColorId);
u8 XV_HdmiTxSS1_IsMasked(XV_HdmiTxSs1 *InstancePtr);

void XV_HdmiTxSs1_SetFfeLevels(XV_HdmiTxSs1 *InstancePtr, u8 FfeLevel);
u8 XV_HdmiTxSs1_GetTxFfe(XV_HdmiTxSs1 *InstancePtr, u8 Lane);
u8 XV_HdmiTxSs1_GetFrlLineRate(XV_HdmiTxSs1 *InstancePtr);
u8 XV_HdmiTxSs1_GetFrlRate(XV_HdmiTxSs1 *InstancePtr);
u8 XV_HdmiTxSs1_GetFrlLanes(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_StartTmdsMode(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_StartFrlTraining(XV_HdmiTxSs1 *InstancePtr,
				XHdmiC_MaxFrlRate FrlRate);
void XV_HdmiTxSs1_SetFrlMaxFrlRate(XV_HdmiTxSs1 *InstancePtr,
				XHdmiC_MaxFrlRate MaxFrlRate);
int XV_HdmiTxSs1_FrlStreamStart(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_FrlStreamStop(XV_HdmiTxSs1 *InstancePtr);
int XV_HdmiTxSs1_TmdsStart(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_SetFrlWrongLtp(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_ClearFrlWrongLtp(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_SetFrlLtp(XV_HdmiTxSs1 *InstancePtr, u8 Lane,
				XV_HdmiTx1_FrlLtpType Ltp);
void XV_HdmiTxSS1_StopFRLStream(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSS1_StartFRLStream(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_SetFrlExtVidCke(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSs1_SetFrlIntVidCke(XV_HdmiTxSs1 *InstancePtr);
u8 *XV_HdmiTxSs1_GetScdcEdRegisters(XV_HdmiTxSs1 *InstancePtr);
void XV_HdmiTxSS1_SetHpdTolerance(XV_HdmiTxSs1 *InstancePtr,
				 XV_HdmiTxSs1_HpdToleranceType Type,
				 u16 ToleranceVal);
#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
