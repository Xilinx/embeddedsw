/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file ipi.h
*
* This file contains definitions related to PSM IPI
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_IPI_H_
#define XPSMFW_IPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpsmfw_plat.h"

#define IPI_PSM_ISR    ( ( IPI_BASEADDR ) + ((u32)0x00010010U) ) /**< PSM Interrupt Status and Clear */
#define IPI_PSM_ISR_PMC_MASK	((u32)0x00000002U) /**< PSM Interrupt Status and Clear Mask for PMC IPI */

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_IPI_H_ */
