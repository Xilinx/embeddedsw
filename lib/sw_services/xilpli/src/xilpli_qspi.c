/******************************************************************************
* Copyright (C) 2018 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xilpli_qspi.c
*
* This is the file which contains qspi related code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xilpli_qspi.h"
#include "xilpli_dma.h"
#include "xilpli.h"
#include "xplmi_hw.h"

#ifdef XILPLI_QSPI

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
static int FlashReadID(XQspiPsu *QspiPsuPtr);

/************************** Variable Definitions *****************************/
static XQspiPsu QspiPsuInstance;
static u32 QspiFlashSize=0U;
static u32 QspiFlashMake=0U;
static u32 ReadCommand=0U;
static XQspiPsu_Msg FlashMsg[5];
static u8 IssiIdFlag=0U;

static u8 TxBfrPtr __attribute__ ((aligned(32)));
static u8 ReadBuffer[10] __attribute__ ((aligned(32)));
static u8 WriteBuffer[10] __attribute__ ((aligned(32)));

/******************************************************************************
*
* This function reads serial FLASH ID connected to the SPI interface.
* It then deduces the make and size of the flash and obtains the
* connection mode to point to corresponding parameters in the flash
* configuration table. The flash driver will function based on this and
* it presently supports Micron and Spansion - 128, 256 and 512Mbit and
* Winbond 128Mbit
*
* @param	none
*
* @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int FlashReadID(XQspiPsu *QspiPsuPtr)
{
	int Status;

	/*
	 * Read ID
	 */
	TxBfrPtr = READ_ID_CMD;
	FlashMsg[0].TxBfrPtr = &TxBfrPtr;
	FlashMsg[0].RxBfrPtr = NULL;
	FlashMsg[0].ByteCount = 1;
	FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

	FlashMsg[1].TxBfrPtr = NULL;
	FlashMsg[1].RxBfrPtr = ReadBuffer;
	FlashMsg[1].ByteCount = 4;
	FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
	FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, &FlashMsg[0], 2);
	if (Status != XILPLI_SUCCESS) {
		goto END;
	}

	XPli_Printf(DEBUG_GENERAL, "FlashID=0x%x 0x%x 0x%x\n\r", ReadBuffer[0],
					ReadBuffer[1], ReadBuffer[2]);

	/*
	 * Deduce flash make
	 */
	if (ReadBuffer[0] == MICRON_ID) {
		QspiFlashMake = MICRON_ID;
		XPli_Printf(DEBUG_INFO, "MICRON ");
	} else if(ReadBuffer[0] == SPANSION_ID) {
		QspiFlashMake = SPANSION_ID;
		XPli_Printf(DEBUG_INFO, "SPANSION ");
	} else if(ReadBuffer[0] == WINBOND_ID) {
		QspiFlashMake = WINBOND_ID;
		XPli_Printf(DEBUG_INFO, "WINBOND ");
	} else if(ReadBuffer[0] == MACRONIX_ID) {
		QspiFlashMake = MACRONIX_ID;
		XPli_Printf(DEBUG_INFO, "MACRONIX ");
	} else if(ReadBuffer[0] == ISSI_ID) {
		QspiFlashMake = ISSI_ID;
		XPli_Printf(DEBUG_INFO, "ISSI ");
	} else {
		Status = XILPLI_ERR_UNSUPPORTED_QSPI;
		XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_UNSUPPORTED_QSPI\r\n");
		goto END;
	}

	/*
	 * Deduce flash Size
	 */
	if (ReadBuffer[2] == FLASH_SIZE_ID_64M) {
		QspiFlashSize = FLASH_SIZE_64M;
		XPli_Printf(DEBUG_INFO, "64M Bits\r\n");
	} else if (ReadBuffer[2] == FLASH_SIZE_ID_128M) {
		QspiFlashSize = FLASH_SIZE_128M;
		XPli_Printf(DEBUG_INFO, "128M Bits\r\n");
	} else if (ReadBuffer[2] == FLASH_SIZE_ID_256M) {
		QspiFlashSize = FLASH_SIZE_256M;
		XPli_Printf(DEBUG_INFO, "256M Bits\r\n");
	} else if ((ReadBuffer[2] == FLASH_SIZE_ID_512M)
			|| (ReadBuffer[2] == MACRONIX_FLASH_SIZE_ID_512M)) {
		QspiFlashSize = FLASH_SIZE_512M;
		XPli_Printf(DEBUG_INFO, "512M Bits\r\n");
	} else if ((ReadBuffer[2] == FLASH_SIZE_ID_1G)
			|| (ReadBuffer[2] == MACRONIX_FLASH_SIZE_ID_1G)) {
		QspiFlashSize = FLASH_SIZE_1G;
		XPli_Printf(DEBUG_INFO, "1G Bits\r\n");
	} else if ((ReadBuffer[2] == FLASH_SIZE_ID_2G)) {
		QspiFlashSize = FLASH_SIZE_2G;
		XPli_Printf(DEBUG_INFO, "2G Bits\r\n");
	}else {
		Status = XILPLI_ERR_UNSUPPORTED_QSPI;
		XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_UNSUPPORTED_QSPI\r\n");
		goto END;
	}
	/* Enable ID flag for ISSI 128M Qspi to enable
	 * DUAL_READ_CMD_24BIT ReadCommand
	 */
	if((QspiFlashMake==ISSI_ID) && (QspiFlashSize==FLASH_SIZE_128M))
	{
		IssiIdFlag=1;
	}
	Status = XILPLI_SUCCESS;
