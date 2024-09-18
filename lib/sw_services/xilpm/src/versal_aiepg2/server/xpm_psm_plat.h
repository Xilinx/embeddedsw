/******************************************************************************
* Copyright (c) 2024, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_PSM_PLAT_H_
#define XPM_PSM_PLAT_H_

#include "xil_types.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* TODO: PSM keep alive needs to be removed from XPLM, then remove these */
#define XPM_PSM_COUNTER_DS_ID				(0x01U)
#define XPM_PSM_KEEP_ALIVE_STS_DS_ID			(0x02U)
/* TODO: Remove once PSM code is re-organized */
#define GLOBAL_CNTRL(BASE)	((BASE) + PSMX_GLOBAL_CNTRL)
#define PSMX_GLOBAL_CNTRL				(0x00000000U)
/******************************************************************************/


/* TODO: Remove all the macros underneath. Added for compilation only */
#define XPM_RPU_CPUHALT_MASK				BIT(0)
#define XPM_PSM_WAKEUP_MASK				BIT(2)
#define PROC_LOCATION_ADDRESS	(0xEBC26000U)
#define PROC_LOCATION_LENGTH	(0x2000U)
#define PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK	(0x00000010U)
#define XPM_MAX_POLL_TIMEOUT				(0x10000000U)
#define XPM_SET_PROC_LIST_PLAT	XPlmi_SetBufferList(PROC_LOCATION_ADDRESS, PROC_LOCATION_LENGTH)

maybe_unused static inline XStatus XPmPsm_SendPowerUpReq(XPm_Power *Power)
{
	(void)Power;
	return XST_INVALID_PARAM;
}
maybe_unused static inline XStatus XPmPsm_SendPowerDownReq(XPm_Power *Power)
{
	(void)Power;
	return XST_INVALID_PARAM;
}

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PSM_PLAT_H_ */
