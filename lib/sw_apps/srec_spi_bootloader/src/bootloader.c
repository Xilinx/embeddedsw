/******************************************************************************
* Copyright (C) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/

/*
 *      Simple SREC Bootloader
 *      This simple bootloader is provided with Xilinx EDK for you to easily re-use in your
 *      own software project. It is capable of booting an SREC format image file
 *      (Mototorola S-record format), given the location of the image in memory.
 *      In particular, this bootloader is designed for images stored in non-volatile flash
 *      memory that is addressable from the processor.
 *
 *      Please modify the define "FLASH_IMAGE_BASEADDR" in the blconfig.h header file
 *      to point to the memory location from which the bootloader has to pick up the
 *      flash image from.
 *
 *      You can include these sources in your software application project in SDK and
 *      build the project for the processor for which you want the bootload to happen.
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
#include <xilisf.h>		/* Serial Flash Library header file */

/* Defines */
#define CR       13

/* Comment the following line, if you want a smaller and faster bootloader which will be silent */
#define VERBOSE

/* Declarations */
static void display_progress (uint32_t lines);
static uint8_t load_exec ();
static uint8_t flash_get_srec_line (uint8_t *buf);
extern void init_stdout();

/* Declarations for ISF/SPI */
#warning "Set your Device ID here, as defined in xparameters.h"
#define SPI_DEVICE_ID		XPAR_SPI_0_DEVICE_ID

/*
 * The following constant defines the slave select signal that is used to
 * to select the Flash device on the SPI bus, this signal is typically
 * connected to the chip select of the device.
 */
#define ISF_SPI_SELECT		0x01

/*
 * Number of bytes per page in the flash device.
 */
#define ISF_PAGE_SIZE		256


/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XIsf Isf;
static XSpi Spi;


XIsf_ReadParam ReadParam;

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
volatile static int TransferInProgress;


/*
 * The user needs to allocate a buffer to be used by the In-system and Serial
 * Flash Library to perform any read/write operations on the Serial Flash
 * device.
 * User applications must pass the address of this memory to the Library in
 * Serial Flash Initialization function, for the Library to work.
 * For Write operations:
 * - The size of this buffer should be equal to the Number of bytes to be
 * written to the Serial Flash + XISF_CMD_MAX_EXTRA_BYTES.
 * - The size of this buffer should be large enough for usage across all the
 * applications that use a common instance of the Serial Flash.
 * - A minimum of one byte and a maximum of ISF_PAGE_SIZE bytes can be written
 * to the Serial Flash, through a single Write operation.
 * The size of this buffer should be equal to XISF_CMD_MAX_EXTRA_BYTES, if the
 * application only reads from the Serial Flash (no write operations).
 */
u8 IsfWriteBuffer[XISF_CMD_SEND_EXTRA_BYTES];


/*
 * Buffer used during Read transactions.
 */
u8 ReadBuffer[ISF_PAGE_SIZE + XISF_CMD_SEND_EXTRA_BYTES];

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

static uint8_t *flbuf;

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
#ifdef __MICROBLAZE__
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
	Status = XSpi_Initialize(&Spi, SPI_DEVICE_ID);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Start the SPI driver so that interrupts and the device are enabled.
	 */
	XSpi_Start(&Spi);

	XSpi_IntrGlobalDisable(&Spi);

	/*
	 * Initialize the Serial Flash Library.
	 */
	Status = XIsf_Initialize(&Isf, &Spi, ISF_SPI_SELECT, IsfWriteBuffer);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	init_stdout();

#ifdef VERBOSE
	print ("Loading SREC image from flash @ address: ");
	putnum (FLASH_IMAGE_BASEADDR);
	print ("\r\n");
#endif

	flbuf = (uint8_t*)FLASH_IMAGE_BASEADDR;
	ret = load_exec ();

	/* If we reach here, we are in error */

#ifdef VERBOSE
	if (ret > LD_SREC_LINE_ERROR) {
		print ("ERROR in SREC line: ");
		putnum (srec_line);
		print (errors[ret]);
	} else {
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
	print  ("Bootloader: Processed (0x)");
	putnum (count);
	print (" S-records");
}
#endif

