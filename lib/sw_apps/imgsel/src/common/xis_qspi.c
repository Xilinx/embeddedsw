/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_qspi.c
*
* This is the main file which will contain the qspi initialization,
* Erase,read and write fucntions
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  		Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  Ana	   18/06/20 	First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_main.h"

#ifdef XIS_UPDATE_A_B_MECHANISM
/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define QSPI_DEVICE_ID		XPAR_XQSPIPSU_0_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XIs_FlashReadID(XQspiPsu *QspiPsuPtr);
static int XIs_MacronixEnable4B(XQspiPsu *QspiPsuPtr);
static int XIs_MacronixEnableQPIMode(XQspiPsu *QspiPsuPtr, int Enable);
static int XIs_QspiFlashErase(u32 Address, u32 ByteCount);
static int XIs_QspiWriteData(u32 Address, u8 *WriteBfrPtr, u32 ByteCount);

/************************** Variable Definitions *****************************/
static XQspiPsu QspiPsuInstance;
static u32 ReadCommand = 0U;
static u32 StatusCmd = 0U;
static XQspiPsu_Msg FlashMsg[5U];
static FlashInfo Flash_Config;
static u8 FSRFlag;
static u8 CmdBfr[8U];
static u8 TxBfrPtr __attribute__ ((aligned(32U)));
static u8 ReadDataBuffer[10U] __attribute__ ((aligned(32U)));
static u8 WriteDataBuffer[10U] __attribute__ ((aligned(32U)));
static u32 MacronixFlash = 0U;
static u32 PageSize = 0U;

/************************** Function Definitions *****************************/
/******************************************************************************
 *
 * This function reads serial FLASH ID connected to the SPI interface.
 * It then deduces the make and size of the flash and obtains the
 * connection mode to point to corresponding parameters in the flash
 * configuration table. The flash driver will function based on this and
 * it presently supports Micron and Spansion - 128, 256 and 512Mbit and
 * Winbond 128Mbit
 *
 * @param	QspiPsuPtr Pointer to QSPI instance.
 *
 * @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
 *
 ******************************************************************************/
