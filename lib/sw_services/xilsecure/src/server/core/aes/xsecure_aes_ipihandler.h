/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_aes_ipihandler.h
*
* This file contains the Xilsecure AES IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0  kal   03/23/21 Initial release
* 5.2  vss	 07/15/23 Added prototype for XSecure_MakeAesFree()
* 5.4  yog   04/29/24 Fixed doxygen grouping.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_aes_server_apis XilSecure AES Server APIs
* @{
*/
#ifndef XSECURE_AES_IPIHANDLER_H_
#define XSECURE_AES_IPIHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int XSecure_AesIpiHandler(XPlmi_Cmd *Cmd);
int XSecure_AesInit(void);
int XSecure_AesKeyWrite(u8  KeySize, u8 KeySrc, u32 KeyAddrLow, u32 KeyAddrHigh);
int XSecure_AesPerformOperation(u32 SrcAddrLow, u32 SrcAddrHigh);
int XSecure_AesKeyZeroize(u32 KeySrc);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_AES_IPIHANDLER_H_ */
/** @} */
