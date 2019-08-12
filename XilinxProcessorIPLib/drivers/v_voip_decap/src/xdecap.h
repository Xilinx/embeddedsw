/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
* @file xdecap.h
*
* This is the main header file for VoIP Decapsulator core. VoIP Decapsulator
* core is used for Incoming Packet Filter, Header Stripping and SDI Video
* Detection.
*
* The VoIP Decapsulator has 3 main Interface
* - AXI4-Stream Interface for accepting Incoming Ethernet Packet
* - AXI4-Stream Interface for transmitting RTP Packet <RTP Header | Payload>
* - AXI4-Lite interface for processor, controls the VoIP Decapsulator.
*
* VoIP Decapsulator Performs Following Operation
* - Filtering Packet to Channel based on Incoming Ethernet Packet Header
* - Striping Incoming Ethernet Header (MAC,IP & UDP) to form RTP Packet
* - SDI Video Information Extraction and Detection on Ethernet Packet (ST2022)
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* VoIP Decapsulator core to be ready.
*
* - Call XDecap_LookupConfig using a device ID to find the core configuration.
* - Call XDecap_CfgInitialize to initialize the device and the driver
*   instance associated with it.
*
* This driver provides interrupt handlers
* - XDecap_IntrHandler, for handling the interrupts from the VoIP Decapsulator
*    core
*
* Application developer needs to register interrupt handler with the processor,
* within their examples. Whenever processor calls registered application's
* interrupt handler associated with interrupt id, application's interrupt
* handler needs to call appropriate peripheral interrupt handler reading
* peripherals Status register.

* This driver provides XDecap_SetCallback API to register functions with
* Generic Decap core instance.
*
* <b> Virtual Memory </b>
*
*
* <b> Threads </b>
*
*
* <b> Asserts </b>
*
*
* <b> Building the driver </b>
*
* The VoIP Decapsulator driver is composed of several source files. This
* allows the user to build and link only those parts of the driver that are
* necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00   mmo   02/12/16 Initial release.

* </pre>
*
******************************************************************************/
#ifndef XDECAP_H_
#define XDECAP_H_ /**< Prevent circular inclusions
                  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xdecap_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#define XDECAP_MAX_CHANNEL   16

/**************************** Type Definitions *******************************/

/** @name Handler Types
* @{
*/
/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from peripheral.
*/
typedef enum {
    XDECAP_PACKET_HANDLER_CONNECT = 1,
    XDECAP_PACKET_HANDLER_LOCKED,
    XDECAP_PACKET_HANDLER_UNLOCKED,
    XDECAP_PACKET_HANDLER_STOP
} XDecap_HandlerType;
/*@}*/

/** @name Module Enable/Disable
* @{
*/
/**
* These constants specify specify the VoIP Decapsulator to enable/disable the
* incoming packet processing
*/
typedef enum {
    XDECAP_MODULE_DISABLE = 0,
    XDECAP_MODULE_ENABLE = 1
} XDecap_ModuleEnable;
/*@}*/

/** @name SDI Video Bit Rate
* @{
*/
/**
* These constants specify Incoming SDI Packet  (SMPTE2022-6) Format
* whether an Integer/Non-Integer
*/
typedef enum {
    XDECAP_INTEGER = 0,
    XDECAP_NON_INTEGER = 1
} XDecap_RXBITRATE;
/*@}*/

/** @name 3G-SDI Level
* @{
*/
/**
* These constants specify Incoming SDI Packet  (SMPTE2022-6) 3G Level
* whether Level A or Level B
*/
typedef enum {
    XDECAP_3G_LEVEL_A = 0,
    XDECAP_3G_LEVEL_B = 1
} XDecap_3G_LEVEL;
/*@}*/

/** @name SDI Mode
* @{
*/
/**
* These constants specify Incoming SDI Packet  (SMPTE2022-6) Mode
*/
typedef enum {
    XDECAP_SDI_SD = 1,
    XDECAP_SDI_HD = 0,
    XDECAP_SDI_3G = 3
} XDecap_SDIMode;
/*@}*/

