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
* @file xpacketizer56.h
*
* This is the main header file for VoIP ST2022-6 Packetizer core. VoIP ST2022-6
* Packetizer core is used for Packetized incoming SDI to AXI4-Stream in to
* ST2022-6 compliant RTP Packet
*
* The VoIP ST2022-6 Packetizer has 3 main Interface
* - AXI4-Stream Interface for accepting Incoming SDI to AXI4-Stream Packet
* - AXI4-Stream Interface for transmitting RTP Packet <RTP Header | Payload>
* - AXI4-Lite interface for processor, controls the VoIP ST2022-6 Packetizer.
*
* VoIP ST2022-6 Packetizer Performs Following Operation
* - Packetized SDI to AXI4-Stream Packet into ST2022-6 RTP Packet
* - Add user configured RTP Header into RTP Packet Header
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* VoIP ST2022-6 Packetizer core to be ready.
*
* - Call XPacketizer56_LookupConfig using a device ID to find the core
*   configuration.
* - Call XPacketizer56_CfgInitialize to initialize the device and the driver
*   instance associated with it.
*
* This driver provides interrupt handlers
* - XPacketizer56_IntrHandler, for handling the interrupts from the VoIP
*   ST2022-6 Packetizer core
*
* Application developer needs to register interrupt handler with the processor,
* within their examples. Whenever processor calls registered application's
* interrupt handler associated with interrupt id, application's interrupt
* handler needs to call appropriate peripheral interrupt handler reading
* peripherals Status register.

* This driver provides XPacketizer56_SetCallback API to register functions with
* VoIP ST2022-6 Packetizer core instance.
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
* The VoIP ST2022-6 Packetizer driver is composed of several source files. This
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

#ifndef XPACKETIZER56_H_
#define XPACKETIZER56_H_ /**< Prevent circular inclusions
                               *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xpacketizer56_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xdebug.h"

/************************** Constant Definitions *****************************/

/** @name Handler Types
* @{
*/
/**
* These constants specify different types of handler and used to differentiate
* interrupt requests from peripheral.
*/
typedef enum {
    XPACKETIZER56_HANDLER_CONNECT = 1,
    XPACKETIZER56_HANDLER_DATAGRAM_PER_FRAME_MISMATCH
}XPacketizer56_HandlerType;
/*@}*/

/** @name Timestamp Reference (Video Header)
* @{
*/
/**
* These constants specify specify the VoIP ST2022-6 Packetizer Timestamp
* Reference in the Video Header
*/
typedef enum {
    XPACKETIZER56_NOT_LOCKED = 0,
    XPACKETIZER56_RESERVED = 1,
    XPACKETIZER56_LOCKED_TO_UTC_TIME = 2,
    XPACKETIZER56_LOCKED_TO_PRIVATE_TIME = 3
}XPacketizer56_TimestampRef;
/*@}*/

/** @name Video Bit Rate
* @{
*/
/**
* These constants specify specify the VoIP ST2022-6 Packetizer Video Bit Rate
* either is an Integer (148.5 MHz or 74.25 MHz) or Non-Integer (148.35 MHz or
* 74.175 MHz)
*/
typedef enum {
    XPACKETIZER56_INTEGER = 0,
    XPACKETIZER56_NON_INTEGER = 1
}XPacketizer56_RXBITRATE;
/*@}*/

/** @name 3G-SDI Level
* @{
*/
/**
* These constants specify specify the VoIP ST2022-6 Packetizer Incoming
* SDI to AXI4-Stream is a SDI-3G Level A or Level B
*/
typedef enum {
    XPACKETIZER56_3G_LEVEL_A = 0,
    XPACKETIZER56_3G_LEVEL_B = 1
}XPacketizer56_3G_LEVEL;
/*@}*/

/** @name SDI Mode
* @{
*/
/**
* These constants specify specify the VoIP ST2022-6 Packetizer Incoming
* SDI to AXI4-Stream SDI Mode, SD-SDI or HD-SDI or 3G-SDI
*/
typedef enum {
    XPACKETIZER56_SDI_SD = 1,
    XPACKETIZER56_SDI_HD = 0,
    XPACKETIZER56_SDI_3G = 3
}XPacketizer56_SDIMode;
/*@}*/

/** @name ST2022-6 Packetizer Module Enable/Disable
* @{
*/
/**
* These constants specify specify the VoIP ST2022-6 Packetizer Module Enable or
* Disable.
*/
typedef enum {
    XPACKETIZER56_MODULE_DISABLE = 0,
    XPACKETIZER56_MODULE_ENABLE  = 1
}XPacketizer56_ModEn;
/*@}*/

