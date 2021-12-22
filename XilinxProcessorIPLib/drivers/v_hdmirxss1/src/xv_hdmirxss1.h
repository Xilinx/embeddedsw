/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirxss1.h
*
* This is main header file of the Xilinx HDMI RX Subsystem driver
*
* <b>HDMI RX Subsystem Overview</b>
*
* Video Subsystem is a collection of IP cores bounded together by software
* to provide an abstract view of the processing pipe. It hides all the
* complexities of programming the underlying cores from end user.
*
* <b>Subsystem Driver Features</b>
*
* Video Subsystem supports following features
*   - AXI Stream Input/Output interface
*   - 1, 2 or 4 pixel-wide video interface
*   - 8/10/12/16 bits per component
*   - RGB & YCbCr color space
*   - Up to 4k2k 60Hz resolution at both Input and Output interface
*   - Interlaced input support (1080i 50Hz/60Hz)
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  EB   22/05/18 Initial release.
* </pre>
*
******************************************************************************/

#ifndef HDMIRXSS1_H /**< prevent circular inclusions by using protection macros*/
#define HDMIRXSS1_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xstatus.h"
#include "xv_hdmirx1.h"
#include "xv_hdmic_vsif.h"
#include "xv_hdmirxss1_frl.h"
#if !defined(XV_CONFIG_LOG_VHDMIRXSS1_DISABLE) && \
                                             !defined(XV_CONFIG_LOG_DISABLE_ALL)
#define XV_HDMIRXSS1_LOG_ENABLE
#endif

#if defined(XPAR_XHDCP_NUM_INSTANCES) || defined(XPAR_XHDCP22_RX_NUM_INSTANCES)
#define USE_HDCP_RX
#define XV_HDMIRXSS1_HDCP_KEYSEL 0x00u
#define XV_HDMIRXSS1_HDCP_MAX_QUEUE_SIZE 16
#endif

#ifdef XPAR_XHDCP_NUM_INSTANCES
#include "xtmrctr.h"
#include "xhdcp1x.h"
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
#include "xhdcp22_rx.h"
#endif
/****************************** Type Definitions ******************************/
/** @name Handler Types
* @{
*/

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#ifdef XV_HDMIRXSS1_LOG_ENABLE
typedef enum {
	XV_HDMIRXSS1_LOG_EVT_NONE = 1,		/**< Log event none. */
	XV_HDMIRXSS1_LOG_EVT_HDMIRX1_INIT,	/**< Log event HDMIRX Init. */
	XV_HDMIRXSS1_LOG_EVT_VTC_INIT,	    /**< Log event VTC Init. */
	XV_HDMIRXSS1_LOG_EVT_HDCPTIMER_INIT,	/**< Log event HDCP Timer Init */
	XV_HDMIRXSS1_LOG_EVT_HDCP14_INIT,	/**< Log event HDCP 14 Init. */
	XV_HDMIRXSS1_LOG_EVT_HDCP22_INIT,	/**< Log event HDCP 22 Init. */
	XV_HDMIRXSS1_LOG_EVT_START,	        /**< Log event HDMIRXSS1 Start. */
	XV_HDMIRXSS1_LOG_EVT_STOP,	        /**< Log event HDMIRXSS1 Stop. */
	XV_HDMIRXSS1_LOG_EVT_RESET,	        /**< Log event HDMIRXSS1 Reset. */
	XV_HDMIRXSS1_LOG_EVT_CONNECT,        /**< Log event Cable connect. */
	XV_HDMIRXSS1_LOG_EVT_DISCONNECT,	    /**< Log event Cable disconnect. */
	XV_HDMIRXSS1_LOG_EVT_LINKSTATUS,     /**< Log event Link Status Error. */
	XV_HDMIRXSS1_LOG_EVT_STREAMUP,	    /**< Log event Stream Up. */
	XV_HDMIRXSS1_LOG_EVT_STREAMDOWN,	    /**< Log event Stream Down. */
	XV_HDMIRXSS1_LOG_EVT_STREAMINIT,	    /**< Log event Stream Init. */
	XV_HDMIRXSS1_LOG_EVT_SETSTREAM,      /**< Log event HDMIRXSS1 Setstream. */
	XV_HDMIRXSS1_LOG_EVT_SETSTREAM_ERR,  /**< Log event HDMIRXSS1 Setstream Err. */
	XV_HDMIRXSS1_LOG_EVT_REFCLOCKCHANGE, /**< Log event TMDS Ref clock change. */
	XV_HDMIRXSS1_LOG_EVT_HDCP14,             /**< Log event Enable HDCP 1.4. */
	XV_HDMIRXSS1_LOG_EVT_HDCP22,             /**< Log event Enable HDCP 2.2. */
	XV_HDMIRXSS1_LOG_EVT_HDMIMODE,           /**< Log event HDMI Mode change. */
	XV_HDMIRXSS1_LOG_EVT_DVIMODE,            /**< Log event HDMI Mode change. */
	XV_HDMIRXSS1_LOG_EVT_SYNCLOSS,           /**< Log event Sync Loss detected. */
	XV_HDMIRXSS1_LOG_EVT_PIX_REPEAT_ERR,     /**< Log event Unsupported Pixel Repetition. */
	XV_HDMIRXSS1_LOG_EVT_SYNCEST,            /**< Log event Sync Loss detected. */
	XV_HDMIRXSS1_LOG_EVT_VICERROR,           /**< Log event vic error detected. */
	XV_HDMIRXSS1_LOG_EVT_LNKRDYERROR,        /**< Log event link ready error detected. */
	XV_HDMIRXSS1_LOG_EVT_VIDRDYERROR,        /**< Log event video ready error detected. */
	XV_HDMIRXSS1_LOG_EVT_SKEWLOCKERROR,      /**< Log event skew lock error detected. */
	XV_HDMIRXSS1_LOG_EVT_FRL_LTS1,
	XV_HDMIRXSS1_LOG_EVT_FRL_LTS2,
	XV_HDMIRXSS1_LOG_EVT_FRL_LTS3,
	XV_HDMIRXSS1_LOG_EVT_FRL_LTS4,
	XV_HDMIRXSS1_LOG_EVT_FRL_LTSP,
	XV_HDMIRXSS1_LOG_EVT_FRL_LTSL,
	XV_HDMIRXSS1_LOG_EVT_FRL_START,
	XV_HDMIRXSS1_LOG_EVT_VRR_RDY,
	XV_HDMIRXSS1_LOG_EVT_DYN_HDR,
	XV_HDMIRXSS1_LOG_EVT_DSC_STRM_EVT,
	XV_HDMIRXSS1_LOG_EVT_DSC_PKT_ERR,
	XV_HDMIRXSS1_LOG_EVT_DSC_DDC_STS_UPDT,
	XV_HDMIRXSS1_LOG_EVT_DUMMY               /**< Dummy Event should be last */
} XV_HdmiRxSs1_LogEvent;