/** @name Match RTP-SSRC
* @{
*/
/**
* These constants specify the RTP-SSRC Field Filtering to be enabled
*/
typedef enum match_ssrc {
    XDECAP_UNMATCH_SSRC    = 0,
    XDECAP_MATCH_SSRC      = XDECAP_MATCH_SELECT_TO_MATCH_SSRC_MASK
} XDecap_MatchSSRCReg;
/*@}*/

/** @name Match UDP Destination Port
* @{
*/
/**
* These constants specify the UDP Destination Port Filtering to be enabled
*/
typedef enum match_udpdest {
    XDECAP_UNMATCH_UDP_DST = 0,
    XDECAP_MATCH_UDP_DST   = XDECAP_MATCH_SELECT_TO_MATCH_UDP_DEST_MASK
} XDecap_MatchUDPDestReg;
/*@}*/

/** @name Match UDP Source Port
* @{
*/
/**
* These constants specify the UDP Source Port Filtering to be enabled
*/
typedef enum match_udpsrc {
    XDECAP_UNMATCH_UDP_SRC = 0,
    XDECAP_MATCH_UDP_SRC   = XDECAP_MATCH_SELECT_TO_MATCH_UDP_SRC_MASK
} XDecap_MatchUDPSrcReg;
/*@}*/

/** @name Match IP Destination Address
* @{
*/
/**
* These constants specify the IP Destination Address Filtering to be enabled
*/
typedef enum match_ipdest {
    XDECAP_UNMATCH_IP_DST  = 0,
    XDECAP_MATCH_IP_DST    = XDECAP_MATCH_SELECT_TO_MATCH_DEST_IP_MASK
} XDecap_MatchIPDestReg;
/*@}*/

/** @name Match IP Source Address
* @{
*/
/**
* These constants specify the IP Source Address Filtering to be enabled
*/
typedef enum match_ipsrc {
    XDECAP_UNMATCH_IP_SRC  = 0,
    XDECAP_MATCH_IP_SRC    = XDECAP_MATCH_SELECT_TO_MATCH_SRC_IP_MASK
} XDecap_MatchIPSrcReg;
/*@}*/

/** @name Match VLAN ID
* @{
*/
/**
* These constants specify the VLAN Tag ID Filtering to be enabled
*/
typedef enum match_vlanreg {
    XDECAP_UNMATCH_VLAN_TAG_ID_REG = 0,
    XDECAP_MATCH_VLAN_TAG_ID_REG   =
                    XDECAP_MATCH_SELECT_TO_MATCH_VLAN_ID_REG_MASK
} XDecap_MatchVLANReg;
/*@}*/

/** @name Match VLAN Packets
* @{
*/
/**
* These constants specify to enable filtering Packet with/without VLAN
*/
typedef enum vlan_matching {
    XDECAP_MATCH_PACKET_WITHOUT_VLAN = 0,
    XDECAP_MATCH_PACKET_WITH_VLAN_AND_MATCH_VLAN_ID = 1
} XDecap_VLANMatching;
/*@}*/

/** @name Channel Enable
* @{
*/
/**
* These constants specify the operating channel enable/disable
*/
typedef enum channel_control_enable {
    XDECAP_CHANNEL_DISABLE = 0,
    XDECAP_CHANNEL_ENABLE = 1
} XDecap_ChannelEnable;
/*@}*/

/** @name VoIP Decapsulator Operating Mode
* @{
*/
/**
* These constants specify the operating mode of VoIP Decapsulator
*/
typedef enum operation_mode {
    XDECAP_NORMAL_MODE   = 0,
    XDECAP_LOSSLESS_MODE = 1
} XDecap_OperationMode;

/** @name VoIP Decapsulator Marker Packet Detection Enable/Disable
* @{
*/
/**
* These constants specify the Marker Packet Detection Enable/Disable of VoIP
* Decapsulator
*/
typedef enum m_pkt_det {
    XDECAP_MARKER_PACKET_DET_DISABLE = 0,
    XDECAP_MARKER_PACKET_DET_ENABLE = 1
} XDecap_MPktDetEn;

