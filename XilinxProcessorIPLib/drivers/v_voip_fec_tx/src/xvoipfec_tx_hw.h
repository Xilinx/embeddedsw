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
* @file xvoipfec_tx_hw.h
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx VoIP FEC Transmitter core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver xvoipfec_tx.h file.
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

#ifndef XVOIPFEC_TX_H   /* prevent circular inclusions */
#define XVOIPFEC_TX_H   /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/* Register Offsets */
#define XVOIPFEC_TX_CONTROL_REG_OFFSET                       ((0*128)+(0*4))
#define XVOIPFEC_TX_STATUS_REG_OFFSET                        ((0*128)+(1*4))
#define XVOIPFEC_TX_CHANNEL_ACCESS_REG_OFFSET                ((0*128)+(2*4))
#define XVOIPFEC_TX_SYSTEM_CONFIG_REG_OFFSET                 ((0*128)+(3*4))
#define XVOIPFEC_TX_HWVERSION_REG_OFFSET                     ((0*128)+(4*4))
#define XVOIPFEC_TX_FEC_BASEADDRESS_REG_OFFSET               ((0*128)+(5*4))
#define XVOIPFEC_TX_DEBUG_CONTROL_REG_OFFSET                 ((0*128)+(6*4))
#define XVOIPFEC_TX_IN_PACKET_COUNT_REG_OFFSET               ((0*128)+(9*4))
#define XVOIPFEC_TX_OUT_PACKET_COUNT_REG_OFFSET              ((0*128)+(10*4))

/* VOIPFEC_TX CONTROL MASK */
#define XVOIPFEC_TX_CONTROL_CHANNEL_UPDATE_MASK              0x2
#define XVOIPFEC_TX_CONTROL_SOFT_RESET_MASK                  0x1
#define XVOIPFEC_TX_CONTROL_CHANNEL_UPDATE_SHIFT             1

/* VOIPFEC_TX STATUS MASK */
#define XVOIPFEC_TX_STATUS_UPDATE_BUSY_MASK                  0x1

/* VOIPFEC_TX CHANNEL ACCESS MASK */
#define XVOIPFEC_TX_CHANNEL_ACCESS_MASK                      0xFFF

/* VOIPFEC_TX SYS CONFIG */
#define XVOIPFEC_TX_SYS_CONFIG_C_CHANNELS_MASK               0xFFF
#define XVOIPFEC_TX_SYS_CONFIG_SUPP_MAX_L_MASK               0x03FF0000
#define XVOIPFEC_TX_SYS_CONFIG_SUPP_MAX_L_SHIFT              16

/* VOIPFEC_TX VERSION */
#define XVOIPFEC_TX_VERSION_REVISION_NUMBER_MASK             0xFF
#define XVOIPFEC_TX_VERSION_PATCH_ID_MASK                    0xF00
#define XVOIPFEC_TX_VERSION_VERSION_REVISION_MASK            0xF000
#define XVOIPFEC_TX_VERSION_VERSION_MINOR_MASK               0xFF0000
#define XVOIPFEC_TX_VERSION_VERSION_MAJOR_MASK               0xFF000000
#define XVOIPFEC_TX_VERSION_PATCH_ID_SHIFT                   8
#define XVOIPFEC_TX_VERSION_VERSION_REVISION_SHIFT           12
#define XVOIPFEC_TX_VERSION_VERSION_MINOR_SHIFT              16
#define XVOIPFEC_TX_VERSION_VERSION_MAJOR_SHIFT              24

/* Channel Registers */
#define XVOIPFEC_TX_FEC_CONFIG_REG_OFFSET                   ((1*128)+(0*4))


/* VOIPFEC_TX FEC_CONFIG */
#define XVOIPFEC_TX_FEC_CONFIG_FEC_D_MASK                   0xFF
#define XVOIPFEC_TX_FEC_CONFIG_FEC_L_MASK                   0x3FF00
#define XVOIPFEC_TX_FEC_CONFIG_FEC_MODE_MASK                0xC0000
#define XVOIPFEC_TX_FEC_CONFIG_NON_BLOCK_ALIGN_MASK         0x100000
#define XVOIPFEC_TX_FEC_CONFIG_FEC_L_SHIFT                  8
#define XVOIPFEC_TX_FEC_CONFIG_FEC_MODE_SHIFT               18
#define XVOIPFEC_TX_FEC_CONFIG_NON_BLOCK_ALIGN_SHIFT        20




/**************************** Type Definitions *******************************/
#define XVOIPFEC_TX_ZEROES                                  0x0

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XVoipFEC_TX_In32        Xil_In32    /**< Input Operations */
#define XVoipFEC_TX_Out32       Xil_Out32   /**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a VoIP FEC Transmitter register. A 32 bit read
* is performed. If the component is implemented in a smaller width, only the
* least significant data is read from the register. The most significant data
* will be read as 0.
*
* @param    BaseAddress is the base address of the VoIP FEC Transmitter core
        instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file).
*
* @return   The 32-bit value of the register.
*
* @note     C-style signature:
*       u32 XVoipFEC_TX_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XVoipFEC_TX_ReadReg(BaseAddress, RegOffset) \
    XVoipFEC_TX_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a VoIP FEC Transmitter register. A 32 bit write
* is performed. If the component is implemented in a smaller width, only the
* least significant data is written.
*
* @param    BaseAddress is the base address of the VoIP FEC Transmitter core
        instance.
* @param    RegOffset is the register offset of the register (defined at
*       the top of this file) to be written.
* @param    Data is the 32-bit value to write into the register.
*
* @return   None.
*
* @note     C-style signature:
*       void XVoipFEC_TX_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XVoipFEC_TX_WriteReg(BaseAddress, RegOffset, Data) \
    XVoipFEC_TX_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
