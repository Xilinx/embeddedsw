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
* @file xframer.h
*
* This is the main header file for VoIP Framer core. VoIP Framer core is
* used for generating Ethernet Headers for Incoming ST2022 RTP Packets or
* RTP Compliant Packet.
*
* The VoIP Framer has 3 main Interface
* - AXI4-Stream Interface for accepting Incoming Ethernet Packet
* - AXI4-Stream Interface for transmitting RTP Packet <RTP Header | Payload>
* - AXI4-Lite interface for processor, controls the VoIP Decapsulator.
*
* VoIP Framer Performs Following Operation
* - Add Ethernet Header, (MAC, IP, & UDP Header)
* - Statistic Monitoring of Incoming and Outgoing Packet
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* VoIP Framer core to be ready.
*
* - Call XFramer_LookupConfig using a device ID to find the core configuration.
* - Call XFramer_CfgInitialize to initialize the device and the driver
*   instance associated with it.
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
* The VoIP Framer driver is composed of several source files. This
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
#ifndef XFRAMER_H_
#define XFRAMER_H_ /**< Prevent circular inclusions
                  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xframer_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#define XFRAMER_MAX_CHANNEL   16

/**************************** Type Definitions *******************************/

/** @name Transmit Enable/Disable
* @{
*/
/**
* These constants specify specify the VoIP Framer to Enable/Disable the
* Transmission Packet.
*/
typedef enum {
    XFRAMER_MODULE_TRANSMIT_DISABLE = 0,
    XFRAMER_MODULE_TRANSMIT_ENABLE  = 1
}XFramerTXEnable;
/*@}*/

/** @name Enable VLAN
* @{
*/
/**
* These constants contains component to Enable VLAN Packet Generation
*/
typedef enum {
    XFRAMER_VLAN_DISABLE = 0,
    XFRAMER_VLAN_ENABLE  = 1
}XFramerVLANEnable;
/*@}*/

/** @name IPV6 Enable (Future Implementation)
* @{
*/
/**
* These constants contains component to enable IPv6 Packet Generation
*/
typedef enum {
    XFRAMER_IPV6_DISABLE = 0,
    XFRAMER_IPV6_ENABLE  = 1
}XFramerIPV6Enable;
/*@}*/

/**
* This typedef contains VoIP Framer Header Generation Parameter
*/
typedef struct {
    u32 Dest_MACAddr_Low;
    u16 Dest_MACAddr_High;
    u16 vlan_pcp_cfi_vid;
    u8  media_ttl;
    u8  media_tos;
    u8  fec_ttl;
    u8  fec_tos;
    u32 ip_src_address;
    u32 ip_dest_address;
    u16 udp_src_port;
    u16 udp_dest_port;
} XFramer_Header;

/**
* This typedef contains VoIP Framer Channel Parameter Configuration
*/
typedef struct {
    u32                 TX_PcktCnt;
    XFramerTXEnable     Transmit_Enable;
    XFramerVLANEnable   is_vlan;
    XFramerIPV6Enable   ipv6;            /* Future Implementation */
    XFramer_Header      PcktHeader;
} XFramer_ChannelCfg;

/**
* This typedef contains configuration information for the VoIP Framer core.
* Each VoIP Framer device should have a configuration structure associated.
*/
typedef struct {
    u16 DeviceId;                       /**< DeviceId is the unique ID of the
                                        **VoIP Framer core */
    UINTPTR BaseAddress;                /**< BaseAddress is the physical
                                        *base address of the core's registers */

    /* <Core Generics Info> */
    u16 HWChannelNumber;                /**< Current Supported Channel
                                        *by the core */
    u16 OverflowHandlingStrategy;       /**< Overflow Handling Strategy
                                        **Enable */
} XFramer_Config;

/**
* The XFramer driver instance data. An instance must be allocated for each
* VoIP Framer core in use.
*/
typedef struct {
    XFramer_Config        Config;
    u32                   IsReady;

    /* Common Header */
    u32 Src_MACAddr_Low;
    u16 Src_MACAddr_High;

    /* Statistic */
    u8  PeakBufferLevel;
    u32 RX_PcktCnt;

    /* Channel Config */
    XFramer_ChannelCfg    ChannelCfg[XFRAMER_MAX_CHANNEL];
} XFramer;


