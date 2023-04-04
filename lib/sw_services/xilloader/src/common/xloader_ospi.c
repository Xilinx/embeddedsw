/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.00  bsv  08/23/2018 Initial release
* 1.01  bsv  09/10/2019 Added support to set OSPI to DDR mode
*       ma   02/03/2020 Change XPlmi_MeasurePerfTime to retrieve Performance
*                       time and print
*       bsv  02/04/2020 Reset qspi instance in init functions for LPD off
*						suspend and resume to work
*       bsv  04/09/2020 Code clean up of Xilloader
* 1.02  bsv  27/06/2020 Add dual stacked mode support
*       bsv  07/08/2020 APIs specific to this file made static
*       skd  07/14/2020 XLoader_OspiCopy prototype changed
*       skd  07/29/2020 Added non-blocking DMA support for Ospi copy
*       skd  08/21/2020 Added support for GIGADEVICE and ISSI flash parts
*       bsv  10/13/2020 Code clean up
* 1.03  ma   03/24/2021 Minor updates to prints in XilLoader
* 1.04  bsv  07/16/2021 Added Macronix flash support
*       bsv  08/31/2021 Code clean up
* 1.05  ma   01/17/2022 Enable SLVERR for OSPI registers
* 1.06  ng   11/11/2022 Updated doxygen comments
*       bm   01/11/2023 Added support for Gigadevice 512M, 1G, 2G parts
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xloader_ospi.h"
#include "xplmi_proc.h"
#include "xloader.h"
#include "xplmi.h"
#include "xparameters.h"	/* SDK generated parameters */
#include "xplmi_status.h"	/* PLMI error codes */

#ifdef XLOADER_OSPI
#include "xospipsv.h"		/* OSPIPSV device driver */
#include "xpm_api.h"
#include "xpm_nodeid.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int FlashReadID(XOspiPsv *OspiPsvPtr);
static int XLoader_FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, u32 Enable);
static int XLoader_FlashSetDDRMode(XOspiPsv *OspiPsvPtr);

/************************** Variable Definitions *****************************/
static XOspiPsv OspiPsvInstance;
static u8 OspiFlashMake;
static u32 OspiFlashSize = 0U;

/*****************************************************************************/
/**
 * @brief	This function reads serial FLASH ID connected to the SPI interface.
 * 			It then deduces the make and size of the flash and obtains the
 * 			connection mode to point to corresponding parameters in the flash
 * 			configuration table. The flash driver will function based on this
 * 			and	it presently supports Micron 512Mb.
 *
 * @param	Ospi Instance Pointer
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_UNSUPPORTED_OSPI on unsupported OSPI flash.
 * 			- XLOADER_ERR_UNSUPPORTED_OSPI_SIZE on unsupported OSPI flash size.
 *
 *****************************************************************************/