static int XIs_FlashReadID(XQspiPsu *QspiPsuPtr)
{
	int Status = XST_FAILURE;

	/*
	 * Read ID
	 */
	TxBfrPtr = READ_ID_CMD;
	FlashMsg[0U].TxBfrPtr = &TxBfrPtr;
	FlashMsg[0U].RxBfrPtr = NULL;
	FlashMsg[0U].ByteCount = 1U;
	FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1U].TxBfrPtr = NULL;
	FlashMsg[1U].RxBfrPtr = ReadDataBuffer;
	FlashMsg[1U].ByteCount = 4U;
	FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1U].Flags = XQSPIPSU_MSG_FLAG_RX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, &FlashMsg[0U], 2U);
	if (Status != XIS_SUCCESS) {
		Status = XST_FAILURE;
		goto END;
	}

	XIs_Printf(DEBUG_GENERAL, "FlashID=0x%x 0x%x 0x%x\n\r",
			ReadDataBuffer[0U], ReadDataBuffer[1U], ReadDataBuffer[2U]);

	/*
	 * Deduce flash make
	 */
	MacronixFlash = 0U;
	if (ReadDataBuffer[0U] == MICRON_ID) {
		Flash_Config.QspiFlashMake = MICRON_ID;
		XIs_Printf(DEBUG_INFO, "MICRON ");
	} else if(ReadDataBuffer[0U] == SPANSION_ID) {
		Flash_Config.QspiFlashMake = SPANSION_ID;
		XIs_Printf(DEBUG_INFO, "SPANSION ");
	} else if(ReadDataBuffer[0U] == WINBOND_ID) {
		Flash_Config.QspiFlashMake = WINBOND_ID;
		XIs_Printf(DEBUG_INFO, "WINBOND ");
	} else if(ReadDataBuffer[0U] == MACRONIX_ID) {
		Flash_Config.QspiFlashMake = MACRONIX_ID;
		XIs_Printf(DEBUG_INFO, "MACRONIX ");
		MacronixFlash = 1U;
	} else if(ReadDataBuffer[0U] == ISSI_ID) {
		Flash_Config.QspiFlashMake = ISSI_ID;
		XIs_Printf(DEBUG_INFO, "ISSI\r\n");
	} else {
		Status = XIS_UNSUPPORTED_QSPI_ERROR;
		XIs_Printf(DEBUG_GENERAL, "XIS_UNSUPPORTED_QSPI_ERROR\r\n");
		goto END;
	}

	/*
	 * Deduce flash Size
	 */
	if (ReadDataBuffer[2U] == FLASH_SIZE_ID_8M) {
		XIs_Printf(DEBUG_INFO, "8M Bits\r\n");
		Flash_Config.QspiFlashSize = FLASH_SIZE_8M;

		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				Flash_Config.SectSize = SECTOR_SIZE_64K;
				Flash_Config.SectMask = FLASH_SINGLE_STACK_MODE_MASK;
				Flash_Config.NumDie = 1U;
				Flash_Config.PageSize = PAGE_SIZE_256;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				Flash_Config.SectSize = SECTOR_SIZE_128K;
				Flash_Config.SectMask = FLASH_PARALLEL_MODE_MASK;
				Flash_Config.NumDie = 1U;
				Flash_Config.PageSize = PAGE_SIZE_512;
				break;

			default:
				Status = XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR;
				XIs_Printf(DEBUG_GENERAL,
						"XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR\r\n");
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
	} else if (ReadDataBuffer[2U] == FLASH_SIZE_ID_16M) {
		Flash_Config.QspiFlashSize = FLASH_SIZE_16M;
		XIs_Printf(DEBUG_INFO, "16M Bits\r\n");

		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				Flash_Config.SectSize = SECTOR_SIZE_64K;
				Flash_Config.SectMask = FLASH_SINGLE_STACK_MODE_MASK;
				Flash_Config.NumDie = 1U;
				Flash_Config.PageSize = PAGE_SIZE_256;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				Flash_Config.SectSize = SECTOR_SIZE_128K;
				Flash_Config.SectMask = FLASH_PARALLEL_MODE_MASK;
				Flash_Config.NumDie = 1U;
				Flash_Config.PageSize = PAGE_SIZE_512;
				break;

			default:
				Status = XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR;
				XIs_Printf(DEBUG_GENERAL,
						"XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR\r\n");
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
	} else if (ReadDataBuffer[2U] == FLASH_SIZE_ID_32M) {
		Flash_Config.QspiFlashSize = FLASH_SIZE_32M;
		XIs_Printf(DEBUG_INFO, "32M Bits\r\n");

		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				Flash_Config.SectSize = SECTOR_SIZE_64K;
				Flash_Config.SectMask = FLASH_SINGLE_STACK_MODE_MASK;
				Flash_Config.NumDie = 1U;
				Flash_Config.PageSize = PAGE_SIZE_256;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				Flash_Config.SectSize = SECTOR_SIZE_128K;
				Flash_Config.SectMask = FLASH_PARALLEL_MODE_MASK;
				Flash_Config.NumDie = 1U;
				Flash_Config.PageSize = PAGE_SIZE_512;
				break;

			default:
				Status = XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR;
				XIs_Printf(DEBUG_GENERAL,
						"XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR\r\n");
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
	} else if (ReadDataBuffer[2U] == FLASH_SIZE_ID_64M) {
		Flash_Config.QspiFlashSize = FLASH_SIZE_64M;
		XIs_Printf(DEBUG_INFO, "64M Bits\r\n");

		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				Flash_Config.SectSize = SECTOR_SIZE_64K;
				Flash_Config.SectMask = FLASH_SINGLE_STACK_MODE_MASK;
				Flash_Config.NumDie = 1U;
				if(Flash_Config.QspiFlashMake == SPANSION_ID) {
					Flash_Config.PageSize = PAGE_SIZE_512;
				} else {
					Flash_Config.PageSize = PAGE_SIZE_256;
				}
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				Flash_Config.SectSize = SECTOR_SIZE_128K;
				Flash_Config.SectMask = FLASH_PARALLEL_MODE_MASK;
				Flash_Config.NumDie = 1U;
				if(Flash_Config.QspiFlashMake == SPANSION_ID) {
					Flash_Config.PageSize = PAGE_SIZE_1024;
				} else {
					Flash_Config.PageSize = PAGE_SIZE_512;
				}
				break;

			default:
				Status = XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR;
				XIs_Printf(DEBUG_GENERAL,
						"XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR\r\n");
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
	} else if (ReadDataBuffer[2U] == FLASH_SIZE_ID_128M) {
		Flash_Config.QspiFlashSize = FLASH_SIZE_128M;
		XIs_Printf(DEBUG_INFO, "128M Bits\r\n");

		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				Flash_Config.SectSize = SECTOR_SIZE_64K;
				Flash_Config.SectMask = FLASH_SINGLE_STACK_MODE_MASK;
				Flash_Config.NumDie = 1U;
				Flash_Config.PageSize = PAGE_SIZE_256;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				Flash_Config.SectSize = SECTOR_SIZE_128K;
				Flash_Config.SectMask = FLASH_PARALLEL_MODE_MASK;
				Flash_Config.NumDie = 1U;
				Flash_Config.PageSize = PAGE_SIZE_512;
				break;

			default:
				Status = XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR;
				XIs_Printf(DEBUG_GENERAL,
						"XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR\r\n");
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
	} else if ((ReadDataBuffer[2U] == FLASH_SIZE_ID_256M)
			|| (ReadDataBuffer[2U] == MACRONIX_FLASH_1_8_V_MX25_ID_256)) {
		Flash_Config.QspiFlashSize = FLASH_SIZE_256M;
		XIs_Printf(DEBUG_INFO, "256M Bits\r\n");

		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				Flash_Config.SectSize = SECTOR_SIZE_64K;
				Flash_Config.SectMask = FLASH_SINGLE_STACK_MODE_MASK;
				Flash_Config.NumDie = 1U;
				Flash_Config.PageSize = PAGE_SIZE_256;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				Flash_Config.SectSize = SECTOR_SIZE_128K;
				if(Flash_Config.QspiFlashMake == ISSI_ID) {
					Flash_Config.SectMask = FLASH_SINGLE_STACK_MODE_MASK;
				}
				else {
					Flash_Config.SectMask = FLASH_PARALLEL_MODE_MASK;
				}
				Flash_Config.NumDie = 1U;
				Flash_Config.PageSize = PAGE_SIZE_512;
				break;

			default:
				Status = XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR;
				XIs_Printf(DEBUG_GENERAL,
						"XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR\r\n");
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
	} else if ((ReadDataBuffer[2U] == FLASH_SIZE_ID_512M)
			|| (ReadDataBuffer[2U] == MACRONIX_FLASH_SIZE_ID_512M)
			|| (ReadDataBuffer[2U] == MACRONIX_FLASH_1_8_V_MX66_ID_512)) {
		Flash_Config.QspiFlashSize = FLASH_SIZE_512M;
		XIs_Printf(DEBUG_INFO, "512M Bits\r\n");

		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				if(Flash_Config.QspiFlashMake == SPANSION_ID) {
					Flash_Config.SectSize = SECTOR_SIZE_256K;
					Flash_Config.SectMask = FLASH_SPANSION_SINGLE_MODE_MASK;
					Flash_Config.NumDie = 1U;
					Flash_Config.PageSize = PAGE_SIZE_512;
				}
				else {
					Flash_Config.SectSize = SECTOR_SIZE_64K;
					Flash_Config.SectMask = FLASH_SINGLE_STACK_MODE_MASK;
					Flash_Config.NumDie = 2U;
					Flash_Config.PageSize = PAGE_SIZE_256;
				}
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				if(Flash_Config.QspiFlashMake == SPANSION_ID) {
					Flash_Config.SectSize = SECTOR_SIZE_512K;
					Flash_Config.SectMask = FALSH_SPANSION_PARALLEL_MODE_MASK;
					Flash_Config.NumDie = 1U;
					Flash_Config.PageSize = PAGE_SIZE_1024;
				}
				else {
					Flash_Config.SectSize = SECTOR_SIZE_128K;
					Flash_Config.SectMask = FLASH_PARALLEL_MODE_MASK;
					Flash_Config.NumDie = 2U;
					Flash_Config.PageSize = PAGE_SIZE_512;
				}
				break;

			default:
				Status = XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR;
				XIs_Printf(DEBUG_GENERAL,
						"XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR\r\n");
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
	} else if ((ReadDataBuffer[2U] == FLASH_SIZE_ID_1G)
			|| (ReadDataBuffer[2U] == MACRONIX_FLASH_SIZE_ID_1G)
			|| (ReadDataBuffer[2U] == MACRONIX_FLASH_1_8_V_SIZE_ID_1G)) {
		Flash_Config.QspiFlashSize = FLASH_SIZE_1G;
		XIs_Printf(DEBUG_INFO, "1G Bits\r\n");

		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				Flash_Config.SectSize = SECTOR_SIZE_64K;
				Flash_Config.SectMask = FLASH_SINGLE_STACK_MODE_MASK;
				Flash_Config.NumDie = 4U;
				Flash_Config.PageSize = PAGE_SIZE_256;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				Flash_Config.SectSize = SECTOR_SIZE_128K;
				Flash_Config.SectMask = FLASH_PARALLEL_MODE_MASK;
				Flash_Config.NumDie = 4U;
				Flash_Config.PageSize = PAGE_SIZE_512;
				break;

			default:
				Status = XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR;
				XIs_Printf(DEBUG_GENERAL,
						"XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR\r\n");
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
	} else if ((ReadDataBuffer[2U] == FLASH_SIZE_ID_2G)
			|| (ReadDataBuffer[2U] == MACRONIX_FLASH_SIZE_ID_2G)
			|| (ReadDataBuffer[2U] == MACRONIX_FLASH_1_8_V_SIZE_ID_2G)) {
		Flash_Config.QspiFlashSize = FLASH_SIZE_2G;
		XIs_Printf(DEBUG_INFO, "2G Bits\r\n");

		switch(QspiPsuPtr->Config.ConnectionMode) {
			case XQSPIPSU_CONNECTION_MODE_SINGLE:
			case XQSPIPSU_CONNECTION_MODE_STACKED:
				Flash_Config.SectSize = SECTOR_SIZE_64K;
				Flash_Config.SectMask = FLASH_SINGLE_STACK_MODE_MASK;
				Flash_Config.NumDie = 4U;
				Flash_Config.PageSize = PAGE_SIZE_256;
				break;
			case XQSPIPSU_CONNECTION_MODE_PARALLEL:
				Flash_Config.SectSize = SECTOR_SIZE_128K;
				Flash_Config.SectMask = FLASH_PARALLEL_MODE_MASK;
				Flash_Config.NumDie = 4U;
				Flash_Config.PageSize = PAGE_SIZE_512;
				break;

			default:
				Status = XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR;
				XIs_Printf(DEBUG_GENERAL,
						"XIS_UNSUPPORTED_QSPI_CONN_MODE_ERROR\r\n");
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}else {
		Status = XIS_UNSUPPORTED_QSPI_ERROR;
		XIs_Printf(DEBUG_GENERAL,"XIS_UNSUPPORTED_QSPI_ERROR\r\n");
		goto END;
	}
	if ((Flash_Config.NumDie > 1U) &&
			(Flash_Config.QspiFlashMake == MICRON_ID)) {
		StatusCmd = READ_FLAG_STATUS_CMD;
		FSRFlag = 1U;
	} else {
		StatusCmd = READ_STATUS_CMD;
		FSRFlag = 0U;
	}

END:
	return Status;
}

