/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xhdcp22_cipher.h
* @addtogroup hdcp22_cipher_v1_0
* @{
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx HDCP22 cipher core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  jo  10/01/15 Initial release.
* 1.1   MG  10/28/15 Added Noise and blank macros
* </pre>
*
******************************************************************************/

#ifndef XHDCP22_CIPHER_H
/**< Prevent circular inclusions by using protection macros */
#define XHDCP22_CIPHER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xhdcp22_cipher_hw.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**
* This typedef contains configuration information for the HDCP22 Cipher core.
* Each HDCP22 Cipher device should have a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;     /**< DeviceId is the unique ID of the HDCP22 Cipher core */
	u32 BaseAddress;  /**< BaseAddress is the physical base address of the core's registers */
} XHdcp22_Cipher_Config;

/**
* The XHdcp22 Cipher driver instance data. An instance must be allocated for each
* HDCP22 Cipher core in use.
*/
typedef struct {
	XHdcp22_Cipher_Config Config; /**< Hardware Configuration */
	u32 IsReady;                  /**< Core and the driver instance are initialized */
} XHdcp22_Cipher;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro enables the HDCP22 Cipher peripheral.
*
* @param  InstancePtr is a pointer to the HDCP22 Cipher core instance.
*
* @return None.
*
* @note   C-style signature:
*         void XHdcp22Cipher_Enable(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_Enable(InstancePtr) \
        XHdcp22Cipher_WriteReg((InstancePtr)->Config.BaseAddress, \
        (XHDCP22_CIPHER_REG_CTRL_SET_OFFSET),(XHDCP22_CIPHER_REG_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro disables the HDCP22 Cipher peripheral.
*
* @param  InstancePtr is a pointer to the HDCP22 Cipher core instance.
*
* @return None.
*
* @note   C-style signature:
*         void XHdcp22Cipher_Disable(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_Disable(InstancePtr) \
        XHdcp22Cipher_WriteReg((InstancePtr)->Config.BaseAddress, \
        (XHDCP22_CIPHER_REG_CTRL_CLR_OFFSET), (XHDCP22_CIPHER_REG_CTRL_RUN_MASK))

/*****************************************************************************/
/**
*
* This macro returns the encrypted enabled state of the HDCP22 Cipher core
* instance.
*
* @param  InstancePtr is a pointer to the HDCP22 Cipher core instance.
*
* @return TRUE if HDCP22 cipher is enabled, FALSE otherwise.
*
* @note   C-style signature:
*         u32 XHdcp22Cipher_IsEnabled(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_IsEnabled(InstancePtr) \
        ((XHdcp22Cipher_GetControlReg((InstancePtr)->Config.BaseAddress)\
        & XHDCP22_CIPHER_REG_CTRL_RUN_MASK) ==  XHDCP22_CIPHER_REG_CTRL_RUN_MASK)

/*****************************************************************************/
/**
*
* This macro sets the HDCP operation mode for the HDCP22 Cipher peripheral.
* The mode
*
* @param  InstancePtr is a pointer to the HDCP22 Cipher core instance.
*
* @return None.
*
* @note   C-style signature:
*         void XHdcp22Cipher_SetTxMode(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_SetTxMode(InstancePtr) \
        XHdcp22Cipher_WriteReg((InstancePtr)->Config.BaseAddress, \
        (XHDCP22_CIPHER_REG_CTRL_CLR_OFFSET), (XHDCP22_CIPHER_REG_CTRL_MODE_MASK))

/*****************************************************************************/
/**
*
* This macro sets the HDCP RX operation mode for the HDCP22 Cipher peripheral.
*
* @param  InstancePtr is a pointer to the HDCP22 Cipher core instance.
*
* @return None.
*
* @note   C-style signature:
*         void XHdcp22Cipher_SetRxMode(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_SetRxMode(InstancePtr) \
        XHdcp22Cipher_WriteReg((InstancePtr)->Config.BaseAddress, \
        (XHDCP22_CIPHER_REG_CTRL_SET_OFFSET), (XHDCP22_CIPHER_REG_CTRL_MODE_MASK))

/*****************************************************************************/
/**
*
* This macro enables HDCP TX encryption for the HDCP22 Cipher peripheral.
*
* @param  InstancePtr is a pointer to the HDCP22 Cipher core instance.
*
* @return None.
*
* @note   C-style signature:
*         void XHdcp22Cipher_EnableTxEncryption(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_EnableTxEncryption(InstancePtr) \
        XHdcp22Cipher_WriteReg((InstancePtr)->Config.BaseAddress, \
        (XHDCP22_CIPHER_REG_CTRL_SET_OFFSET), (XHDCP22_CIPHER_REG_CTRL_ENCRYPT_MASK))

/*****************************************************************************/
/**
*
* This macro disables HDCP TX encryption for the HDCP22 Cipher peripheral.
*
* @param  InstancePtr is a pointer to the HDCP22 Cipher core instance.
*
* @return None.
*
* @note   C-style signature:
*         void XHdcp22Cipher_DisableTxEncryption(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_DisableTxEncryption(InstancePtr) \
        XHdcp22Cipher_WriteReg((InstancePtr)->Config.BaseAddress, \
        (XHDCP22_CIPHER_REG_CTRL_CLR_OFFSET), (XHDCP22_CIPHER_REG_CTRL_ENCRYPT_MASK))

/*****************************************************************************/
/**
*
* This macro returns the encrypted enabled state of HDCP TX encryption
* for the HDCP22 Cipher peripheral.
*
* @param  InstancePtr is a pointer to the HDCP22 Cipher core instance.
*
* @return TRUE if HDCP22 TX encryption is enabled, FALSE otherwise.
*
* @note   C-style signature:
*         u32 XHdcp22Cipher_IsTxEncryptionEnabled(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_IsTxEncryptionEnabled(InstancePtr) \
        ((XHdcp22Cipher_GetControlReg((InstancePtr)->Config.BaseAddress)\
        & XHDCP22_CIPHER_REG_CTRL_ENCRYPT_MASK) ==  XHDCP22_CIPHER_REG_CTRL_ENCRYPT_MASK)

/*****************************************************************************/
/**
*
* This macro returns the encrypted state for the HDCP22 Cipher peripheral.
*
* @param  InstancePtr is a pointer to the HDCP22 Cipher core instance.
*
* @return TRUE if the frame is encrypted, FALSE otherwise.
*
* @note   C-style signature:
*         void XHdcp22Cipher_DisableTxEncryption(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_IsEncrypted(InstancePtr) \
        ((XHdcp22Cipher_GetStatusReg((InstancePtr)->Config.BaseAddress) \
        & XHDCP22_CIPHER_REG_STA_ENCRYPTED_MASK) ==  XHDCP22_CIPHER_REG_STA_ENCRYPTED_MASK)

/*****************************************************************************/
/**
*
* This macro enables or disables noise output for the HDCP22 Cipher
* peripheral.
*
* @param  InstancePtr is a pointer to the HDCP22 Cipher core instance.
* @param  Set specifies TRUE/FALSE either to enable/disable noise output.
*
* @return none.
*
* @note   C-style signature:
*         void XHdcp22Cipher_Noise(u32 BaseAddress, u8 Set)
*
******************************************************************************/
#define XHdcp22Cipher_Noise(InstancePtr, Set) \
{ \
        if (Set) { \
                XHdcp22Cipher_WriteReg((InstancePtr)->Config.BaseAddress, (XHDCP22_CIPHER_REG_CTRL_SET_OFFSET), (XHDCP22_CIPHER_REG_CTRL_NOISE_MASK)); \
        } \
        else { \
                XHdcp22Cipher_WriteReg((InstancePtr)->Config.BaseAddress, (XHDCP22_CIPHER_REG_CTRL_CLR_OFFSET), (XHDCP22_CIPHER_REG_CTRL_NOISE_MASK)); \
        } \
}

/*****************************************************************************/
/**
*
* This macro enables or disables blank screen for the HDCP22 Cipher
* peripheral.
*
* @param  InstancePtr is a pointer to the HDCP22 Cipher core instance.
* @param  Set specifies TRUE/FALSE either to enable/disable blank screen.
*
* @return none.
*
* @note   C-style signature:
*         void XHdcp22Cipher_Blank(u32 BaseAddress, u8 Set)
*
******************************************************************************/
#define XHdcp22Cipher_Blank(InstancePtr, Set) \
{ \
        if (Set) { \
                XHdcp22Cipher_WriteReg((InstancePtr)->Config.BaseAddress, (XHDCP22_CIPHER_REG_CTRL_SET_OFFSET), (XHDCP22_CIPHER_REG_CTRL_BLANK_MASK)); \
        } \
        else { \
                XHdcp22Cipher_WriteReg((InstancePtr)->Config.BaseAddress, (XHDCP22_CIPHER_REG_CTRL_CLR_OFFSET), (XHDCP22_CIPHER_REG_CTRL_BLANK_MASK)); \
        } \
}

/************************** Function Prototypes ******************************/
/* Initialization function in xhdcp22_cipher_sinit.c */
XHdcp22_Cipher_Config *XHdcp22Cipher_LookupConfig(u16 DeviceId);

/* Initialization and control functions in xhdcp22_cipher.c */
int XHdcp22Cipher_CfgInitialize(XHdcp22_Cipher *InstancePtr, XHdcp22_Cipher_Config *CfgPtr, u32 EffectiveAddr);

void XHdcp22Cipher_SetKs(XHdcp22_Cipher *InstancePtr, const u8 *KsPtr, u16 Length);
void XHdcp22Cipher_SetLc128(XHdcp22_Cipher *InstancePtr, const u8 *Lc128Ptr,  u16 Length);
void XHdcp22Cipher_SetRiv(XHdcp22_Cipher *InstancePtr, const u8 *RivPtr,  u16 Length);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XHDCP2_CIPHER_H */
/** @} */