/**
 * This typedef contains the logging mechanism for debug.
 */
typedef struct {
	u16 DataBuffer[128];		/**< Log buffer with event data. */
	u64 TimeRecord[128];		/**< Log time for the event */
	u8 HeadIndex;			/**< Index of the head entry of the
						Event/DataBuffer. */
	u8 TailIndex;			/**< Index of the tail entry of the
						Event/DataBuffer. */
} XV_HdmiRxSs1_Log;
#endif

#ifdef USE_HDCP_RX
/**
* These constants are used to identify fields inside the topology structure
*/
typedef enum {
  XV_HDMIRXSS1_HDCP_TOPOLOGY_DEPTH,
  XV_HDMIRXSS1_HDCP_TOPOLOGY_DEVICECNT,
  XV_HDMIRXSS1_HDCP_TOPOLOGY_MAXDEVSEXCEEDED,
  XV_HDMIRXSS1_HDCP_TOPOLOGY_MAXCASCADEEXCEEDED,
  XV_HDMIRXSS1_HDCP_TOPOLOGY_HDCP2LEGACYDEVICEDOWNSTREAM,
  XV_HDMIRXSS1_HDCP_TOPOLOGY_HDCP1DEVICEDOWNSTREAM,
  XV_HDMIRXSS1_HDCP_TOPOLOGY_INVALID
} XV_HdmiRxSs1_HdcpTopologyField;

/**
* These constants specify the HDCP Events
*/
typedef enum
{
  XV_HDMIRXSS1_HDCP_NO_EVT,
  XV_HDMIRXSS1_HDCP_STREAMUP_EVT,
  XV_HDMIRXSS1_HDCP_STREAMDOWN_EVT,
  XV_HDMIRXSS1_HDCP_CONNECT_EVT,
  XV_HDMIRXSS1_HDCP_DISCONNECT_EVT,
  XV_HDMIRXSS1_HDCP_DVI_MODE_EVT,
  XV_HDMIRXSS1_HDCP_HDMI_MODE_EVT,
  XV_HDMIRXSS1_HDCP_SYNC_LOSS_EVT,
  XV_HDMIRXSS1_HDCP_SYNC_EST_EVT,
  XV_HDMIRXSS1_HDCP_INVALID_EVT
} XV_HdmiRxSs1_HdcpEvent;

/**
* These constants specify the HDCP key types
*/
typedef enum
{
  XV_HDMIRXSS1_KEY_HDCP22_LC128,     /**< HDCP 2.2 LC128 */
  XV_HDMIRXSS1_KEY_HDCP22_PRIVATE,   /**< HDCP 2.2 Private */
  XV_HDMIRXSS1_KEY_HDCP14,           /**< HDCP 1.4 Key */
} XV_HdmiRxSs1_HdcpKeyType;

