/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_qspi.c
*
* This is the file which contains qspi related code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2018 Initial release
* 1.01  sb   08/23/2019 Added QSPI buswidth detection logic
*       har  08/28/2019 Fixed MISRA C violations
*       bsv  08/30/2019 Added fallback and multiboot support in PLM
*       bsv  09/12/2019 Added support for Macronix 1.8V flash parts
* 1.02  bsv  02/04/2020 Reset qspi instance in init functions for LPD off
*						suspend and resume to work
*       bsv  04/09/2020 Code clean up
* 1.03  bsv  07/03/2020 Added support for macronix part P/N:MX25U12835F
*       skd  07/14/2020 XLoader_QspiCopy prototype changed
*       skd  07/29/2020 Added non-blocking DMA support for Qspi copy
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       bm   09/01/2020 Updated Major error codes for Unsupported Qspi Flash
*			IDs and Unsupported Flash Sizes
*       bsv  10/13/2020 Code clean up
* 1.04  ma   03/24/2021 Minor updates to prints in XilLoader
* 1.05  bsv  07/22/2021 Added support for Winbond flash part
*       bsv  08/31/2021 Code clean up
* 1.06  ma   01/17/2022 Enable SLVERR for QSPI registers
* 1.07  bm   07/06/2022 Refactor versal and versal_net code
* 1.08  ng   11/11/2022 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*       ng   06/26/2023 Added support for system device-tree flow
*       ng   08/09/2023 Removed redundant windbond flash size macro
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xloader_qspi.h"
#include "xplmi_dma.h"
#include "xloader.h"
#include "xplmi.h"
#ifdef XLOADER_QSPI
#include "xqspipsu.h"
#include "xpm_api.h"
#include "xpm_nodeid.h"
#include "xloader_plat.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int FlashReadID(XQspiPsu *QspiPsuPtr);

/************************** Variable Definitions *****************************/
static XQspiPsu QspiPsuInstance;
static u32 QspiFlashSize = 0U;
static u32 QspiFlashMake = 0U;
static u32 ReadCommand = 0U;
static u8 QspiMode;
static PdiSrc_t QspiBootMode;
static u8 QspiBusWidth;

/*****************************************************************************/
/**
 * @brief	This function reads serial FLASH ID connected to the SPI
 * 			interface. It then deduces the make and size of the flash and
 * 			obtains the	connection mode to point to corresponding parameters
 * 			in the flash configuration table. The flash driver will function
 * 			based on this and	it presently supports Micron and
 * 			Spansion - 128, 256 and 512Mbit and Winbond 128Mbit.
 *
 * @param	QspiPsuPtr is pointer to qspi instance
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_UNSUPPORTED_QSPI_FLASH_ID on unsupported QSPI flash.
 * 			- XLOADER_ERR_UNSUPPORTED_QSPI_FLASH_SIZE on unsupported flash size.
 *
 *****************************************************************************/
