/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_ospi.c
 *
 * This is the file which contains ospi related code for the PLM.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/

/**
 * @addtogroup ospi_apis OSPI APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplm_ospi.h"
#include "xparameters.h"
#include "xplm_status.h"
#include "xplm_debug.h"
#include "xil_io.h"
#include "xplm_hooks.h"
#include "xplm_hw.h"
#include "xospipsv.h"		/* OSPIPSV device driver */
/************************** Constant Definitions *****************************/
/** @cond spartanup_plm_internal */
#define XPLM_OSPI_TAP_GRAN_SEL_DEFAULT	(100000000U)
/** @endcond */
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static u32 FlashReadID(XOspiPsv *OspiPsvPtr);
static u32 XPlm_FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, u32 Enable);
static u32 XPlm_FlashSetDDRMode(XOspiPsv *OspiPsvPtr);

/************************** Variable Definitions *****************************/
static XOspiPsv OspiPsvInstance; /**< OSPI instance. */
static u8 OspiFlashMake; /**< OSPI flash manufacturer. */
static u32 OspiFlashSize = 0U; /**< OSPI flash supported size. */

/*****************************************************************************/
/**
 * @brief	This function reads serial FLASH ID connected to the SPI interface.
 *		It then deduces the make and size of the flash and obtains the
 *		connection mode to point to corresponding parameters in the flash
 *		configuration table.
 *
 * @param	OspiPsvPtr is the pointer to the OSPI instance.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XPLM_ERR_UNSUPPORTED_OSPI on unsupported OSPI flash.
 * 		- XPLM_ERR_UNSUPPORTED_OSPI_SIZE on unsupported OSPI flash size.
 *
 *****************************************************************************/
