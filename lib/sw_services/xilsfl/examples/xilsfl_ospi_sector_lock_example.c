/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xilsfl_ospi_sector_lock_example.c
 *
 * This file contains a design example using the SFL library for Advanced
 * Sector Protection (ASP) on NOR flash over OSPI. It interacts
 * over stdin: selects an ASP operation id, optionally programs password / NV
 * sector locks, unlocks NV lock bits, and prints NV lock status for a sector
 * range.
 *
 * The hardware which this example runs on, must have an octal serial nor flash
 * for it to run. This example has been tested with the Micron Octal Serial
 * Flash (MT35XU02GCBA).
 *
 * @note
 *
 * Console I/O (scanf) is used; password buffers stay on stack only.
 * Requires BSP xilsfl_CONFIG_ADVANCED_SECTOR_PROTECTION=true.
 *
 *<pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------
 * 1.3   sb   07/20/26 Initial support for Advanced Sector Protection.
 *
 *</pre>
 ******************************************************************************/

/*
 * Requires BSP xilsfl_CONFIG_ADVANCED_SECTOR_PROTECTION=true (maps to CONFIG_ADVANCED_SECTOR_PROTECTION
 * in xilsfl_config.h). The standalone examples/CMakeLists.txt omits this target when ASP is disabled.
 */
#include "xilsfl_config.h"

#if CONFIG_ADVANCED_SECTOR_PROTECTION

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xstatus.h"
#include "xilsfl.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

/************************** Constant Definitions ****************************/

/*
 * Maximum number of sectors whose NV lock state can be fetched in a single
 * READ_SECTOR_LOCK request (bounds the on-stack status buffer).
 */
#define ASP_MAX_READ_SECTORS 256U

/*
 * Map to BSP parameters — change in one place for the board/application.
 */
#define OSPI_BASEADDR XPAR_XOSPIPSV_0_BASEADDR

/**************************** Type Definitions ********************************/

/* None additional — XSfl_UserConfig declared in xilsfl.h */

/***************** Macros (Inline Functions) Definitions *********************/

/* None */

/************************** Function Prototypes ******************************/

static void init_platform(void);
static void cleanup_platform(void);

static void PrintFlashInfoAsp(u8 SflHandlerIdx);
static int XsflAspOspiSectorLockExample(void);

static int AspScanPassword(u8 OutPwd[XSFL_ASP_PASSWORD_MAX_BYTES], u8 LenBytes);
static int AspPwdBufAllZero(const u8 Pw[XSFL_ASP_PASSWORD_MAX_BYTES],
			    u32 LenBytes);

static u32 AspSetPassword(u8 SflHandler);
static u32 AspLockSectors(u8 SflHandler);
static u32 AspUnlockSectors(u8 SflHandler);
static u32 AspReadSectorLockStatus(u8 SflHandler);

static u32 AspGetNumSectors(u8 SflHandler, u32 *NumSectorsOut);
static void AspPrintResult(u32 Result);

/************************** Variable Definitions *****************************/

/**
 * XSfl handler index populated by XSfl_FlashInit; ASP ops use Micron table when
 * JEDEC + flash-type match BSP assignment logic.
 */
static u8 SflHandler;

/*****************************************************************************/
/**
 *
 * Main entry — runs the ASP OSPI sector-lock demonstration.
 *
 * @return	XST_SUCCESS on success path, otherwise XST_FAILURE.
 *
 ******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("\r\nAdvanced Sector Protection example\r\n");

	Status = XsflAspOspiSectorLockExample();
	if (Status != XST_SUCCESS) {
		xil_printf("\r\nExample failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("\r\nSuccessfully ran Advanced Sector Protection example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function is the primary example routine: enables caches, initializes
 * the OSPI controller and NOR flash via XSfl_FlashInit, prints device info,
 * reads an ASP operation id from stdin, and dispatches the selected operation.
 *
 * @return	XST_SUCCESS on success, else XST_FAILURE.
 *
 ******************************************************************************/