static int FlashReadID(XQspiPsu *QspiPsuPtr)
{
	int Status = XST_FAILURE;
	XQspiPsu_Msg FlashMsg[2U] = {0U,};
	u8 TxBfr;
	u8 ReadBuffer[4U] __attribute__ ((aligned(32U)));

	/**
	 * - Read ID.
	 */
	TxBfr = XLOADER_READ_ID_CMD;
	FlashMsg[0U].TxBfrPtr = &TxBfr;
	FlashMsg[0U].RxBfrPtr = NULL;
	FlashMsg[0U].ByteCount = XLOADER_READ_ID_CMD_TX_BYTE_CNT;
	FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1U].TxBfrPtr = NULL;
	FlashMsg[1U].RxBfrPtr = ReadBuffer;
	FlashMsg[1U].ByteCount = XLOADER_READ_ID_CMD_RX_BYTE_CNT;
	FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1U].Flags = XQSPIPSU_MSG_FLAG_RX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, &FlashMsg[0U],
		XPLMI_ARRAY_SIZE(FlashMsg));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XLoader_Printf(DEBUG_GENERAL,
		"FlashID=0x%x 0x%x 0x%x\n\r",
		ReadBuffer[0U], ReadBuffer[1U], ReadBuffer[2U]);

	/**
	 * - Deduce flash make.
	 */
	if (ReadBuffer[0U] == XLOADER_MICRON_ID) {
		QspiFlashMake = XLOADER_MICRON_ID;
		XLoader_Printf(DEBUG_INFO, "MICRON ");
	}
	else if(ReadBuffer[0U] == XLOADER_SPANSION_ID) {
		QspiFlashMake = XLOADER_SPANSION_ID;
		XLoader_Printf(DEBUG_INFO, "SPANSION ");
	}
	else if(ReadBuffer[0U] == XLOADER_WINBOND_ID) {
		QspiFlashMake = XLOADER_WINBOND_ID;
		XLoader_Printf(DEBUG_INFO, "WINBOND ");
	}
	else if(ReadBuffer[0U] == XLOADER_MACRONIX_ID) {
		QspiFlashMake = XLOADER_MACRONIX_ID;
		XLoader_Printf(DEBUG_INFO, "MACRONIX ");
	}
	else if(ReadBuffer[0U] == XLOADER_ISSI_ID) {
		QspiFlashMake = XLOADER_ISSI_ID;
		XLoader_Printf(DEBUG_INFO, "ISSI ");
	}
	else {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UNSUPPORTED_QSPI_FLASH_ID, 0);
		XLoader_Printf(DEBUG_GENERAL,
				"XLOADER_ERR_UNSUPPORTED_QSPI - Unsupported"
				" FlashID \r\n");
		goto END;
	}

	/**
	 * - Deduce flash Size.
	 */
	if (ReadBuffer[2U] == XLOADER_FLASH_SIZE_ID_64M) {
		QspiFlashSize = XLOADER_FLASH_SIZE_64M;
		XLoader_Printf(DEBUG_INFO, "64M Bits\r\n");
	}
	else if ((ReadBuffer[2U] == XLOADER_FLASH_SIZE_ID_128M)
		|| (ReadBuffer[2U] == XLOADER_MACRONIX_FLASH_1_8_V_SIZE_ID_128M)) {
		QspiFlashSize = XLOADER_FLASH_SIZE_128M;
		XLoader_Printf(DEBUG_INFO, "128M Bits\r\n");
	}
	else if (ReadBuffer[2U] == XLOADER_FLASH_SIZE_ID_256M) {
		QspiFlashSize = XLOADER_FLASH_SIZE_256M;
		XLoader_Printf(DEBUG_INFO, "256M Bits\r\n");
	}
	else if ((ReadBuffer[2U] == XLOADER_FLASH_SIZE_ID_512M)
		|| (ReadBuffer[2U] == XLOADER_MACRONIX_FLASH_SIZE_ID_512M)
		|| (ReadBuffer[2U] == XLOADER_MACRONIX_FLASH_1_8_V_SIZE_ID_512M)){
		QspiFlashSize = XLOADER_FLASH_SIZE_512M;
		XLoader_Printf(DEBUG_INFO, "512M Bits\r\n");
	}
	else if ((ReadBuffer[2U] == XLOADER_FLASH_SIZE_ID_1G)
		|| (ReadBuffer[2U] == XLOADER_MACRONIX_FLASH_SIZE_ID_1G)
		|| (ReadBuffer[2U] == XLOADER_MACRONIX_FLASH_1_8_V_SIZE_ID_1G)){
		QspiFlashSize = XLOADER_FLASH_SIZE_1G;
		XLoader_Printf(DEBUG_INFO, "1G Bits\r\n");
	}
	else if ((ReadBuffer[2U] == XLOADER_FLASH_SIZE_ID_2G)
		|| (ReadBuffer[2U] == XLOADER_MACRONIX_FLASH_SIZE_ID_2G)
		|| (ReadBuffer[2U] == XLOADER_MACRONIX_FLASH_1_8_V_SIZE_ID_2G)) {
		QspiFlashSize = XLOADER_FLASH_SIZE_2G;
		XLoader_Printf(DEBUG_INFO, "2G Bits\r\n");
	}
	else {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UNSUPPORTED_QSPI_FLASH_SIZE, 0);
		XLoader_Printf(DEBUG_GENERAL,
				"XLOADER_ERR_UNSUPPORTED_QSPI - Unsupported"
				" FlashSize \r\n");
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the qspi controller
 * 			and driver.
 *
 * @param	DeviceFlags is unused and is only for compatibility with other
 *			boot device copy functions.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_PM_DEV_QSPI on QSPI device request fail.
 * 			- XLOADER_ERR_MEMSET on QSPI instance creatiion fail.
 * 			- XLOADER_ERR_QSPI_INIT on driver initialization fail.
 * 			- XLOADER_ERR_QSPI_MANUAL_START if QSPI driver manual start fails.
 * 			- XLOADER_ERR_QSPI_CONNECTION if connection listed is other than
 * 			Single, Dual, or stacked.
 * 			- XLOADER_ERR_QSPI_PRESCALER_CLK if prescaler setting fails.
 *
 *****************************************************************************/
