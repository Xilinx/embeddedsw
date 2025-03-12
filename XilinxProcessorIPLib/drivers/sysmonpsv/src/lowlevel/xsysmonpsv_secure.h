/******************************************************************************
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv_secure.h
* @addtogroup sysmonpsv_api SYSMONPSV APIs
* @{
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
* 5.2   ak     03/10/25 Fixed doxygen warnings
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

#define XSYSMONPSV_SECURE_WRITE_DEFAULT 0xFFFFFFFF /**< Secure Write Command Payload */
#define XSYSMONPSV_SECURE_READ_DEFAULT 0x0         /**< Secure Read Command Payload  */

#define XSYSMONPSV_SECURE_DEFAULT_PAYLOAD_SIZE 3U  /**< Secure Payload Size  */

/************************** Function Prototypes ******************************/
/******************************************************************************/
/**
 * Initializes the XIL PM.
 *
 * @param	Instanceptr Pointer to the XSysMonPsv instance.
 * @param	GicInst Pointer to XScuGic.
 * @param	IpiInst Pointer to XIpiPsu.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error code
 *              or a reason code.
 *
 ******************************************************************************/
int XSysMonPsv_Xilpm_Init(XSysMonPsv *Instanceptr, XScuGic *const GicInst, XIpiPsu *const IpiInst);


#ifdef __cplusplus
}
#endif
#endif /* _XSYSMONPSV_SECURE_H_ */
/** @} */
