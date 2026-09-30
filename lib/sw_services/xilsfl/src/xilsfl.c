/******************************************************************************
 * Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilsfl.c
 * @brief Implements the serial flash library (SFL) APIs for flash transfers.
 *
 * This file implements the functions required to use the SFL library to
 * perform a transfer. You can access these functions through xilsfl.h.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.0   sb  8/20/24  Initial release
 * 1.0   sb  9/25/24  Add check for bytecount in non-blocking read and
 *                    add support for unaligned byte read
 * 1.1   sb  02/11/25  Add support for x2/x4 operation.
 * 1.3   zm  03/09/26  Add support for W25Q128JW Winbond flash.
 * 1.3   sb  07/20/26  Add support for Advanced Sector Protection (ASP).
 * 1.4   vn  08/26/26  Fix Doxygen warnings.
 * 1.4   vn  09/30/26  Align Doxygen comments with AMD standards
 *
 * </pre>
 *
 ******************************************************************************/

/** @addtogroup xilsfl overview
 * @{
 */

/***************************** Include Files *********************************/
#include "xilsfl.h"
#include "xilsfl_control.h"

#if CONFIG_ADVANCED_SECTOR_PROTECTION
#include <string.h>

/** Micron OCTAL ASP vendor operations table. */
extern const XSfl_AspVendorOps XSfl_AspMicronOctalAspOpsTable;

/** Stub ASP vendor operations table for flash parts without a vendor table. */
static const XSfl_AspVendorOps XSfl_AspVendorStubAspOpsTable;
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XSfl  SflInstance; /**< SFL library instance. */

/*****************************************************************************/
/**
 *
 * @brief Initializes the controller interface and connected serial NOR flash.
 *
 * @param[out]  SflHandler Pointer to the SFL handler index.
 * @param[in]   SflUserOptions Controller and flash configuration information.
 * @param[in]   ControllerInfo Type of controller.
 *
 * @return
 *              - SFL index on successful initialization.
 *              - Error codes otherwise.
 *
 ******************************************************************************/
