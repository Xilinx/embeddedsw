/******************************************************************************
* Copyright (C) 2018 – 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_hdcp_keys.h
*
* This is the main header file for the Xilinx HDCP key loading utility used
* in the HDMI example design. The HDCP cores are used for content protection
* according to the HDCP 1.4 and HDCP 2.2 specifications.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------
* 1.0   MG   26-01-2016 Initial version
* </pre>
*
******************************************************************************/
#ifndef XHDMI_HDCP_KEYS_H_
#define XHDMI_HDCP_KEYS_H_ /**< Prevent circular inclusions
                        *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xstatus.h"
#include "sleep.h"
#include "xil_printf.h"
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
#include "xiicps.h"
#else
#include "xiic.h"
#endif

#include "aes256.h"
#include "sha256.h"
#include <string.h>
#if defined (XPAR_XUARTPSV_NUM_INSTANCES )
#include "xuartpsv.h"
#elif defined (XPAR_XUARTLITE_NUM_INSTANCES)
#include "xuartlite_l.h"
#else
#include "xuartps.h"
#endif

/************************** Function Prototypes ******************************/
int XHdcp_LoadKeys(void *IicPtr,
		u8 *Hdcp22Lc128, u32 Hdcp22Lc128Size, u8 *Hdcp22RxPrivateKey, u32 Hdcp22RxPrivateKeySize,
		u8 *Hdcp14KeyA, u32 Hdcp14KeyASize, u8 *Hdcp14KeyB, u32 Hdcp14KeyBSize);
int XHdcp_KeyManagerInit(u32 BaseAddress, u8 *Hdcp14Key);

#if defined (XPS_BOARD_VEK280_ES) || \
	defined (XPS_BOARD_VEK280_ES_REVB)
#define XPS_BOARD_VEK280
#endif

#if defined (XPS_BOARD_VEK385) || \
	defined (XPS_BOARD_VEK385_1)
#define XPS_BOARD_VEK385
#endif

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
