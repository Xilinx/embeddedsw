/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_sd.h
*
* This is the header file which contains qspi declarations for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2018 Initial release
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
#include "xplmi_hw.h"
#include "xplmi_status.h"

#if defined(XLOADER_SD_0) || defined(XLOADER_SD_1)
#include "xsdps.h"
#include "xplmi_debug.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_BASE_FILE_NAME_LEN_SD_1 	(11U)
#define XLOADER_NUM_DIGITS_IN_FILE_NAME 	(4U)
#define XLOADER_SD_SBD_ADDR_SET_MASK		(0x100U)
#define XLOADER_SD_SBD_ADDR_SHIFT			(0x9U)
#define XLOADER_LOGICAL_DRV_MASK		(0xF0000U)
#define XLOADER_LOGICAL_DRV_SHIFT		(16U)
#define XLOADER_SD_SBD_ADDR_MASK		(0xFFFFU)
#define XLOADER_SD_DRV_NUM_0			(0U)
#define XLOADER_SD_DRV_NUM_1			(1U)
#define XLOADER_SD_DRV_NUM_5			(5U)
#define XLOADER_SD_RAW_BLK_SIZE			(512U)
#define XLOADER_SD_RAW_NUM_SECTORS		(128U)

/************************** Function Prototypes ******************************/
int XLoader_SdInit(u32 DeviceFlags);
int XLoader_SdCopy(u32 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
int XLoader_SdRelease(void);
int XLoader_RawInit(u32 DrvNum);
int XLoader_RawCopy(u32 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);

/************************** Variable Definitions *****************************/

#endif /* end of XLOADER_SD */

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_SD_H */