/**
* These constants specify HDCP repeater content stream management type
*/
typedef enum
{
  XV_HDMIRXSS1_HDCP_STREAMTYPE_0, /**< HDCP Stream Type 0 */
  XV_HDMIRXSS1_HDCP_STREAMTYPE_1  /**< HDCP Stream Type 1 */
} XV_HdmiRxSs1_HdcpContentStreamType;

typedef struct
{
  XV_HdmiRxSs1_HdcpEvent Queue[XV_HDMIRXSS1_HDCP_MAX_QUEUE_SIZE]; /**< Data */
  u8                    Tail;      /**< Tail pointer */
  u8                    Head;      /**< Head pointer */
} XV_HdmiRxSs1_HdcpEventQueue;
#endif

/**
* These constants specify the HDCP protection schemes
*/
typedef enum
{
  XV_HDMIRXSS1_HDCP_NONE,       /**< No content protection */
  XV_HDMIRXSS1_HDCP_14,         /**< HDCP 1.4 */
  XV_HDMIRXSS1_HDCP_22,         /**< HDCP 2.2 */
  XV_HDMIRXSS1_HDCP_BOTH        /**< Both HDCP 1.4 and 2.2 */
} XV_HdmiRxSs1_HdcpProtocol;

/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from peripheral.
*/
typedef enum {
  XV_HDMIRXSS1_HANDLER_CONNECT = 1,                  /**< Handler for connect
                                                         event */
  XV_HDMIRXSS1_HANDLER_BRDGOVERFLOW,                 /**< Handler for
                                                         bridge fifo overflow
                                                         event */
  XV_HDMIRXSS1_HANDLER_AUX,                          /**< Handler for AUX
                                                         peripheral event */
  XV_HDMIRXSS1_HANDLER_AUD,                          /**< Handler for AUD
                                                         peripheral event */
  XV_HDMIRXSS1_HANDLER_LNKSTA,                       /**< Handler for LNKSTA
                                                         peripheral event */
  XV_HDMIRXSS1_HANDLER_DDC,                          /**< Handler for DDC
                                                         peripheral event */
  XV_HDMIRXSS1_HANDLER_STREAM_DOWN,                  /**< Handler for stream
                                                         down event */
  XV_HDMIRXSS1_HANDLER_STREAM_INIT,                  /**< Handler for stream
                                                         init event */
  XV_HDMIRXSS1_HANDLER_STREAM_UP,                    /**< Handler for stream up
                                                         event */
  XV_HDMIRXSS1_HANDLER_PHY_RESET,                   /**< Handler for Phy
                                                         Reset */
  XV_HDMIRXSS1_HANDLER_FRL_CONFIG,                   /**< Handler for FRL
                                                         Config */
  XV_HDMIRXSS1_HANDLER_FRL_START,                    /**< Handler for FRL
                                                         Start */
  XV_HDMIRXSS1_HANDLER_TMDS_CONFIG,                  /**< Handler for TMDS
                                                         Config */
  XV_HDMIRXSS1_HANDLER_HDCP,                         /**< Handler for HDCP 1.4
                                                         event */
  XV_HDMIRXSS1_HANDLER_HDCP_AUTHENTICATED,           /**< Handler for HDCP
                                                         authenticated event */
  XV_HDMIRXSS1_HANDLER_HDCP_UNAUTHENTICATED,         /**< Handler for HDCP
                                                         unauthenticated event*/
  XV_HDMIRXSS1_HANDLER_HDCP_AUTHENTICATION_REQUEST,  /**< Handler for HDCP
                                                         authentication request
                                                         event */
  XV_HDMIRXSS1_HANDLER_HDCP_STREAM_MANAGE_REQUEST,   /**< Handler for HDCP stream
                                                         manage request event */
  XV_HDMIRXSS1_HANDLER_HDCP_TOPOLOGY_UPDATE,         /**< Handler for HDCP
                                                         topology update event*/
  XV_HDMIRXSS1_HANDLER_HDCP_ENCRYPTION_UPDATE,       /**< Handler for HDCP
                                                         encryption status
                                                         update event */
  XV_HDMIRXSS1_HANDLER_TMDS_CLK_RATIO,               /**< Handler type for
                                                         TMDS clock ratio
                                                         change */
  XV_HDMIRXSS1_HANDLER_VIC_ERROR,                    /**< Handler type for
                                                         VIC error change */

  XV_HDMIRXSS1_HANDLER_VFP_CH,                       /**< Handler type for VFP
						       change */
  XV_HDMIRXSS1_HANDLER_VRR_RDY,                      /**< Handler type for VRR
						       ready */
  XV_HDMIRXSS1_HANDLER_DYN_HDR,                      /**< Handler type for
						       Dynamic HDR*/
  XV_HDMIRXSS1_HANDLER_DSC_STRM_CH,  /**< Handler type for DSC stream change event */
  XV_HDMIRXSS1_HANDLER_DSC_PKT_ERR,  /**< Handler type for DSC PPS Packet error event */
  XV_HDMIRXSS1_HANDLER_DSC_STS_UPDT,  /**< Handler type for SCDC Reg 0x10
					   bit 0 Status_Update bit set by
					   HDMI Source */
} XV_HdmiRxSs1_HandlerType;
/*@}*/

