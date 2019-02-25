/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* @file xhdcp22_cipher_hw.h
* @addtogroup hdcp22_cipher_v1_1
* @{
* @details
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx HDCP 2.2 Cipher core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  JB     02/19/19 Initial Release.
* </pre>
*
******************************************************************************/

#ifndef XHDCP22_CIPHER_HW_H
/**< Prevent circular inclusions by using protection macros */
#define XHDCP22_CIPHER_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/
/* HDCP22 Cipher Version Interface base */
#define XHDCP22_CIPHER_VER_BASE             (0*64)
/** VER Identification register * register offset */
#define XHDCP22_CIPHER_VER_ID_OFFSET        ((XHDCP22_CIPHER_VER_BASE)+(0*4))
/** VER Version register * offset */
#define XHDCP22_CIPHER_VER_VERSION_OFFSET   ((XHDCP22_CIPHER_VER_BASE)+(1*4))

/* HDCP22 Cipher Register peripheral interface base */
#define XHDCP22_CIPHER_REG_BASE             (1*64)
/** Control register * register offset */
#define XHDCP22_CIPHER_REG_CTRL_OFFSET      ((XHDCP22_CIPHER_REG_BASE)+(0*4))
/** Control set register * offset */
#define XHDCP22_CIPHER_REG_CTRL_SET_OFFSET  ((XHDCP22_CIPHER_REG_BASE)+(1*4))
/** Control clear register * offset */
#define XHDCP22_CIPHER_REG_CTRL_CLR_OFFSET  ((XHDCP22_CIPHER_REG_BASE)+(2*4))
/** Status register * offset */
#define XHDCP22_CIPHER_REG_STA_OFFSET       ((XHDCP22_CIPHER_REG_BASE)+(3*4))
/** Ks register 1 * offset */
#define XHDCP22_CIPHER_REG_KS_1_OFFSET      ((XHDCP22_CIPHER_REG_BASE)+(4*4))
/** Ks register 2 * offset */
#define XHDCP22_CIPHER_REG_KS_2_OFFSET      ((XHDCP22_CIPHER_REG_BASE)+(5*4))
/** Ks register 3 * offset */
#define XHDCP22_CIPHER_REG_KS_3_OFFSET      ((XHDCP22_CIPHER_REG_BASE)+(6*4))
/** Ks register 4 * offset */
#define XHDCP22_CIPHER_REG_KS_4_OFFSET      ((XHDCP22_CIPHER_REG_BASE)+(7*4))
/** Lc128 register 1 * offset */
#define XHDCP22_CIPHER_REG_LC128_1_OFFSET   ((XHDCP22_CIPHER_REG_BASE)+(8*4))
/** Lc128 register 2 * offset */
#define XHDCP22_CIPHER_REG_LC128_2_OFFSET   ((XHDCP22_CIPHER_REG_BASE)+(9*4))
/** Lc128 register 3 * offset */
#define XHDCP22_CIPHER_REG_LC128_3_OFFSET   ((XHDCP22_CIPHER_REG_BASE)+(10*4))
/** Lc128 register 4 * offset */
#define XHDCP22_CIPHER_REG_LC128_4_OFFSET   ((XHDCP22_CIPHER_REG_BASE)+(11*4))
/** Riv register 1 * offset */
#define XHDCP22_CIPHER_REG_RIV_1_OFFSET     ((XHDCP22_CIPHER_REG_BASE)+(12*4))
/** Riv register 2 * offset */
#define XHDCP22_CIPHER_REG_RIV_2_OFFSET     ((XHDCP22_CIPHER_REG_BASE)+(13*4))
/** InputCtr register 1 * offset */
#define XHDCP22_CIPHER_REG_INPUTCTR_1_OFFSET    ((XHDCP22_CIPHER_REG_BASE)+(14*4))
/** InputCtr register 2 * offset */
#define XHDCP22_CIPHER_REG_INPUTCTR_2_OFFSET    ((XHDCP22_CIPHER_REG_BASE)+(15*4))

/* HDCP22 Cipher Control register masks */
/** Control register Run mask */
#define XHDCP22_CIPHER_REG_CTRL_RUN_MASK        (1<<0)
/** Control register Interrupt Enable mask. Reserved for future use. */
#define XHDCP22_CIPHER_REG_CTRL_IE_MASK         (1<<1)
/** Control register Mode mask */
#define XHDCP22_CIPHER_REG_CTRL_MODE_MASK       (1<<2)
/** Control register Encrypt mask */
#define XHDCP22_CIPHER_REG_CTRL_ENCRYPT_MASK    (1<<3)
/** Control register blank mask */
#define XHDCP22_CIPHER_REG_CTRL_BLANK_MASK    	(1<<4)
/** Control register noise mask */
#define XHDCP22_CIPHER_REG_CTRL_NOISE_MASK    	(1<<5)
/** Control register lane count mask*/
#define XHDCP22_CIPHER_REG_CTRL_LANE_CNT_MASK  	(0x0F<<6)
/** Control register lane count bits position */
#define XHDCP22_CIPHER_REG_CTRL_LANE_CNT_BIT_POS	6

/* HDCP22 Cipher Status register masks */
/** Status register interrupt mask. Reserved for future use.*/
#define XHDCP22_CIPHER_REG_STA_IRQ_MASK         (1<<0)
/** Status register event mask. Reserved for future use.*/
#define XHDCP22_CIPHER_REG_STA_EVT_MASK         (1<<1)
/** Status register encrypted mask. */
#define XHDCP22_CIPHER_REG_STA_ENCRYPTED_MASK   (1<<2)

/* Peripheral ID and General shift values. */
#define XHDCP22_CIPHER_SHIFT_16               16      /**< 16 shift value */
#define XHDCP22_CIPHER_MASK_16                0xFFFF  /**< 16 bit mask value */
#define XHDCP22_CIPHER_VER_ID                 0x2200  /**< Version ID */


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XHdcp22Cipher_In32        Xil_In32        /**< Input Operations */
#define XHdcp22Cipher_Out32       Xil_Out32       /**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a HDCP22 Cipher register.
* A 32 bit read is performed.
* If the component is implemented in a smaller width, only the least
* significant data is read from the register. The most significant data
* will be read as 0.
*
* @param  BaseAddress is the base address of the HDCP22 Cipher core instance.
* @param  RegOffset is the register offset of the register (defined at
*         the top of this file).
*
* @return The 32-bit value of the register.
*
* @note   C-style signature:
*         u32 XHdcp22Cipher_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XHdcp22Cipher_ReadReg(BaseAddress, RegOffset) \
        XHdcp22Cipher_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a HDCP22 Cipher register.
* A 32 bit write is performed. If the component is implemented in a smaller
* width, only the least significant data is written.
*
* @param  BaseAddress is the base address of the HDCP22 Cipher core instance.
* @param  RegOffset is the register offset of the register (defined at
*         the top of this file) to be written.
* @param  Data is the 32-bit value to write into the register.
*
* @return None.
*
* @note   C-style signature:
*         void XHdcp22Cipher_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XHdcp22Cipher_WriteReg(BaseAddress, RegOffset, Data) \
        XHdcp22Cipher_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))
/*****************************************************************************/
/**
*
* This macro reads the status register from the HDCP22 Cipher.
*
* @param  IBaseAddress is the base address of the HDCP22 Cipher core instance.
*
* @return A 32-bit value representing the contents of the status register.
*
* @note   C-style signature:
*         u32 XHdcp22Cipher_GetStatusReg(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_GetStatusReg(BaseAddress) \
        XHdcp22Cipher_ReadReg(BaseAddress, XHDCP22_CIPHER_REG_STA_OFFSET)

/*****************************************************************************/
/**
*
* This macro reads the control register from the HDCP22 Cipher.
*
* @param  BaseAddress is the base address of the HDCP22 Cipher core instance.
*
* @return A 32-bit value representing the contents of the control register.
*
* @note   C-style signature:
*         u32 XHdcp22Cipher_GetStatusReg(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_GetControlReg(BaseAddress) \
        XHdcp22Cipher_ReadReg(BaseAddress, XHDCP22_CIPHER_REG_CTRL_OFFSET)
/*@}*/

/************************** Function Prototypes ******************************/

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XHDCP2_CIPHER_HW_H */

/** @} */
