/******************************************************************************
* Copyright (C) 2009 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/*
 *      Simple SREC Bootloader
 *      It is capable of booting an SREC format image file (Mototorola S-record format),
 *      given the location of the image in memory.
 *      In particular, this bootloader is designed for images stored in non-volatile flash
 *      memory that is addressable from the processor.
 *
 *      Please modify the define "FLASH_IMAGE_BASEADDR" in the blconfig.h header file
 *      to point to the memory location from which the bootloader has to pick up the
 *      flash image from.
 *
 *      You can include these sources in your software application project and build
 *      the project for the processor for which you want the bootload to happen.
 *      You can also subsequently modify these sources to adapt the bootloader for any
 *      specific scenario that you might require it for.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blconfig.h"
#include "portab.h"
#include "errors.h"
#include "srec.h"
#include "xparameters.h"
#include "xspi.h"

/* Defines */
#define CR       13
#define RECORD_TYPE	2
#define BYTE_COUNT	2
#define RECORD_TERMINATOR	2

/* Comment the following line, if you want a smaller and faster bootloader which will be silent */
#define VERBOSE

/* Declarations */
static void display_progress (uint32_t lines);
static uint8_t load_exec ();
static uint8_t flash_get_srec_line (uint8_t *buf);
extern void init_stdout();
uint8  grab_hex_byte (uint8 *buf);

/*
 * The following constant defines the slave select signal that is used to
 * to select the Flash device on the SPI bus, this signal is typically
 * connected to the chip select of the device.
 */
#define SPI_SELECT		0x01

/*
 * Number of bytes per page in the flash device.
 */
#define PAGE_SIZE		256

/*
 * Byte Positions.
 */
#define BYTE1				0 /* Byte 1 position */
#define BYTE2				1 /* Byte 2 position */
#define BYTE3				2 /* Byte 3 position */
#define BYTE4				3 /* Byte 4 position */
#define BYTE5				4 /* Byte 5 position */

#define READ_WRITE_EXTRA_BYTES		4 /* Read/Write extra bytes */
#define	READ_WRITE_EXTRA_BYTES_4BYTE_MODE	5 /**< Command extra bytes */

#define RD_ID_SIZE					4

#define ISSI_ID_BYTE0			0x9D
#define MICRON_ID_BYTE0			0x20

#define ENTER_4B_ADDR_MODE		0xb7 /* Enter 4Byte Mode command */
#define EXIT_4B_ADDR_MODE		0xe9 /* Exit 4Byte Mode command */
#define EXIT_4B_ADDR_MODE_ISSI	0x29
#define	WRITE_ENABLE			0x06 /* Write Enable command */

#define ENTER_4B	1
#define EXIT_4B		0

#define	FLASH_16_MB	0x18
#define FLASH_MAKE		0
#define	FLASH_SIZE		2

#define	READ_CMD	0x03

/* Declarations */
static void display_progress (uint32_t lines);
static uint8_t load_exec ();
static uint8_t flash_get_srec_line (uint8_t *buf);
extern void init_stdout();
uint8  grab_hex_byte (uint8 *buf);
int FlashReadID(void);

#if defined (SDT)
#define SPI_DEVICE_BASEADDR     XPAR_AXI_QUAD_SPI_0_BASEADDR
#else
#define SPI_DEVICE_ID		XPAR_SPI_0_DEVICE_ID
#endif

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XSpi Spi;


int mode = READ_WRITE_EXTRA_BYTES;

u8 WriteBuffer[PAGE_SIZE + READ_WRITE_EXTRA_BYTES];
/*
 * Buffer used during Read transactions.
 */
u8 ReadBuffer[PAGE_SIZE + READ_WRITE_EXTRA_BYTES];

u8 FlashID[3];

extern int srec_line;

#ifdef __cplusplus
extern "C" {
#endif

extern void outbyte(char c);

#ifdef __cplusplus
}
#endif

/* Data structures */
static srec_info_t srinfo;
static uint8_t sr_buf[SREC_MAX_BYTES];
static uint8_t sr_data_buf[SREC_DATA_MAX_BYTES];

u32 flbuf;

#ifdef VERBOSE
static int8_t *errors[] = {
	"",
	"Error while copying executable image into RAM",
	"Error while reading an SREC line from flash",
	"SREC line is corrupted",
	"SREC has invalid checksum."
};
#endif

/* We don't use interrupts/exceptions.
   Dummy definitions to reduce code size on MicroBlaze */
#if defined (__MICROBLAZE__) || defined (__riscv)
void _interrupt_handler () {}
void _exception_handler () {}
void _hw_exception_handler () {}
#endif


