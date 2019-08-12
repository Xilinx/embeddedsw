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
* @file XDepacketizer56.h
*
* This is the main header file for VoIP ST2022-6 Depacketizer core. VoIP
* ST2022-6 Depacketizer core is used to map Incoming ST2022-5 RTP Packet in to
* SDI to AXI4-Stream Packet.
*
* The VoIP ST2022-6 Depacketizer has 3 main Interface
* - AXI4-Stream Interface for accepting Incoming SDI to AXI4-Stream Packet
* - AXI4-Stream Interface for transmitting RTP Packet <RTP Header | Payload>
* - AXI4-Lite interface for processor, controls the VoIP ST2022-6 Depacketizer.
*
* VoIP ST2022-6 Depacketizer Performs Following Operation
* - Map/Convert ST2022-5 RTP Packet in to SDI to AXI4-Stream Packet
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* VoIP ST2022-6 Depacketizer core to be ready.
*
* - Call XDepacketizer56_LookupConfig using a device ID to find the core
*  configuration.
* - Call XDepacketizer56_CfgInitialize to initialize the device and the driver
*  instance associated with it.
*
* This driver provides interrupt handlers
* - XDepacketizer56_IntrHandler, for handling the interrupts from the VoIP
*   ST2022-6 Depacketizer core
*
* Application developer needs to register interrupt handler with the processor,
* within their examples. Whenever processor calls registered application's
* interrupt handler associated with interrupt id, application's interrupt
* handler needs to call appropriate peripheral interrupt handler reading
* peripherals Status register.

* This driver provides XDepacketizer56_SetCallback API to register functions
* with VoIP ST2022-6 Depacketizer core instance.
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
* The VoIP ST2022-6 Depacketizer driver is composed of several source files.
* This allows the user to build and link only those parts of the driver that
* are necessary.
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

#ifndef XDEPACKETIZER56_H_
#define XDEPACKETIZER56_H_ /**< Prevent circular inclusions
                               *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xdepacketizer56_hw.h"
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
    XDEPACKETIZER56_HANDLER_CONNECT = 1,
    XDEPACKETIZER56_HANDLER_DATAGRAM_PER_FRAME_MISMATCH,
    XDEPACKETIZER56_HANDLER_BUFFER_EMPTY
}XDepacketizer56_HandlerType;
/*@}*/

/** @name Timestamp Reference (Video Header)
* @{
*/
/**
* These constants specify the VoIP ST2022-6 Depacketizer Timestamp
* Reference in the Video Header
*/
typedef enum {
    XDEPACKETIZER56_NOT_LOCKED = 0,
    XDEPACKETIZER56_RESERVED = 1,
    XDEPACKETIZER56_LOCKED_TO_UTC_TIME = 2,
    XDEPACKETIZER56_LOCKED_TO_PRIVATE_TIME = 3
}XDepacketizer56_TimestampRef;
/*@}*/

/** @name Video Bit Rate
* @{
*/
/**
* These constants specify the VoIP ST2022-6 Depacketizer Video Bit Rate
* either is an Integer (148.5 MHz or 74.25 MHz) or Non-Integer (148.35 MHz or
* 74.175 MHz)
*/
typedef enum {
    XDEPACKETIZER56_INTEGER = 0,
    XDEPACKETIZER56_NON_INTEGER = 1
}XDepacketizer56_RXBITRATE;
/*@}*/

/** @name 3G-SDI Level
* @{
*/
/**
* These constants specify the VoIP ST2022-6 Depacketizer Incoming
* ST2022-6 RTP Packet is a SDI-3G Level A or Level B
*/
typedef enum {
    XDEPACKETIZER56_LEVEL_A = 0,
    XDEPACKETIZER56_LEVEL_B = 1
}XDepacketizer56_3G_LEVEL;
/*@}*/

/** @name SDI Mode
* @{
*/
/**
* These constants specify the VoIP ST2022-6 Depacketizer Incoming
* ST2022-6 RTP Packet is, SD-SDI or HD-SDI or 3G-SDI
*/
typedef enum {
    XDEPACKETIZER56_SDI_SD = 1,
    XDEPACKETIZER56_SDI_HD = 0,
    XDEPACKETIZER56_SDI_3G = 3
}XDepacketizer56_SDIMode;
/*@}*/

/** @name Frame Config
* @{
*/
/**
* These constants specify the VoIP ST2022-6 Depacketizer to obtained incoming
* ST2022-6 RTP Packet Frame Information either from TUSER Slave AXI4-Stream
* of VoIP ST2022-6 Depacketizer or User Configure Register.
*/
typedef enum {
    XDEPACKETIZER56_DECODE_FROM_VID_SRC_FMT  = 0,
    XDEPACKETIZER56_USER_CFG_FRAME_SETTING = 1
}XDepacketizer56_FrameCfg;
/*@}*/

/** @name ST2022-6 Depacketizer Module Enable/Disable
* @{
*/
/**
* These constants specify the VoIP ST2022-6 Depacketizer Module Enable
* or Disable.
*/
typedef enum {
    XDEPACKETIZER56_MODULE_DISABLE = 0,
    XDEPACKETIZER56_MODULE_ENABLE  = 1
}XDepacketizer56_ModEn;

