/******************************************************************************
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef _XSYSMONPSV_SECURE_H_
#define _XSYSMONPSV_SECURE_H_

/*****************************************************************************/
/**
*
* @file xsysmonpsv_secure.h
*
* @brief Declares the secure-mode register access functions for the driver.
*
* The xsysmonpsv_secure.h header file declares low-level driver functions used
* to read and write SysMon registers in secure mode.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 4.0   se     11/10/22 Secure and Non-Secure mode integration
* 5.2   ak     03/10/25 Fixed doxygen warnings
* 5.3   se     03/25/26 Fix Coverity/MISRA-C violations: U suffix,
*                       (void) cast, sign conversion, parentheses
* 5.4   vn     09/29/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup sysmonpsv_api SYSMONPSV APIs
 * @{
 */

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

/** Secure write command payload. */
#define XSYSMONPSV_SECURE_WRITE_DEFAULT 0xFFFFFFFFU
/** Secure read command payload. */
#define XSYSMONPSV_SECURE_READ_DEFAULT 0x0U
/** Secure payload size. */
#define XSYSMONPSV_SECURE_DEFAULT_PAYLOAD_SIZE 3U

/************************** Function Prototypes ******************************/
/******************************************************************************/
/**
 * @brief Initializes XilPM.
 *
 * @param[in]   Instanceptr Pointer to the XSysMonPsv instance.
 * @param[in]   GicInst Pointer to the XScuGic instance.
 * @param[in]   IpiInst Pointer to the XIpiPsu instance.
 *
 * @return      XST_SUCCESS if successful, else XST_FAILURE, an error code,
 *              or a reason code.
 *
 ******************************************************************************/
int XSysMonPsv_Xilpm_Init(XSysMonPsv *Instanceptr, XScuGic *const GicInst, XIpiPsu *const IpiInst);


#ifdef __cplusplus
}
#endif

/** @} */

#endif /* _XSYSMONPSV_SECURE_H_ */