END:
	return Status;
}


/*****************************************************************************/
/**
 * This function is used to initialize the qspi controller and driver
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int XPli_Qspi24Init(u32 DeviceFlags)
{
	XQspiPsu_Config *QspiConfig;
	int Status;
	u32 QspiMode;

	/**
	 * This parameter is required as per the prototype
	 */
	(void) (DeviceFlags);

	/**
	 * Initialize the QSPI driver so that it's ready to use
	 */
	QspiConfig =  XQspiPsu_LookupConfig(QSPI_DEVICE_ID);
	if (NULL == QspiConfig) {
		Status = XILPLI_ERR_QSPI_INIT;
		XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_INIT\r\n");
		goto END;
	}

	Status =  XQspiPsu_CfgInitialize(&QspiPsuInstance, QspiConfig,
			QspiConfig->BaseAddress);
	if (Status != XILPLI_SUCCESS) {
		Status = XILPLI_ERR_QSPI_INIT;
		XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_INIT\r\n");
		goto END;
	}

	/*
	 * Set Manual Start
	 */
	Status = XQspiPsu_SetOptions(&QspiPsuInstance, XQSPIPSU_MANUAL_START_OPTION);

	if (Status != XILPLI_SUCCESS) {
		Status = XILPLI_ERR_QSPI_MANUAL_START;
		XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_MANUAL_START\r\n");
		goto END;
	}
	/*
	 * Set the pre-scaler for QSPI clock
	 */
	Status = XQspiPsu_SetClkPrescaler(&QspiPsuInstance, XQSPIPSU_CLK_PRESCALE_8);

	if (Status != XILPLI_SUCCESS) {
		Status = XILPLI_ERR_QSPI_PRESCALER_CLK;
		XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_PRESCALER_CLK\r\n");
		goto END;
	}
	XQspiPsu_SelectFlash(&QspiPsuInstance,
		XQSPIPSU_SELECT_FLASH_CS_LOWER, XQSPIPSU_SELECT_FLASH_BUS_LOWER);

	/*
	 * Configure the qspi in linear mode if running in XIP
	 * TBD: XIP Support
	 */

	switch ((u32)(XPAR_XQSPIPSU_0_QSPI_MODE)) {

		case XQSPIPSU_CONNECTION_MODE_SINGLE:
		{
			XPli_Printf(DEBUG_INFO,"QSPI is in single flash connection\r\n");
		} break;

		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
		{
			XPli_Printf(DEBUG_INFO,"QSPI is in Dual Parallel connection\r\n");
		} break;

		case XQSPIPSU_CONNECTION_MODE_STACKED:
		{
			XPli_Printf(DEBUG_INFO,"QSPI is in Dual Stack connection\r\n");
		} break;

		default:
		{
			Status = XILPLI_ERR_QSPI_CONNECTION;
			XPli_Printf(DEBUG_GENERAL,
					"XILPLI_ERR_QSPI_CONNECTION\r\n");
			goto END;
		} break;

	}

	/* Read Flash ID and extract Manufacture and Size information */
	Status = FlashReadID(&QspiPsuInstance);
	if (Status != XILPLI_SUCCESS) {
		goto END;
	}

	/*  TODO add code for 1x, 2x and 4x */
	if(IssiIdFlag==1U) {
		ReadCommand = DUAL_READ_CMD_24BIT;
	} else {
		ReadCommand = QUAD_READ_CMD_24BIT;
	}

	/* TODO add code: For a Stacked connection, read second Flash ID */
	QspiMode=(XPAR_XQSPIPSU_0_QSPI_MODE);
	if ((QspiMode ==(XQSPIPSU_CONNECTION_MODE_PARALLEL)) ||
		(QspiMode ==(XQSPIPSU_CONNECTION_MODE_STACKED) )) {
		QspiFlashSize = 2 * QspiFlashSize;
	}

