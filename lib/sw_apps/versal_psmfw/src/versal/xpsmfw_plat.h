/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_plat.h
*
* This file contains definitions related to versal platform
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  sr   06/23/2022 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_PLAT_H
#define XPSMFW_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xpsmfw_default.h"
#include "psm_local.h"
#include "psm_global.h"

/**
 * IPI Base Address
 */
#define IPI_BASEADDR      (0XFF300000U)

#define UART0_BASEADDR (0xFF000000U)
#define UART1_BASEADDR (0xFF010000U)
#define IOMODULE_BASEADDR (0xFFC80000U)

void XPsmFw_SetIsFwPresent(void);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_PLAT_H_ */