/******************************************************************************
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv_secure.h
* @addtogroup sysmonpsv_api SYSMONPSV APIs
*
*
*
* The xsysmonpsv_secure.h header file contains low level driver functions that are use to read and
* write sysmon registers.
*
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 4.0   se     11/10/22 Secure and Non-Secure mode integration
*
* </pre>
*
******************************************************************************/

#ifndef _XSYSMONPSV_SECURE_H_
#define _XSYSMONPSV_SECURE_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xipipsu_hw.h"
#include "xipipsu.h"
#include "xscugic.h"
#include "xsysmonpsv_lowlevel.h"
#include "pm_api_sys.h"

#define XSYSMONPSV_SECURE_WRITE_DEFAULT 0xFFFFFFFF
#define XSYSMONPSV_SECURE_READ_DEFAULT 0x0

#define XSYSMONPSV_SECURE_DEFAULT_PAYLOAD_SIZE 3U

/************************** Function Prototypes ******************************/
int XSysMonPsv_Xilpm_Init(XSysMonPsv *Instanceptr, XScuGic *const GicInst, XIpiPsu *const IpiInst);


#ifdef __cplusplus
}
#endif
#endif /* _XSYSMONPSV_SECURE_H_ */
