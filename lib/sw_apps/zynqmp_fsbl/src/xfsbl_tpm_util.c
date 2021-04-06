/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


/*****************************************************************************/
/**
*
* @file xfsbl_tpm_util.c
*
* This is the file which contains utilities to read from / write to TPM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bsv  04/01/21 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#include "xparameters.h"

#ifdef XFSBL_TPM
#include "xfsbl_tpm.h"
#include "xspips.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XSpiPs SpiInstance;

/*****************************************************************************/
/**
*
* This function gets TPM status.
*
* @param	StatusPtr is pointer to read status
*
* @return	XFSBL_SUCCESS if successful, else error code.
*
******************************************************************************/
u32 XFsbl_TpmStatusGet(u8* StatusPtr)
{
	u32 Status = XFSBL_FAILURE;

	Status = XFsbl_TpmTransfer(XFSBL_TPM_STS, NULL, StatusPtr, 1U);

	return Status;
}

/*****************************************************************************/
/**
*
* This function sets the TPM status.
*
* @param	StatusVal is to be written to status register
*
* @return	XFSBL_SUCCESS if successful, else error code.
*
******************************************************************************/
u32 XFsbl_TpmStatusSet(u8 StatusVal)
{
	u32 Status = XFSBL_FAILURE;

	Status = XFsbl_TpmTransfer(XFSBL_TPM_STS, &StatusVal, NULL, 1U);

	return Status;
}

/*****************************************************************************/
/**
*
* This function gets TPM access configuration.
*
* @param	AccessPtr is pointer to access configuration variable
*
* @return	XFSBL_SUCCESS if successful, else error code
*
******************************************************************************/
u32 XFsbl_TpmAccessGet(u8* AccessPtr)
{
	u32 Status = XFSBL_FAILURE;

	Status = XFsbl_TpmTransfer(XFSBL_TPM_ACCESS, NULL, AccessPtr, 1U);

	return Status;
}

/*****************************************************************************/
/**
*
* This function sets TPM access configuration.
*
* @param	Access is set access configuration value
*
* @return	XFSBL_SUCCESS if successful, else error code
*
******************************************************************************/
u32 XFsbl_TpmAccessSet(u8 Access)
{
	u32 Status = XST_FAILURE;

	Status = XFsbl_TpmTransfer(XFSBL_TPM_ACCESS, &Access, NULL, 1U);

	return Status;
}

/*****************************************************************************/
/**
*
* This function reads data from TPM FIFO.
*
* @param	Data is to be read to data FIFO
* @param	ByteCount is the number of bytes to read.
*
* @return	XFSBL_SUCCESS if successful, else error code.
*
******************************************************************************/
static inline u32 XFsbl_TpmFifoRead(u8* DataPtr, u8 ByteCount)
{
	u32 Status = XFSBL_FAILURE;

	Status = XFsbl_TpmTransfer(XFSBL_TPM_DATA_FIFO, NULL, DataPtr,
		ByteCount);

	return Status;
}

/*****************************************************************************/
/**
*
* This function write the data TPM FIFO.
*
* @param	DataPtr is data pointer to be written to FIFO.
* @param	ByteCount is the number of bytes to read.
*
* @return	XFSBL_SUCCESS if successful, else error code.
*
******************************************************************************/
static inline u32 XFsbl_TpmFifoWrite(u8* DataPtr, u8 ByteCount)
{
	u32 Status = XFSBL_FAILURE;

	Status = XFsbl_TpmTransfer(XFSBL_TPM_DATA_FIFO, DataPtr, NULL,
		ByteCount);

	return Status;
}

