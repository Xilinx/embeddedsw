/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xhdcp22_common.h
*
* This file contains common functions shared between HDCP22 drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  MH   10/30/15 First Release.
* 1.01  MH   01/15/16 Added prefix to function names.
* 2.00  MH   06/21/17 Changed DIGIT_T type to u32 for ARM support.
*</pre>
*
*****************************************************************************/

#ifndef XHDCP22_COMMON_H_
#define XHDCP22_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#include "bigdigits.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

/* Cryptographic functions */
void XHdcp22Cmn_Sha256Hash(const u8 *Data, u32 DataSize, u8 *HashedData);
int  XHdcp22Cmn_HmacSha256Hash(const u8 *Data, int DataSize, const u8 *Key, int KeySize, u8  *HashedData);
void XHdcp22Cmn_Aes128Encrypt(const u8 *Data, const u8 *Key, u8 *Output);
void XHdcp22Cmn_Aes128Decrypt(const u8 *Data, const u8 *Key, u8 *Output);

#ifdef __cplusplus
}
#endif

#endif /* XHDCP22_COMMON_H_ */
