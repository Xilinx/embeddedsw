/******************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       pre  09/23/25 Fixed misrac violations
 * 1.2   pre  01/16/25 Updated comments
 *       pre  03/13/26 Added support to change TPM interface layer as per customer requirement
 *       pre  03/16/26 Added PCR reading support in TPM
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
#include "xtpm_defs.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XTPM_HASH_TYPE_SHA3			(48U) /**< Length of SHA3 hash in bytes */
#define XTPM_TPM_ROM_PCR_INDEX		(0U) /**< TPM PCR index for ROM measurement */
#define XTPM_TPM_PLM_PCR_INDEX		(1U) /**< TPM PCR index for PLM measurement */
#define XTPM_DATA_SIZE_INDEX		(5U) /**< Index for data size in PCR event command */
#define XTPM_DATA_WORD_LENGTH		(4U) /**< Data word length in bytes */
#define XTPM_ACCESS_TX_LENGTH		(1U) /**< Access command transmit length */
#define XTPM_PCR_EXTEND_INDEX		(13U) /**< Index for PCR number in PCR event command */
#define XTPM_PCR_EVENT_SIZE_INDEX	(28U) /**< Index for event data size in PCR event command */
#define XTPM_PCR_VALUE_START_INDEX  (30U) /**< Start index for PCR value in PCR read response */
#define XTPM_INDEX_0 (0U) /**< Index 0 */
#define XTPM_INDEX_1 (1U) /**< Index 1 */
#define XTPM_INDEX_2 (2U) /**< Index 2 */
#define XTPM_INDEX_3 (3U) /**< Index 3 */
#define XTPM_INDEX_4 (4U) /**< Index 4 */
#define XTPM_INDEX_5 (5U) /**< Index 5 */
#define XTPM_INDEX_6 (6U) /**< Index 6 */
#define XTPM_INDEX_7 (7U) /**< Index 7 */
#define XTPM_INDEX_8 (8U) /**< Index 8 */
#define XTPM_INDEX_9 (9U) /**< Index 9 */

#define XTPM_PCR_2	(2U) /**< Start PCR index for partition hash extend */
#define XTPM_PCR_23	(23U) /**< Final PCR index of TPM */

/************************** Function Prototypes ******************************/
u32 XTpm_Init(void);
int XTpm_MeasureRom(void);
int XTpm_MeasurePlm(void);
int XTpm_MeasurePartition(u32 PcrIndex, const u8* ImageHash);
u32 XTpm_InterfaceInit(void);
u32 XTpm_Transfer(u16 Address, const u8 *TxBuf, u8 *RxBuf, u16 Len);
u32 XTpm_DataTransfer(const u8* TxBuf, u8* RxBuf, u16 Txlen);
u32 XTpm_StatusGet(u8* StatusPtr);
u32 XTpm_StatusSet(u8 StatusVal);
u32 XTpm_AccessGet(u8* AccessPtr);
u32 XTpm_AccessSet(u8 Access);
u32 XTpm_Event(u32 PcrIndex, u16 size, const u8 *data, u8 *Response);
u32 XTpm_StartUp(void);
u32 XTpm_SelfTest(void);
u32 XTpm_PcrRead(u32 PcrIndex, u8 HashAlgo, u8 *Response);
u32 XTpm_ModuleInit(void);

#ifdef __cplusplus
}
#endif
#endif	/* PLM_TPM */
#endif	/* XTPM_H */
