/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xloader_ospi.c
*
* This is the file which contains ospi related code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bsv   08/23/2018 Initial release
* 1.01  bsv   09/10/2019 Added support to set OSPI to DDR mode
* 1.02  ma   02/03/2020 Change XPlmi_MeasurePerfTime to retrieve Performance
*                       time and print
* 2.00  bsv  27/06/2020 Add dual stacked mode support
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xloader_ospi.h"
#include "xplmi_proc.h"
#include "xloader.h"

#ifdef XLOADER_OSPI
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int FlashReadID(XOspiPsv *OspiPsvPtr);
static int XLoader_FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, u32 Enable);
static int XLoader_FlashSetDDRMode(XOspiPsv *OspiPsvPtr);

/************************** Variable Definitions *****************************/
static XOspiPsv OspiPsvInstance;
static u32 OspiFlashMake;
static u32 OspiFlashSize;
static u8 ChipSelect;

/*****************************************************************************/
/**
 * @brief	This function reads serial FLASH ID connected to the SPI interface.
 * It then deduces the make and size of the flash and obtains the
 * connection mode to point to corresponding parameters in the flash
 * configuration table. The flash driver will function based on this and
 * it presently supports Micron 512Mb.
 *
 * @param	Ospi Instance Pointer
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int FlashReadID(XOspiPsv *OspiPsvPtr)
{
	int Status =  XST_FAILURE;
	u32 Index;
	u8 ReadBuffer[XLOADER_READ_ID_BYTES] __attribute__ ((aligned(32U)));
	XOspiPsv_Msg FlashMsg = {0U};
	u32 TempVal;

	/*
	 * Read ID
	 */
	FlashMsg.Opcode = READ_ID;
	FlashMsg.Addrsize = 0U;
	FlashMsg.Addrvalid = FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ReadBuffer;
	FlashMsg.ByteCount = XLOADER_READ_ID_BYTES;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	
	/* Populate Device Id Data */
	for (Index = 0U, TempVal = 0U; Index < 4U;
		++Index, TempVal += XLOADER_READ_ID_BYTES) {
		OspiPsvPtr->DeviceIdData |= (ReadBuffer[Index] << TempVal);
	}
	XLoader_Printf(DEBUG_GENERAL, "FlashID=0x%x 0x%x 0x%x\n\r",
		ReadBuffer[0U], ReadBuffer[1U], ReadBuffer[2U]);

	/*
	 * Deduce flash make
	 */
	if (ReadBuffer[0U] != MICRON_OCTAL_ID_BYTE0) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_UNSUPPORTED_OSPI, 0U);
		XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_UNSUPPORTED_OSPI\r\n");
		goto END;
	}
	else {
		OspiFlashMake = MICRON_OCTAL_ID_BYTE0;
	}

	if (ReadBuffer[2U] == MICRON_OCTAL_ID_BYTE2_512) {
		OspiFlashSize = XLOADER_FLASH_SIZE_512M;
	}
	else if (ReadBuffer[2U] == MICRON_OCTAL_ID_BYTE2_1G) {
		OspiFlashSize = XLOADER_FLASH_SIZE_1G;
	}
	else if (ReadBuffer[2U] == MICRON_OCTAL_ID_BYTE2_2G) {
		OspiFlashSize = XLOADER_FLASH_SIZE_2G;
	}
	else {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_UNSUPPORTED_OSPI_SIZE,
					0x0U);
		XLoader_Printf(DEBUG_GENERAL,
			"XLOADER_ERR_UNSUPPORTED_OSPI_SIZE\r\n");
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the ospi controller and driver.
 *
 * @param	DeviceFlags are ununsed and passed for compatibility with other
 *		boot device APIs.
 *
 * @return	None
 *
 *****************************************************************************/
