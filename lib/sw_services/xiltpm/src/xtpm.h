/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xtpm.h
 *
 * This is the header file which contains TPM specific definitions for PLM.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  tri  03/13/25 Initial release
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

#ifndef XTPM_H
#define XTPM_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_TPM
#include "xil_io.h"
#include "xspips.h"
#include "xparameters.h"
#include "xplmi_debug.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XTPM_HASH_TYPE_SHA3			(48U)
#define XTPM_TPM_ROM_PCR_INDEX		(0U)
#define XTPM_TPM_PLM_PCR_INDEX		(1U)
#define XTPM_DATA_SIZE_INDEX		(5U)
#define XTPM_DATA_WORD_LENGTH		(4U)
#define XTPM_ACCESS_TX_LENGTH		(1U)
#define XTPM_PCR_EXTEND_INDEX		(13U)

/************************** Function Prototypes ******************************/
u32 XTpm_Init(void);
int XTpm_MeasureRom(void);
int XTpm_MeasurePlm(void);
int XTpm_MeasurePartition(u32 PcrIndex, u8* ImageHash);
u32 XTpm_SpiInit(void);
u32 XTpm_Transfer(u16 Address, u8 *TxBuf, u8 *RxBuf, u16 Length);
u32 XTpm_DataTransfer(u8* TxBuf, u8* RxBuf, u16 Txlen);
u32 XTpm_StatusGet(u8* StatusPtr);
u32 XTpm_StatusSet(u8 StatusVal);
u32 XTpm_AccessGet(u8* AccessPtr);
u32 XTpm_AccessSet(u8 Access);
u32 XTpm_Event(u32 pcr_number, u16 size, u8 *data, u8 *resp);

#ifdef __cplusplus
}
#endif
#endif	/* PLM_TPM */
#endif	/* XTPM_H */
/* @} */
