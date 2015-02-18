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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#include "xparameters.h"

#ifdef XFSBL_QSPI
#include "xqspips.h"
#include "xfsbl_qspi.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define QSPI_DEVICE_ID		XPAR_XQSPIPS_0_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XFsbl_PrintArray (u32 DebugType, const u8 *Buf, u32 Len, const char *Str);

/************************** Variable Definitions *****************************/
static XQspiPs QspiInstance;
static XQspiPs *QspiInstancePtr;
static u32 QspiFlashSize=0;
static u32 QspiFlashMake=0;
static u32 ReadCommand=0;

/*
 * The following variables are used to read and write to the eeprom and they
 * are global to avoid having large buffers on the stack
 */
static u8 ReadBuffer[DATA_SIZE + DATA_OFFSET + DUMMY_SIZE];
static u8 WriteBuffer[DATA_OFFSET + DUMMY_SIZE];

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
static u32 FlashReadID(void)
{
	u32 Status = XFSBL_SUCCESS;

	/*
	 * Read ID in Auto mode.
	 */
	WriteBuffer[COMMAND_OFFSET]   = READ_ID_CMD;
	WriteBuffer[ADDRESS_1_OFFSET] = 0x00;		/* 3 dummy bytes */
	WriteBuffer[ADDRESS_2_OFFSET] = 0x00;
	WriteBuffer[ADDRESS_3_OFFSET] = 0x00;

	Status = XQspiPs_PolledTransfer(QspiInstancePtr, WriteBuffer, ReadBuffer,
				RD_ID_SIZE);
	if (Status != XST_SUCCESS) {
		Status = XFSBL_ERROR_QSPI_READ_ID;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_READ_ID\r\n");
		goto END;
	}

	XFsbl_Printf(DEBUG_INFO,"Single Flash Information\r\n");

	XFsbl_Printf(DEBUG_INFO,"FlashID=0x%x 0x%x 0x%x\r\n", ReadBuffer[1],
			ReadBuffer[2],
			ReadBuffer[3]);

	/*
	 * Deduce flash make
	 */
	if (ReadBuffer[1] == MICRON_ID) {
		QspiFlashMake = MICRON_ID;
		XFsbl_Printf(DEBUG_INFO, "MICRON ");
	} else if(ReadBuffer[1] == SPANSION_ID) {
		QspiFlashMake = SPANSION_ID;
		XFsbl_Printf(DEBUG_INFO, "SPANSION ");
	} else if(ReadBuffer[1] == WINBOND_ID) {
		QspiFlashMake = WINBOND_ID;
		XFsbl_Printf(DEBUG_INFO, "WINBOND ");
	} else {
		Status = XFSBL_ERROR_UNSUPPORTED_QSPI;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_UNSUPPORTED_QSPI\r\n");
		goto END;
	}

	/*
	 * Deduce flash Size
	 */
	if (ReadBuffer[3] == FLASH_SIZE_ID_128M) {
		QspiFlashSize = FLASH_SIZE_128M;
		XFsbl_Printf(DEBUG_INFO, "128M Bits\r\n");
	} else if (ReadBuffer[3] == FLASH_SIZE_ID_256M) {
		QspiFlashSize = FLASH_SIZE_256M;
		XFsbl_Printf(DEBUG_INFO, "256M Bits\r\n");
	} else if (ReadBuffer[3] == FLASH_SIZE_ID_512M) {
		QspiFlashSize = FLASH_SIZE_512M;
		XFsbl_Printf(DEBUG_INFO, "512M Bits\r\n");
	} else if (ReadBuffer[3] == FLASH_SIZE_ID_1G) {
		QspiFlashSize = FLASH_SIZE_1G;
		XFsbl_Printf(DEBUG_INFO, "1G Bits\r\n");
	}else {
		Status = XFSBL_ERROR_UNSUPPORTED_QSPI;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_UNSUPPORTED_QSPI\r\n");
		goto END;
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
u32 XFsbl_Qspi24Init()
{
	XQspiPs_Config *QspiConfig;
	u32 Status = XFSBL_SUCCESS;

	QspiInstancePtr = &QspiInstance;

	/**
	 * Initialize the qspi driver
	 */

	/**
	 * Initialize the QSPI driver so that it's ready to use
	 */
	QspiConfig = XQspiPs_LookupConfig(QSPI_DEVICE_ID);
	if (NULL == QspiConfig) {
		Status = XFSBL_ERROR_QSPI_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_INIT\r\n");
		goto END;
	}

	Status = XQspiPs_CfgInitialize(QspiInstancePtr, QspiConfig,
			QspiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XFSBL_ERROR_QSPI_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_INIT\r\n");
		goto END;
	}

	/**
	 * Set the pre-scaler for QSPI clock
	 */
	Status = XQspiPs_SetClkPrescaler(QspiInstancePtr, XQSPIPS_CLK_PRESCALE_8);
	if (Status != XST_SUCCESS) {
		Status = XFSBL_ERROR_QSPI_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_INIT\r\n");
		goto END;
	}

	/**
	 * Set Auto Start and Manual Chip select options and drive the
	 * HOLD_B high.
	 */
	Status = XQspiPs_SetOptions(QspiInstancePtr, XQSPIPS_FORCE_SSELECT_OPTION |
			XQSPIPS_HOLD_B_DRIVE_OPTION);
	if (Status != XST_SUCCESS) {
		Status = XFSBL_ERROR_QSPI_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_INIT\r\n");
		goto END;
	}

	/**
	 *
	 * configure the qspi in linear mode if running in XIP
	 * Configure the the qspi in IO mode
	 */

	switch (XPAR_PSU_QSPI_0_QSPI_MODE) {

		case XQSPIPS_CONNECTION_MODE_SINGLE:
		{
			XFsbl_Printf(DEBUG_INFO,"QSPI is in single flash connection\r\n");
			/**
			 * Single flash IO read
			 */
			XQspiPs_SetLqspiConfigReg(QspiInstancePtr,
					SINGLE_QSPI_IO_CONFIG_QUAD_READ);
		} break;

		case XQSPIPS_CONNECTION_MODE_PARALLEL:
		{
			XFsbl_Printf(DEBUG_INFO,"QSPI is in Dual Parallel connection\r\n");

			/**
			 * Dual parallel flash IO read
			 */
			XQspiPs_SetLqspiConfigReg(QspiInstancePtr,
					DUAL_QSPI_PARALLEL_IO_CONFIG_QUAD_READ);
		} break;

		case XQSPIPS_CONNECTION_MODE_STACKED:
		{
			XFsbl_Printf(DEBUG_INFO,"QSPI is in Dual Stack connection\r\n");

			/**
			 * Dual Stack flash IO read
			 */
			XQspiPs_SetLqspiConfigReg(QspiInstancePtr,
					DUAL_QSPI_STACK_IO_CONFIG_READ);
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
	ReadCommand = QUAD_READ_CMD;

	/**
	 * Assert the FLASH chip select.
	 */
	Status = XQspiPs_SetSlaveSelect(QspiInstancePtr);
	if (Status != XST_SUCCESS) {
		Status = XFSBL_ERROR_QSPI_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_INIT\r\n");
		goto END;
	}

	/**
	 * Read Flash ID and extract Manufacture and Size information
	 */
	Status = FlashReadID();
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	/**
	 * add code: For a Stacked connection, read second Flash ID
	 */
	if ((XPAR_PSU_QSPI_0_QSPI_MODE == XQSPIPS_CONNECTION_MODE_PARALLEL) ||
	(XPAR_PSU_QSPI_0_QSPI_MODE == XQSPIPS_CONNECTION_MODE_STACKED)) {
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
	u32 LqspiCr = 0;
	u32 QspiAddr = 0;

	switch(XPAR_PSU_QSPI_0_QSPI_MODE) {

	case XQSPIPS_CONNECTION_MODE_SINGLE:
		QspiAddr = Address;
		break;

	case XQSPIPS_CONNECTION_MODE_STACKED:
		/**
		 * Get the current LQSPI Config reg value
		 */
		LqspiCr = XQspiPs_GetLqspiConfigReg(QspiInstancePtr);

		/* Select lower or upper Flash based on sector address */
		if(Address > QspiFlashSize) {
			/**
			 * Set selection to U_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiInstancePtr,
					LqspiCr | XQSPIPS_LQSPI_CR_U_PAGE_MASK);

			/**
			 * Subtract first flash size when accessing second flash
			 */
			QspiAddr = Address - QspiFlashSize;

		}else{

			/**
			 * Set selection to L_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiInstancePtr,
					LqspiCr & (~XQSPIPS_LQSPI_CR_U_PAGE_MASK));

			QspiAddr = Address;
		}

		/**
		 * Assert the Flash chip select.
		 */
		XQspiPs_SetSlaveSelect(QspiInstancePtr);
		break;

	case XQSPIPS_CONNECTION_MODE_PARALLEL:
		/**
		 * The effective address in each flash is the actual
		 * address / 2
		 */
		QspiAddr = Address / 2;
		break;
	default:
		/* RealAddr wont be assigned in this case; */
	break;

	}

	return(QspiAddr);
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
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };

	/**
	 * Bank select commands for Micron and Spansion are different
	 */
	if(QspiFlashMake == MICRON_ID) {
		/**
		 * For Micron command WREN should be sent first
		 * except for some specific feature set
		 */
		Status = XQspiPs_PolledTransfer(QspiInstancePtr, &WriteEnableCmd, NULL,
					sizeof(WriteEnableCmd));
		if (Status != XST_SUCCESS) {
			Status = XFSBL_FAILURE;
			goto END;
		}


		WriteBuffer[COMMAND_OFFSET]   = EXTADD_REG_WR;
		WriteBuffer[ADDRESS_1_OFFSET] = BankSel;

		/**
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		Status = XQspiPs_PolledTransfer(QspiInstancePtr, WriteBuffer, NULL,
				BANK_SEL_SIZE);
		if (Status != XST_SUCCESS) {
			Status = XFSBL_FAILURE;
			goto END;
		}
	}
	if(QspiFlashMake == SPANSION_ID) {
		WriteBuffer[COMMAND_OFFSET]   = BANK_REG_WR;
		WriteBuffer[ADDRESS_1_OFFSET] = BankSel;

		/**
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		Status = XQspiPs_PolledTransfer(QspiInstancePtr, WriteBuffer, NULL,
				BANK_SEL_SIZE);
		if (Status != XST_SUCCESS) {
			Status = XFSBL_FAILURE;
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
	u32 QspiAddr=0;
	u32 BankSel=0;
	u32 RemainingBytes=0;
	u32 OverHeadBytes=0;
	u32 TransferBytes=0;

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

	while(RemainingBytes != 0) {

		/**
		 * Copy bytes in terms of 4K blocks from flash
		 */
		if (RemainingBytes > DATA_SIZE)
		{
			TransferBytes = DATA_SIZE;
		} else {
			TransferBytes = RemainingBytes;
		}

		/**
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		QspiAddr = XFsbl_GetQspiAddr((u32 )SrcAddress);

		/**
		 * Select bank
		 * check logic for DualQspi
		 */
		if(QspiFlashSize > BANKSIZE) {
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
		 * check logic for DualQspi
		 */
		if((QspiAddr & BANKMASK) != ((QspiAddr+TransferBytes) & BANKMASK)) {
			TransferBytes = (QspiAddr & BANKMASK) + BANKSIZE - QspiAddr;
		}


		/**
		 * Setup the read command with the specified address and data for the
		 * Flash
		 */
		WriteBuffer[COMMAND_OFFSET]   = ReadCommand;
		WriteBuffer[ADDRESS_1_OFFSET] = (u8)((QspiAddr & 0xFF0000) >> 16);
		WriteBuffer[ADDRESS_2_OFFSET] = (u8)((QspiAddr & 0xFF00) >> 8);
		WriteBuffer[ADDRESS_3_OFFSET] = (u8)(QspiAddr & 0xFF);

		if ((ReadCommand == FAST_READ_CMD) || (ReadCommand == DUAL_READ_CMD) ||
		    (ReadCommand == QUAD_READ_CMD))
		{
			OverHeadBytes = OVERHEAD_SIZE + DUMMY_SIZE;
		} else {
			OverHeadBytes = OVERHEAD_SIZE;
		}

		/**
		 * Send the read command to the Flash to read the specified number
		 * of bytes from the Flash, send the read command and address and
		 * receive the specified number of bytes of data in the data buffer
		 */
		Status = XQspiPs_PolledTransfer(QspiInstancePtr, WriteBuffer,
			ReadBuffer,TransferBytes + OverHeadBytes);
		if (Status != XST_SUCCESS) {
			Status = XFSBL_ERROR_QSPI_READ;
			XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_QSPI_READ\r\n");
			goto END;
		}

		/**
		 * remove if not required
		 */
		XFsbl_PrintArray(DEBUG_DETAILED, ReadBuffer,
			TransferBytes + OverHeadBytes, "QSPI READ DATA");

		/**
		 * Moving the data from local buffer to DDR destination address
		 */
		XFsbl_MemCpy((u8 *)DestAddress, &ReadBuffer[OverHeadBytes],
				TransferBytes);

		XFsbl_Printf(DEBUG_INFO,".");
		XFsbl_Printf(DEBUG_DETAILED,
			"QSPI Read Src 0x%0lx, Dest %0lx, Length %0lx\r\n",
				QspiAddr, DestAddress, TransferBytes);

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
u32 XFsbl_Qspi24Release()
{
	u32 Status = XFSBL_SUCCESS;

	return Status;
}


#endif /* endof XFSBL_QSPI */