int XLoader_OspiInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;
	XOspiPsv_Config *OspiConfig;
	u8 OspiMode;
	(void)DeviceFlags;

	Status = XOspiPsv_DeviceReset(XOSPIPSV_HWPIN_RESET);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	 memset(&OspiPsvInstance, 0U, sizeof(OspiPsvInstance));

	/*
	 * Initialize the OSPI driver so that it's ready to use
	 */
	OspiConfig = XOspiPsv_LookupConfig(XLOADER_OSPI_DEVICE_ID);
	if (NULL == OspiConfig) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_OSPI_INIT, 0x0U);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_OSPI_INIT\r\n");
		goto END;
	}

	Status = XOspiPsv_CfgInitialize(&OspiPsvInstance, OspiConfig);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_OSPI_CFG, Status);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_OSPI_CFG\r\n");
		goto END;
	}

	/*
	 * Enable IDAC controller in OSPI
	 */
	XOspiPsv_SetOptions(&OspiPsvInstance, XOSPIPSV_IDAC_EN_OPTION);
	
	/*
	 * Set the prescaler for OSPIPSV clock
	 */
	XOspiPsv_SetClkPrescaler(&OspiPsvInstance, XOSPIPSV_CLK_PRESCALE_6);
	ChipSelect = XOSPIPSV_SELECT_FLASH_CS0;
	Status = XOspiPsv_SelectFlash(&OspiPsvInstance, ChipSelect);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_OSPI_SEL_FLASH_CS0, Status);
		goto END;
	}

	/*
	 * Read flash ID and obtain all flash related information
	 * It is important to call the read id function before
	 * performing proceeding to any operation, including
	 * preparing the WriteBuffer
	 */
	Status = FlashReadID(&OspiPsvInstance);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_OSPI_READID, Status);
		goto END;
	}

	OspiMode = OspiPsvInstance.Config.ConnectionMode;
	switch(OspiMode) {
		case XOSPIPSV_CONNECTION_MODE_SINGLE:
		case XOSPIPSV_CONNECTION_MODE_STACKED:
			break;

		default:
			Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_OSPI_CONN_MODE, 0U);
			break;
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* OSPI DDR mode is not supported on SPP, EMU and FCV platforms */
	if(XPLMI_PLATFORM == PMC_TAP_VERSION_SILICON) {
		/* Set Flash device and Controller modes */
		Status = XLoader_FlashSetDDRMode(&OspiPsvInstance);
		if (Status != XST_SUCCESS) {
			goto END1;
		}

		if (OspiMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
			ChipSelect = XOSPIPSV_SELECT_FLASH_CS1;
			Status = XOspiPsv_SelectFlash(&OspiPsvInstance, ChipSelect);
			if (Status != XST_SUCCESS) {
				Status = XPLMI_UPDATE_STATUS(
						XLOADER_ERR_OSPI_SEL_FLASH_CS1, Status);
				goto END;
			}

			/* Set Flash device and Controller modes */
			Status = XOspiPsv_SetSdrDdrMode(&OspiPsvInstance,
						XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
			if (Status != XST_SUCCESS) {
				goto END1;
			}

			Status = XLoader_FlashSetDDRMode(&OspiPsvInstance);
			if (Status != XST_SUCCESS) {
				goto END1;
			}

			XOspiPsv_SetClkPrescaler(&OspiPsvInstance,
					XOSPIPSV_CLK_PRESCALE_2);
		}
	}

END1:
	if ((XPLMI_PLATFORM == PMC_TAP_VERSION_SILICON) && (Status != XST_SUCCESS)) {
		Status = XOspiPsv_DeviceReset(XOSPIPSV_HWPIN_RESET);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		ChipSelect = XOSPIPSV_SELECT_FLASH_CS0;
		Status = XOspiPsv_SelectFlash(&OspiPsvInstance, ChipSelect);
		if (Status != XST_SUCCESS) {
			Status = XPLMI_UPDATE_STATUS(
					XLOADER_ERR_OSPI_SEL_FLASH_CS0, Status);
			goto END;
		}

		Status = XOspiPsv_SetSdrDdrMode(&OspiPsvInstance,
				XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
		if (Status != XST_SUCCESS) {
			Status = XPLMI_UPDATE_STATUS(
					XLOADER_ERR_OSPI_SDR_NON_PHY, Status);
			goto END;
		}

		if (OspiMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
			ChipSelect = XOSPIPSV_SELECT_FLASH_CS1;
			Status = XOspiPsv_SelectFlash(&OspiPsvInstance, ChipSelect);
			if (Status != XST_SUCCESS) {
				Status = XPLMI_UPDATE_STATUS(
							XLOADER_ERR_OSPI_SEL_FLASH_CS1, Status);
				goto END;
			}

			Status = XOspiPsv_SetSdrDdrMode(&OspiPsvInstance,
						XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
			if (Status != XST_SUCCESS) {
				Status = XPLMI_UPDATE_STATUS(
							XLOADER_ERR_OSPI_SDR_NON_PHY, Status);
				goto END;
			}
		}
		OspiPsvInstance.Extra_DummyCycle = 0U;
	}

	/*
	 * Enter 4B address mode
	 */
	if (OspiMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
		XLoader_FlashEnterExit4BAddMode(&OspiPsvInstance, TRUE);
		ChipSelect = XOSPIPSV_SELECT_FLASH_CS0;
		Status = XOspiPsv_SelectFlash(&OspiPsvInstance, ChipSelect);
		if (Status != XST_SUCCESS) {
			Status = XPLMI_UPDATE_STATUS(
						XLOADER_ERR_OSPI_SEL_FLASH_CS0, Status);
			goto END;
		}
	}
	XLoader_FlashEnterExit4BAddMode(&OspiPsvInstance, TRUE);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to copy the data from OSPI flash to
 * destination address.
 *
 * @param	SrcAddr is the address of the OSPI flash where copy should start
 *
 * @param	DestAddr is the address of the destination where it should copy to
 *
 * @param	Length Length of the bytes to be copied
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_OspiCopy(u32 SrcAddr, u64 DestAddr, u32 Length, u32 Flags)
{
	int Status = XST_FAILURE;
	XOspiPsv_Msg FlashMsg = {0U};
	u32 TrfLen;
	u8 OspiMode = OspiPsvInstance.Config.ConnectionMode;
#ifdef PLM_PRINT_PERF_DMA
	u64 OspiCopyTime = XPlmi_GetTimerValue();
	XPlmi_PerfTime PerfTime = {0U};
#endif

	XLoader_Printf(DEBUG_INFO, "OSPI Reading Src 0x%0x, Dest 0x%0x%08x, "
		"Length 0x%0x, Flags 0x%0x\r\n", SrcAddr, (u32)(DestAddr >> 32U),
		(u32)(DestAddr), Length, Flags);

	if (OspiMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
		if ((SrcAddr + Length) > (2U * OspiFlashSize)) {
			Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_OSPI_COPY_OVERFLOW, Status);
			goto END;
		}
		else {
			if (SrcAddr < OspiFlashSize) {
				/*
				 * Select lower flash
				 */
				if (ChipSelect == XOSPIPSV_SELECT_FLASH_CS1) {
					ChipSelect = XOSPIPSV_SELECT_FLASH_CS0;
					Status = XOspiPsv_SelectFlash(&OspiPsvInstance, ChipSelect);
					if (Status != XST_SUCCESS) {
						Status = XPLMI_UPDATE_STATUS(
									XLOADER_ERR_OSPI_SEL_FLASH_CS0, Status);
						goto END;
					}
				}
				if ((SrcAddr + Length) > OspiFlashSize) {
					TrfLen = OspiFlashSize - SrcAddr;
				}
				else {
					TrfLen = Length;
				}
			}
			else {
				/*
				 * Select upper flash
				 */
				if (ChipSelect == XOSPIPSV_SELECT_FLASH_CS0) {
					ChipSelect = XOSPIPSV_SELECT_FLASH_CS1;
					Status = XOspiPsv_SelectFlash(&OspiPsvInstance, ChipSelect);
					if (Status != XST_SUCCESS) {
						Status = XPLMI_UPDATE_STATUS(
									XLOADER_ERR_OSPI_SEL_FLASH_CS1, Status);
						goto END;
					}
				}
				SrcAddr = SrcAddr - OspiFlashSize;
				TrfLen = Length;
			}
		}
	}
	else {
		if ((SrcAddr + Length) > OspiFlashSize) {
			Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_OSPI_COPY_OVERFLOW, Status);
			goto END;
		}

		TrfLen = Length;
	}

	/*
	 * Read cmd
	 */
	FlashMsg.Opcode = READ_CMD_OCTAL_4B;
	FlashMsg.Addrsize = XLOADER_OSPI_READ_ADDR_SIZE;
	FlashMsg.Addrvalid = TRUE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.ByteCount = TrfLen;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Addr = SrcAddr;

	if ((DestAddr >> 32U) == 0U) {
		FlashMsg.RxBfrPtr = (u8*)(UINTPTR)DestAddr;
	}
	else {
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.RxAddr64bit = DestAddr;
		FlashMsg.Xfer64bit = 1U;
	}

	if (OspiPsvInstance.SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
		FlashMsg.Dummy = XLOADER_OSPI_DDR_DUMMY_CYCLES +
					OspiPsvInstance.Extra_DummyCycle;
	}
	else {
		FlashMsg.Proto = XOSPIPSV_READ_1_1_8;
		FlashMsg.Dummy = XLOADER_OSPI_SDR_DUMMY_CYCLES;
	}
	
	Status = XOspiPsv_PollTransfer(&OspiPsvInstance, &FlashMsg);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_OSPI_READ, Status);
		goto END;
	}

	if (OspiMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
		if (TrfLen < Length) {
			/*
			 * This code executes when part of image is on flash 0 and the rest
			 * is on flash 1.
			 */
			ChipSelect = XOSPIPSV_SELECT_FLASH_CS1;
			Status = XOspiPsv_SelectFlash(&OspiPsvInstance, ChipSelect);
			if (Status != XST_SUCCESS) {
				Status = XPLMI_UPDATE_STATUS(
							XLOADER_ERR_OSPI_SEL_FLASH_CS1, Status);
				goto END;
			}

			DestAddr = DestAddr + TrfLen;
			TrfLen = Length - TrfLen;
			SrcAddr = 0U;

			/*
			 * Read cmd
			 */
			FlashMsg.Opcode = READ_CMD_OCTAL_4B;
			FlashMsg.Addrsize = XLOADER_OSPI_READ_ADDR_SIZE;
			FlashMsg.Addrvalid = TRUE;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.ByteCount = TrfLen;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
			FlashMsg.Addr = SrcAddr;

			if ((DestAddr >> 32U) == 0U) {
				FlashMsg.RxBfrPtr = (u8*)(UINTPTR)DestAddr;
			}
			else {
				FlashMsg.RxBfrPtr = NULL;
				FlashMsg.RxAddr64bit = DestAddr;
				FlashMsg.Xfer64bit = 1U;
			}

			Status = XOspiPsv_PollTransfer(&OspiPsvInstance, &FlashMsg);
			if (Status != XST_SUCCESS) {
				Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_OSPI_READ,
							Status);
				goto END;
			}
		}
	}

