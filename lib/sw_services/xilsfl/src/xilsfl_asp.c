/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 * @file xilsfl_asp.c
 * @addtogroup xilsfl overview
 * @{
 *
 * Vendor-neutral ASP dispatch layer. Responsibilities:
 *   - Expose all public XSfl_Flash* ASP APIs (vendor-neutral wrappers that
 *     delegate to the ops table bound during XSfl_FlashInit).
 *   - Provide the shared helpers (sector range validation, password unlock)
 *     used by the XSfl_Asp* operations in xilsfl.c.
 *
 * Vendor binding and the XSfl_Asp* operation entry points live in xilsfl.c.
 * Adding a new vendor requires only a new vendor.c / vendor.h pair that
 * exports an XSfl_AspVendorOps table, plus one entry in
 * XSfl_FlashAspAssignVendorOps.
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

#include "xilsfl_config.h"

#if CONFIG_ADVANCED_SECTOR_PROTECTION

#include "xilsfl.h"
#include "xilsfl_asp.h"
#include "xilsfl_control.h"
#include <string.h>

extern XSfl SflInstance;

/*
 * ASP public APIs delegate to XSfl_AspVendorOps hooks bound during XSfl_FlashInit.
 * Each wrapper asserts the handler is in range, reads the vendor table from the
 * instance, asserts the table and the requested hook are present, then invokes it.
 */

/*****************************************************************************/
/**
 *
 * This function reads the Advanced Sector Protection password register width
 * reported by the vendor ASP driver (AspPasswordLenBytes field).
 *
 * @param	SflHandler is the index to the Sfl interface driver instance.
 * @param	LenBytesOut is the pointer that receives the password byte length.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XSfl_FlashAspGetPasswordByteLen(u8 SflHandler, u8 *LenBytesOut)
{
	const XSfl_AspVendorOps *Ops;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	Xil_AssertNonvoid(LenBytesOut != NULL);

	Ops = SflInstance.Instance[SflHandler].SflFlashInfo.AspOps;
	Xil_AssertNonvoid(Ops != NULL);
	Xil_AssertNonvoid(Ops->AspPasswordLenBytes != 0U);
	Xil_AssertNonvoid(Ops->AspPasswordLenBytes <= XSFL_ASP_PASSWORD_MAX_BYTES);

	*LenBytesOut = Ops->AspPasswordLenBytes;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function programs the ASP password register on the NOR flash using the
 * vendor-specific password program command (delegates to AspOps).
 *
 * @param	SflHandler is the index to the Sfl interface driver instance.
 * @param	Password is the buffer holding the password bytes to program.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XSfl_FlashPasswordProgram(u8 SflHandler,
		const u8 Password[XSFL_ASP_PASSWORD_MAX_BYTES])
{
	const XSfl_AspVendorOps *Ops;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	Xil_AssertNonvoid(Password != NULL);

	Ops = SflInstance.Instance[SflHandler].SflFlashInfo.AspOps;
	Xil_AssertNonvoid(Ops != NULL);
	Xil_AssertNonvoid(Ops->PasswordProgram != NULL);
	return Ops->PasswordProgram(SflHandler, Password);
}

/*****************************************************************************/
/**
 *
 * This function reads the ASP password register from NOR flash into the caller
 * buffer using the vendor READ PASSWORD sequence (delegates to AspOps).
 *
 * @param	SflHandler is the index to the Sfl interface driver instance.
 * @param	PasswordOut is the buffer where the read password is stored.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XSfl_FlashPasswordRead(u8 SflHandler,
		u8 PasswordOut[XSFL_ASP_PASSWORD_MAX_BYTES])
{
	const XSfl_AspVendorOps *Ops;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	Xil_AssertNonvoid(PasswordOut != NULL);

	Ops = SflInstance.Instance[SflHandler].SflFlashInfo.AspOps;
	Xil_AssertNonvoid(Ops != NULL);
	Xil_AssertNonvoid(Ops->PasswordRead != NULL);
	return Ops->PasswordRead(SflHandler, PasswordOut);
}

/*****************************************************************************/
/**
 *
 * This function issues the ASP unlock-password command on NOR flash so that
 * protected operations can proceed (delegates to AspOps).
 *
 * @param	SflHandler is the index to the Sfl interface driver instance.
 * @param	Password is the buffer holding the password supplied for unlock.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XSfl_FlashPasswordUnlock(u8 SflHandler,
		const u8 Password[XSFL_ASP_PASSWORD_MAX_BYTES])
{
	const XSfl_AspVendorOps *Ops;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	Xil_AssertNonvoid(Password != NULL);

	Ops = SflInstance.Instance[SflHandler].SflFlashInfo.AspOps;
	Xil_AssertNonvoid(Ops != NULL);
	Xil_AssertNonvoid(Ops->PasswordUnlock != NULL);
	return Ops->PasswordUnlock(SflHandler, Password);
}

/*****************************************************************************/
/**
 *
 * This function reads the 16-bit Sector Protection Register (SPR) from NOR
 * flash via the ASP READ SECTOR PROTECTION command (delegates to AspOps).
 *
 * @param	SflHandler is the index to the Sfl interface driver instance.
 * @param	SprWord is the pointer that receives the SPR value.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XSfl_FlashSectorProtectionRead(u8 SflHandler, u16 *SprWord)
{
	const XSfl_AspVendorOps *Ops;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	Xil_AssertNonvoid(SprWord != NULL);

	Ops = SflInstance.Instance[SflHandler].SflFlashInfo.AspOps;
	Xil_AssertNonvoid(Ops != NULL);
	Xil_AssertNonvoid(Ops->SectorProtectionRead != NULL);
	return Ops->SectorProtectionRead(SflHandler, SprWord);
}

/*****************************************************************************/
/**
 *
 * This function programs the Sector Protection Register on NOR flash using
 * the vendor ASP PROGRAM SECTOR PROTECTION command (delegates to AspOps).
 *
 * @param	SflHandler is the index to the Sfl interface driver instance.
 * @param	SprWord is the 16-bit SPR value to commit to the device.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XSfl_FlashSectorProtectionWrite(u8 SflHandler, u16 SprWord)
{
	const XSfl_AspVendorOps *Ops;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);

	Ops = SflInstance.Instance[SflHandler].SflFlashInfo.AspOps;
	Xil_AssertNonvoid(Ops != NULL);
	Xil_AssertNonvoid(Ops->SectorProtectionWrite != NULL);
	return Ops->SectorProtectionWrite(SflHandler, SprWord);
}

/*****************************************************************************/
/**
 *
 * This function erases ASP nonvolatile lock bits on NOR flash (delegates to
 * AspOps); stacked devices are handled inside the vendor implementation.
 *
 * @param	SflHandler is the index to the Sfl interface driver instance.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XSfl_FlashEraseNVLockBits(u8 SflHandler)
{
	const XSfl_AspVendorOps *Ops;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);

	Ops = SflInstance.Instance[SflHandler].SflFlashInfo.AspOps;
	Xil_AssertNonvoid(Ops != NULL);
	Xil_AssertNonvoid(Ops->EraseNVLockBits != NULL);
	return Ops->EraseNVLockBits(SflHandler);
}

/*****************************************************************************/
/**
 *
 * This function sets the ASP nonvolatile lock bit for the sector that contains
 * SectorAddress (delegates to AspOps).
 *
 * @param	SflHandler is the index to the Sfl interface driver instance.
 * @param	SectorAddress is the byte offset of the sector whose NV lock is set.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XSfl_FlashNVSectorLock(u8 SflHandler, u32 SectorAddress)
{
	const XSfl_AspVendorOps *Ops;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);

	Ops = SflInstance.Instance[SflHandler].SflFlashInfo.AspOps;
	Xil_AssertNonvoid(Ops != NULL);
	Xil_AssertNonvoid(Ops->NVSectorLock != NULL);
	return Ops->NVSectorLock(SflHandler, SectorAddress);
}

/*****************************************************************************/
/**
 *
 * This function reads ASP nonvolatile sector lock state for the sector that
 * contains SectorAddress (delegates to AspOps).
 *
 * @param	SflHandler is the index to the Sfl interface driver instance.
 * @param	SectorAddress is the byte offset used to select the sector for NV lock read.
 * @param	IsLocked is the pointer that receives 1 when locked, 0 when unlocked.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XSfl_FlashNVLockRead(u8 SflHandler, u32 SectorAddress, u8 *IsLocked)
{
	const XSfl_AspVendorOps *Ops;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	Xil_AssertNonvoid(IsLocked != NULL);

	Ops = SflInstance.Instance[SflHandler].SflFlashInfo.AspOps;
	Xil_AssertNonvoid(Ops != NULL);
	Xil_AssertNonvoid(Ops->NVLockRead != NULL);
	return Ops->NVLockRead(SflHandler, SectorAddress, IsLocked);
}

/*****************************************************************************/
/**
 *
 * This function writes the ASP global freeze bit on NOR flash (delegates to
 * AspOps).
 *
 * @param	SflHandler is the index to the Sfl interface driver instance.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XSfl_FlashAspWriteGlobalFreeze(u8 SflHandler)
{
	const XSfl_AspVendorOps *Ops;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);

	Ops = SflInstance.Instance[SflHandler].SflFlashInfo.AspOps;
	Xil_AssertNonvoid(Ops != NULL);
	Xil_AssertNonvoid(Ops->GlobalFreezeWrite != NULL);
	return Ops->GlobalFreezeWrite(SflHandler);
}

/*****************************************************************************/
/**
 *
 * This function returns the number of sectors on the connected flash, derived
 * from the reported device size and sector size.
 *
 * @param	SflHandler is the index to the Sfl interface driver instance.
 * @param	NumSectorsOut receives the total sector count.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 ******************************************************************************/
static u32 XSfl_AspGetNumSectors(u8 SflHandler, u32 *NumSectorsOut)
{
	u32 SectorSize;
	u32 DeviceSize;

	if (NumSectorsOut == NULL) {
		return XST_FAILURE;
	}

	if (XSfl_FlashGetInfo(SflHandler, XSFL_SECT_SIZE, &SectorSize) !=
	    XST_SUCCESS ||
	    XSfl_FlashGetInfo(SflHandler, XSFL_DEVICE_SIZE, &DeviceSize) !=
	    XST_SUCCESS ||
	    SectorSize == 0U) {
		return XST_FAILURE;
	}

	*NumSectorsOut = DeviceSize / SectorSize;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function validates that [StartSector, EndSector] is a well-formed sector
 * range that lies within the device.
 *
 * @param	SflHandler is the index to the Sfl interface driver instance.
 * @param	StartSector is the first sector index (inclusive).
 * @param	EndSector is the last sector index (inclusive).
 *
 * @return	XSFL_ASP_RESULT_OK if valid, else an XSfl_AspResult error code.
 *
 ******************************************************************************/
u32 XSfl_AspValidateSectorRange(u8 SflHandler, u32 StartSector,
		u32 EndSector)
{
	u32 NumSectors;

	if (StartSector > EndSector) {
		return XSFL_ASP_RESULT_INVALID_RANGE;
	}

	if (XSfl_AspGetNumSectors(SflHandler, &NumSectors) != XST_SUCCESS) {
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	if (EndSector >= NumSectors) {
		return XSFL_ASP_RESULT_INVALID_RANGE;
	}

	return XSFL_ASP_RESULT_OK;
}

/** @} */

#endif /* CONFIG_ADVANCED_SECTOR_PROTECTION */