/******************************************************************************
 *
 * This functions translates the address based on the type of interconnection.
 * In case of stacked, this function asserts the corresponding slave select.
 *
 * @param	Address which is to be accessed
 *
 * @return	QspiAddr is the actual flash address to be accessed - for single
 * 			it is unchanged; for stacked, the lower flash size is subtracted;
 * 			for parallel the address is divided by 2.
 *
 ******************************************************************************/
static u32 XIs_GetQspiAddr(u32 Address)
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
			/*
			 * Select lower or upper Flash based on sector address
			 */
			if(Address >= (Flash_Config.QspiFlashSize / 2U)) {
				XQspiPsu_SelectFlash(&QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_UPPER,
						XQSPIPSU_SELECT_FLASH_BUS_LOWER);
				/*
				 * Subtract first flash size when accessing second flash
				 */
				RealAddr = Address - (Flash_Config.QspiFlashSize / 2U);
			}else{
				/*
				 * Set selection to L_PAGE
				 */
				XQspiPsu_SelectFlash(&QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_LOWER,
						XQSPIPSU_SELECT_FLASH_BUS_LOWER);
				RealAddr = Address;
			}
			break;
		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
			/*
			 * The effective address in each flash is the actual
			 * address / 2
			 */
			XQspiPsu_SelectFlash(&QspiPsuInstance,
					XQSPIPSU_SELECT_FLASH_CS_BOTH, XQSPIPSU_SELECT_FLASH_BUS_BOTH);
			RealAddr = Address / 2U;
			break;

		default:
			/*
			 * We should never reach here as error will be triggered during
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
 * This function is used to initialize the qspi controller and driver
 *
 * @param	PageSize which is used to write data to flash
 *
 * @return	None
 *
 *****************************************************************************/