static int FlashReadID(XOspiPsv *OspiPsvPtr)
{
	int Status =  XST_FAILURE;
	u32 Index;
	u8 ReadBuffer[XLOADER_READ_ID_BYTES] __attribute__ ((aligned(32U)));
	XOspiPsv_Msg FlashMsg = {0U};
	u32 TempVal;

	/**
	 * - Read ID.
	 */
	FlashMsg.Opcode = READ_ID;
	FlashMsg.Addrsize = 0U;
	FlashMsg.Addrvalid = FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ReadBuffer;
	FlashMsg.ByteCount = XLOADER_READ_ID_BYTES;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	Status = (int)XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XLoader_Printf(DEBUG_GENERAL, "FlashID=0x%x 0x%x 0x%x\n\r",
		ReadBuffer[0U], ReadBuffer[1U], ReadBuffer[2U]);

	/**
	 * - Deduce flash make.
	 */
	if (ReadBuffer[0U] == MICRON_OCTAL_ID_BYTE0) {
		OspiFlashMake = MICRON_OCTAL_ID_BYTE0;
		XLoader_Printf(DEBUG_INFO, "MICRON");
	}
	else if (ReadBuffer[0U] == GIGADEVICE_OCTAL_ID_BYTE0) {
		OspiFlashMake = GIGADEVICE_OCTAL_ID_BYTE0;
		XLoader_Printf(DEBUG_INFO, "GIGADEVICE");
	}
	else if (ReadBuffer[0U] == ISSI_OCTAL_ID_BYTE0) {
		OspiFlashMake = ISSI_OCTAL_ID_BYTE0;
		XLoader_Printf(DEBUG_INFO, "ISSI");
	}
	else if (ReadBuffer[0U] == MACRONIX_OCTAL_ID_BYTE0) {
		OspiFlashMake = MACRONIX_OCTAL_ID_BYTE0;
		XLoader_Printf(DEBUG_INFO, "MACRONIX");
	}
	else {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UNSUPPORTED_OSPI, 0);
		XLoader_Printf(DEBUG_GENERAL, "XLOADER_ERR_UNSUPPORTED_OSPI\r\n");
		goto END;
	}

	/**
	 * - Deduce flash Size.
	 */
	OspiFlashSize = 0U;
	if (OspiFlashMake == MICRON_OCTAL_ID_BYTE0) {
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
			/* Do nothing */
		}
	}
	else if (OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) {
		if (ReadBuffer[2U] == MACRONIX_OCTAL_ID_BYTE2_512) {
			OspiFlashSize = XLOADER_FLASH_SIZE_512M;
		}
	}
	else if (OspiFlashMake == GIGADEVICE_OCTAL_ID_BYTE0) {
		if (ReadBuffer[2U] == GIGADEVICE_OCTAL_ID_BYTE2_256) {
			OspiFlashSize = XLOADER_FLASH_SIZE_256M;
		}
		else if (ReadBuffer[2U] == GIGADEVICE_OCTAL_ID_BYTE2_512) {
			OspiFlashSize = XLOADER_FLASH_SIZE_512M;
		}
		else if (ReadBuffer[2U] == GIGADEVICE_OCTAL_ID_BYTE2_1G) {
			OspiFlashSize = XLOADER_FLASH_SIZE_1G;
		}
		else if (ReadBuffer[2U] == GIGADEVICE_OCTAL_ID_BYTE2_2G) {
			OspiFlashSize = XLOADER_FLASH_SIZE_2G;
		}
		else {
			/* Do nothing */
		}
	}
	else if (ReadBuffer[2U] == XLOADER_FLASH_SIZE_ID_256M) {
		OspiFlashSize = XLOADER_FLASH_SIZE_256M;
	}
	else if (ReadBuffer[2U] == XLOADER_FLASH_SIZE_ID_512M) {
		OspiFlashSize = XLOADER_FLASH_SIZE_512M;
	}
	else if (ReadBuffer[2U] == XLOADER_FLASH_SIZE_ID_1G) {
		OspiFlashSize = XLOADER_FLASH_SIZE_1G;
	}
	else if (ReadBuffer[2U] == XLOADER_FLASH_SIZE_ID_2G) {
		OspiFlashSize = XLOADER_FLASH_SIZE_2G;
	}
	else {
		/* Do nothing */
	}

	if (OspiFlashSize == 0U) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_UNSUPPORTED_OSPI_SIZE, 0);
		XLoader_Printf(DEBUG_GENERAL,
			"XLOADER_ERR_UNSUPPORTED_OSPI_SIZE\r\n");
		goto END;
	}
	/**
	 * - Populate Device Id Data.
	 */
	OspiPsvPtr->DeviceIdData = 0U;
	if (OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) {
		for (Index = 0U, TempVal = 0U; Index < 4U;
			++Index, TempVal += XLOADER_READ_ID_BYTES) {
			OspiPsvPtr->DeviceIdData |=
				(ReadBuffer[Index >> 1U] << TempVal);
		}
	}
	else {
		for (Index = 0U, TempVal = 0U; Index < 4U;
			++Index, TempVal += XLOADER_READ_ID_BYTES) {
			OspiPsvPtr->DeviceIdData |=
				(ReadBuffer[Index] << TempVal);
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the ospi controller and driver.
 *
 * @param	DeviceFlags are unused and passed for compatibility with other
 * 			boot device APIs.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_PM_DEV_OSPI on OSPI device request fail.
 * 			- XLOADER_ERR_OSPI_INIT on OSPI drive initialization fail.
 * 			- XLOADER_ERR_OSPI_CFG on OSPI configuration fail.
 * 			- XLOADER_ERR_OSPI_SEL_FLASH_CS0 on failure to select flash CS0
 * 			- XLOADER_ERR_OSPI_SEL_FLASH_CS1 on failure to select flash CS1
 * 			- XLOADER_ERR_OSPI_READID on OSPI ReadID fail.
 * 			- XLOADER_ERR_OSPI_CONN_MODE on unsupported OSPI mode.
 * 			- XLOADER_ERR_OSPI_DUAL_BYTE_OP_DISABLE on failure to disable
 * 			dual byte operation.
 * 			- XLOADER_ERR_OSPI_SDR_NON_PHY if unable to set the controller to
 * 			SDR NON PHY mode.
 *
 *****************************************************************************/
int XLoader_OspiInit(u32 DeviceFlags)
{
	int Status = XST_FAILURE;
	XOspiPsv_Config *OspiConfig;
	u8 OspiMode;
	(void)DeviceFlags;
	u32 CapSecureAccess = (u32)PM_CAP_ACCESS | (u32)PM_CAP_SECURE;

	/**
	 * - Request driver for OSPI device.
	*/
	Status = XPm_RequestDevice(PM_SUBSYS_PMC, PM_DEV_OSPI,
		CapSecureAccess, XPM_DEF_QOS, 0U, XPLMI_CMD_SECURE);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_PM_DEV_OSPI, Status);
		goto END;
	}

	/**
	 * - Request driver to reset the OSPI flash device.
	*/
	Status = (int)XOspiPsv_DeviceReset(XOSPIPSV_HWPIN_RESET);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Initialize the OSPI instance.
	*/
	Status = XPlmi_MemSetBytes(&OspiPsvInstance, sizeof(OspiPsvInstance), 0U,
		sizeof(OspiPsvInstance));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Initialize the OSPI driver so that it's ready to use.
	*/
	OspiConfig = XOspiPsv_LookupConfig(XLOADER_OSPI_DEVICE_ID);
	if (NULL == OspiConfig) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_INIT, 0);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_OSPI_INIT\r\n");
		goto END;
	}

	/**
	 * - Configure the OSPI driver.
	*/
	Status = (int)XOspiPsv_CfgInitialize(&OspiPsvInstance, OspiConfig);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_CFG, Status);
		XLoader_Printf(DEBUG_GENERAL,"XLOADER_ERR_OSPI_CFG\r\n");
		goto END;
	}

	/**
	 * - Enable SLVERR.
	*/
	XPlmi_UtilRMW((OspiConfig->BaseAddress + XOSPIPSV_OSPIDMA_SRC_CTRL),
			XOSPIPSV_OSPIDMA_SRC_CTRL_APB_ERR_RESP_MASK,
			XOSPIPSV_OSPIDMA_SRC_CTRL_APB_ERR_RESP_MASK);
	XPlmi_UtilRMW((OspiConfig->BaseAddress + XOSPIPSV_OSPIDMA_DST_CTRL),
			XOSPIPSV_OSPIDMA_DST_CTRL_APB_ERR_RESP_MASK,
			XOSPIPSV_OSPIDMA_DST_CTRL_APB_ERR_RESP_MASK);

	/**
	 * - Enable IDAC controller in OSPI.
	*/
	XOspiPsv_SetOptions(&OspiPsvInstance, XOSPIPSV_IDAC_EN_OPTION);

	/**
	 * - Set the prescaler for OSPIPSV clock.
	*/
	XOspiPsv_SetClkPrescaler(&OspiPsvInstance, XOSPIPSV_CLK_PRESCALE_6);
	Status = (int)XOspiPsv_SelectFlash(&OspiPsvInstance, XOSPIPSV_SELECT_FLASH_CS0);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_SEL_FLASH_CS0,
			Status);
		goto END;
	}

	/**
	 * - Read flash ID and obtain all flash related information
	 * It is important to call the read id function before
	 * performing proceeding to any operation, including
	 * preparing the WriteBuffer
	 */
	Status = FlashReadID(&OspiPsvInstance);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_READID, Status);
		goto END;
	}

	OspiMode = OspiPsvInstance.Config.ConnectionMode;
	switch(OspiMode) {
		case XOSPIPSV_CONNECTION_MODE_SINGLE:
		case XOSPIPSV_CONNECTION_MODE_STACKED:
			break;

		default:
			Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_CONN_MODE, 0);
			break;
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Validate OSPI DDR mode is supported on SPP, EMU and FCV platforms.
	*/
	if(XPLMI_PLATFORM == PMC_TAP_VERSION_SILICON) {
		/**
		 * - Set Flash device and Controller modes.
		*/
		Status = XLoader_FlashSetDDRMode(&OspiPsvInstance);
		if (Status != XST_SUCCESS) {
			goto END1;
		}

		if (OspiMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
			Status = (int)XOspiPsv_SelectFlash(&OspiPsvInstance, XOSPIPSV_SELECT_FLASH_CS1);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(
						XLOADER_ERR_OSPI_SEL_FLASH_CS1, Status);
				goto END;
			}

			/* Set Flash device and Controller modes */
			Status = (int)XOspiPsv_SetSdrDdrMode(&OspiPsvInstance,
						XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
			if (Status != XST_SUCCESS) {
				goto END1;
			}

			Status = XLoader_FlashSetDDRMode(&OspiPsvInstance);
			if (Status != XST_SUCCESS) {
				goto END1;
			}
		}

		XOspiPsv_SetClkPrescaler(&OspiPsvInstance,
			XOSPIPSV_CLK_PRESCALE_2);
	}