int main()
{
	int Status;
	uint8_t ret;

#ifdef VERBOSE
	print ("\r\nSREC SPI Bootloader\r\n");
#endif

	/*
	 * Initialize the SPI driver so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h.
	 */
#if defined (SDT)
	Status = XSpi_Initialize(&Spi, SPI_DEVICE_BASEADDR);
#else
	Status = XSpi_Initialize(&Spi, SPI_DEVICE_ID);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the SPI device as a master and in manual slave select mode such
	 * that the slave select signal does not toggle for every byte of a
	 * transfer, this must be done before the slave select is set.
	 */
	Status = XSpi_SetOptions(&Spi, XSP_MASTER_OPTION |
				 XSP_MANUAL_SSELECT_OPTION);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Select the flash device on the SPI bus, so that it can be
	 * read and written using the SPI bus.
	 */
	Status = XSpi_SetSlaveSelect(&Spi, SPI_SELECT);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the SPI driver so that interrupts and the device are enabled.
	 */
	XSpi_Start(&Spi);

	XSpi_IntrGlobalDisable(&Spi);

	init_stdout();

	Status = FlashReadID( );
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifdef VERBOSE
	xil_printf("Loading SREC image from flash @ address: %x\r\n",FLASH_IMAGE_BASEADDR);
#endif

	flbuf = (u32)FLASH_IMAGE_BASEADDR;
	ret = load_exec ();

	/* If we reach here, we are in error */

#ifdef VERBOSE
	if (ret > LD_SREC_LINE_ERROR) {
		xil_printf("ERROR in SREC line: %x%s",srec_line,errors[ret]);
	}
	else {
		print ("ERROR: ");
		print (errors[ret]);
	}
#endif

	return ret;
}