/** @name VoIP Decapsulator Marker Packet Drop Enable
* @{
*/
/**
* These constants specify the Marker Packet Drop Enable/Disable of
* VoIP Decapsulator
*/
typedef enum m_pkt_drop {
    XDECAP_MARKER_PACKET_DROP_DISABLE = 0,
    XDECAP_MARKER_PACKET_DROP_ENABLE = 1
} XDecap_MPktDropEn;

/**
* This typedef contains Incoming IP Header Filtering Enable Information
*/
typedef struct {
    XDecap_MatchSSRCReg    match_ssrc;
    XDecap_MatchUDPDestReg match_udp_dst;
    XDecap_MatchUDPSrcReg  match_udp_src;
    XDecap_MatchIPDestReg  match_ip_dst;
    XDecap_MatchIPSrcReg   match_ip_src;
    XDecap_MatchVLANReg    match_vlan_reg;
} XDecap_MatchSelect;


/**
* This typedef contains Incoming Packet Header Information
*/
typedef struct {
    /* MAC Header */
    u32 dest_mac_0;
    u16 dest_mac_1;
    u16 vlan_pcp_cfi_vid;

    /* IP Header */
    u8  ttl;
    u8  tos;
    u32 ipv4_src_address;
    u32 ipv4_dest_address;

    /* UDP Header */
    u16 udp_src_port;
    u16 udp_dest_port;

    /* RTP Header */
    u32 ssrc;
} XDecap_Header;

/**
* This typedef contains Incoming Packet SDI Video Extraction Information
* (ST2022-6)
*/
typedef struct {
    /* Packet Format (Related to SDI) */
    u8  video_ts_include;
    u16 datagram_per_frame;
    u16 last_datagram_len;

    /* Video Format */
    XDecap_RXBITRATE   rx_bitrate;
    XDecap_SDIMode     sdi_mode;
    XDecap_3G_LEVEL    level_b_3g;

    /* MAP_FRAME_FRATE_SAMPLE_FMT */
    u8 MAP;
    u8 FRAME;
    u8 FRATE;
    u8 SAMPLE;
} XDecap_StreamStatus;

/**
* This typedef contains VoIP Decapsulator Channel Information
*/
typedef struct {
    XDecap_ChannelEnable  Channel_Enable;
    XDecap_OperationMode  LosslessMode;
    XDecap_MPktDetEn      MPkt_DetEn;
    XDecap_MPktDropEn     MPkt_DropEn;
    XDecap_VLANMatching   is_MatchVlanPckt;
    XDecap_Header         MatchHeader;
    XDecap_MatchSelect    MatchSelect;
    XDecap_StreamStatus   StreamInfo;
    u32                   PacketStopTimer; /*<Packet Stop Timer (On Eth Clock
                                           Domain)*/
} XDecap_ChannelCfg;

/**
* Callback type for VoIP Decapsulator event interrupt.
*
* @param    CallbackRef is a callback reference passed in by the upper
*       layer when setting the callback functions, and passed back to
*       the upper layer when the callback is invoked.
*
* @return   None.
*
* @note     None.
*
*/
typedef void (*XDecap_Callback)(void *CallbackRef);

/**
* This typedef contains configuration information for the VoIP Decapsulator
* core. Each VoIP Decapsulator device should have a configuration structure
* associated.
*/
typedef struct {
   u16 DeviceId;       /**< DeviceId is the unique ID of the VoIP Decapsulator
                       *core */
   UINTPTR BaseAddress;/**< BaseAddress is the physical
                       *base address of the core's registers */

   /*<Core Generics Info>*/
   u16 HWChannelNumber;/**< Current Supported Channel by the core */
} XDecap_Config;