int XIs_QspiInit(void)
{
	int Status = XST_FAILURE;
	XQspiPsu_Config *QspiConfig;
	u32 QspiMode;

	/**
	 * Initialize the QSPI driver so that it's ready to use
	 */
	QspiConfig = XQspiPsu_LookupConfig(QSPI_DEVICE_ID);
	if (NULL == QspiConfig) {
		Status = XIS_QSPI_CONFIG_ERROR;
		XIs_Printf(DEBUG_GENERAL, "XIS_QSPI_CONFIG_ERROR\r\n");
		goto END;
	}

	Status = XQspiPsu_CfgInitialize(&QspiPsuInstance, QspiConfig,
			QspiConfig->BaseAddress);
	if (Status != XIS_SUCCESS) {
		Status = XIS_QSPI_CONFIG_INIT_ERROR;
		XIs_Printf(DEBUG_GENERAL, "XIS_QSPI_CONFIG_INIT_ERROR\r\n");
		goto END;
	}

	/*
	 * Set Manual Start
	 */
	Status = XQspiPsu_SetOptions(&QspiPsuInstance, XQSPIPSU_MANUAL_START_OPTION);
	if (Status != XIS_SUCCESS) {
		Status = XIS_QSPI_MANUAL_START_ERROR;
		XIs_Printf(DEBUG_GENERAL, "XIS_QSPI_MANUAL_START_ERROR\r\n");
		goto END;
	}
	/*
	 * Set the pre-scaler for QSPI clock
	 */
	Status = XQspiPsu_SetClkPrescaler(&QspiPsuInstance, XQSPIPSU_CLK_PRESCALE_8);
	if (Status != XIS_SUCCESS) {
		Status = XIS_QSPI_PRESCALER_CLK_ERROR;
		XIs_Printf(DEBUG_GENERAL, "XIS_QSPI_PRESCALER_CLK_ERROR\r\n");
		goto END;
	}
	XQspiPsu_SelectFlash(&QspiPsuInstance,
			XQSPIPSU_SELECT_FLASH_CS_LOWER, XQSPIPSU_SELECT_FLASH_BUS_LOWER);

	/*
	 * Configure the qspi in linear mode if running in XIP
	 * TBD
	 */
	switch ((u32)XPAR_PSU_QSPI_0_QSPI_MODE) {
		case XQSPIPSU_CONNECTION_MODE_SINGLE:
			XIs_Printf(DEBUG_INFO, "QSPI is in single flash connection\r\n");
			break;
		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
			XIs_Printf(DEBUG_INFO, "QSPI is in Dual Parallel connection\r\n");
			break;
		case XQSPIPSU_CONNECTION_MODE_STACKED:
			XIs_Printf(DEBUG_INFO, "QSPI is in Dual Stack connection\r\n");
			break;

		default:
			Status = XIS_INVALID_QSPI_CONNECTION_ERROR;
			XIs_Printf(DEBUG_GENERAL,
					"XIS_INVALID_QSPI_CONNECTION_ERROR\r\n");
			break;
	}
	if(Status != XIS_SUCCESS) {
		goto END;
	}

	switch (QspiPsuInstance.Config.BusWidth) {
		case XIS_QSPI_BUSWIDTH_ONE:
			XIs_Printf(DEBUG_INFO, "QSPI is using 1 bit bus\r\n");
			ReadCommand = FAST_READ_CMD_32BIT;
			break;
		case XIS_QSPI_BUSWIDTH_TWO:
			XIs_Printf(DEBUG_INFO, "QSPI is using 2 bit bus\r\n");
			ReadCommand = DUAL_READ_CMD_32BIT;
			break;
		case XIS_QSPI_BUSWIDTH_FOUR:
			XIs_Printf(DEBUG_INFO, "QSPI is using 4 bit bus\r\n");
			ReadCommand = QUAD_READ_CMD_32BIT;
			break;

		default:
			Status = XIS_INVALID_QSPI_CONNECTION_ERROR;
			XIs_Printf(DEBUG_GENERAL,
					"XIS_INVALID_QSPI_CONNECTION_ERROR\r\n");
			break;
	}
	if(Status != XIS_SUCCESS) {
		goto END;
	}

	/**
	 * Read Flash ID and extract Manufacture and Size information
	 */
	Status = XIs_FlashReadID(&QspiPsuInstance);
	if (Status != XIS_SUCCESS) {
		goto END;
	}

	if (MacronixFlash == 1U) {
		if (QspiPsuInstance.Config.BusWidth == XIS_QSPI_BUSWIDTH_FOUR) {
			ReadCommand = QUAD_READ_CMD_24BIT2;
		}

		if (QspiPsuInstance.Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL) {
			XQspiPsu_SelectFlash(&QspiPsuInstance,
					XQSPIPSU_SELECT_FLASH_CS_BOTH,
					XQSPIPSU_SELECT_FLASH_BUS_BOTH);
			Status = XIs_MacronixEnable4B(&QspiPsuInstance);
			if (Status != XIS_SUCCESS) {
				Status = XST_FAILURE;
				goto END;
			}
		} else {
			XQspiPsu_SelectFlash(&QspiPsuInstance,
					XQSPIPSU_SELECT_FLASH_CS_LOWER,
					XQSPIPSU_SELECT_FLASH_BUS_LOWER);
			Status = XIs_MacronixEnable4B(&QspiPsuInstance);
			if (Status != XIS_SUCCESS) {
				Status = XST_FAILURE;
				goto END;
			}
			if (QspiPsuInstance.Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_STACKED) {
				XQspiPsu_SelectFlash(&QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_UPPER,
						XQSPIPSU_SELECT_FLASH_BUS_LOWER);
				Status = XIs_MacronixEnable4B(&QspiPsuInstance);
				if (Status != XIS_SUCCESS) {
					Status = XST_FAILURE;
					goto END;
				}
			}
		}
	}
	/**
	 * Add code: For a Stacked connection, read second Flash ID
	 */
	QspiMode = QspiPsuInstance.Config.ConnectionMode;
	if ((QspiMode ==
				(u32)(XQSPIPSU_CONNECTION_MODE_PARALLEL)) ||
			(QspiMode ==
			 (u32)(XQSPIPSU_CONNECTION_MODE_STACKED)) ) {
		Flash_Config.QspiFlashSize = 2U * Flash_Config.QspiFlashSize;
	}
	PageSize = Flash_Config.PageSize;

END:
	return Status;
}

