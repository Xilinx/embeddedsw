/******************************************************************************
* Copyright (c) 2017 - 2021 Xilinx, Inc.  All rights reserved.
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
*       skd  07/14/2020 XLoader_SbiCopy prototype changed
*       bsv  10/13/2020 Code clean up
* 1.04  bsv  08/31/2021 Code clean up
* 1.05  bsv  11/08/2021 Move XLoader_IsJtagSbiMode to Xilloader
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
#ifdef XLOADER_SBI

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XLoader_SbiInit(u32 DeviceFlags);
int XLoader_SbiCopy(u64 SrcAddr, u64 DestAddr, u32 Length, u32 Flags);
int XLoader_SbiRecovery(void);
u8 XLoader_IsJtagSbiMode(void);

/************************** Variable Definitions *****************************/

#endif /* end of XLOADER_SBI */

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_SBI_H */