/**
 * Sub-Core Configuration Table
 */
typedef struct
{
  u16 IsPresent;  /**< Flag to indicate if sub-core is present in the design*/
  u16 DeviceId;   /**< Device ID of the sub-core */
  UINTPTR AbsAddr; /**< Absolute Base Address of hte Sub-cores*/
}XV_HdmiRxSs1_SubCore;

/**
 * Video Processing Subsystem configuration structure.
 * Each subsystem device should have a configuration structure associated
 * that defines the MAX supported sub-cores within subsystem
 */

typedef struct
{
  u16 DeviceId;     /**< DeviceId is the unique ID  of the device */
  UINTPTR BaseAddress;  /**< BaseAddress is the physical base address of the
                        subsystem address range */
  UINTPTR HighAddress;  /**< HighAddress is the physical MAX address of the
                        subsystem address range */
  XVidC_PixelsPerClock Ppc;         /**< Supported Pixel per Clock */
  u8 MaxBitsPerPixel;               /**< Maximum  Supported Color Depth */
  u32 MaxFrlRate;                   /** < Maximum FRL Rate Supporte */
  u32 DynamicHDR;	/**< Dynamic HDR supported */
  u32 DSC; /**< DSC Supported */
  u32 AxiLiteClkFreq;               /**< AXI Lite Clock Frequency in Hz */
  u8 VideoInterface; /**< 0 - AXI4S 1 - Native 2 - Native DE video interface */
  XV_HdmiRxSs1_SubCore HdcpTimer;    /**< Sub-core instance configuration */
  XV_HdmiRxSs1_SubCore Hdcp14;       /**< Sub-core instance configuration */
  XV_HdmiRxSs1_SubCore Hdcp22;       /**< Sub-core instance configuration */
  XV_HdmiRxSs1_SubCore HdmiRx1;       /**< Sub-core instance configuration */
} XV_HdmiRxSs1_Config;

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
typedef void (*XV_HdmiRxSs1_Callback)(void *CallbackRef);

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
typedef u64 (*XV_HdmiRxSs1_LogCallback)(void *CallbackRef);