static int XsflAspOspiSectorLockExample(void)
{
	u32 Status = XST_SUCCESS;
	u32 AspOperationId;
	unsigned long UserInput;
	XSfl_UserConfig SflConfig;

	init_platform();

	SflConfig.Ospi_Config.BaseAddress = OSPI_BASEADDR;
	SflConfig.Ospi_Config.ChipSelect = XSFL_SELECT_FLASH_CS0;
	SflConfig.Ospi_Config.ReadMode = XOSPIPSV_IDAC_EN_OPTION;
#if defined(SPARTANUP)
	SflConfig.Ospi_Config.Quirks = XSFL_BROKEN_DMA;
#endif

	Status = XSfl_FlashInit(&SflHandler, SflConfig, XSFL_OSPI_CNTRL);
	if (Status != XST_SUCCESS) {
		xil_printf("XSfl_FlashInit failed: %lu\r\n",
			   (u32)Status);
		goto done;
	}

	PrintFlashInfoAsp(SflHandler);

	xil_printf("\r\n=== ASP (operation menu) ===\r\n");
	xil_printf("  %u = SET_PASSWORD\r\n", XSFL_ASP_CMD_SET_PASSWORD);
	xil_printf("  %u = LOCK_SECTORS\r\n",
		   XSFL_ASP_CMD_LOCK_SECTORS);
	xil_printf("  %u = UNLOCK_SECTORS (erases ALL NV lock bits)\r\n",
		   XSFL_ASP_CMD_UNLOCK_SECTORS);
	xil_printf("  %u = READ_SECTOR_LOCK\r\n",
		   XSFL_ASP_CMD_READ_SECTOR_LOCK);

	xil_printf("\r\nEnter ASP operation id (decimal): ");
	if (scanf("%lu", &UserInput) != 1) {
		xil_printf("ERROR: scanf failed for operation id\r\n");
		Status = XST_FAILURE;
		goto done;
	}
	AspOperationId = (u32)UserInput;

	switch ((XSfl_AspCmd)AspOperationId) {
	case XSFL_ASP_CMD_SET_PASSWORD:
		xil_printf("\r\n--- SET_PASSWORD ---\r\n");
		Status = AspSetPassword(SflHandler);
		break;
	case XSFL_ASP_CMD_LOCK_SECTORS:
		xil_printf(
		    "\r\n--- LOCK_SECTORS (range + PW if SPR requires) ---\r\n");
		Status = AspLockSectors(SflHandler);
		break;
	case XSFL_ASP_CMD_UNLOCK_SECTORS:
		xil_printf("\r\n--- UNLOCK_SECTORS ---\r\n");
		Status = AspUnlockSectors(SflHandler);
		break;
	case XSFL_ASP_CMD_READ_SECTOR_LOCK:
		xil_printf("\r\n--- READ_SECTOR_LOCK ---\r\n");
		Status = AspReadSectorLockStatus(SflHandler);
		break;
	default:
		xil_printf("ERROR: Invalid operation id - 0x%X\r\n",
			   AspOperationId);
		Status = XST_FAILURE;
		break;
	}

done:
	xil_printf("\r\nDone.\r\n");
	cleanup_platform();
	return (Status == XST_SUCCESS) ? XST_SUCCESS : XST_FAILURE;
}

/*****************************************************************************/
/**
 *
 * This function prints flash identification details including JEDEC ID, sector
 * size, device size, and total number of sectors for the connected NOR flash.
 *
 * @param[in] SflHandlerIdx is the index to the Sfl interface driver instance.
 *
 ******************************************************************************/
