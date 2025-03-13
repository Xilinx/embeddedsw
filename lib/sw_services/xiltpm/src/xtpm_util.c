/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtpm_util.c
*
* This is the file which contains utilities to read from / write to TPM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  tri  03/13/25 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_TPM
#include "xtpm.h"
#include "xtpm_hw.h"
#include "xtpm_error.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XSpiPs SpiInstance;

/*****************************************************************************/
/**
 * @brief	This function gets TPM status.
 *
 * @param	StatusPtr is pointer to read status
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XTpm_StatusGet(u8* StatusPtr)
{
	return XTpm_Transfer(XTPM_STS, NULL, StatusPtr, XTPM_ACCESS_TX_LENGTH);
}

/*****************************************************************************/
/**
 * @brief	This function sets the TPM status.
 *
 * @param	StatusVal is to be written to status register
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XTpm_StatusSet(u8 StatusVal)
{
	return XTpm_Transfer(XTPM_STS, &StatusVal, NULL, XTPM_ACCESS_TX_LENGTH);
}

/*****************************************************************************/
/**
 * @brief	This function gets TPM access configuration.
 *
 * @param	AccessPtr is pointer to access configuration variable
 *
 * @return	XST_SUCCESS if successful, else error code
 *
 ******************************************************************************/
u32 XTpm_AccessGet(u8* AccessPtr)
{
	return XTpm_Transfer(XTPM_ACCESS, NULL, AccessPtr, XTPM_ACCESS_TX_LENGTH);
}

/*****************************************************************************/
/**
 * @brief	This function sets TPM access configuration.
 *
 * @param	Access is set access configuration value
 *
 * @return	XST_SUCCESS if successful, else error code
 *
 ******************************************************************************/
u32 XTpm_AccessSet(u8 Access)
{
	return XTpm_Transfer(XTPM_ACCESS, &Access, NULL, XTPM_ACCESS_TX_LENGTH);
}

/*****************************************************************************/
/**
 * @brief	This function reads data from TPM FIFO.
 *
 * @param	Data is to be read to data FIFO
 * @param	ByteCount is the number of bytes to read.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
static inline u32 XTpm_FifoRead(u8* DataPtr, u8 ByteCount)
{
	return XTpm_Transfer(XTPM_DATA_FIFO, NULL, DataPtr, ByteCount);
}

/*****************************************************************************/
/**
 * @brief	This function write the data TPM FIFO.
 *
 * @param	DataPtr is data pointer to be written to FIFO.
 * @param	ByteCount is the number of bytes to read.
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
static inline u32 XTpm_FifoWrite(u8* DataPtr, u8 ByteCount)
{
	return XTpm_Transfer(XTPM_DATA_FIFO, DataPtr, NULL, ByteCount);
}

/*****************************************************************************/
/**
 * @brief	This function initializes SPI interface required to communicate
 * 			with TPM.
 *
 * @return	XST_SUCCESS if successful, else error code
 *
 ******************************************************************************/