/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro Check the VoIP Framer Busy Status which indicates
* the core is still updating the registers.
*
* @param    InstancePtr is a pointer to the XFramer core instance.
*
* @return   BusyBitStatus.
*
* @note
*
*
*       void XFramer_AssertSoftReset(XFramer *InstancePtr)
*
******************************************************************************/
#define XFramer_BusyBit(InstancePtr) \
      XFramer_ReadReg((InstancePtr)->Config.BaseAddress, (XFRAMER_CONTROL)) \
          & (XFRAMER_STATUS_UPDATE_BUSY_MASK)

/************************** Function Prototypes ******************************/

/* Initialization function in xhdmi_tx_sinit.c */
XFramer_Config *XFramer_LookupConfig(u16 DeviceId);

/* Level 2 Drivers */
int XFramer_CfgInitialize(XFramer *InstancePtr, XFramer_Config *CfgPtr, \
    UINTPTR EffectiveAddr);
void XFramer_CoreConfig(XFramer *InstancePtr);
void XFramer_CoreChannelConfig(XFramer *InstancePtr);
void XFramer_ChannelConfig(XFramer *InstancePtr, u16 Channels);
XFramer_ChannelCfg XFramer_ChStatus(XFramer *InstancePtr, u16 Channels);
XFramer_Config XFramer_CoreInfo(XFramer *InstancePtr);

/* Low Levels Drivers */
void XFramer_SoftReset(XFramer *InstancePtr);
void XFramer_ChannelUpdate(XFramer *InstancePtr);
void XFramer_ChannelAccess(XFramer *InstancePtr, u16 Channels);
void XFramer_EthSrcAddr0(XFramer *InstancePtr);
void XFramer_EthSrcAddr1(XFramer *InstancePtr);
u16 XFramer_GetEthSrcAddr1(XFramer *InstancePtr);
u32 XFramer_GetEthSrcAddr0(XFramer *InstancePtr);
u8 XFramer_GetPeakBufferLevel(XFramer *InstancePtr);
void XFramer_ClearPeakBufferLevel(XFramer *InstancePtr) ;
u32 XFramer_GetRXPcktCnt(XFramer *InstancePtr) ;
void XFramer_ClearRXPcktCnt(XFramer *InstancePtr);
u32 XFramer_GetDropPcktCnt(XFramer *InstancePtr);
void XFramer_ClearDropPcktCnt(XFramer *InstancePtr);
void XFramer_TransmitEnable(XFramer *InstancePtr, u16 Channels);
u8 XFramer_GetTransmitEnable(XFramer *InstancePtr, u16 Channels);
void XFramer_EthDestAddr0(XFramer *InstancePtr, u16 Channels);
void XFramer_EthDestAddr1(XFramer *InstancePtr, u16 Channels);
void XFramer_VLANID(XFramer *InstancePtr, u16 Channels);
void XFramer_SetVLAN(XFramer *InstancePtr, u16 Channels);
void XFramer_MediaTOS(XFramer *InstancePtr, u16 Channels);
void XFramer_MediaTTL(XFramer *InstancePtr, u16 Channels);
void XFramer_FECTOS(XFramer *InstancePtr, u16 Channels);
void XFramer_FECTTL(XFramer *InstancePtr, u16 Channels);
void XFramer_IP0Dest(XFramer *InstancePtr, u16 Channels);
void XFramer_IP0Src(XFramer *InstancePtr, u16 Channels);
void XFramer_UDPDest(XFramer *InstancePtr, u16 Channels);
void XFramer_UDPSrc(XFramer *InstancePtr, u16 Channels);
u32 XFramer_GetTXPcktCnt(XFramer *InstancePtr, u16 Channels);
void XFramer_ClearTXPcktCnt(XFramer *InstancePtr, u16 Channels);


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
