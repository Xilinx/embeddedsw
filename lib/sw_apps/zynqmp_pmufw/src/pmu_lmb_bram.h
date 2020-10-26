/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef PMU_LMB_BRAM_H_
#define PMU_LMB_BRAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PMU_LMB_BRAM_ECC_STATUS_REG		0xFFD50000U
#define PMU_LMB_BRAM_ECC_IRQ_EN_REG		0xFFD50004U
#define PMU_LMB_BRAM_CE_CNT_REG			0xFFD5000CU

#define PMU_LMB_BRAM_CE_MASK			0x2U

#ifdef __cplusplus
}
#endif

#endif /* _PMU_LMB_BRAM_H_ */
