/******************************************************************************
 * Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef XILSFL_ASP_H
#define XILSFL_ASP_H

/*****************************************************************************/
/**
 * @file xilsfl_asp.h
 * @brief Declares the Advanced Sector Protection (ASP) interface of the SFL.
 *
 * This file defines the ASP XSfl_AspVendorOps vendor dispatch table of the
 * serial flash library (SFL). It also declares the vendor-neutral XSfl_Flash*
 * ASP APIs. The implementations are in xilsfl_asp.c.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.3   sb  07/20/26  Initial support for Advanced Sector Protection.
 * 1.4   vn  09/30/26  Align Doxygen comments with AMD standards
 *
 * </pre>
 *
 ******************************************************************************/

/** @addtogroup xilsfl overview
 * @{
 */

#include "xil_types.h"
#include "xilsfl_config.h"

#if CONFIG_ADVANCED_SECTOR_PROTECTION

/**
 * @brief Defines the ASP command identifiers for the sector lock menu.
 *
 * The example or application menu uses these identifiers for dispatch.
 */
typedef enum {
	XSFL_ASP_CMD_SET_PASSWORD      = 0U, /**< Set the ASP password. */
	XSFL_ASP_CMD_LOCK_SECTORS     = 1U,  /**< Lock a range of sectors. */
	XSFL_ASP_CMD_UNLOCK_SECTORS   = 2U,  /**< Unlock the sectors. */
	XSFL_ASP_CMD_READ_SECTOR_LOCK = 3U   /**< Read sector lock state. */
} XSfl_AspCmd;

/**
 * @brief Defines the result codes that the ASP operations return.
 *
 * XSFL_ASP_RESULT_OK aliases XST_SUCCESS. The remaining values describe the
 * failure so that callers (and unit tests) can react without parsing console
 * output.
 */
typedef enum {
	XSFL_ASP_RESULT_OK                = 0U, /**< Operation succeeded. */
	XSFL_ASP_RESULT_INVALID_PASSWORD  = 2U, /**< Password is not valid. */
	XSFL_ASP_RESULT_PASSWORD_MISMATCH = 3U, /**< Password does not match. */
	XSFL_ASP_RESULT_PASSWORD_LOCKED   = 4U, /**< Password protection is
						 * already active. */
	XSFL_ASP_RESULT_INVALID_RANGE     = 5U, /**< Sector range is invalid. */
	XSFL_ASP_RESULT_OP_FAILED         = 6U  /**< Operation failed. */
} XSfl_AspResult;

/**
 * @brief Holds the parameters for the ASP operations.
 *
 * Each operation uses these fields:
 * - XSfl_AspSetPassword: Password, PasswordLen.
 * - XSfl_AspLockSectors: StartSector, EndSector, Password/PasswordLen (if SPR
 *   password protection is enabled).
 * - XSfl_AspUnlockSectors: Password/PasswordLen (if SPR password protection
 *   is enabled).
 * - XSfl_AspReadSectorLock: StartSector, EndSector, LockStatusOut,
 *   LockStatusCap.
 */
typedef struct {
	const u8 *Password;	 /**< Device-length password bytes, or NULL if
				  * none. */
	u8        PasswordLen;	 /**< Effective device password length in
				  * bytes. */
	u32       StartSector;	 /**< LOCK/READ: first sector index
				  * (inclusive). */
	u32       EndSector;	 /**< LOCK/READ: last sector index
				  * (inclusive). */
	u8       *LockStatusOut; /**< READ: caller buffer, one byte per sector.
				  * Can be NULL. */
	u32       LockStatusCap; /**< READ: capacity of LockStatusOut in
				  * entries. */
} XSfl_AspParams;

/**
 * Maximum password buffer length, in bytes, for the generic XSfl_FlashAsp* ASP
 * APIs. The effective device length is XSfl_AspVendorOps.AspPasswordLenBytes.
 */
#ifndef XSFL_ASP_PASSWORD_MAX_BYTES
#define XSFL_ASP_PASSWORD_MAX_BYTES	32U
#endif

