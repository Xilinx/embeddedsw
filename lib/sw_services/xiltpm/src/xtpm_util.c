/******************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
*       pre  08/23/25 Did enhancements needed
*       pre  09/23/25 Fixed misrac violations
*       pre  10/23/25 Fixed bug in TPM command transmission
* 1.2   pre  01/16/25 Updated comments for RTF documentation
*       pre  03/13/26 Added support to change TPM interface layer as per customer requirement
*
* </pre>
*
* @note
*
******************************************************************************/

/**
 * @addtogroup xtpm_apis XilTPM APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_TPM
#include "xtpm.h"
#include "xtpm_hw.h"
#include "xtpm_error.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef struct {
	u32 (*XTpm_InterfaceInit)(void); /**< Function pointer to TPM interface initialization
										  function */
	s32 (*XTpm_TransferViaInterface)(void *InstancePtr, u8 *SendBufPtr,
			  u8 *RecvBufPtr, u32 ByteCount); /**< Function pointer to TPM command transmission
                                                   function via the initialized interface */
} XTpm_Interface_t;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static u32 XTpm_SpiInit(void);
static s32 XTpm_SpiTransfer(void *InstancePtr, u8 *SendBufPtr, u8 *RecvBufPtr, u32 ByteCount);

/************************** Variable Definitions *****************************/
static XSpiPs SpiInstance;
/**
 * @brief Structure can be overridden by customer-defined functions to use
 *        other transport layers (for example, I2C). The structure is initialized
 *        with SPI function pointers by default.
 */
static XTpm_Interface_t XTpm_Interface = {
	.XTpm_InterfaceInit = XTpm_SpiInit,
	.XTpm_TransferViaInterface = XTpm_SpiTransfer
};

/*************************************************************************************************/
/**
 * @brief	This function initializes the TPM interface
 *
 * @return
 * 			- XST_SUCCESS if initialization is successful
 * 			- Error code if initialization fails
 *
 *************************************************************************************************/
u32 XTpm_InterfaceInit(void)
{
	return XTpm_Interface.XTpm_InterfaceInit();
}

/*************************************************************************************************/
/**
 * @brief	This function sends command to TPM via the initialized interface and gets response.
 *
 * @param	InstancePtr is pointer to the initialized interface instance
 * @param	SendBufPtr is pointer to buffer in which command to be sent is present
 * @param	RecvBufPtr is pointer to buffer in which response from TPM gets stored
 * @param	ByteCount is the number of bytes to be sent and received
 * @return
 * 			- XST_SUCCESS if command transmission and response reception are successful
 * 			- Error code if any of the operations fail
 *
 *************************************************************************************************/
static s32 XTpm_SpiTransfer(void *InstancePtr, u8 *SendBufPtr, u8 *RecvBufPtr, u32 ByteCount)
{
    return XSpiPs_PolledTransfer((XSpiPs *)InstancePtr, SendBufPtr, RecvBufPtr, ByteCount);
}

/*****************************************************************************/
/**
 * @brief	This function gets TPM status and write to the input address.
 *
 * @param	StatusPtr is pointer to location in which value read from status register gets stored
 *
 * @return
 * 			- XST_SUCCESS if read is successful
 * 			- Error code if read fails
 *
 ******************************************************************************/
u32 XTpm_StatusGet(u8* StatusPtr)
{
	return XTpm_Transfer(XTPM_STS, NULL, StatusPtr, XTPM_ACCESS_TX_LENGTH);
}

/*****************************************************************************/
/**
 * @brief	This function sets the TPM status with the input value.
 *
 * @param	StatusVal is value to be set in status register
 *
 * @return
 * 			- XST_SUCCESS if write is successful
 * 			- Error code if write fails
 *
 ******************************************************************************/
u32 XTpm_StatusSet(u8 StatusVal)
{
	return XTpm_Transfer(XTPM_STS, (const u8 *)&StatusVal, NULL, XTPM_ACCESS_TX_LENGTH);
}