/**
* The XVprocss driver instance data. The user is required to allocate a variable
* of this type for every XVprocss device in the system. A pointer to a variable
* of this type is then passed to the driver API functions.
*/
typedef struct
{
  XV_HdmiRxSs1_Config Config;    /**< Hardware configuration */
  u32 IsReady;                  /**< Device and the driver instance are
                                     initialized */
  u8 AppMajVer;       /**< Major Version of application used by the driver */
  u8 AppMinVer;       /**< Minor Version of application used by the driver */

#ifdef XV_HDMIRXSS1_LOG_ENABLE
  XV_HdmiRxSs1_Log Log;				/**< A log of events. */
#endif

#ifdef XPAR_XHDCP_NUM_INSTANCES
  XTmrCtr *HdcpTimerPtr;           /**< handle to sub-core driver instance */
  XHdcp1x *Hdcp14Ptr;                /**< handle to sub-core driver instance */
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  XHdcp22_Rx  *Hdcp22Ptr;           /**< handle to sub-core driver instance */
#endif
  XV_HdmiRx1 *HdmiRx1Ptr;             /**< handle to sub-core driver instance */

  /*Callbacks */
  XV_HdmiRxSs1_Callback ConnectCallback; /**< Callback for connect event */
  void *ConnectRef;     /**< To be passed to the connect callback */

  XV_HdmiRxSs1_Callback BrdgOverflowCallback;  /**< Callback for Bridge overflow
                                           event */
  void *BrdgOverflowRef;         /**< To be passed to the Bridge overflow
                                    callback */

  XV_HdmiRxSs1_Callback AuxCallback;     /**< Callback for AUX event */
  void *AuxRef;         /**< To be passed to the AUX callback */

  XV_HdmiRxSs1_Callback AudCallback;     /**< Callback for AUD event */
  void *AudRef;         /**< To be passed to the AUD callback */

  XV_HdmiRxSs1_Callback LnkStaCallback;  /**< Callback for LNKSTA event */
  void *LnkStaRef;      /**< To be passed to the LNKSTA callback */

  XV_HdmiRxSs1_Callback DdcCallback;     /**< Callback for PDDC event */
  void *DdcRef;         /**< To be passed to the DDC callback */

  XV_HdmiRxSs1_Callback StreamDownCallback;/**< Callback for stream down event */
  void *StreamDownRef;  /**< To be passed to the stream down callback */

  XV_HdmiRxSs1_Callback StreamInitCallback;/**< Callback for stream init event */
  void *StreamInitRef;  /**< To be passed to the stream init callback */

  XV_HdmiRxSs1_Callback StreamUpCallback; /**< Callback for stream up event */
  void *StreamUpRef;    /**< To be passed to the stream up callback */

  XV_HdmiRxSs1_Callback HdcpCallback;    /**< Callback for HDCP 1.4 event */
  void *HdcpRef;        /**< To be passed to the hdcp callback */

  XV_HdmiRxSs1_Callback TmdsClkRatioCallback;  /**< Callback for scdc TMDS clock
                                                   ratio change callback */
  void *TmdsClkRatioRef; /**< To be passed to the scdc tmds clock ratio change
                             callback */

  XV_HdmiRxSs1_Callback VicErrorCallback;  /**< Callback for VIC error
                                               detection */
  void *VicErrorRef;   /**< To be passed to the VIC error callback */

  XV_HdmiRxSs1_LogCallback LogWriteCallback; /**< Callback for log write */
  u8 *LogWriteRef;  /**< To be passed to the log write callback */

  XV_HdmiRxSs1_Callback PhyResetCallback; /**< Callback for config retry */
  void *PhyResetRef;  /**< To be passed to the log write callback */

  XV_HdmiRxSs1_Callback FrlConfigCallback; /**< Callback for stream up */
  void *FrlConfigRef;  /**< To be passed to the stream up callback */

  XV_HdmiRxSs1_Callback FrlStartCallback; /**< Callback for stream up */
  void *FrlStartRef;  /**< To be passed to the stream up callback */

  XV_HdmiRxSs1_Callback TmdsConfigCallback; /**< Callback for stream up */
  void *TmdsConfigRef;  /**< To be passed to the stream up callback */

  XV_HdmiRxSs1_Callback FrlLtsLCallback; /**< Callback for FRL LTS:L */
  void *FrlLtsLRef;  /**< To be passed to FRL LTS:L callback */

  XV_HdmiRxSs1_Callback FrlLts1Callback; /**< Callback for FRL LTS:1 */
  void *FrlLts1Ref;  /**< To be passed to FRL LTS:1 callback */

  XV_HdmiRxSs1_Callback FrlLts2Callback; /**< Callback for FRL LTS:2 */
  void *FrlLts2Ref;  /**< To be passed to FRL LTS:2 callback */

  XV_HdmiRxSs1_Callback FrlLts3Callback; /**< Callback for FRL LTS:3 */
  void *FrlLts3Ref;  /**< To be passed to FRL LTS:3 callback */

  XV_HdmiRxSs1_Callback FrlLts4Callback; /**< Callback for FRL LTS:4 */
  void *FrlLts4Ref;  /**< To be passed to FRL LTS:4 callback */

  XV_HdmiRxSs1_Callback FrlLtsPCallback; /**< Callback for FRL LTS:P */
  void *FrlLtsPRef;  /**< To be passed to FRL LTS:P callback */

  XV_HdmiRxSs1_Callback VfpChangeCallback; /**< Callback for vfp change event */
  void *VfpChangeRef;  /**< To be passed to vfp change event callback */

  XV_HdmiRxSs1_Callback VrrRdyCallback; /**< Callback for Vrr Ready event */
  void *VrrRdyRef;  /**< To be passed to Vrr Ready event callback */

  XV_HdmiRxSs1_Callback DynHdrCallback; /**< Callback for Dynamic HDR event */
  void *DynHdrRef;  /**< To be passed to Dynamic HDR event callback */

  XV_HdmiRxSs1_Callback DSCStreamChangeEventCallback; /**< Callback for DSC stream change event */
  void *DSCStrmChgEvtRef;  /**< To be passed to DSC Stream change event callback */

  XV_HdmiRxSs1_Callback DSCPktErrCallback; /**< Callback for DSC PPS packet error event */
  void *DSCPktErrRef;  /**< To be passed to DSC PPS packet error event callback */

  XV_HdmiRxSs1_Callback DSCStsUpdtEvtCallback; /**< Callback for SCDC reg 0x10 bit 0 Status_Update bit
						    set from Source event */
  void *DSCStsUpdtEvtRef;  /**< To be passed to Status_Update bit set event callback */

  /* Scratch pad*/
  u8 IsStreamConnected;         /**< HDMI RX Stream Connected */
  u8 IsStreamUp;                /**< HDMI RX Stream Up */
  u8 AudioChannels;             /**< Number of Audio Channels */
  int IsLinkStatusErrMax;       /**< Link Error Status Maxed */
  u8 *EdidPtr;                     /**< Default Edid Pointer */
  u16 EdidLength;               /**< Default Edid Length */
  u8 TMDSClockRatio;            /**< HDMI RX TMDS clock ratio */

  u8 EnableHDCPLogging;         /**< HDCP Logging Enabling */
  u8 EnableHDMILogging;         /**< HDMI Logging Enabling */

  XHdmiC_AVI_InfoFrame AVIInfoframe;	/**< AVI InfoFrame */
  XHdmiC_GeneralControlPacket GCP;		/**< General Control Packet */
  XHdmiC_AudioInfoFrame AudioInfoframe;	/**< Audio InfoFrame */
  XHdmiC_VSIF VSIF;						/**< Vendor Specific InfoFrame */
  XHdmiC_DRMInfoFrame DrmInfoframe;	/**< Static HDR infoframe */

  XVidC_DelayHandler UserTimerWaitUs; /**< Custom user function for
                                           delay/sleep. */
  void *UserTimerPtr;                 /**< Pointer to a timer instance
                                           used by the custom user
                                           delay/sleep function. */

  XV_HdmiRxSs1_HdcpProtocol      HdcpProtocol;   /**< HDCP protocol selected */
#ifdef USE_HDCP_RX
  /**< HDCP specific */
  u32                           HdcpIsReady;    /**< HDCP ready flag */
  XV_HdmiRxSs1_HdcpEventQueue    HdcpEventQueue;         /**< HDCP event queue */
#endif
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  u8                            *Hdcp22Lc128Ptr;     /**< Pointer to HDCP 2.2
                                                         LC128 */
  u8                            *Hdcp22PrivateKeyPtr;   /**< Pointer to HDCP 2.2
                                                             Private key */
#endif
#ifdef XPAR_XHDCP_NUM_INSTANCES
  u8                            *Hdcp14KeyPtr;   /**< Pointer to HDCP 1.4 key */
#endif
} XV_HdmiRxSs1;