/**
 * SPR password-protection bit (active-low). Protection is enabled when this bit
 * is 0.
 */
#define XSFL_ASP_PASSWORD_PROT_MODE_MASK	0x04U

/**
 * @brief Holds the vendor-specific ASP implementations.
 *
 * XSfl_FlashInit selects a pointer table (for example, Micron MT35 OCTAL ASP).
 * NULL members mean that the silicon does not support the operation.
 *
 * Applications use the XSfl_Flash* ASP entry points, which delegate here.
 */
typedef struct {
	u32 (*SectorProtectionRead)(u8 SflHandler, u16 *SprWord);
						/**< Reads the Sector Protection
						 * Register (SPR). */
	u32 (*SectorProtectionWrite)(u8 SflHandler, u16 SprWord);
						/**< Programs the Sector
						 * Protection Register (SPR). */
	u32 (*EraseNVLockBits)(u8 SflHandler);	/**< Erases the nonvolatile lock
						 * bits. */
	u32 (*NVSectorLock)(u8 SflHandler, u32 SectorAddress);
						/**< Sets the nonvolatile lock
						 * bit of a sector. */
	u32 (*NVLockRead)(u8 SflHandler, u32 SectorAddress, u8 *IsLocked);
						/**< Reads the nonvolatile lock
						 * state of a sector. */
	u32 (*PasswordProgram)(u8 SflHandler,
			       const u8 Password[XSFL_ASP_PASSWORD_MAX_BYTES]);
						/**< Programs the ASP
						 * password. */
	u32 (*PasswordRead)(u8 SflHandler,
			    u8 PasswordOut[XSFL_ASP_PASSWORD_MAX_BYTES]);
						/**< Reads the ASP password. */
	u32 (*PasswordUnlock)(u8 SflHandler,
			      const u8 Password[XSFL_ASP_PASSWORD_MAX_BYTES]);
						/**< Unlocks the device with the
						 * ASP password. */
	u32 (*GlobalFreezeWrite)(u8 SflHandler);
						/**< Writes the ASP global
						 * freeze bit. It is NULL if the
						 * device does not support
						 * it. */
	u8 AspPasswordLenBytes;	/**< Device password byte count for this vendor,
				 * or 0 when a password is not applicable. Must
				 * not exceed @ref
				 * XSFL_ASP_PASSWORD_MAX_BYTES. */
} XSfl_AspVendorOps;

/************************** ASP Function Prototypes ****************************/

u32 XSfl_FlashAspGetPasswordByteLen(u8 SflHandler, u8 *LenBytesOut);
u32 XSfl_FlashPasswordProgram(u8 SflHandler,
		const u8 Password[XSFL_ASP_PASSWORD_MAX_BYTES]);
u32 XSfl_FlashPasswordRead(u8 SflHandler,
		u8 PasswordOut[XSFL_ASP_PASSWORD_MAX_BYTES]);
u32 XSfl_FlashPasswordUnlock(u8 SflHandler,
		const u8 Password[XSFL_ASP_PASSWORD_MAX_BYTES]);

u32 XSfl_FlashSectorProtectionRead(u8 SflHandler, u16 *SprWord);
u32 XSfl_FlashSectorProtectionWrite(u8 SflHandler, u16 SprWord);
u32 XSfl_FlashEraseNVLockBits(u8 SflHandler);
u32 XSfl_FlashNVSectorLock(u8 SflHandler, u32 SectorAddress);
u32 XSfl_FlashNVLockRead(u8 SflHandler, u32 SectorAddress, u8 *IsLocked);
u32 XSfl_AspValidateSectorRange(u8 SflHandler, u32 StartSector, u32 EndSector);
u32 XSfl_FlashAspWriteGlobalFreeze(u8 SflHandler);

#endif /* CONFIG_ADVANCED_SECTOR_PROTECTION */

/** @} */
#endif /* XILSFL_ASP_H */
