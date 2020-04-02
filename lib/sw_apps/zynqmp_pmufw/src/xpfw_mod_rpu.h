/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPFW_MOD_RPU_H_
#define XPFW_MOD_RPU_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Macros for RPU_0 Status, Cfg and standby mode masks */
#define RPU_0_CFG_REG			0xFF9A0100U
#define RPU_0_STATUS_REG		0xFF9A0104U
#define RUN_MODE_MASK			0x6U
#define RPU_HALT_MASK			0x1U

/* Mask to know RPU_0 is powered down */
#define RPU_POWER_UP_MASK 		0x400U

/* Macros to indicate STL task started on PMU */
#define STL_STARTED		0x20000000U
#define CHECK_STL_STARTED 		100U
#define XPFW_RPU_RUNMODE_TIME 	100U

void ModRpuInit(void);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_MOD_RPU_H_ */