static uint8_t load_exec ()
{
	uint8_t ret;
	void (*laddr)();
	int8_t done = 0;

	srinfo.sr_data = sr_data_buf;

	while (!done) {
		if ((ret = flash_get_srec_line (sr_buf)) != 0)
			return ret;

		if ((ret = decode_srec_line (sr_buf, &srinfo)) != 0)
			return ret;
#ifdef VERBOSE
		display_progress (srec_line);
#endif
		switch (srinfo.type) {
			case SREC_TYPE_0:
				break;
			case SREC_TYPE_1:
			case SREC_TYPE_2:
			case SREC_TYPE_3:
				memcpy ((void*)srinfo.addr, (void*)srinfo.sr_data, srinfo.dlen);
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
	print ("\r\nExecuting program starting at address: ");
	putnum ((uint32_t)laddr);
	print ("\r\n");
#endif

	(*laddr)();

	/* We will be dead at this point */
	return 0;
}


static uint8_t flash_get_srec_line (uint8_t *buf)
{
	int Status;
	uint8_t c;
	int count = 0;
	int mode = XISF_CMD_SEND_EXTRA_BYTES;
	while (1) {
		TransferInProgress = TRUE;

		/*
		 * Set the
		 * - Address in the Serial Flash where the data is to be read from.
		 * - Number of bytes to be read from the Serial Flash.
		 * - Read Buffer to which the data is to be read.
		 */
		ReadParam.Address = flbuf++;
		ReadParam.NumBytes = 1;
		ReadParam.ReadPtr = ReadBuffer;

#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) || \
    (XPAR_XISF_FLASH_FAMILY == WINBOND) ||  \
    (XPAR_XISF_FLASH_FAMILY == SPANSION) || (XPAR_XISF_FLASH_FAMILY == SST))
		/*
		 * Check if the flash part is micron or spansion in which case,
		 * switch to 4 byte addressing mode if FlashSize is >128Mb.
		 */
		if (((Isf.ManufacturerID == XISF_MANUFACTURER_ID_MICRON) ||
			(Isf.ManufacturerID == XISF_MANUFACTURER_ID_SPANSION)) &&
			(((u8)Isf.DeviceCode) > XISF_SPANSION_ID_BYTE2_128)) {
			Status = XIsf_WriteEnable(&Isf, XISF_WRITE_ENABLE);
			if(Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			Status = IsfWaitForFlashNotBusy();
			if(Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			mode = XISF_CMD_SEND_EXTRA_BYTES_4BYTE_MODE;
			XIsf_MicronFlashEnter4BAddMode(&Isf);

			Status = IsfWaitForFlashNotBusy();
			if(Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
#endif
		Status = XIsf_Read(&Isf, XISF_READ, (void*) &ReadParam);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		IsfWaitForFlashNotBusy();
		c  = ReadBuffer[mode];

		if (c == 0xD) {
			/* Eat up the 0xA too */
			TransferInProgress = TRUE;
			ReadParam.Address = flbuf++;
			ReadParam.NumBytes = 1;
			ReadParam.ReadPtr = ReadBuffer;

			XIsf_Read(&Isf, XISF_READ, (void*) &ReadParam);
			IsfWaitForFlashNotBusy();
			c  = ReadBuffer[mode];

			return 0;
		}

#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) || \
    (XPAR_XISF_FLASH_FAMILY == WINBOND) ||  \
    (XPAR_XISF_FLASH_FAMILY == SPANSION) || (XPAR_XISF_FLASH_FAMILY == SST))
		/*
		 * For micron or spansion flash part supporting 4 byte addressing mode,
		 * exit from 4 Byte mode if FlashSize is >128Mb.
		 */
		if (((Isf.ManufacturerID == XISF_MANUFACTURER_ID_MICRON) ||
			(Isf.ManufacturerID == XISF_MANUFACTURER_ID_SPANSION)) &&
			(((u8)Isf.DeviceCode) > XISF_SPANSION_ID_BYTE2_128)) {
			Status = XIsf_WriteEnable(&Isf, XISF_WRITE_ENABLE);
			if(Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			Status = IsfWaitForFlashNotBusy();
			if(Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			XIsf_MicronFlashExit4BAddMode(&Isf);

			Status = IsfWaitForFlashNotBusy();
			if(Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
#endif

		*buf++ = c;
		count++;
		if (count > SREC_MAX_BYTES)
			return LD_SREC_LINE_ERROR;
	}
}

int IsfWaitForFlashNotBusy(void)
{
	int Status;
	u8 StatusReg;

	while(1) {

		/*
		 * Get the Status Register.
		 */
		TransferInProgress = TRUE;
		Status = XIsf_GetStatus(&Isf, ReadBuffer);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}


		/*
		 * Check if the Serial Flash is ready to accept the next
		 * command. If so break.
		 */
		StatusReg = ReadBuffer[BYTE2];
		if((StatusReg & XISF_SR_IS_READY_MASK) == 0) {
			break;
		}
	}

	return XST_SUCCESS;
}


#ifdef __PPC__

#include <unistd.h>

/* Save some code and data space on PowerPC
   by defining a minimal exit */
void exit (int ret)
{
	_exit (ret);
}
#endif