u32 XSfl_FlashInit(u8 *SflHandler, XSfl_UserConfig SflUserOptions, u8 ControllerInfo){

	u32 FlashSize;
	u32 Status;
	u8 FlashType;
	u8 FCTIndex;
	u8 DualSize;
	u8 Idx = SflInstance.Index;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 SflReadBuffer[8];
#else
	u8 SflReadBuffer[8] __attribute__ ((aligned(4)));
#endif

	/* Validate the input arguments */
	Xil_AssertNonvoid(SflHandler != NULL);

	if(Idx >= XSFL_NUM_INSTANCES){
#ifdef XSFL_DEBUG
		xil_printf("Sfl instance index is exceeded the max sfl no. of instances\n");
#endif
		return XST_FAILURE;
	}

	switch(ControllerInfo){
#ifdef XPAR_XOSPIPSV_NUM_INSTANCES
		case XSFL_OSPI_CNTRL:
			Status = XSfl_OspiInit(&SflInstance.Instance[Idx], &SflUserOptions);
			break;
#endif
		default:
#ifdef XSFL_DEBUG
			xil_printf("Invalid Controller Type\n");
#endif
			Status = XST_FAILURE;

	}
	if (Status != (u32)XST_SUCCESS) {
#ifdef XSFL_DEBUG
		xil_printf("Sfl controller initialization Failed\n");
#endif
		return XST_FAILURE;
	}

	/* Read Device ID */
	Status = XSfl_FlashIdRead(&SflInstance.Instance[Idx],
			SflInstance.Instance[Idx].CntrlInfo.ChipSelectNum, SflReadBuffer);
	if (Status != XST_SUCCESS) {
#ifdef XSFL_DEBUG
		xil_printf("Sfl ReadFlashId Failed\n");
#endif
		return XST_FAILURE;
	}

	FCTIndex = SflInstance.Instance[Idx].SflFlashInfo.FlashIndex;
	FlashType = Flash_Config_Table[FCTIndex].FlashType;
	FlashSize = Flash_Config_Table[FCTIndex].FlashDeviceSize;
	SflInstance.Instance[Idx].SflFlashInfo.AddrSize = 3;

	if ((FlashType == XSFL_QSPI_FLASH) &&
			((SflInstance.Instance[Idx].CntrlInfo.BusWidth == XSFL_X4_BUS_WIDTH) ||
			 (SflInstance.Instance[Idx].CntrlInfo.BusWidth == 0U))) {
		u8 cs;

		/* In Stacked mode, both flash chips must be configured in Quad mode. */
		if (SflInstance.Instance[Idx].SflFlashInfo.ConnectionMode ==
				XSFL_CONNECTION_MODE_STACKED) {
			for (cs = XSFL_SELECT_FLASH_CS0; cs <= XSFL_SELECT_FLASH_CS1; cs++) {
				Status = XSfl_FlashEnableQuadMode(&SflInstance.Instance[Idx], cs);
				if (Status != XST_SUCCESS) {
#ifdef XSFL_DEBUG
					xil_printf("XSfl_FlashEnableQuadMode CS%u Failed\r\n", cs);
#endif
					return XST_FAILURE;
				}
			}
		/* In normal mode, enable Quad mode in the configured flash chip */
		} else {

			Status = XSfl_FlashEnableQuadMode(&SflInstance.Instance[Idx],
					SflInstance.Instance[Idx].CntrlInfo.ChipSelectNum);
			if (Status != XST_SUCCESS) {
#ifdef XSFL_DEBUG
				xil_printf("XSfl_FlashEnableQuadMode CS%u Failed\r\n",
						SflInstance.Instance[Idx].CntrlInfo.ChipSelectNum);
#endif
				return XST_FAILURE;
			}
		}
	}

	if (SflInstance.Instance[Idx].CntrlInfo.CntrlType == XSFL_OSPI_CNTRL) {
		if ((FlashType == XSFL_OSPI_FLASH) &&
				((SflInstance.Instance[Idx].CntrlInfo.BusWidth == XSFL_X8_BUS_WIDTH) ||
				 !(SflInstance.Instance[Idx].CntrlInfo.BusWidth))) {
			/* Set Flash device and Controller mode to DDR Phy */
			Status = XSfl_FlashSetSDRDDRMode(&SflInstance.Instance[Idx], XSFL_EDGE_MODE_DDR_PHY);
			if (Status == XST_SUCCESS) {
#ifdef XSFL_DEBUG
				xil_printf("Controller and Flash are in DDR Phy mode\n");
#endif
				SflInstance.Instance[Idx].CntrlInfo.SdrDdrMode = XSFL_EDGE_MODE_DDR_PHY;
				goto set_mode_done;
			} else {
				Status = SflInstance.Instance[Idx].CntrlInfo.DeviceReset(XOSPIPSV_HWPIN_RESET);
				if (Status != (u32)XST_SUCCESS) {
#ifdef XSFL_DEBUG
					xil_printf("Sfl device reset failed\n");
#endif
					return XST_FAILURE;
				}
			}
		}

		/* Set Flash device and Controller mode to SDR Phy */
		if (SflInstance.Instance[Idx].CntrlInfo.RefClockHz <= Flash_Config_Table[FCTIndex].SdrMaxFreq) {
			Status = XSfl_FlashSetSDRDDRMode(&SflInstance.Instance[Idx], XSFL_EDGE_MODE_SDR_PHY);
			if (Status == XST_SUCCESS) {
#ifdef XSFL_DEBUG
				xil_printf("Controller and Flash are in SDR Phy mode\n");
#endif
				SflInstance.Instance[Idx].CntrlInfo.SdrDdrMode = XSFL_EDGE_MODE_SDR_PHY;
				goto set_mode_done;
			} else {
				Status = SflInstance.Instance[Idx].CntrlInfo.DeviceReset(XOSPIPSV_HWPIN_RESET);
				if (Status != (u32)XST_SUCCESS) {
#ifdef XSFL_DEBUG
					xil_printf("Sfl device reset failed\n");
#endif
					return XST_FAILURE;
				}
			}
		}

		/* Set Flash device and Controller mode to SDR Phy */
		Status = XSfl_FlashSetSDRDDRMode(&SflInstance.Instance[Idx], XSFL_EDGE_MODE_SDR_NON_PHY);
		if (Status != XST_SUCCESS) {
#ifdef XSFL_DEBUG
			xil_printf("XSfl_FlashSetSDRDDRMode Failed\n");
#endif
			return XST_FAILURE;
		} else {
			SflInstance.Instance[Idx].CntrlInfo.SdrDdrMode = XSFL_EDGE_MODE_SDR_NON_PHY;
#ifdef XSFL_DEBUG
			xil_printf("Controller and Flash are in SDR Non Phy mode\n");
#endif
		}
	}

set_mode_done:

#ifdef XSFL_DEBUG
	xil_printf("Flash connection mode : %d\n\r",
			SflInstance.Instance[Idx].SflFlashInfo.ConnectionMode);
	xil_printf("where 0 - Single; 1 - Stacked; 2 - Parallel\n\r");
#endif

	if (FlashSize > XSFL_SIXTEENMB) {
		Status = XSfl_FlashEnterExit4BAddMode(&SflInstance.Instance[Idx], 1,
				SflInstance.Instance[Idx].CntrlInfo.ChipSelectNum);
		if (Status != XST_SUCCESS) {
#ifdef XSFL_DEBUG
			xil_printf("Sfl FlashEnterExit4BAddMode Failed\n");
#endif
			return XST_FAILURE;
		}
		SflInstance.Instance[Idx].SflFlashInfo.AddrSize = 4;
	}

	if (SflInstance.Instance[Idx].SflFlashInfo.ConnectionMode ==
			XSFL_CONNECTION_MODE_STACKED) {
		DualSize = 2;
	} else {
		DualSize = 1;
	}

	/* Copy flash device information in sfl instance */
	SflInstance.Instance[Idx].SflFlashInfo.DeviceSize = Flash_Config_Table[FCTIndex].FlashDeviceSize * DualSize;
	SflInstance.Instance[Idx].SflFlashInfo.PageCount = Flash_Config_Table[FCTIndex].NumPage * DualSize;
	SflInstance.Instance[Idx].SflFlashInfo.SectCount = Flash_Config_Table[FCTIndex].NumSect * DualSize;
	SflInstance.Instance[Idx].SflFlashInfo.SectSize = Flash_Config_Table[FCTIndex].SectSize;
	SflInstance.Instance[Idx].SflFlashInfo.PageSize = Flash_Config_Table[FCTIndex].PageSize;

#if CONFIG_ADVANCED_SECTOR_PROTECTION
	XSfl_FlashAspAssignVendorOps(&SflInstance.Instance[Idx]);
#endif

	*SflHandler = Idx;
	/* Increment the Sfl Index to point next available instance */
	SflInstance.Index++;

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief Reads from the flash connected to the specific interface.
 *
 * @param[in]   SflHandler Index of the SFL interface component to use.
 * @param[in]   Address Address of the flash to read.
 * @param[in]   ByteCount Total size to read.
 * @param[out]  ReadBfrPtr Read buffer to which valid received data is written.
 * @param[in]   RxAddr64bit 64-bit address of the destination read buffer to
 *              which valid received data is written.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_FlashReadStart(u8 SflHandler, u32 Address, u32 ByteCount,
		u8 *ReadBfrPtr, u64 RxAddr64bit){
	u32 Status;

	/* Validate the input arguments */
	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	Xil_AssertNonvoid(ReadBfrPtr != NULL);

	/* Check is byte count is word aligned */
	if((ByteCount % 4) != 0) {
#ifdef XSFL_DEBUG
		xil_printf("Byte count is not word aligned\n");
#endif
		return XST_FAILURE;
	}

	XSfl_Interface *SflInstancePtr = &SflInstance.Instance[SflHandler];
	Status = XSfl_FlashNonBlockingReadProcess(SflInstancePtr,Address,ByteCount,ReadBfrPtr, RxAddr64bit);
	if (Status != XST_SUCCESS) {
#ifdef XSFL_DEBUG
		xil_printf("XSfl_FlashReadProcess Failed\n");
#endif
		return XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief Checks the read completion status of the flash on the interface.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_FlashReadDone(u8 SflHandler){
	u32 Status;

	/* Validate the input arguments */
	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);

	XSfl_Interface *SflInstancePtr = &SflInstance.Instance[SflHandler];
	Status = XSfl_FlashTransferDone(SflInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief Reads data from the flash.
 *
 * @param[in]   SflHandler Index of the SFL interface component to use.
 * @param[in]   Address Address of the flash to read.
 * @param[in]   ByteCount Total size to read.
 * @param[out]  ReadBfrPtr Read buffer to which valid received data is written.
 * @param[in]   RxAddr64bit 64-bit address of the destination read buffer to
 *              which valid received data is written.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_FlashRead(u8 SflHandler, u32 Address, u32 ByteCount,
		u8 *ReadBfrPtr, u64 RxAddr64bit){
	u32 Status;

	/* Validate the input arguments */
	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	Xil_AssertNonvoid(ReadBfrPtr != NULL);

	XSfl_Interface *SflInstancePtr = &SflInstance.Instance[SflHandler];
	Status = XSfl_FlashReadProcess(SflInstancePtr,Address,ByteCount,ReadBfrPtr, RxAddr64bit);
	if (Status != XST_SUCCESS) {
#ifdef XSFL_DEBUG
		xil_printf("XSfl_FlashReadProcess Failed\n");
#endif
		return XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief Writes to the serial NOR flash connected to the interface driver.
 *
 * @param[in]   SflHandler Index of the SFL interface component to use.
 * @param[in]   Address Flash address to write the data to.
 * @param[in]   ByteCount Number of bytes to write.
 * @param[in]   WriteBfrPtr Pointer to the write buffer that holds the data to
 *              transmit.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_FlashWrite(u8 SflHandler, u32 Address, u32 ByteCount,
		u8 *WriteBfrPtr){
	/* Validate the input arguments */
	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	Xil_AssertNonvoid(WriteBfrPtr != NULL);

	u32 Status;
	u32 PageSize;
	u32 PageNum;
	u32 PageCount;
	u32 RemainingBytes = 0;
	u32 PartialBytes = 0;

	XSfl_Interface *SflInstancePtr = &SflInstance.Instance[SflHandler];
	PageSize = SflInstancePtr->SflFlashInfo.PageSize;

	/* Calculate partial bytes if address is not aligned with Page size */
	if((Address % PageSize) != 0 ){
		PartialBytes = PageSize - (Address % PageSize);

		if(PartialBytes > ByteCount) {
			PartialBytes = ByteCount;
		}

		Status = XSfl_FlashPageWrite(SflInstancePtr, Address,
				PartialBytes, WriteBfrPtr);
		if (Status != XST_SUCCESS) {
#ifdef XSFL_DEBUG
			xil_printf("XSfl_FlashPageWrite Failed\n");
#endif
			return XST_FAILURE;
		}

		Address += PartialBytes;
		ByteCount -= PartialBytes;
	}

	/* Calculate No. of pages to write */
	PageCount = ByteCount / PageSize;

	/* Loop over the each page to write */
	for (PageNum = 0; PageNum < PageCount; PageNum++) {
		Status = XSfl_FlashPageWrite(SflInstancePtr,(PageNum * PageSize) + Address,
				(PageSize), WriteBfrPtr + PartialBytes + (PageNum * PageSize));
		if (Status != XST_SUCCESS) {
#ifdef XSFL_DEBUG
			xil_printf("XSfl_FlashPageWrite Failed\n");
#endif
			return XST_FAILURE;
		}
	}

	/* Calculate remaining bytes if any */
	RemainingBytes = ByteCount % PageSize;
	if(RemainingBytes) {
		Address = (PageNum * PageSize) + Address;
		Status = XSfl_FlashPageWrite(SflInstancePtr, Address,
				RemainingBytes, WriteBfrPtr + PartialBytes + (PageNum * PageSize));
		if (Status != XST_SUCCESS) {
#ifdef XSFL_DEBUG
			xil_printf("XSfl_FlashPageWrite Failed\n");
#endif
			return XST_FAILURE;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief Erases the serial NOR flash connected to the specific interface.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   Address Sector offset address of the flash at which to erase
 *              the data.
 * @param[in]   ByteCount Total size to erase.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_FlashErase(u8 SflHandler, u32 Address, u32 ByteCount)
{
	u32 Status;
	u32 Sector;
	u32 SectorCount;
	u32 SectSize;

	/* Validate the input arguments */
	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);

	XSfl_Interface *SflInstancePtr = &SflInstance.Instance[SflHandler];
	SectSize = SflInstancePtr->SflFlashInfo.SectSize;

	/* Check is the Address and byte count is aligned with sector size */
	if(((Address % SectSize) != 0) || ((ByteCount % SectSize) != 0)) {
#ifdef XSFL_DEBUG
		xil_printf(" Address or ByteCount is not aligned with sector size\n");
#endif
		return XST_FAILURE;
	}

	SectorCount = ByteCount / SectSize;
	/* Loop over the each sector to erase */
	for (Sector = 0; Sector < SectorCount; Sector++){
		Status = XSfl_SectorErase(SflInstancePtr, Address);
		if (Status != XST_SUCCESS) {
#ifdef XSFL_DEBUG
			xil_printf("XSfl_SectorErase Failed\n");
#endif
			return XST_FAILURE;
		}

		Address += SectSize;
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief Gets the details of the serial flash on the specific interface.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   Option Option that selects the specific flash detail to get.
 * @param[out]  DataPtr Pointer to which the flash information is copied.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_FlashGetInfo(u8 SflHandler, u8 Option, u32 *DataPtr){

	XSfl_Interface *SflInstancePtr = &SflInstance.Instance[SflHandler];
	u32 FCTIndex = SflInstancePtr->SflFlashInfo.FlashIndex;
	u32 Status = XST_SUCCESS;

	switch(Option){
		case XSFL_FLASH_ID:
			*DataPtr = Flash_Config_Table[FCTIndex].jedec_id;
			break;
		case XSFL_DEVICE_SIZE:
			*DataPtr = SflInstancePtr->SflFlashInfo.DeviceSize;
			break;
		case XSFL_SECT_SIZE:
			*DataPtr = SflInstancePtr->SflFlashInfo.SectSize;
			break;
		case XSFL_PAGE_SIZE:
			*DataPtr = SflInstancePtr->SflFlashInfo.PageSize;
			break;
		case XSFL_FLASH_TYPE:
			*DataPtr = Flash_Config_Table[FCTIndex].FlashType;
			break;
		default:
#ifdef XSFL_DEBUG
			xil_printf("Invalid Option\n");
#endif
			Status = XST_FAILURE;
	}

	return Status;
}

#if CONFIG_ADVANCED_SECTOR_PROTECTION
/*****************************************************************************/
/**
 *
 * @brief Assigns the ASP vendor operations table of an SFL interface.
 *
 * The function selects the XSfl_Interface ASP vendor ops table pointer. The
 * selection is based on the JEDEC maker ID and the configured flash type in the
 * Flash Configuration Table.
 *
 * @param[in,out] IfPtr Pointer to the initialized SFL interface whose AspOps
 *                field is filled in.
 *
 ******************************************************************************/
void XSfl_FlashAspAssignVendorOps(XSfl_Interface *IfPtr)
{
	u32 FlashType;

	if (IfPtr == NULL) {
		return;
	}

	FlashType = Flash_Config_Table[IfPtr->SflFlashInfo.FlashIndex].FlashType;

	/* Extend with additional vendors / JEDEC + flash-type checks here */
	if (((IfPtr->SflFlashInfo.FlashMake == XSFL_MICRON_OCTAL_ID_BYTE0) &&
	     (FlashType == XSFL_OSPI_FLASH)) ||
	    ((IfPtr->SflFlashInfo.FlashMake == XSFL_MICRON_QUAD_ID_BYTE0) &&
	     (FlashType == XSFL_QSPI_FLASH))) {
		IfPtr->SflFlashInfo.AspOps = &XSfl_AspMicronOctalAspOpsTable;
		return;
	}

	IfPtr->SflFlashInfo.AspOps = &XSfl_AspVendorStubAspOpsTable;
}

/*****************************************************************************/
/**
 *
 * @brief Performs the ASP set-password sequence.
 *
 * The function verifies that password protection is not already active in the
 * SPR. It programs the supplied password, reads it back to verify it, and
 * commits the SPR to enable password protection.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   Params Parameters that carry the password bytes and length.
 *
 * @return
 *              - XSFL_ASP_RESULT_OK if successful.
 *              - An XSfl_AspResult error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_AspSetPassword(u8 SflHandler, const XSfl_AspParams *Params)
{
	u32 Status;
	u16 SprBefore;
	u16 SprWithPswdEnabled;
	const XSfl_AspVendorOps *Ops;
	u8 Readback[XSFL_ASP_PASSWORD_MAX_BYTES];

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	if ((Params == NULL) || (Params->Password == NULL) ||
	    (Params->PasswordLen == 0U) ||
	    (Params->PasswordLen > XSFL_ASP_PASSWORD_MAX_BYTES)) {
		return XSFL_ASP_RESULT_INVALID_PASSWORD;
	}

	/* No vendor ASP implementation for this flash: not a password problem. */
	Ops = SflInstance.Instance[SflHandler].SflFlashInfo.AspOps;
	if ((Ops == NULL) || (Ops->AspPasswordLenBytes == 0U)) {
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	/* The vendor sequence transmits a fixed width, so nothing else fits. */
	if (Params->PasswordLen != Ops->AspPasswordLenBytes) {
		return XSFL_ASP_RESULT_INVALID_PASSWORD;
	}

	Status = XSfl_FlashSectorProtectionRead(SflHandler, &SprBefore);
	if (Status != XST_SUCCESS) {
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	if ((SprBefore & XSFL_ASP_PASSWORD_PROT_MODE_MASK) == 0U) {
		/* Password protection already active; refuse to reprogram. */
		return XSFL_ASP_RESULT_PASSWORD_LOCKED;
	}

	Status = XSfl_FlashPasswordProgram(SflHandler, Params->Password);
	if (Status != XST_SUCCESS) {
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	memset(Readback, 0, sizeof(Readback));
	Status = XSfl_FlashPasswordRead(SflHandler, Readback);
	if (Status != XST_SUCCESS) {
		memset(Readback, 0, sizeof(Readback));
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	if (memcmp(Readback, Params->Password, Params->PasswordLen) != 0) {
		memset(Readback, 0, sizeof(Readback));
		return XSFL_ASP_RESULT_PASSWORD_MISMATCH;
	}
	memset(Readback, 0, sizeof(Readback));

	SprWithPswdEnabled =
	    SprBefore & (u16)~(u16)XSFL_ASP_PASSWORD_PROT_MODE_MASK;

	Status = XSfl_FlashSectorProtectionWrite(SflHandler, SprWithPswdEnabled);
	if (Status != XST_SUCCESS) {
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	return XSFL_ASP_RESULT_OK;
}

/*****************************************************************************/
/**
 *
 * @brief Locks a range of sectors using ASP nonvolatile lock commands.
 *
 * If password protection is enabled, the function unlocks first. It sets the
 * NV lock bit for each not-already-locked sector in the range. It enables the
 * global freeze bit when protection is active.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   Params Parameters that carry the sector range and optional
 *              password.
 *
 * @return
 *              - XSFL_ASP_RESULT_OK if successful.
 *              - An XSfl_AspResult error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_AspLockSectors(u8 SflHandler, const XSfl_AspParams *Params)
{
	u32 Status;
	u16 SprValue;
	u32 SectorSize;
	u32 SectorIdx;
	u8 Protected;
	const XSfl_AspVendorOps *Ops;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	if (Params == NULL) {
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	Ops = SflInstance.Instance[SflHandler].SflFlashInfo.AspOps;

	Status = XSfl_AspValidateSectorRange(SflHandler, Params->StartSector,
			Params->EndSector);
	if (Status != XSFL_ASP_RESULT_OK) {
		return Status;
	}

	if (XSfl_FlashGetInfo(SflHandler, XSFL_SECT_SIZE, &SectorSize) !=
	    XST_SUCCESS) {
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	if (XSfl_FlashSectorProtectionRead(SflHandler, &SprValue) !=
	    XST_SUCCESS) {
		return XSFL_ASP_RESULT_OP_FAILED;
	}
	Protected = ((SprValue & XSFL_ASP_PASSWORD_PROT_MODE_MASK) == 0U) ?
			1U : 0U;

	if (Protected != 0U) {
		if ((Ops == NULL) || (Ops->AspPasswordLenBytes == 0U)) {
			return XSFL_ASP_RESULT_OP_FAILED;
		}

		if ((Params->Password == NULL) || (Params->PasswordLen == 0U) ||
		    (Params->PasswordLen != Ops->AspPasswordLenBytes)) {
			return XSFL_ASP_RESULT_INVALID_PASSWORD;
		}

		if (XSfl_FlashPasswordUnlock(SflHandler, Params->Password) !=
		    XST_SUCCESS) {
			return XSFL_ASP_RESULT_PASSWORD_MISMATCH;
		}
	}

	for (SectorIdx = Params->StartSector; SectorIdx <= Params->EndSector;
	     SectorIdx++) {
		u32 SectorAddr = SectorIdx * SectorSize;
		u8 NvLocked;

		if (XSfl_FlashNVLockRead(SflHandler, SectorAddr, &NvLocked) !=
		    XST_SUCCESS) {
			return XSFL_ASP_RESULT_OP_FAILED;
		}
		if (NvLocked != 0U) {
			continue;
		}

		if (XSfl_FlashNVSectorLock(SflHandler, SectorAddr) !=
		    XST_SUCCESS) {
			return XSFL_ASP_RESULT_OP_FAILED;
		}
	}

	if (Protected != 0U) {
		if (XSfl_FlashAspWriteGlobalFreeze(SflHandler) != XST_SUCCESS) {
			return XSFL_ASP_RESULT_OP_FAILED;
		}
	}

	return XSFL_ASP_RESULT_OK;
}

/*****************************************************************************/
/**
 *
 * @brief Erases all ASP nonvolatile lock bits.
 *
 * The erase is a bulk erase across every sector on all dies. If password
 * protection is enabled, the function unlocks first and enables the global
 * freeze bit afterwards.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   Params Parameters that carry the optional password.
 *
 * @return
 *              - XSFL_ASP_RESULT_OK if successful.
 *              - An XSfl_AspResult error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_AspUnlockSectors(u8 SflHandler, const XSfl_AspParams *Params)
{
	u16 SprValue;
	u8 Protected;
	const XSfl_AspVendorOps *Ops;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);

	Ops = SflInstance.Instance[SflHandler].SflFlashInfo.AspOps;

	if (XSfl_FlashSectorProtectionRead(SflHandler, &SprValue) !=
	    XST_SUCCESS) {
		return XSFL_ASP_RESULT_OP_FAILED;
	}
	Protected = ((SprValue & XSFL_ASP_PASSWORD_PROT_MODE_MASK) == 0U) ?
			1U : 0U;

	if (Protected != 0U) {
		if ((Ops == NULL) || (Ops->AspPasswordLenBytes == 0U)) {
			return XSFL_ASP_RESULT_OP_FAILED;
		}

		if ((Params == NULL) || (Params->Password == NULL) ||
		    (Params->PasswordLen == 0U) ||
		    (Params->PasswordLen != Ops->AspPasswordLenBytes)) {
			return XSFL_ASP_RESULT_INVALID_PASSWORD;
		}

		if (XSfl_FlashPasswordUnlock(SflHandler, Params->Password) !=
		    XST_SUCCESS) {
			return XSFL_ASP_RESULT_PASSWORD_MISMATCH;
		}
	}

	if (XSfl_FlashEraseNVLockBits(SflHandler) != XST_SUCCESS) {
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	if (Protected != 0U) {
		if (XSfl_FlashAspWriteGlobalFreeze(SflHandler) != XST_SUCCESS) {
			return XSFL_ASP_RESULT_OP_FAILED;
		}
	}

	return XSFL_ASP_RESULT_OK;
}

/*****************************************************************************/
/**
 *
 * @brief Reads the nonvolatile lock state for a range of sectors.
 *
 * The function stores one flag per sector (1 locked, 0 unlocked) into the
 * caller buffer.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   Params Parameters that carry the sector range and the output
 *              buffer and capacity.
 *
 * @return
 *              - XSFL_ASP_RESULT_OK if successful.
 *              - An XSfl_AspResult error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_AspReadSectorLock(u8 SflHandler, const XSfl_AspParams *Params)
{
	u32 Status;
	u32 SectorSize;
	u32 SectorIdx;
	u32 OutIdx;
	u32 RangeCount;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	if (Params == NULL) {
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	Status = XSfl_AspValidateSectorRange(SflHandler, Params->StartSector,
			Params->EndSector);
	if (Status != XSFL_ASP_RESULT_OK) {
		return Status;
	}

	if (Params->LockStatusOut == NULL) {
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	RangeCount = (Params->EndSector - Params->StartSector) + 1U;
	if (RangeCount > Params->LockStatusCap) {
		return XSFL_ASP_RESULT_INVALID_RANGE;
	}

	if (XSfl_FlashGetInfo(SflHandler, XSFL_SECT_SIZE, &SectorSize) !=
	    XST_SUCCESS) {
		return XSFL_ASP_RESULT_OP_FAILED;
	}

	OutIdx = 0U;
	for (SectorIdx = Params->StartSector; SectorIdx <= Params->EndSector;
	     SectorIdx++) {
		u32 SectorAddr = SectorIdx * SectorSize;
		u8 NvLocked;

		if (XSfl_FlashNVLockRead(SflHandler, SectorAddr, &NvLocked) !=
		    XST_SUCCESS) {
			return XSFL_ASP_RESULT_OP_FAILED;
		}
		Params->LockStatusOut[OutIdx] = NvLocked;
		OutIdx++;
	}

	return XSFL_ASP_RESULT_OK;
}
#endif /* CONFIG_ADVANCED_SECTOR_PROTECTION */

/** @} */
