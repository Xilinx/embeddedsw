/******************************************************************************
 * Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/*
 * Simple OSPI SREC Bootloader
 * It is capable of booting an SREC format image file (Mototorola S-record format),
 * given the location of the image in memory.
 * In particular, this bootloader is designed for images stored in non-volatile flash
 * memory that is addressable from the processor.
 *
 * Please modify the define "FLASH_IMAGE_BASEADDR" in the blconfig.h header file
 * to point to the memory location from which the bootloader has to pick up the
 * flash image from.
 *
 * You can include these sources in your software application project and build
 * the project for the processor for which you want the bootload to happen.
 * You can also subsequently modify these sources to adapt the bootloader for any
 * specific scenario that you might require it for.
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
#include "xilsfl.h"

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define OSPI_BASEADDR           XPAR_XOSPIPSV_0_BASEADDR

/* Defines */
#define CR                13
#define RECORD_TYPE        2
#define BYTE_COUNT         2
#define RECORD_TERMINATOR  2

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



u8 SflHandler;     /* file descriptor for the XSfl instance*/

int mode = READ_WRITE_EXTRA_BYTES;

u8 WriteBuffer[PAGE_SIZE + READ_WRITE_EXTRA_BYTES];
/*
 * Buffer used during Read transactions.
 */
u8 ReadBuffer[PAGE_SIZE + READ_WRITE_EXTRA_BYTES];

u32 FlashID;

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
static const int8 *errors[] = {
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
	XSfl_UserConfig SflUserOptions;

	SflUserOptions.Ospi_Config.ChipSelect = XSFL_SELECT_FLASH_CS0;
	SflUserOptions.Ospi_Config.BaseAddress = OSPI_BASEADDR;
	SflUserOptions.Ospi_Config.ReadMode = XOSPIPSV_IDAC_EN_OPTION;
#if defined(SPARTANUP)
	SflUserOptions.Ospi_Config.Quirks = XSFL_BROKEN_DMA;
#endif

#ifdef VERBOSE
	print("\r\nSREC OSPI Bootloader\r\n");
#endif
	Status = XSfl_FlashInit(&SflHandler, SflUserOptions, XSFL_OSPI_CNTRL);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	init_stdout();

	Status = XSfl_FlashGetInfo(SflHandler, XSFL_FLASH_ID, &FlashID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifdef VERBOSE
	xil_printf("FlashID=0x%x\n\r", FlashID);
#endif

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

	srinfo.sr_data = sr_data_buf;

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
	u64 RxAddr64bit = 0;

	Status = XSfl_FlashRead(SflHandler, flbuf, (RECORD_TYPE + BYTE_COUNT), ReadBuffer, RxAddr64bit);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	flbuf += RECORD_TYPE + BYTE_COUNT;

	/*
	 * Get the number of bytes (address + data + checksum) in a record.
	 */
	len = grab_hex_byte((ReadBuffer + RECORD_TYPE)) * 2;

	for (i = 0; i < (RECORD_TYPE + BYTE_COUNT); i++) {
		*buf++ = ReadBuffer[i];
	}

	Status = XSfl_FlashRead(SflHandler, flbuf, len, ReadBuffer, RxAddr64bit);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	flbuf += (len + RECORD_TERMINATOR);

	for (i = 0; i < len; i++) {
		*buf++ = ReadBuffer[i];
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
