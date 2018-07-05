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
* @file xvoipfec_rx.h
*
* This is the main header file for VoIP FEC Receiver core. VoIP FEC Receiver
* core is used for ST2022-6 Packet Recovery.
*
* The VoIP FEC Receiver has 6 main Interface
* - AXI4-Stream Interface for accepting Incoming Ethernet Packet
* - AXI4-Stream Interface for transmitting RTP Packet <RTP Header | Payload>
* - AXI4-Lite interface for processor, controls the VoIP FEC Receiver.
* - AXI-MM interface for External Memory Interface
* - AXI4-Stream Interface to transmit Event Status of the Core
* - AXI4-Stream Interface to receive Packet Request from the Core.
*
* VoIP FEC Receiver Performs Following Operation
* - Recovers ST2022-6 Packet Loss based on ST2022-5 Packets
* - Recovers Network Loss based on ST2022-7
* - Buffers and Re-Order Packet based on the Incoming RTP Sequence Number
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* VoIP FEC Receiver core to be ready.
*
* - Call XVoipFEC_RX_LookupConfig using a device ID to find the core
*  configuration.
* - Call XVoipFEC_RX_CfgInitialize to initialize the device and the driver
*  instance associated with it.
*
* <b> Virtual Memory </b>
*
* <b> Threads </b>
*
* <b> Asserts </b>
*
* <b> Building the driver </b>
*
* The VoIP FEC Receiver driver is composed of several source files. This
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
#ifndef XVOIPFEC_RX_H_
#define XVOIPFEC_RX_H_ /**< Prevent circular inclusions
                  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xvoipfec_rx_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xdebug.h"

/************************** Constant Definitions *****************************/
#define XVOIPFEC_RX_MAX_CHANNEL  XPAR_XVOIPFEC_RX_0_NUM_OF_CHANNELS

/**************************** Type Definitions *******************************/

/** @name VoIP FEC Receiver Channel Enable
* @{
*/
/**
* These constants specify FEC Receiver Channel Enable/Disable
*/
typedef enum channel_disable {
    XVOIPFEC_RX_CHANNEL_DISABLE = 0,
    XVOIPFEC_RX_CHANNEL_ENABLE = 1
}XVoipFEC_TX_ChannelEnable;
/*@}*/

/** @name ST2022-1/5 FEC Recovery Enable or Disable
* @{
*/
/**
* These constants specify the Recovery Enable/Disable
*/
typedef enum fec_recv_disable {
    XVOIPFEC_RX_FEC_RECOVERY_ENABLE = 0,
    XVOIPFEC_RX_FEC_RECOVERY_DISABLE = 1
}XVoipFEC_TX_FECRecoveryDisable;
/*@}*/

/** @name Incoming Packet Processing Type
* @{
*/
/**
* These constants specify the operating channel Enable/Disable
*/
typedef enum media_packet_bypass {
    XVOIPFEC_RX_MEDIA_PACKET_PROCESSED = 0,
    XVOIPFEC_RX_MEDIA_PACKET_BYPASS = 1
}XVoipFEC_TX_MediaPacketBypass;
/*@}*/

/** @name OOR Operation Selection
* @{
*/
/**
* These constants specify the OOR Operating Selection
*/
typedef enum oor_selection {
    XVOIPFEC_RX_OOR_BASED_ON_RTP_SEQ_NUMBER = 0,
    XVOIPFEC_RX_OOR_BASED_ON_RTP_TS = 1
}XVoipFEC_TX_OORModeSelection;
/*@}*/

/**
* This typedef contains VoIP FEC Receiver Channel Parameters
*/
typedef struct {
    XVoipFEC_TX_ChannelEnable      Channel_Enable;
    XVoipFEC_TX_MediaPacketBypass  Media_Packet_Bypass;
    XVoipFEC_TX_FECRecoveryDisable FEC_Recovery_Disable;
} XVoipFEC_RX_FECParams;

/**
* This typedef contains VoIP FEC Receiver Channel Statistic
*/
typedef struct {
    u32 valid_pkts_cnt;
    u32 unrecv_pkts_cnt;
    u32 corr_pkts_cnt;
    u32 dup_pkts_cnt;
    u32 oor_pkts_cnt;
} XVoipFEC_RX_FECStatistics;