/**
* The XDecap driver instance data. An instance must be allocated for each
* VoIP Decapsulator core in use.
*/
typedef struct {
   XDecap_Config        Config;           /**< Hardware Configuration */
   u32                  IsReady;          /**< Core and the driver instance
                                          *are initialized */

   /* Current Interrupt Channel */
   u16                  IntrCh;

   /* Current Interrupt Status */
   u8 PacketLockInterrupt;
   u8 PacketUnLockInterrupt;
   u8 PacketStopInterrupt;
   u8 FinalUnLockInterrupt;

   /*Callbacks */
   XDecap_Callback PacketLockCallback;     /**< Callback for Packet lock event
                                           *interrupt */
   void *PacketLockRef;                    /**< To be passed to the connect
                                           *interrupt callback */
   u32 IsPacketLockCallbackSet;            /**< Set flag. This flag is set to
                                           *true when the callback has been
                                           *registered */

   XDecap_Callback PacketUnLockCallback;   /**< Callback for Packet unlock
                                           *event interrupt */
   void *PacketUnLockRef;                  /**< To be passed to the connect
                                           *interrupt callback */
   u32 IsPacketUnLockCallbackSet;          /**< Set flag. This flag is set to
                                           *true when the callback has been
                                           *registered */

   XDecap_Callback PacketStopCallback;     /**< Callback for Packet Stop event
                                           *interrupt */
   void *PacketStopRef;                    /**< To be passed to the connect
                                           *interrupt callback */
   u32 IsPacketStopCallbackSet;            /**< Set flag. This flag is set to
                                           *true when the callback has been
                                           *registered */

   /* Packet Lock/UnLock Setting */
   u16 PacketLockWindow;
   u16 PacketUnLockWindow;

   /* Statistic */
   /* General/Non-Filtered Packet */
   u32 RXPcktCnt;
   u32 MisMatchedPcktCnt;
   u32 ErrPcktCnt;
   /* Channel/Filtered Packet */
   u32 MediaValid_pkt_cnt[XDECAP_MAX_CHANNEL];
   u32 FECValid_pkt_cnt[XDECAP_MAX_CHANNEL];
   u32 ReOrdered_pkt_cnt[XDECAP_MAX_CHANNEL];
   u32 Drop_pkt_cnt[XDECAP_MAX_CHANNEL];

   /* Supported Operational Channel */
   u16 OpChannel;

   /* Module Enable */
   XDecap_ModuleEnable  ModuleEnable;

   /* Channel Config */
   XDecap_ChannelCfg    ChannelCfg[XDECAP_MAX_CHANNEL];
} XDecap;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro Check the VoIP Decapsulator Busy Status which indicates
* the core is still updating the registers.
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   BusyBitStatus.
*
* @note
*
*       void XDecap_BusyBit(XDecap *InstancePtr)
*
******************************************************************************/
#define XDecap_BusyBit(InstancePtr) \
        XDecap_ReadReg((InstancePtr)->Config.BaseAddress, \
               (XDECAP_STATUS_OFFSET)) & (XDECAP_STATUS_UPDATE_BUSY_MASK)

/*****************************************************************************/
/**
*
* This macro Check the VoIP Decapsulator Current Supported Channel in the
* Hardware.
*
* @param    InstancePtr is a pointer to the XDecap core instance.
*
* @return   Current Supported Number of Channel in the Hardware.
*
* @note
*
*       void XDecap_NumSupportedChannel(XDecap *InstancePtr)
*
******************************************************************************/
#define XDecap_NumSupportedChannel(InstancePtr) \
        XDecap_ReadReg((InstancePtr)->Config.BaseAddress, \
              (XDECAP_SYS_CONFIG_OFFSET)) & (XDECAP_SYS_CONFIG_C_CHANNELS_MASK)

/************************** Function Prototypes ******************************/

/* Initialization function in xdecap_sinit.c */
XDecap_Config *XDecap_LookupConfig(u16 DeviceId);

/* Level 2 Drivers */
int XDecap_CfgInitialize(XDecap *InstancePtr, XDecap_Config *CfgPtr,
                    UINTPTR EffectiveAddr);
void XDecap_CoreChannelConfig(XDecap *InstancePtr);
void XDecap_ChannelConfig(XDecap *InstancePtr, u16 Channels);
XDecap_Config XDecap_CoreInfo(XDecap *InstancePtr);
XDecap_ChannelCfg XDecap_ChStatus(XDecap *InstancePtr, u16 Channels);