/*****************************************************************************/
/**
 * @brief	This function gets TPM access configuration and writes to the input address.
 *
 * @param	AccessPtr is pointer to location in which value read from access configuration register gets stored
 *
 * @return
 * 			- XST_SUCCESS if read is successful
 * 			- Error code if read fails
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
 * @param	Access is value to be set in access configuration register
 *
 * @return
 * 			- XST_SUCCESS if write is successful
 * 			- Error code if write fails
 *
 ******************************************************************************/
u32 XTpm_AccessSet(u8 Access)
{
	return XTpm_Transfer(XTPM_ACCESS, (const u8 *)&Access, NULL, XTPM_ACCESS_TX_LENGTH);
}

/*****************************************************************************/
/**
 * @brief	This function reads data from TPM FIFO and writes to buffer at input address.
 *
 * @param	DataPtr is pointer to buffer in which data read from FIFO gets stored
 * @param	ByteCount is the number of bytes to be read.
 *
 * @return
 * 			- XST_SUCCESS if read is successful
 * 			- Error code if read fails
 *
 ******************************************************************************/
static inline u32 XTpm_FifoRead(u8* DataPtr, u8 ByteCount)
{
	return XTpm_Transfer(XTPM_DATA_FIFO, NULL, DataPtr, ByteCount);
}

/*****************************************************************************/
/**
 * @brief	This function writes data to the TPM FIFO from buffer at input address.
 *
 * @param	DataPtr is pointer to buffer in which data to be written to FIFO is present.
 * @param	ByteCount is the number of bytes to be written.
 *
 * @return
 * 			- XST_SUCCESS if write is successful
 * 			- Error code if write fails
 *
 ******************************************************************************/
static inline u32 XTpm_FifoWrite(const u8* DataPtr, u8 ByteCount)
{
	return XTpm_Transfer(XTPM_DATA_FIFO, DataPtr, NULL, ByteCount);
}

/*************************************************************************************************/
/**
 * @brief	This function initializes SPI interface with required configuration to communicate
 * 			with TPM.
 *
 * @return
 * 			- XST_SUCCESS if SPI initialization is successful
 * 			- XTPM_ERR_SPIPS_CONFIG if SPI configuration info is not found
 * 			- XTPM_ERR_SPIPS_CFG_INIT if SPI configuration initialization fails
 * 			- XTPM_ERR_SPIPS_SELF_TEST if SPI self-test fails
 * 			- XTPM_ERR_SPIPS_SET_OPTIONS in case of any failure during SPI options setting
 * 			- XTPM_ERR_SPIPS_SET_CLK_PRESCALER in case of failure during clock prescalar setting
 * 			- XST_DEVICE_BUSY in case of failure during chip select assertion
 *
 *************************************************************************************************/