END1:
	if ((XPLMI_PLATFORM == PMC_TAP_VERSION_SILICON) && (Status != XST_SUCCESS)) {
		Status = (int)XOspiPsv_DeviceReset(XOSPIPSV_HWPIN_RESET);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = (int)XOspiPsv_SelectFlash(&OspiPsvInstance, XOSPIPSV_SELECT_FLASH_CS0);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
					XLOADER_ERR_OSPI_SEL_FLASH_CS0, Status);
			goto END;
		}
		if (OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) {
			Status = (int)XOspiPsv_ConfigDualByteOpcode(
				&OspiPsvInstance,
				(u32)XOSPIPSV_DUAL_BYTE_OP_DISABLE);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_OSPI_DUAL_BYTE_OP_DISABLE,
					Status);
				goto END;
			}
		}

		Status = (int)XOspiPsv_SetSdrDdrMode(&OspiPsvInstance,
				XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
					XLOADER_ERR_OSPI_SDR_NON_PHY, Status);
			goto END;
		}

		if (OspiMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
			Status = (int)XOspiPsv_SelectFlash(&OspiPsvInstance,
				XOSPIPSV_SELECT_FLASH_CS1);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_OSPI_SEL_FLASH_CS1, Status);
				goto END;
			}

			Status = (int)XOspiPsv_SetSdrDdrMode(&OspiPsvInstance,
						XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(
					XLOADER_ERR_OSPI_SDR_NON_PHY, Status);
				goto END;
			}
		}
		OspiPsvInstance.Extra_DummyCycle = 0U;
	}

	/**
	 * - Enter 4B address mode.
	 */
	if (OspiMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
		XLoader_FlashEnterExit4BAddMode(&OspiPsvInstance, TRUE);
		Status = (int)XOspiPsv_SelectFlash(&OspiPsvInstance, XOSPIPSV_SELECT_FLASH_CS0);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(
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
 * 			destination address.
 *
 * @param	SrcAddr is the address of the OSPI flash where copy should start
 *
 * @param	DestAddr is the address of the destination where it should copy to
 *
 * @param	Length Length of the bytes to be copied
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_OSPI_COPY_OVERFLOW if source address outside the flash
 * 			range.
 * 			- XLOADER_ERR_OSPI_SEL_FLASH_CS0 if OSPI driver is unable to select
 * 			flash CS0.
 * 			- XLOADER_ERR_OSPI_SEL_FLASH_CS1 if OSPI driver is unable to select
 * 			flash CS1.
 * 			- XLOADER_ERR_OSPI_READ on OSPI driver read fail.
 *
 *****************************************************************************/
int XLoader_OspiCopy(u64 SrcAddr, u64 DestAddr, u32 Length, u32 Flags)
{
	int Status = XST_FAILURE;
	u32 SrcAddrLow = (u32)SrcAddr;
	XOspiPsv_Msg FlashMsg = {0U};
	u32 TrfLen;
	u32 FlagsTmp;
	u8 OspiMode = OspiPsvInstance.Config.ConnectionMode;
	u8 ReadCmd = READ_CMD_OCTAL_4B;
	u8 Proto;
	u8 Dummy;
	static u8 ChipSelect = XOSPIPSV_SELECT_FLASH_CS0;
#ifdef PLM_PRINT_PERF_DMA
	u64 OspiCopyTime = XPlmi_GetTimerValue();
	XPlmi_PerfTime PerfTime = {0U};
#endif

	XLoader_Printf(DEBUG_INFO, "OSPI Reading Src 0x%0x, Dest 0x%0x%08x, "
		"Length 0x%0x, Flags 0x%0x\r\n", SrcAddrLow, (u32)(DestAddr >> 32U),
		(u32)(DestAddr), Length, Flags);

	Flags = Flags & XPLMI_DEVICE_COPY_STATE_MASK;
	/**
	 * - Verify that previous DMA copy is finished.
	*/
	if (Flags == XPLMI_DEVICE_COPY_STATE_WAIT_DONE) {
		do {
			Status = (int)XOspiPsv_CheckDmaDone(&OspiPsvInstance);
		} while (Status != XST_SUCCESS);
		goto END1;
	}
	FlagsTmp = Flags;
	/**
	 * - Check OSPI connection mode.
	*/
	if (OspiMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
		if ((SrcAddrLow + Length) > (2U * OspiFlashSize)) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_COPY_OVERFLOW, Status);
			goto END;
		}
		else {
			/**
			 * - For stacked mode connection select upper flash or lower flash
			 * depending on source address.
			 */
			if (SrcAddrLow < OspiFlashSize) {
				/*
				 * Select lower flash
				 */
				if (ChipSelect == XOSPIPSV_SELECT_FLASH_CS1) {
					ChipSelect = XOSPIPSV_SELECT_FLASH_CS0;
					Status = (int)XOspiPsv_SelectFlash(
						&OspiPsvInstance, ChipSelect);
					if (Status != XST_SUCCESS) {
						Status = XPlmi_UpdateStatus(
									XLOADER_ERR_OSPI_SEL_FLASH_CS0, Status);
						goto END;
					}
				}
				if ((SrcAddrLow + Length) > OspiFlashSize) {
					TrfLen = OspiFlashSize - SrcAddrLow;
					Flags = XPLMI_DEVICE_COPY_STATE_BLK;
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
					Status = (int)XOspiPsv_SelectFlash(&OspiPsvInstance, ChipSelect);
					if (Status != XST_SUCCESS) {
						Status = XPlmi_UpdateStatus(
									XLOADER_ERR_OSPI_SEL_FLASH_CS1, Status);
						goto END;
					}
				}
				SrcAddrLow = SrcAddrLow - OspiFlashSize;
				TrfLen = Length;
			}
		}
	}
	else {
		if ((SrcAddrLow + Length) > OspiFlashSize) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_COPY_OVERFLOW, Status);
			goto END;
		}

		TrfLen = Length;
	}

	/**
	 * - Generate the Read cmd.
	*/
	FlashMsg.Addrsize = XLOADER_OSPI_READ_ADDR_SIZE;
	FlashMsg.Addrvalid = TRUE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.ByteCount = TrfLen;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Addr = SrcAddrLow;

	if ((DestAddr >> 32U) == 0U) {
		FlashMsg.RxBfrPtr = (u8*)(UINTPTR)DestAddr;
	}
	else {
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.RxAddr64bit = DestAddr;
		FlashMsg.Xfer64bit = 1U;
	}

	if (OspiPsvInstance.SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		Proto = XOSPIPSV_READ_8_8_8;
		if (OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) {
			Dummy = XLOADER_MACRONIX_OSPI_DDR_DUMMY_CYCLES +
				OspiPsvInstance.Extra_DummyCycle;
			ReadCmd = READ_CMD_OPI_MX;
		}
		else {
			Dummy = XLOADER_OSPI_DDR_DUMMY_CYCLES +
				OspiPsvInstance.Extra_DummyCycle;
		}
	}
	else {
		if (OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) {
			Dummy = OspiPsvInstance.Extra_DummyCycle;
			ReadCmd = READ_CMD_4B;
			Proto = XOSPIPSV_READ_1_1_1;
		}
		else {
			Dummy = XLOADER_OSPI_SDR_DUMMY_CYCLES +
				OspiPsvInstance.Extra_DummyCycle;
			Proto = XOSPIPSV_READ_1_1_8;
		}
	}

	FlashMsg.Opcode = ReadCmd;
	FlashMsg.Proto = Proto;
	FlashMsg.Dummy = Dummy;
	if (OspiPsvInstance.DualByteOpcodeEn != 0U) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}

	/**
	 * - Start the DMA Transfer operation if flag is set to non blocking copy.
	*/
	if (Flags == XPLMI_DEVICE_COPY_STATE_INITIATE) {
		Status = (int)XOspiPsv_StartDmaTransfer(&OspiPsvInstance, &FlashMsg);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_READ, Status);
		}
		goto END1;
	}

	/**
	 * - Otherwise start the transfer on the bus in polled mode.
	*/
	Status = (int)XOspiPsv_PollTransfer(&OspiPsvInstance, &FlashMsg);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_READ, Status);
		goto END1;
	}

	/**
	 * - If the OSPI connection is stacked mode, then select the appropriate
	 * flash, read the command and Start the DMA Transfer operation if flag is
	 * set to non blocking copy. Otherwise start the transfer on the bus in
	 * polled mode.
	*/
	if (OspiMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
		if (TrfLen < Length) {
			/*
			 * This code executes when part of image is on flash 0 and the rest
			 * is on flash 1.
			 */
			ChipSelect = XOSPIPSV_SELECT_FLASH_CS1;
			Status = (int)XOspiPsv_SelectFlash(&OspiPsvInstance, ChipSelect);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(
							XLOADER_ERR_OSPI_SEL_FLASH_CS1, Status);
				goto END;
			}

			DestAddr = DestAddr + TrfLen;
			TrfLen = Length - TrfLen;
			SrcAddrLow = 0U;

			/*
			 * Read cmd
			 */
			FlashMsg.Opcode = ReadCmd;
			if (OspiPsvInstance.DualByteOpcodeEn != 0U) {
				FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
			}
			FlashMsg.Addrsize = XLOADER_OSPI_READ_ADDR_SIZE;
			FlashMsg.Addrvalid = TRUE;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.ByteCount = TrfLen;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
			FlashMsg.Addr = SrcAddrLow;
			FlashMsg.Proto = Proto;
			FlashMsg.Dummy = Dummy;

			if ((DestAddr >> 32U) == 0U) {
				FlashMsg.RxBfrPtr = (u8*)(UINTPTR)DestAddr;
			}
			else {
				FlashMsg.RxBfrPtr = NULL;
				FlashMsg.RxAddr64bit = DestAddr;
				FlashMsg.Xfer64bit = 1U;
			}

			/* Check to call blocking or non-blocking DMA */
			if (FlagsTmp == XPLMI_DEVICE_COPY_STATE_INITIATE) {
				 /* Non-Blocking DMA transfer */
				Status = (int)XOspiPsv_StartDmaTransfer(&OspiPsvInstance, &FlashMsg);
				if (Status != XST_SUCCESS) {
					Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_READ, Status);
				}
				goto END1;
			}

			/* Blocking DMA transfer */
			Status = (int)XOspiPsv_PollTransfer(&OspiPsvInstance, &FlashMsg);
			if (Status != XST_SUCCESS) {
				Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_READ, Status);
			}

		}
	}