static void PrintFlashInfoAsp(u8 SflHandlerIdx)
{
	u32 JedecId;
	u32 SectorSize;
	u32 DeviceSize;
	u32 NumSectors;

	xil_printf("\r\n=== ASP Utility ===\r\n");

	if (XSfl_FlashGetInfo(SflHandlerIdx, XSFL_FLASH_ID, &JedecId) !=
	    XST_SUCCESS ||
	    XSfl_FlashGetInfo(SflHandlerIdx, XSFL_SECT_SIZE, &SectorSize) !=
	    XST_SUCCESS ||
	    XSfl_FlashGetInfo(SflHandlerIdx, XSFL_DEVICE_SIZE, &DeviceSize) !=
	    XST_SUCCESS) {
		xil_printf("(Flash info read failed)\r\n");
		return;
	}

	xil_printf("FlashID=0x%02X 0x%02X 0x%02X\r\n",
		   (u32)((JedecId >> 16) & 0xFFU),
		   (u32)((JedecId >> 8) & 0xFFU),
		   (u32)(JedecId & 0xFFU));
	xil_printf("Sector size : %lu bytes\r\n",
		   (unsigned long)SectorSize);
	xil_printf("Flash size  : %lu bytes\r\n",
		   (unsigned long)DeviceSize);
	NumSectors =
	    (SectorSize != 0U) ? (DeviceSize / SectorSize) : 0U;
	xil_printf("Number of Sectors : %lu\r\n",
		   (unsigned long)NumSectors);
	xil_printf("Assigned Vendor hooks\r\n");
}

/*****************************************************************************/
/**
 *
 * This function computes the total number of sectors on the connected flash
 * from the device size and sector size reported by XSfl_FlashGetInfo.
 *
 * @param[in] SflHandler is the index to the Sfl interface driver instance.
 * @param[in] NumSectorsOut is the pointer that receives the total sector count.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 ******************************************************************************/
static u32 AspGetNumSectors(u8 SflHandler, u32 *NumSectorsOut)
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
 * This function prints a human-readable status message for an XSfl_AspResult
 * code returned by the XSfl_Asp* operations.
 *
 * @param[in] Result is the XSfl_AspResult code to describe.
 *
 ******************************************************************************/
static void AspPrintResult(u32 Result)
{
	switch ((XSfl_AspResult)Result) {
	case XSFL_ASP_RESULT_OK:
		xil_printf("ASP: OK\r\n");
		break;
	case XSFL_ASP_RESULT_INVALID_PASSWORD:
		xil_printf("ASP: invalid password\r\n");
		break;
	case XSFL_ASP_RESULT_PASSWORD_MISMATCH:
		xil_printf("ASP: password mismatch / unlock failed\r\n");
		break;
	case XSFL_ASP_RESULT_PASSWORD_LOCKED:
		xil_printf("ASP: flash is already locked\r\n");
		break;
	case XSFL_ASP_RESULT_INVALID_RANGE:
		xil_printf("ASP: invalid sector range\r\n");
		break;
	default:
		xil_printf("ASP: operation failed\r\n");
		break;
	}
}

/*****************************************************************************/
/**
 *
 * This function enables instruction and data caches for the target processor.
 *
 ******************************************************************************/
static void init_platform(void)
{
#if defined(ARMR5)
	Xil_ICacheEnable();
	Xil_DCacheEnable();
#elif defined(__MICROBLAZE__)
#ifdef XPAR_MICROBLAZE_USE_ICACHE
	Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
	Xil_DCacheEnable();
#endif
#endif
}

/*****************************************************************************/
/**
 *
 * This function disables instruction and data caches for the target processor.
 *
 ******************************************************************************/
static void cleanup_platform(void)
{
#if defined(ARMR5)
	Xil_DCacheDisable();
	Xil_ICacheDisable();
#elif defined(__MICROBLAZE__)
#ifdef XPAR_MICROBLAZE_USE_DCACHE
	Xil_DCacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_ICACHE
	Xil_ICacheDisable();
#endif
#endif
}

