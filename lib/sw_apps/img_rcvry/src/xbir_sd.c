/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xbir_sd.c
*
* This is the main file which will contain the SD initialization, read and
* write functions.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.00  bsv   07/25/21   First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsdps.h"
#include "xbir_sd.h"
#include "xbir_config.h"
#include "xbir_err.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define XBIR_SD_DEVICE_ID	XPAR_XSDPS_1_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XBIR_SDPS_BLOCK_SIZE		(512U)
#define XBIR_SDPS_WRITE_CHUNK_SIZE	(0x200000U)
#define XBIR_SD_RAW_NUM_SECTORS		(XBIR_SDPS_WRITE_CHUNK_SIZE / \
	XBIR_SDPS_BLOCK_SIZE)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XSdPs SdInstance;

/*****************************************************************************/
/**
 * @brief
 * This function is used to initialize SD driver
 *
 * @return	XST_SUCCESS on successful initialization
 *		Error code on failure
 *
 *****************************************************************************/
int Xbir_SdInit(void)
{
	int Status = XST_FAILURE;
	XSdPs_Config *SdConfig;

	/* Initialize the SD driver so that it's ready to use */
	SdConfig =  XSdPs_LookupConfig(XBIR_SD_DEVICE_ID);
	if (NULL == SdConfig) {
		Status = XBIR_ERROR_SD_CONFIG;
		goto END;
	}

	Status =  XSdPs_CfgInitialize(&SdInstance, SdConfig,
			SdConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_SD_CONFIG_INIT;
		goto END;
	}

	Status = XSdPs_CardInitialize(&SdInstance);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_SD_CARD_INIT;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function is used to copy the data from SD card to destination
 * address.
 *
 * @param 	SrcAddr Address of SD card where copy should start from
 * @param 	DestAddr Pointer to destination where it should copy to
 * @param	Length Number of data bytes to be copied
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int Xbir_SdRead(u64 SrcAddr, u8* DestAddr, u64 Length)
{
	int Status = XST_FAILURE;
	u16 NumOfBlocks = (Length / XBIR_SDPS_BLOCK_SIZE);
	u64 BlockNumber = (SrcAddr / XBIR_SDPS_BLOCK_SIZE);

	Status  = XSdPs_ReadPolled(&SdInstance, BlockNumber, NumOfBlocks,
		(u8*)DestAddr);

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function writes data in Write Buffer to SD card.
 *
 * @param	WrBuffer Pointer to data to be written
 * @param	Length  Number of bytes to write
 *
 * @return	XST_SUCCESS on successful write
 * 		Error code on failure
 *
 ******************************************************************************/
int Xbir_SdWrite(u8 *WrBuffer, u64 Length)
{
	int Status = XST_FAILURE;
	u64 NumBlocks = Length / XBIR_SDPS_BLOCK_SIZE;
	u64 BlockIndex;

	for (BlockIndex = 0UL; BlockIndex < NumBlocks;
		BlockIndex += XBIR_SD_RAW_NUM_SECTORS) {
		Status = XSdPs_WritePolled(&SdInstance, BlockIndex,
			XBIR_SD_RAW_NUM_SECTORS, WrBuffer);
		if (Status != XST_SUCCESS) {
			Status = XBIR_ERROR_SD_WRITE;
			goto END;
		}
		WrBuffer += XBIR_SDPS_WRITE_CHUNK_SIZE;

	}
	if ((Length % XBIR_SDPS_BLOCK_SIZE) != 0L) {
		Status = XSdPs_WritePolled(&SdInstance, BlockIndex, 1U,
			WrBuffer);
		if (Status != XST_SUCCESS) {
			Status = XBIR_ERROR_SD_WRITE;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function erases given sectors in SD card from the start.
 *
 * @param	Length  Number of bytes to erase
 *
 * @return	XST_SUCCESS on successful write
 * 		Error code on failure
 *
 ******************************************************************************/
int Xbir_SdErase(u64 Length)
{
	int Status = XST_FAILURE;
	u64 NumBlocks = Length / XBIR_SDPS_BLOCK_SIZE;
	u64 BlockIndex;
	u8 WrBuffer[XBIR_SDPS_WRITE_CHUNK_SIZE] = {0U};

	for (BlockIndex = 0UL; BlockIndex < NumBlocks;
		BlockIndex += XBIR_SD_RAW_NUM_SECTORS) {
		Status = XSdPs_WritePolled(&SdInstance, BlockIndex,
			XBIR_SD_RAW_NUM_SECTORS, WrBuffer);
		if (Status != XST_SUCCESS) {
			Xbir_Printf("%0x..\n\r", BlockIndex);
			Status = XBIR_ERROR_SD_ERASE;
			goto END;
		}
	}
	if ((Length % XBIR_SDPS_BLOCK_SIZE) != 0L) {
		Status = XSdPs_WritePolled(&SdInstance, BlockIndex, 1U,
			WrBuffer);
		if (Status != XST_SUCCESS) {
			Status = XBIR_ERROR_SD_ERASE;
		}
	}

END:
	return Status;
}