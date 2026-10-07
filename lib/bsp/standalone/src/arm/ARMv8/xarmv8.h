/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
* @file xarmv8.h
* @brief ARMv8 processor register definitions and hardware configuration.
*/

#ifndef XIL_XARMV8_H_
#define XIL_XARMV8_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#ifdef SDT
#include "xil_types.h"

/**************************** Type Definitions ******************************/
/** @brief ARM v8 processor configuration parameters. */
typedef struct {
		u32 TimestampFreq; /**< Frequency of the ARM generic timer in Hz. */
		u32 CpuFreq; /**< CPU clock frequency in Hz. */
		u32 CpuId;	/**< CPU Number */
} XARMv8_Config;
#endif
#ifdef __cplusplus
}
#endif
#endif /* XIL_XARMV8_H_ */
