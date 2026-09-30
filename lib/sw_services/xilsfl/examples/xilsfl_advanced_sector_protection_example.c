/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

/******************************************************************************/
/**
 * @file xilsfl_advanced_sector_protection_example.c
 * @brief Demonstrates Advanced Sector Protection (ASP) using the SFL library.
 *
 * This file contains a design example using the serial flash library (SFL) for
 * ASP on serial NOR flash. It interacts over stdin. It selects an ASP operation
 * ID and optionally programs the password or nonvolatile (NV) sector locks. It
 * also unlocks NV lock bits and prints the NV lock status for a sector range.
 *
 * The same example covers both quad (QSPI, 4-4-4) and octal (OSPI, 8-8-8) NOR
 * flash devices. The library drives both through the OSPI controller. The ASP
 * layer selects the command framing and dummy cycles from the detected flash
 * make. This example has been tested with the Micron Octal Serial Flash
 * (MT35XU02GCBA).
 *
 * @note
 * The example uses console I/O (scanf). Password buffers stay on
 * the stack only. This example requires the BSP setting
 * xilsfl_CONFIG_ADVANCED_SECTOR_PROTECTION=true.
 *
 *<pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------
 * 1.3   sb   07/20/26 Initial support for Advanced Sector Protection.
 * 1.4   vn   09/30/26 Align Doxygen comments with AMD standards
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
#include "xil_printf.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

/************************** Constant Definitions ****************************/

/**
 * Window size, in sectors, used to fetch the NV lock state. It bounds only the
 * on-stack status buffer. Larger spans are read in successive windows of this
 * size, so this is not a device or vendor limit.
 */
#define ASP_LOCK_STATUS_CHUNK 256U

/**
 * OSPI controller base address. It maps to the BSP parameters, so you can
 * change it in one place for the board or application.
 */
#define OSPI_BASEADDR XPAR_XOSPIPSV_0_BASEADDR

/**************************** Type Definitions ********************************/

/* None additional — XSfl_UserConfig declared in xilsfl.h */

/***************** Macros (Inline Functions) Definitions *********************/

/* None */

/************************** Function Prototypes ******************************/

static void PrintFlashInfoAsp(u8 SflHandlerIdx);
static int XsflAdvancedSectorProtectionExample(void);

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
 * XSfl handler index populated by XSfl_FlashInit. ASP operations use the Micron
 * table when the JEDEC ID and flash type match the BSP assignment logic.
 */
static u8 SflHandler;

/*****************************************************************************/
/**
 * @brief Runs the Advanced Sector Protection demonstration.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - XST_FAILURE otherwise.
 *
 ******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("\r\nAdvanced Sector Protection example\r\n");

	Status = XsflAdvancedSectorProtectionExample();
	if (Status != XST_SUCCESS) {
		xil_printf("\r\nExample failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("\r\nSuccessfully ran Advanced Sector Protection example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Runs the primary Advanced Sector Protection example routine.
 *
 * This function initializes the controller and NOR flash through
 * XSfl_FlashInit and prints the device information. It then reads an ASP
 * operation ID from stdin and dispatches the selected operation.
 *
 * XSFL_OSPI_CNTRL is the controller type for both quad and octal flash devices.
 * XSfl_FlashInit detects the protocol of the connected device.
 *
 * @return
 *              - XST_SUCCESS on success.
 *              - XST_FAILURE otherwise.
 *
 ******************************************************************************/
static int XsflAdvancedSectorProtectionExample(void)
{
	u32 Status = XST_SUCCESS;
	u32 AspOperationId;
	unsigned long UserInput;
	XSfl_UserConfig SflConfig;

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
	return (Status == XST_SUCCESS) ? XST_SUCCESS : XST_FAILURE;
}