/*****************************************************************************/
/**
 *
 * This function sets the ASP password on the NOR flash. It reads the Sector
 * Protection Register to verify password protection is not already active,
 * prompts the user for double-entry hex password, programs the password,
 * verifies by readback, and commits the SPR.
 *
 * @param[in] SflHandler is the index to the Sfl interface driver instance.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
static u32 AspSetPassword(u8 SflHandler)
{
	u32 Status;
	u32 Result;
	u16 SprBefore;
	u8 AspPswdLen;
	u8 AspPswdFirst[XSFL_ASP_PASSWORD_MAX_BYTES];
	u8 AspPswdConfirm[XSFL_ASP_PASSWORD_MAX_BYTES];
	XSfl_AspParams Params;

	memset(AspPswdFirst, 0, sizeof(AspPswdFirst));
	memset(AspPswdConfirm, 0, sizeof(AspPswdConfirm));

	/* Gate on current SPR so we abort before prompting if already locked. */
	Status = XSfl_FlashSectorProtectionRead(SflHandler, &SprBefore);
	if (Status != XST_SUCCESS) {
		AspPrintResult(XSFL_ASP_RESULT_OP_FAILED);
		return XSFL_ASP_RESULT_OP_FAILED;
	}
	if ((SprBefore & XSFL_ASP_PASSWORD_PROT_MODE_MASK) == 0U) {
		AspPrintResult(XSFL_ASP_RESULT_PASSWORD_LOCKED);
		return XSFL_ASP_RESULT_PASSWORD_LOCKED;
	}

	Status = XSfl_FlashAspGetPasswordByteLen(SflHandler, &AspPswdLen);
	if (Status != XST_SUCCESS) {
		AspPrintResult(XSFL_ASP_RESULT_OP_FAILED);
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	xil_printf("ASP: first entry (%u hex digits)\r\n",
		   (unsigned)(AspPswdLen * 2U));
	if (!AspScanPassword(AspPswdFirst, AspPswdLen)) {
		memset(AspPswdFirst, 0, sizeof(AspPswdFirst));
		AspPrintResult(XSFL_ASP_RESULT_INVALID_PASSWORD);
		return XSFL_ASP_RESULT_INVALID_PASSWORD;
	}

	xil_printf("ASP: confirm\r\n");
	if (!AspScanPassword(AspPswdConfirm, AspPswdLen)) {
		memset(AspPswdFirst, 0, sizeof(AspPswdFirst));
		memset(AspPswdConfirm, 0, sizeof(AspPswdConfirm));
		AspPrintResult(XSFL_ASP_RESULT_INVALID_PASSWORD);
		return XSFL_ASP_RESULT_INVALID_PASSWORD;
	}

	if (memcmp(AspPswdFirst, AspPswdConfirm, AspPswdLen) != 0) {
		memset(AspPswdFirst, 0, sizeof(AspPswdFirst));
		memset(AspPswdConfirm, 0, sizeof(AspPswdConfirm));
		AspPrintResult(XSFL_ASP_RESULT_PASSWORD_MISMATCH);
		return XSFL_ASP_RESULT_PASSWORD_MISMATCH;
	}
	memset(AspPswdConfirm, 0, sizeof(AspPswdConfirm));

	if (AspPwdBufAllZero(AspPswdFirst, AspPswdLen)) {
		memset(AspPswdFirst, 0, sizeof(AspPswdFirst));
		AspPrintResult(XSFL_ASP_RESULT_INVALID_PASSWORD);
		return XSFL_ASP_RESULT_INVALID_PASSWORD;
	}

	memset(&Params, 0, sizeof(Params));
	Params.Password = AspPswdFirst;
	Params.PasswordLen = AspPswdLen;

	Result = XSfl_AspSetPassword(SflHandler, &Params);
	memset(AspPswdFirst, 0, sizeof(AspPswdFirst));

	AspPrintResult(Result);
	return Result;
}

