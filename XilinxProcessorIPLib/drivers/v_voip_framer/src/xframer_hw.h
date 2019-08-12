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
* @file xframer_hw.h
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx VoIP Framer core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver xframer.h file.
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

#ifndef XFRAMER_HW_H_
#define XFRAMER_HW_H_       /**< Prevent circular inclusions
                  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/* VOIP FRAMER REGISTER OFFSETS */

/* GENERAL SPACE OF VOIP FRAMER */
#define XFRAMER_CONTROL                                  ((0*128)+(0*4))
#define XFRAMER_STATUS                                   ((0*128)+(1*4))
#define XFRAMER_CHANNEL_ACCESS                           ((0*128)+(2*4))
#define XFRAMER_SYS_CONFIG                               ((0*128)+(3*4))
#define XFRAMER_VERSION                                  ((0*128)+(4*4))
#define XFRAMER_ETH_SRC_ADDR_LOW                         ((0*128)+(5*4))
#define XFRAMER_ETH_SRC_ADDR_HIGH                        ((0*128)+(6*4))
#define XFRAMER_PEAK_BUF_LEVEL                           ((0*128)+(7*4))
#define XFRAMER_RX_PCKT_CNT                              ((0*128)+(8*4))
#define XFRAMER_DROP_PCKT_CNT                            ((0*128)+(9*4))
#define XFRAMER_STAT_RESET                               ((0*128)+(12*4))

/* VOIP FRAMER CONTROL MASK */
#define XFRAMER_CONTROL_CHANNEL_UPDATE_MASK              0x2
#define XFRAMER_CONTROL_SOFT_RESET_MASK                  0x1
#define XFRAMER_CONTROL_CHANNEL_UPDATE_SHIFT             1

/* VOIP FRAMER STATUS MASK */
#define XFRAMER_STATUS_UPDATE_BUSY_MASK                  0x1

/* VOIP FRAMER CHANNEL ACCESS MASK */
#define XFRAMER_CHANNEL_ACCESS_MASK                      0xFFF

/* VOIP FRAMER SYS CONFIG */
#define XFRAMER_SYS_CONFIG_C_CHANNELS_MASK               0x0000FFF
#define XFRAMER_SYS_CONFIG_C_MAC_DW_MASK                 0x0FF0000
#define XFRAMER_SYS_CONFIG_C_HITLESS_MASK                0x1000000
#define XFRAMER_SYS_CONFIG_C_MAC_DW_SHIFT                16
#define XFRAMER_SYS_CONFIG_C_HITLESS_SHIFT               24

/* VOIP FRAMER VERSION */
#define XFRAMER_VERSION_REVISION_NUMBER_MASK             0xFF
#define XFRAMER_VERSION_PATCH_ID_MASK                    0xF00
#define XFRAMER_VERSION_VERSION_REVISION_MASK            0xF000
#define XFRAMER_VERSION_VERSION_MINOR_MASK               0xFF0000
#define XFRAMER_VERSION_VERSION_MAJOR_MASK               0xFF000000
#define XFRAMER_VERSION_PATCH_ID_SHIFT                   8
#define XFRAMER_VERSION_VERSION_REVISION_SHIFT           12
#define XFRAMER_VERSION_VERSION_MINOR_SHIFT              16
#define XFRAMER_VERSION_VERSION_MAJOR_SHIFT              24

/* VOIP FRAMER ETH SRC ADDR HIGH */
#define XFRAMER_ETH_SRC_ADDR_HIGH_MASK                   0xFFFF

/* XFRAMER_PEAK_BUF_LEVEL */
#define XFRAMER_PEAK_BUF_LEVEL_MASK                      0x3F

/* XFRAMER_STAT_RESET */
#define XFRAMER_STAT_RESET_PEAK_BUFF_LEVEL_MASK          0x1
#define XFRAMER_STAT_RESET_RX_PCKT_CNT_MASK              0x2
#define XFRAMER_STAT_RESET_DROP_PCKT_CNT_MASK            0x4
#define XFRAMER_STAT_RESET_RX_PCKT_CNT_SHIFT             1
#define XFRAMER_STAT_RESET_DROP_PCKT_CNT_SHIFT           2

