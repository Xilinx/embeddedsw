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
* @file xpacketizer56_hw.h
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx VoIP ST2022-6 Packetizer core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver xpacketizer56.h
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

#ifndef XPACKETIZER56_H /* prevent circular inclusions */
#define XPACKETIZER56_H /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/* Register Offsets */
#define XPACKETIZER56_CONTROL_REG_OFFSET                       ((0*128)+(0*4))
#define XPACKETIZER56_HWVERSION_REG_OFFSET                     ((0*128)+(3*4))
#define XPACKETIZER56_MODULE_CONTROL_REG_OFFSET                ((0*128)+(4*4))
#define XPACKETIZER56_VIDEO_FORMAT_REG_OFFSET                  ((0*128)+(5*4))
#define XPACKETIZER56_VID_SRC_FMT_REG_OFFSET                   ((0*128)+(6*4))
#define XPACKETIZER56_MEDIA_HEADER_REG_OFFSET                  ((0*128)+(7*4))
#define XPACKETIZER56_MEDIA_PAYLOAD_LENGTH_REG_OFFSET          ((0*128)+(8*4))
#define XPACKETIZER56_SSRC_REG_OFFSET                          ((0*128)+(9*4))
#define XPACKETIZER56_OUTPUT_PKT_CNT_REG_OFFSET                ((0*128)+(10*4))
#define XPACKETIZER56_RECEIVED_FRAME_CNT_REG_OFFSET            ((0*128)+(11*4))
#define XPACKETIZER56_STATISTIC_RESET_REG_OFFSET               ((0*128)+(12*4))
#define XPACKETIZER56_ERROR_STATUS_REG_OFFSET                  ((0*128)+(13*4))
#define XPACKETIZER56_INTR_STATUS_REG_OFFSET                   ((0*128)+(16*4))
#define XPACKETIZER56_INTR_MASK_REG_OFFSET                     ((0*128)+(17*4))
#define XPACKETIZER56_INTR_CLEAR_REG_OFFSET                    ((0*128)+(18*4))

/* XPACKETIZER56_CONTROL_REG_OFFSET */
#define XPACKETIZER56_CONTROL_SOFT_RESET_MASK                  0x1

/* XPACKETIZER56_HWVERSION_REG_OFFSET */
#define XPACKETIZER56_VERSION_REVISION_NUMBER_MASK             0xFF
#define XPACKETIZER56_VERSION_PATCH_ID_MASK                    0xF00
#define XPACKETIZER56_VERSION_VERSION_REVISION_MASK            0xF000
#define XPACKETIZER56_VERSION_VERSION_MINOR_MASK               0xFF0000
#define XPACKETIZER56_VERSION_VERSION_MAJOR_MASK               0xFF000000
#define XPACKETIZER56_VERSION_PATCH_ID_SHIFT                   8
#define XPACKETIZER56_VERSION_VERSION_REVISION_SHIFT           12
#define XPACKETIZER56_VERSION_VERSION_MINOR_SHIFT              16
#define XPACKETIZER56_VERSION_VERSION_MAJOR_SHIFT              24

/* XPACKETIZER56_MODULE_CONTROL_REG_OFFSET */
#define XPACKETIZER56_MODULE_CONTROL_ENABLE_MASK               0x1
#define XPACKETIZER56_MODULE_CONTROL_LOSSLESS_MASK             0x2
#define XPACKETIZER56_MODULE_CONTROL_CHANNEL_NUM_MASK          0x0FFF0000
#define XPACKETIZER56_MODULE_CONTROL_LOSSLESS_SHIFT            1
#define XPACKETIZER56_MODULE_CONTROL_CHANNEL_NUM_SHIFT         16