/** @name HDMI RX SS Dynamic HDR Error type
* @{
*/
typedef enum {
	XV_HDMIRXSS1_DYNHDR_ERR_SEQID = 1, /* Sequence id or ECC error */
	XV_HDMIRXSS1_DYNHDR_ERR_MEMWR = 2, /* Memory write error */
} XV_HdmiRxSs1_DynHdrErrType;

/**
* This typedef contains HDMI RX stream specific Dynamic HDR info.
*/
typedef struct {
	XV_HdmiRxSs1_DynHdrErrType err; /* Error type */
	u16 pkt_type;	/* Packet Type */
	u16 pkt_length; /* Packet length */
	u8 gof;		/* Graphics Overlay Flag */
} XV_HdmiRxSs1_DynHDR_Info;

/************************** Macros Definitions *******************************/
#ifdef USE_HDCP_RX
#define XV_HdmiRxSs1_HdcpIsReady(InstancePtr) \
  (InstancePtr)->HdcpIsReady
#endif
/************************** Function Prototypes ******************************/
XV_HdmiRxSs1_Config* XV_HdmiRxSs1_LookupConfig(u32 DeviceId);
void XV_HdmiRxSs1_SetUserTimerHandler(XV_HdmiRxSs1 *InstancePtr,
	XVidC_DelayHandler CallbackFunc, void *CallbackRef);
void XV_HdmiRxSS1_HdmiRxIntrHandler(XV_HdmiRxSs1 *InstancePtr);
int XV_HdmiRxSs1_CfgInitialize(XV_HdmiRxSs1 *InstancePtr,
	XV_HdmiRxSs1_Config *CfgPtr,
	UINTPTR EffectiveAddr);
