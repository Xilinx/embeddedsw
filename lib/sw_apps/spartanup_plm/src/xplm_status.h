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

/**
 * @addtogroup spartanup_plm_apis SpartanUP PLM APIs
 * @{
 */

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
	 * 0x903 - Error when DMA Self-test fails. It occurs when DMA is in
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

	/** 0x90C - Error copying CDO CMD to buffer. */
	XPLM_ERR_MEMCPY_CMD_EXEC,

	/**
	 * 0x90D - Error when the break length required to jump is less
	 * than the processed CDO length
	 */
	XPLM_ERR_INVALID_BREAK_LENGTH,

	/** 0x90E - Error if OSPI flash make is not supported. */
	XPLM_ERR_UNSUPPORTED_OSPI_FLASH_MAKE,

	/** 0x90F - Error if OSPI flash size is not supported. */
	XPLM_ERR_UNSUPPORTED_OSPI_SIZE,

	/**
	 * 0x910 - Error when OSPI driver lookup fails.
	 * Obversed when OSPI is not selected in CIPS
	 */
	XPLM_ERR_OSPI_CFG,

	/** 0x911 - Error when OSPI driver CFG fails */
	XPLM_ERR_OSPI_INIT,

	/**
	 * 0x912 - Error when OSPI driver is unable to select flash CS0.
	 * Check minor code for OSPI driver error code
	 */
	XPLM_ERR_OSPI_SEL_FLASH_CS0,

	/** 0x913 - Error if failed to read OSPI flash ID. */
	XPLM_ERR_OSPI_READ_ID,

	/** 0x914 - Error if failed to read the data from flash. */
	XPLM_ERR_OSPI_READ_DATA,

	/** 0x915 - Error if failed to enable write mode in flash. */
	XPLM_ERR_OSPI_SET_DDR_WRITE_ENABLE,

	/** 0x916 - Error if failed to configure DDR mode in flash. */
	XPLM_ERR_OSPI_SET_DDR_WRITE_CFG_REG,

	/** 0x917 - Error if failed to enable dual byte op code. */
	XPLM_ERR_OSPI_SET_DDR_DUAL_BYTE_OP_ENABLE,

	/** 0x918 - Error if failed to set the controller to DDR PHY mode */
	XPLM_ERR_OSPI_SET_DDR_PHY,

	/** 0x919 - Error if failed to readback configuraion from flash. */
	XPLM_ERR_OSPI_SET_DDR_READ_CFG_REG,

	/**
	 * 0x91A - Error if failed to validate configuration after enabling
	 * DDR PHY mode.
	 */
	XPLM_ERR_OSPI_SET_DDR_CFG_MISMATCH,

	/** 0x91B - Error if failed to disable DUAL BYTE OP code. */
	XPLM_ERR_OSPI_DUAL_BYTE_OP_DISABLE,

	/** 0x91C - Error if 0SPI connection mode is other than single. */
	XPLM_ERR_OSPI_SINGLE_CONN_MODE,

	/** 0x91D - Error if failed to set the controller to SDR NON PHY mode */
	XPLM_ERR_OSPI_SET_SDR_NON_PHY,

	/** 0x91E - Error if failed to set the controller to SDR PHY mode */
	XPLM_ERR_OSPI_SET_SDR_PHY,

	/** 0x91F - Error when source address in OSPI copy exceeds flash size */
	XPLM_ERR_OSPI_COPY_LENGTH_OVERFLOW,

	/** 0x920 - Error if failed to enable write mode in flash. */
	XPLM_ERR_OSPI_4B_ADDR_MODE_WRITE_ENABLE,

	/** 0x921 - Error if failed to enter or exit 4 byte addressing mode. */
	XPLM_ERR_OSPI_4B_ADDR_MODE_ENTER_OR_EXIT_CMD,

	/** 0x922 - Error if failed to verify 4 byte addressing mode. */
	XPLM_ERR_OSPI_4B_ADDR_MODE_ENTER_OR_EXIT_CMD_STATUS,

	/** 0x923 - Error if failed disable write mode in flash. */
	XPLM_ERR_OSPI_4B_ADDR_MODE_WRITE_DISABLE,

	/** 0x924 - Error if failed to clean AES key. */
	XPLM_ERR_AES_KEY_CLEAR_TIMEOUT,

	/** 0x925 - Error if the ref clk divisor in RTCA sets more than 6-bits. */
	XPLM_ERR_RTCA_OSPI_CLK_DIV_MAX,

	/** 0x926 - Error if the OSPI configuration is set to DDR and NON_PHY mode. */
	XPLM_ERR_RTCA_OSPI_INVLD_DDR_PHY_CFG,

	/** 0x927 - Error if failed to read QSPI flash ID. */
	XPLM_ERR_QSPI_READ_ID,

	/** 0x928 - Error if QSPI flash make is not supported. */
	XPLM_ERR_QSPI_FLASH_MAKE_NOT_SUPPORTED,

	/** 0x929 - Error if QSPI flash size is not supported. */
	XPLM_ERR_QSPI_FLASH_SIZE_NOT_SUPPORTED,

	/**
	 * 0x92A - Error when QSPI driver lookup fails.
	 * Obversed when QSPI is not selected in CIPS.
	 */
	XPLM_ERR_QSPI_CFG_NOT_FOUND,

	/** 0x92B - Error when QSPI driver CFG fails. */
	XPLM_ERR_QSPI_CFG_INIT,

	/** 0x92C - Error if QSPI driver failed to select flash. */
	XPLM_ERR_QSPI_FLASH_CS,

	/** 0x92D - Error if QSPI connection mode is other than single. */
	XPLM_ERR_QSPI_SINGLE_CONN_MODE,

	/** 0x92E - Error when source address in OSPI copy exceeds flash size */
	XPLM_ERR_QSPI_COPY_LENGTH_OVERFLOW,

	/** 0x92F - Error if failed to read the data from flash. */
	XPLM_ERR_QSPI_READ,

	/** 0x930 - Error when QSPI read length is greater than flash size */
	XPLM_ERR_QSPI_LENGTH,

	/** 0x931 - Error if QSPI bus width is not supported. */
	XPLM_ERR_QSPI_BUS_WIDTH_NOT_SUPPORTED,

	/** 0x932 - Error if failed to select the flash bank for read. */
	XPLM_ERR_QSPI_READ_BANK_SEL,

	/** 0x933 - Error if failed to select the flash bank*/
	XPLM_ERR_QSPI_BANK_SEL,

	/** 0x934 - Error if failed to enable write mode during flash bank select. */
	XPLM_ERR_QSPI_BANK_SEL_SEND_WREN,

	/**
	 * 0x935 - Error if failed to extended address write command
	 * for spansion flash make.
	 */
	XPLM_ERR_QSPI_BANK_SEL_SEND_EXT_ADDR,

	/** 0x936 - Error if failed to verify the extended address write cmd. */
	XPLM_ERR_QSPI_BANK_SEL_VERIFY_EXT_ADDR,

	/** 0x937 - Error if the QSPI configuration in RTCA is set to DDR mode */
	XPLM_ERR_RTCA_QSPI_INVLD_DDR_CFG,

	/* IO module */
	/** 0x938 - Error if failed to initialize IO module. */
	XPLM_ERR_IO_MOD_INIT,

	/* Assert */
	/** 0x939 - Assertion error */
	XPLM_ERR_ASSERT,

	/* PPDI */
	/** 0x93A - Error if failed to initialize ROM instance to zero. */
	XPLM_ERR_PPDI_ROM_INST_ZEROIZE,

	/** 0x93B - Error if failed to initialize boot header table to zero. */
	XPLM_ERR_PPDI_BH_ZEROIZE,

	/** 0x93C - Error if failed to read boot header from SBI buffer. */
	XPLM_ERR_PPDI_SBI_BUF_READ_WIDTH_WORD,

	/** 0x93D - Error if failed to read boot header from SBI buffer. */
	XPLM_ERR_PPDI_SBI_BUF_READ_BH,

	/** 0x93E - Error if failed to validate Boot Header checksum */
	XPLM_ERR_CALC_BH_CHECKSUM,

	/** 0x93F - Error if failed to initialize hash block to zero. */
	XPLM_ERR_HASH_BLOCK_ZEROIZE,

	/** 0x940 - Error if failed to read hash block from SBI buffer. */
	XPLM_ERR_HASH_BLOCK_READ_SBI_BUF,

	/** 0x941 - Error if failed to read GCM TAG from SBI buffer */
	XPLM_ERR_GCM_TAG_READ_SBI_BUF,

	/** 0x942 - Error if failed to authenticate Hash block */
	XPLM_ERR_HASH_BLOCK_AUTHENTICATION,

	/** 0x943 - Error if failed to validate "XLNX" image identification word. */
	XPLM_ERR_PPDI_INVALID_IMG_DET_WORD,

	/** 0x944 - Error if the secondary boot interface configured it not supported. */
	XPLM_ERR_INVALID_SECONDARY_BOOT_INTF,

	/** 0x945 - Error if BH encryption status is other than @ref XPLM_ENC_STATUS_eFUSE_PUF_KEK */
	XPLM_ERR_PPDI_DEC_EFUSE_ONLY,

	/** 0x946 - Error if encryption source or type is not supported. */
	XPLM_ERR_PPDI_INVALID_BH_ENC_STATUS,

	/** 0x947 - Error if failed to validate A-HWROT. */
	XPLM_ERR_PPDI_AHWROT_UNSIGNED_IMG,

	/**
	 * 0x948 - Error if partial PDI authentication status is different
	 * from full PDI authentication status.
	 */
	XPLM_ERR_PPDI_SEC_TRANSITION_AUTH,

	/**
	 * 0x949 - Error if partial PDI encryption source or type does
	 * not match with full PDI.
	 */
	XPLM_ERR_PPDI_SEC_TO_SEC_KEY_MISMATCH,

	/** 0x94A - Error if family key is used to encrypt partial PDI. */
	XPLM_ERR_PPDI_FAMILY_KEY_NOT_ALLOWED,

	/** 0x94B - Error if failed to validate encryption or authentication status. */
	XPLM_ERR_PPDI_TEMPORAL_ERR_AUTH_ENC_STATUS,

	/** 0x94C - Error if PUF is disabled. */
	XPLM_ERR_PPDI_PUF_DISABLED,

	/** 0x94D - Error if partial PDI contains PLM firmware. */
	XPLM_ERR_PPDI_PMCFWLEN_NON_ZERO,

	/** 0x94E - Error if CDO length is not byte aligned */
	XPLM_ERR_PPDI_CDO_LEN_ALIGN,

	/** 0x94F - Error if PUF image ID is set for partial PDI. */
	XPLM_ERR_PPDI_PUF_IMG_NOT_SUPPORTED,

	/** 0x950 - Error if failed to initialize PUF HD to zero. */
	XPLM_ERR_PPDI_PUF_HD_BUFF_ZEROIZE,

	/** 0x951 - Error if failed to validate PUF HD. */
	XPLM_ERR_PUF_HD_DIGEST_VALIDATION,

	/** 0x952 - Error if failed to initialize chunk instance. */
	XPLM_ERR_INIT_CHUNK_INST,

	/** 0x953 - Error if failed to reset crypto engines. */
	XPLM_ERR_SECURE_CLR,

	/** 0x954 - Error if failed to validate PUF digest for partial PDI. */
	XPLM_ERR_PPDI_INVLD_PUF_DIGEST,

	/** 0x955 - Error if Glitch detected. */
	XPLM_ERR_GLITCH_DETECTED,

	/* CDO command errors are in range from 0xC00 to 0xCFF. */
	/** 0xC00 - feature cmd - Error if the CMD ID is not supported. */
	XPLM_ERR_FEATURE_NOT_SUPPORTED = 0xC00U,

	/** 0xC01 - mask_poll cmd - Error if the command length exceeds the maximum supported. */
	XPLM_ERR_MASK_POLL_INVLD_CMD_LEN,

	/** 0xC02 - mask_poll cmd - Error if timeout while expecting for a value. */
	XPLM_ERR_MASK_POLL_TIMEOUT,

	/* Mask poll errors from 0xC03 to 0xC4F are reserved. */

	/** 0xC50 - cframe_read cmd - Error if readback is disabled in secure boot. */
	XPLM_ERR_RDBK_DISABLED = 0xC50U,

	/** 0xC51 - cframe_read cmd - Error if the invalid interface type is selected for readback. */
	XPLM_ERR_RDBK_INVALID_INFR_SEL,

	/** 0xC52 - cframe_read cmd - Error if failed to transfer CFI read cmd paylod to CCU. */
	XPLM_ERR_RDBK_PAYLOAD_TO_CCU,

	/** 0xC53 - cframe_read cmd - Error if failed to readback the bitstream from CCU. */
	XPLM_ERR_RDBK_CCU_READ,

	/** 0xC54 - cframe_read cmd - Error if failed to read the readback data from the SBI interface. */
	XPLM_ERR_RDBK_READ_TIMEOUT,

	/** 0xC55 - set cmd - Error if failed to set the memory with the value. */
	XPLM_ERR_SET_MEM,

	/** 0xC56 - write_keyhole cmd - Error if failed to transfer the payload to CCU. */
	XPLM_ERR_KEYHOLE_XFER,

	/** 0xC57 - begin cmd - Error if the number of nested begin exceeds the limit of 10. */
	XPLM_ERR_MAX_NESTED_BEGIN,

	/** 0xC58 - begin cmd - Error if failed to store the end offset of begin command. */
	XPLM_ERR_STORE_END_OFFSET,

	/** 0xC59 - end cmd - Error if begin and end cmds are not paired. */
	XPLM_ERR_INVLD_BEGIN_END_PAIR,

	/** 0xC5A - end/break cmd - Error if end address is not valid. */
	XPLM_ERR_INVLD_END_ADDR,


	/** 0xA00 to 0xA10 are dedicated for PUF SHA Digest errors */
	XPLM_ERR_PUF_SHA_DIGEST = 0xA00,

	/** Errors starting from 0xD00 are related to secure load rom hook. */
	XPLM_ERR_SEC_LOAD = 0xD00,

	/** Exceptions are in range from 0xE00 to 0xE24, this range is derived from the RISC-V last exception ID. */
	/* Exception has occurred during PLM initialization.*/
	XPLM_ERR_EXCEPTION = 0xE00,
	/* Error codes from 0xE00 to 0xE24 are reserved for exceptions. */
} XPlm_Status_t;

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_STATUS_H */

/** @} end of spartanup_plm_apis group*/
