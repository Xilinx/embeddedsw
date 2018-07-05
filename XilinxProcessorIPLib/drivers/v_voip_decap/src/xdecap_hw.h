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
* @file xdecap_hw.h
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx VoIP Decapsulator core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver xdecap.h file.
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
#ifndef XDecap_HW_H_
#define XDecap_HW_H_        /**< Prevent circular inclusions
                  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/*VOIP DECAPSULATOR REGISTER OFFSETS*/

/*GENERAL SPACE OF VOIP DECAPSULATOR*/
#define XDECAP_CONTROL_OFFSET                                  ((0*128)+(0*4))
#define XDECAP_STATUS_OFFSET                                   ((0*128)+(1*4))
#define XDECAP_CHANNEL_ACCESS_OFFSET                           ((0*128)+(2*4))
#define XDECAP_SYS_CONFIG_OFFSET                               ((0*128)+(3*4))
#define XDECAP_VERSION_OFFSET                                  ((0*128)+(4*4))
#define XDECAP_STREAM_DETECT_WINDOW_OFFSET                     ((0*128)+(5*4))
#define XDECAP_RX_PKT_CNT_OFFSET                               ((0*128)+(6*4))
#define XDECAP_MISMATCHED_PKT_CNT_OFFSET                       ((0*128)+(7*4))
#define XDECAP_ERROR_PKT_CNT_OFFSET                            ((0*128)+(8*4))
#define XDECAP_CLEAR_STATISTIC_OFFSET                          ((0*128)+(9*4))
#define XDECAP_PEAK_BUFFER_LEVEL_OFFSET                        ((0*128)+(10*4))
#define XDECAP_MODULE_CONTROL_OFFSET                           ((0*128)+(11*4))
#define XDECAP_CH_INTR_GROUP_SUMMARY_OFFSET                    ((0*128)+(12*4))
#define XDECAP_CH_INTR_GROUP_0_OFFSET                          ((0*128)+(13*4))

/*VOIP DECAPSULATOR CONTROL MASK*/
#define XDECAP_CONTROL_CHANNEL_UPDATE_MASK                      0x2
#define XDECAP_CONTROL_SOFT_RESET_MASK                          0x1
#define XDECAP_CONTROL_CHANNEL_UPDATE_SHIFT                     1

/*VOIP DECAPSULATOR STATUS MASK*/
#define XDECAP_STATUS_UPDATE_BUSY_MASK                          0x1

/*VOIP DECAPSULATOR CHANNEL ACCESS MASK */
#define XDECAP_CHANNEL_ACCESS_MASK                              0xFFF

/*VOIP DECAPSULATOR SYS CONFIG */
#define XDECAP_SYS_CONFIG_C_CHANNELS_MASK                       0xFFF

/*VOIP DECAPSULATOR VERSION*/
#define XDECAP_VERSION_REVISION_NUMBER_MASK                     0xFF
#define XDECAP_VERSION_PATCH_ID_MASK                            0xF00
#define XDECAP_VERSION_VERSION_REVISION_MASK                    0xF000
#define XDECAP_VERSION_VERSION_MINOR_MASK                       0xFF0000
#define XDECAP_VERSION_VERSION_MAJOR_MASK                       0xFF000000
#define XDECAP_VERSION_PATCH_ID_SHIFT                           8
#define XDECAP_VERSION_VERSION_REVISION_SHIFT                   12
#define XDECAP_VERSION_VERSION_MINOR_SHIFT                      16
#define XDECAP_VERSION_VERSION_MAJOR_SHIFT                      24

/*XDECAP_CLEAR_STATISTIC_OFFSET*/
#define XDECAP_CLEAR_STATISTIC_MASK                             0x7
#define XDECAP_CLEAR_RX_PCKT_CNT_STATISTIC_MASK                 0x1
#define XDECAP_CLEAR_MISMATCHED_PKT_CNT_STATISTIC_MASK          0x2
#define XDECAP_CLEAR_ERR_PKT_CNT_STATISTIC_MASK                 0x4

/*XDECAP_STREAM_DETECT_WINDOW_OFFSET*/
#define XDECAP_STREAM_DETECT_WINDOW_VIDEO_LOCK_MASK             0xFFFF
#define XDECAP_STREAM_DETECT_WINDOW_VIDEO_UNLOCK_MASK           0xFFFF0000
#define XDECAP_STREAM_DETECT_WINDOW_VIDEO_UNLOCK_SHIFT          16