/**
* This typedef contains VoIP FEC Receiver and ST2022-1/5 Status
*/
typedef struct {
    u8 Row_Detected;
    u8 Col_Detected;
    u16 FEC_L;
    u16 FEC_D;
    u16 Buffer_Depth;
} XVoipFEC_RX_FECStatus;

/**
* This typedef contains VoIP FEC Receiver Channel ST2022-7/Seamless Status
*/
typedef struct {
    u32 RTP_TS_Diff;              /**< RTP Timestamp Difference between
                                  * 2 Identical Sequence Number Packet from
                                  * 2 Link */
} XVoipFEC_RX_HitlessStatus;

/**
* This typedef contains VoIP FEC Receiver Channel OOR Management
*/
typedef struct {
    XVoipFEC_TX_OORModeSelection  OORTimestampEnable;  /**< If Disable OOR
                                                       * Based On Sequence
                                                       * Number */
    u32 RTPTimestampWindow;
} XVoipFEC_RX_OORMgmnt;

/**
* This typedef contains VoIP FEC Receiver Channel Memory Configuration
* or Information
*/
typedef struct {
    u32 RTPCh_BaseAddress;
    u32 RTPCh_MapBaseAddress;
    u16 RTPCh_BufferDepth;
}XVoipFEC_RX_MemoryCfg;

/**
* This typedef contains VoIP FEC Receiver Channel Configuration/Information
*/
typedef struct {
    /* Configurable Parameter */
    XVoipFEC_RX_MemoryCfg      RTPMemoryCfg;
    XVoipFEC_RX_FECParams      FECParams;
    XVoipFEC_RX_OORMgmnt       OORMgmt;

    /* Status/Statistic Monitoring */
    XVoipFEC_RX_FECStatus      FECStatus;
    XVoipFEC_RX_FECStatistics  FECStatistic;
    XVoipFEC_RX_HitlessStatus  HitlessStatus;
} XVoipFEC_RX_ChannelCfg;

/**
* This typedef contains VoIP FEC Receiver Error Status of Bufer
* Full Information
*/
typedef struct {
    u8 Request_Que_Ov;
    u8 Event_Que_Ov;
    u8 Buffer_Level_Update_Que_Ov;
    u8 Primary_Input_Buffer_Que_Ov;
    u8 Secondary_Input_Buffer_Que_Ov;
    u8 Output_Buffer_Que_Ov;
}XVoipFEC_RX_GenErrStatus;

/**
* This typedef contains configuration information for the Generic Decap core.
* Each Generic Decap device should have a configuration structure associated.
*/
typedef struct {
    u16 DeviceId;      /**< DeviceId is the unique ID of the VoIP FEC Receiver
                       * core */
    UINTPTR BaseAddress;/**< BaseAddress is the physical base address of the
                       * core's registers */

    /*<Core Generics Info>*/
    u16 HWChannelNumber;             /**< Current Supported Channel by the
                                     * core */
    u32 MediaBaseAddress;            /**< Media Packet Buffer Base Address */
    u32 MediaBufferDepthPerChannel;  /**< Media Packet Buffer Depth Per
                                     * Channel */
    u32 FECBaseAddress;              /**< FEC Packet Buffer Base Address */
    u32 FECBufferDepth;              /**< FEC Packet Buffer Depth */
    u8  FECEnable;                   /**< ST2022-1/5/FEC Recovery Enable */
    u8  SeamlessEnable;              /**< ST2022-7/Seamless Switching Enable */
    u8  ProgMemoryEnable;            /**< Configurable External Memory Base
                                     * Address & Buffer Depth Enable */
} XVoipFEC_RX_Config;

/**
* The XVoipFEC_RX driver instance data. An instance must be allocated for each
* VoIP FEC Receiver core in use.
*/
typedef struct {
    XVoipFEC_RX_Config   Config;
    u32                  IsReady;

    /* General Space Register */
    u32 FECProcessingDelay;

    /* General Space Statistic */
    u32 FECPcktDropCnt;
    u32 FECBuffPeakDataCount;
    XVoipFEC_RX_GenErrStatus FECGenErrStatus;

    /* General Space Memory Cfg */
    u32 FECBaseAddress;
    u32 FECBufferDepth;

   /* Current System Operational Number of Channel */
   u16 OpChannel;

    /* Channel Config */
    XVoipFEC_RX_ChannelCfg ChannelCfg[XVOIPFEC_RX_MAX_CHANNEL];
} XVoipFEC_RX;