/******************************************************************************
 *
 * Static API used for Macronix flash to enable 4BYTE mode
 *
 * @param	QspiPsuPtr Pointer to QSPI instance.
 *
 * @return	XIS_SUCCESS if success, otherwise XST_FAILURE.
 *
 ******************************************************************************/
static int XIs_MacronixEnable4B(XQspiPsu *QspiPsuPtr)
{
	int Status = XST_FAILURE;

	XIs_Printf(DEBUG_GENERAL, "MACRONIX_FLASH_MODE\r\n");

	/*Enable register write*/
	TxBfrPtr = WRITE_ENABLE_CMD;
	FlashMsg[0U].TxBfrPtr = &TxBfrPtr;
	FlashMsg[0U].RxBfrPtr = NULL;
	FlashMsg[0U].ByteCount = 1U;
	FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, &FlashMsg[0U], 1U);
	if (Status != XIS_SUCCESS) {
		goto END;
	}

	/*Enable 4 byte mode*/
	TxBfrPtr = 0xB7U;
	FlashMsg[0U].TxBfrPtr = &TxBfrPtr;
	FlashMsg[0U].RxBfrPtr = NULL;
	FlashMsg[0U].ByteCount = 1U;
	FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, &FlashMsg[0U], 1U);
	if (Status != XIS_SUCCESS) {
		goto END;
	}

	XIs_Printf(DEBUG_GENERAL, "MACRONIX_ENABLE_4BYTE_DONE\r\n");

END:
	return Status;
}

/******************************************************************************
 *
 * Static API used for Macronix flash to enable or disable QPI mode
 *
 * @param	QspiPsuPtr Pointer to QSPI instance.
 * @param    Enable valid values are 0 (disable) and 1 (enable).
 *
 * @return	XIS_SUCCESS if success, otherwise XIS_QSPI_READ_ERROR.
 *
 ******************************************************************************/
