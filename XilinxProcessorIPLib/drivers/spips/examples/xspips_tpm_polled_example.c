/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xspips_tpm_polled_example.c
*
* This file contains a design example using the SPI driver (XSpiPs) in
* polled mode and hardware device with a Trusted Platform Module device.
* This example sends TPM commands to device based on user inputs and displays
* the response.
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  sg  04/02/21 Initial release
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* EDK generated parameters */
#include "xspips.h"		/* SPI device driver */
#include "xil_printf.h"
#include <stdio.h>

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SPI_DEVICE_ID		XPAR_XSPIPS_0_DEVICE_ID
#define TPM_SPI_SELECT		0x00

#define	XSPIPS_TPM_ACCESS 	0x0000
#define	XSPIPS_TPM_STS		0x0018
#define	XSPIPS_TPM_DATA_FIFO	0x0024
#define	XSPIPS_TPM_DID_VID	0x0F00
#define	XSPIPS_TPM_RID		0x0F04

#define	XSPIPS_TPM_ACCESS_VALID		0x80
#define	XSPIPS_TPM_ACCESS_ACT_LOCAL	0x20
#define	XSPIPS_TPM_ACCESS_REQ_PEND	0x04
#define	XSPIPS_TPM_ACCESS_REQ_USE 	0x02

#define	XSPIPS_TPM_STS_VALID 		0x80
#define	XSPIPS_TPM_STS_CMD_READY	0x40
#define	XSPIPS_TPM_STS_GO 		0x20
#define	XSPIPS_TPM_STS_DATA_AVAIL	0x10
#define	XSPIPS_TPM_STS_DATA_EXPECT 	0x08

#define XSPIPS_TPM_TX_HEAD_SIZE		4
#define XSPIPS_TPM_RX_HEAD_SIZE		10
#define XSPIPS_TPM_SPI_MAX_SIZE		64

#define XSPIPS_TPM2_CMD_SELFTEST	0x01
#define XSPIPS_TPM2_CMD_STARTUP		0x02
#define XSPIPS_TPM2_CMD_PCR_READ	0x03
#define XSPIPS_TPM2_CMD_PCR_RESET	0x04
#define XSPIPS_TPM2_CMD_PCR_EXTEND	0x05
#define XSPIPS_EXIT_CMD_LOOP		0x06

#define XSPIPS_TPM_PCR_EXT_CMD_SIZE	33

#define XSPIPS_TPM_SHA1_DIGEST_SIZE	20
#define XSPIPS_TPM_SHA256_DIGEST_SIZE	32

#define XSPIPS_TPM_RESP_MAX_SIZE	4096	/* Maximum possible TPM response size in bytes */
#define XSPIPS_TPM_REQ_MAX_SIZE		1024	/* Maximum possible TPM request size in bytes */

/**************************** Type Definitions *******************************/
int SpiPsReadID(XSpiPs *SpiPtr);
int SpiPsTpmStatusGet(XSpiPs *SpiPtr, u8* StatusPtr);
int SpiPsTpmStatusSet(XSpiPs *SpiPtr, u8 StatusVal);
int SpiPsTpmAccessGet(XSpiPs *SpiPtr, u8* Access);
int SpiPsTpmAccessSet(XSpiPs *SpiPtr, u8 Access);
int SpiPsTpmFifoRead(XSpiPs *SpiPtr, u8* DataPtr, u8 ByteCount);
int SpiPsTpmFifoWrite(XSpiPs *SpiPtr, u8* DataPtr, u8 ByteCount);
int SpiPsTpmTransfer(XSpiPs *SpiPtr, u32 Address, u8* TxBuf, u8* RxBuf, u16 Length);
int SpiPsTpmDataTransfer(XSpiPs *SpiPtr, u8* TxBuf, u8* RxBuf, u16 TxLen);
int SpiPsTpmPolledExample(XSpiPs *SpiInstancePtr, u16 SpiDeviceId);
/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that the
 * are initialized to zero each time the program runs.  They could be local
 * but should at least be static so they are zeroed.
 */
static XSpiPs SpiInstance;

u8 TpmTxBuffer[XSPIPS_TPM_REQ_MAX_SIZE + XSPIPS_TPM_TX_HEAD_SIZE];
u8 TpmRxBuffer[XSPIPS_TPM_RESP_MAX_SIZE + XSPIPS_TPM_TX_HEAD_SIZE];
u8 TpmRespBuffer[XSPIPS_TPM_RESP_MAX_SIZE];

