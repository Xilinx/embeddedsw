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
* @file xdepacketizer56_hw.h
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx VoIP ST2022-6 Depacketizer
* core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver xdepacketizer56.h
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

#ifndef XDEPACKETIZER56_H   /* prevent circular inclusions */
#define XDEPACKETIZER56_H   /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/* Register Offsets */
#define XDEPACKETIZER56_CONTROL_REG_OFFSET                     ((0*128)+(0*4))
#define XDEPACKETIZER56_STATUS_REG_OFFSET                      ((0*128)+(1*4))
#define XDEPACKETIZER56_BUF_LEVEL_REG_OFFSET                   ((0*128)+(2*4))
#define XDEPACKETIZER56_HWVERSION_REG_OFFSET                   ((0*128)+(3*4))
#define XDEPACKETIZER56_MODULE_CONTROL_REG_OFFSET              ((0*128)+(4*4))
#define XDEPACKETIZER56_VIDEO_FORMAT_REG_OFFSET                ((0*128)+(5*4))
#define XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_REG_OFFSET  ((0*128)+(6*4))
#define XDEPACKETIZER56_MEDIA_HEADER_REG_OFFSET                ((0*128)+(7*4))
#define XDEPACKETIZER56_MEDIA_PYLD_LEN_REG_OFFSET              ((0*128)+(8*4))
#define XDEPACKETIZER56_FRAME_SIZE_REG_OFFSET                  ((0*128)+(9*4))
#define XDEPACKETIZER56_INPUT_PKT_CNT_REG_OFFSET               ((0*128)+(10*4))
#define XDEPACKETIZER56_TRANSMIT_FRAME_CNT_REG_OFFSET          ((0*128)+(11*4))
#define XDEPACKETIZER56_STATISTIC_CLEAR_REG_OFFSET             ((0*128)+(12*4))
#define XDEPACKETIZER56_ERROR_STATUS_REG_OFFSET                ((0*128)+(13*4))
#define XDEPACKETIZER56_INTR_STATUS_REG_OFFSET                 ((0*128)+(16*4))
#define XDEPACKETIZER56_INTR_MASK_REG_OFFSET                   ((0*128)+(17*4))
#define XDEPACKETIZER56_INTR_CLEAR_REG_OFFSET                  ((0*128)+(18*4))

/* XDEPACKETIZER56_CONTROL_REG_OFFSET */
#define XDEPACKETIZER56_CONTROL_SOFT_RESET_MASK                0x1

/* XDEPACKETIZER56_BUF_LEVEL_REG_OFFSET */
#define XDEPACKETIZER56_BUF_LEVEL_PEAK_BUFFER_LEVEL_MASK       0x3F
#define XDEPACKETIZER56_BUF_LEVEL_CURR_BUFFER_LEVEL_MASK       0x3F0000
#define XDEPACKETIZER56_BUF_LEVEL_CURR_BUFFER_LEVEL_SHIFT      16

/* XDEPACKETIZER56_STATUS_REG_OFFSET */
#define XDEPACKETIZER56_STATUS_UPDATE_BUSY_MASK                0x1

/* XDEPACKETIZER56_HWVERSION_REG_OFFSET */
#define XDEPACKETIZER56_VERSION_REVISION_NUMBER_MASK           0xFF
#define XDEPACKETIZER56_VERSION_PATCH_ID_MASK                  0xF00
#define XDEPACKETIZER56_VERSION_VERSION_REVISION_MASK          0xF000
#define XDEPACKETIZER56_VERSION_VERSION_MINOR_MASK             0xFF0000
#define XDEPACKETIZER56_VERSION_VERSION_MAJOR_MASK             0xFF000000
#define XDEPACKETIZER56_VERSION_PATCH_ID_SHIFT                 8
#define XDEPACKETIZER56_VERSION_VERSION_REVISION_SHIFT         12
#define XDEPACKETIZER56_VERSION_VERSION_MINOR_SHIFT            16
#define XDEPACKETIZER56_VERSION_VERSION_MAJOR_SHIFT            24

/* XDEPACKETIZER56_MODULE_CONTROL_REG_OFFSET */
#define XDEPACKETIZER56_MODULE_CONTROL_ENABLE_MASK             0x1

/* XDEPACKETIZER56_VIDEO_FORMAT_REG_OFFSET */
#define XDEPACKETIZER56_VIDEO_FORMAT_VIDEO_MODE_MASK           0x3
#define XDEPACKETIZER56_VIDEO_FORMAT_3G_LEVEL_B_MASK           0x4
#define XDEPACKETIZER56_VIDEO_FORMAT_BITRATE_DIV_1_001_MASK    0x8
#define XDEPACKETIZER56_VIDEO_FORMAT_3G_LEVEL_B_SHIFT          2
#define XDEPACKETIZER56_VIDEO_FORMAT_BITRATE_DIV_1_001_SHIFT   3

/* XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_REG_OFFSET */
#define XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_MASK      0xF0000000
#define XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_MASK    0xFF00000
#define XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_MASK    0xFF000
#define XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_MASK   0xF00
#define XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_MAP_SHIFT     28
#define XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRAME_SHIFT   20
#define XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_FRATE_SHIFT   12
#define XDEPACKETIZER56_MAP_FRAME_FRATE_SAMPLE_FMT_SAMPLE_SHIFT  8

/* XDEPACKETIZER56_MEDIA_HEADER_REG_OFFSET */
#define XDEPACKETIZER56_MEDIA_HEADER_VIDEO_TS_INCLUDE_MASK       0x1

/* XDEPACKETIZER56_MEDIA_PYLD_LEN_REG_OFFSET */
#define XDEPACKETIZER56_MEDIA_PYLD_LEN_MASK                      0x7FF

/* XDEPACKETIZER56_FRAME_SIZE_REG_OFFSET */
#define XDEPACKETIZER56_FRAME_SIZE_USER_CONFIG_MASK               0x80000000
#define XDEPACKETIZER56_FRAME_SIZE_LAST_DATAGRAM_LEN_MASK         0x07FF0000
#define XDEPACKETIZER56_FRAME_SIZE_DATAGRAM_PER_FRAME_MASK        0xFFFF
#define XDEPACKETIZER56_FRAME_SIZE_USER_CONFIG_SHIFT              31
#define XDEPACKETIZER56_FRAME_SIZE_LAST_DATAGRAM_LEN_SHIFT        16

/* XDEPACKETIZER56_STATISTIC_CLEAR_REG_OFFSET */
#define XDEPACKETIZER56_STATISTIC_CLEAR_MASK                      0x7
#define XDEPACKETIZER56_STATISTIC_CLEAR_INPUT_PKT_CNT_MASK        0x1
#define XDEPACKETIZER56_STATISTIC_CLEAR_OUT_FRAME_CNT_MASK        0x2
#define XDEPACKETIZER56_STATISTIC_CLEAR_PEAK_BUF_LEVEL_MASK       0x3

/* XDEPACKETIZER56_ERROR_STATUS_REG_OFFSET */
#define XDEPACKETIZER56_ERROR_STATUS_DET_DATAGRAM_PER_FRAME_MASK  0xFFFF
#define XDEPACKETIZER56_ERROR_STATUS_BUFFER_LEVEL_MASK            0x3F0000
#define XDEPACKETIZER56_ERROR_STATUS_BUFFER_LEVEL_SHIFT           16

/* XDEPACKETIZER56_INTR_STATUS_REG_OFFSET */
#define XDEPACKETIZER56_INTR_STATUS_DATAGRAM_FRAME_MISMATCH_MASK  0x1
#define XDEPACKETIZER56_INTR_STATUS_BUFFER_EMPTY_MASK             0x2
#define XDEPACKETIZER56_INTR_STATUS_BUFFER_EMPTY_SHIFT            1


/**************************** Type Definitions *******************************/
#define XDEPACKETIZER56_ZEROES                                   0x0

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XDepacketizer56_In32     Xil_In32   /**< Input Operations */
#define XDepacketizer56_Out32    Xil_Out32  /**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a ST2022-6 Depacketizer register. A 32 bit
* read is performed. If the component is implemented in a smaller width, only
* the least significant data is read from the register. The most significant
* data will be read as 0.
*
* @param    BaseAddress is the base address of the ST2022-6 Depacketizer core
*       instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file).
*
* @return   The 32-bit value of the register.
*
* @note     C-style signature:
*       u32 XDepacketizer56_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XDepacketizer56_ReadReg(BaseAddress, RegOffset) \
    XDepacketizer56_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a ST2022-6 Depacketizer register. A 32 bit write
* is performed. If the component is implemented in a smaller width, only the
* least significant data is written.
*
* @param    BaseAddress is the base address of the ST2022-6 Depacketizer core
*       instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file) to be written.
* @param    Data is the 32-bit value to write into the register.
*
* @return   None.
*
* @note     C-style signature:
*       void XDepacketizer56_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XDepacketizer56_WriteReg(BaseAddress, RegOffset, Data) \
    XDepacketizer56_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