END1:
#ifdef	PLM_PRINT_PERF_DMA
	XPlmi_MeasurePerfTime(OspiCopyTime, &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_PERF,
	" %u.%03u ms OSPI Copy time: SrcAddr: 0x%08x, DestAddr: 0x%0x08x,"
	"%u Bytes, Flags: 0x%0x\n\r",
	(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac,
	SrcAddrLow, (u32)(DestAddr >> 32U), (u32)DestAddr, Length, Flags);
#endif
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This API enters the flash device into 4 bytes addressing mode.
 * 			As per the Micron spec, before issuing the command to enter into
 * 			4 byte addr mode, a write enable command is issued.
 *
 * @param	OspiPtr is a pointer to the OSPIPSV driver component to use.
 * @param	Enable is a either 1 or 0 if 1 then enters 4 byte if 0 exits.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_OSPI_4BMODE on OSPI driver unable to
 * 			enter/exit 4B mode.
 *
******************************************************************************/
static int XLoader_FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, u32 Enable)
{
	int Status = XST_FAILURE;
	u8 Command;
	u8 FlashStatus = 0U;
	XOspiPsv_Msg FlashMsg = {0U};

	if (OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) {
		Status = XST_SUCCESS;
		goto END;
	}

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
	Status = (int)XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_4BMODE, Status);
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
	Status = (int)XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_4BMODE, Status);
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
			FlashMsg.Dummy += XLOADER_OSPI_SDR_DUMMY_CYCLES;
		}
		Status = (int)XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_4BMODE,
						Status);
			goto END;
		}
		if ((FlashStatus & XLOADER_OSPI_WRITE_DONE_MASK) ==
			XLOADER_OSPI_WRITE_DONE_MASK) {
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

	Status = (int)XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_OSPI_4BMODE, Status);
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
* @return
* 			- XST_SUCCESS on success and error code on failure
*
******************************************************************************/
static int XLoader_FlashSetDDRMode(XOspiPsv *OspiPsvPtr)
{
	int Status = XST_FAILURE;
	u8 ConfigReg[2U] __attribute__ ((aligned(4U)));
	u8 Data[2U] __attribute__ ((aligned(4U))) = {XLOADER_WRITE_CFG_REG_VAL,
		XLOADER_WRITE_CFG_REG_VAL};
	u8 MacronixData[2U] __attribute__ ((aligned(4U))) = {
		XLOADER_MACRONIX_WRITE_CFG_REG_VAL,
		XLOADER_MACRONIX_WRITE_CFG_REG_VAL};
	XOspiPsv_Msg FlashMsg = {0U};
	u8 WriteRegOpcode = WRITE_CONFIG_REG;
	u8 ReadRegOpcode = READ_CONFIG_REG;
	u8 AddrSize = XLOADER_OSPI_WRITE_CFG_REG_CMD_ADDR_SIZE;
	u8 Dummy = XLOADER_OSPI_SDR_DUMMY_CYCLES;
	u8* TxData = &Data[0U];

	if (OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) {
		WriteRegOpcode = WRITE_CONFIG2_REG_MX;
		ReadRegOpcode = READ_CONFIG2_REG_MX;
		AddrSize = XLOADER_MACRONIX_OSPI_WRITE_CFG_REG_CMD_ADDR_SIZE;
		Dummy = XLOADER_MACRONIX_OSPI_SET_DDR_DUMMY_CYCLES;
		TxData = &MacronixData[0U];
	}
	/** Write enable command */
	FlashMsg.Opcode = OSPI_WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0U;
	FlashMsg.Addrvalid = FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0U;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;

	if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}

	Status = (int)XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Write Config register */
	FlashMsg.Opcode = WriteRegOpcode;
	FlashMsg.Addrvalid = TRUE;
	FlashMsg.Addrsize = AddrSize;
	FlashMsg.Addr = 0U;
	FlashMsg.ByteCount = XLOADER_OSPI_WRITE_CFG_REG_CMD_BYTE_CNT;
	FlashMsg.TxBfrPtr = TxData;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = FALSE;
	FlashMsg.Proto = 0U;
	if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}

	Status = (int)XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) &&
		(OspiPsvPtr->DualByteOpcodeEn == 0U)) {
		Status = (int)XOspiPsv_ConfigDualByteOpcode(OspiPsvPtr,
			(u32)XOSPIPSV_DUAL_BYTE_OP_ENABLE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = (int)XOspiPsv_SetSdrDdrMode(OspiPsvPtr,
		XOSPIPSV_EDGE_MODE_DDR_PHY);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Read Configuration register */
	FlashMsg.Opcode = ReadRegOpcode;
	FlashMsg.Addrsize = XLOADER_OSPI_READ_CFG_REG_CMD_ADDR_SIZE;
	FlashMsg.Addr = 0U;
	FlashMsg.Addrvalid = TRUE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ConfigReg;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = Dummy + OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = FALSE;
	FlashMsg.ByteCount = XLOADER_OSPI_READ_CFG_REG_CMD_BYTE_CNT;
	FlashMsg.Proto = XOSPIPSV_READ_8_8_8;

	if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	Status = (int)XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if (ConfigReg[0U] != TxData[0U]) {
		Status = XST_FAILURE;
		goto END;
	}
	XLoader_Printf(DEBUG_GENERAL,"OSPI mode switched to DDR\n\r");

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function releases control of OSPI.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_OspiRelease(void)
{
	int Status = XST_FAILURE;

	/**
	 * - Request the OSPI driver to release the device.
	*/
	Status = XPm_ReleaseDevice(PM_SUBSYS_PMC, PM_DEV_OSPI,
		XPLMI_CMD_SECURE);

	return Status;
}
#endif