int XLoader_QspiInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;
	const XQspiPsu_Config *QspiConfig =
		XQspiPsu_LookupConfig(XLOADER_QSPI_DEVICE);
	u32 CapSecureAccess = (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE;

	Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_QSPI,
		CapSecureAccess, XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_QSPI, Status);
		goto END;
	}
	QspiBootMode = (PdiSrc_t)DeviceFlags;
	Status = XPlmi_MemSetBytes(&QspiPsuInstance, sizeof(QspiPsuInstance),
				0U, sizeof(QspiPsuInstance));
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_MEMSET, (int)XLOADER_ERR_MEMSET_QSPI_PSU_INST);
		goto END;
	}

	/**
	 * - Initialize the QSPI driver so that it's ready to use.
	 */
	if (NULL == QspiConfig) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_QSPI_INIT, 0);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_QSPI_INIT\r\n");
		goto END;
	}

	Status = XQspiPsu_CfgInitialize(&QspiPsuInstance, QspiConfig,
			QspiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_QSPI_INIT, Status);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_QSPI_INIT\r\n");
		goto END;
	}

	/** - Enable SLVERR. */
	XPlmi_UtilRMW((QspiPsuInstance.Config.BaseAddress +
			XQSPIPSU_QSPIDMA_DST_CTRL_OFFSET),
			XQSPIPSU_QSPIDMA_DST_CTRL_APB_ERR_RESP_MASK,
			XQSPIPSU_QSPIDMA_DST_CTRL_APB_ERR_RESP_MASK);

	/**
	 * - Set Manual Start.
	 */
	Status = XQspiPsu_SetOptions(&QspiPsuInstance,
				XQSPIPSU_MANUAL_START_OPTION);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_QSPI_MANUAL_START, Status);
		XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_QSPI_MANUAL_START\r\n");
		goto END;
	}

	/**
	 * - Set the pre-scaler for QSPI clock.
	 */
	Status = XQspiPsu_SetClkPrescaler(&QspiPsuInstance,
				XQSPIPSU_CLK_PRESCALE_8);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_QSPI_PRESCALER_CLK, Status);
		XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_QSPI_PRESCALER_CLK\r\n");
		goto END;
	}
	XQspiPsu_SelectFlash(&QspiPsuInstance,
		XQSPIPSU_SELECT_FLASH_CS_LOWER, XQSPIPSU_SELECT_FLASH_BUS_LOWER);

	QspiMode = (u8)XLOADER_QSPI_CONNECTION_MODE;
	switch (QspiMode) {
		case XQSPIPSU_CONNECTION_MODE_SINGLE:
			XLoader_Printf(DEBUG_INFO,
				"QSPI is in single flash connection\r\n");
			break;
		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
			XLoader_Printf(DEBUG_INFO,
				"QSPI is in Dual Parallel connection\r\n");
			break;
		case XQSPIPSU_CONNECTION_MODE_STACKED:
			XLoader_Printf(DEBUG_INFO,
				"QSPI is in Dual Stack connection\r\n");
			break;
		default:
			Status = XPlmi_UpdateStatus(XLOADER_ERR_QSPI_CONNECTION, Status);
			XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_QSPI_CONNECTION\r\n");
			break;
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}
	QspiBusWidth = (u8)XLOADER_QSPI_BUS_WIDTH;
	switch (QspiBusWidth) {
		case XLOADER_QSPI_BUSWIDTH_ONE:
			if (QspiBootMode == XLOADER_PDI_SRC_QSPI24) {
				ReadCommand = XLOADER_FAST_READ_CMD_24BIT;
			}
			else {
				ReadCommand = XLOADER_FAST_READ_CMD_32BIT;
			}
			QspiBusWidth = XQSPIPSU_SELECT_MODE_SPI;
			break;
		case XLOADER_QSPI_BUSWIDTH_TWO:
			if (QspiBootMode == XLOADER_PDI_SRC_QSPI24) {
				ReadCommand = XLOADER_DUAL_READ_CMD_24BIT;
			}
			else {
				ReadCommand = XLOADER_DUAL_READ_CMD_32BIT;
			}
			QspiBusWidth = XQSPIPSU_SELECT_MODE_DUALSPI;
			break;
		case XLOADER_QSPI_BUSWIDTH_FOUR:
			if (QspiBootMode == XLOADER_PDI_SRC_QSPI24) {
				ReadCommand = XLOADER_QUAD_READ_CMD_24BIT;
			}
			else {
				ReadCommand = XLOADER_QUAD_READ_CMD_32BIT;
			}
			QspiBusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
			break;
		default:
			Status = XPlmi_UpdateStatus(
					XLOADER_ERR_QSPI_CONNECTION, Status);
			XLoader_Printf(DEBUG_GENERAL,
					"XLOADER_ERR_QSPI_CONNECTION\r\n");
			break;
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Read Flash ID and extract Manufacture and Size information. */
	Status = FlashReadID(&QspiPsuInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((QspiMode == XQSPIPSU_CONNECTION_MODE_PARALLEL) ||
		(QspiMode == XQSPIPSU_CONNECTION_MODE_STACKED)) {
		QspiFlashSize = 2U * QspiFlashSize;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the qspi controller
 * 			and driver.
 *
 * @param   ImageOffsetAddress is the Offset Address of the image in PDI
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 * 			- XLOADER_ERR_QSPI_CONNECTION if connection listed is other than
 * 			Single, Dual, or stacked.
 *
 *****************************************************************************/
int XLoader_QspiGetBusWidth(u64 ImageOffsetAddress)
{
	int Status = XST_FAILURE;
	u32 QspiWidthBuffer[4U];

	/* Qspi width detection for 1x, 2x and 4x */
	/** - Update the flash ReadCommand. */
	if (QspiBootMode == XLOADER_PDI_SRC_QSPI24) {
		if (QspiFlashMake == XLOADER_MACRONIX_ID) {
			ReadCommand = XLOADER_QUAD_READ_CMD_24BIT2;
		}
		else {
			ReadCommand = XLOADER_QUAD_READ_CMD_24BIT;
		}
	}
	else {
		ReadCommand = XLOADER_QUAD_READ_CMD_32BIT;
	}
	QspiBusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
	/** - Detect the QSPI bus width from the boot header. */
	Status = XLoader_QspiCopy((ImageOffsetAddress +
				XLOADER_QSPI_BUSWIDTH_PDI_OFFSET),
				(u64)(UINTPTR)(&QspiWidthBuffer),
				XLOADER_QSPI_BUSWIDTH_LENGTH, 0U);

	if ((Status == XST_SUCCESS) &&
		(QspiWidthBuffer[0U] == XLOADER_QSPI_BUSWIDTH_DETECT_VALUE)) {
		XLoader_Printf(DEBUG_INFO, "ConnectionType: QUAD\n\r");
	}
	else {
		if (QspiBootMode == XLOADER_PDI_SRC_QSPI24) {
			ReadCommand = XLOADER_DUAL_READ_CMD_24BIT;
		}
		else {
			ReadCommand = XLOADER_DUAL_READ_CMD_32BIT;
		}
		QspiBusWidth = XQSPIPSU_SELECT_MODE_DUALSPI;
		Status = XLoader_QspiCopy((ImageOffsetAddress +
					XLOADER_QSPI_BUSWIDTH_PDI_OFFSET),
					(u64)(UINTPTR)(&QspiWidthBuffer),
					XLOADER_QSPI_BUSWIDTH_LENGTH, 0U);
		if((Status == XST_SUCCESS) && (QspiWidthBuffer[0U] ==
			XLOADER_QSPI_BUSWIDTH_DETECT_VALUE)){
			XLoader_Printf(DEBUG_INFO, "ConnectionType: DUAL\n\r");
		}
		else {
			if (QspiBootMode == XLOADER_PDI_SRC_QSPI24) {
				ReadCommand = XLOADER_FAST_READ_CMD_24BIT;
			}
			else {
				ReadCommand = XLOADER_FAST_READ_CMD_32BIT;
			}
			QspiBusWidth = XQSPIPSU_SELECT_MODE_SPI;
			Status = XLoader_QspiCopy((ImageOffsetAddress +
					XLOADER_QSPI_BUSWIDTH_PDI_OFFSET),
					(u64)(UINTPTR)(&QspiWidthBuffer),
					XLOADER_QSPI_BUSWIDTH_LENGTH, 0U);
			if ((Status == XST_SUCCESS) && (QspiWidthBuffer[0U] ==
					XLOADER_QSPI_BUSWIDTH_DETECT_VALUE)){
				XLoader_Printf(DEBUG_INFO, "ConnectionType: "
						"SINGLE\n\r");
			}
			else {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_QSPI_CONNECTION, Status);
				XLoader_Printf(DEBUG_GENERAL,
					"XLOADER_ERR_QSPI_CONNECTION\r\n");
			}
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This functions translates the address based on the type of
 * 			interconnection. In case of stacked, this function asserts the
 * 			corresponding slave select.
 *
 * @param	Addr which is to be accessed
 *
 * @return	QspiAddr is the actual flash address to be accessed
 *		- for single, it is unchanged
 *		- for stacked the lower flash size is subtracted
 *		- for parallel the address is divided by 2.
 *
 *****************************************************************************/
static u32 XLoader_GetQspiAddr(u32 Addr)
{
	u32 RealAddr;

	switch(QspiPsuInstance.Config.ConnectionMode) {
		case XQSPIPSU_CONNECTION_MODE_SINGLE:
			XQspiPsu_SelectFlash(&QspiPsuInstance,
				XQSPIPSU_SELECT_FLASH_CS_LOWER,
				XQSPIPSU_SELECT_FLASH_BUS_LOWER);
			RealAddr = Addr;
			break;
		case XQSPIPSU_CONNECTION_MODE_STACKED:
			/**
			 * - Select lower or upper Flash based on sector address for
             *   stacked connection type.
			 */
			if (Addr >= (QspiFlashSize / 2U)) {
				XQspiPsu_SelectFlash(&QspiPsuInstance,
					XQSPIPSU_SELECT_FLASH_CS_UPPER,
					XQSPIPSU_SELECT_FLASH_BUS_LOWER);
				/**
				 * - Subtract first flash size when accessing second flash.
				 */
				RealAddr = Addr - (QspiFlashSize / 2U);
			}
			else {
				/*
				 * Set selection to L_PAGE.
				 */
				XQspiPsu_SelectFlash(&QspiPsuInstance,
					XQSPIPSU_SELECT_FLASH_CS_LOWER,
					XQSPIPSU_SELECT_FLASH_BUS_LOWER);
				RealAddr = Addr;
			}
			break;
		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
			/**
			 * - If connection type is parallel, then effective address in
             *   each flash is the actual address / 2.
			 */
			XQspiPsu_SelectFlash(&QspiPsuInstance,
				XQSPIPSU_SELECT_FLASH_CS_BOTH,
				XQSPIPSU_SELECT_FLASH_BUS_BOTH);
			RealAddr = Addr / 2U;
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
 * @brief	This functions selects the current bank
 *
 * @param	BankSel is the bank to be selected in the flash device(s).
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_QSPI_READ if QSPI driver fails to read flash.
 *
 *****************************************************************************/
static int SendBankSelect(u32 BankSel)
{
	int Status =  XST_FAILURE;
	XQspiPsu_Msg FlashMsg[2U] = {0U,};
	u8 TxBfr;
	u8 ReadBuffer[10U] __attribute__ ((aligned(32U))) = {0U};
	u8 WriteBuffer[10U] __attribute__ ((aligned(32U)));

	/*
	 * Bank select commands for Micron and Spansion are different.
	 * Macronix bank select is same as Micron.
	 */
	if ((QspiFlashMake == XLOADER_MICRON_ID) ||
		(QspiFlashMake == XLOADER_MACRONIX_ID))	{
		/**
		 * - For micron command WREN should be sent first
		 * except for some specific feature set.
		 */
		TxBfr = XLOADER_WRITE_ENABLE_CMD;
		FlashMsg[0U].TxBfrPtr = &TxBfr;
		FlashMsg[0U].RxBfrPtr = NULL;
		FlashMsg[0U].ByteCount = XLOADER_QSPI_WRITE_ENABLE_CMD_BYTE_CNT;
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg[0U], 1U);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_QSPI_READ, Status);
			XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_QSPI_READ\r\n");
			goto END;
		}

		/**
		 * Send the Extended address register write command
		 * written, no receive buffer required.
		 */
		WriteBuffer[XLOADER_COMMAND_OFST] = XLOADER_EXTADD_REG_WR_CMD;
		WriteBuffer[XLOADER_ADDR_1_OFST] = (u8)BankSel;

		FlashMsg[0U].TxBfrPtr = WriteBuffer;
		FlashMsg[0U].RxBfrPtr = NULL;
		FlashMsg[0U].ByteCount = XLOADER_EXTADD_REG_WR_CMD_BYTE_CNT;
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance,
						&FlashMsg[0U], 1U);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_QSPI_READ,
						Status);
			XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_QSPI_READ\r\n");
			goto END;
		}

		/**
		 * Verify extended address register read command.
		 */
		WriteBuffer[XLOADER_COMMAND_OFST] = XLOADER_EXTADD_REG_RD_CMD;
		FlashMsg[0U].TxBfrPtr = WriteBuffer;
		FlashMsg[0U].RxBfrPtr = NULL;
		FlashMsg[0U].ByteCount = XLOADER_EXTADD_REG_RD_CMD_BYTE_CNT;
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1U].TxBfrPtr = NULL;
		FlashMsg[1U].RxBfrPtr = ReadBuffer;
		FlashMsg[1U].ByteCount = XLOADER_EXTADD_REG_RD_CMD_BYTE_CNT;
		FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1U].Flags = XQSPIPSU_MSG_FLAG_RX;

		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance,
				&FlashMsg[0U], XPLMI_ARRAY_SIZE(FlashMsg));
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_QSPI_READ,
						Status);
			XLoader_Printf(DEBUG_GENERAL,
					"XLOADER_ERR_QSPI_READ\r\n");
			goto END;
		}
	}
	else if (QspiFlashMake == XLOADER_SPANSION_ID) {
		/**
		 * Send the Extended address register write command
		 * written, no receive buffer required.
		 */
		WriteBuffer[XLOADER_COMMAND_OFST] = XLOADER_BANK_REG_WR_CMD;
		WriteBuffer[XLOADER_ADDR_1_OFST] = (u8)BankSel;

		FlashMsg[0U].TxBfrPtr = WriteBuffer;
		FlashMsg[0U].RxBfrPtr = NULL;
		FlashMsg[0U].ByteCount = XLOADER_BANK_REG_WR_CMD_BYTE_CNT;
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(
				&QspiPsuInstance, &FlashMsg[0U], 1U);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
				XLOADER_ERR_QSPI_READ, Status);
			XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_QSPI_READ\r\n");
			goto END;
		}

		/**
		 * For testing - Read bank to verify
		 * Bank register read command.
		 */
		WriteBuffer[XLOADER_COMMAND_OFST] = XLOADER_BANK_REG_RD_CMD;
		FlashMsg[0U].TxBfrPtr = WriteBuffer;
		FlashMsg[0U].RxBfrPtr = NULL;
		FlashMsg[0U].ByteCount = XLOADER_BANK_REG_RD_CMD_BYTE_CNT;
		FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1U].TxBfrPtr = NULL;
		FlashMsg[1U].RxBfrPtr = ReadBuffer;
		FlashMsg[1U].ByteCount = XLOADER_BANK_REG_RD_CMD_BYTE_CNT;
		FlashMsg[1U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1U].Flags = XQSPIPSU_MSG_FLAG_RX;

		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance,
				&FlashMsg[0U], XPLMI_ARRAY_SIZE(FlashMsg));
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_QSPI_READ,
						Status);
			XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_QSPI_READ\r\n");
			goto END;
		}

		if (ReadBuffer[0U] != BankSel) {
			XLoader_Printf(DEBUG_INFO, "Bank Select %u != "
					"Register Read %u\n\r",
					BankSel, ReadBuffer[0U]);
			Status = XPlmi_UpdateStatus(XLOADER_ERR_QSPI_READ,
						Status);
			XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_QSPI_READ\r\n");
			goto END;
		}
	}
	else {
		/* MISRA-C compliance */
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to copy the data from QSPI flash to
 * 			destination address.
 *
 * @param	SrcAddr is the address of the QSPI flash where copy should
 *			start from.
 * @param	DestAddr is the address of the destination where it
 *			should copy to.
 * @param	Length of the bytes to be copied
 * @param	Flags denote the blocking / non-blocking nature of copy
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_QSPI_LENGTH if read length is greater than flash size.
 * 			- XLOADER_ERR_QSPI_READ if driver fails to read.
 *
 *****************************************************************************/
int XLoader_QspiCopy(u64 SrcAddr, u64 DestAddr, u32 Length, u32 Flags)
{
	int Status = XST_FAILURE;
	u32 QspiAddr;
	u32 OrigAddr;
	u32 BankSel;
	u32 RemainingBytes;
	u32 TransferBytes;
	u32 DiscardByteCnt;
	u32 BankSize;
	u64 BankMask;
	u32 SrcAddrLow = (u32)SrcAddr;
	XQspiPsu_Msg FlashMsg[3U] = {0U,};
	u8 WriteBuffer[10U] __attribute__ ((aligned(32U))) = {0U};
	u64 DestOffset = 0U;
	u32 ParallelDmaFlags = Flags & XPLMI_DEVICE_COPY_STATE_MASK;

#ifdef PLM_PRINT_PERF_DMA
	u64 QspiCopyTime = XPlmi_GetTimerValue();
	XPlmi_PerfTime PerfTime = {0U};
#endif

	XLoader_Printf(DEBUG_INFO, "QSPI Reading Src 0x%08x, Dest 0x%0x%08x, "
			"Length 0x%08x, Flags 0x%0x\r\n", SrcAddrLow,
			(u32)(DestAddr >> 32U), (u32)DestAddr, Length, Flags);

	/* Just wait for the Data to be copied */
	if (ParallelDmaFlags == XPLMI_DEVICE_COPY_STATE_WAIT_DONE) {
		do {
			Status = XQspiPsu_CheckDmaDone(&QspiPsuInstance);
		} while (Status != XST_SUCCESS);
		goto END;
	}

	/**
	 * - Check the read length with Qspi flash size.
	 */
	if ((SrcAddrLow + Length) > QspiFlashSize) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_QSPI_LENGTH, 0);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_QSPI_LENGTH\r\n");
		goto END;
	}

	if (QspiBootMode == XLOADER_PDI_SRC_QSPI32) {
		DiscardByteCnt = XLOADER_QSPI32_COPY_DISCARD_BYTE_CNT;
	}
	else {
		DiscardByteCnt = XLOADER_QSPI24_COPY_DISCARD_BYTE_CNT;
	}

	FlashMsg[0U].RxBfrPtr = (u8 *)NULL;
	FlashMsg[0U].ByteCount = DiscardByteCnt;
	FlashMsg[0U].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0U].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1U].TxBfrPtr = (u8 *)NULL;
	FlashMsg[1U].RxBfrPtr = (u8 *)NULL;
	FlashMsg[1U].ByteCount = XLOADER_DUMMY_CLOCKS;
	FlashMsg[1U].BusWidth = QspiBusWidth;
	FlashMsg[1U].Flags = 0U;

	FlashMsg[2U].TxBfrPtr = (u8 *)NULL;
	FlashMsg[2U].Xfer64bit = 1U;
	FlashMsg[2U].BusWidth = QspiBusWidth;
	FlashMsg[2U].Flags = XQSPIPSU_MSG_FLAG_RX;

	if (QspiFlashMake == XLOADER_WINBOND_ID) {
		BankSize = XLOADER_WINBOND_BANKSIZE;
		BankMask = XLOADER_WINBOND_BANKMASK;
	}
	else {
		BankSize = XLOADER_BANKSIZE;
		BankMask = XLOADER_BANKMASK;
	}

	if (QspiMode == XQSPIPSU_CONNECTION_MODE_PARALLEL) {
		FlashMsg[2U].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
		BankSize *= 2U;
		BankMask *= 2U;
	}

	/**
	 * - Update no of bytes to be copied.
	 */
	RemainingBytes = Length;

	while (RemainingBytes > 0U) {
		if (RemainingBytes > XLOADER_DMA_DATA_TRAN_SIZE) {
			TransferBytes = XLOADER_DMA_DATA_TRAN_SIZE;
		}
		else {
			TransferBytes = RemainingBytes;
		}

		/**
		 * - Translate address based on type of connection.
		 * If stacked assert the slave select based on address.
		 */
		QspiAddr = XLoader_GetQspiAddr(SrcAddrLow);

		if ((QspiBootMode == XLOADER_PDI_SRC_QSPI24) ||
			(QspiFlashMake == XLOADER_WINBOND_ID)) {

			/**
			 * - Multiply address by 2 in case of Dual Parallel.
			 * This address is used to calculate the bank crossing
			 * condition.
			 */
			if (QspiPsuInstance.Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL) {
				OrigAddr = QspiAddr * 2U;
			}
			else {
				OrigAddr = QspiAddr;
			}

			/**
			 * - Select bank check logic for DualQspi.
			 */
			if (QspiFlashSize > BankSize) {
				if (QspiFlashMake == XLOADER_WINBOND_ID) {
					BankSel = QspiAddr / XLOADER_WINBOND_BANKSIZE;
				}
				else {
					BankSel = QspiAddr / XLOADER_BANKSIZE;
				}
				Status = SendBankSelect(BankSel);
				if (Status != XST_SUCCESS) {
					Status = XPlmi_UpdateStatus(
						XLOADER_ERR_QSPI_READ, Status);
					XLoader_Printf(DEBUG_GENERAL,
						"XLOADER_ERR_QSPI_READ\r\n");
					goto END;
				}
			}

			/**
			 * - If data to be read spans beyond the current bank, then
			 * calculate Transfer Bytes in current bank. Else
			 * transfer bytes are same.
			 */
			if ((OrigAddr & BankMask) !=
				(((u64)OrigAddr + TransferBytes) & BankMask)) {
				TransferBytes = (u32)((OrigAddr & BankMask) +
						BankSize - OrigAddr);
			}
		}

		XLoader_Printf(DEBUG_DETAILED, "QSPI Read Src 0x%08x, "
				"Dest 0x%0x%08x, Length %08x\r\n",
				QspiAddr, (u32)((DestAddr + DestOffset) >> 32U),
				(u32)(DestAddr + DestOffset), TransferBytes);

		/**
		 * - Setup the read command with the specified address
		 * and data for the Flash.
		 */

		WriteBuffer[XLOADER_COMMAND_OFST] = (u8)ReadCommand;

		if (QspiBootMode == XLOADER_PDI_SRC_QSPI32) {
			WriteBuffer[XLOADER_ADDR_4_OFST] =
					(u8)(QspiAddr & 0xFFU);
			QspiAddr >>= 8U;
		}

		WriteBuffer[XLOADER_ADDR_3_OFST] = (u8)(QspiAddr & 0xFFU);
		QspiAddr >>= 8U;

		WriteBuffer[XLOADER_ADDR_2_OFST] = (u8)(QspiAddr & 0xFFU);
		QspiAddr >>= 8U;

		WriteBuffer[XLOADER_ADDR_1_OFST] = (u8)(QspiAddr & 0xFFU);

		FlashMsg[0U].TxBfrPtr = WriteBuffer;
		FlashMsg[2U].RxAddr64bit = DestAddr + DestOffset;
		FlashMsg[2U].ByteCount = TransferBytes;

		/**
		 * - Send the read command to the Flash to read the specified number
		 * of bytes from the Flash, send the read command and address and
		 * receive the specified number of bytes of data in the data buffer.
		 */
		if ((ParallelDmaFlags == XPLMI_DEVICE_COPY_STATE_INITIATE) &&
				(RemainingBytes == TransferBytes)) {
			Status = XQspiPsu_StartDmaTransfer(&QspiPsuInstance, &FlashMsg[0U],
						XPLMI_ARRAY_SIZE(FlashMsg));
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_QSPI_READ, Status);
			}
			goto END;
		}

		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg[0U],
				XPLMI_ARRAY_SIZE(FlashMsg));
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_QSPI_READ,
				Status);
			XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_QSPI_READ\r\n");
			goto END;
		}

		/*
		 * Update the variables
		 */
		RemainingBytes -= TransferBytes;
		DestOffset += TransferBytes;
		SrcAddrLow += TransferBytes;
	}

	Status = XST_SUCCESS;

END:
#ifdef	PLM_PRINT_PERF_DMA
	XPlmi_MeasurePerfTime(QspiCopyTime, &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_PERF, "%u.%03u ms QSPI Copy time:"
	"SrcAddr: 0x%08x, DestAddr: 0x%0x08x, %u Bytes, Flags: 0x%0x\n\r",
	(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac, SrcAddrLow,
	(u32)((DestAddr + DestOffset) >> 32U), (u32)(DestAddr + DestOffset), Length,
	ParallelDmaFlags);
#endif
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function releases control of QSPI.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_QspiRelease(void)
{
	int Status = XST_FAILURE;

    /**
     * - Release control of QSPI by calling XPm_ReleaseDevice.
     */
	Status = XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_QSPI, XPLMI_CMD_SECURE);

	return Status;
}
#endif