void XV_HdmiRxSS1_SetAppVersion(XV_HdmiRxSs1 *InstancePtr, u8 maj, u8 min);
void XV_HdmiRxSs1_Start(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_Stop(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_Reset(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_RXCore_VRST(XV_HdmiRxSs1 *InstancePtr, u8 Reset);
void XV_HdmiRxSs1_RXCore_LRST(XV_HdmiRxSs1 *InstancePtr, u8 Reset);
void XV_HdmiRxSs1_VRST(XV_HdmiRxSs1 *InstancePtr, u8 Reset);
void XV_HdmiRxSs1_SYSRST(XV_HdmiRxSs1 *InstancePtr, u8 Reset);
int XV_HdmiRxSs1_SetCallback(XV_HdmiRxSs1 *InstancePtr,
		XV_HdmiRxSs1_HandlerType HandlerType,
		void *CallbackFunc,
		void *CallbackRef);
int XV_HdmiRxSs1_SetLogCallback(XV_HdmiRxSs1 *InstancePtr,
	u64 *CallbackFunc,
	void *CallbackRef);
void XV_HdmiRxSs1_SetEdidParam(XV_HdmiRxSs1 *InstancePtr, u8 *EdidDataPtr,
				u16 Length);
void XV_HdmiRxSs1_LoadDefaultEdid(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_LoadEdid(XV_HdmiRxSs1 *InstancePtr, u8 *EdidDataPtr,
			u16 Length);
void XV_HdmiRxSs1_SetHpd(XV_HdmiRxSs1 *InstancePtr, u8 Value);
void XV_HdmiRxSs1_ToggleHpd(XV_HdmiRxSs1 *InstancePtr);
XHdmiC_Aux *XV_HdmiRxSs1_GetAuxiliary(XV_HdmiRxSs1 *InstancePtr);
XHdmiC_AVI_InfoFrame *XV_HdmiRxSs1_GetAviInfoframe(XV_HdmiRxSs1 *InstancePtr);
XHdmiC_AudioInfoFrame *XV_HdmiRxSs1_GetAudioInfoframe(XV_HdmiRxSs1 *InstancePtr);
XHdmiC_VSIF *XV_HdmiRxSs1_GetVSIF(XV_HdmiRxSs1 *InstancePtr);
XHdmiC_DRMInfoFrame *XV_HdmiRxSs1_GetDrmInfoframe(XV_HdmiRxSs1 *InstancePtr);
XHdmiC_GeneralControlPacket *XV_HdmiRxSs1_GetGCP(XV_HdmiRxSs1 *InstancePtr);
u32 XV_HdmiRxSs1_SetStream(XV_HdmiRxSs1 *InstancePtr,
    u32 Clock,
    u32 LineRate);
XVidC_VideoStream *XV_HdmiRxSs1_GetVideoStream(XV_HdmiRxSs1 *InstancePtr);
u8 XV_HdmiRxSs1_GetVideoIDCode(XV_HdmiRxSs1 *InstancePtr);
u8 XV_HdmiRxSs1_GetVideoStreamType(XV_HdmiRxSs1 *InstancePtr);
u8 XV_HdmiRxSs1_GetTransportMode(XV_HdmiRxSs1 *InstancePtr);
u8 XV_HdmiRxSs1_GetVideoStreamScramblingFlag(XV_HdmiRxSs1 *InstancePtr);
u8 XV_HdmiRxSs1_GetAudioChannels(XV_HdmiRxSs1 *InstancePtr);
XV_HdmiRx1_AudioFormatType XV_HdmiRxSs1_GetAudioFormat(XV_HdmiRxSs1 *InstancePtr);
u32 XV_HdmiRxSs1_GetAudioAcrCtsVal(XV_HdmiRxSs1 *InstancePtr);
u32 XV_HdmiRxSs1_GetAudioAcrNVal(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_RefClockChangeInit(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_ReportInfo(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_RegisterDebug(XV_HdmiRxSs1 *InstancePtr);
int  XV_HdmiRxSs1_IsStreamUp(XV_HdmiRxSs1 *InstancePtr);
int  XV_HdmiRxSs1_IsStreamConnected(XV_HdmiRxSs1 *InstancePtr);

void XV_HdmiRxSs1_SetDefaultPpc(XV_HdmiRxSs1 *InstancePtr, u8 Id);
void XV_HdmiRxSs1_SetPpc(XV_HdmiRxSs1 *InstancePtr, u8 Id, u8 Ppc);
XVidC_PixelsPerClock XV_HdmiRxSs1_GetCorePpc(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_AudioMute(XV_HdmiRxSs1 *InstancePtr, u8 Enable);
void XV_HdmiRxSs1_VfpControl(XV_HdmiRxSs1 *InstancePtr, u8 Enable);
XV_HdmiC_VrrInfoFrame *XV_HdmiRxSs1_GetVrrIf(XV_HdmiRxSs1 *InstancePtr);

void XV_HdmiRxSs1_DynHDR_DM_Enable(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_DynHDR_DM_Disable(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_DynHDR_SetAddr(XV_HdmiRxSs1 *InstancePtr, u64 Addr);
void XV_HdmiRxSs1_DynHDR_GetInfo(XV_HdmiRxSs1 *InstancePtr,
				 XV_HdmiRxSs1_DynHDR_Info *RxDynInfoPtr);

u32 XV_HdmiRxSs1_DSC_IsEnableStream(XV_HdmiRxSs1 *InstancePtr);
int XV_HdmiRxSs1_DSC_SetDecodeFail(XV_HdmiRxSs1 *InstancePtr);
int XV_HdmiRxSs1_DSC_SetDscFrlMax(XV_HdmiRxSs1 *InstancePtr);

void XV_HdmiRxSs1_ReportCoreInfo(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_DebugInfo(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_DdcRegDump(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_ReportTiming(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_ReportLinkQuality(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_ReportAudio(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_ReportInfoFrame(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_ReportSubcoreVersion(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_ReportDRMInfo(XV_HdmiRxSs1 *InstancePtr);

#ifdef XV_HDMIRXSS1_LOG_ENABLE
void XV_HdmiRxSs1_LogReset(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_LogWrite(XV_HdmiRxSs1 *InstancePtr, XV_HdmiRxSs1_LogEvent Evt, u8 Data);
u16 XV_HdmiRxSs1_LogRead(XV_HdmiRxSs1 *InstancePtr);
#endif
void XV_HdmiRxSs1_LogDisplay(XV_HdmiRxSs1 *InstancePtr);

/* Fixed Rate Link */
void XV_HdmiRxSs1_FrlLinkRetrain(XV_HdmiRxSs1 *InstancePtr, u8 LtpThreshold,
		XV_HdmiRx1_FrlLtp DefaultLtp);
void XV_HdmiRxSs1_FrlModeEnable(XV_HdmiRxSs1 *InstancePtr, u8 LtpThreshold,
				XV_HdmiRx1_FrlLtp DefaultLtp, u8 FfeSuppFlag);
void XV_HdmiRxSs1_SetFrlFltNoTimeout(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_ClearFrlFltNoTimeout(XV_HdmiRxSs1 *InstancePtr);

#ifdef USE_HDCP_RX
void XV_HdmiRxSs1_HdcpSetKey(XV_HdmiRxSs1 *InstancePtr, XV_HdmiRxSs1_HdcpKeyType KeyType, u8 *KeyPtr);
int XV_HdmiRxSs1_HdcpEnable(XV_HdmiRxSs1 *InstancePtr);
int XV_HdmiRxSs1_HdcpDisable(XV_HdmiRxSs1 *InstancePtr);
int XV_HdmiRxSs1_HdcpClearEvents(XV_HdmiRxSs1 *InstancePtr);
int XV_HdmiRxSs1_HdcpPushEvent(XV_HdmiRxSs1 *InstancePtr, XV_HdmiRxSs1_HdcpEvent Event);
int XV_HdmiRxSs1_HdcpPoll(XV_HdmiRxSs1 *InstancePtr);
int XV_HdmiRxSs1_HdcpSetProtocol(XV_HdmiRxSs1 *InstancePtr, XV_HdmiRxSs1_HdcpProtocol Protocol);
int XV_HdmiRxSs1_HdcpSetCapability(XV_HdmiRxSs1 *InstancePtr, XV_HdmiRxSs1_HdcpProtocol Protocol);
XV_HdmiRxSs1_HdcpProtocol XV_HdmiRxSs1_HdcpGetProtocol(XV_HdmiRxSs1 *InstancePtr);
int XV_HdmiRxSs1_HdcpIsEnabled(XV_HdmiRxSs1 *InstancePtr);
int XV_HdmiRxSs1_HdcpIsAuthenticated(XV_HdmiRxSs1 *InstancePtr);
int XV_HdmiRxSs1_HdcpIsEncrypted(XV_HdmiRxSs1 *InstancePtr);
int XV_HdmiRxSs1_HdcpIsInProgress(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_HdcpInfo(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSs1_HdcpSetInfoDetail(XV_HdmiRxSs1 *InstancePtr, u8 Verbose);
int XV_HdmiRxSs1_HdcpSetTopology(XV_HdmiRxSs1 *InstancePtr, void *TopologyPtr);
int XV_HdmiRxSs1_HdcpSetTopologyReceiverIdList(XV_HdmiRxSs1 *InstancePtr, u8 *ListPtr, u32 ListSize);
int XV_HdmiRxSs1_HdcpSetTopologyField(XV_HdmiRxSs1 *InstancePtr,
	XV_HdmiRxSs1_HdcpTopologyField Field, u32 Value);
int XV_HdmiRxSs1_HdcpSetRepeater(XV_HdmiRxSs1 *InstancePtr, u8 Set);
int XV_HdmiRxSs1_HdcpSetTopologyUpdate(XV_HdmiRxSs1 *InstancePtr);
XV_HdmiRxSs1_HdcpContentStreamType XV_HdmiRxSs1_HdcpGetContentStreamType(XV_HdmiRxSs1 *InstancePtr);
int XV_HdmiRxSs1_HdcpIsRepeater(XV_HdmiRxSs1 *InstancePtr);
int XV_HdmiRxSs1_HdcpIsInWaitforready(XV_HdmiRxSs1 *InstancePtr);
int XV_HdmiRxSs1_HdcpIsInComputations(XV_HdmiRxSs1 *InstancePtr);

#ifdef XPAR_XHDCP_NUM_INSTANCES
int XV_HdmiRxSs1_HdcpTimerStart(void *InstancePtr, u16 TimeoutInMs);
int XV_HdmiRxSs1_HdcpTimerStop(void *InstancePtr);
int XV_HdmiRxSs1_HdcpTimerBusyDelay(void *InstancePtr, u16 DelayInMs);
void XV_HdmiRxSS1_HdcpIntrHandler(XV_HdmiRxSs1 *InstancePtr);
void XV_HdmiRxSS1_HdcpTimerIntrHandler(XV_HdmiRxSs1 *InstancePtr);
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
void XV_HdmiRxSS1_Hdcp22TimerIntrHandler(XV_HdmiRxSs1 *InstancePtr);
#endif

#endif /* USE_HDCP_RX*/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
