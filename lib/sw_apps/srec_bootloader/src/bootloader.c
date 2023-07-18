/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

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
 *      You can include these sources in your software application project in XPS and
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

/* Defines */
#define CR       13

/* Comment the following line, if you want a smaller and faster bootloader which will be silent */
#define VERBOSE

/* Declarations */
static void display_progress (uint32 lines);
static uint8 load_exec ();
static uint8 flash_get_srec_line (uint8 *buf);
extern void init_stdout();

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
static uint8 sr_buf[SREC_MAX_BYTES];
static uint8 sr_data_buf[SREC_DATA_MAX_BYTES];

static uint8 *flbuf;

#ifdef VERBOSE
static int8 *errors[] = {
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
	uint8 ret;

	init_stdout();

#ifdef VERBOSE
	print ("\r\nSREC Bootloader\r\n");
	print ("Loading SREC image from flash @ address: ");
	putnum (FLASH_IMAGE_BASEADDR);
	print ("\r\n");
#endif

	flbuf = (uint8 *)FLASH_IMAGE_BASEADDR;
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
static void display_progress (uint32 count)
{
	/* Send carriage return */
	outbyte (CR);
	print  ("Bootloader: Processed (0x)");
	putnum (count);
	print (" S-records");
}
#endif

static uint8 load_exec ()
{
	uint8 ret;
	void (*laddr)();
	int8 done = 0;

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
	print ("\r\nExecuting program starting at address: ");
	putnum ((uint32)laddr);
	print ("\r\n");
#endif

	(*laddr)();

	/* We will be dead at this point */
	return 0;
}


static uint8 flash_get_srec_line (uint8 *buf)
{
	uint8 c;
	int count = 0;

	while (1) {
		c  = *flbuf++;
		if (c == 0xD) {
			/* Eat up the 0xA too */
			c = *flbuf++;
			return 0;
		}

		*buf++ = c;
		count++;
		if (count > SREC_MAX_BYTES) {
			return LD_SREC_LINE_ERROR;
		}
	}
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
