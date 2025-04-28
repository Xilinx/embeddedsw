/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xfsbl_tpm.h
*
* This is the header file which contains TPM specific definitions for FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bsv  04/01/21 Initial release
* 2.00  sd   04/25/25 Fix incorrect TPM initialization
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XFSBL_TPM_H
#define XFSBL_TPM_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"

#ifdef XFSBL_TPM
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef XFSBL_HASH_TYPE_SHA3
#define XFSBL_HASH_TYPE_SHA3			(48U)
#endif

#define XFSBL_OCM_START_ADDR		(0xFFFC0000U)
#define XFSBL_HASH_LENGTH_IN_WORDS	(XFSBL_HASH_TYPE_SHA3 / 4U)
#define XFSBL_SHA3_BLOCK_LEN		(104U)
#define	XFSBL_START_KECCAK_PADDING_MASK		(0x1U)
#define XFSBL_END_KECCAK_PADDING_MASK		(0x80U)

#define XFSBL_TPM_CSU_ROM_PCR_INDEX		(0U)
#define XFSBL_TPM_FSBL_PCR_INDEX		(1U)
#define XFSBL_TPM_ATF_PCR_INDEX			(2U)
#define XFSBL_TPM_UBOOT_PCR_INDEX		(3U)
#define XFSBL_TPM_PMU_PCR_INDEX			(4U)
#define XFSBL_TPM_PL_PCR_INDEX			(6U)
#define XFSBL_TPM_DATA_SIZE_INDEX	(5U)
#define XFSBL_TPM_PCR_EXTEND_INDEX	(13U)

#define	XFSBL_TPM_DID_VID			(0x0F00U)
#define	XFSBL_TPM_RID				(0x0F04U)
#define	XFSBL_TPM_DID_VID_LEN			(0x4U)
#define	XFSBL_TPM_RID_LEN				(0x1U)
#define XFSBL_TPM_SPI_SELECT		(0x00U)
#define XFSBL_TPM_RST_DELAY		(1000U)
#define XFSBL_TPM_INIT_DELAY		(2000000U)
#define	XFSBL_TPM_ACCESS_VALID		(0x80U)
#define	XFSBL_TPM_ACCESS_ACT_LOCAL	(0x20U)
#define	XFSBL_TPM_ACCESS_REQ_USE	(0x02U)
#define	XFSBL_TPM_STS_VALID 		(0x80U)
#define	XFSBL_TPM_STS_CMD_READY		(0x40U)
#define	XFSBL_TPM_STS_GO 		(0x20U)
#define	XFSBL_TPM_STS_DATA_AVAIL	(0x10U)
#define XFSBL_TPM_TX_HEAD_SIZE		(4U)
#define XFSBL_TPM_RX_HEAD_SIZE		(10U)
#define XFSBL_TPM_PCR_EXT_CMD_SIZE	(33U)
#define XFSBL_TPM_PCR_EVENT_CMD_SIZE	(29U)
#define XFSBL_TPM_SPI_MAX_SIZE		(64U)
#define XFSBL_TPM_MAX_INDEX			(24U)
#define XFSBL_TPM_PCR_READ_COMMAND_SIZE	(20U)
#define XFSBL_TPM_HASH_ALG_OFFSET		(15U)
#define XFSBL_TPM_PCR_SELECT_OFFSET		(17U)
#define XFSBL_TPM_PCR_DATA_OFFSET		(30U)
#define XFSBL_TPM_PCR_DIGEST_SIZE		(32U)
#define XFSBL_TPM_RESP_MIN_SIZE			(128U)
#define XFSBL_TPM_HASH_ALG_SHA1			(0x04U)
#define XFSBL_TPM_HASH_ALG_SHA256		(0x0BU)
#define XFSBL_TPM_DIVD_VAL				(0x08U)
/* Maximum possible TPM request size in bytes */
#define XFSBL_TPM_REQ_MAX_SIZE		(1024U)
#define XFSBL_TPM_DIGEST_SIZE		(32U)
/* Maximum possible TPM response size in bytes */
#define XFSBL_TPM_RESP_MAX_SIZE	(4096U)
#define	XFSBL_TPM_ACCESS 	(0x00U)
#define	XFSBL_TPM_STS		(0x18U)
#define	XFSBL_TPM_DATA_FIFO	(0x24U)

/************************** Function Prototypes ******************************/
u32 XFsbl_TpmInit(void);
u32 XFsbl_TpmMeasureRom(void);
u32 XFsbl_TpmMeasurePartition(u8 PcrIndex, u8* PartitionHash);
u32 XFsbl_SpiInit(void);
u32 XFsbl_TpmTransfer(u16 Address, u8 *TxBuf, u8 *RxBuf, u16 Length);
u32 XFsbl_TpmDataTransfer(u8* TxBuf, u8* RxBuf, u16 Txlen);
u32 XFsbl_TpmStatusGet(u8* StatusPtr);
u32 XFsbl_TpmStatusSet(u8 StatusVal);
u32 XFsbl_TpmAccessGet(u8* AccessPtr);
u32 XFsbl_TpmAccessSet(u8 Access);
u32 XFsbl_TpmEvent(u32 PcrIndex, u16 Size, u8 *Data, u8 *Response);
void XFsbl_ReadAllTpmRegisters(void);

#endif /* XFSBL_TPM */

#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_TPM_H */
