/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/


/******************************************************************************/
/**
 *
 * @file xilsfl_ospi_flash_readwrite_example.c
 *
 *
 * This file contains a design example using the SFL library
 * The hardware which this example runs on, must have an octal
 * serial nor flash for it to run.
 *
 * This example has been tested with the Micron Octal Serial Flash(MT35XU02GCBA).
 *
 * @note
 *
 * None.
 *

 *</pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------
 * 1.0   sb  8/20/24   Initial release
 *
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xilsfl.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define OSPI_BASEADDR		XPAR_XOSPIPSV_0_BASEADDR

/*
 * Number of flash pages to be written.
 */
#define PAGE_COUNT		32
/*
 * Max page size to initialize write and read buffer
 */
#define MAX_PAGE_SIZE 1024

/*
 * Flash address to which data is to be written.
 * This test address should be aligned with sector offest address.
 */
#define TEST_ADDRESS		0x00

#define UNIQUE_VALUE		0x0A

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int SflReadWriteExample(void);

/************************** Variable Definitions *****************************/
u8 SflHandler;     /* file descriptor for the XSfl instance*/

/*
 * The following variables are used to read and write to the flash and they
 * are global to avoid having large buffers on the stack
 */
#ifdef __ICCARM__
#pragma data_alignment = 64
u8 ReadBuffer[(PAGE_COUNT * MAX_PAGE_SIZE)];
#pragma data_alignment = 4
u8 WriteBuffer[(PAGE_COUNT * MAX_PAGE_SIZE)];
#else
u8 ReadBuffer[(PAGE_COUNT * MAX_PAGE_SIZE)] __attribute__ ((aligned(64)));
u8 WriteBuffer[(PAGE_COUNT * MAX_PAGE_SIZE)] __attribute__ ((aligned(4)));
#endif

/*
 * The following variable specify the max amount of data and the size of the
 * the  page size to transfer the data to
 * and from the Flash. Initialized to single flash page size.
 */
u32 MaxData;

/*****************************************************************************/
/**
 *
 * Main function to call the SflReadWriteExample.
 *
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note		None
 *
 ******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("SflReadWriteExample Test\r\n");

	/*
	 * Run the SflReadWrite Example.
	 */
	Status = SflReadWriteExample();
	if (Status != XST_SUCCESS) {
		xil_printf("SflReadWriteExample Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SflReadWriteExample\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * The purpose of this function is to illustrate how to use the SFL
 * Library with SPI/QSPI/OSPI interfaced with flash devices.
 *
 * @return	XST_SUCCESS if successful, else XST_FAILURE.
 *
 * @note		None.
 *
 *****************************************************************************/
int SflReadWriteExample(void) {
	int Status;
	u8 UniqueValue;
	u32 Count;
	u32 SectSize;
	u32 ByteCount;
	u64 RxAddr64bit = 0;

	u32 ReadBfrSize ;
	XSfl_UserConfig SflUserOptions;

	SflUserOptions.Ospi_Config.ChipSelect = XSFL_SELECT_FLASH_CS0;
	SflUserOptions.Ospi_Config.BaseAddress = OSPI_BASEADDR;
	SflUserOptions.Ospi_Config.ReadMode = XOSPIPSV_IDAC_EN_OPTION;

	Status = XSfl_FlashInit(&SflHandler, SflUserOptions, XSFL_OSPI_CNTRL);
	if (Status != XST_SUCCESS ) {
		return XST_FAILURE;
	}

	Status = XSfl_FlashGetInfo(SflHandler, XSFL_SECT_SIZE, &SectSize);
	if (Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	/*  Set max data for Erase, Write and read operations */
	MaxData = PAGE_COUNT * MAX_PAGE_SIZE;
	ReadBfrSize = MaxData;

	for (UniqueValue = UNIQUE_VALUE, Count = 0;
			Count < MaxData;
			Count++, UniqueValue++) {
		WriteBuffer[Count] = (u8)(UniqueValue);
	}

	/*
	 * Erase byte count and address should be align with sector size.
	 */
	ByteCount = SectSize;

	Status = XSfl_FlashErase(SflHandler, TEST_ADDRESS, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSfl_FlashWrite(SflHandler, TEST_ADDRESS, MaxData, WriteBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	for (Count = 0; Count < ReadBfrSize; Count++) {
		ReadBuffer[Count] = 0;
	}

	Status = XSfl_FlashReadStart(SflHandler, TEST_ADDRESS, MaxData, ReadBuffer, RxAddr64bit);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSfl_FlashReadDone(SflHandler);
	while (Status != XST_SUCCESS) {
		Status = XSfl_FlashReadDone(SflHandler);
	}

	/*
	 * Setup a pointer to the start of the data that was read into the read
	 * buffer and verify the data read is the data that was written
	 */
	for (Count = 0; Count < MaxData;
			Count++) {
		if (ReadBuffer[Count] != WriteBuffer[Count]) {
			return XST_FAILURE;
		}
	}

	return Status;
}
