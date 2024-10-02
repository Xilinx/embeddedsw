/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_qspi.c
 *
 * This is the file which contains qspi related code for the PLM.
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
 * @addtogroup qspi_apis QSPI APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplm_qspi.h"
#include "xplm_dma.h"
#include "xplm_ospi.h"
#include "xplm_debug.h"
#include "xplm_hooks.h"
#include "xplm_hw.h"
#include "xospipsv.h"

/************************** Constant Definitions *****************************/
/** @cond spartanup_plm_internal */
#define XPLM_FLASH_SIZE_ID_64M		(0x17U)
#define XPLM_FLASH_SIZE_ID_128M		(0x18U)
#define XPLM_FLASH_SIZE_ID_256M		(0x19U)
#define XPLM_FLASH_SIZE_ID_512M		(0x20U)
#define XPLM_FLASH_SIZE_ID_1G		(0x21U)
#define XPLM_FLASH_SIZE_ID_2G		(0x22U)
#define XPLM_FLASH_SIZE_64M		(0x0800000U)
#define XPLM_FLASH_SIZE_128M		(0x1000000U)
#define XPLM_FLASH_SIZE_256M		(0x2000000U)
#define XPLM_FLASH_SIZE_512M		(0x4000000U)
#define XPLM_FLASH_SIZE_1G		(0x8000000U)
#define XPLM_FLASH_SIZE_2G		(0x10000000U)

#define XPLM_OSPI_TAP_GRAN_SEL_DEFAULT	(100000000U)
/** @endcond */
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static u32 FlashReadID(XOspiPsv *OspiPsvPtr);
static u32 XPlm_QspiGetBusWidth(void);

/************************** Variable Definitions *****************************/
static XOspiPsv OspiPsvInstance; /**< QSPI instance */
static u32 QspiFlashSize = 0U; /**< QSPI flash supported size */
static u32 QspiFlashMake = 0U; /**< QSPI flash manufacturer */
static u32 ReadCommand = 0U; /**< QSPI flash read command to use */
static XPlm_BootModes QspiBootMode; /**< QSPI 24/32 boot mode */
static u8 QspiProto; /**< QSPI protocol to use */