static u32 XTpm_SpiInit(void)
{
	u32 Status = (u32)XST_FAILURE;
	const XSpiPs_Config *SpiConfig;

	/**
	 * - Initializes the SPI driver so that it is ready to use.
	 * Returns XTPM_ERR_SPIPS_CONFIG error if the configuration info is not found.
	 */
	SpiConfig = XSpiPs_LookupConfig(XTPM_SPI_DEVICE_ID);
	if (NULL == SpiConfig) {
		Status = (u32)XTPM_ERR_SPIPS_CONFIG;
		goto END;
	}

	/**
	 * - Initializes SPI instance. Returns XTPM_ERR_SPIPS_CFG_INIT error in case of failure.
	 */
	Status = (u32)XSpiPs_CfgInitialize(&SpiInstance, SpiConfig,
		SpiConfig->BaseAddress);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_SPIPS_CFG_INIT;
		goto END;
	}

	/**
	 * - Performs a self-test to check hardware build.
	 * Returns XTPM_ERR_SPIPS_SELF_TEST error in case of failure.
	 */
	Status = (u32)XSpiPs_SelfTest(&SpiInstance);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_SPIPS_SELF_TEST;
		goto END;
	}

	/**
	 * - Sets the SPI device as a master. External loopback is required.
	 * Returns XTPM_ERR_SPIPS_SET_OPTIONS in case of failure.
	 */
	Status = (u32)XSpiPs_SetOptions(&SpiInstance, XSPIPS_MANUAL_START_OPTION |
		XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_SPIPS_SET_OPTIONS;
		goto END;
	}

	/**
	 * - Sets clock pre-scaler. Returns XTPM_ERR_SPIPS_SET_CLK_PRESCALER error
	 * in case of failure.
	 */
	Status = (u32)XSpiPs_SetClkPrescaler(&SpiInstance, XSPIPS_CLK_PRESCALE_32);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_SPIPS_SET_CLK_PRESCALER;
		goto END;
	}

	/**
	 * - Asserts the TPM chip select. Returns XST_SUCCESS
	 * if successful and error code in case of failure
	 */
	Status = (u32)XSpiPs_SetSlaveSelect(&SpiInstance, XTPM_SPI_SELECT);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends a command to TPM and reads response received from TPM.
 *
 * @param	TxBuf is pointer to transmit buffer from which data to be transmitted is read
 * @param	RxBuf is pointer to receive buffer to which received data gets stored
 * @param	Txlen is data transfer length in bytes
 *
 * @return
 * 			- XST_SUCCESS if successful
 * 			- XTPM_ERR_SET_ACCESS if it fails during setting of access register with command
 * 			  ready/go request
 * 			- XTPM_ERR_GET_ACCESS if it fails during reading of access register
 * 			- XTPM_ERR_SPIPS_FIFO_WRITE if it fails to write to TPM
 * 			- XTPM_ERR_SPIPS_FIFO_READ if it fails to read from TPM
 * 			- XST_FAILURE if received response length is greater than 4096 bytes
 *
 *************************************************************************************************/
u32 XTpm_DataTransfer(const u8* TxBuf, u8* RxBuf, u16 Txlen)
{
	u32 Status = (u32)XST_FAILURE;
	u8 StatusVal = 0U;
	u16 RxLen;

	/** - Sets command ready request. Returns XTPM_ERR_SET_ACCESS error in case of failure */
	Status = XTpm_StatusSet(XTPM_STS_CMD_READY);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_SET_ACCESS;
		goto END;
	}

	do {
		/** - Checks for command ready status. Returns XTPM_ERR_GET_ACCESS error in case of failure */
		Status = XTpm_StatusGet(&StatusVal);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XTPM_ERR_GET_ACCESS;
			goto END;
		}
	} while ((StatusVal & XTPM_STS_CMD_READY) == 0U);

	/** - Writes Data to device. Returns XTPM_ERR_SPIPS_FIFO_WRITE error in case of failure */
	Status = XTpm_FifoWrite(TxBuf, (u8)Txlen);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_SPIPS_FIFO_WRITE;
		goto END;
	}

	/** - Sets command to go. Returns XTPM_ERR_SET_ACCESS error in case of failure */
	Status = XTpm_StatusSet(XTPM_STS_GO);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_SET_ACCESS;
		goto END;
	}
	StatusVal = 0U;
	do {
		/**
		 * - Checks for command ready and valid. Returns XTPM_ERR_GET_ACCESS error
		 * in case of failure
		 */
		Status = XTpm_StatusGet(&StatusVal);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XTPM_ERR_GET_ACCESS;
			goto END;
		}
	} while ((StatusVal &
		(XTPM_STS_VALID | XTPM_STS_DATA_AVAIL)) !=
		(XTPM_STS_VALID | XTPM_STS_DATA_AVAIL));

	/** - Reads Data from device. Returns XTPM_ERR_SPIPS_FIFO_READ error in case of failure */
	Status = XTpm_FifoRead(RxBuf, XTPM_RX_HEAD_SIZE);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XTPM_ERR_SPIPS_FIFO_READ;
		goto END;
	}

	/* Reads received response length */
	RxLen = RxBuf[XTPM_DATA_SIZE_INDEX];
	if (RxLen > (u16)XTPM_RESP_MAX_SIZE) {
		Status = (u32)XST_FAILURE;
		goto END;
	}

	/**
	 * - Reads received response data. Returns XST_SUCCESS if
	 * successful and XTPM_ERR_SPIPS_FIFO_READ in case of failure
	 */
	if (RxBuf[XTPM_DATA_SIZE_INDEX] > XTPM_RX_HEAD_SIZE) {
		RxLen = (u16)RxBuf[XTPM_DATA_SIZE_INDEX] - XTPM_RX_HEAD_SIZE;
		Status = XTpm_FifoRead(&RxBuf[XTPM_RX_HEAD_SIZE],
			(u8)RxLen);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XTPM_ERR_SPIPS_FIFO_READ;
		}
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function transfers control information to TPM based on input(Txn/Recv).
 *
 * @param 	Address is register address to which data is to be written/read
 * @param	TxBuf is pointer to transmit buffer in which data to be transmitted is present
 * @param	RxBuf is pointer to receive buffer in which received data gets stored
 * @param	Len is length of data to be transferred in bytes
 *
 * @return
 * 			- XST_SUCCESS if successful
 * 			- Error code on failure
 *
 *************************************************************************************************/
