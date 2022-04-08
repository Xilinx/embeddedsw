/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PSM_H_
#define XPM_PSM_H_

#include "xil_types.h"
#ifdef __cplusplus
extern "C" {
#endif

/* PSM Global Registers */
#define PSM_GLOBAL_CNTRL				(0x00000000U)
#define XPM_MAX_POLL_TIMEOUT				(0x10000000U)
#define PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK	(0x00000010U)
#define XPM_PSM_WAKEUP_MASK				BIT(2)
#define XPM_RPU_NCPUHALT_MASK		BIT(0)
#define XPM_DOMAIN_INIT_STATUS_REG		PMC_GLOBAL_PERS_GLOB_GEN_STORAGE0
u32 XPmPsm_FwIsPresent(void);

/* PSM MODULE Data Structures IDs */
#define XPM_PSM_COUNTER_DS_ID				(0x01U)
#define XPM_PSM_KEEP_ALIVE_STS_DS_ID			(0x02U)

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PSM_H_ */