/** @name ST2022-6 Packetizer Operating Mode
* @{
*/
/**
* These constants specify specify the VoIP ST2022-6 Packetizer Operating Mode,
* Normal Mode or Lossless Mode.
*/
typedef enum {
    XPACKETIZER56_NORMAL = 0,
    XPACKETIZER56_LOSSLESS  = 1
}XPacketizer56_LosslessMode;
/*@}*/

/**
* Callback type for VoIP ST2022-6 Packetizer event interrupt.
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
typedef void (*XPacketizer56_Callback)(void *CallbackRef);

/**************************** Type Definitions *******************************/
/**
* This typedef contains configuration information for the VoIP ST2022-6
* Packetizer core. Each VoIP Decapsulator device should have a configuration
* structure associated.
*/
typedef struct {
    u16 DeviceId;
    UINTPTR BaseAddress;
}XPacketizer56_Config;

/**
* This typedef contains Incoming SDI to AXI4-Stream Information
*/
typedef struct {
    u8 IsConnected;

    /* Datagram Per Frame */
    u16 datagram_per_frame;

    /* Last Datagram Length */
    u16 last_datagram_len;

    /* RTP Header - SSRC */
    u32 ssrc;

    /* Timestamp Reference */
    XPacketizer56_TimestampRef ts_ref;

    /* Video TS Include */
    u8 video_ts_include;

    /*Media Payload Length */
    u16 media_payload_length;

    /* Video Info */
    XPacketizer56_RXBITRATE rx_bitrate;
    XPacketizer56_SDIMode   sdi_mode;
    XPacketizer56_3G_LEVEL  level_b_3g;

    /* MAP_FRAME_FRATE_SAMPLE_FMT */
    u8 MAP;
    u8 FRAME;
    u8 FRATE;
    u8 SAMPLE;
}XPacketizer56_Stream;

/**
* This typedef contains Incoming Frame Count and Outgoing Packet
* Count Information
*/
typedef struct {
    u32 output_pkt_cnt;
    u32 sdi_frame_cnt;
}XPacketizer56_ModuleStatistic;

/**
* The VoIP ST2022-6 Packetizer driver instance data. An instance must be
* allocated for each VoIP ST2022-6 Packetizer core in use.
*/
typedef struct {
    XPacketizer56_Config        Config;
    u32                         IsReady;

    /*Interrupt*/
    u32 DatagramperFrameMismatchInterrupt;

    /*Callbacks */
    XPacketizer56_Callback DatagramMismatchCallback; /**< Callback for error
                                                     ***status: Datagram per
                                                     ***frame mismatch event
                                                     ***interrupt */
    void *DatagramMismatchRef;                       /**< To be passed to the
                                                     ***connect interrupt
                                                     ***callback */
    u32 IsDatagramMismatchCallbackSet;               /**< Set flag. This flag
                                                     ***is set to true when the
                                                     ***callback has been
                                                     ***registered */

    /*SDI Stream Information*/
    XPacketizer56_Stream *Stream;

    /* Module Statistic */
    XPacketizer56_ModuleStatistic Statistic;

    /*Channel Number*/
    u16  Channel_Number;

    /*Lossless Mode*/
    XPacketizer56_LosslessMode LoslessMode;

    /*Module Enable*/
    XPacketizer56_ModEn ModuleEnable;

} XPacketizer56;

/***************** Macros (Inline Functions) Definitions *********************/
XPacketizer56_Config *XPacketizer56_LookupConfig(u16 DeviceId);

int XPacketizer56_CfgInitialize(XPacketizer56 *InstancePtr,
                               XPacketizer56_Config *CfgPtr,
                                       UINTPTR EffectiveAddr);

// INterrupt Functions
void XPacketizer56_IntrHandler(void *InstancePtr);
int XPacketizer56_SetCallback(XPacketizer56 *InstancePtr,
                             XPacketizer56_HandlerType HandlerType,
                             void *CallbackFunc, void *CallbackRef);
XPacketizer56_Stream
                    XPacketizer56_MediaDatagramInfo(XPacketizer56 *InstancePtr);

//Low Level Drivers
void XPacketizer56_ModEnable (XPacketizer56 *InstancePtr);
void XPacketizer56_LosslessEnable (XPacketizer56 *InstancePtr);
void XPacketizer56_RegClear(XPacketizer56 *InstancePtr);
void XPacketizer56_ModStatus (XPacketizer56 *InstancePtr);
void XPacketizer56_VidFormat (XPacketizer56 *InstancePtr);
void XPacketizer56_RTPMediaHeader (XPacketizer56 *InstancePtr);
void XPacketizer56_SetChannel (XPacketizer56 *InstancePtr);
void XPacketizer56_SetSSRC (XPacketizer56 *InstancePtr);


#endif /* XPACKETIZER56_H_ */