END:
	return Status;
}



/******************************************************************************
*
* This functions translates the address based on the type of interconnection.
* In case of stacked, this function asserts the corresponding slave select.
*
* @param	Addr which is to be accessed
*
* @return	QspiAddr is the actual flash address to be accessed - for single
*			it is unchanged; for stacked, the lower flash size is subtracted;
*			for parallel the address is divided by 2.
*
* @note		None.
*
*
******************************************************************************/
static u32 XPli_GetQspiAddr(u32 Addr )
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
			/*
			 * Select lower or upper Flash based on sector address
			 */
			if(Addr >= (QspiFlashSize/2U)) {
				XQspiPsu_SelectFlash(&QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_UPPER,
						XQSPIPSU_SELECT_FLASH_BUS_LOWER);
				/*
				 * Subtract first flash size when accessing second flash
				 */
				RealAddr = Addr - (QspiFlashSize/2U);
			}else{
				/*
				 * Set selection to L_PAGE
				 */
				XQspiPsu_SelectFlash(&QspiPsuInstance,
						XQSPIPSU_SELECT_FLASH_CS_LOWER,
						XQSPIPSU_SELECT_FLASH_BUS_LOWER);
				RealAddr = Addr;
			}
			break;

		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
			/*
			 * The effective address in each flash is the actual
			 * address / 2
			 */
			XQspiPsu_SelectFlash(&QspiPsuInstance,
			XQSPIPSU_SELECT_FLASH_CS_BOTH, XQSPIPSU_SELECT_FLASH_BUS_BOTH);
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

	return(RealAddr);
}