/*XDECAP_PEAK_BUFFER_LEVEL_OFFSET*/
#define XDECAP_PEAK_BUFFER_LEVEL_MASK                           0x3F

/*DECAP_MODULE_CONTROL_OFFSET*/
#define XDECAP_MODULE_CONTROL_MODULE_ENABLE_MASK                0x1

/*XDECAP_CH_INTR_GROUP_SUMMARY_OFFSET*/
#define XDECAP_CH_INTR_GROUP_SUMMARY_G0_MASK                    0x1

/*CHANNEL SPACE OF VOIP DECAPSULATOR */
#define XDECAP_CHANNEL_CONTROL_OFFSET                           ((1*128)+(0*4))
#define XDECAP_VIDEO_STOP_TIMER_OFFSET                          ((1*128)+(1*4))
#define XDECAP_IP_HDR_PARAM_OFFSET                              ((1*128)+(2*4))
#define XDECAP_MATCH_VLAN_ID_OFFSET                             ((1*128)+(4*4))
#define XDECAP_MATCH_DEST_IP0_OFFSET                            ((1*128)+(5*4))
#define XDECAP_MATCH_DEST_IP1_OFFSET                            ((1*128)+(6*4))
#define XDECAP_MATCH_DEST_IP2_OFFSET                            ((1*128)+(7*4))
#define XDECAP_MATCH_DEST_IP3_OFFSET                            ((1*128)+(8*4))
#define XDECAP_MATCH_SRC_IP0_OFFSET                             ((1*128)+(9*4))
#define XDECAP_MATCH_SRC_IP1_OFFSET                             ((1*128)+(10*4))
#define XDECAP_MATCH_SRC_IP2_OFFSET                             ((1*128)+(11*4))
#define XDECAP_MATCH_SRC_IP3_OFFSET                             ((1*128)+(12*4))
#define XDECAP_MATCH_UDP_SRC_PORT_OFFSET                        ((1*128)+(13*4))
#define XDECAP_MATCH_UDP_DEST_PORT_OFFSET                       ((1*128)+(14*4))
#define XDECAP_MATCH_SSRC_OFFSET                                ((1*128)+(15*4))
#define XDECAP_MATCH_SELECT_OFFSET                              ((1*128)+(16*4))
#define XDECAP_VIDEO_FORMAT_OFFSET                              ((1*128)+(17*4))
#define XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_OFFSET                ((1*128)+(18*4))
#define XDECAP_MEDIA_HEADER_OFFSET                              ((1*128)+(19*4))
#define XDECAP_MEDIA_VALID_PACKET_COUNT_OFFSET                  ((1*128)+(20*4))
#define XDECAP_FEC_VALID_PACKET_COUNT_OFFSET                    ((1*128)+(21*4))
#define XDECAP_FEC_REORDERED_PACKET_COUNT_OFFSET                ((1*128)+(22*4))
#define XDECAP_DROP_PACKET_COUNT_OFFSET                         ((1*128)+(23*4))
#define XDECAP_STATISTIC_RESET_OFFSET                           ((1*128)+(24*4))
#define XDECAP_PACKET_INTERVAL_OFFSET                           ((1*128)+(25*4))
#define XDECAP_MATCH_PAYLOAD_TYPE_OFFSET                        ((1*128)+(27*4))
#define XDECAP_INTERRUPT_STATUS_OFFSET                          ((1*128)+(28*4))
#define XDECAP_INTERRUPT_MASK_OFFSET                            ((1*128)+(29*4))
#define XDECAP_INTERRUPT_CLEAR_OFFSET                           ((1*128)+(30*4))

/*XDECAP_CHANNEL_CONTROL_OFFSET*/
#define XDECAP_CHANNEL_CONTROL_CHANNEL_ENABLE_MASK              0x1
#define XDECAP_CHANNEL_CONTROL_LOSSLESS_MODE_MASK               0x2
#define XDECAP_CHANNEL_CONTROL_M_PKT_DET_EN_MASK                0x4
#define XDECAP_CHANNEL_CONTROL_DROP_M_PKT_EN_MASK               0x8
#define XDECAP_CHANNEL_CONTROL_LOSSLESS_MODE_SHIFT              1
#define XDECAP_CHANNEL_CONTROL_M_PKT_DET_EN_SHIFT               2
#define XDECAP_CHANNEL_CONTROL_DROP_M_PKT_EN_SHIFT              3