/*****************************************************************************/
/**
*
* This function initializes SPI interface required to communicate with TPM.
*
* @return	XFSBL_SUCCESS if successful, else error code
*
******************************************************************************/
u32 XFsbl_SpiInit(void)
{
	u32 Status = XFSBL_FAILURE;
	XSpiPs_Config *SpiConfig;

	/*
	 * Initialize the SPI driver so that it's ready to use
	 */
	SpiConfig = XSpiPs_LookupConfig(XFSBL_SPI_DEVICE_ID);
	if (NULL == SpiConfig) {
		goto END;
	}

	Status = XSpiPs_CfgInitialize(&SpiInstance, SpiConfig,
		SpiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/*
	 * Perform a self-test to check hardware build
	 */
	Status = XSpiPs_SelfTest(&SpiInstance);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/*
	 * Set the Spi device as a master. External loopback is required
	 */
	XSpiPs_SetOptions(&SpiInstance, XSPIPS_MANUAL_START_OPTION |
		XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION);

	XSpiPs_SetClkPrescaler(&SpiInstance, XSPIPS_CLK_PRESCALE_16);

	/*
	 * Assert the TPM chip select
	 */
	Status = XSpiPs_SetSlaveSelect(&SpiInstance, XFSBL_TPM_SPI_SELECT);

END:
	return Status;
}

/*****************************************************************************/
/**
*
* This function sends a command to TPM and reads response.
*
* @param	TxBuf is pointer to transmit buffer
* @param	RxBuf is pointer to receive buffer
* @param	Txlen is data transfer length
*
* @return	XFSBL_SUCCESS if successful, else error code.
*
******************************************************************************/
u32 XFsbl_TpmDataTransfer(u8* TxBuf, u8* RxBuf, u16 Txlen)
{
	u32 Status = XFSBL_FAILURE;
	u8 StatusVal = 0U;
	u16 RxLen;

	/* Set command ready request */
	Status = XFsbl_TpmStatusSet(XFSBL_TPM_STS_CMD_READY);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	do {
		/* Check for command ready status */
		Status = XFsbl_TpmStatusGet(&StatusVal);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	} while ((StatusVal & XFSBL_TPM_STS_CMD_READY) == 0U);

	/* Write Data to device */
	Status = XFsbl_TpmFifoWrite(TxBuf, Txlen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Set command to go */
	Status = XFsbl_TpmStatusSet(XFSBL_TPM_STS_GO);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	StatusVal = 0U;
	do {
		/* Check for command ready and valid */
		Status = XFsbl_TpmStatusGet(&StatusVal);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	} while ((StatusVal &
		(XFSBL_TPM_STS_VALID | XFSBL_TPM_STS_DATA_AVAIL)) !=
		(XFSBL_TPM_STS_VALID | XFSBL_TPM_STS_DATA_AVAIL));

	/* Read Data from device */
	Status = XFsbl_TpmFifoRead(RxBuf, XFSBL_TPM_RX_HEAD_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	RxLen = RxBuf[XFSBL_TPM_DATA_SIZE_INDEX];
	if (RxLen > XFSBL_TPM_RESP_MAX_SIZE) {
		Status = XST_FAILURE;
		goto END;
	}

	if (RxBuf[XFSBL_TPM_DATA_SIZE_INDEX] > XFSBL_TPM_RX_HEAD_SIZE)
	{
		RxLen = RxBuf[XFSBL_TPM_DATA_SIZE_INDEX] - XFSBL_TPM_RX_HEAD_SIZE;
		Status = XFsbl_TpmFifoRead(&RxBuf[XFSBL_TPM_RX_HEAD_SIZE],
			RxLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
*
* This function transfers control information to TPM.
*
* @param 	Address is register address
* @param	TxBuf is pointer to transmit buffer
* @param	RxBuf is pointer to receive buffer
* @param	Length is transfer length
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
u32 XFsbl_TpmTransfer(u16 Address, u8 *TxBuf, u8 *RxBuf, u16 Length)
{
	int Status = XST_FAILURE;
	u8 TranLen;
	u16 RxOffset = 0U;
	u8 TpmTxBuffer[XFSBL_TPM_REQ_MAX_SIZE + XFSBL_TPM_TX_HEAD_SIZE] = {0U};
	u8 TpmRxBuffer[XFSBL_TPM_RESP_MAX_SIZE + XFSBL_TPM_TX_HEAD_SIZE] = {0U};

	if ((Length > XFSBL_TPM_REQ_MAX_SIZE) ||
		(Length > XFSBL_TPM_RESP_MAX_SIZE))
	{
		goto END;
	}

	while (Length) {
		TranLen = (Length <= XFSBL_TPM_SPI_MAX_SIZE) ? Length :
			XFSBL_TPM_SPI_MAX_SIZE;

		TpmTxBuffer[0U] = (RxBuf ? 0x80U : 0U) | (TranLen - 1U);
		TpmTxBuffer[1U] = 0xD4U;
		TpmTxBuffer[2U] = Address >> 8U;
		TpmTxBuffer[3U] = (u8)Address;
		if (TxBuf != NULL) {
			(void)XFsbl_MemCpy(&TpmTxBuffer[XFSBL_TPM_TX_HEAD_SIZE],
				TxBuf, TranLen);
		}

		Status = XSpiPs_PolledTransfer(&SpiInstance, TpmTxBuffer,
			TpmRxBuffer, (TranLen + XFSBL_TPM_TX_HEAD_SIZE));
		if (Status != XST_SUCCESS) {
			goto END;
		}

		if (RxBuf != NULL) {
			(void)XFsbl_MemCpy(&RxBuf[RxOffset],
				&TpmRxBuffer[XFSBL_TPM_TX_HEAD_SIZE], TranLen);
			RxOffset += TranLen;
		}
		Length -= TranLen;
	}
	Status = XFSBL_SUCCESS;

END:
	return Status;
}
#endif /* XFSBL_TPM */