static u8 tpm2_getcap[] = {
		0x80, 0x01, /* TPM_ST_NO_SESSIONS */
		0x00, 0x00, 0x00, 0x16, /* Command Size */
		0x00, 0x00, 0x01, 0x7A, /* TPM_CC_Get Cap */
		0x00, 0x00, 0x00, 0x06, /* TPM_SU_CLEAR */
		0x00, 0x00, 0x01, 0x29, 0x00, 0x00, 0x00, 0x01
};

static u8 tpm2_selftest[] = {
		0x80, 0x01, /* TPM_ST_NO_SESSIONS */
		0x00, 0x00, 0x00, 0x0C, /* Command Size */
		0x00, 0x00, 0x01, 0x43, /* TPM_CC_Selftest */
		0x00, 0x00 /* TPM_SU_CLEAR */
};

static u8 tpm2_startup[] = {
		0x80, 0x01, /* TPM_ST_NO_SESSIONS */
		0x00, 0x00, 0x00, 0x0C, /* Command Size */
		0x00, 0x00, 0x01, 0x44, /* TPM_CC_Startup */
		0x00, 0x00 /* TPM_SU_CLEAR */
};

static u8 tpm2_pcrread[] = {
		0x80, 0x01, /* TPM_ST_NO_SESSIONS */
		0x00, 0x00, 0x00, 0x14, /* Command Size */
		0x00, 0x00, 0x01, 0x7E, /* TPM_CC_PCR_Read */
		0x00, 0x00, 0x00, 0x01, /* PCR Count */
		0x00, 0x00, /* Hash algorithm */
		0x03, /* Size PCRs */
		0x00, 0x00, 0x00 /* PCR Select */
};

static u8 tpm2_pcrreset[] = {
		0x80, 0x02, /* TPM_ST_SESSIONS */
		0x00, 0x00, 0x00, 0x1B, /* Command Size */
		0x00, 0x00, 0x01, 0x3D, /* TPM_CC_PCR_Reset */
		0x00, 0x00, 0x00, 0x00, /* PCR_Index */
		0x00, 0x00, /* NULL Password */
		0x00, 0x09, /* Authorization Size */
		0x40, 0x00, 0x00, 0x09, /* Password authorization session */
		0x00, 0x00, 0x01, 0x00, 0x00
};

static u8 tpm2_pcrextend[XSPIPS_TPM_PCR_EXT_CMD_SIZE +
XSPIPS_TPM_SHA256_DIGEST_SIZE] = {
		0x80, 0x02, /* TPM_ST_SESSIONS */
		0x00, 0x00, 0x00, 0x00, /* Command Size */
		0x00, 0x00, 0x01, 0x82, /* TPM_CC_PCR_Extend */
		0x00, 0x00, 0x00, 0x00, /* PCR_Index */
		0x00, 0x00, /* NULL Password */
		0x00, 0x09, /* Authorization Size */
		0x40, 0x00, 0x00, 0x09, /* Password authorization session */
		0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, /* TPML_DIGEST_VALUES */
		0x00, 0x00 /* Hash Algorithm */
};