/*XDECAP_IP_HDR_PARAM_OFFSET*/
#define XDECAP_IP_HDR_PARAM_TTL_MASK                            0xFF
#define XDECAP_IP_HDR_PARAM_TOS_MASK                            0xFF00
#define XDECAP_IP_HDR_PARAM_TOS_SHIFT                           8

/*XDECAP_MATCH_VLAN_ID_OFFSET*/
#define XDECAP_MATCH_VLAN_ID_VLAN_FILTER_ENABLE_MASK            0x80000000
#define XDECAP_MATCH_VLAN_ID_VLAN_TAG_MASK                      0xFFFF
#define XDECAP_MATCH_VLAN_ID_VLAN_FILTER_ENABLE_SHIFT           31

/*VOIP DECAPSULATOR MATCH UDP PORT*/
#define XDECAP_MATCH_UDP_PORT_UDP_SRC_MASK                      0xFFFF

/*VOIP DECAPSULATOR MATCH UDP PORT*/
#define XDECAP_MATCH_UDP_PORT_UDP_SRC_MASK                      0xFFFF

/*XDECAP_MATCH_SELECT_OFFSET*/
#define XDECAP_MATCH_SELECT_FILTER_ALL                        0x3F
#define XDECAP_MATCH_SELECT_TO_MATCH_VLAN_ID_REG_MASK         0x1
#define XDECAP_MATCH_SELECT_TO_MATCH_SRC_IP_MASK              0x2
#define XDECAP_MATCH_SELECT_TO_MATCH_DEST_IP_MASK             0x4
#define XDECAP_MATCH_SELECT_TO_MATCH_UDP_SRC_MASK             0x8
#define XDECAP_MATCH_SELECT_TO_MATCH_UDP_DEST_MASK            0x10
#define XDECAP_MATCH_SELECT_TO_MATCH_SSRC_MASK                0x20
#define XDECAP_MATCH_SELECT_TO_MATCH_DYNAMIC_PT_MASK          0x40
#define XDECAP_MATCH_SELECT_TO_MATCH_SRC_IP_SHIFT             1
#define XDECAP_MATCH_SELECT_TO_MATCH_DEST_IP_SHIFT            2
#define XDECAP_MATCH_SELECT_TO_MATCH_UDP_SRC_SHIFT            3
#define XDECAP_MATCH_SELECT_TO_MATCH_UDP_DEST_SHIFT           4
#define XDECAP_MATCH_SELECT_TO_MATCH_SSRC_SHIFT               5
#define XDECAP_MATCH_SELECT_TO_MATCH_DYNAMIC_PT_SHIFT         6

/*XDECAP_MATCH_UDP_SRC_PORT_OFFSET*/
#define XDECAP_MATCH_UDP_SRC_PORT_MASK                          0xFFFF

/*XDECAP_MATCH_UDP_SRC_PORT_OFFSET*/
#define XDECAP_MATCH_UDP_DEST_PORT_MASK                         0xFFFF

/*XDECAP_VIDEO_FORMAT_OFFSET*/
#define XDECAP_VIDEO_FORMAT_VIDEO_MODE_MASK                     0x3
#define XDECAP_VIDEO_FORMAT_3G_LEVEL_B_MASK                     0x4
#define XDECAP_VIDEO_FORMAT_BITRATE_DIV_1_001_MASK              0x8
#define XDECAP_VIDEO_FORMAT_SDI_PACKET_LOCK_STATUS_MASK         0x80000000
#define XDECAP_VIDEO_FORMAT_3G_LEVEL_B_SHIFT                    2
#define XDECAP_VIDEO_FORMAT_BITRATE_DIV_1_001_SHIFT             3
#define XDECAP_VIDEO_FORMAT_SDI_PACKET_LOCK_STATUS_SHIFT        31

/*XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_OFFSET*/
#define XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_MASK              0xF0000000
#define XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_MASK            0xFF00000
#define XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_MASK            0xFF000
#define XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_MASK           0xF00
#define XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_D_PT_MASK             0x7F
#define XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_SHIFT             28
#define XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_SHIFT           20
#define XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_SHIFT           12
#define XDECAP_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_SHIFT          8

