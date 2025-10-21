/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdcp22_cipher_dp.h
* @addtogroup hdcp22_cipher_dp Overview
* @{
* @details
*
* This is the main header file of the Xilinx HDCP 2.2 Cipher device driver.
* The Cipher implements the AES-128 standard for encrypting and decrypting
* audiovisual content. The Cipher is required to be programmed with the
* global constant Lc128, random number Riv, and session key Ks before encryption
* is enabled. Internally, the cipher uses the Enhanced Encryption Signaling
* Status (EESS) to determine when to encrypt and decrypt frames. It also
* manages the data and frame counters to ensure the transmitter and receiver
* Ciphers are synchronized.
*
* <b>Software Initialization and Configuration</b>
*
* The application needs to do the following steps to run the Cipher.
* - Call XHdcp22Cipher_LookupConfig using the device ID to find the
*   core configuration instance.
* - Call XHdcp22Cipher_CfgInitialize to intitialize the device instance.
* - Call XHdcp22Cipher_SetTxMode or XHdcp22Cipher_SetRxMode to setup
*   the Cipher as either a transmitter or receiver.
* - Call XHdcp22Cipher_Enable to enable the cipher.
* - Call XHdcp22Cipher_SetLc128 to program the Lc128 constant.
* - Call XHdcp22Cipher_SetRiv to program the random number Riv.
* - Call XHdcp22Cipher_SetKs to program the session key Ks.
* - If operating as a transmitter call XHdcp22Cipher_EnableTxEncryption
*   to enable encryption or XHdcp22Cipher_DisableTxEncryption to
*   disable encryption.
*
* <b>Interrupts</b>
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  JB     02/19/19 Initial Release.
* 2.00  JB     12/24/21 File name changed from xhdcp22_cipher.h to
				xhdcp22_cipher_dp.h
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
#include "xhdcp22_cipher_dp_hw.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**
* This typedef contains configuration information for the HDCP22 Cipher core.
* Each HDCP22 Cipher device should have a configuration structure associated.
*/
typedef struct {
#ifndef SDT
	u16 DeviceId;     /**< DeviceId is the unique ID of the HDCP22 Cipher core */
#else
	char *Name;
#endif
	UINTPTR BaseAddress;  /**< BaseAddress is the physical base address of the core's registers */
} XHdcp22_Cipher_Dp_Config;

/**
* The XHdcp22 Cipher driver instance data. An instance must be allocated for each
* HDCP22 Cipher core in use.
*/
typedef struct {
	XHdcp22_Cipher_Dp_Config Config; /**< Hardware Configuration */
	u32 IsReady;                  /**< Core and the driver instance are initialized */
} XHdcp22_Cipher_Dp;

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
*         void XHdcp22Cipher_Dp_Enable(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_Dp_Enable(InstancePtr) \
        XHdcp22Cipher_Dp_WriteReg((InstancePtr)->Config.BaseAddress, \
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
*         void XHdcp22Cipher_Dp_Disable(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_Dp_Disable(InstancePtr) \
        XHdcp22Cipher_Dp_WriteReg((InstancePtr)->Config.BaseAddress, \
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
*         u32 XHdcp22Cipher_Dp_IsEnabled(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_Dp_IsEnabled(InstancePtr) \
        ((XHdcp22Cipher_Dp_GetControlReg((InstancePtr)->Config.BaseAddress)\
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
*         void XHdcp22Cipher_Dp_SetTxMode(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_Dp_SetTxMode(InstancePtr) \
        XHdcp22Cipher_Dp_WriteReg((InstancePtr)->Config.BaseAddress, \
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
*         void XHdcp22Cipher_Dp_SetRxMode(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_Dp_SetRxMode(InstancePtr) \
        XHdcp22Cipher_Dp_WriteReg((InstancePtr)->Config.BaseAddress, \
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
*         void XHdcp22Cipher_Dp_EnableTxEncryption(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_Dp_EnableTxEncryption(InstancePtr) \
        XHdcp22Cipher_Dp_WriteReg((InstancePtr)->Config.BaseAddress, \
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
*         void XHdcp22Cipher_Dp_DisableTxEncryption(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_Dp_DisableTxEncryption(InstancePtr) \
        XHdcp22Cipher_Dp_WriteReg((InstancePtr)->Config.BaseAddress, \
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
*         u32 XHdcp22Cipher_Dp_IsTxEncryptionEnabled(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_Dp_IsTxEncryptionEnabled(InstancePtr) \
        ((XHdcp22Cipher_Dp_GetControlReg((InstancePtr)->Config.BaseAddress)\
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
*         void XHdcp22Cipher_Dp_DisableTxEncryption(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_Dp_IsEncrypted(InstancePtr) \
        ((XHdcp22Cipher_Dp_GetStatusReg((InstancePtr)->Config.BaseAddress) \
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
*         void XHdcp22Cipher_Dp_Noise(u32 BaseAddress, u8 Set)
*
******************************************************************************/
#define XHdcp22Cipher_Dp_Noise(InstancePtr, Set) \
{ \
        if (Set) { \
                XHdcp22Cipher_Dp_WriteReg((InstancePtr)->Config.BaseAddress, (XHDCP22_CIPHER_REG_CTRL_SET_OFFSET), (XHDCP22_CIPHER_REG_CTRL_NOISE_MASK)); \
        } \
        else { \
                XHdcp22Cipher_Dp_WriteReg((InstancePtr)->Config.BaseAddress, (XHDCP22_CIPHER_REG_CTRL_CLR_OFFSET), (XHDCP22_CIPHER_REG_CTRL_NOISE_MASK)); \
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
*         void XHdcp22Cipher_Dp_Blank(u32 BaseAddress, u8 Set)
*
******************************************************************************/
#define XHdcp22Cipher_Dp_Blank(InstancePtr, Set) \
{ \
        if (Set) { \
                XHdcp22Cipher_Dp_WriteReg((InstancePtr)->Config.BaseAddress, (XHDCP22_CIPHER_REG_CTRL_SET_OFFSET), (XHDCP22_CIPHER_REG_CTRL_BLANK_MASK)); \
        } \
        else { \
                XHdcp22Cipher_Dp_WriteReg((InstancePtr)->Config.BaseAddress, (XHDCP22_CIPHER_REG_CTRL_CLR_OFFSET), (XHDCP22_CIPHER_REG_CTRL_BLANK_MASK)); \
        } \
}

/*****************************************************************************/
/**
*
* This macro reads the version for the HDCP22 Cipher
* peripheral.
*
* @param  InstancePtr is a pointer to the HDCP22 Cipher core instance.
*
* @return version.
*
* @note   C-style signature:
*         void XHdcp22Cipher_GetVersion(u32 BaseAddress)
*
******************************************************************************/
#define XHdcp22Cipher_Dp_GetVersion(InstancePtr) \
        XHdcp22Cipher_Dp_ReadReg((InstancePtr)->Config.BaseAddress, XHDCP22_CIPHER_VER_VERSION_OFFSET)

/************************** Function Prototypes ******************************/
/* Initialization function in xhdcp22_cipher_sinit.c */
#ifndef SDT
XHdcp22_Cipher_Dp_Config *XHdcp22Cipher_Dp_LookupConfig(u16 DeviceId);
#else
XHdcp22_Cipher_Dp_Config *XHdcp22Cipher_Dp_LookupConfig(UINTPTR BaseAddress);
#endif
/* Initialization and control functions in xhdcp22_cipher.c */
int XHdcp22Cipher_Dp_CfgInitialize(XHdcp22_Cipher_Dp *InstancePtr, XHdcp22_Cipher_Dp_Config *CfgPtr, UINTPTR EffectiveAddr);

void XHdcp22Cipher_Dp_SetKs(XHdcp22_Cipher_Dp *InstancePtr, const u8 *KsPtr, u16 Length);
void XHdcp22Cipher_Dp_SetLc128(XHdcp22_Cipher_Dp *InstancePtr, const u8 *Lc128Ptr,  u16 Length);
void XHdcp22Cipher_Dp_SetRiv(XHdcp22_Cipher_Dp *InstancePtr, const u8 *RivPtr,  u16 Length);
void XHdcp22Cipher_Dp_SetLanecount(XHdcp22_Cipher_Dp *InstancePtr, u8 LaneCount);
void XHdcp22Cipher_Dp_SetMst(XHdcp22_Cipher_Dp *InstancePtr, u8 Mode);
/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XHDCP2_CIPHER_H */

/** @} */