u32 XTpm_Transfer(u16 Address, const u8 *TxBuf, u8 *RxBuf, u16 Len)
{
	u32 Status = (u32)XST_FAILURE;
	u8 TranLen;
	volatile u16 Length = Len;
	u16 RxOffset = 0U;
	u16 TxOffset = 0U;

	/* Initializes transmit buffer */
	static u8 TpmTxBuffer[XTPM_REQ_MAX_SIZE + XTPM_TX_HEAD_SIZE] = {0U};
	/* Initializes receive buffer */
	static u8 TpmRxBuffer[XTPM_RESP_MAX_SIZE + XTPM_TX_HEAD_SIZE] = {0U};

	/**
	 * - Length validation. Return XTPM_ERR_DATA_TX_LENGTH_LIMIT
	 * error if the length is greater than 1024 bytes
	 */
	if (Length > XTPM_REQ_MAX_SIZE) {
		Status = (u32)XTPM_ERR_DATA_TX_LENGTH_LIMIT;
		goto END;
	}

	/* Transfers data */
	while (Length != 0U) {
		TranLen = (u8)((Length <= XTPM_SPI_MAX_SIZE) ? Length :
			XTPM_SPI_MAX_SIZE);

		/** - Prepares data to be transferred */
		TpmTxBuffer[0U] = ((RxBuf != NULL) ? 0x80U : 0U) | (TranLen - XTPM_ACCESS_TX_LENGTH);
		TpmTxBuffer[1U] = 0xD4U;
		TpmTxBuffer[2U] = (u8)(Address >> 8U);
		TpmTxBuffer[3U] = (u8)Address;
		if (TxBuf != NULL) {
			(void)Xil_SMemCpy(&TpmTxBuffer[XTPM_TX_HEAD_SIZE], TranLen,
				&TxBuf[TxOffset], TranLen, TranLen);
			TxOffset += TranLen;
		}

		/**
		 * - Transfers data and receive response in polled mode.
		 * Returns XTPM_ERR_SPIPS_POLLING_TRANSFER in case of failure
		 */
		Status = (u32)XTpm_Interface.XTpm_TransferViaInterface((void *)&SpiInstance, TpmTxBuffer,
			TpmRxBuffer, (u32)(TranLen + XTPM_TX_HEAD_SIZE));
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XTPM_ERR_SPIPS_POLLING_TRANSFER;
			goto END;
		}

		/** - Copies received response to receive buffer. Returns XST_SUCCESS */
		if (RxBuf != NULL) {
			(void)Xil_SMemCpy(&RxBuf[RxOffset], TranLen,
				&TpmRxBuffer[XTPM_TX_HEAD_SIZE], TranLen, TranLen);
			RxOffset += TranLen;
		}
		Length -= TranLen;
	}
	if (Length == 0U) {
		Status = (u32)XST_SUCCESS;
	}
	else {
		Status = (u32)XTPM_ERR_SPIPS_TRANSFER;
	}

END:
	return Status;
}

#endif	/* PLM_TPM */

/** @} End of xtpm_apis group */
