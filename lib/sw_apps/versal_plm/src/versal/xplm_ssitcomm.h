/**************************************************************************************************
* Copyright (c) 2024, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/


/*************************************************************************************************/
/**
*
* @file versal/xplm_ssitcomm.h
*
* This file contains PLMI ssit secure communication specific declarations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- ---------------------------------------------------------------------------
* 1.00  pre  07/11/2024 Initial release
*
* </pre>
*
* @note
*
**************************************************************************************************/

#ifndef XPLM_SSITCOMM_H
#define XPLM_SSITCOMM_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *****************************************************/
#include "xsecure_aes.h"
#include "xplmi_ssit.h"

/************************** Constant Definitions *************************************************/
#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
#ifdef PLM_ENABLE_SECURE_PLM_TO_PLM_COMM
/**************************** Type Definitions ***************************************************/
typedef struct
{
	int (*Init)(XSecure_Aes *InstancePtr, XSecure_AesKeySrc KeySrc,
	           XSecure_AesKeySize KeySize, u64 IvAddr);
	int (*UpdateAad)(XSecure_Aes *InstancePtr, u64 AadAddr, u32 AadSize);
	int (*UpdateData)(XSecure_Aes *InstancePtr, u64 InDataAddr,
	           u64 OutDataAddr, u32 Size, u8 IsLastChunk);
	int (*Final)(XSecure_Aes *InstancePtr, u64 GcmTagAddr);
} XPlm_SsitCommOps;

typedef struct
{
	u64 AadAddr;
	u64 InDataAddr;
	u64 TagAddr;
	u32 *IvPtr;
	u32 *OutDataPtr;
	u32 AADLen;
	u32 DataLen;
	XSecure_AesKeySrc KeySrc;
	XPlmi_Operation OperationFlag;
	u32 SlrIndex;
	u32 IsCfgSecCommCmd;
	u32 TempRespBuf[XPLMI_CMD_RESP_SIZE];
} XPlm_SsitCommParams;

/***************** Macros (Inline Functions) Definitions *****************************************/

/************************** Function Prototypes **************************************************/
int XPlm_SsitCommAesKeyWrite(u32 SlrIndex, u32 KeyAddr);
int XPlm_SsitCommKeyIvUpdate(XPlmi_Cmd *Cmd);
#endif
int XPlm_SsitCommSendMessage(u32* Buf, u32 BufSize, u32 SlrIndex, u32 IsCfgSecCommCmd);
int XPlm_SsitCommReceiveMessage(u32* Buf, u32 BufSize, u32 SlrIndex, u32 IsCfgSecCommCmd);
XPlmi_SsitCommFunctions *XPlm_SsitCommGetFuncsPtr(void);
#endif
/************************** Variable Definitions *************************************************/

#ifdef __cplusplus
}
#endif
#endif  /* XPLM_SSITCOMM_H */