/*****************************************************************************/
/**
 * @brief Prints the flash identification details of the connected NOR flash.
 *
 * The details include the JEDEC ID, sector size, device size, and total number
 * of sectors.
 *
 * @param[in]   SflHandlerIdx Index of the SFL interface driver instance.
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
 * @brief Computes the total number of sectors on the connected flash.
 *
 * The computation uses the device size and sector size that XSfl_FlashGetInfo
 * reports.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[out]  NumSectorsOut Pointer that receives the total sector count.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - XST_FAILURE otherwise.
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
 * @brief Prints a human-readable status message for an XSfl_AspResult code.
 *
 * The XSfl_Asp* operations return this code.
 *
 * @param[in]   Result XSfl_AspResult code to describe.
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
 * @brief Sets the ASP password on the NOR flash.
 *
 * This function reads the Sector Protection Register (SPR) to verify that
 * password protection is not already active. It prompts for a double-entry hex
 * password and programs the password. It then verifies the password by
 * readback and commits the SPR.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
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
 * @brief Locks a range of sectors using ASP nonvolatile lock commands.
 *
 * If password protection is enabled in the SPR, this function prompts for the
 * password and unlocks before applying NV sector locks over the range.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
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
 * @brief Erases the ASP nonvolatile lock bits for all sectors.
 *
 * This function uses a bulk erase operation covering every sector on all dies.
 * No per-sector granularity is available for NV lock erase.
 * If password protection is enabled in the SPR, it prompts for the password
 * and unlocks before erasing the NV lock bits.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
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
 * @brief Reads and displays the nonvolatile lock status for a sector range.
 *
 * This function issues READ NV LOCK commands for each sector address.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
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
	u8 LockStatus[ASP_LOCK_STATUS_CHUNK];
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
	 * Walk the requested span in windows of ASP_LOCK_STATUS_CHUNK so the
	 * whole 0..(NumSectors-1) range can be displayed with a small fixed
	 * buffer instead of one sized to the entire device.
	 */
	for (ChunkStart = StartSector; ChunkStart <= EndSector;
	     ChunkStart += ASP_LOCK_STATUS_CHUNK) {
		ChunkEnd = ChunkStart + (ASP_LOCK_STATUS_CHUNK - 1U);
		if (ChunkEnd > EndSector) {
			ChunkEnd = EndSector;
		}

		memset(&Params, 0, sizeof(Params));
		Params.StartSector = ChunkStart;
		Params.EndSector = ChunkEnd;
		Params.LockStatusOut = LockStatus;
		Params.LockStatusCap = ASP_LOCK_STATUS_CHUNK;

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
 * @brief Reads a hex password string from stdin and converts it to bytes.
 *
 * This function prompts for and reads a hex password string from stdin. It
 * validates each character and converts the string into a binary byte array.
 *
 * @param[out]  OutPwd Buffer that receives the parsed password bytes.
 * @param[in]   LenBytes Expected password length in bytes.
 *
 * @return
 *              - 1 if successful.
 *              - 0 on input or validation failure.
 *
 ******************************************************************************/
static int AspScanPassword(u8 OutPwd[XSFL_ASP_PASSWORD_MAX_BYTES],
			   u8 LenBytes)
{
	/* Sized for the scanf field width below plus the terminator, so the two
	 * cannot drift apart. 71 digits covers the 64 of a 32-byte password.
	 * Deliberately not derived from XSFL_ASP_PASSWORD_MAX_BYTES: the field
	 * width has to be a literal in the format string, so a smaller value of
	 * that macro must not be able to shrink this buffer below it.
	 */
	char AspHexInput[72];
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
 * @brief Checks whether the password buffer contains all zeros.
 *
 * An all-zero value is not a valid ASP password value.
 *
 * @param[in]   Pw Password buffer to check.
 * @param[in]   LenBytes Number of bytes to examine.
 *
 * @return
 *              - 1 if all bytes are zero.
 *              - 0 otherwise.
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

#error "xilsfl_advanced_sector_protection_example.c requires xilsfl_CONFIG_ADVANCED_SECTOR_PROTECTION=true in the BSP. Enable via config_bsp, rebuild the BSP, then create/build this example."

#endif /* CONFIG_ADVANCED_SECTOR_PROTECTION */
