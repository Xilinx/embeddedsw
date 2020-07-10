/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_sbi.h
*
* This is the header file which contains qspi declarations for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   09/21/2017 Initial release
* 1.01  kc   04/09/2019 Added support for PCIe secondary boot mode and
*						 partial PDI load
*       kc   05/21/2019 Updated error code for partial PDI load
* 1.02  bsv  04/09/2020 Code clean up
* 1.03  bsv  07/07/2020 Remove unused functions
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XLOADER_SBI_H
#define XLOADER_SBI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xplmi_dma.h"
#include "xplmi_status.h"
#ifdef XLOADER_SBI
#include "xplmi_debug.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XLoader_SbiInit(u32 DeviceFlags);
int XLoader_SbiCopy(u32 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);

/************************** Variable Definitions *****************************/

#endif /* end of XLOADER_SBI */

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_SBI_H */