static int XIs_MacronixEnableQPIMode(XQspiPsu *QspiPsuPtr, int Enable)
{
	int Status = XST_FAILURE;

	/*Enable register write*/
	TxBfrPtr = WRITE_ENABLE_CMD;
	FlashMsg[0U].TxBfrPtr = &TxBfrPtr;
	FlashMsg[0U].RxBfrPtr = NULL;
	FlashMsg[0U].ByteCount = 1U;
	if (Enable == ENABLE_QPI) {
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	} else {
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
	}
	FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg[0U], 1U);
	if (Status != XIS_SUCCESS) {
		Status = XIS_QSPI_READ_ERROR;
		XIs_Printf(DEBUG_GENERAL, "XIS_QSPI_READ_ERROR\r\n");
		goto END;
	}

	if (Enable == ENABLE_QPI) {
		TxBfrPtr = 0x35U;
	} else {
		TxBfrPtr = 0xF5U;
	}
	FlashMsg[0U].TxBfrPtr = &TxBfrPtr;
	FlashMsg[0U].RxBfrPtr = NULL;
	FlashMsg[0U].ByteCount = 1U;
	if (Enable == ENABLE_QPI) {
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	} else {
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
	}
	FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg[0U], 1U);
	if (Status != XIS_SUCCESS) {
		Status = XIS_QSPI_READ_ERROR;
		XIs_Printf(DEBUG_GENERAL, "XIS_QSPI_READ_ERROR\r\n");
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to copy the data from QSPI flash to destination
 * address
 *
 * @param 	SrcAddress is the address of the QSPI flash where copy should
 * 			start from
 * @param 	DestAddress is the address of the destination where it
 * 			should copy to
 * @param	Length Length of the bytes to be copied
 *
 * @return	XIS_SUCCESS for successful copy
 * 			errors as mentioned in xis_error.h
 *
 *****************************************************************************/
int XIs_QspiRead(u32 SrcAddress, u8* DestAddress, u32 Length)
{
	int Status = XST_FAILURE;
	u32 QspiAddr;
	u32 RemainingBytes;
	u32 TransferBytes;
	u32 DiscardByteCnt;

	XIs_Printf(DEBUG_INFO, "QSPI Reading Src 0x%0lx, Dest %0lx,"
			"Length %0lx\r\n", SrcAddress, DestAddress, Length);

	/**
	 * Check the read length with Qspi flash size
	 */
	if ((SrcAddress + Length) > Flash_Config.QspiFlashSize) {
		Status = XIS_QSPI_LENGTH_ERROR;
		XIs_Printf(DEBUG_GENERAL, "XIS_QSPI_LENGTH_ERROR\r\n");
		goto END;
	}

	/**
	 * Update no of bytes to be copied
	 */
	RemainingBytes = Length;

	while(RemainingBytes > 0U) {

		if (RemainingBytes > DMA_DATA_TRAN_SIZE) {
			TransferBytes = DMA_DATA_TRAN_SIZE;
		} else {
			TransferBytes = RemainingBytes;
		}

		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		QspiAddr = XIs_GetQspiAddr((u32 )SrcAddress);

		XIs_Printf(DEBUG_INFO,".");
		XIs_Printf(DEBUG_DETAILED,
				"QSPI Read Src 0x%0lx, Dest %0lx, Length %0lx\r\n",
				QspiAddr, DestAddress, TransferBytes);

		/*
		 * Setup the read command with the specified address and data for the
		 * Flash
		 */
		if ((MacronixFlash == 1U) &&
				(QspiPsuInstance.Config.BusWidth == XIS_QSPI_BUSWIDTH_FOUR)) {
			/* Enable QPI mode */
			Status = XIs_MacronixEnableQPIMode(&QspiPsuInstance, ENABLE_QPI);
			if (Status != XIS_SUCCESS) {
				Status = XIS_QSPI_4BYTE_ENETER_ERROR;
				XIs_Printf(DEBUG_GENERAL, "XIS_QSPI_4BYTE_ENETER_ERROR\r\n");
				goto END;
			}

			/*Command*/
			WriteDataBuffer[COMMAND_OFFSET]	= (u8)ReadCommand;
			DiscardByteCnt = 1U;
			/*MACRONIX is in QPI MODE 4-4-4*/
			FlashMsg[0U].TxBfrPtr = WriteDataBuffer;
			FlashMsg[0U].RxBfrPtr = NULL;
			FlashMsg[0U].ByteCount = DiscardByteCnt;
			FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
			FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;
			/*Address*/
			WriteDataBuffer[ADDRESS_1_OFFSET] =
				(u8)((QspiAddr & 0xFF000000U) >> 24U);
			WriteDataBuffer[ADDRESS_2_OFFSET] =
				(u8)((QspiAddr & 0xFF0000U) >> 16U);
			WriteDataBuffer[ADDRESS_3_OFFSET] =
				(u8)((QspiAddr & 0xFF00U) >> 8U);
			WriteDataBuffer[ADDRESS_4_OFFSET] =
				(u8)(QspiAddr & 0xFFU);
			DiscardByteCnt = 4U;

			FlashMsg[1U].TxBfrPtr = &WriteDataBuffer[ADDRESS_1_OFFSET];
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
			if (Status != XIS_SUCCESS) {
				Status = XIS_QSPI_READ_ERROR;
				XIs_Printf(DEBUG_GENERAL,"XIS_QSPI_READ_ERROR\r\n");
				goto END;
			}

			/* Disable QPI mode */
			Status = XIs_MacronixEnableQPIMode(&QspiPsuInstance, DISABLE_QPI);
			if (Status != XIS_SUCCESS) {
				Status = XIS_QSPI_4BYTE_ENETER_ERROR;
				XIs_Printf(DEBUG_GENERAL, "XIS_QSPI_4BYTE_ENETER_ERROR\r\n");
				goto END;
			}
		} else {
			WriteDataBuffer[COMMAND_OFFSET]   = (u8)ReadCommand;
			WriteDataBuffer[ADDRESS_1_OFFSET] =
				(u8)((QspiAddr & 0xFF000000U) >> 24U);
			WriteDataBuffer[ADDRESS_2_OFFSET] =
				(u8)((QspiAddr & 0xFF0000U) >> 16U);
			WriteDataBuffer[ADDRESS_3_OFFSET] =
				(u8)((QspiAddr & 0xFF00U) >> 8U);
			WriteDataBuffer[ADDRESS_4_OFFSET] =
				(u8)(QspiAddr & 0xFFU);
			DiscardByteCnt = 5U;

			FlashMsg[0U].TxBfrPtr = WriteDataBuffer;
			FlashMsg[0U].RxBfrPtr = NULL;
			FlashMsg[0U].ByteCount = DiscardByteCnt;
			FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

			/*
			 * It is recommended to have a separate entry for dummy
			 */
			if ((ReadCommand == FAST_READ_CMD_32BIT) ||
					(ReadCommand == DUAL_READ_CMD_32BIT) ||
					(ReadCommand == QUAD_READ_CMD_32BIT)) {
				/*
				 * It is recommended that Bus width value during dummy
				 * phase should be same as data phase
				 */
				if (ReadCommand == FAST_READ_CMD_32BIT) {
					FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
				}

				if (ReadCommand == DUAL_READ_CMD_32BIT) {
					FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_DUALSPI;
				}

				if (ReadCommand == QUAD_READ_CMD_32BIT) {
					FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
				}

				FlashMsg[1U].TxBfrPtr = NULL;
				FlashMsg[1U].RxBfrPtr = NULL;
				FlashMsg[1U].ByteCount = DUMMY_CLOCKS;
				FlashMsg[1U].Flags = 0U;
			}

			if (ReadCommand == FAST_READ_CMD_32BIT) {
				FlashMsg[2U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			}

			if (ReadCommand == DUAL_READ_CMD_32BIT) {
				FlashMsg[2U].BusWidth = XQSPIPSU_SELECT_MODE_DUALSPI;
			}

			if (ReadCommand == QUAD_READ_CMD_32BIT) {
				FlashMsg[2U].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
			}

			FlashMsg[2U].TxBfrPtr = NULL;
			FlashMsg[2U].RxBfrPtr = (u8 *)DestAddress;
			FlashMsg[2U].ByteCount = TransferBytes;
			FlashMsg[2U].Flags = XQSPIPSU_MSG_FLAG_RX;

			if(QspiPsuInstance.Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL){
				FlashMsg[2U].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
			}

			/*
			 * Send the read command to the Flash to read the specified number
			 * of bytes from the Flash, send the read command and address and
			 * receive the specified number of bytes of data in the data buffer
			 */
			Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg[0U], 3U);
			if (Status != XIS_SUCCESS) {
				Status = XIS_QSPI_READ_ERROR;
				XIs_Printf(DEBUG_GENERAL, "XIS_QSPI_READ_ERROR\r\n");
				goto END;
			}
		}
		/*
		 * Update the variables
		 */
		RemainingBytes -= TransferBytes;
		DestAddress += TransferBytes;
		SrcAddress += TransferBytes;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function writes to the  serial Flash connected to the QSPIPSU interface.
 * All the data put into the buffer must be in the same page of the device with
 * page boundaries being on 256 byte boundaries.
 *
 * @param	Address contains the address to write data to in the Flash.
 * @param	Pointer to the write buffer (which is to be transmitted)
 * @param	ByteCount contains the number of bytes to write.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 ******************************************************************************/
static int XIs_QspiWriteData(u32 Address, u8 *WriteBfrPtr, u32 ByteCount)
{
	int Status = XST_FAILURE;
	u8 WriteEnableCmd;
	u8 ReadStatusCmd;
	u8 FlashStatus[2U];
	u8 WriteCmd[5U];
	u32 RealAddr;
	u32 CmdByteCount;

	WriteEnableCmd = WRITE_ENABLE_CMD;
	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	RealAddr = XIs_GetQspiAddr(Address);
	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	FlashMsg[0U].TxBfrPtr = &WriteEnableCmd;
	FlashMsg[0U].RxBfrPtr = NULL;
	FlashMsg[0U].ByteCount = 1U;
	FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

	Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, FlashMsg, 1);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	WriteCmd[COMMAND_OFFSET]   = WRITE_CMD;
	WriteCmd[ADDRESS_1_OFFSET] =
		(u8)((RealAddr & 0xFF000000U) >> 24U);
	WriteCmd[ADDRESS_2_OFFSET] =
		(u8)((RealAddr & 0xFF0000U) >> 16U);
	WriteCmd[ADDRESS_3_OFFSET] =
		(u8)((RealAddr & 0xFF00U) >> 8U);
	WriteCmd[ADDRESS_4_OFFSET] =
		(u8)(RealAddr & 0xFFU);
	CmdByteCount = 5U;

	FlashMsg[0U].TxBfrPtr = WriteCmd;
	FlashMsg[0U].RxBfrPtr = NULL;
	FlashMsg[0U].ByteCount = CmdByteCount;
	FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1U].TxBfrPtr = WriteBfrPtr;
	FlashMsg[1U].RxBfrPtr = NULL;
	FlashMsg[1U].ByteCount = ByteCount;
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
		ReadStatusCmd = StatusCmd;
		FlashMsg[0U].TxBfrPtr = &ReadStatusCmd;
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
			if (FSRFlag) {
				FlashStatus[1U] &= FlashStatus[0U];
			} else {
				FlashStatus[1U] |= FlashStatus[0U];
			}
		}

		if (FSRFlag != 0U) {
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

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function erases the sectors in the  serial Flash connected to the
 * QSPIPSU interface.
 *
 * @param	Address contains the address of the first sector which needs to
 *			be erased.
 * @param	ByteCount contains the total size to be erased.
 * @param	Pointer to the write buffer (which is to be transmitted)
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 *****************************************************************************/
static int XIs_QspiFlashErase(u32 Address, u32 ByteCount)
{
	int Status = XST_FAILURE;
	u8 WriteEnableCmd;
	u8 ReadStatusCmd;
	u8 FlashStatus[2U];
	int Sector;
	u32 RealAddr;
	u32 NumSect;
	u8 *WriteBfrPtr = CmdBfr;

	WriteEnableCmd = WRITE_ENABLE_CMD;
	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 *
	 * Calculate no. of sectors to erase based on byte count
	 */
	NumSect = (ByteCount / (Flash_Config.SectSize)) + 1U;
	/*
	 * If ByteCount to k sectors,
	 * but the address range spans from N to N+k+1 sectors, then
	 * increment no. of sectors to be erased
	 */
	if (((Address + ByteCount) & Flash_Config.SectMask) ==
			((Address + (NumSect * Flash_Config.SectSize)) &
			 Flash_Config.SectMask)) {
		NumSect++;
	}

	for (Sector = 0U; Sector < NumSect; Sector++) {
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = XIs_GetQspiAddr(Address);

		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate
		 * transfer before the write
		 */
		FlashMsg[0U].TxBfrPtr = &WriteEnableCmd;
		FlashMsg[0U].RxBfrPtr = NULL;
		FlashMsg[0U].ByteCount = 1U;
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;
		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, FlashMsg, 1U);
		if (Status != XST_SUCCESS) {
			Status = XIS_POLLED_TRANSFER_ERROR;
			goto END;
		}

		WriteBfrPtr[COMMAND_OFFSET]   = SPINOR_OP_BE_4K_4B;
		/*
		 * To be used only if 4B address sector erase cmd is
		 * supported by flash
		 */
		WriteBfrPtr[ADDRESS_1_OFFSET] =
			(u8)((RealAddr & 0xFF000000U) >> 24U);
		WriteBfrPtr[ADDRESS_2_OFFSET] =
			(u8)((RealAddr & 0xFF0000U) >> 16U);
		WriteBfrPtr[ADDRESS_3_OFFSET] =
			(u8)((RealAddr & 0xFF00U) >> 8U);
		WriteBfrPtr[ADDRESS_4_OFFSET] =
			(u8)(RealAddr & 0xFFU);

		FlashMsg[0U].ByteCount = 5U;
		FlashMsg[0U].TxBfrPtr = WriteBfrPtr;
		FlashMsg[0U].RxBfrPtr = NULL;
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, FlashMsg, 1U);
		if (Status != XST_SUCCESS) {
			Status = XIS_POLLED_TRANSFER_ERROR;
			goto END;
		}
		/*
		 * Wait for the erase command to be completed
		 */
		while (1U) {
			ReadStatusCmd = StatusCmd;
			FlashMsg[0U].TxBfrPtr = &ReadStatusCmd;
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
				Status = XIS_POLLED_TRANSFER_ERROR;
				goto END;
			}

			if (QspiPsuInstance.Config.ConnectionMode ==
					XQSPIPSU_CONNECTION_MODE_PARALLEL) {
				if (FSRFlag) {
					FlashStatus[1U] &= FlashStatus[0U];
				}
				else {
					FlashStatus[1U] |= FlashStatus[0U];
				}
			}

			if (FSRFlag != 0U) {
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
		Address += Flash_Config.SectSize;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * This function writes to the  serial Flash connected to the QSPIPSU interface.
 * All the data put into the buffer must be in the same page of the device with
 * page boundaries being on 256 byte boundaries.
 *
 * @param	Address contains the address to write data to in the Flash.
 * @param	WriteDataBuffer to the write buffer (which is to be transmitted)
 * @param	ByteCount contains the number of bytes to write.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 ******************************************************************************/
int XIs_QspiWrite(u32 Address, u8 *WriteDataBuffer, u32 ByteCount)
{
	int Status = XST_FAILURE;
	u32 PageIndex;
	u32 PageCount;

	PageCount = (ByteCount / PageSize);

	Status = XIs_QspiFlashErase(Address, ByteCount);
	if (Status != XST_SUCCESS) {
		XIs_Printf(DEBUG_GENERAL, "QSPI Erase Flash failed\r\n");
		goto END;
	}

	for(PageIndex = 0; PageIndex < PageCount; PageIndex++) {
		Status = XIs_QspiWriteData(Address,
						(u8 *)(WriteDataBuffer + (PageIndex * PageSize)),
										PageSize);
		if (Status != XST_SUCCESS) {
			XIs_Printf(DEBUG_GENERAL, "QSPI Write Failed\r\n");
			goto END;
		}
	}

END:
	return Status;
}
#endif
