/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xbir_qspi.c
*
* This is the main file which will contain the QSPI initialization,
* erase, read and write function
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.00  bsv   07/02/20   First release
* 2.00  bsv   03/15/22   Fix bug in stacked mode
* 3.00  skd   01/31/23   Added debug print levels
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xqspipsu.h"
#include "xbir_qspi.h"
#include "xbir_qspi_hw.h"
#include "xbir_qspimap.h"
#include "xbir_config.h"
#include "xbir_err.h"
#include "xbir_sys.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define XBIR_QSPI_DEVICE_ID	XPAR_XQSPIPSU_0_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int Xbir_QspiFlashReadID(XQspiPsu *QspiPsuPtr);
static int Xbir_QspiMacronixEnable4B(XQspiPsu *QspiPsuPtr);
static int Xbir_QspiMacronixEnableQPIMode(XQspiPsu *QspiPsuPtr, int Enable);
static int Xbir_GetFlashInfo(u8 VendorId, u8 SizeId);
static int Xbir_FlashEnterExit4BAddMode(XQspiPsu *QspiPsuPtr, u8 Enable);

/************************** Variable Definitions *****************************/
static XQspiPsu QspiPsuInstance;
static u32 RdCmd = 0U;
static int StatusCmd = 0;
static Xbir_QspiFlashInfo FlashInfo;
static u8 FsrFlag;
static u8 MacronixFlash = FALSE;

/*****************************************************************************/
/**
 * @brief
 * This function reads serial FLASH ID connected to the SPI interface.
 * It then deduces the make and size of the flash and obtains the
 * connection mode to point to corresponding parameters in the flash
 * configuration table. The flash driver will function based on this and
 * it presently supports Micron and Spansion - 128, 256 and 512Mbit and
 * Winbond 128Mbit
 *
 * @param	QspiPsuPtr Pointer to QSPI instance.
 *
 * @return	XST_SUCCESS on successful read ID
 * 		Error code on failure
 *
 *****************************************************************************/
static int Xbir_QspiFlashReadID(XQspiPsu *QspiPsuPtr)
{
	int Status = XST_FAILURE;
	u8 ReadBuffer[4U] __attribute__ ((aligned(32U))) = {0U};
	XQspiPsu_Msg FlashMsg[3U] = {0U};
	u8 RdCmd = READ_ID_CMD;

	/* Read ID */
	FlashMsg[0U].TxBfrPtr = &RdCmd;
	FlashMsg[0U].RxBfrPtr = NULL;
	FlashMsg[0U].ByteCount = 1U;
	FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1U].TxBfrPtr = NULL;
	FlashMsg[1U].RxBfrPtr = ReadBuffer;
	FlashMsg[1U].ByteCount = 4U;
	FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1U].Flags = XQSPIPSU_MSG_FLAG_RX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, &FlashMsg[0U], 2U);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Deduce flash make */
	if (ReadBuffer[0U] == MICRON_ID) {
		FlashInfo.FlashMake = MICRON_ID;
	} else if(ReadBuffer[0U] == SPANSION_ID) {
		FlashInfo.FlashMake = SPANSION_ID;
	} else if(ReadBuffer[0U] == WINBOND_ID) {
		FlashInfo.FlashMake = WINBOND_ID;
	} else if(ReadBuffer[0U] == MACRONIX_ID) {
		FlashInfo.FlashMake = MACRONIX_ID;
		MacronixFlash = TRUE;
	} else if(ReadBuffer[0U] == ISSI_ID) {
		FlashInfo.FlashMake = ISSI_ID;
	} else {
		Status = XBIR_ERROR_QSPI_VENDOR;
		goto END;
	}
	Status = Xbir_GetFlashInfo(ReadBuffer[0U], ReadBuffer[2U]);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_QSPI_CONN_MODE;
		goto END;
	}

	if ((FlashInfo.NumDie > 1U) &&
			(FlashInfo.FlashMake == MICRON_ID)) {
		StatusCmd = READ_FLAG_STATUS_CMD;
		FsrFlag = 1U;
	} else {
		StatusCmd = READ_STATUS_CMD;
		FsrFlag = 0U;
	}

	Xbir_Printf(DEBUG_PRINT_ALWAYS, "[Flash Image Info]\r\n");
	Xbir_Printf(DEBUG_PRINT_ALWAYS, "\t Flash size : %uMB\r\n",
		FlashInfo.FlashSize / (1024U * 1024U));
	Xbir_Printf(DEBUG_PRINT_ALWAYS, "\tSector size : %uKB\r\n",
		FlashInfo.SectSize / 1024U);
	Xbir_Printf(DEBUG_PRINT_ALWAYS, "\tPageSize in bytes: 0x%08X\r\n\r\n",
			FlashInfo.PageSize);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This functions translates the address based on the type of interconnection.
 * In case of stacked, this function asserts the corresponding slave select.
 *
 * @param	Address which is to be accessed
 *
 * @return	QspiAddr is the actual flash address to be accessed - for single
 * 			it is unchanged; for stacked, the lower flash size is
 * 			subtracted; for parallel the address is divided by 2.
 *
 *****************************************************************************/
