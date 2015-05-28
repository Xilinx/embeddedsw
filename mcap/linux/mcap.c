/******************************************************************************
* Copyright (C) 2014-2015 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file mcap.c
* MCAP Interface Library Test Application
*
******************************************************************************/

#include "mcap_lib.h"

static const char options[] = "x:pC:rmfdvHhDa::";
static char help_msg[] =
"Usage: mcap [options]\n"
"\n"
"Options:\n"
"\t-x\t\tSpecify MCAP Device Id in hex (MANDATORY)\n"
"\t-p    <file>\tProgram Bitstream (.bin/.bit/.rbt)\n"
"\t-C    <file>\tPartial Reconfiguration Clear File(.bin/.bit/.rbt)\n"
"\t-r\t\tPerforms Simple Reset\n"
"\t-m\t\tPerforms Module Reset\n"
"\t-f\t\tPerforms Full Reset\n"
"\t-D\t\tRead Data Registers\n"
"\t-d\t\tDump all the MCAP Registers\n"
"\t-v\t\tVerbose information of MCAP Device\n"
"\t-h/H\t\tHelp\n"
"\t-a <address> [type [data]]  Access Device Configuration Space\n"
"\t\t      here type[data] - b for byte data [8 bits]\n"
"\t\t      here type[data] - h for half word data [16 bits]\n"
"\t\t      here type[data] - w for word data [32 bits]\n"
"\n"
;

int main(int argc, char **argv)
{
	struct mcap_dev *mdev;
	int i, modreset = 0, fullreset = 0, reset = 0;
	int program = 0, verbose = 0, device_id = 0;
	int data_regs = 0, dump_regs = 0, access_config = 0;
	int programconfigfile = 0;

	while ((i = getopt(argc, argv, options)) != -1) {
		switch (i) {
		case 'a':
			access_config = 1;
			break;
		case 'd':
			dump_regs = 1;
			break;
		case 'm':
			modreset = 1;
			break;
		case 'f':
			fullreset = 1;
			break;
		case 'r':
			reset = 1;
			break;
		case 'D':
			data_regs = 1;
			break;
		case 'h':
		case 'H':
			printf("%s", help_msg);
			return 1;
		case 'C':
			programconfigfile = 1;
			break;
		case 'p':
			program = 1;
			break;
		case 'v':
			verbose++;
			break;
		case 'x':
			device_id = (int) strtol(argv[2], NULL, 16);
			break;
		default:
			printf("%s", help_msg);
			return 1;
		}
	}

	if (!device_id) {
		printf("No device id specified...\n");
		printf("%s", help_msg);
		return 1;
	}

	mdev = (struct mcap_dev *)MCapLibInit(device_id);
	if (!mdev)
		return 1;

	if (verbose) {
		MCapShowDevice(mdev, verbose);
		goto free;
	}

	if (access_config) {
		if (argc < 6) {
			printf("%s", help_msg);
			goto free;
		}
		if (MCapAccessConfigSpace(mdev, argc, argv))
			printf("%s", help_msg);
		goto free;
	}

	if (fullreset) {
		MCapFullReset(mdev);
		goto free;
	}

	if (modreset) {
		MCapModuleReset(mdev);
		goto free;
	}

	if (reset) {
		MCapReset(mdev);
		goto free;
	}

	if (programconfigfile) {
		if (argc > 6)
			mdev->is_multiplebit = 1;

		MCapConfigureFPGA(mdev, argv[4], EMCAP_PARTIALCONFIG_FILE);

		if(!mdev->is_multiplebit)
			goto free;
	}

	if (program) {
		if (argc > 6)
			MCapConfigureFPGA(mdev, argv[6], EMCAP_CONFIG_FILE);
		else
			MCapConfigureFPGA(mdev, argv[4], EMCAP_CONFIG_FILE);
		goto free;
	}

	if (dump_regs) {
		MCapDumpRegs(mdev);
		goto free;
	}

	if (data_regs) {
		MCapDumpReadRegs(mdev);
		goto free;
	}

	if (i == -1 && argc == 1)
		MCapShowDevice(mdev, 0);

free:
	MCapLibFree(mdev);

	return 0;
}