/* XPACKETIZER56_VIDEO_FORMAT_REG_OFFSET */
#define XPACKETIZER56_VIDEO_FORMAT_VIDEO_MODE_MASK             0x3
#define XPACKETIZER56_VIDEO_FORMAT_3G_LEVEL_B_MASK             0x4
#define XPACKETIZER56_VIDEO_FORMAT_BITRATE_DIV_1_001_MASK      0x8
#define XPACKETIZER56_VIDEO_FORMAT_3G_LEVEL_B_SHIFT            2
#define XPACKETIZER56_VIDEO_FORMAT_BITRATE_DIV_1_001_SHIFT     3

/* XPACKETIZER56_VID_SRC_FMT_REG_OFFSET */
#define XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_MASK      0xF0000000
#define XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_MASK    0xFF00000
#define XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_MASK    0xFF000
#define XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_MASK   0xF00
#define XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_SHIFT     28
#define XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_SHIFT   20
#define XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_SHIFT   12
#define XPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_SHIFT  8

/* XPACKETIZER56_MEDIA_HEADER_REG_OFFSET */
#define XPACKETIZER56_MEDIA_HEADER_VID_TIMESTAMP_INCL_MASK     0x1
#define XPACKETIZER56_MEDIA_HEADER_VIDEO_TS_REFERENCE_MASK     0x6
#define XPACKETIZER56_MEDIA_HEADER_CLOCK_FREQ_MASK             0xF0000
#define XPACKETIZER56_MEDIA_HEADER_VIDEO_TS_REFERENCE_SHIFT    1
#define XPACKETIZER56_MEDIA_HEADER_CLOCK_FREQ_SHIFT            16

/* XPACKETIZER56_MEDIA_PAYLOAD_LENGTH_REG_OFFSET */
#define XPACKETIZER56_MEDIA_CONFIG_MEDIA_PAYLOAD_LENGTH_MASK   0x7FF

/* XPACKETIZER56_STATISTIC_RESET_REG_OFFSET */
#define XPACKETIZER56_STATISTIC_CLEAR_MASK                     0x1

/* XPACKETIZER56_ERROR_STATUS_REG_OFFSET */
#define XPACKETIZER56_ERROR_STATUS_DET_DATAGRAM_PER_FRAME_MASK 0xFFFF

/* XPACKETIZER56_INTR_STATUS_REG_OFFSET */
#define XPACKETIZER56_INTR_STATUS_DATAGRAM_FRAME_MISMATCH_MASK 0x1

/* XPACKETIZER56_STATISTIC_CLEAR_REG_OFFSET */
#define XPACKETIZER56_STATISTIC_CLEAR_MASK                     0x1

/* XPACKETIZER56_BUFFER_RESET_REG_OFFSET */
#define XPACKETIZER56_BUFFER_RESET_MASK                        0x1

/**************************** Type Definitions *******************************/
#define XPACKETIZER56_ZEROES                                   0x0

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XPacketizer56_In32      Xil_In32    /**< Input Operations */
#define XPacketizer56_Out32     Xil_Out32   /**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a VoIP ST2022-6 Packetizer register. A 32 bit
* read is performed. If the component is implemented in a smaller width, only
* the least significant data is read from the register. The most significant
* data will be read as 0.
*
* @param    BaseAddress is the base address of the VoIP ST2022-6 Packetizer
*       core instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file).
*
* @return   The 32-bit value of the register.
*
* @note     C-style signature:
*       u32 XPacketizer56_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XPacketizer56_ReadReg(BaseAddress, RegOffset) \
    XPacketizer56_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
*
* This macro reads a value from a VoIP ST2022-6 Packetizer register. A 32 bit
* write is performed. If the component is implemented in a smaller width, only
* the least significant data is written.
*
* @param    BaseAddress is the base address of the VoIP ST2022-6 Packetizer
*       core instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file) to be written.
* @param    Data is the 32-bit value to write into the register.
*
* @return   None.
*
* @note     C-style signature:
*       void XPacketizer56_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XPacketizer56_WriteReg(BaseAddress, RegOffset, Data) \
    XPacketizer56_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