static u32 Xbir_GetQspiAddr(u32 Address)
{
	u32 RealAddr;

	switch(QspiPsuInstance.Config.ConnectionMode) {
		case XQSPIPSU_CONNECTION_MODE_SINGLE:
			XQspiPsu_SelectFlash(&QspiPsuInstance,
					XQSPIPSU_SELECT_FLASH_CS_LOWER,
					XQSPIPSU_SELECT_FLASH_BUS_LOWER);
			RealAddr = Address;
			break;

		case XQSPIPSU_CONNECTION_MODE_STACKED:
			/* Select lower or upper Flash based on sector address
			 */
			if (Address >= FlashInfo.FlashSize) {
				XQspiPsu_SelectFlash(&QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_UPPER,
						XQSPIPSU_SELECT_FLASH_BUS_LOWER);
				/* Subtract first flash size when accessing
				 *  second flash
				 */
				RealAddr = Address - FlashInfo.FlashSize;
			} else {
				/* Set selection to L_PAGE */
				XQspiPsu_SelectFlash(&QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_LOWER,
						XQSPIPSU_SELECT_FLASH_BUS_LOWER);
				RealAddr = Address;
			}
			break;

		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
			/* The effective address in each flash is the actual
			 * address / 2
			 */
			XQspiPsu_SelectFlash(&QspiPsuInstance,
				XQSPIPSU_SELECT_FLASH_CS_BOTH,
				XQSPIPSU_SELECT_FLASH_BUS_BOTH);
			RealAddr = Address / 2U;
			break;

		default:
			/* We should never reach here as error will be triggered during
			 * QSPI Init for invalid connection mode. Hence, assign a value (0)
			 * to RealAddr, to avoid warning.
			 */
			RealAddr = 0U;
			break;
	}

	if (RealAddr >= FlashInfo.FlashSize) {
		RealAddr = 0U;
	}

	return RealAddr;
}

/*****************************************************************************/
/**
 * @brief
 * This function is used to initialize the qspi controller and driver
 *
 * @param	None
 *
 * @return	XST_SUCCESS on successful initialization
 *		Error code on failure
 *
 *****************************************************************************/
