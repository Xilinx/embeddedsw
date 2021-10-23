/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xbir_qspi.h
*
* This is the qspi header file which contains definitions for the qspi.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.00  bsv   07/02/20   First release
*
* </pre>
*
******************************************************************************/

#ifndef XBIR_QSPI_H
#define XBIR_QSPI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
#define XBIR_QSPI_NO_IMG_ERASED	(0xFFU)

/**************************** Type Definitions *******************************/
typedef struct {
	u32 NumOfSectorsErased;
	u32 TotalNumOfSectors;
	u32 SectorSize;
	u8 CurrentImgErased;
	u8 State;
} Xbir_FlashEraseStats;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int Xbir_QspiInit (void);
int Xbir_QspiRead (u32 SrcAddr, u8 *DestAddr, u32 Length);
int Xbir_QspiWrite(u32 Addr, u8 *WrBuff, u32 Len);
int Xbir_QspiFlashErase(u32 Address, u32 Length);
int Xbir_QspiWrite(u32 Address, u8 *WrBuffer, u32 Length);
void Xbir_QspiGetPageSize(u16 *PageSize);
void Xbir_QspiGetSectorSize(u32 *SectorSize);
void Xbir_QspiEraseStatsInit(void);
Xbir_FlashEraseStats* Xbir_GetFlashEraseStats(void);

#ifdef __cplusplus
}
#endif

#endif  /* XBIR_QSPI_H */
