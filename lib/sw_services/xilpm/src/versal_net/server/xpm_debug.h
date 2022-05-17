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
/* Invalid power domain ID error */
#define XPM_INT_ERR_INVALID_PWR_DOMAIN		0x3000U
/* Invalid state */
#define XPM_INT_ERR_INVALID_STATE		0x3003U
/* Power domain state is OFF */
#define XPM_INT_ERR_PWR_DOMAIN_OFF		0x3007U

// /*********************** Generic House Cleaning Errors ***********************/
// /*************************** (0x3100) - (0x31FF) *****************************/
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
/* Error during device init */
#define XPM_INT_ERR_DEVICE_INIT			0x3305U
/* Error during Add requirement of device*/
#define XPM_INT_ERR_ADD_REQUIREMENT		0x3306U
/* Error during add clock */
#define XPM_INT_ERR_ADD_CLK			0x3307U
/* Error during add reset node */
#define XPM_INT_ERR_ADD_RST			0x3308U
/* Error during set device node */
#define XPM_INT_ERR_SET_DEV_NODE		0x3309U
/* Error during get device permission */
#define XPM_INT_ERR_GET_DEVICE_PERMISSION	0x330CU
/* Subsystem has no permission to that device */
#define XPM_INT_ERR_DEVICE_PERMISSION		0x330DU
/* Shared resource has no permission to access */
#define XPM_INT_ERR_SHARED_RESOURCE		0x330EU
/* Device is busy */
#define XPM_INT_ERR_DEVICE_BUSY			0x330FU
/* Error during power up device parent power domain */
#define XPM_INT_ERR_DEVICE_PWR_PARENT_UP	0x3310U
/* Error during device request */
#define XPM_INT_ERR_DEVICE_REQUEST		0x3311U
/* Error during device release */
#define XPM_INT_ERR_DEVICE_RELEASE		0x3312U
/* Error during device change state */
#define XPM_INT_ERR_DEVICE_CHANGE_STATE	0x3313U

// /************************* Reset specific Errors *****************************/
// /************************** (0x3400) - (0x34FF) ******************************/
/* Invalid reset state */
#define XPM_INT_ERR_RST_STATE			0x3401U
/* Error during reset release */
#define XPM_INT_ERR_RST_RELEASE			0x3402U
/* Error during reset assert */
#define XPM_INT_ERR_RST_ASSERT			0x3403U
/* Error during set reset node */
#define XPM_INT_ERR_SET_RESET_NODE		0x3404U

// /************************** FPD specific Errors ******************************/
// /************************** (0x3500) - (0x35FF) ******************************/

// /************************* Miscellaneous Errors ******************************/
// /************************** (0x3600) - (0x36FF) ******************************/
/* PSMFW is not present */
#define XPM_INT_ERR_PSMFW_NOT_PRESENT		0x3602U
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
/* Invalid subclass */
#define XPM_INT_ERR_INVALID_SUBCLASS		0x360BU
/* Buffer too small */
#define XPM_INT_ERR_BUFFER_TOO_SMALL		0x360CU
/* Invalid event */
#define XPM_INT_ERR_INVALID_EVENT		0x360DU
/* Invalid state transition */
#define XPM_INT_ERR_INVALID_STATE_TRANS		0x3611U
/* Error during parsing isolation node. getting invalid format */
#define XPM_INT_ERR_ISO_INVALID_FORMAT		0x3613U
/* A topology iso node has exceed number of dependencies */
#define XPM_INT_ERR_ISO_MAX_DEPENDENCIES	0X3614U

// /************************** PLD specific Errors ******************************/
// /************************** (0x3700) - (0x37FF) ******************************/

// /************************** NPD specific Errors ******************************/
// /************************** (0x3800) - (0x38FF) ******************************/

// /************************* Clock specific errors *****************************/
// /************************** (0x3900) - (0x39FF) ******************************/
/* Error during clock enable */
#define XPM_INT_ERR_CLK_ENABLE			0x3900U
/* Max clock parents error */
#define XPM_INT_ERR_MAX_CLK_PARENTS		0x3901U
/* Error during clock init */
#define XPM_INT_ERR_CLK_INIT			0x3902U
/* Topology nodes greater than max number of nodes */
#define XPM_INT_ERR_CLK_TOPOLOGY_MAX_NUM_NODES	0x3903U
/* Invalid clock type */
#define XPM_INT_ERR_INVALID_CLK_TYPE		0x3904U
/* Invalid clock parent */
#define XPM_INT_ERR_INVALID_CLK_PARENT		0x3905U
/* Read only clock */
#define XPM_INT_ERR_READ_ONLY_CLK		0x3906U
/* PLL permission error as PLLs are shared */
#define XPM_INT_ERR_PLL_PERMISSION		0x3907U
/* Error during clock disable */
#define XPM_INT_ERR_CLK_DISABLE			0x3908U
/* Error enabling CPM_TOPSW_REF clk */
#define XPM_INT_ERR_CPM_TOPSW_REF_CLK_ENABLE	0x3909U
/* Error enabling USB clk */
#define XPM_INT_ERR_USB_CLK_ENABLE		0x390AU
/* Error enabling CAN_0 clk */
#define XPM_INT_ERR_CAN0_CLK_ENABLE		0x390BU
/* Error enabling CAN_1 clk */
#define XPM_INT_ERR_CAN1_CLK_ENABLE		0x390CU

// /************************** CPM specific Errors ******************************/
// /************************** (0x4000) - (0x40FF) ******************************/
/* Invalid subsystem ID */
#define XPM_INT_ERR_INVALID_SUBSYSTEMID		0x4600U
/* Subsystem not allowed to access clock */
#define XPM_INT_ERR_CLOCK_PERMISSION		0x4601U
/* Subsystem not allowed to access reset */
#define XPM_INT_ERR_RESET_PERMISSION		0x4602U
/* Subsystem not allowed to access pin */
#define XPM_INT_ERR_PIN_PERMISSION		0x4603U
/* Subsystem already added */
#define XPM_INT_ERR_SUBSYS_ADDED		0x4604U
/* Error during set subsystem state */
#define XPM_INT_ERR_SUBSYS_SET_STATE		0x4605U
/* Subsystem not allowed access */
#define  XPM_INT_ERR_SUBSYS_ACCESS		0x4606U

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
/* Error during core init */
#define XPM_INT_ERR_CORE_INIT			0x4501U

// /************************ Subsys specific Errors *****************************/
// /************************** (0x4600) - (0x46FF) ******************************/

// /************************* Power specific Errors *****************************/
// /************************** (0x4700) - (0x47FF) ******************************/
/* Error during parent power up */
#define XPM_INT_ERR_PWR_PARENT_UP		0x4700U
/* Error during PSM power up request */
#define XPM_INT_ERR_PSM_PWR_UP			0x4701U
/* Error during PSM power down request */
#define XPM_INT_ERR_PSM_PWR_DWN			0x4702U

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
