/******************************************************************************
* Copyright (C) 2001 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xspi_i.h
* @addtogroup spi Overview
* @{
*
* This header file contains internal identifiers. It is intended for internal
* use only.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rpm  10/11/01 First release
* 1.00b jhl  03/14/02 Repartitioned driver for smaller files.
* 1.00b rpm  04/24/02 Moved register definitions to xspi_l.h
* 1.11a wgr  03/22/07 Converted to new coding style.
* 1.12a sv   03/28/08 Removed the Macro for statistics, moved the interrupt
*                     register definitions and bit definitions to _l.h.
* 2.00a sv   07/30/08 Removed the Macro for statistics, moved the interrupt
*                     register definitions and bit definitions to _l.h.
* </pre>
*
******************************************************************************/

#ifndef XSPI_I_H		/* prevent circular inclusions */
#define XSPI_I_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xspi_l.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

void XSpi_Abort(XSpi *InstancePtr);

/************************** Variable Definitions *****************************/

extern XSpi_Config XSpi_ConfigTable[];

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