/*****************************************************************************/
/**
 *
 * This function locks a range of sectors using ASP nonvolatile lock commands.
 * If password protection is enabled in the SPR, it prompts for the password
 * and unlocks before applying NV sector locks over the specified range.
 *
 * @param[in] SflHandler is the index to the Sfl interface driver instance.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
static u32 AspLockSectors(u8 SflHandler)
{
	u32 Status;
	u32 Result;
	u16 SprValue;
	u32 StartSector = 0U;
	u32 EndSector = 0U;
	unsigned long UserInput;
	u32 NumSectors;
	u8 AspPswd[XSFL_ASP_PASSWORD_MAX_BYTES];
	u8 HavePassword = 0U;
	XSfl_AspParams Params;

	if (AspGetNumSectors(SflHandler, &NumSectors) == XST_SUCCESS &&
	    NumSectors > 0U) {
		xil_printf("\r\nASP: valid sector index 0..%lu\r\n",
			   (unsigned long)(NumSectors - 1U));
	}

	xil_printf("\r\nASP: Start sector index: ");
	if (scanf("%lu", &UserInput) != 1) {
		xil_printf("ASP: scanf failed\r\n");
		return XSFL_ASP_RESULT_OP_FAILED;
	}
	StartSector = (u32)UserInput;
	xil_printf("ASP: End sector index: ");
	if (scanf("%lu", &UserInput) != 1) {
		xil_printf("ASP: scanf failed\r\n");
		return XSFL_ASP_RESULT_OP_FAILED;
	}
	EndSector = (u32)UserInput;

	memset(&Params, 0, sizeof(Params));
	Params.StartSector = StartSector;
	Params.EndSector = EndSector;

	/* Prompt for a password only when the SPR shows protection enabled. */
	Status = XSfl_FlashSectorProtectionRead(SflHandler, &SprValue);
	if (Status != XST_SUCCESS) {
		AspPrintResult(XSFL_ASP_RESULT_OP_FAILED);
		return XSFL_ASP_RESULT_OP_FAILED;
	}
	if ((SprValue & XSFL_ASP_PASSWORD_PROT_MODE_MASK) == 0U) {
		u8 AspPswdLen;

		Status = XSfl_FlashAspGetPasswordByteLen(SflHandler, &AspPswdLen);
		if (Status != XST_SUCCESS) {
			AspPrintResult(XSFL_ASP_RESULT_OP_FAILED);
			return XSFL_ASP_RESULT_OP_FAILED;
		}
		if (!AspScanPassword(AspPswd, AspPswdLen)) {
			memset(AspPswd, 0, sizeof(AspPswd));
			AspPrintResult(XSFL_ASP_RESULT_INVALID_PASSWORD);
			return XSFL_ASP_RESULT_INVALID_PASSWORD;
		}
		Params.Password = AspPswd;
		Params.PasswordLen = AspPswdLen;
		HavePassword = 1U;
	}

	Result = XSfl_AspLockSectors(SflHandler, &Params);
	if (HavePassword != 0U) {
		memset(AspPswd, 0, sizeof(AspPswd));
	}

	AspPrintResult(Result);
	return Result;
}

/*****************************************************************************/
/**
 *
 * This function erases the ASP nonvolatile lock bits for all sectors using a
 * bulk erase operation covering every sector on all dies. No per-sector
 * granularity is available for NV lock erase.
 * If password protection is enabled in the SPR, it prompts for the password
 * and unlocks before erasing the NV lock bits.
 *
 * @param[in] SflHandler is the index to the Sfl interface driver instance.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
static u32 AspUnlockSectors(u8 SflHandler)
{
	u32 Status;
	u32 Result;
	u16 SprValue;
	u8 AspPswd[XSFL_ASP_PASSWORD_MAX_BYTES];
	u8 HavePassword = 0U;
	XSfl_AspParams Params;

	xil_printf("\r\nASP: Erasing ALL nonvolatile lock bits (bulk erase)\r\n");

	memset(&Params, 0, sizeof(Params));

	/* Prompt for a password only when the SPR shows protection enabled. */
	Status = XSfl_FlashSectorProtectionRead(SflHandler, &SprValue);
	if (Status != XST_SUCCESS) {
		AspPrintResult(XSFL_ASP_RESULT_OP_FAILED);
		return XSFL_ASP_RESULT_OP_FAILED;
	}
	if ((SprValue & XSFL_ASP_PASSWORD_PROT_MODE_MASK) == 0U) {
		u8 AspPswdLen;

		Status = XSfl_FlashAspGetPasswordByteLen(SflHandler, &AspPswdLen);
		if (Status != XST_SUCCESS) {
			AspPrintResult(XSFL_ASP_RESULT_OP_FAILED);
			return XSFL_ASP_RESULT_OP_FAILED;
		}
		if (!AspScanPassword(AspPswd, AspPswdLen)) {
			memset(AspPswd, 0, sizeof(AspPswd));
			AspPrintResult(XSFL_ASP_RESULT_INVALID_PASSWORD);
			return XSFL_ASP_RESULT_INVALID_PASSWORD;
		}
		Params.Password = AspPswd;
		Params.PasswordLen = AspPswdLen;
		HavePassword = 1U;
	}

	Result = XSfl_AspUnlockSectors(SflHandler, &Params);
	if (HavePassword != 0U) {
		memset(AspPswd, 0, sizeof(AspPswd));
	}

	AspPrintResult(Result);
	return Result;
}

