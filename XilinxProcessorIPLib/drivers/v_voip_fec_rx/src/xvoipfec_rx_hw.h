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
* @file xvoipfec_rx_hw.h
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx VoIP FEC Receiver core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver xvoipfec_rx.h
* file.
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

#ifndef XVOIPFEC_RX_H   /* prevent circular inclusions */
#define XVOIPFEC_RX_H   /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/* General Space Register Offsets */
#define XVOIPFEC_RX_CONTROL_REG_OFFSET                       ((0*128)+(0*4))
#define XVOIPFEC_RX_STATUS_REG_OFFSET                        ((0*128)+(1*4))
#define XVOIPFEC_RX_CHANNEL_ACCESS_REG_OFFSET                ((0*128)+(2*4))
#define XVOIPFEC_RX_SYSTEM_CONFIG_REG_OFFSET                 ((0*128)+(3*4))
#define XVOIPFEC_RX_HWVERSION_REG_OFFSET                     ((0*128)+(4*4))
#define XVOIPFEC_RX_SYSTEM_STATUS_REG_OFFSET                 ((0*128)+(5*4))
#define XVOIPFEC_RX_FEC_BASEADDRESS_REG_OFFSET               ((0*128)+(6*4))
#define XVOIPFEC_RX_FEC_PROCESSING_DELAY_REG_OFFSET          ((0*128)+(8*4))
#define XVOIPFEC_RX_FEC_PACKET_DROP_REG_OFFSET               ((0*128)+(9*4))
#define XVOIPFEC_RX_FEC_PEAK_DATA_CNT_REG_OFFSET             ((0*128)+(10*4))

/* VOIPFEC_RX CONTROL MASK */
#define XVOIPFEC_RX_CONTROL_CHANNEL_CLEAR_MASK               0x4
#define XVOIPFEC_RX_CONTROL_CHANNEL_UPDATE_MASK              0x2
#define XVOIPFEC_RX_CONTROL_SOFT_RESET_MASK                  0x1
#define XVOIPFEC_RX_CONTROL_CHANNEL_UPDATE_SHIFT             1
#define XVOIPFEC_RX_CONTROL_CHANNEL_CLEAR_SHIFT              2

/* VOIPFEC_RX STATUS MASK */
#define XVOIPFEC_RX_STATUS_UPDATE_BUSY_MASK                  0x1

/* VOIPFEC_RX CHANNEL ACCESS MASK */
#define XVOIPFEC_RX_CHANNEL_ACCESS_MASK                      0xFFF

/* VOIPFEC_RX SYS CONFIG */
#define XVOIPFEC_RX_SYS_CONFIG_C_CHANNELS_MASK               0xFFF
#define XVOIPFEC_RX_SYS_CONFIG_FEC_RECOVERY_SUPP_MASK        0x1000
#define XVOIPFEC_RX_SYS_CONFIG_SEAMLESS_SWITCHING_SUPP_MASK  0x2000
#define XVOIPFEC_RX_SYS_CONFIG_FEC_RECOVERY_SUPP_SHIFT       16
#define XVOIPFEC_RX_SYS_CONFIG_SEAMLESS_SWITCHING_SUPP_SHIFT 17

/* VOIPFEC_RX VERSION */
#define XVOIPFEC_RX_VERSION_REVISION_NUMBER_MASK             0xFF
#define XVOIPFEC_RX_VERSION_PATCH_ID_MASK                    0xF00
#define XVOIPFEC_RX_VERSION_VERSION_REVISION_MASK            0xF000
#define XVOIPFEC_RX_VERSION_VERSION_MINOR_MASK               0xFF0000
#define XVOIPFEC_RX_VERSION_VERSION_MAJOR_MASK               0xFF000000
#define XVOIPFEC_RX_VERSION_PATCH_ID_SHIFT                   8
#define XVOIPFEC_RX_VERSION_VERSION_REVISION_SHIFT           12
#define XVOIPFEC_RX_VERSION_VERSION_MINOR_SHIFT              16
#define XVOIPFEC_RX_VERSION_VERSION_MAJOR_SHIFT              24

/* VOIPFEC_RX SYSTEM STATUS */
#define XVOIPFEC_RX_SYSTEM_STATUS_REQ_Q_OV_MASK              0x1
#define XVOIPFEC_RX_SYSTEM_STATUS_EVENT_Q_OV_MASK            0x2
#define XVOIPFEC_RX_SYSTEM_STATUS_BUFF_LVL_UP_Q_OV_MASK      0x4
#define XVOIPFEC_RX_SYSTEM_STATUS_PRI_IN_BUFF_OV_MASK        0x80
#define XVOIPFEC_RX_SYSTEM_STATUS_SEC_IN_BUFF_OV_MASK        0x100
#define XVOIPFEC_RX_SYSTEM_STATUS_OUT_BUFF_OV_MASK           0x200
#define XVOIPFEC_RX_SYSTEM_STATUS_EVENT_Q_OV_SHIFT           1
#define XVOIPFEC_RX_SYSTEM_STATUS_BUFF_LVL_UP_Q_OV_SHIFT     2
#define XVOIPFEC_RX_SYSTEM_STATUS_PRI_IN_BUFF_OV_SHIFT       7
#define XVOIPFEC_RX_SYSTEM_STATUS_SEC_IN_BUFF_OV_SHIFT       8
#define XVOIPFEC_RX_SYSTEM_STATUS_OUT_BUFF_OV_SHIFT          9

