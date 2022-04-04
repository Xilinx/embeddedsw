/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_DEBUG_H_
#define XPM_DEBUG_H_

#include "xpm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// /*********************** Power domain specific Errors ***********************/
// /************************** (0x3000) - (0x30FF) ******************************/

// /*********************** Generic House Cleaning Errors ***********************/
// /*************************** (0x3100) - (0x31FF) *****************************/
/* Invalid power domain ID error */
#define XPM_INT_ERR_INVALID_PWR_DOMAIN		0x3000U
/* Error during power domain init */
#define XPM_INT_ERR_POWER_DOMAIN_INIT		0x3100U
/* Invalid base address */
#define XPM_INT_ERR_INVALID_BASEADDR		0x3101U
/* Error during BISR repair */
#define XPM_INT_ERR_BISR_REPAIR			0x3108U
/* Invalid trim type */
#define XPM_INT_ERR_INVALID_TRIM_TYPE		0x310EU
/* Error during self test */
#define XPM_INT_ERR_SELF_TEST			0x310FU

// /************************** LPD specific Errors ******************************/
// /************************** (0x3200) - (0x32FF) ******************************/

// /************************* Device specific Errors ****************************/
// /************************** (0x3300) - (0x33FF) ******************************/
/* Error during device lookup configuration */
#define XPM_INT_ERR_DEVICE_LOOKUP		0x3301U
/* Error during configuration initialization */
#define XPM_INT_ERR_CFG_INIT			0x3302U

// /************************* Reset specific Errors *****************************/
// /************************** (0x3400) - (0x34FF) ******************************/

// /************************** FPD specific Errors ******************************/
// /************************** (0x3500) - (0x35FF) ******************************/

// /************************* Miscellaneous Errors ******************************/
// /************************** (0x3600) - (0x36FF) ******************************/
/* Invalid parameter passed to function */
#define XPM_INT_ERR_INVALID_PARAM		0x3603U
/* Invalid processor */
#define XPM_INT_ERR_INVALID_PROC		0x3606U
/* Invalid NODE ID */
#define XPM_INT_ERR_INVALID_NODE		0x3607U
/* Invalid NODE index */
#define XPM_INT_ERR_INVALID_NODE_IDX		0x3608U
/* Invalid arguments */
#define XPM_INT_ERR_INVALID_ARGS		0x3609U

// /************************** PLD specific Errors ******************************/
// /************************** (0x3700) - (0x37FF) ******************************/

// /************************** NPD specific Errors ******************************/
// /************************** (0x3800) - (0x38FF) ******************************/

// /************************* Clock specific errors *****************************/
// /************************** (0x3900) - (0x39FF) ******************************/

// /************************** CPM specific Errors ******************************/
// /************************** (0x4000) - (0x40FF) ******************************/

// /************************** AIE specific Errors ******************************/
// /************************** (0x4100) - (0x41FF) ******************************/

// /************************** PMC specific Errors ******************************/
// /************************** (0x4200) - (0x42FF) ******************************/

/************************** BISR specific errors *****************************/
/************************** (0x4300) - (0x43FF) ******************************/
/* BISR unsupported ID */
#define XPM_INT_ERR_BISR_UNSUPPORTED_ID		0x4300U
/* Invalid BISR ID */
#define XPM_INT_ERR_BISR_INVALID_ID		0x4304U
/* Bad tag type */
#define XPM_INT_ERR_BAD_TAG_TYPE		0x4305U
/* Unknown tag ID */
#define XPM_INT_ERR_BISR_UNKN_TAG_ID		0x4306U

// /*********************** Protection specific Errors **************************/
// /************************** (0x4400) - (0x44FF) ******************************/

// /************************* Proc specific Errors ******************************/
// /************************** (0x4500) - (0x45FF) ******************************/

// /************************ Subsys specific Errors *****************************/
// /************************** (0x4600) - (0x46FF) ******************************/

// /************************* Power specific Errors *****************************/
// /************************** (0x4700) - (0x47FF) ******************************/

// /*********************** Power rail specific Errors **************************/
// /************************** (0x4800) - (0x48FF) ******************************/

// /*********************** PL Device specific Errors **************************/
// /************************** (0x4900) - (0x497F) ******************************/


// /*********************** AIE Device specific Errors **************************/
// /************************** (0x4980) - (0x49FF) ******************************/

// /*********************** Register blind write check errors********************/
// /************************** (0x5000) - (0x50FF) ******************************/


// /************************* Sysmon Specific Errors ****************************/
// /************************** (0x5100) - (0x51FF) ******************************/

// /**************************** VDU Specific Errors ****************************/
// /************************** (0x5200) - (0x52FF) ******************************/

/*************************** Undefined error *********************************/
#define XPM_INT_ERR_UNDEFINED			0xFFFFU

#define XPm_PrintDbgErr(Status, DbgErr)			\
	do {						\
		if (XST_SUCCESS != (Status)) {		\
			PmErr("0x%x\r\n", (DbgErr));	\
		}					\
	} while (XPM_FALSE_COND)

#endif /* XPM_DEBUG_H_ */