/**
* Callback type for VoIP ST2022-6 Depacketizer event interrupt.
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
typedef void (*XDepacketizer56_Callback)(void *CallbackRef);

/**************************** Type Definitions *******************************/
/**
* This typedef contains configuration information for the VoIP ST2022-6
* Depacketizer core. Each VoIP Depacketizer device should have a configuration
* structure associated.
*/
typedef struct {
    u16 DeviceId;
    UINTPTR BaseAddress;
}XDepacketizer56_Config;

/**
* This typedef contains Incoming SDI to AXI4-Stream Information
*/
typedef struct {
    u8 IsConnected;

    /* Datagram Per Frame */
    u16 datagram_per_frame;

    /* Last Datagram Length */
    u16 last_datagram_len;

    /* User Configure Frame details */
    XDepacketizer56_FrameCfg FrameCfg;

    /* Video TS Include */
    u8 video_ts_include;

    /*Media Payload Length */
    u16 media_payload_length;

    /* Video Info */
    XDepacketizer56_RXBITRATE rx_bitrate;
    XDepacketizer56_SDIMode   sdi_mode;
    XDepacketizer56_3G_LEVEL  level_b_3g;

    /* MAP_FRAME_FRATE_SAMPLE_FMT */
    u8 MAP;
    u8 FRAME;
    u8 FRATE;
    u8 SAMPLE;
}XDepacketizer56_Stream;

/**
* This typedef contains Incoming Packet Count, Outgoing Frame
* Count Information, Current Buffer Level and Observed Peak Buffer Level
*/
typedef struct {
    u8  peak_buf_level;
    u8  curr_buf_level;
    u32 input_pkt_cnt;
    u32 sdi_tx_frame_cnt;
}XDepacketizer56_ModuleStatistic;

/**
* This typedef contains the Module Status Component
*/
typedef struct {
    u16 ErrDetected_Datagram_per_Frame;
    u16 StatBuffer_Level;
}XDepacketizer56_ModuleStatus;

/**
* The VoIP ST2022-6 Depacketizer driver instance data. An instance must be
* allocated for each VoIP ST2022-6 Depacketizer core in use.
*/
typedef struct {
    XDepacketizer56_Config        Config;
    u32                           IsReady;

    /*Interrupt*/
    u8 DatagramperFrameMismatchInterrupt;
    u8 BufferEmptyInterrupt;

    /*Callbacks */
    XDepacketizer56_Callback DatagramMismatchCallback;  /**< Callback for error
                                                        * status: Datagram per
                                                        * frame mismatch event
                                                        interrupt */
    void *DatagramMismatchRef;                          /**< To be passed to
                                                        * the connect interrupt
                                                        * callback */
    u32 IsDatagramMismatchCallbackSet;                  /**< Set flag. This
                                                        * flag is set to true
                                                        * when the callback has
                                                        * been registered */

    XDepacketizer56_Callback BufferEmptyCallback;       /**< Callback for error
                                                        * status: Buffer Empty
                                                        * event interrupt */
    void *BufferEmptyRef;                               /**< To be passed to
                                                        * the connect interrupt
                                                        * callback */
    u32 IsBufferEmptyCallbackSet;                       /**< Set flag. This
                                                        * flag is set to true
                                                        * when the callback has
                                                        * been registered */

    /* Module Enable */
    XDepacketizer56_ModEn ModuleEnable;

    /* SDI Stream Information */
    XDepacketizer56_Stream Stream;

    /* Module Status */
    XDepacketizer56_ModuleStatus Status;

    /* Module Statistic */
    XDepacketizer56_ModuleStatistic Statistics;

} XDepacketizer56;

/***************** Macros (Inline Functions) Definitions *********************/
XDepacketizer56_Config *XDepacketizer56_LookupConfig(u16 DeviceId);

/* Top Level API */
int XDepacketizer56_CfgInitialize(XDepacketizer56 *InstancePtr,
                                     XDepacketizer56_Config *CfgPtr,
                                                   UINTPTR EffectiveAddr);
XDepacketizer56_ModuleStatus
                     XDepacketizer56_ErroStatus (XDepacketizer56 *InstancePtr);

/* Interrupt Drivers */
void XDepacketizer56_IntrHandler(void *InstancePtr);
int XDepacketizer56_SetCallback(XDepacketizer56 *InstancePtr,
                         XDepacketizer56_HandlerType HandlerType,
                                        void *CallbackFunc, void *CallbackRef);
XDepacketizer56_ModuleStatistic
         XDepacketizer56_ModuleStatisticRegValue(XDepacketizer56 *InstancePtr);

/* Low Level Drivers */
void XDepacketizer56_ModEnable (XDepacketizer56 *InstancePtr);
void XDepacketizer56_RegClear(XDepacketizer56 *InstancePtr);
void XDepacketizer56_VidFormat (XDepacketizer56 *InstancePtr);
void XDepacketizer56_RTPMediaHeader (XDepacketizer56 *InstancePtr);
void XDepacketizer56_ResetStatistic (XDepacketizer56 *InstancePtr);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
