/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef LPD_SLCR_SECURE_H_
#define LPD_SLCR_SECURE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * LPD_SLCR_SECURE Base Address
 */
#define LPD_SLCR_SECURE_BASEADDR      ((u32)0xFF4B0000U)

/**
 * Register: SLCR_USB
 */
#define SLCR_USB    ( ( LPD_SLCR_SECURE_BASEADDR ) + ((u32)0X00000034U) )

#define TZ_USB3_0_SHIFT		0U
#define TZ_USB3_0_MASK		((u32)0x00000001U)
#define TZ_USB3_1_SHIFT		1U
#define TZ_USB3_1_MASK		((u32)0x00000002U)

#ifdef __cplusplus
}
#endif


#endif /* _LPD_SLCR_SECURE_H_ */