static u32 FlashReadID(XOspiPsv *OspiPsvPtr)
{
	u32 Status =  XST_FAILURE;
	u32 Index;
	u8 ReadBuffer[XPLM_READ_ID_BYTES] __attribute__ ((aligned(32U)));
	XOspiPsv_Msg FlashMsg = {0U};
	u32 TempVal;

	/** - Read flash ID. */
	FlashMsg.Opcode = READ_ID;
	FlashMsg.Addrsize = 0U;
	FlashMsg.Addrvalid = (u8)FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ReadBuffer;
	FlashMsg.ByteCount = XPLM_READ_ID_BYTES;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_OSPI_READ_ID;
		goto END;
	}

	XPlm_Printf(DEBUG_PRINT_ALWAYS, "FlashID=0x%x 0x%x 0x%x\n\r",
		    ReadBuffer[0U], ReadBuffer[1U], ReadBuffer[2U]);

	/**
	 * - Deduce flash make.
	 */
	if (ReadBuffer[0U] == MICRON_OCTAL_ID_BYTE0) {
		OspiFlashMake = MICRON_OCTAL_ID_BYTE0;
		XPlm_Printf(DEBUG_PRINT_ALWAYS, "MICRON\n\r");
	} else if (ReadBuffer[0U] == GIGADEVICE_OCTAL_ID_BYTE0) {
		OspiFlashMake = GIGADEVICE_OCTAL_ID_BYTE0;
		XPlm_Printf(DEBUG_PRINT_ALWAYS, "GIGADEVICE\n\r");
	} else if (ReadBuffer[0U] == ISSI_OCTAL_ID_BYTE0) {
		OspiFlashMake = ISSI_OCTAL_ID_BYTE0;
		XPlm_Printf(DEBUG_PRINT_ALWAYS, "ISSI\n\r");
	} else if (ReadBuffer[0U] == MACRONIX_OCTAL_ID_BYTE0) {
		OspiFlashMake = MACRONIX_OCTAL_ID_BYTE0;
		XPlm_Printf(DEBUG_PRINT_ALWAYS, "MACRONIX\n\r");
	} else {
		Status = (u32)XPLM_ERR_UNSUPPORTED_OSPI_FLASH_MAKE;
		goto END;
	}

	/**
	 * - Deduce flash Size.
	 */
	OspiFlashSize = 0U;
	if (OspiFlashMake == MICRON_OCTAL_ID_BYTE0) {
		if (ReadBuffer[2U] == MICRON_OCTAL_ID_BYTE2_512) {
			OspiFlashSize = XPLM_FLASH_SIZE_512M;
		} else if (ReadBuffer[2U] == MICRON_OCTAL_ID_BYTE2_1G) {
			OspiFlashSize = XPLM_FLASH_SIZE_1G;
		} else if (ReadBuffer[2U] == MICRON_OCTAL_ID_BYTE2_2G) {
			OspiFlashSize = XPLM_FLASH_SIZE_2G;
		} else {
			/* Do nothing */
		}
	} else if (OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) {
		if (ReadBuffer[2U] == MACRONIX_OCTAL_ID_BYTE2_512) {
			OspiFlashSize = XPLM_FLASH_SIZE_512M;
		}
	} else if (OspiFlashMake == GIGADEVICE_OCTAL_ID_BYTE0) {
		if (ReadBuffer[2U] == GIGADEVICE_OCTAL_ID_BYTE2_256) {
			OspiFlashSize = XPLM_FLASH_SIZE_256M;
		} else if (ReadBuffer[2U] == GIGADEVICE_OCTAL_ID_BYTE2_512) {
			OspiFlashSize = XPLM_FLASH_SIZE_512M;
		} else if (ReadBuffer[2U] == GIGADEVICE_OCTAL_ID_BYTE2_1G) {
			OspiFlashSize = XPLM_FLASH_SIZE_1G;
		} else if (ReadBuffer[2U] == GIGADEVICE_OCTAL_ID_BYTE2_2G) {
			OspiFlashSize = XPLM_FLASH_SIZE_2G;
		} else {
			/* Do nothing */
		}
	} else if (ReadBuffer[2U] == XPLM_FLASH_SIZE_ID_256M) {
		OspiFlashSize = XPLM_FLASH_SIZE_256M;
	} else if (ReadBuffer[2U] == XPLM_FLASH_SIZE_ID_512M) {
		OspiFlashSize = XPLM_FLASH_SIZE_512M;
	} else if (ReadBuffer[2U] == XPLM_FLASH_SIZE_ID_1G) {
		OspiFlashSize = XPLM_FLASH_SIZE_1G;
	} else if (ReadBuffer[2U] == XPLM_FLASH_SIZE_ID_2G) {
		OspiFlashSize = XPLM_FLASH_SIZE_2G;
	} else {
		/* Do nothing */
	}

	if (OspiFlashSize == 0U) {
		Status = (u32)XPLM_ERR_UNSUPPORTED_OSPI_SIZE;
		goto END;
	}
	/**
	 * - Store device ID in OSPI instance.
	 */
	OspiPsvPtr->DeviceIdData = 0U;
	if (OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) {
		for (Index = 0U, TempVal = 0U; Index < 4U; ++Index, TempVal += XPLM_READ_ID_BYTES) {
			OspiPsvPtr->DeviceIdData |= (ReadBuffer[Index >> 1U] << TempVal);
		}
	} else {
		for (Index = 0U, TempVal = 0U; Index < 4U; ++Index, TempVal += XPLM_READ_ID_BYTES) {
			OspiPsvPtr->DeviceIdData |= (ReadBuffer[Index] << TempVal);
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the ospi controller and driver.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XPLM_ERR_OSPI_INIT on OSPI drive initialization fail.
 *		- XPLM_ERR_OSPI_CFG on OSPI configuration fail.
 *		- XPLM_ERR_OSPI_SEL_FLASH_CS0 on failure to select flash CS0
 *		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
u32 XPlm_OspiInit(void)
{
	u32 Status = (u32)XST_FAILURE;
	XOspiPsv_Config *OspiConfig;
	u32 RtcaCfg;

	/** - Zeroize OSPI instance. */
	memset(&OspiPsvInstance, 0, sizeof(OspiPsvInstance));

	OspiConfig = XOspiPsv_LookupConfig(XPAR_XOSPIPSV_0_DEVICE_ID);
	if (NULL == OspiConfig) {
		Status = (u32)XPLM_ERR_OSPI_CFG;
		goto END;
	}

	RtcaCfg = Xil_In32(XPLM_RTCFG_OSPI_CLK_CFG);
	if ((RtcaCfg & XPLM_RTCFG_OSPI_ECO_MASK) == XPLM_RTCFG_OSPI_ECO_MASK) {
		OspiConfig->InputClockHz = XOSPIPSV_TAP_GRAN_SEL_MIN_FREQ;
	} else {
		OspiConfig->InputClockHz = XPLM_OSPI_TAP_GRAN_SEL_DEFAULT;
	}

	/**
	 * - Configure the OSPI driver.
	 */
	Status = XOspiPsv_CfgInitialize(&OspiPsvInstance, OspiConfig);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_OSPI_INIT;
		goto END;
	}

	/**
	 * - Enable SLVERR.
	 */
	XPlm_UtilRMW((OspiConfig->BaseAddress + XOSPIPSV_OSPIDMA_SRC_CTRL),
		     XOSPIPSV_OSPIDMA_SRC_CTRL_APB_ERR_RESP_MASK,
		     XOSPIPSV_OSPIDMA_SRC_CTRL_APB_ERR_RESP_MASK);
	XPlm_UtilRMW((OspiConfig->BaseAddress + XOSPIPSV_OSPIDMA_DST_CTRL),
		     XOSPIPSV_OSPIDMA_DST_CTRL_APB_ERR_RESP_MASK,
		     XOSPIPSV_OSPIDMA_DST_CTRL_APB_ERR_RESP_MASK);

	/**
	 * - Enable IDAC controller in OSPI.
	 */
	XOspiPsv_SetOptions(&OspiPsvInstance, XOSPIPSV_IDAC_EN_OPTION);

	/**
	 * - Set the prescaler for OSPIPSV clock.
	 */
	XOspiPsv_SetClkPrescaler(&OspiPsvInstance, XOSPIPSV_CLK_PRESCALE_4);
	Status = XOspiPsv_SelectFlash(&OspiPsvInstance, XOSPIPSV_SELECT_FLASH_CS0);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_OSPI_SEL_FLASH_CS0;
		goto END;
	}

	/** - Read and update flash ID. */
	Status = FlashReadID(&OspiPsvInstance);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	if (OspiPsvInstance.Config.ConnectionMode != XOSPIPSV_CONNECTION_MODE_SINGLE) {
		Status = (u32)XPLM_ERR_OSPI_SINGLE_CONN_MODE;
		goto END;
	}

	/**
	 * - Set the OSPI controller mode to DDR/SDR based on the RTCA configuration. Error out if
	 * the configuration is DDR NON-PHY.
	 */
	if (((Xil_In32(PMC_TAP_VERSION) & PMC_TAP_VERSION_PLATFORM_MASK) >> PMC_TAP_VERSION_PLATFORM_SHIFT)
	    == PMC_TAP_VERSION_SILICON) {
		if ((RtcaCfg & XPLM_RTCFG_OSPI_PHY_MODE_MASK) == XPLM_RTCFG_OSPI_PHY_MODE_MASK) {
			if ((RtcaCfg & XPLM_RTCFG_OSPI_XDR_MODE_MASK) == XPLM_OQSPI_DDR_MODE) {
				OspiPsvInstance.SdrDdrMode = XOSPIPSV_EDGE_MODE_DDR_PHY;
				Status = XPlm_FlashSetDDRMode(&OspiPsvInstance);
				if (Status != (u32)XST_SUCCESS) {
					goto END;
				}
			} else {
				Status = XOspiPsv_SetSdrDdrMode(&OspiPsvInstance, XOSPIPSV_EDGE_MODE_SDR_PHY);
				if (Status != (u32)XST_SUCCESS) {
					Status = (u32)XPLM_ERR_OSPI_SET_SDR_PHY;
					goto END;
				}
			}
		} else {
			if ((RtcaCfg & XPLM_RTCFG_OSPI_XDR_MODE_MASK) == XPLM_OQSPI_DDR_MODE) {
				Status = (u32)XPLM_ERR_RTCA_OSPI_INVLD_DDR_PHY_CFG;
				goto END;
			}
			XOspiPsv_SetClkPrescaler(&OspiPsvInstance, XOSPIPSV_CLK_PRESCALE_4);
		}
	}

	/** - Set the addressing mode to 4 byte addressing. */
	Status = XPlm_FlashEnterExit4BAddMode(&OspiPsvInstance, TRUE);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to copy the data from OSPI flash to
 *		destination address.
 *
 * @param	SrcAddr is the address of the OSPI flash where copy should start
 * @param	DestAddr is the address of the destination where it should copy to
 * @param	Length is the length of the bytes to be copied
 * @param	Flags not used
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
u32 XPlm_OspiCopy(u64 SrcAddr, u32 DestAddr, u32 Length, u32 Flags)
{
	u32 Status = (u32)XST_FAILURE;
	u32 SrcAddrLow = (u32)SrcAddr;
	XOspiPsv_Msg FlashMsg = {0U};
	u32 TrfLen;
	u8 ReadCmd = READ_CMD_OCTAL_4B;
	u8 Proto;
	u8 Dummy;

	XPlm_Printf(DEBUG_INFO, "OSPI Reading Src 0x%0x, Dest 0x%08x, "
		    "Length 0x%0x, Flags 0x%0x\r\n", SrcAddrLow, DestAddr, Length, Flags);

	/** - Validate the flash size with the length to be copied. */
	if ((SrcAddrLow + Length) > OspiFlashSize) {
		Status = (u32)XPLM_ERR_OSPI_COPY_LENGTH_OVERFLOW;
		XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%08x\r\n", Status);
		goto END;
	}

	/** - Generate the flash read command. */
	TrfLen = Length;
	FlashMsg.Addrsize = XPLM_OSPI_READ_ADDR_SIZE;
	FlashMsg.Addrvalid = TRUE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.ByteCount = TrfLen;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Addr = SrcAddrLow;
	FlashMsg.RxBfrPtr = (u8 *)(UINTPTR)DestAddr;

	if (OspiPsvInstance.SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		Proto = XOSPIPSV_READ_8_8_8;
		if (OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) {
			Dummy = XPLM_MACRONIX_OSPI_DDR_DUMMY_CYCLES +
				OspiPsvInstance.Extra_DummyCycle;
			ReadCmd = READ_CMD_OPI_MX;
		} else {
			Dummy = XPLM_OSPI_DDR_DUMMY_CYCLES +
				OspiPsvInstance.Extra_DummyCycle;
		}
	} else {
		if (OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) {
			Dummy = OspiPsvInstance.Extra_DummyCycle;
			ReadCmd = READ_CMD_4B;
			Proto = XOSPIPSV_READ_1_1_1;
		} else {
			Dummy = XPLM_OSPI_SDR_DUMMY_CYCLES +
				OspiPsvInstance.Extra_DummyCycle;
			Proto = XOSPIPSV_READ_1_1_8;
		}
	}

	FlashMsg.Opcode = ReadCmd;
	FlashMsg.Proto = Proto;
	FlashMsg.Dummy = Dummy;

	if (OspiPsvInstance.DualByteOpcodeEn == XOSPIPSV_DUAL_BYTE_OP_ENABLE) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}

	/** - Start the copy in polled mode. */
	Status = XOspiPsv_PollTransfer(&OspiPsvInstance, &FlashMsg);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_OSPI_READ_DATA;
		XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%08x\r\n", Status);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This API enters the flash device into 4-byte addressing mode.
 *		As per the Micron spec, before issuing the command to enter into
 *		4-byte addr mode, a write enable command is issued.
 *
 * @param	OspiPsvPtr is the pointer to the OSPI instance.
 * @param	Enable 0x1 to enter 4-byte addressing mode, 0x0 to exit 4-byte addressing mode.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- and errors from @ref XPlm_Status_t.
 *
 */
/******************************************************************************/
static u32 XPlm_FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, u32 Enable)
{
	u32 Status = (u32)XST_FAILURE;
	u8 Command;
	u8 FlashStatus = 0U;
	XOspiPsv_Msg FlashMsg = {0U};

	/** - Skip updating addressing mode for Macronix flash. */
	if (OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) {
		Status = (u32)XST_SUCCESS;
		goto END;
	}

	if (Enable == 0x1U) {
		Command = ENTER_4B_ADDR_MODE;
	} else {
		Command = EXIT_4B_ADDR_MODE;
	}

	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	} else {
		FlashMsg.Proto = XOSPIPSV_WRITE_1_1_1;
	}

	/** - Set the flash to write mode. */
	FlashMsg.Opcode = OSPI_WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0U;
	FlashMsg.Addrvalid = (u8)FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0U;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = (u8)FALSE;
	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_OSPI_4B_ADDR_MODE_WRITE_ENABLE;
		goto END;
	}

	/** - Enable/Disable 4 byte addressing mode. */
	FlashMsg.Opcode = Command;
	FlashMsg.Addrvalid = (u8)FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = XPLM_OSPI_ENTER_4B_ADDR_MODE_CMD_BYTE_CNT;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.Addrsize = XPLM_OSPI_ENTER_4B_ADDR_MODE_CMD_ADDR_SIZE;
	FlashMsg.IsDDROpCode = (u8)FALSE;
	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_OSPI_4B_ADDR_MODE_ENTER_OR_EXIT_CMD;
		goto END;
	}

	FlashMsg.Opcode = READ_FLAG_STATUS_CMD;
	FlashMsg.Addrvalid = (u8)FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = &FlashStatus;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Addrsize = XPLM_OSPI_READ_FLAG_STATUS_CMD_ADDR_SIZE;
	FlashMsg.IsDDROpCode = (u8)FALSE;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
		FlashMsg.ByteCount = XPLM_OSPI_DDR_MODE_BYTE_CNT;
	} else {
		FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
		FlashMsg.ByteCount = XPLM_OSPI_DDR_MODE_BYTE_CNT;
		FlashMsg.Proto = XOSPIPSV_READ_1_1_1;
	}

	while (1U) {
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Dummy += XPLM_OSPI_SDR_DUMMY_CYCLES;
		}
		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XPLM_ERR_OSPI_4B_ADDR_MODE_ENTER_OR_EXIT_CMD_STATUS;
			goto END;
		}
		if ((FlashStatus & XPLM_OSPI_WRITE_DONE_MASK) == XPLM_OSPI_WRITE_DONE_MASK) {
			break;
		}
	}

	/** - Disable flash write mode. */
	FlashMsg.Opcode = WRITE_DISABLE_CMD;
	FlashMsg.Addrsize = 0U;
	FlashMsg.Addrvalid = (u8)FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0U;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = (u8)FALSE;

	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	} else {
		FlashMsg.Proto = XOSPIPSV_WRITE_1_1_1;
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_OSPI_4B_ADDR_MODE_WRITE_DISABLE;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets the flash device to Octal DDR mode.
 *
 * @param	OspiPsvPtr is the pointer to the OSPI instance.
 *
 * @return
 *		- XST_SUCCESS on success
 *		- and errors from @ref XPlm_Status_t.
 */
/*****************************************************************************/
static u32 XPlm_FlashSetDDRMode(XOspiPsv *OspiPsvPtr)
{
	u32 Status = (u32)XST_FAILURE;
	u8 ConfigReg[2U] __attribute__ ((aligned(4U)));
	u8 Data[2U] __attribute__ ((aligned(4U))) = {XPLM_WRITE_CFG_REG_VAL,
						     XPLM_WRITE_CFG_REG_VAL
						    };
	u8 MacronixData[2U] __attribute__ ((aligned(4U))) = {
		XPLM_MACRONIX_WRITE_CFG_REG_VAL,
		XPLM_MACRONIX_WRITE_CFG_REG_VAL
	};
	XOspiPsv_Msg FlashMsg = {0U};
	u8 WriteRegOpcode = WRITE_CONFIG_REG;
	u8 ReadRegOpcode = READ_CONFIG_REG;
	u8 AddrSize = XPLM_OSPI_WRITE_CFG_REG_CMD_ADDR_SIZE;
	u8 Dummy = XPLM_OSPI_SDR_DUMMY_CYCLES;
	u8 *TxData = &Data[0U];

	if (OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) {
		WriteRegOpcode = WRITE_CONFIG2_REG_MX;
		ReadRegOpcode = READ_CONFIG2_REG_MX;
		AddrSize = XPLM_MACRONIX_OSPI_WRITE_CFG_REG_CMD_ADDR_SIZE;
		Dummy = XPLM_MACRONIX_OSPI_SET_DDR_DUMMY_CYCLES;
		TxData = &MacronixData[0U];
	}
	/** - Write enable command. */
	FlashMsg.Opcode = OSPI_WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0U;
	FlashMsg.Addrvalid = (u8)FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0U;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;

	if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_OSPI_SET_DDR_WRITE_ENABLE;
		goto END;
	}

	/** - Write Config register. */
	FlashMsg.Opcode = WriteRegOpcode;
	FlashMsg.Addrvalid = (u8)TRUE;
	FlashMsg.Addrsize = AddrSize;
	FlashMsg.Addr = 0U;
	FlashMsg.ByteCount = XPLM_OSPI_WRITE_CFG_REG_CMD_BYTE_CNT;
	FlashMsg.TxBfrPtr = TxData;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = (u8)FALSE;
	FlashMsg.Proto = 0U;
	if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_OSPI_SET_DDR_WRITE_CFG_REG;
		goto END;
	}

	if ((OspiFlashMake == MACRONIX_OCTAL_ID_BYTE0) &&
	    (OspiPsvPtr->DualByteOpcodeEn == 0U)) {
		Status = XOspiPsv_ConfigDualByteOpcode(OspiPsvPtr,
						       (u32)XOSPIPSV_DUAL_BYTE_OP_ENABLE);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XPLM_ERR_OSPI_SET_DDR_DUAL_BYTE_OP_ENABLE;
			goto END;
		}
	}

	Status = XOspiPsv_SetSdrDdrMode(OspiPsvPtr, XOSPIPSV_EDGE_MODE_DDR_PHY);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_OSPI_SET_DDR_PHY;
		goto END;
	}

	/** - Read back configuration register to validate it. */
	FlashMsg.Opcode = ReadRegOpcode;
	FlashMsg.Addrsize = XPLM_OSPI_READ_CFG_REG_CMD_ADDR_SIZE;
	FlashMsg.Addr = 0U;
	FlashMsg.Addrvalid = (u8)TRUE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ConfigReg;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = Dummy + OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = (u8)FALSE;
	FlashMsg.ByteCount = XPLM_OSPI_READ_CFG_REG_CMD_BYTE_CNT;
	FlashMsg.Proto = XOSPIPSV_READ_8_8_8;

	if (OspiPsvPtr->DualByteOpcodeEn != 0U) {
		FlashMsg.ExtendedOpcode = (u8)(~FlashMsg.Opcode);
	}
	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_OSPI_SET_DDR_READ_CFG_REG;
		goto END;
	}
	if (ConfigReg[0U] != TxData[0U]) {
		Status = (u32)XPLM_ERR_OSPI_SET_DDR_CFG_MISMATCH;
		goto END;
	}
	XPlm_Printf(DEBUG_PRINT_ALWAYS, "OSPI mode switched to DDR\n\r");

END:
	return Status;
}

/** @} end of ospi_apis group*/