/*****************************************************************************/
/**
 *
 * This function reads and displays the nonvolatile lock status for a range
 * of sectors by issuing READ NV LOCK commands for each sector address.
 *
 * @param[in] SflHandler is the index to the Sfl interface driver instance.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
static u32 AspReadSectorLockStatus(u8 SflHandler)
{
	u32 StartSector = 0U;
	u32 EndSector = 0U;
	u32 ChunkStart;
	u32 ChunkEnd;
	u32 SectorSize;
	u32 Result;
	u32 SectorIdx;
	unsigned long UserInput;
	u32 NumSectors;
	u8 LockStatus[ASP_MAX_READ_SECTORS];
	XSfl_AspParams Params;

	if (AspGetNumSectors(SflHandler, &NumSectors) != XST_SUCCESS ||
	    NumSectors == 0U) {
		AspPrintResult(XSFL_ASP_RESULT_OP_FAILED);
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	xil_printf("\r\nASP: valid sector index 0..%lu\r\n",
		   (unsigned long)(NumSectors - 1U));

	xil_printf("\r\nASP: Start sector index: ");
	if (scanf("%lu", &UserInput) != 1) {
		xil_printf("ASP: scanf failed\r\n");
		return XSFL_ASP_RESULT_OP_FAILED;
	}
	StartSector = (u32)UserInput;
	xil_printf("ASP: End sector index: ");
	if (scanf("%lu", &UserInput) != 1) {
		xil_printf("ASP: scanf failed\r\n");
		return XSFL_ASP_RESULT_OP_FAILED;
	}
	EndSector = (u32)UserInput;

	if (StartSector > EndSector || EndSector >= NumSectors) {
		AspPrintResult(XSFL_ASP_RESULT_INVALID_RANGE);
		return XSFL_ASP_RESULT_INVALID_RANGE;
	}

	if (XSfl_FlashGetInfo(SflHandler, XSFL_SECT_SIZE, &SectorSize) !=
	    XST_SUCCESS) {
		AspPrintResult(XSFL_ASP_RESULT_OP_FAILED);
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	xil_printf("\r\nSector lock [%lu..%lu]\r\n",
		   (unsigned long)StartSector, (unsigned long)EndSector);

	/*
	 * Walk the requested span in windows of ASP_MAX_READ_SECTORS so the
	 * whole 0..(NumSectors-1) range can be displayed with a small fixed
	 * buffer instead of one sized to the entire device.
	 */
	for (ChunkStart = StartSector; ChunkStart <= EndSector;
	     ChunkStart += ASP_MAX_READ_SECTORS) {
		ChunkEnd = ChunkStart + (ASP_MAX_READ_SECTORS - 1U);
		if (ChunkEnd > EndSector) {
			ChunkEnd = EndSector;
		}

		memset(&Params, 0, sizeof(Params));
		Params.StartSector = ChunkStart;
		Params.EndSector = ChunkEnd;
		Params.LockStatusOut = LockStatus;
		Params.LockStatusCap = ASP_MAX_READ_SECTORS;

		Result = XSfl_AspReadSectorLock(SflHandler, &Params);
		if (Result != XSFL_ASP_RESULT_OK) {
			AspPrintResult(Result);
			return Result;
		}

		for (SectorIdx = ChunkStart; SectorIdx <= ChunkEnd;
		     SectorIdx++) {
			u32 SectorAddr = SectorIdx * SectorSize;

			xil_printf("  sector %4lu addr 0x%08lX NV=%u\r\n",
				   (unsigned long)SectorIdx,
				   (unsigned long)SectorAddr,
				   (unsigned)LockStatus[SectorIdx -
							 ChunkStart]);
		}
	}

	return XSFL_ASP_RESULT_OK;
}

