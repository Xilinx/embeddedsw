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
* @file xvoipfec_tx.h
*
* This is the main header file for VoIP FEC Transmitter core. VoIP FEC
* Transmitter core is used to generate redundant (FEC:ST2022-1 or ST2022-5)
* packets based on user configured setting
*
* The VoIP FEC Transmitter has 4 main Interface
* - AXI4-Stream Interface for accepting Incoming Ethernet Packet
* - AXI4-Stream Interface for transmitting RTP Packet <RTP Header | Payload>
* - AXI4-Lite interface for processor, controls the VoIP FEC Transmitter.
* - AXI-MM interface for External Memory Interface
*
* VoIP FEC Transmitter Performs Following Operation
* - Generate Redundant Packet (FEC:ST2022-1 or ST2022-5)
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do following steps in order for preparing the
* VoIP FEC Transmitter core to be ready.
*
* - Call XVoipFEC_TX_LookupConfig using a device ID to find the core
*  configuration.
* - Call XVoipFEC_TX_CfgInitialize to initialize the device and the driver
*   instance associated with it.
*
* <b> Virtual Memory </b>
*
* <b> Threads </b>
*
* <b> Asserts </b>
*
* <b> Building the driver </b>
*
* The VoIP FEC Transmitter driver is composed of several source files. This
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
#ifndef XVOIPFEC_TX_H_
#define XVOIPFEC_TX_H_ /**< Prevent circular inclusions
                  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xvoipfec_tx_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#define XVOIPFEC_TX_MAX_CHANNEL             XPAR_XVOIPFEC_TX_0_CHANNELS

/**************************** Type Definitions *******************************/

/** @name Non Block Align Components
* @{
*/
/**
* These constants specify different types of FEC Block Align
*/
typedef enum non_block_allign {
    BLOCK_ALLIGN = 0,
    NON_BLOCK_ALLIGN = 1
}XVoipFEC_TX_NonBlockAlign;
/*@}*/

/** @name FEC Mode
* @{
*/
/**
* These constants specify component of FEC Mode Configuration
*/
typedef enum fec_mode {
    FEC_BYPASS = 0,
    FEC_1D = 2,
    FEC_2D = 3
}XVoipFEC_TX_FECMode;
/*@}*/

/**
* This typedef contains FEC Configuration Parameters
*/
typedef struct {
    u16 fec_l;
    u16 fec_d;
    XVoipFEC_TX_NonBlockAlign  non_block_allign;
    XVoipFEC_TX_FECMode        fec_mode;
} XVoipFEC_TX_FECParams;

/**
* This typedef contains FEC Configuration Parameters on Channel Space
*/
typedef struct {
    u16 isEnable;
    XVoipFEC_TX_FECParams fec_params;
} XVoipFEC_TX_ChannelCfg;

/**
* This typedef contains FEC Statistic Components
*/
typedef struct {
    u32 incoming_packet_count;
    u32 outgoing_packet_count;
} XVoipFEC_TX_PktCnt;

/**
* This typedef contains configuration information for the VoIP FEC Transmitter
* core. Each VoIP FEC Transmitter device should have a configuration structure
* associated.
*/
typedef struct {
    u16 DeviceId;
    UINTPTR BaseAddress;              /**< BaseAddress is the physical base
                                      *address of the core's registers */

    /* <Core Generics Info> */
    u16 HWChannelNumber;              /**< Current Supported Channel by the
                                      *core */
    u32 FECBaseAddress;               /**< Configured FEC BaseAddress */
    u16 MaximumFECL;                  /**< Maximum Supported FEC L Value */
} XVoipFEC_TX_Config;

/**
* The XVoipFEC_TX driver instance data. An instance must be allocated for each
* VoIP FEC Transmitter core in use.
*/
typedef struct {
    XVoipFEC_TX_Config   Config;
    u32                  IsReady;

    /* Status */
    u32 FEC_BaseAddr;

    /* Statistic */
    XVoipFEC_TX_PktCnt TX_Pckt_Cnt;

    /* Channel Config */
    XVoipFEC_TX_ChannelCfg ChannelCfg[XVOIPFEC_TX_MAX_CHANNEL];
} XVoipFEC_TX;


/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro Check the VoIP FEC Transmitter Busy Status which indicates
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
#define XVoipFEC_TX_BusyBit(InstancePtr) \
        XVoipFEC_TX_ReadReg((InstancePtr)->Config.BaseAddress, \
             (XVOIPFEC_TX_STATUS_REG_OFFSET)) & \
                  (XVOIPFEC_TX_STATUS_UPDATE_BUSY_MASK)

/************************** Function Prototypes ******************************/

/* Initialization function in xvoipfec_tx_sinit.c */
XVoipFEC_TX_Config *XVoipFEC_TX_LookupConfig(u16 DeviceId);

/* Level 2 Drivers/API */
int XVoipFEC_TX_CfgInitialize(XVoipFEC_TX *InstancePtr,
                                 XVoipFEC_TX_Config *CfgPtr,
                                                         UINTPTR EffectiveAddr);
XVoipFEC_TX_Config XVoipFEC_TX_CoreStatusRegValue(XVoipFEC_TX *InstancePtr);
XVoipFEC_TX_PktCnt XVoipFEC_TX_CoreStatisics(XVoipFEC_TX *InstancePtr);
XVoipFEC_TX_FECParams
          XVoipFEC_TX_FECParamsRegValue(XVoipFEC_TX *InstancePtr, u16 Channels);
void XVoipFEC_TX_SetFECParams(XVoipFEC_TX *InstancePtr, u16 Channels);
void XVoipFEC_TX_CoreChannelConfig(XVoipFEC_TX *InstancePtr);

/* Low Levels Drivers */
void XVoipFEC_TX_SoftReset(XVoipFEC_TX *InstancePtr);
void XVoipFEC_TX_ChannelUpdate(XVoipFEC_TX *InstancePtr);
void XVoipFEC_ChannelAccess(XVoipFEC_TX *InstancePtr, u16 Channels);
void XVoipFEC_TX_SetFEC_D(XVoipFEC_TX *InstancePtr, u16 Channels);
void XVoipFEC_TX_SetFEC_L(XVoipFEC_TX *InstancePtr, u16 Channels);
void XVoipFEC_TX_SetFECMode(XVoipFEC_TX *InstancePtr, u16 Channels);
void XVoipFEC_TX_SetFECNonBlockAllign(XVoipFEC_TX *InstancePtr, u16 Channels);


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
