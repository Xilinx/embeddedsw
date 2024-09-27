/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilsfl.c
 * @addtogroup xilsfl overview
 * @{
 *
 * The xilsfl.c file implements the functions required to use the SFL library to
 * perform a transfer. These are accessible to the user via sfl.h.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.0   sb  8/20/24  Initial release
 * 1.0   sb  9/25/24  Add check for bytecount in non-blocking read and
 *                    add support for unaligned byte read
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xilsfl.h"
#include "xilsfl_control.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XSfl  SflInstance;

/*****************************************************************************/
/**
 *
 * This function initializes controller interface and serial nor flash connected.
 *
 * @param	SflUserOptions contains the controller and flash configuration info.
 * @param	ControllerInfo contains the type of controller.
 *
 * @return	SFL index on successful initialization, else Error codes.
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

	if (SflInstance.Instance[Idx].CntrlInfo.CntrlType == XSFL_OSPI_CNTRL) {
		if (FlashType == XSFL_OSPI_FLASH){
			/* Set Flash device and Controller mode to DDR Phy */
			Status = XSfl_FlashSetSDRDDRMode(&SflInstance.Instance[Idx], XSFL_EDGE_MODE_DDR_PHY,
					SflReadBuffer);
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
			Status = XSfl_FlashSetSDRDDRMode(&SflInstance.Instance[Idx], XSFL_EDGE_MODE_SDR_PHY,
					SflReadBuffer);
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
		Status = XSfl_FlashSetSDRDDRMode(&SflInstance.Instance[Idx], XSFL_EDGE_MODE_SDR_NON_PHY,
				SflReadBuffer);
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

	*SflHandler = Idx;
	/* Increment the Sfl Index to point next available instance */
	SflInstance.Index++;

	return Status;
}

/*****************************************************************************/
/**
 *
 * This function reads from the flash connected to the specific interface.
 *
 * @param	SflHandler is a index to the Sfl interface component to use.
 * @param	Address contains the address of the flash that needs to be read.
 * @param	ByteCount contains the total size to be erased.
 * @param	ReadBfrPtr is the pointer to the read buffer to which valid received data should be
 * 			written
 * @param	RxAddr64bit is of the 64bit address of destination read buffer to which
 *              valid received data should be written.
 *
 * @return	XST_SUCCESS if successful, else error code.
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
 * This function used to check read completion status of serial Flash
 * connected to the Specific interface.
 *
 * @param	SflHandler is a pointer to the Sfl interface driver component to use.
 *
 * @return	XST_SUCCESS if successful, else error code.
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
 * This function performs read from the flash.
 *
 * @param	SflHandler is a index to the Sfl interface component to use.
 * @param	Address contains the address of the flash that needs to be read.
 * @param	ByteCount contains the total size to be erased.
 * @param	ReadBfrPtr is the pointer to the read buffer to which valid received data should be
 * 			written
 * @param	RxAddr64bit is of the 64bit address of destination read buffer to which
 *              valid received data should be written.
 *
 * @return	XST_SUCCESS if successful, else error code.
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
 * This function writes to the serial nor flash connected to the interface driver.
 *
 * @param	SflHandler is a index to the Sfl interface component to use.
 * @param	Address contains the address to write data to in the Flash.
 * @param	ByteCount contains the number of bytes to be write.
 * @param	WriteBfrPtr is Pointer to the write buffer (which is to be transmitted)
 *
 * @return	XST_SUCCESS if successful, else error code.
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
 * This function erases the serial nor flash connected to the
 * specific interface.
 *
 * @param	SflHandler is a pointer to the Sfl interface component to use.
 * @param	Address is the sector offset address of the flash at which data to be erased.
 * @param	ByteCount contains the total size to be erased.
 *
 * @return	XST_SUCCESS if successful, else error code.
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
 * This function used to get details serial Flash
 * connected to the Specific interface.
 *
 * @param	SflHandler is a pointer to the Sfl interface driver component to use.
 * @param	Option is to get specific details of the flash.
 * @param	DataPtr is the pointer to the flash info to be copied.
 *
 * @return	XST_SUCCESS if successful, else error code.
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
		default:
#ifdef XSFL_DEBUG
			xil_printf("Invalid Option\n");
#endif
			Status = XST_FAILURE;
	}

	return Status;
}
/** @} */