/* General Low Levels Drivers */
void XDecap_SoftReset(XDecap *InstancePtr);
void XDecap_ChannelUpdate(XDecap *InstancePtr);
void XDecap_ChannelAccess(XDecap *InstancePtr, u16 Channels);
u32 XDecap_RXPacketsCnt(XDecap *InstancePtr);
u32 XDecap_DropPacketsCnt(XDecap *InstancePtr);
u32 XDecap_ErrorPacketsCnt(XDecap *InstancePtr);
u32 XDecap_MismatchPacketCnt(XDecap *InstancePtr);
void XDecap_ClearGeneralStatistic(XDecap *InstancePtr);
void XDecap_PacketLockWindow(XDecap *InstancePtr);
void XDecap_PacketUnLockWindow(XDecap *InstancePtr);
u8 XDecap_PeakBufferLv(XDecap *InstancePtr);
void XDecap_ModuleEn(XDecap *InstancePtr);

/* Channel Low Levels Drivers */
u32 XDecap_MediaValidPcktCnt(XDecap *InstancePtr, u16 Channels);
u32 XDecap_FECValidPcktCnt(XDecap *InstancePtr, u16 Channels);
u32 XDecap_ReOrderedPcktCnt(XDecap *InstancePtr, u16 Channels);
u32 XDecap_DropPcktCnt(XDecap *InstancePtr, u16 Channels);
void XDecap_SetOperationMode(XDecap *InstancePtr, u16 Channels);
void XDecap_SetIPType(XDecap *InstancePtr, u16 Channels);
void XDecap_ControlChannelEn(XDecap *InstancePtr, u16 Channels);
void XDecap_ControlMPktDetEn(XDecap *InstancePtr, u16 Channels);
void XDecap_ControlMPktDropEn(XDecap *InstancePtr, u16 Channels);
void XDecap_ChannelClearStatistic(XDecap *InstancePtr, u16 Channels);
void XDecap_SetMatchSelect(XDecap *InstancePtr, u16 Channels);
void XDecap_MatchIPv4Dest(XDecap *InstancePtr, u16 Channels);
void XDecap_MatchIPv4Src(XDecap *InstancePtr, u16 Channels);
void XDecap_MatchUDPDest(XDecap *InstancePtr, u16 Channels);
void XDecap_MatchUDPSrc(XDecap *InstancePtr, u16 Channels);
void XDecap_ToMatchVLANID(XDecap *InstancePtr, u16 Channels);
void XDecap_MatchVLANID(XDecap *InstancePtr, u16 Channels);
void XDecap_MatchSSRC(XDecap *InstancePtr, u16 Channels);
XDecap_StreamStatus XDecap_StreamStat(XDecap *InstancePtr, u16 Channels);
u8 XDecap_SDIPacketLockStatus (XDecap *InstancePtr, u16 Channels);
u8 XDecap_VideoLockIntrStatus(XDecap *InstancePtr, u16 Channels);
u8 XDecap_VideoUnLockIntrStatus(XDecap *InstancePtr, u16 Channels);
u8 XDecap_StreamStopIntrStatus(XDecap *InstancePtr, u16 Channels);
void XDecap_EnableVideoLockIntr(XDecap *InstancePtr, u16 Channels);
void XDecap_EnableVideoUnLockIntr(XDecap *InstancePtr, u16 Channels);
void XDecap_EnablePacketStopIntr(XDecap *InstancePtr, u16 Channels);
void XDecap_DisableVideoLockIntr(XDecap *InstancePtr, u16 Channels);
void XDecap_DisableVideoUnLockIntr(XDecap *InstancePtr, u16 Channels);
void XDecap_DisablePacketStopIntr(XDecap *InstancePtr, u16 Channels);
void XDecap_IntrClear(XDecap *InstancePtr, u16 Channels);
void XDecap_PacketStopTimer(XDecap *InstancePtr, u16 Channels);


/*  Interrupt Functions */
void XDecap_IntrHandler(void *InstancePtr);
int  XDecap_SetCallback(XDecap *InstancePtr, XDecap_HandlerType HandlerType,
    void *CallbackFunc, void *CallbackRef);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
