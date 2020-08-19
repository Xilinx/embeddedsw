/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
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
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xqspipsu.h"
#include "xbir_qspi.h"
#include "xbir_qspi_hw.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define XBIR_QSPI_DEVICE_ID	XPAR_XQSPIPSU_0_DEVICE_ID

/**************************** Type Definitions *******************************/
typedef struct{
	u8 QspiFlashMake;
	u32 QspiFlashSize;
	u32 SectSize;
	u32 SectMask;
	u8 NumDie;
} Xbir_QspiFlashInfo;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int Xbir_QspiFlashReadID(XQspiPsu *QspiPsuPtr);
static int Xbir_QspiMacronixEnable4B(XQspiPsu *QspiPsuPtr);
static int Xbir_QspiMacronixEnableQPIMode(XQspiPsu *QspiPsuPtr, int Enable);

/************************** Variable Definitions *****************************/
static XQspiPsu Xbir_QspiPsuInstance;
static u32 RdCmd = 0U;
static int StatusCmd = 0;
static Xbir_QspiFlashInfo FlashCfg;
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
		FlashCfg.QspiFlashMake = MICRON_ID;
	} else if(ReadBuffer[0U] == SPANSION_ID) {
		FlashCfg.QspiFlashMake = SPANSION_ID;
	} else if(ReadBuffer[0U] == WINBOND_ID) {
		FlashCfg.QspiFlashMake = WINBOND_ID;
	} else if(ReadBuffer[0U] == MACRONIX_ID) {
		FlashCfg.QspiFlashMake = MACRONIX_ID;
		MacronixFlash = TRUE;
	} else if(ReadBuffer[0U] == ISSI_ID) {
		FlashCfg.QspiFlashMake = ISSI_ID;
	} else {
		Status = XBIR_ERROR_UNSUPPORTED_QSPI_CONN_MODE;
		goto END;
	}

	/* Deduce flash Size */
	if (ReadBuffer[2U] == FLASH_SIZE_ID_8M) {
		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FlashCfg.SectSize = SECTOR_SIZE_64K;
				FlashCfg.SectMask = 0xFFFF0000U;
				FlashCfg.NumDie = 1U;
				break;

			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FlashCfg.SectSize = SECTOR_SIZE_128K;
				FlashCfg.SectMask = 0xFFFE0000U;
				FlashCfg.NumDie = 1U;
				break;

			default:
				Status = XBIR_ERROR_UNSUPPORTED_QSPI_CONN_MODE;
				break;
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
		FlashCfg.QspiFlashSize = FLASH_SIZE_8M;
	} else if (ReadBuffer[2U] == FLASH_SIZE_ID_16M) {
		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FlashCfg.SectSize = SECTOR_SIZE_64K;
				FlashCfg.SectMask = 0xFFFF0000U;
				FlashCfg.NumDie = 1U;
				break;

			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FlashCfg.SectSize = SECTOR_SIZE_128K;
				FlashCfg.SectMask = 0xFFFE0000U;
				FlashCfg.NumDie = 1U;
				break;

			default:
				Status = XBIR_ERROR_UNSUPPORTED_QSPI_CONN_MODE;
				break;
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
		FlashCfg.QspiFlashSize = FLASH_SIZE_16M;
	} else if (ReadBuffer[2U] == FLASH_SIZE_ID_32M) {
		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FlashCfg.SectSize = SECTOR_SIZE_64K;
				FlashCfg.SectMask = 0xFFFF0000U;
				FlashCfg.NumDie = 1U;
				break;

			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FlashCfg.SectSize = SECTOR_SIZE_128K;
				FlashCfg.SectMask = 0xFFFE0000U;
				FlashCfg.NumDie = 1U;
				break;

			default:
				Status = XBIR_ERROR_UNSUPPORTED_QSPI_CONN_MODE;
				break;
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
		FlashCfg.QspiFlashSize = FLASH_SIZE_32M;
	} else if (ReadBuffer[2U] == FLASH_SIZE_ID_64M) {
		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FlashCfg.SectSize = SECTOR_SIZE_64K;
				FlashCfg.SectMask = 0xFFFF0000U;
				FlashCfg.NumDie = 1U;
				break;

			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FlashCfg.SectSize = SECTOR_SIZE_128K;
				FlashCfg.SectMask = 0xFFFE0000U;
				FlashCfg.NumDie = 1U;
				break;

			default:
				Status = XBIR_ERROR_UNSUPPORTED_QSPI_CONN_MODE;
				break;
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
		FlashCfg.QspiFlashSize = FLASH_SIZE_64M;
	} else if (ReadBuffer[2U] == FLASH_SIZE_ID_128M) {
		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FlashCfg.SectSize = SECTOR_SIZE_64K;
				FlashCfg.SectMask = 0xFFFF0000U;
				FlashCfg.NumDie = 1U;
				break;

			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FlashCfg.SectSize = SECTOR_SIZE_128K;
				FlashCfg.SectMask = 0xFFFE0000U;
				FlashCfg.NumDie = 1U;
				break;

			default:
				Status = XBIR_ERROR_UNSUPPORTED_QSPI_CONN_MODE;
				break;
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
		FlashCfg.QspiFlashSize = FLASH_SIZE_128M;
	} else if ((ReadBuffer[2U] == FLASH_SIZE_ID_256M) ||
		(ReadBuffer[2U] == MACRONIX_FLASH_1_8_V_MX25_ID_256)) {
		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FlashCfg.SectSize = SECTOR_SIZE_64K;
				FlashCfg.SectMask = 0xFFFF0000U;
				FlashCfg.NumDie = 1U;
				break;

			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FlashCfg.SectSize = SECTOR_SIZE_128K;
				if(FlashCfg.QspiFlashMake == ISSI_ID) {
					FlashCfg.SectMask = 0xFFFF0000U;
				}
				else {
					FlashCfg.SectMask = 0xFFFE0000U;
				}
				FlashCfg.NumDie = 1U;
				break;

			default:
				Status = XBIR_ERROR_UNSUPPORTED_QSPI_CONN_MODE;
				break;
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
		FlashCfg.QspiFlashSize = FLASH_SIZE_256M;
	} else if ((ReadBuffer[2U] == FLASH_SIZE_ID_512M) ||
		(ReadBuffer[2U] == MACRONIX_FLASH_SIZE_ID_512M) ||
		(ReadBuffer[2U] == MACRONIX_FLASH_1_8_V_MX66_ID_512)) {
		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				if(FlashCfg.QspiFlashMake == SPANSION_ID) {
					FlashCfg.SectSize = SECTOR_SIZE_256K;
					FlashCfg.SectMask = 0xFFFC0000U;
					FlashCfg.NumDie = 1U;
				}
				else {
					FlashCfg.SectSize = SECTOR_SIZE_64K;
					FlashCfg.SectMask = 0xFFFF0000U;
					FlashCfg.NumDie = 2U;
				}
				break;

			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				if(FlashCfg.QspiFlashMake == SPANSION_ID) {
					FlashCfg.SectSize = SECTOR_SIZE_512K;
					FlashCfg.SectMask = 0xFFF80000U;
					FlashCfg.NumDie = 1U;
				}
				else {
					FlashCfg.SectSize = SECTOR_SIZE_128K;
					FlashCfg.SectMask = 0xFFFE0000U;
					FlashCfg.NumDie = 2U;
				}
				break;

			default:
				Status = XBIR_ERROR_UNSUPPORTED_QSPI_CONN_MODE;
				break;
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
		FlashCfg.QspiFlashSize = FLASH_SIZE_512M;
	} else if ((ReadBuffer[2U] == FLASH_SIZE_ID_1G) ||
		(ReadBuffer[2U] == MACRONIX_FLASH_SIZE_ID_1G) ||
		(ReadBuffer[2U] == MACRONIX_FLASH_1_8_V_SIZE_ID_1G)) {
		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FlashCfg.SectSize = SECTOR_SIZE_64K;
				FlashCfg.SectMask = 0xFFFF0000U;
				FlashCfg.NumDie = 4U;
				break;

			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FlashCfg.SectSize = SECTOR_SIZE_128K;
				FlashCfg.SectMask = 0xFFFE0000U;
				FlashCfg.NumDie = 4U;
				break;

			default:
				Status = XBIR_ERROR_UNSUPPORTED_QSPI_CONN_MODE;
				break;
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
		FlashCfg.QspiFlashSize = FLASH_SIZE_1G;
	} else if ((ReadBuffer[2U] == FLASH_SIZE_ID_2G) ||
		(ReadBuffer[2U] == MACRONIX_FLASH_SIZE_ID_2G) ||
		(ReadBuffer[2U] == MACRONIX_FLASH_1_8_V_SIZE_ID_2G)) {
		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				FlashCfg.SectSize = SECTOR_SIZE_64K;
				FlashCfg.SectMask = 0xFFFF0000U;
				FlashCfg.NumDie = 4U;
				break;

			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				FlashCfg.SectSize = SECTOR_SIZE_128K;
				FlashCfg.SectMask = 0xFFFE0000U;
				FlashCfg.NumDie = 4U;
				break;

			default:
				Status = XBIR_ERROR_UNSUPPORTED_QSPI_CONN_MODE;
				break;
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
		FlashCfg.QspiFlashSize = FLASH_SIZE_2G;
	} else {
		Status = XBIR_ERROR_UNSUPPORTED_QSPI_CONN_MODE;
		goto END;
	}

	if ((FlashCfg.NumDie > 1U) &&
			(FlashCfg.QspiFlashMake == MICRON_ID)) {
		StatusCmd = READ_FLAG_STATUS_CMD;
		FsrFlag = 1U;
	} else {
		StatusCmd = READ_STATUS_CMD;
		FsrFlag = 0U;
	}

	xil_printf("[Flash Image Info]\r\n");
	xil_printf("\t Flash size : %uMB\r\n",
		FlashCfg.QspiFlashSize / (1024U * 1024U));
	xil_printf("\tSector size : %uKB\r\n",
		FlashCfg.SectSize / 1024U);
	xil_printf("\tSector Mask : 0x%08X\r\n\r\n",
			FlashCfg.SectMask);

	Status = XST_SUCCESS;

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

	switch(Xbir_QspiPsuInstance.Config.ConnectionMode) {
		case XQSPIPSU_CONNECTION_MODE_SINGLE:
			XQspiPsu_SelectFlash(&Xbir_QspiPsuInstance,
					XQSPIPSU_SELECT_FLASH_CS_LOWER,
					XQSPIPSU_SELECT_FLASH_BUS_LOWER);
			RealAddr = Address;
			break;

		case XQSPIPSU_CONNECTION_MODE_STACKED:
			/* Select lower or upper Flash based on sector address
			 */
			if(Address >= (FlashCfg.QspiFlashSize / 2U)) {
				XQspiPsu_SelectFlash(&Xbir_QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_UPPER,
						XQSPIPSU_SELECT_FLASH_BUS_LOWER);
				/* Subtract first flash size when accessing
				 *  second flash
				 */
				RealAddr = Address - (FlashCfg.QspiFlashSize / 2U);
			}else{
				/* Set selection to L_PAGE */
				XQspiPsu_SelectFlash(&Xbir_QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_LOWER,
						XQSPIPSU_SELECT_FLASH_BUS_LOWER);
				RealAddr = Address;
			}
			break;

		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
			/* The effective address in each flash is the actual
			 * address / 2
			 */
			XQspiPsu_SelectFlash(&Xbir_QspiPsuInstance,
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
	u32 QspiMode;

	/* Initialize the QSPI driver so that it's ready to use */
	QspiConfig =  XQspiPsu_LookupConfig(XBIR_QSPI_DEVICE_ID);
	if (NULL == QspiConfig) {
		Status = XBIR_QSPI_CONFIG_FAILED;
		goto END;
	}

	Status =  XQspiPsu_CfgInitialize(&Xbir_QspiPsuInstance, QspiConfig,
			QspiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XBIR_QSPI_CONFIG_INIT_FAILED;
		goto END;
	}

	/* Set Manual Start */
	Status = XQspiPsu_SetOptions(&Xbir_QspiPsuInstance,
		XQSPIPSU_MANUAL_START_OPTION);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_QSPI_MANUAL_START;
		goto END;
	}

	/* Set the pre-scaler for QSPI clock */
	Status = XQspiPsu_SetClkPrescaler(&Xbir_QspiPsuInstance,
		XQSPIPSU_CLK_PRESCALE_8);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_QSPI_PRESCALER_CLK;
		goto END;
	}

	XQspiPsu_SelectFlash(&Xbir_QspiPsuInstance,
		XQSPIPSU_SELECT_FLASH_CS_LOWER,	XQSPIPSU_SELECT_FLASH_BUS_LOWER);

	/* Configure the qspi in linear mode if running in XIP
	 * TBD
	 */
	switch ((u32)XPAR_PSU_QSPI_0_QSPI_MODE) {
		case XQSPIPSU_CONNECTION_MODE_SINGLE:
			break;

		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
			break;

		case XQSPIPSU_CONNECTION_MODE_STACKED:
			break;

		default:
			Status = XBIR_ERROR_INVALID_QSPI_CONNECTION;
			break;
	}

	if (Status != XST_SUCCESS) {
		goto END;
	}

	switch (Xbir_QspiPsuInstance.Config.BusWidth) {
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
			Status = XBIR_ERROR_INVALID_QSPI_CONNECTION;
			break;
	}

	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Read Flash ID and extract Manufacture and Size information */
	Status = Xbir_QspiFlashReadID(&Xbir_QspiPsuInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (MacronixFlash == 1U) {
		if (Xbir_QspiPsuInstance.Config.BusWidth == XBIR_QSPI_BUSWIDTH_FOUR) {
			RdCmd = QUAD_READ_CMD_24BIT2;
		}

		if (Xbir_QspiPsuInstance.Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL) {
			XQspiPsu_SelectFlash(&Xbir_QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_BOTH,
						XQSPIPSU_SELECT_FLASH_BUS_BOTH);
			Status = Xbir_QspiMacronixEnable4B(&Xbir_QspiPsuInstance);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		} else {
			XQspiPsu_SelectFlash(&Xbir_QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_LOWER,
						XQSPIPSU_SELECT_FLASH_BUS_LOWER);
			Status = Xbir_QspiMacronixEnable4B(&Xbir_QspiPsuInstance);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			if (Xbir_QspiPsuInstance.Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_STACKED) {
				XQspiPsu_SelectFlash(&Xbir_QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_UPPER,
						XQSPIPSU_SELECT_FLASH_BUS_LOWER);
				Status = Xbir_QspiMacronixEnable4B(&Xbir_QspiPsuInstance);
				if (Status != XST_SUCCESS) {
					goto END;
				}
			}
		}
	}

	/* Add code: For a Stacked connection, read second Flash ID */
	QspiMode = Xbir_QspiPsuInstance.Config.ConnectionMode;
	if ((QspiMode == (u32)XQSPIPSU_CONNECTION_MODE_PARALLEL) ||
		(QspiMode == (u32)XQSPIPSU_CONNECTION_MODE_STACKED)) {
		FlashCfg.QspiFlashSize = 2U * FlashCfg.QspiFlashSize;
	}

END:
	return Status;
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

	Status = XQspiPsu_PolledTransfer(&Xbir_QspiPsuInstance, &FlashMsg, 1U);
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

	Status = XQspiPsu_PolledTransfer(&Xbir_QspiPsuInstance, &FlashMsg, 1U);
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
	XQspiPsu_Msg FlashMsg[5U];
	u8 WriteBuf[5U] __attribute__ ((aligned(32U))) = {0U};

	/* Check the read length with Qspi flash size */
	if ((SrcAddress + Length) > FlashCfg.QspiFlashSize) {
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
				(Xbir_QspiPsuInstance.Config.BusWidth == XBIR_QSPI_BUSWIDTH_FOUR)) {
			/* Enable QPI mode */
			Status = Xbir_QspiMacronixEnableQPIMode(&Xbir_QspiPsuInstance, ENABLE_QPI);
			if (Status != XST_SUCCESS) {
				Status = XBIR_QSPI_4BYTE_ENETER_ERROR;
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
			if(Xbir_QspiPsuInstance.Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL){
				FlashMsg[3U].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
			}

			Status = XQspiPsu_PolledTransfer(&Xbir_QspiPsuInstance, &FlashMsg[0U], 4U);
			if (Status != XST_SUCCESS) {
				Status = XBIR_ERROR_QSPI_READ;
				goto END;
			}

			/* Disable QPI mode */
			Status = Xbir_QspiMacronixEnableQPIMode(&Xbir_QspiPsuInstance, DISABLE_QPI);
			if (Status != XST_SUCCESS) {
				Status = XBIR_QSPI_4BYTE_ENETER_ERROR;
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
			FlashMsg[2U].RxBfrPtr = (u8 *)DestAddress;
			FlashMsg[2U].ByteCount = TransferBytes;
			FlashMsg[2U].Flags = XQSPIPSU_MSG_FLAG_RX;

			if (Xbir_QspiPsuInstance.Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL){
				FlashMsg[2U].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
			}

			/*
			 * Send the read command to the Flash to read the
			 * specified number of bytes from the Flash, send the
			 * read command and address and receive the specified
			 * number of bytes of data in the data buffer
			 */
			Status = XQspiPsu_PolledTransfer(&Xbir_QspiPsuInstance,
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
	u8 FlashStatus[2U];
	u8 WrCmd[5U];
	u32 RealAddr;
	u32 CmdByteCount;
	XQspiPsu_Msg FlashMsg[5U];

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

	Status = XQspiPsu_PolledTransfer(&Xbir_QspiPsuInstance, FlashMsg, 1);
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

	if (Xbir_QspiPsuInstance.Config.ConnectionMode ==
			XQSPIPSU_CONNECTION_MODE_PARALLEL) {
		FlashMsg[1U].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
	}

	Status = XQspiPsu_PolledTransfer(&Xbir_QspiPsuInstance, FlashMsg, 2U);
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
		if (Xbir_QspiPsuInstance.Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL) {
			FlashMsg[1U].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
		}

		Status = XQspiPsu_PolledTransfer(&Xbir_QspiPsuInstance, FlashMsg, 2U);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		if (Xbir_QspiPsuInstance.Config.ConnectionMode ==
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
	u8 FlashStatus[2U];
	int Sector;
	u32 RealAddr;
	u32 NumSect;
	u8 WrBuffer[8U];
	u32 SectorOffset;
	XQspiPsu_Msg FlashMsg[5U];

	WrEnableCmd = WRITE_ENABLE_CMD;
	/* If the erase size is less than the total size of the flash, use
	 * sector erase command
	 *
	 * Calculate no. of sectors to erase based on byte count
	 */
	SectorOffset = (Address % FlashCfg.SectSize);
	Length += SectorOffset;
	Address -= SectorOffset;
	NumSect = (Length / (FlashCfg.SectSize));
	if ((Length % FlashCfg.SectSize) != 0U) {
		NumSect++;
	}

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
		Status = XQspiPsu_PolledTransfer(&Xbir_QspiPsuInstance, FlashMsg, 1U);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		WrBuffer[COMMAND_OFFSET]   = SPINOR_OP_BE_4K_4B;
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

		Status = XQspiPsu_PolledTransfer(&Xbir_QspiPsuInstance, FlashMsg, 1U);
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
			if (Xbir_QspiPsuInstance.Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL) {
				FlashMsg[1U].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
			}

			Status = XQspiPsu_PolledTransfer(&Xbir_QspiPsuInstance,
					FlashMsg, 2U);
			if (Status != XST_SUCCESS) {
				goto END;
			}

			if (Xbir_QspiPsuInstance.Config.ConnectionMode ==
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
		Address += FlashCfg.SectSize;
	}

END:
	return Status;
}