/******************************************************************************
* Copyright (c) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_sd.h
*
* This is the header file which contains SD declarations for XilLoader.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   09/21/2017 Initial release
* 1.01  bsv  06/26/2019 Added secondary boot support
*       bsv  02/12/2020 Added support for SD/eMMC raw boot mode
*       bsv  02/23/2020 Added multi partition support for SD/eMMC FS boot modes
*       bsv  03/14/2020 Added eMMC0 FS and raw boot mode support
*       bsv  03/17/2020 Changes relatd to multiple partitions in SD/eMMC boot
*       bsv  02/04/2020 Reset file system instance in init functions for LPD off
*						suspend and resume to work
* 1.02  bsv  04/09/2020 Code clean up
*       bsv  04/28/2020 Changed SD drive number to 5 when both SD0 and SD1 are
*						in design
* 1.03  bsv  07/01/2020 Unmount file system after loading PDIs
*       skd  07/14/2020 Added 64bit support for SD copy destination address
*       bsv  07/16/2020 Force Cdn bit to 1 to improve performance
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       bsv  09/04/2020 Updated XLOADER_BASE_FILE_NAME_LEN_SD_1 macro value
*       bsv  10/13/2020 Code clean up
* 1.04  bm   12/15/2020 Removed XLOADER_SD_MAX_BOOT_FILES_LIMIT macro
* 1.05  bsv  08/31/2021 Code clean up
*       dd   09/11/2023 MISRA-C violation Rule 17.8 fixed
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XLOADER_SD_H
#define XLOADER_SD_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_status.h"

#if defined(XLOADER_SD_0) || defined(XLOADER_SD_1)
#include "xsdps.h"
#include "xplmi_debug.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_BASE_FILE_NAME_LEN_SD		(20U)
#define XLOADER_LOGICAL_DRV_MASK		(0xFU)
#define XLOADER_LOGICAL_DRV_SHIFT		(16U)
#define XLOADER_SD_DRV_NUM_0			(0U)
#define XLOADER_SD_DRV_NUM_1			(1U)
#define XLOADER_SD_DRV_NUM_5			(5U)
#define XLOADER_SD_RAW_BLK_SIZE			(512U)
#define XLOADER_SD_CHUNK_SIZE		(0x200000U)
#define XLOADER_NUM_SECTORS		(0x1000U)

/************************** Function Prototypes ******************************/
int XLoader_SdInit(u32 DeviceFlagsVal);
int XLoader_SdCopy(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags);
int XLoader_SdRelease(void);
int XLoader_RawInit(u32 DeviceFlags);
int XLoader_RawCopy(u64 SrcAddress, u64 DestAddress, u32 Len, u32 Flags);
int XLoader_RawRelease(void);

/************************** Variable Definitions *****************************/

#endif /* end of XLOADER_SD */

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_SD_H */