/******************************************************************************
*
* This functions selects the current bank
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Pointer to the write buffer which contains data to be transmitted
* @param	BankSel is the bank to be selected in the flash device(s).
*
* @return	XST_SUCCESS if bank selected, otherwise XST_FAILURE.
*
* @note		None.
*
*
******************************************************************************/
static int SendBankSelect(u32 BankSel)
{
	int Status;

	/*
	 * bank select commands for Micron and Spansion are different
	 * Macronix bank select is same as Micron
	 */
	if ((QspiFlashMake == MICRON_ID) || (QspiFlashMake == MACRONIX_ID))	{
		/*
		 * For micron command WREN should be sent first
		 * except for some specific feature set
		 */
		TxBfrPtr = WRITE_ENABLE_CMD;
		FlashMsg[0].TxBfrPtr = &TxBfrPtr;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg[0], 1);
		if (Status != XILPLI_SUCCESS) {
			Status = XILPLI_ERR_QSPI_READ;
			XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_READ\r\n");
			goto END;
		}

		/*
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		WriteBuffer[COMMAND_OFST]   = EXTADD_REG_WR_CMD;
		WriteBuffer[ADDR_1_OFST] = (u8)BankSel;

		FlashMsg[0].TxBfrPtr = WriteBuffer;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 2;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg[0], 1);
		if (Status != XILPLI_SUCCESS) {
			Status = XILPLI_ERR_QSPI_READ;
			XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_READ\r\n");
			goto END;
		}
	}

	if (QspiFlashMake == SPANSION_ID) {
		/*
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		WriteBuffer[COMMAND_OFST]   = BANK_REG_WR_CMD;
		WriteBuffer[ADDR_1_OFST] = (u8)BankSel;

		FlashMsg[0].TxBfrPtr = WriteBuffer;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 2;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg[0], 1);
		if (Status != XILPLI_SUCCESS) {
			Status = XILPLI_ERR_QSPI_READ;
			XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_READ\r\n");
			goto END;
		}
	}

	/*
	 * For testing - Read bank to verify
	 */
	if ((QspiFlashMake == MICRON_ID) || (QspiFlashMake == MACRONIX_ID)) {
		/*
		 * Extended address register read command
		 */

		WriteBuffer[COMMAND_OFST]   = EXTADD_REG_RD_CMD;
		FlashMsg[0].TxBfrPtr = WriteBuffer;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1].TxBfrPtr = NULL;
		FlashMsg[1].RxBfrPtr = ReadBuffer;
		FlashMsg[1].ByteCount = 1;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg[0], 2);
		if (Status != XILPLI_SUCCESS) {
			Status = XILPLI_ERR_QSPI_READ;
			XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_READ\r\n");
			goto END;
		}
	}

	if (QspiFlashMake == SPANSION_ID) {
		/*
		 * Bank register read command
		 */
		WriteBuffer[COMMAND_OFST]   = BANK_REG_RD_CMD;
		FlashMsg[0].TxBfrPtr = WriteBuffer;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 1;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		FlashMsg[1].TxBfrPtr = NULL;
		FlashMsg[1].RxBfrPtr = ReadBuffer;
		FlashMsg[1].ByteCount = 1;
		FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[1].Flags = XQSPIPSU_MSG_FLAG_RX;

		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg[0], 2);
		if (Status != XILPLI_SUCCESS) {
			Status = XILPLI_ERR_QSPI_READ;
			XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_READ\r\n");
			goto END;
		}

		if (ReadBuffer[0] != BankSel) {
			XPli_Printf(DEBUG_INFO, "Bank Select %d != Register Read %d\n\r", BankSel,
				ReadBuffer[0]);
			Status = XILPLI_ERR_QSPI_READ;
			XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_READ\r\n");
			goto END;
		}
	}

	/* Winbond can be added here */
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to copy the data from QSPI flash to destination
 * address
 *
 * @param SrcAddr is the address of the QSPI flash where copy should
 * start from
 *
 * @param DestAddr is the address of the destination where it
 * should copy to
 *
 * @param Length Length of the bytes to be copied
 *
 * @return
 *		- XILPLI_SUCCESS for successful copy
 *		- errors as mentioned in xilpli_error.h
 *
 *****************************************************************************/
