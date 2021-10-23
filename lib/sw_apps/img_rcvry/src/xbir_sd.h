/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xbir_sd.h
*
* This is the SD header file which contains declarations for SD.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.00  bsv   07/25/21   First release
*
* </pre>
*
******************************************************************************/

#ifndef XBIR_SD_H
#define XBIR_SD_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XBIR_SDPS_BLOCK_SIZE		(0x200U)
#define XBIR_SDPS_CHUNK_SIZE	(0x100000U)
#define XBIR_SD_ERASE_NUM_CHUNKS	(0x400U)
#define XBIR_SD_RAW_NUM_SECTORS		(0x800U)

/************************** Function Prototypes ******************************/
int Xbir_SdInit (u8 DrvNum);
int Xbir_SdRead (u32 SrcAddr, u8 *DestAddr, u32 Length);
int Xbir_SdWrite(u32 Offset, u8 *WrBuffer, u32 Length);
int Xbir_SdErase(u32 Offset, u32 Length);

#ifdef __cplusplus
}
#endif

#endif  /* XBIR_SD_H */