/* Below digest just for reference user can set as required */
static u8 sha1[XSPIPS_TPM_SHA1_DIGEST_SIZE] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 05};
static u8 sha256[XSPIPS_TPM_SHA256_DIGEST_SIZE] = {
		0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

/*****************************************************************************/
/**
*
* Main function to call the Spi TPM example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("SPI TPM Polled Mode Example Test \r\n");

	/*
	 * Run the Spi TPM polled example.
	 */
	Status = SpiPsTpmPolledExample(&SpiInstance, SPI_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SPI TPM Polled Mode Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SPI TPM Polled Mode Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @param	SpiInstancePtr is a pointer to the Spi Instance.
* @param	SpiDeviceId is the Device Id of Spi.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note
*
*
*****************************************************************************/
int SpiPsTpmPolledExample(XSpiPs *SpiInstancePtr, u16 SpiDeviceId)
{
	XSpiPs_Config *SpiConfig;
	int Index;
	int Option;
	int Status;
	int PcrIndex;
	int HashAlgo;
	u8 Access;
	u8 ExitCmdLoop = 0;

	/*
	 * Initialize the SPI driver so that it's ready to use
	 */
	SpiConfig = XSpiPs_LookupConfig(SpiDeviceId);
	if (NULL == SpiConfig) {
		return XST_FAILURE;
	}

	Status = XSpiPs_CfgInitialize(SpiInstancePtr, SpiConfig,
				       SpiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to check hardware build
	 */
	Status = XSpiPs_SelfTest(SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the Spi device as a master. External loopback is required.
	 */
	XSpiPs_SetOptions(SpiInstancePtr, XSPIPS_MANUAL_START_OPTION |
				XSPIPS_MASTER_OPTION |
			   XSPIPS_FORCE_SSELECT_OPTION);

	XSpiPs_SetClkPrescaler(SpiInstancePtr, XSPIPS_CLK_PRESCALE_16);

	/*
	 * Assert the TPM chip select
	 */
	Status = XSpiPs_SetSlaveSelect(SpiInstancePtr, TPM_SPI_SELECT);

	/* Set access request to use */
	Status = SpiPsTpmAccessSet(SpiInstancePtr, XSPIPS_TPM_ACCESS_REQ_USE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	do {
		/* Check for access valid and locality */
		Status = SpiPsTpmAccessGet(SpiInstancePtr, &Access);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	} while (!((Access & XSPIPS_TPM_ACCESS_VALID)
			&& (Access & XSPIPS_TPM_ACCESS_ACT_LOCAL)));

	/*
	 * Read TPM ID information
	 */
	Status = SpiPsReadID(SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = SpiPsTpmDataTransfer(SpiInstancePtr, tpm2_getcap, TpmRespBuffer,
			tpm2_getcap[5]);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (!ExitCmdLoop) {
		xil_printf("TPM CMD Menu:\r\n");
		xil_printf("1: tpm2_selftest\r\n");
		xil_printf("2: tpm2_startup\r\n");
		xil_printf("3: tpm2_pcrread\r\n");
		xil_printf("4: tpm2_pcrreset\r\n");
		xil_printf("5: tpm2_pcrextend\r\n");
		xil_printf("6: Exit CMD loop\r\n");

		xil_printf("Select TPM command\r\n");
		scanf("%d", &Option);

		switch (Option) {

		case XSPIPS_TPM2_CMD_SELFTEST:
			xil_printf("CMD: tpm2_selftest\r\n");
			Status = SpiPsTpmDataTransfer(SpiInstancePtr, tpm2_selftest,
					TpmRespBuffer, tpm2_selftest[5]);
			break;

		case XSPIPS_TPM2_CMD_STARTUP:
			xil_printf("CMD: tpm2_startup\r\n");
			Status = SpiPsTpmDataTransfer(SpiInstancePtr, tpm2_startup,
					TpmRespBuffer, tpm2_startup[5]);
			break;

		case XSPIPS_TPM2_CMD_PCR_READ:
			xil_printf("CMD: tpm2_pcrread\r\n");

			xil_printf("Select PCR Index(0-23)\r\n");
			scanf("%d", &PcrIndex);
			xil_printf("PCR Index: %d\r\n", PcrIndex);

			if (PcrIndex > 23) {
				xil_printf("Invalid PCR index\r\n");
				break;
			}

			xil_printf("Select hash algorithm 1: sha1 2: sha256\r\n");
			scanf("%d", &HashAlgo);

			if (HashAlgo == 1) {
				xil_printf("Hash: sha1\r\n");
				tpm2_pcrread[15] = 0x04;
			} else if (HashAlgo == 2) {
				xil_printf("Hash: sha256\r\n");
				tpm2_pcrread[15] = 0x0B;
			} else {
				xil_printf("Invalid PCR index\r\n");
				break;
			}

			tpm2_pcrread[17 + (PcrIndex / 8)] = (1 << (PcrIndex % 8));

			Status = SpiPsTpmDataTransfer(SpiInstancePtr, tpm2_pcrread,
					TpmRespBuffer, tpm2_pcrread[5]);
			break;

		case XSPIPS_TPM2_CMD_PCR_RESET:
			xil_printf("CMD: tpm2_pcrreset\r\n");

			xil_printf("Select PCR Index(16-23)\r\n");

			scanf("%d", &PcrIndex);
			xil_printf("PCR Index: %d\r\n", PcrIndex);

			if ((PcrIndex < 16) || (PcrIndex > 23)) {
				xil_printf("Invalid PCR index\r\n");
				break;
			}

			tpm2_pcrreset[13] = PcrIndex;

			Status = SpiPsTpmDataTransfer(SpiInstancePtr, tpm2_pcrreset,
					TpmRespBuffer, tpm2_pcrreset[5]);
			break;

		case XSPIPS_TPM2_CMD_PCR_EXTEND:
			xil_printf("CMD: tpm2_pcrextend\r\n");

			xil_printf("Select PCR Index(0-16 and 23)\r\n");

			scanf("%d", &PcrIndex);
			xil_printf("PCR Index: %d\r\n", PcrIndex);

			if (PcrIndex > 16) {
				if(PcrIndex != 23) {
					xil_printf("Invalid PCR index\r\n");
					break;
				}
			}
			tpm2_pcrextend[13] = PcrIndex;

			xil_printf("Select hash algorithm 1: sha1 2: sha256\r\n");
			scanf("%d", &HashAlgo);

			if (HashAlgo == 1) {
				xil_printf("Hash: sha1\r\n");

				tpm2_pcrextend[32] = 0x04;

				tpm2_pcrextend[5] = XSPIPS_TPM_PCR_EXT_CMD_SIZE
						+ XSPIPS_TPM_SHA1_DIGEST_SIZE;

				for(Index= 0; Index<XSPIPS_TPM_SHA1_DIGEST_SIZE; Index++)
					tpm2_pcrextend[XSPIPS_TPM_PCR_EXT_CMD_SIZE + Index] =
							sha1[Index];

			} else if (HashAlgo == 2) {
				xil_printf("Hash: sha256\r\n");

				tpm2_pcrextend[32] = 0x0B;

				tpm2_pcrextend[5] = XSPIPS_TPM_PCR_EXT_CMD_SIZE
						+ XSPIPS_TPM_SHA256_DIGEST_SIZE;

				for(Index= 0; Index<XSPIPS_TPM_SHA256_DIGEST_SIZE; Index++)
					tpm2_pcrextend[XSPIPS_TPM_PCR_EXT_CMD_SIZE + Index] =
							sha256[Index];

			} else {
				xil_printf("Invalid PCR index\r\n");
				break;
			}

			Status = SpiPsTpmDataTransfer(SpiInstancePtr, tpm2_pcrextend,
					TpmRespBuffer, tpm2_pcrextend[5]);
			break;

		case XSPIPS_EXIT_CMD_LOOP:
			ExitCmdLoop = 1;
			break;

		default:
			xil_printf("Invalid option selected\r\n");

		}

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (!ExitCmdLoop) {
			for (Index = 0; Index < TpmRespBuffer[5]; Index++) {
				xil_printf("Response Data[%d] = 0x%x\r\n", Index,
						TpmRespBuffer[Index]);
			}
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads vendor identification information.
*
* @param	SpiPtr is a pointer to the SPIPS driver component to use.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SpiPsReadID(XSpiPs *SpiPtr)
{
	int Status;
	u32 VendId;
	u8 RevId;

	Status = SpiPsTpmTransfer(SpiPtr, XSPIPS_TPM_DID_VID, NULL, (u8*)&VendId, 4);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = SpiPsTpmTransfer(SpiPtr, XSPIPS_TPM_RID, NULL, &RevId, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("Vendor ID 0x%x Revision ID 0x%x\r\n", VendId, RevId);

	return XST_SUCCESS;
}



/*****************************************************************************/
/**
*
* This function gets TPM status.
*
* @param	SpiPtr is a pointer to the SPIPS driver component to use.
* @param	StatusPtr is pointer to read status
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SpiPsTpmStatusGet(XSpiPs *SpiPtr, u8* StatusPtr) {
	int Status;

	Status = SpiPsTpmTransfer(SpiPtr, XSPIPS_TPM_STS, NULL, StatusPtr, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets the TPM status.
*
* @param	SpiPtr is a pointer to the SPIPS driver component to use.
* @param	StatusVal is to be written to status register
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SpiPsTpmStatusSet(XSpiPs *SpiPtr, u8 StatusVal) {
	int Status;

	Status = SpiPsTpmTransfer(SpiPtr, XSPIPS_TPM_STS, &StatusVal, NULL, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function gets TPM access configuration.
*
* @param	SpiPtr is a pointer to the SPIPS driver component to use.
* @param	AccessPtr is pointer to get access configuration
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SpiPsTpmAccessGet(XSpiPs *SpiPtr, u8* AccessPtr) {
	int Status;

	Status = SpiPsTpmTransfer(SpiPtr, XSPIPS_TPM_ACCESS, NULL, AccessPtr, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets TPM access configuration.
*
* @param	SpiPtr is a pointer to the SPIPS driver component to use.
* @param	Access is set access configuration value
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SpiPsTpmAccessSet(XSpiPs *SpiPtr, u8 Access) {
	int Status;

	Status = SpiPsTpmTransfer(SpiPtr, XSPIPS_TPM_ACCESS, &Access, NULL, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads data from TPM FIFO.
*
* @param	SpiPtr is a pointer to the SPIPS driver component to use.
* @param	Data is to be read to data FIFO
* @param	ByteCount is the number of bytes to read.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SpiPsTpmFifoRead(XSpiPs *SpiPtr, u8* DataPtr, u8 ByteCount) {
	int Status;

	Status = SpiPsTpmTransfer(SpiPtr, XSPIPS_TPM_DATA_FIFO, NULL, DataPtr, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function write the data TPM FIFO.
*
* @param	SpiPtr is a pointer to the SPIPS driver component to use.
* @param	DataPtr is data pointer to be written to FIFO.
* @param	ByteCount is the number of bytes to read.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SpiPsTpmFifoWrite(XSpiPs *SpiPtr, u8* DataPtr, u8 ByteCount) {
	int Status;

	Status = SpiPsTpmTransfer(SpiPtr, XSPIPS_TPM_DATA_FIFO, DataPtr, NULL, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets TPM access configuration.
*
* @param	SpiPtr is a pointer to the SPIPS driver component to use.
* @param 	Address register address
* @param	TxBuf is pointer to transmit buffer
* @param	RxBuf is pointer to receive buffer
* @param	Length is transfer length
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SpiPsTpmTransfer(XSpiPs *SpiPtr, u32 Address, u8 *TxBuf, u8 *RxBuf,
		u16 Length)
{
	u16 TranLen;
	u16 RxOffset = 0;
	int Index;
	int Status;

	while (Length) {

		TranLen = (Length <= XSPIPS_TPM_SPI_MAX_SIZE) ? Length :
		XSPIPS_TPM_SPI_MAX_SIZE;

		TpmTxBuffer[0] = (RxBuf ? 0x80 : 0) | (TranLen - 1);
		TpmTxBuffer[1] = 0xD4;
		TpmTxBuffer[2] = Address >> 8;
		TpmTxBuffer[3] = Address;

		if (TxBuf != NULL) {
			for (Index = 0; Index < TranLen; Index++) {
				TpmTxBuffer[XSPIPS_TPM_TX_HEAD_SIZE + Index] = TxBuf[Index];
			}
		}

		Status = XSpiPs_PolledTransfer(SpiPtr, TpmTxBuffer, TpmRxBuffer,
				TranLen + XSPIPS_TPM_TX_HEAD_SIZE);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		if (RxBuf != NULL) {
			for (Index = 0; Index < TranLen; Index++) {
				RxBuf[Index + RxOffset] = TpmRxBuffer[XSPIPS_TPM_TX_HEAD_SIZE + Index];
			}
		}
		Length -= TranLen;
		RxOffset += TranLen;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets TPM access configuration.
*
* @param	SpiPtr is a pointer to the SPIPS driver component to use.
* @param	TxBuf is pointer to transmit buffer
* @param	RxBuf is pointer to receive buffer
* @param	Txlen is data transfer length
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SpiPsTpmDataTransfer(XSpiPs *SpiPtr, u8* TxBuf, u8* RxBuf, u16 Txlen)
{
	u8 StatusVal;
	int Status;
	u16 RxLen;

	do {
		/* Set command ready request */
		Status = SpiPsTpmStatusSet(SpiPtr, XSPIPS_TPM_STS_CMD_READY);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/* Check for command ready status */
		Status = SpiPsTpmStatusGet(SpiPtr, &StatusVal);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	} while (!(StatusVal & XSPIPS_TPM_STS_CMD_READY));

	/* Write Data to device */
	Status = SpiPsTpmFifoWrite(SpiPtr, TxBuf, Txlen);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set command to go */
	Status = SpiPsTpmStatusSet(SpiPtr, XSPIPS_TPM_STS_GO);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	do {
		/* Check for command ready and valid */
		Status = SpiPsTpmStatusGet(SpiPtr, &StatusVal);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	} while (!((StatusVal & XSPIPS_TPM_STS_VALID)
			&& (StatusVal & XSPIPS_TPM_STS_DATA_AVAIL)));

	/* Read Data from device */
	Status = SpiPsTpmFifoRead(SpiPtr, RxBuf, XSPIPS_TPM_RX_HEAD_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	RxLen = RxBuf[5];

	if(RxLen > XSPIPS_TPM_RESP_MAX_SIZE)
		return XST_FAILURE;

	RxLen = RxBuf[5] - XSPIPS_TPM_RX_HEAD_SIZE;

	if(RxLen) {
		Status = SpiPsTpmFifoRead(SpiPtr, RxBuf + XSPIPS_TPM_RX_HEAD_SIZE, RxLen);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