int Xbir_QspiInit(void)
{
	int Status = XST_FAILURE;
	XQspiPsu_Config *QspiConfig;

	/* Initialize the QSPI driver so that it's ready to use */
	QspiConfig =  XQspiPsu_LookupConfig(XBIR_QSPI_DEVICE_ID);
	if (NULL == QspiConfig) {
		Status = XBIR_ERROR_QSPI_CONFIG;
		goto END;
	}

	Status =  XQspiPsu_CfgInitialize(&QspiPsuInstance, QspiConfig,
			QspiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_QSPI_CONFIG_INIT;
		goto END;
	}

	/* Set Manual Start */
	Status = XQspiPsu_SetOptions(&QspiPsuInstance,
		XQSPIPSU_MANUAL_START_OPTION);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_QSPI_MANUAL_START;
		goto END;
	}

	/* Set the pre-scaler for QSPI clock */
	Status = XQspiPsu_SetClkPrescaler(&QspiPsuInstance,
		XQSPIPSU_CLK_PRESCALE_8);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_QSPI_PRESCALER_CLK;
		goto END;
	}

	XQspiPsu_SelectFlash(&QspiPsuInstance,
			XQSPIPSU_SELECT_FLASH_CS_LOWER,
			XQSPIPSU_SELECT_FLASH_BUS_LOWER);

	/* Read Flash ID and extract Manufacturer and Size information */
	Status = Xbir_QspiFlashReadID(&QspiPsuInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	switch ((u32)XPAR_PSU_QSPI_0_QSPI_MODE) {
		case XQSPIPSU_CONNECTION_MODE_SINGLE:
			break;

		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				XQspiPsu_SelectFlash(&QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_BOTH,
						XQSPIPSU_SELECT_FLASH_BUS_BOTH);
				FlashInfo.SectorMask -= FlashInfo.SectSize;
				FlashInfo.SectSize *= 2U;
				FlashInfo.PageSize *= 2U;
			break;

		case XQSPIPSU_CONNECTION_MODE_STACKED:
				FlashInfo.NumSectors *= 2U;
			break;

		default:
			Status = XBIR_ERROR_INVALID_QSPI_CONN;
			break;
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	switch (QspiPsuInstance.Config.BusWidth) {
		case XBIR_QSPI_BUSWIDTH_ONE:
			RdCmd = FAST_READ_CMD_32BIT;
			break;

		case XBIR_QSPI_BUSWIDTH_TWO:
			RdCmd = DUAL_READ_CMD_32BIT;
			break;

		case XBIR_QSPI_BUSWIDTH_FOUR:
			RdCmd = QUAD_READ_CMD_32BIT;
			break;

		default:
			Status = XBIR_ERROR_INVALID_QSPI_CONN;
			break;
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (MacronixFlash == 1U) {
		if (QspiPsuInstance.Config.BusWidth == XBIR_QSPI_BUSWIDTH_FOUR) {
			RdCmd = QUAD_READ_CMD_24BIT2;
		}

		if (QspiPsuInstance.Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL) {
			Status = Xbir_QspiMacronixEnable4B(&QspiPsuInstance);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		} else {
			XQspiPsu_SelectFlash(&QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_LOWER,
						XQSPIPSU_SELECT_FLASH_BUS_LOWER);
			Status = Xbir_QspiMacronixEnable4B(&QspiPsuInstance);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			if (QspiPsuInstance.Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_STACKED) {
				XQspiPsu_SelectFlash(&QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_UPPER,
						XQSPIPSU_SELECT_FLASH_BUS_LOWER);
				Status = Xbir_QspiMacronixEnable4B(&QspiPsuInstance);
				if (Status != XST_SUCCESS) {
					goto END;
				}
			}
		}
	}

	Status = Xbir_FlashEnterExit4BAddMode(&QspiPsuInstance, 1U);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the Qspi Flash Erase stats structure
 *
 * @return	None
 *
 *****************************************************************************/
void Xbir_QspiEraseStatsInit(void)
{
	Xbir_FlashEraseStats *FlashEraseStats = Xbir_GetFlashEraseStats();

	FlashEraseStats->State = XBIR_FLASH_ERASE_NOTSTARTED;
	FlashEraseStats->NumOfSectorsErased = 0U;
	FlashEraseStats->CurrentImgErased = XBIR_QSPI_NO_IMG_ERASED;
	Xbir_QspiGetSectorSize(&FlashEraseStats->SectorSize);
	FlashEraseStats->TotalNumOfSectors = XBIR_QSPI_MAX_BOOT_IMG_SIZE /
						FlashEraseStats->SectorSize;
	if ((XBIR_QSPI_MAX_BOOT_IMG_SIZE % FlashEraseStats->SectorSize) != 0U) {
		FlashEraseStats->TotalNumOfSectors += 1U;
	}
}

/*****************************************************************************/
/**
 * @brief
 * Static API used for Macronix flash to enable 4BYTE mode
 *
 * @param	QspiPsuPtr Pointer to QSPI instance.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int Xbir_QspiMacronixEnable4B(XQspiPsu *QspiPsuPtr)
{
	int Status = XST_FAILURE;
	XQspiPsu_Msg FlashMsg = {0U};
	u8 WriteCmd;

	/* Enable register write */
	WriteCmd = WRITE_ENABLE_CMD;
	FlashMsg.TxBfrPtr = &WriteCmd;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 1U;
	FlashMsg.BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg.Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, &FlashMsg, 1U);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Enable 4 byte mode*/
	WriteCmd = 0xB7U;
	FlashMsg.TxBfrPtr = &WriteCmd;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 1U;
	FlashMsg.BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg.Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, &FlashMsg, 1U);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief
 * Static API used for Macronix flash to enable or disable QPI mode
 *
 * @param	QspiPsuPtr 	Pointer to QSPI instance
 * @param    	Enable 		0 = disable and 1 = enable
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ****************************************************************************/
static int Xbir_QspiMacronixEnableQPIMode(XQspiPsu *QspiPsuPtr, int Enable)
{
	int Status = XST_FAILURE;
	XQspiPsu_Msg FlashMsg = {0U};
	u8 WrCmd = WRITE_ENABLE_CMD;

	/* Enable register write */
	FlashMsg.TxBfrPtr = &WrCmd;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 1U;
	if (Enable == ENABLE_QPI) {
		FlashMsg.BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	} else {
		FlashMsg.BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
	}
	FlashMsg.Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg, 1U);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_QSPI_READ;
		goto END;
	}

	if (Enable == ENABLE_QPI) {
		WrCmd = 0x35U;
	}
	else {
		WrCmd = 0xF5U;
	}
	FlashMsg.TxBfrPtr = &WrCmd;
	FlashMsg.ByteCount = 1U;
	if (Enable == ENABLE_QPI) {
		FlashMsg.BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	} else {
		FlashMsg.BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
	}
	FlashMsg.Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg, 1U);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_QSPI_READ;
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function is used to copy the data from QSPI flash to destination
 * address
 *
 * @param 	SrcAddress 	Address of the QSPI flash where copy should
 * 			start from
 * @param 	DestAddress 	Pointer to destination where it should copy to
 * @param	Length		Length of data bytes to be copied
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int Xbir_QspiRead(u32 SrcAddress, u8* DestAddress, u32 Length)
{
	int Status = XST_FAILURE;
	u32 QspiAddr;
	u32 RemainingBytes;
	u32 TransferBytes;
	u32 DiscardByteCnt;
	XQspiPsu_Msg FlashMsg[5U] = {0U};
	u8 WriteBuf[5U] __attribute__ ((aligned(32U))) = {0U};
	u32 FlashSize = FlashInfo.FlashSize;

	if ((QspiPsuInstance.Config.ConnectionMode ==
		XQSPIPSU_CONNECTION_MODE_STACKED) ||
		(QspiPsuInstance.Config.ConnectionMode ==
		XQSPIPSU_CONNECTION_MODE_PARALLEL)) {
		FlashSize *= 2U;
	}

	/* Check the read length with Qspi flash size */
	if ((SrcAddress + Length) > FlashSize) {
		Status = XBIR_ERROR_QSPI_LENGTH;
		goto END;
	}

	/* Update no of bytes to be copied */
	RemainingBytes = Length;

	while (RemainingBytes > 0U) {
		if (RemainingBytes > DMA_DATA_TRAN_SIZE) {
			TransferBytes = DMA_DATA_TRAN_SIZE;
		} else {
			TransferBytes = RemainingBytes;
		}

		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		QspiAddr = Xbir_GetQspiAddr((u32)SrcAddress);

		/*
		 * Setup the read command with the specified address and data for the
		 * Flash
		 */
		if ((MacronixFlash == 1U) &&
				(QspiPsuInstance.Config.BusWidth == XBIR_QSPI_BUSWIDTH_FOUR)) {
			/* Enable QPI mode */
			Status = Xbir_QspiMacronixEnableQPIMode(&QspiPsuInstance, ENABLE_QPI);
			if (Status != XST_SUCCESS) {
				Status = XBIR_ERROR_QSPI_4BYTE_ENTER;
				goto END;
			}

			/*Command*/
			WriteBuf[COMMAND_OFFSET] = (u8)RdCmd;
			DiscardByteCnt = 1U;
			/*MACRONIX is in QPI MODE 4-4-4*/
			FlashMsg[0U].TxBfrPtr = WriteBuf;
			FlashMsg[0U].RxBfrPtr = NULL;
			FlashMsg[0U].ByteCount = DiscardByteCnt;
			FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
			FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;
			/*Address*/
			WriteBuf[ADDRESS_1_OFFSET] =
							(u8)((QspiAddr & 0xFF000000U) >> 24U);
			WriteBuf[ADDRESS_2_OFFSET] =
							(u8)((QspiAddr & 0xFF0000U) >> 16U);
			WriteBuf[ADDRESS_3_OFFSET] =
							(u8)((QspiAddr & 0xFF00U) >> 8U);
			WriteBuf[ADDRESS_4_OFFSET] =
							(u8)(QspiAddr & 0xFFU);
			DiscardByteCnt = 4U;

			FlashMsg[1U].TxBfrPtr = &WriteBuf[ADDRESS_1_OFFSET];
			FlashMsg[1U].RxBfrPtr = NULL;
			FlashMsg[1U].ByteCount = DiscardByteCnt;
			FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
			FlashMsg[1U].Flags = XQSPIPSU_MSG_FLAG_TX;

			/*Dummy*/
			/*Default Dummy is 6 when QPI READ MODE(ECH)*/
			FlashMsg[2U].TxBfrPtr = NULL;
			FlashMsg[2U].RxBfrPtr = NULL;
			FlashMsg[2U].ByteCount = DUMMY_CLOCKS_MACRONIX;
			FlashMsg[2U].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
			FlashMsg[2U].Flags = 0U;

			/*Data*/
			FlashMsg[3U].TxBfrPtr = NULL;
			FlashMsg[3U].RxBfrPtr = (u8 *)DestAddress;
			FlashMsg[3U].ByteCount = TransferBytes;
			FlashMsg[3U].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
			FlashMsg[3U].Flags = XQSPIPSU_MSG_FLAG_RX;
			if(QspiPsuInstance.Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL){
				FlashMsg[3U].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
			}

			Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg[0U], 4U);
			if (Status != XST_SUCCESS) {
				Status = XBIR_ERROR_QSPI_READ;
				goto END;
			}

			/* Disable QPI mode */
			Status = Xbir_QspiMacronixEnableQPIMode(&QspiPsuInstance, DISABLE_QPI);
			if (Status != XST_SUCCESS) {
				Status = XBIR_ERROR_QSPI_4BYTE_ENTER;
				goto END;
			}
		} else {
			WriteBuf[COMMAND_OFFSET]   = (u8)RdCmd;
			WriteBuf[ADDRESS_1_OFFSET] =
							(u8)((QspiAddr & 0xFF000000U) >> 24U);
			WriteBuf[ADDRESS_2_OFFSET] =
							(u8)((QspiAddr & 0xFF0000U) >> 16U);
			WriteBuf[ADDRESS_3_OFFSET] =
							(u8)((QspiAddr & 0xFF00U) >> 8U);
			WriteBuf[ADDRESS_4_OFFSET] =
							(u8)(QspiAddr & 0xFFU);
			DiscardByteCnt = 5U;

			FlashMsg[0U].TxBfrPtr = WriteBuf;
			FlashMsg[0U].RxBfrPtr = NULL;
			FlashMsg[0U].ByteCount = DiscardByteCnt;
			FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

			/* It is recommended to have a separate entry for dummy
			 */
			if ((RdCmd == FAST_READ_CMD_32BIT) ||
					(RdCmd == DUAL_READ_CMD_32BIT) ||
					(RdCmd == QUAD_READ_CMD_32BIT)) {
				/* It is recommended that Bus width value during dummy
				 * phase should be same as data phase
				 */
				if (RdCmd == FAST_READ_CMD_32BIT) {
					FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
				}

				if (RdCmd == DUAL_READ_CMD_32BIT) {
					FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_DUALSPI;
				}

				if (RdCmd == QUAD_READ_CMD_32BIT) {
					FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
				}

				FlashMsg[1U].TxBfrPtr = NULL;
				FlashMsg[1U].RxBfrPtr = NULL;
				FlashMsg[1U].ByteCount = DUMMY_CLOCKS;
				FlashMsg[1U].Flags = 0U;
			}

			if (RdCmd == FAST_READ_CMD_32BIT) {
				FlashMsg[2U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			}

			if (RdCmd == DUAL_READ_CMD_32BIT) {
				FlashMsg[2U].BusWidth = XQSPIPSU_SELECT_MODE_DUALSPI;
			}

			if (RdCmd == QUAD_READ_CMD_32BIT) {
				FlashMsg[2U].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
			}

			FlashMsg[2U].TxBfrPtr = NULL;
			FlashMsg[2U].RxBfrPtr = DestAddress;
			FlashMsg[2U].ByteCount = TransferBytes;
			FlashMsg[2U].Flags = XQSPIPSU_MSG_FLAG_RX;

			if (QspiPsuInstance.Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL){
				FlashMsg[2U].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
			}

			/*
			 * Send the read command to the Flash to read the
			 * specified number of bytes from the Flash, send the
			 * read command and address and receive the specified
			 * number of bytes of data in the data buffer
			 */
			Status = XQspiPsu_PolledTransfer(&QspiPsuInstance,
				&FlashMsg[0U], 3U);
			if (Status != XST_SUCCESS) {
				Status = XBIR_ERROR_QSPI_READ;
				goto END;
			}
		}

		/* Update the variables */
		RemainingBytes -= TransferBytes;
		DestAddress += TransferBytes;
		SrcAddress += TransferBytes;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function writes to the  serial Flash connected to the QSPIPSU interface.
 * All the data put into the buffer must be in the same page of the device with
 * page boundaries being on 256 byte boundaries.
 *
 * @param	Address 	Address to write data to in the Flash.
 * @param	Length 		Number of bytes to write.
 * @param	WrBuffer	Pointer to data to be written
 *
 * @return	XST_SUCCESS on successful write
 * 		Error code on failure
 *
 ******************************************************************************/
int Xbir_QspiWrite(u32 Address, u8 *WrBuffer, u32 Length)
{
	int Status = XST_FAILURE;
	u8 WrEnableCmd;
	u8 RdStatusCmd;
	u8 FlashStatus[2U] = {0U};
	u8 WrCmd[5U];
	u32 RealAddr;
	u32 CmdByteCount;
	XQspiPsu_Msg FlashMsg[2U] = {0U};

	WrEnableCmd = WRITE_ENABLE_CMD;
	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	RealAddr = Xbir_GetQspiAddr(Address);
	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	FlashMsg[0U].TxBfrPtr = &WrEnableCmd;
	FlashMsg[0U].RxBfrPtr = NULL;
	FlashMsg[0U].ByteCount = 1U;
	FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	WrCmd[COMMAND_OFFSET]   = WRITE_CMD;
	WrCmd[ADDRESS_1_OFFSET] =
				(u8)((RealAddr & 0xFF000000U) >> 24U);
	WrCmd[ADDRESS_2_OFFSET] =
				(u8)((RealAddr & 0xFF0000U) >> 16U);
	WrCmd[ADDRESS_3_OFFSET] =
				(u8)((RealAddr & 0xFF00U) >> 8U);
	WrCmd[ADDRESS_4_OFFSET] =
				(u8)(RealAddr & 0xFFU);
	CmdByteCount = 5U;

	FlashMsg[0U].TxBfrPtr = WrCmd;
	FlashMsg[0U].RxBfrPtr = NULL;
	FlashMsg[0U].ByteCount = CmdByteCount;
	FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1U].TxBfrPtr = WrBuffer;
	FlashMsg[1U].RxBfrPtr = NULL;
	FlashMsg[1U].ByteCount = Length;
	FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1U].Flags = XQSPIPSU_MSG_FLAG_TX;

	if (QspiPsuInstance.Config.ConnectionMode ==
			XQSPIPSU_CONNECTION_MODE_PARALLEL) {
		FlashMsg[1U].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
	}

	Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, FlashMsg, 2U);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/*
	 * Wait for the write command to the Flash to be completed, it takes
	 * some time for the data to be written
	 */
	while (1U) {
		RdStatusCmd = StatusCmd;
		FlashMsg[0U].TxBfrPtr = &RdStatusCmd;
		FlashMsg[0U].RxBfrPtr = NULL;
		FlashMsg[0U].ByteCount = 1U;
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1U].TxBfrPtr = NULL;
		FlashMsg[1U].RxBfrPtr = FlashStatus;
		FlashMsg[1U].ByteCount = 2U;
		FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1U].Flags = XQSPIPSU_MSG_FLAG_RX;
		if (QspiPsuInstance.Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL) {
			FlashMsg[1U].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
		}

		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, FlashMsg, 2U);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		if (QspiPsuInstance.Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL) {
			if (FsrFlag) {
				FlashStatus[1U] &= FlashStatus[0U];
			} else {
				FlashStatus[1U] |= FlashStatus[0U];
			}
		}

		if (FsrFlag != 0U) {
			if ((FlashStatus[1U] & 0x80U) != 0U) {
				break;
			}
		} else {
			if ((FlashStatus[1U] & 0x01U) == 0U) {
				break;
			}
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function erases the sectors in the serial Flash connected to the
 * QSPIPSU interface.
 *
 * @param	Address 	Address of the first sector which needs to
 *			be erased.
 * @param	Length 		Total size to be erased.
 *
 * @return	XST_SUCCESS on successful erase
 *		XST_FAILURE on failure
 *
 * @note	None.
 *
 *****************************************************************************/
int Xbir_QspiFlashErase(u32 Address, u32 Length)
{
	int Status = XST_FAILURE;
	u8 WrEnableCmd;
	u8 RdStatusCmd;
	u8 FlashStatus[2U] = {0U};
	int Sector;
	u32 RealAddr;
	u32 NumSect;
	u8 WrBuffer[8U];
	XQspiPsu_Msg FlashMsg[2U] = {0U};
	u32 SectorOffset;

	SectorOffset = Address & (FlashInfo.SectSize - 1U);
	Address = Address - SectorOffset;
	Length = Length + SectorOffset;

	SectorOffset = Length &  (FlashInfo.SectSize - 1U);
	if (SectorOffset != 0U) {
		Length = Length + FlashInfo.SectSize - SectorOffset;
	}
	NumSect = (Length / FlashInfo.SectSize);
	WrEnableCmd = WRITE_ENABLE_CMD;

	for (Sector = 0U; Sector < NumSect; Sector++) {
		/* Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = Xbir_GetQspiAddr(Address);

		/* Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate
		 * transfer before the write
		 */
		FlashMsg[0U].TxBfrPtr = &WrEnableCmd;
		FlashMsg[0U].RxBfrPtr = NULL;
		FlashMsg[0U].ByteCount = 1U;
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;
		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, FlashMsg, 1U);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		WrBuffer[COMMAND_OFFSET] = SEC_ERASE_CMD;

		/* To be used only if 4B address sector erase cmd is
		 * supported by flash
		 */
		WrBuffer[ADDRESS_1_OFFSET] =
				(u8)((RealAddr & 0xFF000000U) >> 24U);
		WrBuffer[ADDRESS_2_OFFSET] =
				(u8)((RealAddr & 0xFF0000U) >> 16U);
		WrBuffer[ADDRESS_3_OFFSET] =
				(u8)((RealAddr & 0xFF00U) >> 8U);
		WrBuffer[ADDRESS_4_OFFSET] =
				(u8)(RealAddr & 0xFFU);

		FlashMsg[0U].ByteCount = 5U;
		FlashMsg[0U].TxBfrPtr = WrBuffer;
		FlashMsg[0U].RxBfrPtr = NULL;
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, FlashMsg, 1U);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/* Wait for the erase command to be completed */
		while (1U) {
			RdStatusCmd = StatusCmd;
			FlashMsg[0U].TxBfrPtr = &RdStatusCmd;
			FlashMsg[0U].RxBfrPtr = NULL;
			FlashMsg[0U].ByteCount = 1U;
			FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

			FlashMsg[1U].TxBfrPtr = NULL;
			FlashMsg[1U].RxBfrPtr = FlashStatus;
			FlashMsg[1U].ByteCount = 2U;
			FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			FlashMsg[1U].Flags = XQSPIPSU_MSG_FLAG_RX;
			if (QspiPsuInstance.Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL) {
				FlashMsg[1U].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
			}

			Status = XQspiPsu_PolledTransfer(&QspiPsuInstance,
					FlashMsg, 2U);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			if (QspiPsuInstance.Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL) {
				if (FsrFlag) {
					FlashStatus[1U] &= FlashStatus[0U];
				}
				else {
					FlashStatus[1U] |= FlashStatus[0U];
				}
			}

			if (FsrFlag) {
				if ((FlashStatus[1U] & 0x80U) != 0U) {
					break;
				}
			}
			else {
				if ((FlashStatus[1U] & 0x01U) == 0U) {
					break;
				}
			}
		}
		Address += FlashInfo.SectSize;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This API gets the detailed information about the flash that the input
 * VendorID and SizeID correspond to.
 *
 * @param	VendorId is the ID of the flash vendor
 * @param	SizeID is the ID corresponding to the size of the flash
 *
 * @return	XST_SUCCESS on successful matching of VendorID and SizeID
 *			XST_FAILURE on failure
 *
 ******************************************************************************/
static int Xbir_GetFlashInfo(u8 VendorId, u8 SizeId)
{
	int Status = XST_FAILURE;
	u8 Index = 0U;
	Xbir_QspiFlashInfo FlashInfoTbl[] = {
		/* SPANSION */
		/*S25FL064L*/
		{SPANSION_ID, FLASH_SIZE_ID_64M, SECTOR_SIZE_64K, NUM_OF_SECTORS128, BYTES256_PER_PAGE, FLASH_SIZE_64M, 0xFFFF0000U, 1U},
		/*S25FL128L*/
		{SPANSION_ID, FLASH_SIZE_ID_128M, SECTOR_SIZE_64K, NUM_OF_SECTORS256, BYTES256_PER_PAGE, FLASH_SIZE_128M, 0xFFFF0000U, 1U},
		/*S25FL256L*/
		{SPANSION_ID, FLASH_SIZE_ID_256M, SECTOR_SIZE_64K, NUM_OF_SECTORS512, BYTES256_PER_PAGE, FLASH_SIZE_128M, 0xFFFF0000U, 1U},
		/*S25FL512S*/
		{SPANSION_ID, FLASH_SIZE_ID_256M, SECTOR_SIZE_256K, NUM_OF_SECTORS256, BYTES512_PER_PAGE, FLASH_SIZE_256M, 0xFFFC0000U, 1U},
		/* SPANSION 1GBIT IS HANDLED AS 512MBIT STACKED */
		/* MICRON */
		/*N25Q128A11*/
		{MICRON_ID, FLASH_SIZE_ID_128M, SECTOR_SIZE_64K, NUM_OF_SECTORS256, BYTES256_PER_PAGE, FLASH_SIZE_128M, 0xFFFF0000U, 1U},
		/*N25Q128A13*/
		{MICRON_ID, FLASH_SIZE_ID_128M, SECTOR_SIZE_64K, NUM_OF_SECTORS256, BYTES256_PER_PAGE, FLASH_SIZE_128M, 0xFFFF0000U, 1U},
		/*N25Q256AX1*/
		{MICRON_ID, FLASH_SIZE_ID_256M, SECTOR_SIZE_64K, NUM_OF_SECTORS512, BYTES256_PER_PAGE, FLASH_SIZE_256M, 0xFFFF0000U, 1U},
		/*N25Q256A*/
		{MICRON_ID, FLASH_SIZE_ID_256M, SECTOR_SIZE_64K, NUM_OF_SECTORS512, BYTES256_PER_PAGE, FLASH_SIZE_256M, 0xFFFF0000U, 1U},
		/*MT25QU512A*/
		{MICRON_ID, FLASH_SIZE_ID_512M, SECTOR_SIZE_64K, NUM_OF_SECTORS1024, BYTES256_PER_PAGE, FLASH_SIZE_512M, 0xFFFF0000U, 2U},
		/*N25Q512AX3*/
		{MICRON_ID, FLASH_SIZE_ID_512M, SECTOR_SIZE_64K, NUM_OF_SECTORS1024, BYTES256_PER_PAGE, FLASH_SIZE_512M, 0xFFFF0000U, 2U},
		/*N25Q00A*/
		{MICRON_ID, FLASH_SIZE_ID_1G, SECTOR_SIZE_64K, NUM_OF_SECTORS2048, BYTES256_PER_PAGE, FLASH_SIZE_1G, 0xFFFF0000U, 4U},
		/*N25Q00*/
		{MICRON_ID, FLASH_SIZE_ID_1G, SECTOR_SIZE_64K, NUM_OF_SECTORS2048, BYTES256_PER_PAGE, FLASH_SIZE_1G, 0xFFFF0000U, 4U},
		/*MT25QU02G*/
		{MICRON_ID, FLASH_SIZE_ID_2G, SECTOR_SIZE_64K, NUM_OF_SECTORS4096, BYTES256_PER_PAGE, FLASH_SIZE_2G, 0xFFFF0000U, 4U},
		/*MT25QL02G*/
		{MICRON_ID, FLASH_SIZE_ID_2G, SECTOR_SIZE_64K, NUM_OF_SECTORS4096, BYTES256_PER_PAGE, FLASH_SIZE_2G, 0xFFFF0000U, 4U},
		/* WINBOND */
		/*W25Q128FW*/
		{WINBOND_ID, FLASH_SIZE_ID_128M, SECTOR_SIZE_64K, NUM_OF_SECTORS256, BYTES256_PER_PAGE, FLASH_SIZE_128M, 0xFFFF0000U, 1U},
		/*W25Q128JV*/
		{WINBOND_ID, FLASH_SIZE_ID_128M, SECTOR_SIZE_64K, NUM_OF_SECTORS256, BYTES256_PER_PAGE, FLASH_SIZE_128M, 0xFFFF0000U, 1U},
		/* MACRONIX */
		/* MX66L1G45G*/
		{MACRONIX_ID, MACRONIX_FLASH_SIZE_ID_1G, SECTOR_SIZE_64K, NUM_OF_SECTORS2048, BYTES256_PER_PAGE, FLASH_SIZE_1G, 0xFFFF0000U, 1U},
		/* MX66L1G55G*/
		{MACRONIX_ID, MACRONIX_FLASH_SIZE_ID_1G, SECTOR_SIZE_64K, NUM_OF_SECTORS2048, BYTES256_PER_PAGE, FLASH_SIZE_1G, 0xFFFF0000U, 1U},
		/* MX66U1G45G*/
		{MACRONIX_ID, MACRONIX_FLASH_1_8_V_SIZE_ID_1G, SECTOR_SIZE_64K, NUM_OF_SECTORS2048, BYTES256_PER_PAGE, FLASH_SIZE_1G, 0xFFFF0000U, 1U},
		/* MX66L2G45G*/
		{MACRONIX_ID, MACRONIX_FLASH_SIZE_ID_2G, SECTOR_SIZE_64K, NUM_OF_SECTORS4096, BYTES256_PER_PAGE, FLASH_SIZE_2G, 0xFFFF0000U, 1U},
		/* MX66U2G45G*/
		{MACRONIX_ID, 0x3CU, SECTOR_SIZE_64K, NUM_OF_SECTORS4096, BYTES256_PER_PAGE, FLASH_SIZE_2G, 0xFFFF0000U, 1U},
		/* ISSI */
		/* IS25WP080D */
		{ISSI_ID, FLASH_SIZE_ID_8M, SECTOR_SIZE_64K, NUM_OF_SECTORS16, BYTES256_PER_PAGE, FLASH_SIZE_8M, 0xFFFF0000U, 1U},
		/* IS25LP080D */
		{ISSI_ID, FLASH_SIZE_ID_8M, SECTOR_SIZE_64K, NUM_OF_SECTORS16, BYTES256_PER_PAGE, FLASH_SIZE_8M, 0xFFFF0000U, 1U},
		/* IS25WPSPANSION_ID6D */
		{ISSI_ID, FLASH_SIZE_ID_16M, SECTOR_SIZE_64K, NUM_OF_SECTORS32, BYTES256_PER_PAGE, FLASH_SIZE_16M, 0xFFFF0000U, 1U},
		/* IS25LPSPANSION_ID6D */
		{ISSI_ID, FLASH_SIZE_ID_16M, SECTOR_SIZE_64K, NUM_OF_SECTORS32, BYTES256_PER_PAGE, FLASH_SIZE_16M, 0xFFFF0000U, 1U},
		/* IS25WP032 */
		{ISSI_ID, FLASH_SIZE_ID_32M, SECTOR_SIZE_64K, NUM_OF_SECTORS64, BYTES256_PER_PAGE, FLASH_SIZE_32M, 0xFFFF0000U, 1U},
		/*IS25LP032*/
		{ISSI_ID, FLASH_SIZE_ID_32M, SECTOR_SIZE_64K, NUM_OF_SECTORS64, BYTES256_PER_PAGE, FLASH_SIZE_32M, 0xFFFF0000U, 1U},
		/*IS25WP064*/
		{ISSI_ID, FLASH_SIZE_ID_64M, SECTOR_SIZE_64K, NUM_OF_SECTORS128, BYTES256_PER_PAGE, FLASH_SIZE_64M, 0xFFFF0000U, 1U},
		/*IS25LP064*/
		{ISSI_ID, FLASH_SIZE_ID_64M, SECTOR_SIZE_64K, NUM_OF_SECTORS128, BYTES256_PER_PAGE, FLASH_SIZE_64M, 0xFFFF0000U, 1U},
		/*IS25WP128*/
		{ISSI_ID, FLASH_SIZE_ID_128M, SECTOR_SIZE_64K, NUM_OF_SECTORS256, BYTES256_PER_PAGE, FLASH_SIZE_128M, 0xFFFF0000U, 1U},
		/*IS25LP128*/
		{ISSI_ID, FLASH_SIZE_ID_128M, SECTOR_SIZE_64K, NUM_OF_SECTORS256, BYTES256_PER_PAGE, FLASH_SIZE_128M, 0xFFFF0000U, 1U},
		/*IS25LP256D*/
		{ISSI_ID, FLASH_SIZE_ID_256M, SECTOR_SIZE_64K, NUM_OF_SECTORS512, BYTES256_PER_PAGE, FLASH_SIZE_256M, 0xFFFF0000U, 1U},
		/*IS25WP256D*/
		{ISSI_ID, FLASH_SIZE_ID_256M, SECTOR_SIZE_64K, NUM_OF_SECTORS512, BYTES256_PER_PAGE, FLASH_SIZE_256M, 0xFFFF0000U, 1U},
		/*IS25LP512M*/
		{ISSI_ID, MACRONIX_FLASH_SIZE_ID_512M, SECTOR_SIZE_64K, NUM_OF_SECTORS1024, BYTES256_PER_PAGE, FLASH_SIZE_512M, 0xFFFF0000U, 1U},
		/*IS25WP512M*/
		{ISSI_ID, MACRONIX_FLASH_SIZE_ID_512M, SECTOR_SIZE_64K, NUM_OF_SECTORS1024, BYTES256_PER_PAGE, FLASH_SIZE_512M, 0xFFFF0000U, 1U},
		/*IS25LPSPANSION_IDG*/
		{ISSI_ID, MACRONIX_FLASH_SIZE_ID_1G, SECTOR_SIZE_64K, NUM_OF_SECTORS2048, BYTES256_PER_PAGE, FLASH_SIZE_1G, 0xFFFF0000U, 1U},
		/*IS25WPSPANSION_IDG*/
		{ISSI_ID, MACRONIX_FLASH_SIZE_ID_1G, SECTOR_SIZE_64K, NUM_OF_SECTORS2048, BYTES256_PER_PAGE, FLASH_SIZE_1G, 0xFFFF0000U, 1U},
	};

	for (; Index < sizeof(FlashInfoTbl) / sizeof(FlashInfoTbl[0U]);
				Index++) {
		if ((FlashInfoTbl[Index].FlashMake == VendorId) &&
				(FlashInfoTbl[Index].FlashSizeId == SizeId)) {
			FlashInfo.SectSize = FlashInfoTbl[Index].SectSize;
			FlashInfo.PageSize = FlashInfoTbl[Index].PageSize;
			FlashInfo.NumDie = FlashInfoTbl[Index].NumDie;
			FlashInfo.FlashSize = FlashInfoTbl[Index].FlashSize;
			Status = XST_SUCCESS;
			break;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This API enters the flash device into 4 bytes addressing mode.
 * As per the Micron and ISSI spec, before issuing the command to enter
 * into 4 byte addr mode, a write enable command is issued.
 * For Macronix and Winbond flash parts write
 * enable is not required.
 *
 * @param	QspiPsuPtr is a pointer to the QSPIPSU driver component to use.
 * @param	Enable is a either 1 or 0 if 1 then enters 4 byte if 0 exits.
 *
 * @return
 *		- XST_SUCCESS if successful
 *		- XST_FAILURE if it fails
 *
 ******************************************************************************/
static int Xbir_FlashEnterExit4BAddMode(XQspiPsu *QspiPsuPtr, u8 Enable)
{
	int Status = XST_FAILURE;
	u8 WriteEnableCmd;
	u8 Cmd;
	u8 WriteDisableCmd;
	u8 ReadStatusCmd;
	u8 WriteBuffer[2U] = {0U};
	u8 FlashStatus[2U] = {0U};
	XQspiPsu_Msg FlashMsg[2U] = {0U};

	if (Enable) {
		Cmd = ENTER_4B_ADDR_MODE;
	} else {
		if (FlashInfo.FlashMake == ISSI_ID)
			Cmd = EXIT_4B_ADDR_MODE_ISSI;
		else
			Cmd = EXIT_4B_ADDR_MODE;
	}

	switch (FlashInfo.FlashMake) {
	case ISSI_ID:
	case MICRON_ID:
		WriteEnableCmd = WRITE_ENABLE_CMD;
		/*
		 * Send the write enable command to the
		 * Flash so that it can be written to, this
		 * needs to be sent as a separate transfer before
		 * the write
		 */
		FlashMsg[0U].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0U].RxBfrPtr = NULL;
		FlashMsg[0U].ByteCount = 1U;
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1U);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		break;

	case SPANSION_ID:
		if (Enable) {
			WriteBuffer[0U] = BANK_REG_WR_CMD;
			WriteBuffer[1U] = 1U << 7U;
		} else {
			WriteBuffer[0U] = BANK_REG_WR_CMD;
			WriteBuffer[1U] = 0U << 7U;
		}

		FlashMsg[0U].TxBfrPtr = WriteBuffer;
		FlashMsg[0U].RxBfrPtr = NULL;
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;
		FlashMsg[0U].ByteCount = 2U;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1U);
		if (Status != XST_SUCCESS) {
			break;
		}

	default:
		/*
		 * For Macronix and Winbond flash parts
		 * Write enable command is not required.
		 */
		break;
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	FlashMsg[0U].TxBfrPtr = &Cmd;
	FlashMsg[0U].RxBfrPtr = NULL;
	FlashMsg[0U].ByteCount =1U;
	FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1U);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	while (1U) {
		ReadStatusCmd = StatusCmd;

		FlashMsg[0U].TxBfrPtr = &ReadStatusCmd;
		FlashMsg[0U].RxBfrPtr = NULL;
		FlashMsg[0U].ByteCount = 1;
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1U].TxBfrPtr = NULL;
		FlashMsg[1U].RxBfrPtr = FlashStatus;
		FlashMsg[1U].ByteCount = 2;
		FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1U].Flags = XQSPIPSU_MSG_FLAG_RX;

		if (QspiPsuPtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL) {
			FlashMsg[1U].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
		}

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2U);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		if (QspiPsuPtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL) {
			if (FsrFlag) {
				FlashStatus[1U] &= FlashStatus[0U];
			} else {
				FlashStatus[1U] |= FlashStatus[0U];
			}
		}

		if (FsrFlag) {
			if ((FlashStatus[1U] & 0x80U) != 0U) {
				break;
			}
		} else {
			if ((FlashStatus[1U] & 0x01U) == 0U) {
				break;
			}
		}
	}

	switch (FlashInfo.FlashMake) {
	case ISSI_ID:
	case MICRON_ID:
		WriteDisableCmd = WRITE_DISABLE_CMD;
		/*
		 * Send the write enable command to the
		 * Flash so that it can be written to,
		 * this needs to be sent as a separate
		 * transfer before
		 * the write
		 */
		FlashMsg[0U].TxBfrPtr = &WriteDisableCmd;
		FlashMsg[0U].RxBfrPtr = NULL;
		FlashMsg[0U].ByteCount = sizeof(WriteDisableCmd);
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 1U);
		if (Status != XST_SUCCESS) {
			break;
		}
		break;

	default:
		/*
		 * For Macronix and Winbond flash parts
		 * Write disable command is not required.
		 */
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This API returns the sector size of the flash
 *
 * @param	SectorSize is pointer to sector size of flash
 *
 * @return	None
 *
 ******************************************************************************/
void Xbir_QspiGetSectorSize(u32 *SectorSize)
{
	*SectorSize = FlashInfo.SectSize;
}

/*****************************************************************************/
/**
 * @brief
 * This API returns the page size of the flash
 *
 * @param	PageSize is pointer to page size of flash
 *
 * @return	None
 *
 ******************************************************************************/
void Xbir_QspiGetPageSize(u16 *PageSize)
{
	*PageSize = FlashInfo.PageSize;
}

/*****************************************************************************/
/**
 * @brief	This function returns pointer to the flash erase stats structure
 * image id
 *
 * @return	Pointer to FlashEraseStats
 *
 *****************************************************************************/
Xbir_FlashEraseStats* Xbir_GetFlashEraseStats(void)
{
	static Xbir_FlashEraseStats FlashEraseStats;

	return &FlashEraseStats;
}
