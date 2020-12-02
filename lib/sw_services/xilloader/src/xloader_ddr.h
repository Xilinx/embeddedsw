/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_ddr.h
*
* This is the header file which contains DDR interface declarations
* for the xilloader.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/12/2019 Initial release
* 1.01  kc   09/04/2019 Added support to use non blocking DMA in
*						DdrCopy function
* 1.02  bsv  04/09/2020 Code clean up of Xilloader
* 1.03  skd  07/14/2020 Added 64bit support for DDR source address
*       bsv  10/13/2020 Code clean up
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XLOADER_DDR_H
#define XLOADER_DDR_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XLoader_DdrInit(u32 DeviceFlags);
int XLoader_DdrCopy(u64 SrcAddr, u64 DestAddr, u32 Length, u32 Flags);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_DDR_H */