XStatus XPli_Qspi24Copy(u32 SrcAddr, u64 DestAddr, u32 Length, u32 Flags)
{
	u32 QspiAddr, OrigAddr;
	u32 BankSel;
	u32 RemainingBytes;
	u32 TransferBytes;
	u32 DiscardByteCnt;
	u8 BankSwitchFlag=0U;
	u32 BankSize;
	u32 BankMask;
	int Status;

	XPli_Printf(DEBUG_INFO,"QSPI Reading Src 0x%0lx, Dest 0x%0lx, Length 0x%0lx, Flags 0x%0x\r\n",
			SrcAddr, (u32)DestAddr, Length, Flags);

	/* Set QSPI DMA in AXI FIXED / INCR mode.
	 * Fixed mode is used for CFI loading */
	XPli_SetQspiDmaMode(Flags);


	/**
	 * Check the read length with Qspi flash size
	 */
	if ((SrcAddr + Length) > QspiFlashSize)
	{
		Status = XILPLI_ERR_QSPI_LENGTH;
		XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_LENGTH\r\n");
		goto END;
	}

	/* Multiply bank size & mask in case of Dual Parallel */
	if (QspiPsuInstance.Config.ConnectionMode ==
	    XQSPIPSU_CONNECTION_MODE_PARALLEL){
		BankSize =  SINGLEBANKSIZE * 2U;
		BankMask =  SINGLEBANKMASK * 2U;
	} else {
		BankSize = SINGLEBANKSIZE;
		BankMask = SINGLEBANKMASK;
	}

	/**
	 * Update no of bytes to be copied
	 */
	RemainingBytes = Length;

	while(RemainingBytes > 0U) {

		if (RemainingBytes > DMA_DATA_TRAN_SIZE)
		{
			TransferBytes = DMA_DATA_TRAN_SIZE;
		} else {
			TransferBytes = RemainingBytes;
		}

		/**
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		QspiAddr = XPli_GetQspiAddr((u32 )SrcAddr);

		/**
		 * Multiply address by 2 in case of Dual Parallel
		 * This address is used to calculate the bank crossing
		 * condition
		 */
		if (QspiPsuInstance.Config.ConnectionMode ==
		    XQSPIPSU_CONNECTION_MODE_PARALLEL){
			OrigAddr = QspiAddr * 2U;
		} else {
			OrigAddr = QspiAddr;
		}

		/**
		 * Select bank
		 * check logic for DualQspi
		 */
		if(QspiFlashSize > BankSize) {
			BankSel = QspiAddr/BANKSIZE;
			Status = SendBankSelect(BankSel);
			if (Status != XILPLI_SUCCESS) {
				Status = XILPLI_ERR_QSPI_READ;
				XPli_Printf(DEBUG_GENERAL,
					"XILPLI_ERR_QSPI_READ\r\n");
				goto END;
			}
		}


		/**
		 * If data to be read spans beyond the current bank, then
		 * calculate Transfer Bytes in current bank. Else
		 * transfer bytes are same
		 */
		if ((OrigAddr & BankMask) != ((OrigAddr + TransferBytes) & BankMask)) {
			TransferBytes = (OrigAddr & BankMask) + BankSize - OrigAddr;
		}

		//XPli_Printf(DEBUG_INFO,".");
		XPli_Printf(DEBUG_DETAILED,
			"QSPI Read Src 0x%0lx, Dest %0lx, Length %0lx\r\n",
					QspiAddr, (u32)DestAddr, TransferBytes);

		/**
		 * Setup the read command with the specified address
		 * and data for the Flash
		 */

		WriteBuffer[COMMAND_OFST]   = (u8)ReadCommand;
		WriteBuffer[ADDR_1_OFST] = (u8)((QspiAddr & 0xFF0000U) >> 16);
		WriteBuffer[ADDR_2_OFST] = (u8)((QspiAddr & 0xFF00U) >> 8);
		WriteBuffer[ADDR_3_OFST] = (u8)(QspiAddr & 0xFFU);
		DiscardByteCnt = 4;

		FlashMsg[0].TxBfrPtr = WriteBuffer;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = DiscardByteCnt;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		/* It is recommended to have a separate entry for dummy */
		if ((ReadCommand == FAST_READ_CMD_24BIT) ||
		    (ReadCommand == DUAL_READ_CMD_24BIT) ||
		    (ReadCommand == QUAD_READ_CMD_24BIT)) {
			/* Update Dummy cycles as per flash specs for QUAD IO */

			/*
			 * It is recommended that Bus width value during dummy
			 * phase should be same as data phase
			 */
			if (ReadCommand == FAST_READ_CMD_24BIT) {
				FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			}

			if (ReadCommand == DUAL_READ_CMD_24BIT) {
				FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_DUALSPI;
			}

			if (ReadCommand == QUAD_READ_CMD_24BIT) {
				FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
			}

			FlashMsg[1].TxBfrPtr = NULL;
			FlashMsg[1].RxBfrPtr = NULL;
			FlashMsg[1].ByteCount = DUMMY_CLOCKS;
			FlashMsg[1].Flags = 0U;
		}

		if (ReadCommand == FAST_READ_CMD_24BIT) {
			FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		}

		if (ReadCommand == DUAL_READ_CMD_24BIT) {
			FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_DUALSPI;
		}

		if (ReadCommand == QUAD_READ_CMD_24BIT) {
			FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
		}

		FlashMsg[2].TxBfrPtr = NULL;
		FlashMsg[2].RxAddr64bit = DestAddr;
		FlashMsg[2].RxBfrPtr = (u8 *)DestAddr;

		FlashMsg[2].ByteCount = TransferBytes;
		FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_RX;

		if(QspiPsuInstance.Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL){
			FlashMsg[2].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
		}

		/**
		 * Send the read command to the Flash to read the specified number
		 * of bytes from the Flash, send the read command and address and
		 * receive the specified number of bytes of data in the data buffer
		 */
		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg[0], 3);
		if (Status != XILPLI_SUCCESS) {
			Status = XILPLI_ERR_QSPI_READ;
			XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_READ\r\n");
			goto END;
		}

		/**
		 * Update the variables
		 */
		RemainingBytes -= TransferBytes;
		DestAddr += TransferBytes;
		SrcAddr += TransferBytes;
	}

	if(BankSwitchFlag == 1U)
	{
		/*
		 * Reset Bank selection to zero
		 */
		Status = SendBankSelect(0);
		if (Status != XILPLI_SUCCESS) {
			Status = XILPLI_ERR_QSPI_READ;
			XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_READ\r\n");
			goto END;
		}
	}
	else
	{
		Status = XILPLI_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to release the Qspi settings
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int XPli_Qspi24Release(void)
{
	int Status = XILPLI_SUCCESS;

	return Status;
}


/*****************************************************************************/
/**
 * This function is used to initialize the qspi controller and driver
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int XPli_Qspi32Init(u32 DeviceFlags)
{
	XQspiPsu_Config *QspiConfig;
	int Status;
	u32 QspiMode;

	/**
	 * This parameter is required as per the prototype
	 */
	(void) (DeviceFlags);

	/**
	 * Initialize the QSPI driver so that it's ready to use
	 */
	QspiConfig =  XQspiPsu_LookupConfig(QSPI_DEVICE_ID);
	if (NULL == QspiConfig) {
		Status = XILPLI_ERR_QSPI_INIT;
		XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_INIT\r\n");
		goto END;
	}

	Status =  XQspiPsu_CfgInitialize(&QspiPsuInstance, QspiConfig,
			QspiConfig->BaseAddress);
	if (Status != XILPLI_SUCCESS) {
		Status = XILPLI_ERR_QSPI_INIT;
		XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_INIT\r\n");
		goto END;
	}

	/*
	 * Set Manual Start
	 */
	Status = XQspiPsu_SetOptions(&QspiPsuInstance,
				     XQSPIPSU_MANUAL_START_OPTION);
	if (Status != XILPLI_SUCCESS) {
		Status = XILPLI_ERR_QSPI_MANUAL_START;
		XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_MANUAL_START\r\n");
		goto END;
	}
	/*
	 * Set the pre-scaler for QSPI clock
	 */
	Status = XQspiPsu_SetClkPrescaler(&QspiPsuInstance,
					  XQSPIPSU_CLK_PRESCALE_8);
	if (Status != XILPLI_SUCCESS) {
		Status = XILPLI_ERR_QSPI_PRESCALER_CLK;
		XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_PRESCALER_CLK\r\n");
		goto END;
	}
	XQspiPsu_SelectFlash(&QspiPsuInstance,
		XQSPIPSU_SELECT_FLASH_CS_LOWER, XQSPIPSU_SELECT_FLASH_BUS_LOWER);

	/*
	 * Configure the qspi in linear mode if running in XIP
	 * TBD
	 */

	switch ((u32)XPAR_XQSPIPSU_0_QSPI_MODE) {

		case XQSPIPSU_CONNECTION_MODE_SINGLE:
			{
				XPli_Printf(DEBUG_INFO,
				      "QSPI is in single flash connection\r\n");
			} break;

		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
			{
				XPli_Printf(DEBUG_INFO,
				      "QSPI is in Dual Parallel connection\r\n");
			} break;

		case XQSPIPSU_CONNECTION_MODE_STACKED:
			{
				XPli_Printf(DEBUG_INFO,
				      "QSPI is in Dual Stack connection\r\n");
			} break;

		default:
			{
				Status = XILPLI_ERR_QSPI_CONNECTION;
				XPli_Printf(DEBUG_GENERAL,
				      "XILPLI_ERR_QSPI_CONNECTION\r\n");
				goto END;
			} break;

	}


	/* add code for 1x, 2x and 4x */
	ReadCommand = QUAD_READ_CMD_32BIT;

	/* Read Flash ID and extract Manufacture and Size information */
	Status = FlashReadID(&QspiPsuInstance);
	if (Status != XILPLI_SUCCESS) {
		goto END;
	}

	/* add code: For a Stacked connection, read second Flash ID */
	QspiMode = XPAR_XQSPIPSU_0_QSPI_MODE;
	if ((QspiMode == (s32)(XQSPIPSU_CONNECTION_MODE_PARALLEL)) ||
	    (QspiMode == (s32)(XQSPIPSU_CONNECTION_MODE_STACKED)) ) {
		QspiFlashSize = 2 * QspiFlashSize;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to copy the data from QSPI flash to destination
 * address
 *
 * @param SrcAddr is the address of the QSPI flash where copy should
 * start from
 *
 * @param DestAddr is the address of the destination where it
 * should copy to
 *
 * @param Length Length of the bytes to be copied
 *
 * @return
 * 		- XILPLI_SUCCESS for successful copy
 * 		- errors as mentioned in xilpli_error.h
 *
 *****************************************************************************/
XStatus XPli_Qspi32Copy(u32 SrcAddr, u64 DestAddr, u32 Length, u32 Flags)
{
	int Status;
	u32 QspiAddr;
	u32 RemainingBytes;
	u32 TransferBytes;
	u32 DiscardByteCnt;

	XPli_Printf(DEBUG_INFO,"QSPI Reading Src 0x%0lx, Dest 0x%0lx, Length 0x%0lx, Flags 0x%0x\r\n",
			SrcAddr, (u32 )DestAddr, Length, Flags);

	/* Set QSPI DMA in AXI FIXED / INCR mode.
	 * Fixed mode is used for CFI loading */
	XPli_SetQspiDmaMode(Flags);

	/**
	 * Check the read length with Qspi flash size
	 */
	if ((SrcAddr + Length) > QspiFlashSize)
	{
		Status = XILPLI_ERR_QSPI_LENGTH;
		XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_LENGTH\r\n");
		goto END;
	}


	/**
	 * Update no of bytes to be copied
	 */
	RemainingBytes = Length;

	while(RemainingBytes > 0U) {

		if (RemainingBytes > DMA_DATA_TRAN_SIZE)
		{
			TransferBytes = DMA_DATA_TRAN_SIZE;
		} else {
			TransferBytes = RemainingBytes;
		}

		/**
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		QspiAddr = XPli_GetQspiAddr((u32 )SrcAddr);

		XPli_Printf(DEBUG_INFO,".");
		XPli_Printf(DEBUG_DETAILED,
					"QSPI Read Src 0x%0lx, Dest %0lx, Length %0lx\r\n",
						QspiAddr, DestAddr, TransferBytes);

		/**
		 * Setup the read command with the specified address and data for the
		 * Flash
		 */

		WriteBuffer[COMMAND_OFST]   = (u8)ReadCommand;
		WriteBuffer[ADDR_1_OFST] = (u8)((QspiAddr & 0xFF000000U) >> 24);
		WriteBuffer[ADDR_2_OFST] = (u8)((QspiAddr & 0xFF0000U) >> 16);
		WriteBuffer[ADDR_3_OFST] = (u8)((QspiAddr & 0xFF00U) >> 8);
		WriteBuffer[ADDR_4_OFST] = (u8)(QspiAddr & 0xFFU);
		DiscardByteCnt = 5;

		FlashMsg[0].TxBfrPtr = WriteBuffer;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = DiscardByteCnt;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		/*
		 * It is recommended to have a separate entry for dummy
		 */
		if ((ReadCommand == FAST_READ_CMD_32BIT) ||
				(ReadCommand == DUAL_READ_CMD_32BIT) ||
				(ReadCommand == QUAD_READ_CMD_32BIT)) {

			/* Update Dummy cycles as per flash specs for QUAD IO */

			/*
			 * It is recommended that Bus width value during dummy
			 * phase should be same as data phase
			 */
			if (ReadCommand == FAST_READ_CMD_32BIT) {
				FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
			}

			if (ReadCommand == DUAL_READ_CMD_32BIT) {
				FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_DUALSPI;
			}

			if (ReadCommand == QUAD_READ_CMD_32BIT) {
				FlashMsg[1].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
			}

			FlashMsg[1].TxBfrPtr = NULL;
			FlashMsg[1].RxBfrPtr = NULL;
			FlashMsg[1].ByteCount = DUMMY_CLOCKS;
			FlashMsg[1].Flags = 0U;
		}

		if (ReadCommand == FAST_READ_CMD_32BIT) {
			FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		}

		if (ReadCommand == DUAL_READ_CMD_32BIT) {
			FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_DUALSPI;
		}

		if (ReadCommand == QUAD_READ_CMD_32BIT) {
			FlashMsg[2].BusWidth = XQSPIPSU_SELECT_MODE_QUADSPI;
		}

		FlashMsg[2].TxBfrPtr = NULL;
        FlashMsg[2].RxAddr64bit = DestAddr;
        FlashMsg[2].RxBfrPtr = (u8 *)DestAddr;

		FlashMsg[2].ByteCount = TransferBytes;
		FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_RX;

		if(QspiPsuInstance.Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL){
			FlashMsg[2].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
		}

		/**
		 * Send the read command to the Flash to read the specified number
		 * of bytes from the Flash, send the read command and address and
		 * receive the specified number of bytes of data in the data buffer
		 */
		Status = XQspiPsu_PolledTransfer(&QspiPsuInstance, &FlashMsg[0], 3);
		if (Status != XILPLI_SUCCESS) {
			Status = XILPLI_ERR_QSPI_READ;
			XPli_Printf(DEBUG_GENERAL,"XILPLI_ERR_QSPI_READ\r\n");
			goto END;
		}

		/**
		 * Update the variables
		 */
		RemainingBytes -= TransferBytes;
		DestAddr += TransferBytes;
		SrcAddr += TransferBytes;

	}
	Status = XILPLI_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to release the Qspi settings
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
int XPli_Qspi32Release(void)
{
	int Status = XILPLI_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * This function is used to set the QSPI DMA destination channel AXI settings
 *
 * @param	DmaMode		Selects between AXI Fixed and Burst modes
 *
 * @return	None
 *
 *****************************************************************************/
void XPli_SetQspiDmaMode(u32 DmaMode)
{

	if (DmaMode == XILPLI_READ_AXI_FIXED)
	{
		Xil_Out32(XILPLI_QSPIDMA_DST_CTRL,
			Xil_In32(XILPLI_QSPIDMA_DST_CTRL) |
				XQSPIPSU_QSPIDMA_DST_CTRL_AXI_BRST_TYPE_MASK);
	} else {
		Xil_Out32(XILPLI_QSPIDMA_DST_CTRL,
			Xil_In32(XILPLI_QSPIDMA_DST_CTRL) &
				~XQSPIPSU_QSPIDMA_DST_CTRL_AXI_BRST_TYPE_MASK);
	}

}



#endif /* endof XILPLI_QSPI */