/* CHANNEL SPACE OF VOIP FRAMER */
#define XFRAMER_CHANNEL_CTRL                             ((1*128)+(0*4))
#define XFRAMER_ETH_DEST_ADDR_LOW                        ((1*128)+(1*4))
#define XFRAMER_ETH_DEST_ADDR_HIGH                       ((1*128)+(2*4))
#define XFRAMER_VLAN_TAG_INFO                            ((1*128)+(3*4))
#define XFRAMER_MEDIA_IP_VER_TOS_TTL                     ((1*128)+(4*4))
#define XFRAMER_FEC_IP_VER_TOS_TTL                       ((1*128)+(5*4))
#define XFRAMER_SRC_IP0                                  ((1*128)+(6*4))
#define XFRAMER_DEST_IP0                                 ((1*128)+(10*4))
#define XFRAMER_SOURCE_UDP_PORT                          ((1*128)+(14*4))
#define XFRAMER_DEST_UDP_PORT                            ((1*128)+(15*4))
#define XFRAMER_TX_PKT_CNT                               ((1*128)+(16*4))
#define XFRAMER_CHAN_STAT_RESET                          ((1*128)+(17*4))

/* VOIP FRAMER CHANNEL CONTROL */
#define XFRAMER_CHANNEL_CTRL_TRANSMIT_ENABLE_MASK        0x1

/* VOIP FRAMER ETH DEST ADDR HIGH */
#define XFRAMER_ETH_DEST_ADDR_HIGH_MASK                  0xFFFF

/* VOIP FRAMER VLAN TAG INFO */
#define XFRAMER_VLAN_TAG_INFO_VLAN_ID_MASK               0xFFFF
#define XFRAMER_VLAN_TAG_INFO_WITH_VLAN_MASK             0x80000000
#define XFRAMER_VLAN_TAG_INFO_WITH_VLAN_SHIFT            31

/* XFRAMER_MEDIA_IP_VER_TOS_TTL & XFRAMER_FEC_IP_VER_TOS_TTL */
#define XFRAMER_IP_VER_TOS_TTL_TTL_MASK                  0xFF
#define XFRAMER_IP_VER_TOS_TTL_TOS_MASK                  0xFF00
#define XFRAMER_IP_VER_TOS_TTL_TOS_SHIFT                 8

/* XFRAMER_SOURCE_UDP_PORT */
#define XFRAMER_SOURCE_UDP_PORT_MASK                     0xFFFF

/* XFRAMER_DEST_UDP_PORT */
#define XFRAMER_DEST_UDP_PORT_MASK                       0xFFFF

/* XFRAMER_CHAN_STAT_RESET_PORT */
#define XFRAMER_CHAN_STAT_RESET_TX_PKT_CNT_MASK          0x1



/**************************** Type Definitions *******************************/
#define XFRAMER_ZEROES                                   0x0

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XFramer_In32        Xil_In32    /**< Input Operations */
#define XFramer_Out32       Xil_Out32   /**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a VoIP Framer register. A 32 bit read is
* performed. If the component is implemented in a smaller width, only the least
* significant data is read from the register. The most significant data
* will be read as 0.
*
* @param    BaseAddress is the base address of the VoIP Framer core instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file).
*
* @return   The 32-bit value of the register.
*
* @note     C-style signature:
*       u32 XFramer_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XFramer_ReadReg(BaseAddress, RegOffset) \
    XFramer_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a VoIP Framer register. A 32 bit write is
* performed. If the component is implemented in a smaller width, only the least
* significant data is written.
*
* @param    BaseAddress is the base address of the VoIP Framer core instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file) to be written.
* @param    Data is the 32-bit value to write into the register.
*
* @return   None.
*
* @note     C-style signature:
*       void XFramer_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XFramer_WriteReg(BaseAddress, RegOffset, Data) \
    XFramer_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