/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro Check the VoIP FEC Receiver Busy Status which indicates
* the core is still updating the registers.
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @return   BusyBitStatus.
*
* @note
*
*
*       void XVoipFEC_RX_AssertSoftReset(XVoipFEC_RX *InstancePtr)
*
******************************************************************************/
#define XVoipFEC_RX_BusyBit(InstancePtr) \
        XVoipFEC_RX_ReadReg((InstancePtr)->Config.BaseAddress, \
            (XVOIPFEC_RX_STATUS_REG_OFFSET)) & \
                  (XVOIPFEC_RX_STATUS_UPDATE_BUSY_MASK)

/*****************************************************************************/
/**
*
* This macro Check the Maximum Supported Channel by the VoIP FEC Receiver Core.
*
* @param    InstancePtr is a pointer to the XVoipFEC_RX core instance.
*
* @return   MaximumNumberOfChannelsupported.
*
* @note
*
*
*       void XVoipFEC_RX_NumSupportedChannel(XVoipFEC_RX *InstancePtr)
*
******************************************************************************/
#define XVoipFEC_RX_NumSupportedChannel(InstancePtr) \
        XVoipFEC_RX_ReadReg((InstancePtr)->Config.BaseAddress, \
             (XVOIPFEC_RX_SYSTEM_CONFIG_REG_OFFSET)) & \
                   (XVOIPFEC_RX_SYS_CONFIG_C_CHANNELS_MASK)



/************************** Function Prototypes ******************************/

/* Initialization function in xvoipfec_rx_sinit.c */
XVoipFEC_RX_Config *XVoipFEC_RX_LookupConfig(u16 DeviceId);

/* Level 2 Drivers */
int XVoipFEC_RX_CfgInitialize(XVoipFEC_RX *InstancePtr,
                       XVoipFEC_RX_Config *CfgPtr, UINTPTR EffectiveAddr);

XVoipFEC_RX_Config XVoipFEC_RX_CoreInfo(XVoipFEC_RX *InstancePtr);
XVoipFEC_RX_FECStatus XVoipFEC_RX_FECStatusRegValue
                             (XVoipFEC_RX *InstancePtr, u16 Channels);
XVoipFEC_RX_FECStatistics XVoipFEC_RX_FECStatisticsRegValue
                             (XVoipFEC_RX *InstancePtr, u16 Channels);
XVoipFEC_RX_FECParams XVoipFEC_RX_FECParamsRegValue
                             (XVoipFEC_RX *InstancePtr, u16 Channels);
XVoipFEC_RX_HitlessStatus XVoipFEC_HitlessStatusRegValue
                             (XVoipFEC_RX *InstancePtr, u16 Channels);
void XVoipFEC_RX_CoreChannelConfig(XVoipFEC_RX *InstancePtr);
void XVoipFEC_RX_SetFECParams(XVoipFEC_RX *InstancePtr, u16 Channels);
void XVoipFEC_RX_ClearChannel(XVoipFEC_RX *InstancePtr, u16 Channels);


/* Low Levels Drivers */
void XVoipFEC_RX_SoftReset(XVoipFEC_RX *InstancePtr);
void XVoipFEC_RX_ChannelUpdate(XVoipFEC_RX *InstancePtr);
void XVoipFEC_RX_ChannelClear(XVoipFEC_RX *InstancePtr);
void XVoipFEC_RX_ChannelAccess(XVoipFEC_RX *InstancePtr, u16 Channels);
void XVoipFEC_RX_SetFECProcessingDelay(XVoipFEC_RX *InstancePtr);
u32 XVoipFEC_RX_GetFECProcessingDelay(XVoipFEC_RX *InstancePtr);
u32 XVoipFEC_RX_GetFECPacketDropCnt(XVoipFEC_RX *InstancePtr);
u32 XVoipFEC_RX_GetFECBufferPeakDataCnt(XVoipFEC_RX *InstancePtr);
void XVoipFEC_RX_ChannelEnable(XVoipFEC_RX *InstancePtr, u16 Channels);
void XVoipFEC_RX_MediaPacketBypass(XVoipFEC_RX *InstancePtr, u16 Channels);
void XVoipFEC_RX_FECRecoveryDisable(XVoipFEC_RX *InstancePtr, u16 Channels);
void XVoipFEC_RX_OORCfg(XVoipFEC_RX *InstancePtr, u16 Channels);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