/*****************************************************************************/
/**
*
* This function enables writes to the Serial Flash memory.
*
* @param	Spi is a pointer to the instance of the Spi device.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int FlashWriteEnable(XSpi *Spi)
{
	int Status;
	u8 *NULLPtr = NULL;

	/*
	 * Prepare the WriteBuffer.
	 */
	WriteBuffer[BYTE1] = WRITE_ENABLE;

	Status = XSpi_Transfer(Spi, WriteBuffer, NULLPtr, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief
 * This API enters the flash device into 4 bytes addressing mode.
 *
 * @param	Spi is a pointer to the instance of the Spi device.
 * @param	Enable is a either 1 or 0 if 1 then enters 4 byte if 0 exits.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *		- XST_FAILURE if it fails.
 *
 *
 ******************************************************************************/
int FlashEnterExit4BAddMode(XSpi *Spi, unsigned int Enable)
{
	int Status;
	u8 *NULLPtr = NULL;

	if ((FlashID[FLASH_MAKE] == MICRON_ID_BYTE0) ||
	    (FlashID[FLASH_MAKE] == ISSI_ID_BYTE0)) {

		Status = FlashWriteEnable(Spi);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	if (Enable) {
		WriteBuffer[BYTE1] = ENTER_4B_ADDR_MODE;
	}
	else {
		if (FlashID[FLASH_MAKE] == ISSI_ID_BYTE0) {
			WriteBuffer[BYTE1] = EXIT_4B_ADDR_MODE_ISSI;
		}
		else {
			WriteBuffer[BYTE1] = EXIT_4B_ADDR_MODE;
		}
	}

	Status = XSpi_Transfer(Spi, WriteBuffer, NULLPtr, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This function reads serial FLASH ID connected to the SPI interface.
*
* @param	None.
*
* @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashReadID(void)
{
	int Status;
	int i;

	/* Read ID in Auto mode.*/
	WriteBuffer[BYTE1] = 0x9f;
	WriteBuffer[BYTE2] = 0xff;		/* 4 dummy bytes */
	WriteBuffer[BYTE3] = 0xff;
	WriteBuffer[BYTE4] = 0xff;
	WriteBuffer[BYTE5] = 0xff;

	Status = XSpi_Transfer(&Spi, WriteBuffer, ReadBuffer, 5);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	for (i = 0; i < 3; i++) {
		FlashID[i] = ReadBuffer[i + 1];
	}
#ifdef VERBOSE
	xil_printf("FlashID=0x%x 0x%x 0x%x\n\r", ReadBuffer[1], ReadBuffer[2],
		   ReadBuffer[3]);
#endif
	return XST_SUCCESS;
}

#ifdef VERBOSE
static void display_progress (uint32_t count)
{
	/* Send carriage return */
	outbyte (CR);
	xil_printf("Bootloader: Processed (0x)%x S-records",count);
}
#endif

static uint8_t load_exec ()
{
	uint8_t ret;
	void (*laddr)();
	int8_t done = 0;
	int Status;

	srinfo.sr_data = sr_data_buf;

	if (FlashID[FLASH_SIZE] > FLASH_16_MB) {
		Status = FlashEnterExit4BAddMode(&Spi, ENTER_4B);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		mode = READ_WRITE_EXTRA_BYTES_4BYTE_MODE;
	}
	while (!done) {
		if ((ret = flash_get_srec_line (sr_buf)) != 0) {
			return ret;
		}

		if ((ret = decode_srec_line (sr_buf, &srinfo)) != 0) {
			return ret;
		}

#ifdef VERBOSE
		display_progress (srec_line);
#endif
		switch (srinfo.type) {
			case SREC_TYPE_0:
				break;
			case SREC_TYPE_1:
			case SREC_TYPE_2:
			case SREC_TYPE_3:
				memcpy ((void *)srinfo.addr, (void *)srinfo.sr_data, srinfo.dlen);
				break;
			case SREC_TYPE_5:
				break;
			case SREC_TYPE_7:
			case SREC_TYPE_8:
			case SREC_TYPE_9:
				laddr = (void (*)())srinfo.addr;
				done = 1;
				ret = 0;
				break;
		}
	}

	if (FlashID[FLASH_SIZE] > FLASH_16_MB) {
		Status = FlashEnterExit4BAddMode(&Spi, EXIT_4B);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		mode = READ_WRITE_EXTRA_BYTES;
	}
#ifdef VERBOSE
	xil_printf("\r\nExecuting program starting at address: %x\r\n",(uint32_t)laddr);
#endif
	(*laddr)();

	/* We will be dead at this point */
	return 0;
}

static uint8_t flash_get_srec_line (uint8_t *buf)
{
	int Status;
	int i;
	int len;
	u8 ReadCmd = READ_CMD;

	/*
	 * Read 1st 4bytes of a record. Its contains the information about
	 * the type of the record and number of bytes that follow in the
	 * rest of the record (address + data + checksum).
	 */
	if (mode == READ_WRITE_EXTRA_BYTES) {
		WriteBuffer[BYTE1] = ReadCmd;
		WriteBuffer[BYTE2] = (u8) (flbuf >> 16);
		WriteBuffer[BYTE3] = (u8) (flbuf >> 8);
		WriteBuffer[BYTE4] = (u8) flbuf;
	}
	else {
		WriteBuffer[BYTE1] = ReadCmd;
		WriteBuffer[BYTE2] = (u8) (flbuf >> 24);
		WriteBuffer[BYTE3] = (u8) (flbuf >> 16);
		WriteBuffer[BYTE4] = (u8) (flbuf >> 8);
		WriteBuffer[BYTE5] = (u8) flbuf;
	}

	Status = XSpi_Transfer(&Spi, WriteBuffer, ReadBuffer,
			       (RECORD_TYPE + BYTE_COUNT + mode));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	flbuf += RECORD_TYPE + BYTE_COUNT;

	/*
	 * Get the number of bytes (address + data + checksum) in a record.
	 */
	len = grab_hex_byte((ReadBuffer + mode + RECORD_TYPE)) * 2;

	for (i = 0; i < (RECORD_TYPE + BYTE_COUNT); i++) {
		*buf++ = ReadBuffer[mode + i];
	}

	/*
	 * Read address + data + checksum from the record.
	 */
	if (mode == READ_WRITE_EXTRA_BYTES) {
		WriteBuffer[BYTE1] = ReadCmd;
		WriteBuffer[BYTE2] = (u8) (flbuf >> 16);
		WriteBuffer[BYTE3] = (u8) (flbuf >> 8);
		WriteBuffer[BYTE4] = (u8) flbuf;
	}
	else {
		WriteBuffer[BYTE1] = ReadCmd;
		WriteBuffer[BYTE2] = (u8) (flbuf >> 24);
		WriteBuffer[BYTE3] = (u8) (flbuf >> 16);
		WriteBuffer[BYTE4] = (u8) (flbuf >> 8);
		WriteBuffer[BYTE5] = (u8) flbuf;
	}

	Status = XSpi_Transfer(&Spi, WriteBuffer, ReadBuffer, (len + mode));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	flbuf += (len + RECORD_TERMINATOR);

	for (i = 0; i < len; i++) {
		*buf++ = ReadBuffer[mode + i];
	}

	if ((RECORD_TYPE + BYTE_COUNT + len) > SREC_MAX_BYTES) {
		return LD_SREC_LINE_ERROR;
	}

	return 0;

}
#ifdef __PPC__

#include <unistd.h>

/* Save some code and data space on PowerPC
   by defining a minimal exit */
void exit (int ret):
{
	_exit (ret);
}
#endif
