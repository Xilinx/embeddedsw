/******************************************************************************
 * Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilsfl_asp.h
 * @addtogroup xilsfl overview
 * @{
 *
 * Advanced Sector Protection (ASP): XSfl_AspVendorOps vendor dispatch table and
 * vendor-neutral XSfl_Flash* ASP API declarations (implementations in xilsfl.c).
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.3   sb  07/20/26  Initial support for Advanced Sector Protection.
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef XILSFL_ASP_H
#define XILSFL_ASP_H

#include "xil_types.h"
#include "xilsfl_config.h"

#if CONFIG_ADVANCED_SECTOR_PROTECTION

/**
 * ASP command identifiers for sector lock example/application menu dispatch.
 */
typedef enum {
	XSFL_ASP_CMD_SET_PASSWORD      = 0U,
	XSFL_ASP_CMD_LOCK_SECTORS     = 1U,
	XSFL_ASP_CMD_UNLOCK_SECTORS   = 2U,
	XSFL_ASP_CMD_READ_SECTOR_LOCK = 3U
} XSfl_AspCmd;

/**
 * Result codes returned by the ASP operations. XSFL_ASP_RESULT_OK
 * aliases XST_SUCCESS; the remaining values describe the failure so callers
 * (and unit tests) can react without parsing console output.
 */
typedef enum {
	XSFL_ASP_RESULT_OK                = 0U,
	XSFL_ASP_RESULT_INVALID_PASSWORD  = 2U,
	XSFL_ASP_RESULT_PASSWORD_MISMATCH = 3U,
	XSFL_ASP_RESULT_PASSWORD_LOCKED   = 4U,
	XSFL_ASP_RESULT_INVALID_RANGE     = 5U,
	XSFL_ASP_RESULT_OP_FAILED         = 6U
} XSfl_AspResult;

/**
 * Parameters for the ASP operations. Fields are used per operation:
 *   XSfl_AspSetPassword    : Password, PasswordLen
 *   XSfl_AspLockSectors    : StartSector, EndSector, Password/PasswordLen
 *                            (if SPR password protection is enabled)
 *   XSfl_AspUnlockSectors  : Password/PasswordLen (if SPR password protection
 *                            is enabled)
 *   XSfl_AspReadSectorLock : StartSector, EndSector, LockStatusOut,
 *                            LockStatusCap
 */
typedef struct {
	const u8 *Password;     /**< device-length password bytes; NULL if none */
	u8        PasswordLen;   /**< effective device password length in bytes  */
	u32       StartSector;   /**< LOCK/READ: first sector index (inclusive)   */
	u32       EndSector;     /**< LOCK/READ: last sector index (inclusive)    */
	u8       *LockStatusOut; /**< READ: caller buffer, 1 byte/sector; may NULL*/
	u32       LockStatusCap; /**< READ: capacity of LockStatusOut in entries  */
} XSfl_AspParams;

/**
 * Max password buffer length (bytes) for generic XSfl_FlashAsp* ASP APIs; the
 * effective device length is XSfl_AspVendorOps.AspPasswordLenBytes.
 */
#ifndef XSFL_ASP_PASSWORD_MAX_BYTES
#define XSFL_ASP_PASSWORD_MAX_BYTES	32U
#endif

/** SPR password-protection bit (active-low): protection is enabled when this bit is 0. */
#define XSFL_ASP_PASSWORD_PROT_MODE_MASK	0x04U

/**
 * Vendor-specific ASP implementations. XSfl_FlashInit selects a pointer table
 * (for example, Micron MT35 OCTAL ASP). NULL members mean unsupported on that silicon.
 *
 * Applications use @ref XSfl_Flash* ASP entry points; those delegate here.
 */
typedef struct {
	u32 (*SectorProtectionRead)(u8 SflHandler, u16 *SprWord);
	u32 (*SectorProtectionWrite)(u8 SflHandler, u16 SprWord);
	u32 (*EraseNVLockBits)(u8 SflHandler);
	u32 (*NVSectorLock)(u8 SflHandler, u32 SectorAddress);
	u32 (*NVLockRead)(u8 SflHandler, u32 SectorAddress, u8 *IsLocked);
	u32 (*PasswordProgram)(u8 SflHandler,
			       const u8 Password[XSFL_ASP_PASSWORD_MAX_BYTES]);
	u32 (*PasswordRead)(u8 SflHandler,
			    u8 PasswordOut[XSFL_ASP_PASSWORD_MAX_BYTES]);
	u32 (*PasswordUnlock)(u8 SflHandler,
			      const u8 Password[XSFL_ASP_PASSWORD_MAX_BYTES]);
	/** Write ASP global freeze bit; NULL if unsupported on this device. */
	u32 (*GlobalFreezeWrite)(u8 SflHandler);
	/** Device password byte count for this vendor; 0 when password not applicable. Never exceed @ref XSFL_ASP_PASSWORD_MAX_BYTES. */
	u8 AspPasswordLenBytes;
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

/** @} */

#endif /* CONFIG_ADVANCED_SECTOR_PROTECTION */

#endif /* XILSFL_ASP_H */
