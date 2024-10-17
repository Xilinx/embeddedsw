/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_ufs.h
*
* This is the header file which contains ufs declarations for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  sk  07/23/2024 Initial release for UFS Support
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XLOADER_UFS_H
#define XLOADER_UFS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_status.h"
#ifdef XLOADER_UFS
#include "xufspsxc.h"
#include "xplmi_debug.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_BASE_FILE_NAME_LEN_UFS		(20U)
#define XLOADER_LOGICAL_DRV_MASK		(0xFU)
#define XLOADER_LOGICAL_DRV_SHIFT		(16U)
#define XLOADER_SD_DRV_NUM_0			(0U)
#define XLOADER_SD_DRV_NUM_1			(1U)
#define XLOADER_UFS_DRV_NUM_2			(2U)
#define XLOADER_UFS_DRV_NUM_0			(0U)
#define XLOADER_SD_DRV_NUM_5			(5U)
#define XLOADER_SD_RAW_BLK_SIZE			(512U)
#define XLOADER_SD_CHUNK_SIZE		(0x200000U)
#define XLOADER_NUM_SECTORS		(0x1000U)

/************************** Function Prototypes ******************************/
int XLoader_UfsInit(u32 DeviceFlags);
int XLoader_UfsCopy(u64 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
int XLoader_UfsRelease(void);

/************************** Variable Definitions *****************************/

#endif /* end of XLOADER_UFS */

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_UFS_H */