/* Channel Registers */
#define XVOIPFEC_RX_FEC_CHAN_CONFIG_REG_OFFSET             ((1*128)+(0*4))
#define XVOIPFEC_RX_CHAN_BUF_BASE_ADDR_REG_OFFSET          ((1*128)+(1*4))
#define XVOIPFEC_RX_CHAN_MAP_BASE_ADDR_REG_OFFSET          ((1*128)+(2*4))
#define XVOIPFEC_RX_CHAN_BUFFER_DEPTH_REG_OFFSET           ((1*128)+(3*4))
#define XVOIPFEC_RX_VALID_PKTS_CNT_REG_OFFSET              ((1*128)+(4*4))
#define XVOIPFEC_RX_UNRECV_PKTS_CNT_REG_OFFSET             ((1*128)+(5*4))
#define XVOIPFEC_RX_CORR_PKTS_CNT_REG_OFFSET               ((1*128)+(6*4))
#define XVOIPFEC_RX_DUP_PKTS_CNT_REG_OFFSET                ((1*128)+(7*4))
#define XVOIPFEC_RX_CHANNEL_STATUS_REG_OFFSET              ((1*128)+(10*4))
#define XVOIPFEC_RX_CURR_BUFFER_DEPTH_REG_OFFSET           ((1*128)+(11*4))
#define XVOIPFEC_RX_OOR_PKTS_CNT_REG_OFFSE                 ((1*128)+(12*4))
#define XVOIPFEC_RX_OOR_MANAGEMENT_REG_OFFSET              ((1*128)+(13*4))
#define XVOIPFEC_RX_RTP_TS_DIFFERENCE_REG_OFFSET           ((1*128)+(14*4))

/* VOIPFEC_RX FEC_CONFIG */
#define XVOIPFEC_RX_FEC_CHAN_CONFIG_CHANNEL_ENABLE_MASK     0x1
#define XVOIPFEC_RX_FEC_CHAN_CONFIG_MEDIA_PKT_BYPASS_MASK   0x2
#define XVOIPFEC_RX_FEC_CHAN_CONFIG_FEC_RECV_DISABLE_MASK   0x4
#define XVOIPFEC_RX_FEC_CHAN_CONFIG_MEDIA_PKT_BYPASS_SHIFT  1
#define XVOIPFEC_RX_FEC_CHAN_CONFIG_FEC_RECV_DISABLE_SHIFT  2

/* XVOIPFEC_RX_CHAN_BUFFER_DEPTH_OFFSET */
#define XVOIPFEC_RX_CHAN_BUFFER_DEPTH_MASK                  0x1FFFF

/* VOIPFEC_RX CHANNEL_STATUS */
#define XVOIPFEC_RX_CHANNEL_STATUS_FEC_L_MASK               0x3FF
#define XVOIPFEC_RX_CHANNEL_STATUS_FEC_D_MASK               0xFFC00
#define XVOIPFEC_RX_CHANNEL_STATUS_COL_FEC_DETECTED_MASK    0x100000
#define XVOIPFEC_RX_CHANNEL_STATUS_ROW_FEC_DETECTED_MASK    0x200000
#define XVOIPFEC_RX_CHANNEL_STATUS_FEC_D_SHIFT              10
#define XVOIPFEC_RX_CHANNEL_STATUS_COL_FEC_DETECTED_SHIFT   20
#define XVOIPFEC_RX_CHANNEL_STATUS_ROW_FEC_DETECTED_SHIFT   21

/* XVOIPFEC_RX_OOR_MANAGEMENT_REG_OFFSET */
#define XVOIPFEC_RX_OOR_MANAGEMENT_TS_WINDOW_MASK           0x7FFFFFFF
#define XVOIPFEC_RX_OOR_MANAGEMENT_OOR_TS_SELECT_MASK       0x80000000
#define XVOIPFEC_RX_OOR_MANAGEMENT_OOR_TS_SELECT_SHIFT      31

/* VOIPFEC_RX CURR_BUFFER_DEPTH */
#define XVOIPFEC_RX_CURR_BUFFER_DEPTH_MASK                  0x1FFFF


/* XVOIPFEC_RX_RTP_TS_DIFFERENCE_REG_OFFSET */
#define XVOIPFEC_RX_SEAMLESS_PROTECT_RTP_TS_DIFF_MASK       0xFFFFFFFF

/**************************** Type Definitions *******************************/
#define XVOIPFEC_RX_ZEROES                                  0x0

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XVoipFEC_RX_In32        Xil_In32    /**< Input Operations */
#define XVoipFEC_RX_Out32       Xil_Out32   /**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a VoIP FEC Receiver register. A 32 bit read is
* performed. If the component is implemented in a smaller width, only the least
* significant data is read from the register. The most significant data
* will be read as 0.
*
* @param    BaseAddress is the base address of the VoIP FEC Receiver core
        instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file).
*
* @return   The 32-bit value of the register.
*
* @note     C-style signature:
*       u32 XVoipFEC_RX_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XVoipFEC_RX_ReadReg(BaseAddress, RegOffset) \
    XVoipFEC_RX_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a VoIP FEC Receiver register. A 32 bit write is
* performed. If the component is implemented in a smaller width, only the least
* significant data is written.
*
* @param    BaseAddress is the base address of the VoIP FEC Receiver core
*       instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file) to be written.
* @param    Data is the 32-bit value to write into the register.
*
* @return   None.
*
* @note     C-style signature:
*       void XVoipFEC_RX_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XVoipFEC_RX_WriteReg(BaseAddress, RegOffset, Data) \
    XVoipFEC_RX_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
