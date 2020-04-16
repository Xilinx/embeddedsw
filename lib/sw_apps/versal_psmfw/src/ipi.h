/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
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

/**
 * IPI Base Address
 */
#define IPI_BASEADDR      ((u32)0XFF300000U)

#define IPI_PSM_ISR    ( ( IPI_BASEADDR ) + ((u32)0x00010010U) )

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_IPI_H_ */
