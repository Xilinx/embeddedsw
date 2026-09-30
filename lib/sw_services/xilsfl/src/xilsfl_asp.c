/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilsfl_asp.c
 * @brief Implements the vendor-neutral ASP dispatch layer of the SFL library.
 *
 * Vendor-neutral Advanced Sector Protection (ASP) dispatch layer of the serial
 * flash library (SFL). Responsibilities:
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
 * 1.4   vn  09/30/26  Align Doxygen comments with AMD standards
 *
 * </pre>
 *
 ******************************************************************************/

/** @addtogroup xilsfl overview
 * @{
 */

#include "xilsfl_config.h"

#if CONFIG_ADVANCED_SECTOR_PROTECTION

#include "xilsfl.h"
#include "xilsfl_asp.h"
#include "xilsfl_control.h"
#include <string.h>

extern XSfl SflInstance; /**< SFL library instance. */

/*
 * ASP public APIs delegate to XSfl_AspVendorOps hooks bound during XSfl_FlashInit.
 * Each wrapper asserts the handler is in range, reads the vendor table from the
 * instance, asserts the table and the requested hook are present, then invokes it.
 */

/*****************************************************************************/
/**
 *
 * @brief Reads the ASP password register width reported by the vendor driver.
 *
 * The function reads the Advanced Sector Protection password register width
 * reported by the vendor ASP driver (AspPasswordLenBytes field).
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[out]  LenBytesOut Pointer that receives the password byte length.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
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
 * @brief Programs the ASP password register on the NOR flash.
 *
 * The function uses the vendor-specific password program command (delegates to
 * AspOps).
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   Password Buffer that holds the password bytes to program.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
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
 * @brief Reads the ASP password register from the NOR flash.
 *
 * The function reads the register into the caller buffer using the vendor READ
 * PASSWORD sequence (delegates to AspOps).
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[out]  PasswordOut Buffer where the read password is stored.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
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
 * @brief Issues the ASP unlock-password command on the NOR flash.
 *
 * Protected operations can then proceed (delegates to AspOps).
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   Password Buffer that holds the password supplied for unlock.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
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
 * @brief Reads the 16-bit Sector Protection Register (SPR) from NOR flash.
 *
 * The function uses the ASP READ SECTOR PROTECTION command (delegates to
 * AspOps).
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[out]  SprWord Pointer that receives the SPR value.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
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
 * @brief Programs the Sector Protection Register on the NOR flash.
 *
 * The function uses the vendor ASP PROGRAM SECTOR PROTECTION command
 * (delegates to AspOps).
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   SprWord 16-bit SPR value to commit to the device.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
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
 * @brief Erases the ASP nonvolatile lock bits on the NOR flash.
 *
 * The function delegates to AspOps. The vendor implementation handles stacked
 * devices.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
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
 * @brief Sets the ASP nonvolatile lock bit for a sector.
 *
 * The function sets the lock bit for the sector that contains SectorAddress
 * (delegates to AspOps).
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   SectorAddress Byte offset of the sector whose NV lock is set.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
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
 * @brief Reads the ASP nonvolatile sector lock state.
 *
 * The function reads the lock state for the sector that contains
 * SectorAddress (delegates to AspOps).
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   SectorAddress Byte offset used to select the sector for the NV
 *              lock read.
 * @param[out]  IsLocked Pointer that receives 1 when locked, 0 when unlocked.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
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
 * @brief Writes the ASP global freeze bit on the NOR flash.
 *
 * The function delegates to AspOps.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
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
 * @brief Returns the number of sectors on the connected flash.
 *
 * The function derives the count from the reported device size and sector
 * size.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[out]  NumSectorsOut Pointer that receives the total sector count.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - XST_FAILURE otherwise.
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
 * @brief Validates a sector range.
 *
 * The function validates that [StartSector, EndSector] is a well-formed sector
 * range that lies within the device.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   StartSector First sector index (inclusive).
 * @param[in]   EndSector Last sector index (inclusive).
 *
 * @return
 *              - XSFL_ASP_RESULT_OK if valid.
 *              - An XSfl_AspResult error code otherwise.
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

#endif /* CONFIG_ADVANCED_SECTOR_PROTECTION */

/** @} */