u32 XTpm_SpiInit(void)
{
	int Status = XST_FAILURE;
	const XSpiPs_Config *SpiConfig;

	/*
	 * Initialize the SPI driver so that it's ready to use
	 */
	SpiConfig = XSpiPs_LookupConfig(XTPM_SPI_DEVICE_ID);
	if (NULL == SpiConfig) {
		Status = XTPM_ERR_SPIPS_CONFIG;
		goto END;
	}

	Status = (u32)XSpiPs_CfgInitialize(&SpiInstance, SpiConfig,
		SpiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XTPM_ERR_SPIPS_CFG_INIT;
		goto END;
	}

	/*
	 * Perform a self-test to check hardware build
	 */
	Status = XSpiPs_SelfTest(&SpiInstance);
	if (Status != XST_SUCCESS) {
		Status = XTPM_ERR_SPIPS_SELF_TEST;
		goto END;
	}

	/*
	 * Set the Spi device as a master. External loopback is required
	 */
	Status = XSpiPs_SetOptions(&SpiInstance, XSPIPS_MANUAL_START_OPTION |
		XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION);
	if (Status != XST_SUCCESS) {
		Status = XTPM_ERR_SPIPS_SET_OPTIONS;
		goto END;
	}

	Status = XSpiPs_SetClkPrescaler(&SpiInstance, XSPIPS_CLK_PRESCALE_16);
	if (Status != XST_SUCCESS) {
		Status = XTPM_ERR_SPIPS_SET_CLK_PRESCALER;
		goto END;
	}

	/*
	 * Assert the TPM chip select
	 */
	Status = XSpiPs_SetSlaveSelect(&SpiInstance, XTPM_SPI_SELECT);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends a command to TPM and reads response.
 *
 * @param	TxBuf is pointer to transmit buffer
 * @param	RxBuf is pointer to receive buffer
 * @param	Txlen is data transfer length
 *
 * @return	XST_SUCCESS if successful, else error code.
 *
 ******************************************************************************/
u32 XTpm_DataTransfer(u8* TxBuf, u8* RxBuf, u16 Txlen)
{
	int Status = XST_FAILURE;
	u8 StatusVal = 0U;
	u16 RxLen;

	/* Set command ready request */
	Status = XTpm_StatusSet(XTPM_STS_CMD_READY);
	if (Status != XST_SUCCESS) {
		Status = XTPM_ERR_SET_ACCESS;
		goto END;
	}

	do {
		/* Check for command ready status */
		Status = XTpm_StatusGet(&StatusVal);
		if (Status != XST_SUCCESS) {
			Status = XTPM_ERR_GET_ACCESS;
			goto END;
		}
	} while ((StatusVal & XTPM_STS_CMD_READY) == 0U);

	/* Write Data to device */
	Status = XTpm_FifoWrite(TxBuf, (u8)Txlen);
	if (Status != XST_SUCCESS) {
		Status = XTPM_ERR_SPIPS_FIFO_WRITE;
		goto END;
	}

	/* Set command to go */
	Status = XTpm_StatusSet(XTPM_STS_GO);
	if (Status != XST_SUCCESS) {
		Status = XTPM_ERR_SET_ACCESS;
		goto END;
	}
	StatusVal = 0U;
	do {
		/* Check for command ready and valid */
		Status = XTpm_StatusGet(&StatusVal);
		if (Status != XST_SUCCESS) {
			Status = XTPM_ERR_GET_ACCESS;
			goto END;
		}
	} while ((StatusVal &
		(XTPM_STS_VALID | XTPM_STS_DATA_AVAIL)) !=
		(XTPM_STS_VALID | XTPM_STS_DATA_AVAIL));

	/* Read Data from device */
	Status = XTpm_FifoRead(RxBuf, XTPM_RX_HEAD_SIZE);
	if (Status != (u32)XST_SUCCESS) {
		Status = XTPM_ERR_SPIPS_FIFO_READ;
		goto END;
	}

	RxLen = RxBuf[XTPM_DATA_SIZE_INDEX];
	if (RxLen > XTPM_RESP_MAX_SIZE) {
		Status = XST_FAILURE;
		goto END;
	}

	if (RxBuf[XTPM_DATA_SIZE_INDEX] > XTPM_RX_HEAD_SIZE) {
		RxLen = (u16)RxBuf[XTPM_DATA_SIZE_INDEX] - XTPM_RX_HEAD_SIZE;
		Status = XTpm_FifoRead(&RxBuf[XTPM_RX_HEAD_SIZE],
			(u8)RxLen);
		if (Status != XST_SUCCESS) {
			Status = XTPM_ERR_SPIPS_FIFO_READ;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function transfers control information to TPM.
 *
 * @param 	Address is register address
 * @param	TxBuf is pointer to transmit buffer
 * @param	RxBuf is pointer to receive buffer
 * @param	Len is transfer length
 *
 * @return	XST_SUCCESS if successful, else Error Code.
 *
 * @note		None.
 *
 ******************************************************************************/
u32 XTpm_Transfer(u16 Address, u8 *TxBuf, u8 *RxBuf, u16 Len)
{
	int Status = XST_FAILURE;
	u8 TranLen;
	u16 Length = Len;
	u16 RxOffset = 0U;
	u8 TpmTxBuffer[XTPM_REQ_MAX_SIZE + XTPM_TX_HEAD_SIZE] = {0U};
	u8 TpmRxBuffer[XTPM_RESP_MAX_SIZE + XTPM_TX_HEAD_SIZE] = {0U};

	if (Length > XTPM_REQ_MAX_SIZE) {
		Status = XTPM_ERR_DATA_TX_LENGTH_LIMIT;
		goto END;
	}

	while (Length != 0) {
		TranLen = (u8)((Length <= XTPM_SPI_MAX_SIZE) ? Length :
			XTPM_SPI_MAX_SIZE);

		TpmTxBuffer[0U] = ((RxBuf != NULL) ? 0x80U : 0U) | (TranLen - XTPM_ACCESS_TX_LENGTH);
		TpmTxBuffer[1U] = 0xD4U;
		TpmTxBuffer[2U] = (u8)(Address >> 8U);
		TpmTxBuffer[3U] = (u8)Address;
		if (TxBuf != NULL) {
			(void)Xil_SMemCpy(&TpmTxBuffer[XTPM_TX_HEAD_SIZE], TranLen,
				TxBuf, TranLen, TranLen);
		}

		Status = XSpiPs_PolledTransfer(&SpiInstance, TpmTxBuffer,
			TpmRxBuffer, (u32)(TranLen + XTPM_TX_HEAD_SIZE));
		if (Status != XST_SUCCESS) {
			Status = XTPM_ERR_SPIPS_POLLING_TRANSFER;
			goto END;
		}

		if (RxBuf != NULL) {
			(void)Xil_SMemCpy(&RxBuf[RxOffset], TranLen,
				&TpmRxBuffer[XTPM_TX_HEAD_SIZE], TranLen, TranLen);
			RxOffset += TranLen;
		}
		Length -= TranLen;
	}
	if (Length == 0) {
		Status = XST_SUCCESS;
	}
	else {
		Status = XTPM_ERR_SPIPS_TRANSFER;
	}

END:
	return Status;
}

#endif	/* PLM_TPM */

/**
 * @}
 * @endcond
 */
