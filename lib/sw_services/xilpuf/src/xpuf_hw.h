/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_hw.h
*
* This file contains PUF hardware interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   kal  08/01/2019 Initial release
* 1.1   har  01/27/2020 Added offsets and register definitions for registers of
*                       EFUSE_CACHE module
*                       Moved definition of XPuf_ReadReg and XPuf_WriteReg to
*                       xpuf.c
*                       Renamed macros related to PUF Command register
*
* </pre>
*
******************************************************************************/
#ifndef XPUF_HW_H
#define XPUF_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_io.h"

/*************************** Constant Definitions *****************************/
/*
 * PMC_GLOBAL Base Address
 */
#define XPUF_PMC_GLOBAL_BASEADDR			(0xF1110000U)

#define XPUF_PMC_GLOBAL_PMC_BOOT_ERR_OFFSET		(0x00020100U)
#define XPUF_PMC_GLOBAL_PUF_CMD_OFFSET			(0x00040000U)
#define XPUF_PMC_GLOBAL_PUF_CFG0_OFFSET			(0x00040004U)
#define XPUF_PMC_GLOBAL_PUF_CFG1_OFFSET			(0x00040008U)
#define XPUF_PMC_GLOBAL_PUF_SHUT_OFFSET			(0x0004000CU)
#define XPUF_PMC_GLOBAL_PUF_STATUS_OFFSET		(0x00040010U)
#define XPUF_PMC_GLOBAL_PUF_WORD_OFFSET			(0x00040018U)
#define XPUF_PMC_GLOBAL_PUF_SYN_ADDR_OFFSET		(0x00040020U)
#define XPUF_PMC_GLOBAL_PUF_AUX_OFFSET			(0x00040024U)
#define XPUF_PMC_GLOBAL_PUF_CHASH_OFFSET		(0x00040028U)
#define XPUF_PMC_GLOBAL_PUF_CLEAR_OFFSET		(0x0004002CU)
#define XPUF_PMC_GLOBAL_PUF_ID_0_OFFSET			(0x00040030U)
#define XPUF_PMC_GLOBAL_PUF_SEG_STATUS_0_OFFSET		(0x00040050U)

/* PUF COMMAND register definition */
#define XPUF_CMD_NO_OP			(0x00U)
#define XPUF_CMD_REGISTRATION		(0x01U)
#define XPUF_CMD_REGEN_ON_DEMAND	(0x02U)
#define XPUF_CMD_REGEN_ID_ONLY		(0x03U)

/* PUF CFG0 register definition */
#define XPUF_CFG0_GLOBAL_FILTER_ENABLE		(0x01U)
#define XPUF_CFG0_HASH_SEL			(0x02U)

/* PUF CFG1 register definition */
#define XPUF_CFG1_INIT_VAL_4K			(0x0C230090U)
#define XPUF_CFG1_INIT_VAL_12K			(0x00230150U)

/* PUF Registration mode */
#define XPUF_REGISTRATION_MODE_4K		(0x0U)
#define XPUF_REGISTRATION_MODE_12K		(0x1U)

/* PUF SHUT register definition */
#define XPUF_SHUTTER_OPEN_TIME_MASK		(0xFFFFFFU)
#define XPUF_SHUTTER_OPEN_SET_SHIFT		(24U)

/* PUF STATUS register definition */
#define XPUF_STATUS_SYNDROME_WORD_RDY		(0x01U << 0U)
#define XPUF_STATUS_ID_ZEROIZED			(0x01U << 1U)
#define XPUF_STATUS_ID_RDY			(0x01U << 2U)
#define XPUF_STATUS_KEY_RDY			(0x01U << 3U)
#define XPUF_STATUS_PUF_DONE			(0x01U << 30U)

/* PUF TM STATUS register definition */
#define XPUF_TEST_MODE_STATUS_DONE		(0x01U)

/* PUF TM TR register definition */
#define XPUF_TEST_MODE_RESULT_FRR_TEST_PASSED	(0x01U << 0U)
#define XPUF_TEST_MODE_RESULT_ER_TEST_MASK	(0xFFU << 16U)
#define XPUF_TEST_MODE_RESULT_US_MASK		(0x03U << 24U)

/*
 * EFUSE_CACHE Base Address
 */
#define XPUF_EFUSE_CACHE_BASEADDR		(0xF1250000U)

#define XPUF_PUF_ECC_PUF_CTRL_OFFSET		(0x000000A4U)
#define XPUF_EFUSE_CACHE_SECURITY_CONTROL	(0x000000ACU)

/* EFUSE_CACHE PUF_ECC_PUF_CTRL register definition */
#define XPUF_PUF_REGEN_DIS			(1U << 31U)
#define XPUF_PUF_HD_INVLD			(1U << 30U)

/* EFUSE_CACHE SECURITY_CONTROL register definition */
#define XPUF_PUF_DIS				(1U << 18U)

/* Reset value of PUF_SYN_ADDR register */
#define XPUF_EFUSE_SYN_ADD_INIT			(0xF1250A04U)

/***************** Macros (Inline Functions) Definitions ********************/

#ifdef __cplusplus
}
#endif

#endif /* XPUF_HW_H */