#ifdef	PLM_PRINT_PERF_DMA
	XPlmi_MeasurePerfTime(OspiCopyTime, &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_PERF,
	" %u.%u ms OSPI Copy time: SrcAddr: 0x%08x, DestAddr: 0x%0x08x,"
	"%u Bytes, Flags: 0x%0x\n\r",
	(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac,
	SrcAddr, (u32)(DestAddr >> 32U), (u32)DestAddr, Length, Flags);
#endif

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This API enters the flash device into 4 bytes addressing mode.
* As per the Micron spec, before issuing the command to enter into 4 byte addr
* mode, a write enable command is issued.
*
* @param	OspiPtr is a pointer to the OSPIPSV driver component to use.
* @param	Enable is a either 1 or 0 if 1 then enters 4 byte if 0 exits.
*
* @return	XST_SUCCESS on success and error code on failure
*
******************************************************************************/
static int XLoader_FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, u32 Enable)
{
	int Status = XST_FAILURE;
	u32 Command;
	u8 FlashStatus = 0U;
	XOspiPsv_Msg FlashMsg = {0U};

	if (Enable) {
		Command = ENTER_4B_ADDR_MODE;
	}
	else {
		Command = EXIT_4B_ADDR_MODE;
	}

	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}
	else {
		FlashMsg.Proto = XOSPIPSV_WRITE_1_1_1;
	}

	FlashMsg.Opcode = OSPI_WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0U;
	FlashMsg.Addrvalid = FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0U;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = FALSE;
	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_OSPI_4BMODE, Status);
		goto END;
	}

	FlashMsg.Opcode = Command;
	FlashMsg.Addrvalid = FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = XLOADER_OSPI_ENTER_4B_ADDR_MODE_CMD_BYTE_CNT;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.Addrsize = XLOADER_OSPI_ENTER_4B_ADDR_MODE_CMD_ADDR_SIZE;
	FlashMsg.IsDDROpCode = FALSE;
	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_OSPI_4BMODE, Status);
		goto END;
	}

	FlashMsg.Opcode = READ_FLAG_STATUS_CMD;
	FlashMsg.Addrvalid = FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = &FlashStatus;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Addrsize = XLOADER_OSPI_READ_FLAG_STATUS_CMD_ADDR_SIZE;
	FlashMsg.IsDDROpCode = FALSE;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
		FlashMsg.ByteCount = XLOADER_OSPI_DDR_MODE_BYTE_CNT;
	}
	else {
		FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
		FlashMsg.ByteCount = XLOADER_OSPI_DDR_MODE_BYTE_CNT;
		FlashMsg.Proto = XOSPIPSV_READ_1_1_1;
	}

	while (1U) {
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Dummy += XLOADER_OSPI_DUMMY_CYCLES;
		}
		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_OSPI_4BMODE,
						Status);
			goto END;
		}
		if ((FlashStatus & 0x80U) != 0U) {
			break;
		}
	}

	FlashMsg.Opcode = WRITE_DISABLE_CMD;
	FlashMsg.Addrsize = 0U;
	FlashMsg.Addrvalid = FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0U;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = FALSE;

	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}
	else {
		FlashMsg.Proto = XOSPIPSV_WRITE_1_1_1;
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XLOADER_ERR_OSPI_4BMODE, Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function sets the flash device to Octal DDR mode.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV instance.
*
* @return	XST_SUCCESS on success and error code on failure
*
******************************************************************************/
static int XLoader_FlashSetDDRMode(XOspiPsv *OspiPsvPtr)
{
	int Status = XST_FAILURE;
	u8 ConfigReg[2U] __attribute__ ((aligned(4U)));
	u8 Data[2U] __attribute__ ((aligned(4U))) = {XLOADER_WRITE_CFG_REG_VAL,
						XLOADER_WRITE_CFG_REG_VAL};
	XOspiPsv_Msg FlashMsg = {0U};

	/* Write enable command */
	FlashMsg.Opcode = OSPI_WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0U;
	FlashMsg.Addrvalid = FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0U;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = FALSE;

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Write Config register */
	FlashMsg.Opcode = WRITE_CONFIG_REG;
	FlashMsg.Addrvalid = TRUE;
	FlashMsg.Addrsize = XLOADER_OSPI_WRITE_CFG_REG_CMD_ADDR_SIZE;
	FlashMsg.ByteCount = XLOADER_OSPI_WRITE_CFG_REG_CMD_BYTE_CNT;
	FlashMsg.TxBfrPtr = Data;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = FALSE;

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XOspiPsv_SetSdrDdrMode(OspiPsvPtr, XOSPIPSV_EDGE_MODE_DDR_PHY);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Read Configuration register */
	FlashMsg.Opcode = READ_CONFIG_REG;
	FlashMsg.Addrvalid = TRUE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ConfigReg;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = XLOADER_OSPI_DUMMY_CYCLES + OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = FALSE;
	FlashMsg.ByteCount = XLOADER_OSPI_READ_CFG_REG_CMD_BYTE_CNT;
	FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
	FlashMsg.Addrsize = XLOADER_OSPI_READ_CFG_REG_CMD_ADDR_SIZE;

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if (ConfigReg[0U] != Data[0U]) {
		Status = XST_FAILURE;
		goto END;
	}
	XLoader_Printf(DEBUG_GENERAL,"OSPI mode switched to DDR\n\r");

END:
	return Status;
}
#endif
