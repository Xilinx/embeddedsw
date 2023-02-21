/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xbir_sd.h"
#if (defined(XBIR_SD_0) || defined(XBIR_SD_1))
#include "xsdps.h"
#include "xbir_config.h"
#include "xbir_err.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

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
	SdConfig =  XSdPs_LookupConfig(XPAR_XSDPS_0_DEVICE_ID);
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
		goto END;
	}

	Status = XSdPs_Set_Mmc_ExtCsd(&SdInstance, XSDPS_MMC_PART_CFG_0_ARG);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_MMC_PART_CONFIG;
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
int Xbir_SdRead(u32 SrcAddr, u8* DestAddr, u32 Length)
{
	int Status = XST_FAILURE;
	u16 NumOfBlocks = (Length / XBIR_SDPS_BLOCK_SIZE);
	u32 BlockNumber = (SrcAddr / XBIR_SDPS_BLOCK_SIZE);

	Status  = XSdPs_ReadPolled(&SdInstance, BlockNumber, NumOfBlocks,
		(u8*)DestAddr);
	if (Status != XST_SUCCESS) {
		Status = XBIR_ERROR_SD_READ;
		goto END;
	}

	if ((Length % XBIR_SDPS_BLOCK_SIZE) != 0U) {
		BlockNumber += NumOfBlocks;
		DestAddr += NumOfBlocks * XBIR_SDPS_BLOCK_SIZE;
		Status = XSdPs_ReadPolled(&SdInstance, BlockNumber, 1U,
			DestAddr);
		if (Status != XST_SUCCESS) {
			Status = XBIR_ERROR_SD_READ;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function writes data in Write Buffer to SD card.
 *
 * @param	Offset  Starting offset to write
 * @param	WrBuffer Pointer to data to be written
 * @param	Length  Number of bytes to write
 *
 * @return	XST_SUCCESS on successful write
 * 		Error code on failure
 *
 ******************************************************************************/
int Xbir_SdWrite(u32 Offset, u8 *WrBuffer, u32 Length)
{
	int Status = XST_FAILURE;
	u64 NumBlocks = Length / XBIR_SDPS_BLOCK_SIZE;
	u32 BlockIndex;

	Offset /= XBIR_SDPS_BLOCK_SIZE;
	for (BlockIndex = 0U; BlockIndex < NumBlocks;
		BlockIndex += XBIR_SD_RAW_NUM_SECTORS) {
		Status = XSdPs_WritePolled(&SdInstance, (Offset + BlockIndex),
			XBIR_SD_RAW_NUM_SECTORS, WrBuffer);
		if (Status != XST_SUCCESS) {
			Status = XBIR_ERROR_SD_WRITE;
			goto END;
		}
		WrBuffer += XBIR_SDPS_CHUNK_SIZE;
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
 * @param	Offset  Starting offset to erase
 *
 * @return	XST_SUCCESS on successful write
 * 		Error code on failure
 *
 ******************************************************************************/
int Xbir_SdErase(u32 Offset, u32 Length)
{
	int Status = XST_FAILURE;

	Status = XSdPs_Erase(&SdInstance, Offset, (Offset + Length));

	return Status;
}

#endif /* end of XBIR_SD */