/*XDECAP_MEDIA_HEADER_OFFSET*/
#define XDECAP_MEDIA_HEADER_VIDEO_TS_INCLUDE_MASK               0x1
#define XDECAP_MEDIA_HEADER_VIDEO_TS_REFERENCE_MASK             0x6
#define XDECAP_MEDIA_HEADER_FEC_USAGE_MASK                      0x70
#define XDECAP_MEDIA_HEADER_VIDEO_SRC_FRMT_PRSNT_MASK           0x00000100
#define XDECAP_MEDIA_HEADER_CF_VALUE_MASK                       0x000F0000
#define XDECAP_MEDIA_HEADER_FRAME_COUNTER_MASK                  0x0FF00000
#define XDECAP_MEDIA_HEADER_EXT_FIELD_MASK                      0xF0000000
#define XDECAP_MEDIA_HEADER_VIDEO_TS_REFERENCE_SHIFT            1
#define XDECAP_MEDIA_HEADER_FEC_USAGE_SHIFT                     4
#define XDECAP_MEDIA_HEADER_VIDEO_SRC_FRMT_PRSNT_SHIFT          8
#define XDECAP_MEDIA_HEADER_CF_VALUE_SHIFT                      16
#define XDECAP_MEDIA_HEADER_FRAME_COUNTER_SHIFT                 20
#define XDECAP_MEDIA_HEADER_EXT_FIELD_SHIFT                     28

/*XDECAP_STATISTIC_RESET_OFFSET*/
#define XDECAP_STATISTIC_RESET_ALL_PACKET_COUNTERS_MASK         0xF
#define XDECAP_STATISTIC_RESET_MEDIA_VALID_PACKET_COUNT_MASK    0x1
#define XDECAP_STATISTIC_RESET_FEC_VALID_PACKET_COUNT_MASK      0x2
#define XDECAP_STATISTIC_RESET_REORDERED_PACKET_COUNT_MASK      0x4
#define XDECAP_STATISTIC_RESET_DROP_PACKET_COUNT_MASK           0x8
#define XDECAP_STATISTIC_RESET_FEC_VALID_PACKET_COUNT_SHIFT     1
#define XDECAP_STATISTIC_RESET_REORDERED_PACKET_COUNT_SHIFT     2
#define XDECAP_STATISTIC_RESET_DROP_PACKET_COUNT_SHIFT          3

/*XDECAP_MATCH_PAYLOAD_TYPE_OFFSET*/
#define XDECAP_MATCH_PAYLOAD_TYPE_INCOMING_PCKT_TYPE_MASK       0xF0000000
#define XDECAP_MATCH_PAYLOAD_TYPE_RTP_DYNAMIC_PT_MASK           0x7F
#define XDECAP_MATCH_PAYLOAD_TYPE_INCOMING_PCKT_TYPE_SHIFT      28

/*XDECAP_INTERRUPT_STATUS_OFFSET,XDECAP_INTERRUPT_CLEAR_OFFSET &
**XDECAP_INTERRUPT_STATUS_OFFSET,XDECAP_INTERRUPT_CLEAR_OFFSET &
**XDECAP_INTERRUPT_MASK_OFFSET*/
#define XDECAP_INTERRUPT_STATUS_MASK                            0x1F
#define XDECAP_INTERRUPT_STATUS_PACKET_LOCKED_MASK              0x1
#define XDECAP_INTERRUPT_STATUS_PACKET_UNLOCKED_MASK            0x2
#define XDECAP_INTERRUPT_STATUS_STREAM_STOP_INTERRUPT_MASK      0x4
#define XDECAP_INTERRUPT_STATUS_PACKET_UNLOCKED_SHIFT           1
#define XDECAP_INTERRUPT_STATUS_STREAM_STOP_INTERRUPT_SHIFT     2


/**************************** Type Definitions *******************************/
#define XDECAP_ZEROES                                   0x0

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XDecap_In32     Xil_In32    /**< Input Operations */
#define XDecap_Out32    Xil_Out32   /**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a Decap register. A 32 bit read is performed.
* If the component is implemented in a smaller width, only the least
* significant data is read from the register. The most significant data
* will be read as 0.
*
* @param    BaseAddress is the base address of the Decap core instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file).
*
* @return   The 32-bit value of the register.
*
* @note     C-style signature:
*       u32 XDecap_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XDecap_ReadReg(BaseAddress, RegOffset) \
    XDecap_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a Decap register. A 32 bit write is performed.
* If the component is implemented in a smaller width, only the least
* significant data is written.
*
* @param    BaseAddress is the base address of the Decap core instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file) to be written.
* @param    Data is the 32-bit value to write into the register.
*
* @return   None.
*
* @note     C-style signature:
*       void XDecap_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XDecap_WriteReg(BaseAddress, RegOffset, Data) \
    XDecap_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
