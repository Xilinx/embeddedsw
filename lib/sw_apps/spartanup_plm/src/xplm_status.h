/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_status.h
 *
 * This is the header file which contains status codes.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/

#ifndef XPLM_STATUS_H
#define XPLM_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/* Error codes */
typedef enum {
	/** 0x901 - Error when DMA driver lookup fails. */
	XPLM_ERR_DMA_LOOKUP = 0x901,

	/** 0x902 - Error when DMA driver config fails. */
	XPLM_ERR_DMA_CFG,

	/**
	 * 0x903 - Error when DMA Self test fails. It occurs when DMA is in
	 * reset and PLM tries to initialize it.
	 */
	XPLM_ERR_DMA_SELFTEST,

	/** 0x904 - Error if DMA transfer failed waiting for done. */
	XPLM_ERR_DMA_XFER_WAIT,

	/** 0x905 - Error if tried to process unregistered API ID */
	XPLM_ERR_CMD_APIID,

	/** 0x906 - Error if handler is not registered for the given CDO cmd */
	XPLM_ERR_CMD_HANDLER_NULL,

	/** 0x907 - Error if the given CDO cmd is not registered */
	XPLM_ERR_MODULE_NOT_REGISTERED,

	/** 0x908 - Error if the handler is not assigned */
	XPLM_ERR_INVLD_RESUME_HANDLER,

	/** 0x909 - Error if failed to initialize the CDO instance */
	XPLM_ERR_INIT_CDO_INSTANCE,


	/**
	 * 0x90A - Error when valid CDO header ID is not present in CDO header.
	 * Can happen when different partition type is processed as CDO
	 */
	XPLM_ERR_CDO_HDR_ID,

	/**
	 * 0x90B - Error when CDO header checksum is wrong.
	 * Can happen when CDO header is corrupted
	 */
	XPLM_ERR_CDO_CHECKSUM,

	/** 0x90C - Error if DMA Xfer failed in Src Channel wait for done */
	XPLM_ERR_DMA_XFER_WAIT_SRC,

	/** 0x90D - Error if DMA Xfer failed in Dest Channel wait for done */
	XPLM_ERR_DMA_XFER_WAIT_DEST,

	/** 0x90E - Error copying CDO CMD to buffer. */
	XPLM_ERR_MEMCPY_CMD_EXEC,

	/**
	 * 0x90F - Error when the break length required to jump is less
	 * than the processed CDO length
	 */
	XPLM_ERR_INVALID_BREAK_LENGTH,

	/** 0x910 - Error if OSPI flash make is not supported. */
	XPLM_ERR_UNSUPPORTED_OSPI_FLASH_MAKE,

	/** 0x911 - Error if OSPI flash size is not supported. */
	XPLM_ERR_UNSUPPORTED_OSPI_SIZE,

	/**
	 * 0x912 - Error when OSPI driver lookup fails.
	 * Obversed when OSPI is not selected in CIPS
	 */
	XPLM_ERR_OSPI_CFG,

	/** 0x913 - Error when OSPI driver CFG fails */
	XPLM_ERR_OSPI_INIT,

	/**
	 * 0x914 - Error when OSPI driver is unable to select flash CS0.
	 * Check minor code for OSPI driver error code
	 */
	XPLM_ERR_OSPI_SEL_FLASH_CS0,

	/** 0x915 - Error if failed to read OSPI flash ID. */
	XPLM_ERR_OSPI_READ_ID,

	/** 0x916 - Error if failed to read the data from flash. */
	XPLM_ERR_OSPI_READ_DATA,

	/** 0x917 - Error if failed to enable write mode in flash. */
	XPLM_ERR_OSPI_SET_DDR_WRITE_ENABLE,

	/** 0x918 - Error if failed to configure DDR mode in flash. */
	XPLM_ERR_OSPI_SET_DDR_WRITE_CFG_REG,

	/** 0x919 - Error if failed to enable dual byte op code. */
	XPLM_ERR_OSPI_SET_DDR_DUAL_BYTE_OP_ENABLE,

	/** 0x91A - Error if failed to set the controller to DDR PHY mode */
	XPLM_ERR_OSPI_SET_DDR_PHY,

	/** 0x91B - Error if failed to readback configuraion from flash. */
	XPLM_ERR_OSPI_SET_DDR_READ_CFG_REG,

	/**
	 * 0x91C - Error if failed to validate configuration after enabling
	 * DDR PHY mode.
	 */
	XPLM_ERR_OSPI_SET_DDR_CFG_MISMATCH,

	/** 0x91D - Error if failed to disable DUAL BYTE OP code. */
	XPLM_ERR_OSPI_DUAL_BYTE_OP_DISABLE,

	/** 0x91E - Error if 0SPI connection mode is other than single. */
	XPLM_ERR_OSPI_SINGLE_CONN_MODE,

	/** 0x91F - Error if failed to set the controller to SDR NON PHY mode */
	XPLM_ERR_OSPI_SET_SDR_NON_PHY,

	/** 0x920 - Error if failed to set the controller to SDR PHY mode */
	XPLM_ERR_OSPI_SET_SDR_PHY,

	/** 0x921 - Error when source address in OSPI copy exceeds flash size */
	XPLM_ERR_OSPI_COPY_LENGTH_OVERFLOW,

	/** 0x922 - Error if failed to enable write mode in flash. */
	XPLM_ERR_OSPI_4B_ADDR_MODE_WRITE_ENABLE,

	/** 0x923 - Error if failed to enter or exit 4 byte addressing mode. */
	XPLM_ERR_OSPI_4B_ADDR_MODE_ENTER_OR_EXIT_CMD,

	/** 0x924 - Error if failed to verify 4 byte addressing mode. */
	XPLM_ERR_OSPI_4B_ADDR_MODE_ENTER_OR_EXIT_CMD_STATUS,

	/** 0x925 - Error if failed disable write mode in flash. */
	XPLM_ERR_OSPI_4B_ADDR_MODE_WRITE_DISABLE,

	/** 0x926 - Error if failed to clean AES key. */
	XPLM_ERR_AES_KEY_CLEAR_TIMEOUT,

	/** 0x927 - Error if the ref clk divisor in RTCA sets more than 6-bits. */
	XPLM_ERR_RTCA_OSPI_CLK_DIV_MAX,

	/** 0x928 - Error if the OSPI configuration is set to DDR and NON_PHY mode. */
	XPLM_ERR_RTCA_OSPI_INVLD_DDR_PHY_CFG,

	/** 0x929 - Error if failed to read QSPI flash ID. */
	XPLM_ERR_QSPI_READ_ID,

	/** 0x92A - Error if QSPI flash make is not supported. */
	XPLM_ERR_QSPI_FLASH_MAKE_NOT_SUPPORTED,

	/** 0x92B - Error if QSPI flash size is not supported. */
	XPLM_ERR_QSPI_FLASH_SIZE_NOT_SUPPORTED,

	/**
	 * 0x92C - Error when QSPI driver lookup fails.
	 * Obversed when QSPI is not selected in CIPS.
	 */
	XPLM_ERR_QSPI_CFG_NOT_FOUND,

	/** 0x92D - Error when QSPI driver CFG fails. */
	XPLM_ERR_QSPI_CFG_INIT,

	/** 0x92E - Error if QSPI driver failed to select flash. */
	XPLM_ERR_QSPI_FLASH_CS,

	/** 0x92F - Error if QSPI connection mode is other than single. */
	XPLM_ERR_QSPI_SINGLE_CONN_MODE,

	/** 0x930 - Error when source address in OSPI copy exceeds flash size */
	XPLM_ERR_QSPI_COPY_LENGTH_OVERFLOW,

	/** 0x931 - Error if failed to read the data from flash. */
	XPLM_ERR_QSPI_READ,

	/** 0x932 - Error when QSPI read length is greater than flash size */
	XPLM_ERR_QSPI_LENGTH,

	/** 0x933 - Error if QSPI bus width is not supported. */
	XPLM_ERR_QSPI_BUS_WIDTH_NOT_SUPPORTED,

	/** 0x934 - Error if failed to select the flash bank for read. */
	XPLM_ERR_QSPI_READ_BANK_SEL,

	/** 0x935 - Error if failed to select the flash bank*/
	XPLM_ERR_QSPI_BANK_SEL,

	/** 0x936 - Error if failed to enable write mode during flash bank select. */
	XPLM_ERR_QSPI_BANK_SEL_SEND_WREN,

	/**
	 * 0x937 - Error if failed to extended address write command
	 * for spansion flash make.
	 */
	XPLM_ERR_QSPI_BANK_SEL_SEND_EXT_ADDR,

	/** 0x938 - Error if failed to verify the extended address write cmd. */
	XPLM_ERR_QSPI_BANK_SEL_VERIFY_EXT_ADDR,

	/** 0x939 - Error if the QSPI configuration in RTCA is set to DDR mode */
	XPLM_ERR_RTCA_QSPI_INVLD_DDR_CFG,

	/* IO module */
	/** 0x93A - Error if failed to initialize IO module. */
	XPLM_ERR_IO_MOD_INIT,

	/** 0x93B - Error if IO module self test failed. */
	XPLM_ERR_IO_MOD_SELF_TEST,

	/* Assert */
	/** 0x93C - Assertion error */
	XPLM_ERR_ASSERT,

	/* PPDI */
	/** 0x93D - Error if failed to initialize ROM instance to zero. */
	XPLM_ERR_PPDI_ROM_INST_ZEROIZE,

	/** 0x93E - Error if failed to initialize boot header table to zero. */
	XPLM_ERR_PPDI_BH_ZEROIZE,

	/** 0x93F - Error if failed to read boot header from SBI buffer. */
	XPLM_ERR_PPDI_SBI_BUF_READ_BH,

	/** 0x940 - Error if failed to validate Boot Header checksum */
	XPLM_ERR_CALC_BH_CHECKSUM,

	/** 0x941 - Error if failed to initialize hash block to zero. */
	XPLM_ERR_HASH_BLOCK_ZEROIZE,

	/** 0x942 - Error if failed to read hash block from SBI buffer. */
	XPLM_ERR_HASH_BLOCK_READ_SBI_BUF,

	/** 0x943 - Error if failed to read GCM TAG from SBI buffer */
	XPLM_ERR_GCM_TAG_READ_SBI_BUF,

	/** 0x944 - Error if failed to authenticate Hash block */
	XPLM_ERR_HASH_BLOCK_AUTHENTICATION,

	/** 0x945 - Error if failed to validate "XLNX" image identification word. */
	XPLM_ERR_PPDI_INVALID_IMG_DET_WORD,

	/** 0x946 - Error if the secondary boot interface configured it not supported. */
	XPLM_ERR_INVALID_SECONDARY_BOOT_INTF,

	/** 0x947 -   */
	XPLM_ERR_PPDI_DEC_EFUSE_ONLY,

	/** 0x948 - Error if encryption source or type is not supported. */
	XPLM_ERR_PPDI_INVALID_BH_ENC_STATUS,

	/** 0x949 - Error if failed to validate A-HWROT. */
	XPLM_ERR_PPDI_AHWROT_UNSIGNED_IMG,

	/**
	 * 0x94A - Error if partial PDI authentication status is different
	 * from full PDI authentication status.
	 */
	XPLM_ERR_PPDI_SEC_TRANSITION_AUTH,

	/**
	 * 0x94B - Error if partial PDI encryption source or type does
	 * not match with full PDI.
	 */
	XPLM_ERR_PPDI_SEC_TO_SEC_KEY_MISMATCH,

	/** 0x94C - Error if family key is used to encrypt partial PDI. */
	XPLM_ERR_PPDI_FAMILY_KEY_NOT_ALLOWED,

	/** 0x94D - Error if failed to validate encryption or authentication status. */
	XPLM_ERR_PPDI_TEMPORAL_ERR_AUTH_ENC_STATUS,

	/** 0x94E - Error if PUF is disabled. */
	XPLM_ERR_PPDI_PUF_DISABLED,

	/** 0x94F - Error if partial PDI contains PLM firmware. */
	XPLM_ERR_PPDI_PMCFWLEN_NON_ZERO,

	/** 0x950 - Error if CDO length is not byte aligned */
	XPLM_ERR_PPDI_CDO_LEN_ALIGN,

	/** 0x951 - Error if PUF image ID is set for partial PDI. */
	XPLM_ERR_PPDI_PUF_IMG_NOT_SUPPORTED,

	/** 0x952 - Error if failed to initialize PUF HD to zero. */
	XPLM_ERR_PPDI_PUF_HD_BUFF_ZEROIZE,

	/** 0x953 - Error if failed to validate PUF HD. */
	XPLM_ERR_PUF_HD_DIGEST_VALIDATION,

	/* 0xA00 - 0xA10 are dedicated for PUF SHA Digest errors */
	XPLM_ERR_PUF_SHA_DIGEST = 0xA00,

	/* Errors starting from 0xD00 are related to secure load rom hook. */
	XPLM_ERR_SEC_LOAD = 0xD00,

	/* Exceptions are in range from 0xE00 to 0xE24, this range is derived from the RISC-V last exception ID. */
	/* Exception has occurred during PLM initialization.*/
	XPLM_ERR_EXCEPTION = 0xE00,
	/* Error codes from 0xE00 to 0xE24 are reserved for exceptions. */

	/* CDO command errors. */
	XPLM_ERR_CDO_CMD = 0x1000,
} XPlm_Status_t;

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_STATUS_H */
