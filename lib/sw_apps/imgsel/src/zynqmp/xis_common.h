/******************************************************************************
* Copyright (c) 2020-2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_common.h
*
* This file contains Common API's and Macros used across the
* the application
*
*
******************************************************************************/

#ifndef XIS_COMMON_H
#define XIS_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_util.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XIS_CSU_MULTI_BOOT							(0xFFCA0010U)
#define XIS_CRL_APB_RESET_CTRL 						(0xFF5E0218U)
#define XIS_CSU_APB_RESET_VAL  						(0x10U)
#define XIS_ERROR_STATUS_REGISTER_OFFSET			(0xFFD80060U)
#define XIS_CRL_APB_TIMESTAMP_REF_CTRL_OFFSET		(0XFF5E0128U)
#define XIS_CRL_APB_TIMESTAMP_MASK					(0x01003F07U)
#define XIS_CRL_APB_TIMESTAMP_VALUE					(0x01000A00U)
#define XIS_CRL_APB_RST_LPD_IOU2_OFFSET				(0XFF5E0238U)
#define XIS_CRL_APB_RST_LPD_IOU2_MASK				(0x00100000U)
#define XIS_CRL_APB_RST_LPD_IOU2_VALUE				(0x00000000U)

#define XIs_In32(Addr)								Xil_In32(Addr)
#define XIs_Out32(Addr, Data)						Xil_Out32(Addr, Data)

/************************** Function Prototypes ******************************/

void XIs_UpdateMultiBootValue(u32 Offset);
void XIs_UpdateError(int Error);
void XIs_Softreset(void);
void XIs_ClockConfigs(void);

#ifdef __cplusplus
}
#endif

#endif
