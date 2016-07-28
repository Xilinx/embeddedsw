/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xfsbl_qspi.c
*
* This is the file which contains qspi related code for the FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   10/21/13 Initial release
* 2.00  sg   12/03/15 Added GQSPI driver support
*                     32Bit boot mode support
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#include "xparameters.h"
#include "xil_cache.h"

#ifdef XFSBL_QSPI
#include "xqspipsu.h"
#include "xfsbl_qspi.h"

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
void XFsbl_PrintArray (u32 DebugType, const u8 *Buf, u32 Len, const char *Str);

/************************** Variable Definitions *****************************/
static XQspiPsu QspiPsuInstance;
static XQspiPsu *QspiPsuInstancePtr;
static u32 QspiFlashSize=0;
static u32 QspiFlashMake=0;
static u32 ReadCommand=0;
static XQspiPsu_Msg FlashMsg[5];
static u8 IssiIdFlag=0;

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
int FlashReadID(XQspiPsu *QspiPsuPtr)
{
	int Status = XST_SUCCESS;

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

	Status = XQspiPsu_PolledTransfer(QspiPsuPtr, FlashMsg, 2);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XFsbl_Printf(DEBUG_GENERAL, "FlashID=0x%x 0x%x 0x%x\n\r", ReadBuffer[0],
					ReadBuffer[1], ReadBuffer[2]);

	/*
	 * Deduce flash make
	 */
	if (ReadBuffer[0] == MICRON_ID) {
		QspiFlashMake = MICRON_ID;
		XFsbl_Printf(DEBUG_INFO, "MICRON ");
	} else if(ReadBuffer[0] == SPANSION_ID) {
		QspiFlashMake = SPANSION_ID;
		XFsbl_Printf(DEBUG_INFO, "SPANSION ");
	} else if(ReadBuffer[0] == WINBOND_ID) {
		QspiFlashMake = WINBOND_ID;
		XFsbl_Printf(DEBUG_INFO, "WINBOND ");
	} else if(ReadBuffer[0] == MACRONIX_ID) {
		QspiFlashMake = MACRONIX_ID;
		XFsbl_Printf(DEBUG_INFO, "MACRONIX ");
	} else if(ReadBuffer[0] == ISSI_ID) {
		QspiFlashMake = ISSI_ID;
		XFsbl_Printf(DEBUG_INFO, "ISSI ");
	} else {
		Status = XFSBL_ERROR_UNSUPPORTED_QSPI;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_UNSUPPORTED_QSPI\r\n");
		goto END;
	}

	/*
	 * Deduce flash Size
	 */
	if (ReadBuffer[2] == FLASH_SIZE_ID_64M) {
		QspiFlashSize = FLASH_SIZE_64M;
		XFsbl_Printf(DEBUG_INFO, "64M Bits\r\n");
	} else if (ReadBuffer[2] == FLASH_SIZE_ID_128M) {
		QspiFlashSize = FLASH_SIZE_128M;
		XFsbl_Printf(DEBUG_INFO, "128M Bits\r\n");
	} else if (ReadBuffer[2] == FLASH_SIZE_ID_256M) {
		QspiFlashSize = FLASH_SIZE_256M;
		XFsbl_Printf(DEBUG_INFO, "256M Bits\r\n");
	} else if ((ReadBuffer[2] == FLASH_SIZE_ID_512M)
			|| (ReadBuffer[2] == MACRONIX_FLASH_SIZE_ID_512M)) {
		QspiFlashSize = FLASH_SIZE_512M;
		XFsbl_Printf(DEBUG_INFO, "512M Bits\r\n");
	} else if ((ReadBuffer[2] == FLASH_SIZE_ID_1G)
			|| (ReadBuffer[2] == MACRONIX_FLASH_SIZE_ID_1G)) {
		QspiFlashSize = FLASH_SIZE_1G;
		XFsbl_Printf(DEBUG_INFO, "1G Bits\r\n");
	}else {
		Status = XFSBL_ERROR_UNSUPPORTED_QSPI;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_UNSUPPORTED_QSPI\r\n");
		goto END;
	}
	/* Enable ID flag for ISSI 128M Qspi to enable
	 * DUAL_READ_CMD_24BIT ReadCommand
	 */
	if((QspiFlashMake==ISSI_ID) && (QspiFlashSize==FLASH_SIZE_128M))
	{
		IssiIdFlag=1;
	}

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
u32 XFsbl_Qspi24Init(u32 DeviceFlags)
{
	XQspiPsu_Config *QspiConfig;
	u32 Status = XFSBL_SUCCESS;

	QspiPsuInstancePtr = &QspiPsuInstance;

	/**
	 * Initialize the QSPI driver so that it's ready to use
	 */
	QspiConfig =  XQspiPsu_LookupConfig(QSPI_DEVICE_ID);
	if (NULL == QspiConfig) {
		Status = XFSBL_ERROR_QSPI_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_INIT\r\n");
		goto END;
	}

	Status =  XQspiPsu_CfgInitialize(QspiPsuInstancePtr, QspiConfig,
			QspiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XFSBL_ERROR_QSPI_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_INIT\r\n");
		goto END;
	}

	/*
	 * Set Manual Start
	 */
	XQspiPsu_SetOptions(QspiPsuInstancePtr, XQSPIPSU_MANUAL_START_OPTION);

	/*
	 * Set the pre-scaler for QSPI clock
	 */
	XQspiPsu_SetClkPrescaler(QspiPsuInstancePtr, XQSPIPSU_CLK_PRESCALE_8);

	XQspiPsu_SelectFlash(QspiPsuInstancePtr,
		XQSPIPSU_SELECT_FLASH_CS_LOWER, XQSPIPSU_SELECT_FLASH_BUS_LOWER);

	/*
	 * Configure the qspi in linear mode if running in XIP
	 * TBD: XIP Support
	 */

	switch (XPAR_PSU_QSPI_0_QSPI_MODE) {

		case XQSPIPSU_CONNECTION_MODE_SINGLE:
		{
			XFsbl_Printf(DEBUG_INFO,"QSPI is in single flash connection\r\n");
		} break;

		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
		{
			XFsbl_Printf(DEBUG_INFO,"QSPI is in Dual Parallel connection\r\n");
		} break;

		case XQSPIPSU_CONNECTION_MODE_STACKED:
		{
			XFsbl_Printf(DEBUG_INFO,"QSPI is in Dual Stack connection\r\n");
		}break;

		default:
		{
			Status = XFSBL_ERROR_INVALID_QSPI_CONNECTION;
			XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_INVALID_QSPI_CONNECTION\r\n");
			goto END;
		}break;

	}
	/**
	 * Read Flash ID and extract Manufacture and Size information
	 */
	Status = FlashReadID(QspiPsuInstancePtr);
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}
	/**
	 *  add code for 1x, 2x and 4x
	 *
	 */
	if(IssiIdFlag==1) {
		ReadCommand = DUAL_READ_CMD_24BIT;
	} else {
		ReadCommand = QUAD_READ_CMD_24BIT;
	}
	/**
	 * add code: For a Stacked connection, read second Flash ID
	 */
	if ((XPAR_PSU_QSPI_0_QSPI_MODE ==
			XQSPIPSU_CONNECTION_MODE_PARALLEL) ||
			(XPAR_PSU_QSPI_0_QSPI_MODE ==
					XQSPIPSU_CONNECTION_MODE_STACKED) ) {
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
* @param	Address which is to be accessed
*
* @return	QspiAddr is the actual flash address to be accessed - for single
* 			it is unchanged; for stacked, the lower flash size is subtracted;
* 			for parallel the address is divided by 2.
*
* @note		None.
*
*
******************************************************************************/
static u32 XFsbl_GetQspiAddr(u32 Address )
{
	u32 RealAddr;

	switch(QspiPsuInstancePtr->Config.ConnectionMode) {

		case XQSPIPSU_CONNECTION_MODE_SINGLE:
			XQspiPsu_SelectFlash(QspiPsuInstancePtr,
					XQSPIPSU_SELECT_FLASH_CS_LOWER,
					XQSPIPSU_SELECT_FLASH_BUS_LOWER);
			RealAddr = Address;
			break;

		case XQSPIPSU_CONNECTION_MODE_STACKED:
			/*
			 * Select lower or upper Flash based on sector address
			 */
			if(Address >= (QspiFlashSize/2)) {
				XQspiPsu_SelectFlash(QspiPsuInstancePtr,
						XQSPIPSU_SELECT_FLASH_CS_UPPER,
						XQSPIPSU_SELECT_FLASH_BUS_LOWER);
				/*
				 * Subtract first flash size when accessing second flash
				 */
				RealAddr = Address - (QspiFlashSize/2);
			}else{
				/*
				 * Set selection to L_PAGE
				 */
				XQspiPsu_SelectFlash(QspiPsuInstancePtr,
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
			XQspiPsu_SelectFlash(QspiPsuInstancePtr,
			XQSPIPSU_SELECT_FLASH_CS_BOTH, XQSPIPSU_SELECT_FLASH_BUS_BOTH);
			RealAddr = Address / 2;
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
	u32 Status = XFSBL_SUCCESS;

	/*
	 * bank select commands for Micron and Spansion are different
	 * Macronix bank select is same as Micron
	 */
	if (QspiFlashMake == MICRON_ID || QspiFlashMake == MACRONIX_ID)	{
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

		Status = XQspiPsu_PolledTransfer(QspiPsuInstancePtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			Status = XFSBL_ERROR_QSPI_READ;
			XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_READ\r\n");
			goto END;
		}

		/*
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		WriteBuffer[COMMAND_OFFSET]   = EXTADD_REG_WR_CMD;
		WriteBuffer[ADDRESS_1_OFFSET] = BankSel;

		FlashMsg[0].TxBfrPtr = WriteBuffer;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 2;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuInstancePtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			Status = XFSBL_ERROR_QSPI_READ;
			XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_READ\r\n");
			goto END;
		}
	}

	if (QspiFlashMake == SPANSION_ID) {
		/*
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		WriteBuffer[COMMAND_OFFSET]   = BANK_REG_WR_CMD;
		WriteBuffer[ADDRESS_1_OFFSET] = BankSel;

		FlashMsg[0].TxBfrPtr = WriteBuffer;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = 2;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		Status = XQspiPsu_PolledTransfer(QspiPsuInstancePtr, FlashMsg, 1);
		if (Status != XST_SUCCESS) {
			Status = XFSBL_ERROR_QSPI_READ;
			XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_READ\r\n");
			goto END;
		}
	}

	/*
	 * For testing - Read bank to verify
	 */
	if (QspiFlashMake == MICRON_ID || QspiFlashMake == MACRONIX_ID) {
		/*
		 * Extended address register read command
		 */

		WriteBuffer[COMMAND_OFFSET]   = EXTADD_REG_RD_CMD;
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

		Status = XQspiPsu_PolledTransfer(QspiPsuInstancePtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			Status = XFSBL_ERROR_QSPI_READ;
			XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_READ\r\n");
			goto END;
		}
	}

	if (QspiFlashMake == SPANSION_ID) {
		/*
		 * Bank register read command
		 */
		WriteBuffer[COMMAND_OFFSET]   = BANK_REG_RD_CMD;
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

		Status = XQspiPsu_PolledTransfer(QspiPsuInstancePtr, FlashMsg, 2);
		if (Status != XST_SUCCESS) {
			Status = XFSBL_ERROR_QSPI_READ;
			XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_READ\r\n");
			goto END;
		}

		if (ReadBuffer[0] != BankSel) {
			XFsbl_Printf(DEBUG_INFO, "Bank Select %d != Register Read %d\n\r", BankSel,
				ReadBuffer[0]);
			Status = XFSBL_ERROR_QSPI_READ;
			XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_READ\r\n");
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
 * @param SrcAddress is the address of the QSPI flash where copy should
 * start from
 *
 * @param DestAddress is the address of the destination where it
 * should copy to
 *
 * @param Length Length of the bytes to be copied
 *
 * @return
 * 		- XFSBL_SUCCESS for successful copy
 * 		- errors as mentioned in xfsbl_error.h
 *
 *****************************************************************************/
u32 XFsbl_Qspi24Copy(u32 SrcAddress, PTRSIZE DestAddress, u32 Length)
{
	u32 Status = XFSBL_SUCCESS;
	u32 QspiAddr=0, OrigAddr=0;
	u32 BankSel=0;
	u32 RemainingBytes=0;
	u32 TransferBytes=0;
	u32 DiscardByteCnt;
	u8 BankSwitchFlag=0;
	u32 BankSize=SINGLEBANKSIZE;
	u32 BankMask=SINGLEBANKMASK;

	XFsbl_Printf(DEBUG_INFO,"QSPI Reading Src 0x%0lx, Dest %0lx, Length %0lx\r\n",
			SrcAddress, DestAddress, Length);

	/**
	 * Check the read length with Qspi flash size
	 */
	if ((SrcAddress + Length) > QspiFlashSize)
	{
		Status = XFSBL_ERROR_QSPI_LENGTH;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_LENGTH\r\n");
		goto END;
	}

	/* Multiply bank size & mask in case of Dual Parallel */
	if (QspiPsuInstancePtr->Config.ConnectionMode ==
	    XQSPIPSU_CONNECTION_MODE_PARALLEL){
		BankSize =  SINGLEBANKSIZE * 2;
		BankMask =  SINGLEBANKMASK * 2;
	}

	/**
	 * Update no of bytes to be copied
	 */
	RemainingBytes = Length;

	while(RemainingBytes > 0) {

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
		QspiAddr = XFsbl_GetQspiAddr((u32 )SrcAddress);

		/**
		 * Multiply address by 2 in case of Dual Parallel
		 * This address is used to calculate the bank crossing
		 * condition
		 */
		if (QspiPsuInstancePtr->Config.ConnectionMode ==
		    XQSPIPSU_CONNECTION_MODE_PARALLEL){
			OrigAddr = QspiAddr * 2;
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
			if (Status != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_QSPI_READ;
				XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_QSPI_READ\r\n");
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

		XFsbl_Printf(DEBUG_INFO,".");
				XFsbl_Printf(DEBUG_DETAILED,
					"QSPI Read Src 0x%0lx, Dest %0lx, Length %0lx\r\n",
						QspiAddr, DestAddress, TransferBytes);

		/**
		 * Setup the read command with the specified address and data for the
		 * Flash
		 */

		WriteBuffer[COMMAND_OFFSET]   = ReadCommand;
		WriteBuffer[ADDRESS_1_OFFSET] = (u8)((QspiAddr & 0xFF0000) >> 16);
		WriteBuffer[ADDRESS_2_OFFSET] = (u8)((QspiAddr & 0xFF00) >> 8);
		WriteBuffer[ADDRESS_3_OFFSET] = (u8)(QspiAddr & 0xFF);
		DiscardByteCnt = 4;

		FlashMsg[0].TxBfrPtr = WriteBuffer;
		FlashMsg[0].RxBfrPtr = NULL;
		FlashMsg[0].ByteCount = DiscardByteCnt;
		FlashMsg[0].BusWidth = XQSPIPSU_SELECT_MODE_SPI;
		FlashMsg[0].Flags = XQSPIPSU_MSG_FLAG_TX;

		/* It is recommended to have a separate entry for dummy */
		if ((ReadCommand == FAST_READ_CMD_24BIT) || (ReadCommand == DUAL_READ_CMD_24BIT) ||
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
			FlashMsg[1].Flags = 0;
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
		FlashMsg[2].RxBfrPtr = (u8 *)DestAddress;
		FlashMsg[2].ByteCount = TransferBytes;
		FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_RX;

		if(QspiPsuInstancePtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL){
			FlashMsg[2].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
		}

		/**
		 * Send the read command to the Flash to read the specified number
		 * of bytes from the Flash, send the read command and address and
		 * receive the specified number of bytes of data in the data buffer
		 */
		Status = XQspiPsu_PolledTransfer(QspiPsuInstancePtr, FlashMsg, 3);
		if (Status != XST_SUCCESS) {
			Status = XFSBL_ERROR_QSPI_READ;
			XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_READ\r\n");
			goto END;
		}

		/**
		 * Update the variables
		 */
		RemainingBytes -= TransferBytes;
		DestAddress += TransferBytes;
		SrcAddress += TransferBytes;
	}

	if(BankSwitchFlag == 1)
	{
		/*
		 * Reset Bank selection to zero
		 */
		Status = SendBankSelect(0);
		if (Status != XST_SUCCESS) {
			Status = XFSBL_ERROR_QSPI_READ;
			XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_READ\r\n");
			goto END;
		}
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
u32 XFsbl_Qspi24Release()
{
	u32 Status = XFSBL_SUCCESS;

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
u32 XFsbl_Qspi32Init(u32 DeviceFlags)
{
	XQspiPsu_Config *QspiConfig;
	u32 Status = XFSBL_SUCCESS;

	QspiPsuInstancePtr = &QspiPsuInstance;

	/**
	 * Initialize the QSPI driver so that it's ready to use
	 */
	QspiConfig =  XQspiPsu_LookupConfig(QSPI_DEVICE_ID);
	if (NULL == QspiConfig) {
		Status = XFSBL_ERROR_QSPI_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_INIT\r\n");
		goto END;
	}

	Status =  XQspiPsu_CfgInitialize(QspiPsuInstancePtr, QspiConfig,
			QspiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XFSBL_ERROR_QSPI_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_INIT\r\n");
		goto END;
	}

	/*
	 * Set Manual Start
	 */
	XQspiPsu_SetOptions(QspiPsuInstancePtr, XQSPIPSU_MANUAL_START_OPTION);

	/*
	 * Set the pre-scaler for QSPI clock
	 */
	XQspiPsu_SetClkPrescaler(QspiPsuInstancePtr, XQSPIPSU_CLK_PRESCALE_8);

	XQspiPsu_SelectFlash(QspiPsuInstancePtr,
		XQSPIPSU_SELECT_FLASH_CS_LOWER, XQSPIPSU_SELECT_FLASH_BUS_LOWER);

	/*
	 * Configure the qspi in linear mode if running in XIP
	 * TBD
	 */

	switch (XPAR_PSU_QSPI_0_QSPI_MODE) {

		case XQSPIPSU_CONNECTION_MODE_SINGLE:
		{
			XFsbl_Printf(DEBUG_INFO,"QSPI is in single flash connection\r\n");
		} break;

		case XQSPIPSU_CONNECTION_MODE_PARALLEL:
		{
			XFsbl_Printf(DEBUG_INFO,"QSPI is in Dual Parallel connection\r\n");
		} break;

		case XQSPIPSU_CONNECTION_MODE_STACKED:
		{
			XFsbl_Printf(DEBUG_INFO,"QSPI is in Dual Stack connection\r\n");
		}break;

		default:
		{
			Status = XFSBL_ERROR_INVALID_QSPI_CONNECTION;
			XFsbl_Printf(DEBUG_GENERAL,
					"XFSBL_ERROR_INVALID_QSPI_CONNECTION\r\n");
			goto END;
		}break;

	}


	/**
	 *  add code for 1x, 2x and 4x
	 *
	 */
	ReadCommand = QUAD_READ_CMD_32BIT;

	/**
	 * Read Flash ID and extract Manufacture and Size information
	 */
	Status = FlashReadID(QspiPsuInstancePtr);
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	/**
	 * add code: For a Stacked connection, read second Flash ID
	 */
	if ((XPAR_PSU_QSPI_0_QSPI_MODE ==
			XQSPIPSU_CONNECTION_MODE_PARALLEL) ||
			(XPAR_PSU_QSPI_0_QSPI_MODE ==
					XQSPIPSU_CONNECTION_MODE_STACKED) ) {
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
 * @param SrcAddress is the address of the QSPI flash where copy should
 * start from
 *
 * @param DestAddress is the address of the destination where it
 * should copy to
 *
 * @param Length Length of the bytes to be copied
 *
 * @return
 * 		- XFSBL_SUCCESS for successful copy
 * 		- errors as mentioned in xfsbl_error.h
 *
 *****************************************************************************/
u32 XFsbl_Qspi32Copy(u32 SrcAddress, PTRSIZE DestAddress, u32 Length)
{
	u32 Status = XFSBL_SUCCESS;
	u32 QspiAddr=0;
	u32 RemainingBytes=0;
	u32 TransferBytes=0;
	u32 DiscardByteCnt;

	XFsbl_Printf(DEBUG_INFO,"QSPI Reading Src 0x%0lx, Dest %0lx, Length %0lx\r\n",
			SrcAddress, DestAddress, Length);

	/**
	 * Check the read length with Qspi flash size
	 */
	if ((SrcAddress + Length) > QspiFlashSize)
	{
		Status = XFSBL_ERROR_QSPI_LENGTH;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_LENGTH\r\n");
		goto END;
	}


	/**
	 * Update no of bytes to be copied
	 */
	RemainingBytes = Length;

	while(RemainingBytes > 0) {

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
		QspiAddr = XFsbl_GetQspiAddr((u32 )SrcAddress);

		XFsbl_Printf(DEBUG_INFO,".");
		XFsbl_Printf(DEBUG_DETAILED,
					"QSPI Read Src 0x%0lx, Dest %0lx, Length %0lx\r\n",
						QspiAddr, DestAddress, TransferBytes);

		/**
		 * Setup the read command with the specified address and data for the
		 * Flash
		 */

		WriteBuffer[COMMAND_OFFSET]   = ReadCommand;
		WriteBuffer[ADDRESS_1_OFFSET] = (u8)((QspiAddr & 0xFF000000) >> 24);
		WriteBuffer[ADDRESS_2_OFFSET] = (u8)((QspiAddr & 0xFF0000) >> 16);
		WriteBuffer[ADDRESS_3_OFFSET] = (u8)((QspiAddr & 0xFF00) >> 8);
		WriteBuffer[ADDRESS_4_OFFSET] = (u8)(QspiAddr & 0xFF);
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
			FlashMsg[1].Flags = 0;
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
		FlashMsg[2].RxBfrPtr = (u8 *)DestAddress;
		FlashMsg[2].ByteCount = TransferBytes;
		FlashMsg[2].Flags = XQSPIPSU_MSG_FLAG_RX;

		if(QspiPsuInstancePtr->Config.ConnectionMode ==
				XQSPIPSU_CONNECTION_MODE_PARALLEL){
			FlashMsg[2].Flags |= XQSPIPSU_MSG_FLAG_STRIPE;
		}

		/**
		 * Send the read command to the Flash to read the specified number
		 * of bytes from the Flash, send the read command and address and
		 * receive the specified number of bytes of data in the data buffer
		 */
		Status = XQspiPsu_PolledTransfer(QspiPsuInstancePtr, FlashMsg, 3);
		if (Status != XST_SUCCESS) {
			Status = XFSBL_ERROR_QSPI_READ;
			XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_READ\r\n");
			goto END;
		}

		/**
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
 * This function is used to release the Qspi settings
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
u32 XFsbl_Qspi32Release()
{
	u32 Status = XFSBL_SUCCESS;

	return Status;
}

#endif /* endof XFSBL_QSPI */
