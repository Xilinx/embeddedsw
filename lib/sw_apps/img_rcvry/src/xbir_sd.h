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

/************************** Function Prototypes ******************************/
int Xbir_SdInit (void);
int Xbir_SdRead (u64 SrcAddr, u8 *DestAddr, u64 Length);
int Xbir_SdWrite (u8 *WrBuff, u64 Length);
int Xbir_SdErase(u64 Length);

#ifdef __cplusplus
}
#endif

#endif  /* XBIR_SD_H */