/*****************************************************************************/
/**
 *
 * This function prompts and reads a hex password string from stdin, validates
 * each character, and converts it into a binary byte array.
 *
 * @param[in] OutPwd is the buffer that receives the parsed password bytes.
 * @param[in] LenBytes is the expected password length in bytes.
 *
 * @return	1 if successful, 0 on input or validation failure.
 *
 ******************************************************************************/
static int AspScanPassword(u8 OutPwd[XSFL_ASP_PASSWORD_MAX_BYTES],
			   u8 LenBytes)
{
	char AspHexInput[XSFL_ASP_PASSWORD_MAX_BYTES * 2U + 8];
	unsigned int ByteValue;
	u32 ByteIdx;
	u32 HexDigits = (u32)LenBytes * 2U;

	memset(OutPwd, 0, XSFL_ASP_PASSWORD_MAX_BYTES);

	xil_printf(
	    "\r\nEnter (%lu hex digits, MSB-first per byte):\r\n",
	    (unsigned long)HexDigits);

	if (scanf("%71s", AspHexInput) != 1) {
		xil_printf("ASP: scanf failed\r\n");
		memset(AspHexInput, 0, sizeof(AspHexInput));
		return 0;
	}

	if (strlen(AspHexInput) != HexDigits) {
		xil_printf("ASP: need exactly %lu hex digits\r\n",
			   (unsigned long)HexDigits);
		memset(AspHexInput, 0, sizeof(AspHexInput));
		return 0;
	}

	for (ByteIdx = 0U; ByteIdx < (u32)LenBytes; ByteIdx++) {
		if (!isxdigit((unsigned char)AspHexInput[ByteIdx * 2U]) ||
		    !isxdigit((unsigned char)AspHexInput[ByteIdx * 2U + 1U])) {
			xil_printf("ASP: bad hex digit\r\n");
			memset(AspHexInput, 0, sizeof(AspHexInput));
			memset(OutPwd, 0, XSFL_ASP_PASSWORD_MAX_BYTES);
			return 0;
		}
		if (sscanf(&AspHexInput[ByteIdx * 2U], "%2x", &ByteValue) != 1) {
			memset(AspHexInput, 0, sizeof(AspHexInput));
			memset(OutPwd, 0, XSFL_ASP_PASSWORD_MAX_BYTES);
			return 0;
		}
		OutPwd[ByteIdx] = (u8)ByteValue;
	}

	memset(AspHexInput, 0, sizeof(AspHexInput));
	return 1;
}

/*****************************************************************************/
/**
 *
 * This function checks whether the password buffer contains all zeros, which
 * is not a valid ASP password value.
 *
 * @param[in] Pw is the password buffer to check.
 * @param[in] LenBytes is the number of bytes to examine.
 *
 * @return	1 if all bytes are zero, 0 otherwise.
 *
 ******************************************************************************/
static int AspPwdBufAllZero(const u8 Pw[XSFL_ASP_PASSWORD_MAX_BYTES],
			    u32 LenBytes)
{
	u32 ByteIdx;

	for (ByteIdx = 0U; ByteIdx < LenBytes; ByteIdx++) {
		if (Pw[ByteIdx] != 0U)
			return 0;
	}
	return 1;
}

#else /* CONFIG_ADVANCED_SECTOR_PROTECTION */

#error "xilsfl_ospi_sector_lock_example.c requires xilsfl_CONFIG_ADVANCED_SECTOR_PROTECTION=true in the BSP. Enable via config_bsp, rebuild the BSP, then create/build this example."

#endif /* CONFIG_ADVANCED_SECTOR_PROTECTION */