/*****************************************************************************/
/**
 * @brief	This function reads serial FLASH ID connected to the SPI
 *		interface. It then deduces the make and size of the flash and
 *		obtains the connection mode to point to corresponding parameters
 *		in the flash configuration table.
 *
 * @param	OspiPsvPtr is the pointer to QSPI instance
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 FlashReadID(XOspiPsv *OspiPsvPtr)
{
	u32 Status = (u32)XST_FAILURE;
	XOspiPsv_Msg FlashMsg = {0U};
	u8 ReadBuffer[4U] __attribute__ ((aligned(32U)));
	u32 Index;
	u32 TempVal;

	/** - Read flash ID. */
	FlashMsg.Opcode = XPLM_READ_ID_CMD;
	FlashMsg.Addrsize = 0U;
	FlashMsg.Addrvalid = FALSE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ReadBuffer;
	FlashMsg.ByteCount = XPLM_READ_ID_CMD_RX_BYTE_CNT;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_QSPI_READ_ID;
		goto END;
	}

	XPlm_Printf(DEBUG_GENERAL, "FlashID=0x%x 0x%x 0x%x\n\r", ReadBuffer[0U], ReadBuffer[1U],
		    ReadBuffer[2U]);

	/**
	 * - Deduce flash make.
	 */
	if ((ReadBuffer[0U] == XPLM_MICRON_ID) || (ReadBuffer[0U] == XPLM_MICRON_QEMU_ID)) {
		QspiFlashMake = XPLM_MICRON_ID;
		XPlm_Printf(DEBUG_INFO, "MICRON ");
	} else if (ReadBuffer[0U] == XPLM_SPANSION_ID) {
		QspiFlashMake = XPLM_SPANSION_ID;
		XPlm_Printf(DEBUG_INFO, "SPANSION ");
	} else if (ReadBuffer[0U] == XPLM_WINBOND_ID) {
		QspiFlashMake = XPLM_WINBOND_ID;
		XPlm_Printf(DEBUG_INFO, "WINBOND ");
	} else if (ReadBuffer[0U] == XPLM_MACRONIX_ID) {
		QspiFlashMake = XPLM_MACRONIX_ID;
		XPlm_Printf(DEBUG_INFO, "MACRONIX ");
	} else if (ReadBuffer[0U] == XPLM_ISSI_ID) {
		QspiFlashMake = XPLM_ISSI_ID;
		XPlm_Printf(DEBUG_INFO, "ISSI ");
	} else {
		Status = (u32)XPLM_ERR_QSPI_FLASH_MAKE_NOT_SUPPORTED;
		goto END;
	}

	/**
	 * - Deduce flash Size.
	 */
	if (ReadBuffer[2U] == XPLM_FLASH_SIZE_ID_64M) {
		QspiFlashSize = XPLM_FLASH_SIZE_64M;
		XPlm_Printf(DEBUG_INFO, "64M Bits\r\n");
	} else if ((ReadBuffer[2U] == XPLM_FLASH_SIZE_ID_128M)
		   || (ReadBuffer[2U] == XPLM_MACRONIX_FLASH_1_8_V_SIZE_ID_128M)) {
		QspiFlashSize = XPLM_FLASH_SIZE_128M;
		XPlm_Printf(DEBUG_INFO, "128M Bits\r\n");
	} else if (ReadBuffer[2U] == XPLM_FLASH_SIZE_ID_256M) {
		QspiFlashSize = XPLM_FLASH_SIZE_256M;
		XPlm_Printf(DEBUG_INFO, "256M Bits\r\n");
	} else if ((ReadBuffer[2U] == XPLM_FLASH_SIZE_ID_512M)
		   || (ReadBuffer[2U] == XPLM_MACRONIX_FLASH_SIZE_ID_512M)
		   || (ReadBuffer[2U] == XPLM_MACRONIX_FLASH_1_8_V_SIZE_ID_512M)) {
		QspiFlashSize = XPLM_FLASH_SIZE_512M;
		XPlm_Printf(DEBUG_INFO, "512M Bits\r\n");
	} else if ((ReadBuffer[2U] == XPLM_FLASH_SIZE_ID_1G)
		   || (ReadBuffer[2U] == XPLM_MACRONIX_FLASH_SIZE_ID_1G)
		   || (ReadBuffer[2U] == XPLM_MACRONIX_FLASH_1_8_V_SIZE_ID_1G)) {
		QspiFlashSize = XPLM_FLASH_SIZE_1G;
		XPlm_Printf(DEBUG_INFO, "1G Bits\r\n");
	} else if ((ReadBuffer[2U] == XPLM_FLASH_SIZE_ID_2G)
		   || (ReadBuffer[2U] == XPLM_MACRONIX_FLASH_SIZE_ID_2G)
		   || (ReadBuffer[2U] == XPLM_MACRONIX_FLASH_1_8_V_SIZE_ID_2G)) {
		QspiFlashSize = XPLM_FLASH_SIZE_2G;
		XPlm_Printf(DEBUG_INFO, "2G Bits\r\n");
	} else {
		Status = (u32)XPLM_ERR_QSPI_FLASH_SIZE_NOT_SUPPORTED;
		goto END;
	}

	/**
	 * - Store device ID in QSPI instance.
	 */
	OspiPsvPtr->DeviceIdData = 0U;
	for (Index = 0U, TempVal = 0U; Index < 4U; ++Index, TempVal += 4U) {
		OspiPsvPtr->DeviceIdData |=
			(ReadBuffer[Index] << TempVal);
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to initialize the qspi controller and driver.
 *
 * @param	Mode is used to validate if it is either @ref XPLM_BOOT_MODE_QSPI24
 * or XPLM_BOOT_MODE_QSPI32.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
u32 XPlm_QspiInit(XPlm_BootModes Mode)
{
	u32 Status = (u32)XST_FAILURE;
	XOspiPsv_Config *OspiConfig;
	u32 RtcaCfg;

	QspiBootMode = Mode;
	if (Mode == XPLM_BOOT_MODE_QSPI24) {
		XPlm_Printf(DEBUG_INFO, "XPLM_BOOT_MODE_QSPI24\r\n");
	} else if (Mode == XPLM_BOOT_MODE_QSPI32) {
		XPlm_Printf(DEBUG_INFO, "XPLM_BOOT_MODE_QSPI32\r\n");
	} else {
		XPlm_Printf(DEBUG_INFO, "unknown boot mode\r\n");
		goto END;
	}

	/** - Zeroize QSPI instance. */
	memset(&OspiPsvInstance, 0, sizeof(OspiPsvInstance));

	OspiConfig = XOspiPsv_LookupConfig(XPLM_OSPI_DEVICE);
	if (NULL == OspiConfig) {
		Status = (u32)XPLM_ERR_QSPI_CFG_NOT_FOUND;
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
		Status = (u32)XPLM_ERR_QSPI_CFG_INIT;
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
	(void)XOspiPsv_SetClkPrescaler(&OspiPsvInstance, XOSPIPSV_CLK_PRESCALE_4);
	Status = XOspiPsv_SelectFlash(&OspiPsvInstance, XOSPIPSV_SELECT_FLASH_CS0);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_QSPI_FLASH_CS;
		goto END;
	}

	/** - Read and update flash ID. */
	Status = (u32)FlashReadID(&OspiPsvInstance);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	if (OspiPsvInstance.Config.ConnectionMode != XOSPIPSV_CONNECTION_MODE_SINGLE) {
		Status = (u32)XPLM_ERR_QSPI_SINGLE_CONN_MODE;
		goto END;
	}

	if ((RtcaCfg & XPLM_RTCFG_OSPI_XDR_MODE_MASK) == XPLM_OQSPI_DDR_MODE) {
		Status = (u32)XPLM_ERR_RTCA_QSPI_INVLD_DDR_CFG;
		goto END;
	}

	if ((RtcaCfg & XPLM_RTCFG_OSPI_PHY_MODE_MASK) == XPLM_RTCFG_OSPI_PHY_MODE_MASK) {
		Status = XOspiPsv_SetSdrDdrMode(&OspiPsvInstance, XOSPIPSV_EDGE_MODE_SDR_PHY);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XPLM_ERR_OSPI_SET_SDR_PHY;
			goto END;
		}
	}

	Status = XPlm_QspiGetBusWidth();

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to set the read command to 24/32 bit and qspi protocol.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XPLM_ERR_QSPI_BUS_WIDTH_NOT_SUPPORTED if bus width not supported.
 *
 *****************************************************************************/
static u32 XPlm_QspiGetBusWidth(void)
{
	u32 Status = (u32)XPLM_ERR_QSPI_BUS_WIDTH_NOT_SUPPORTED;
	XRomBootRom *InstancePtr = HooksTbl->InstancePtr;

	XPlm_Printf(DEBUG_INFO, "ReadID: 0x%x\n\r", InstancePtr->FlashOpcode);
	ReadCommand = InstancePtr->FlashOpcode;

	switch (ReadCommand) {
		case 0x3U:
			ReadCommand = XPLM_FAST_READ_CMD_24BIT;
			QspiProto = XOSPIPSV_READ_1_1_1;
			break;
		case 0x13U:
			ReadCommand = XPLM_FAST_READ_CMD_32BIT;
			QspiProto = XOSPIPSV_READ_1_1_1;
			break;
		case XPLM_DUAL_READ_CMD_24BIT:
		case XPLM_DUAL_READ_CMD_32BIT:
			QspiProto = XOSPIPSV_READ_1_1_2;
			break;
		case XPLM_QUAD_READ_CMD_24BIT:
		case XPLM_QUAD_READ_CMD_32BIT:
			QspiProto = XOSPIPSV_READ_1_1_4;
			break;
		default:
			goto END;
			break;
	}

	Status = (u32)XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This functions selects the flash bank.
 *
 * @param	BankSel is the bank to be selected in the flash device(s).
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 SendBankSelect(u32 BankSel)
{
	u32 Status =  XST_FAILURE;
	XOspiPsv_Msg FlashMsg = {0U};
	u8 ReadBuffer[10U] __attribute__ ((aligned(32U))) = {0U};
	u8 WriteBuffer[10U] __attribute__ ((aligned(32U)));

	/*
	 * Bank select commands for Micron and Spansion are different.
	 * Macronix bank select is same as Micron.
	 */
	/**
	 * - Send "WREN" command first for Micron/Macronix flash devices, not required for Spansion.
	 * - Send the Extended address register and verify it.
	 */
	if ((QspiFlashMake == XPLM_MICRON_ID) || (QspiFlashMake == XPLM_MACRONIX_ID)) {
		/* For Micro/Macronix flash devices, "WREN" command should be sent first. */
		FlashMsg.Proto = XOSPIPSV_WRITE_1_1_1;
		FlashMsg.Opcode = XPLM_WRITE_ENABLE_CMD;
		FlashMsg.Addrsize = 0U;
		FlashMsg.Addrvalid = FALSE;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0U;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = FALSE;
		Status = XOspiPsv_PollTransfer(&OspiPsvInstance, &FlashMsg);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XPLM_ERR_QSPI_BANK_SEL_SEND_WREN;
			XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%08x\r\n", Status);
			goto END;
		}

		/* Send the Extended address register write command. */
		WriteBuffer[0U] = (u8)BankSel;

		FlashMsg.Proto = XOSPIPSV_WRITE_1_1_1;
		FlashMsg.Opcode = XPLM_EXTADD_REG_WR_CMD;
		FlashMsg.Addrsize = 0U;
		FlashMsg.Addrvalid = FALSE;
		FlashMsg.TxBfrPtr = WriteBuffer;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 1U;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = FALSE;
		Status = XOspiPsv_PollTransfer(&OspiPsvInstance, &FlashMsg);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XPLM_ERR_QSPI_BANK_SEL_SEND_EXT_ADDR;
			XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%08x\r\n", Status);
			goto END;
		}

		/* Verify extended address register read command. */
		FlashMsg.Proto = XOSPIPSV_WRITE_1_1_1;
		FlashMsg.Opcode = XPLM_EXTADD_REG_RD_CMD;
		FlashMsg.Addrsize = 0U;
		FlashMsg.Addrvalid = FALSE;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = ReadBuffer;
		FlashMsg.ByteCount = 1U;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
		FlashMsg.IsDDROpCode = FALSE;
		Status = XOspiPsv_PollTransfer(&OspiPsvInstance, &FlashMsg);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XPLM_ERR_QSPI_BANK_SEL_VERIFY_EXT_ADDR;
			XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%08x\r\n", Status);
			goto END;
		}
	} else if (QspiFlashMake == XPLM_SPANSION_ID) {
		/* Send the Extended address register write command. */
		WriteBuffer[0U] = (u8)BankSel;

		FlashMsg.Proto = XOSPIPSV_WRITE_1_1_1;
		FlashMsg.Opcode = XPLM_BANK_REG_WR_CMD;
		FlashMsg.Addrsize = 0U;
		FlashMsg.Addrvalid = FALSE;
		FlashMsg.TxBfrPtr = WriteBuffer;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 1U;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = FALSE;
		Status = XOspiPsv_PollTransfer(&OspiPsvInstance, &FlashMsg);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XPLM_ERR_QSPI_BANK_SEL_SEND_EXT_ADDR;
			XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%08x\r\n", Status);
			goto END;
		}

		/* Verify extended address register read command. */
		FlashMsg.Proto = XOSPIPSV_WRITE_1_1_1;
		FlashMsg.Opcode = XPLM_BANK_REG_RD_CMD;
		FlashMsg.Addrsize = 0U;
		FlashMsg.Addrvalid = FALSE;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = ReadBuffer;
		FlashMsg.ByteCount = 1U;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
		FlashMsg.IsDDROpCode = FALSE;
		Status = XOspiPsv_PollTransfer(&OspiPsvInstance, &FlashMsg);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XPLM_ERR_QSPI_BANK_SEL_VERIFY_EXT_ADDR;
			XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%08x\r\n", Status);
			goto END;
		}

	} else {
		/* MISRA-C compliance */
		Status = XST_SUCCESS;
		goto END;
	}

	if (ReadBuffer[0U] != BankSel) {
		XPlm_Printf(DEBUG_INFO, "Bank Select %u != Register Read %u\n\r", BankSel, ReadBuffer[0U]);
		Status = (u32)XPLM_ERR_QSPI_BANK_SEL;
		XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%08x\r\n", Status);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is used to copy the data from QSPI flash to destination address.
 *
 * @param	SrcAddr is the address of the QSPI flash where copy should start from.
 * @param	DestAddr is the address of the destination where it should copy to.
 * @param	Length is the length of the bytes to be copied
 * @param	Flags not used
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
u32 XPlm_QspiCopy(u64 SrcAddr, u32 DestAddr, u32 Length, u32 Flags)
{
	u32 Status = (u32)XST_FAILURE;
	u32 OrigAddr;
	u32 BankSel;
	u32 RemainingBytes;
	u32 TransferBytes;
	u32 BankSize;
	u64 BankMask;
	u32 SrcAddrLow = (u32)SrcAddr;
	XOspiPsv_Msg FlashMsg = {0U};
	u64 DestOffset = 0U;

	XPlm_Printf(DEBUG_INFO, "QSPI Reading Src 0x%08x, Dest 0x%08x, Length 0x%08x, Flags 0x%0x\r\n",
		    SrcAddrLow, (u32)DestAddr, Length, Flags);

	/** - Validate the flash size with the length to be copied. */
	if ((SrcAddrLow + Length) > QspiFlashSize) {
		Status = (u32)XPLM_ERR_QSPI_COPY_LENGTH_OVERFLOW;
		XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%08x\r\n", Status);
		goto END;
	}

	if (QspiBootMode == XPLM_BOOT_MODE_QSPI32) {
		FlashMsg.Addrsize = 4U;
	} else {
		FlashMsg.Addrsize = 3U;

	}
	FlashMsg.Addrvalid = TRUE;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Addr = SrcAddrLow;
	FlashMsg.Opcode = ReadCommand;
	FlashMsg.Proto = QspiProto;
	FlashMsg.Dummy = XPLM_DUMMY_CLOCKS + OspiPsvInstance.Extra_DummyCycle;

	if (QspiFlashMake == XPLM_WINBOND_ID) {
		BankSize = XPLM_WINBOND_BANKSIZE;
		BankMask = XPLM_WINBOND_BANKMASK;
	} else {
		BankSize = XPLM_BANKSIZE;
		BankMask = XPLM_BANKMASK;
	}

	/** - Start the copy in polled mode. */
	RemainingBytes = Length;
	while (RemainingBytes > 0U) {
		if (RemainingBytes > XPLM_DMA_DATA_TRAN_SIZE) {
			TransferBytes = XPLM_DMA_DATA_TRAN_SIZE;
		} else {
			TransferBytes = RemainingBytes;
		}

		if ((QspiBootMode == XPLM_BOOT_MODE_QSPI24) ||
		    (QspiFlashMake == XPLM_WINBOND_ID)) {

			OrigAddr = SrcAddrLow;

			/* Select bank check logic for DualQspi. */
			if (QspiFlashSize > BankSize) {
				if (QspiFlashMake == XPLM_WINBOND_ID) {
					BankSel = SrcAddrLow / XPLM_WINBOND_BANKSIZE;
				} else {
					BankSel = SrcAddrLow / XPLM_BANKSIZE;
				}
				Status = SendBankSelect(BankSel);
				if (Status != (u32)XST_SUCCESS) {
					Status = (u32)XPLM_ERR_QSPI_READ_BANK_SEL;
					XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%08x\r\n", Status);
					goto END;
				}
			}

			/*
			 * If data to be read spans beyond the current bank, then
			 * calculate Transfer Bytes in current bank. Else
			 * transfer bytes are same.
			 */
			if ((OrigAddr & BankMask) != (((u64)OrigAddr + TransferBytes) & BankMask)) {
				TransferBytes = (u32)((OrigAddr & BankMask) + BankSize - OrigAddr);
			}
		}

		XPlm_Printf(DEBUG_DETAILED, "QSPI Read Src 0x%08x, Dest 0x%08x, Length %08x\r\n", SrcAddrLow,
			    (u32)(DestAddr + DestOffset), TransferBytes);

		FlashMsg.RxBfrPtr = (u8 *)(UINTPTR)(DestAddr + DestOffset);
		FlashMsg.Opcode = (u8)ReadCommand;
		FlashMsg.Addr = SrcAddrLow;
		FlashMsg.ByteCount = TransferBytes;

		Status = XOspiPsv_PollTransfer(&OspiPsvInstance, &FlashMsg);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XPLM_ERR_QSPI_READ;
			XPlm_Printf(DEBUG_INFO, "PLM ERR: 0x%08x\r\n", Status);
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
	return Status;
}

/** @} end of qspi_apis group*/
