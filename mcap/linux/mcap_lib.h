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
* @file mcap_lib.h
*  MCAP Interface Library support header file
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "pci.h"
#include "lspci.h"
#include "byteswap.h"

/* Register Definitions */
#define MCAP_EXT_CAP_HEADER	0x00
#define MCAP_VEND_SPEC_HEADER	0x04
#define MCAP_FPGA_JTAG_ID	0x08
#define MCAP_FPGA_BIT_VERSION	0x0C
#define MCAP_STATUS		0x10
#define MCAP_CONTROL		0x14
#define MCAP_DATA		0x18
#define MCAP_READ_DATA_0	0x1C
#define MCAP_READ_DATA_1	0x20
#define MCAP_READ_DATA_2	0x24
#define MCAP_READ_DATA_3	0x28

#define MCAP_CTRL_MODE_MASK		(1 << 0)
#define MCAP_CTRL_REG_READ_MASK		(1 << 1)
#define MCAP_CTRL_RESET_MASK		(1 << 4)
#define MCAP_CTRL_MOD_RESET_MASK	(1 << 5)
#define MCAP_CTRL_IN_USE_MASK		(1 << 8)
#define MCAP_CTRL_DESIGN_SWITCH_MASK	(1 << 12)
#define MCAP_CTRL_DATA_REG_PROT_MASK	(1 << 16)

#define MCAP_STS_ERR_MASK		(1 << 0)
#define MCAP_STS_EOS_MASK		(1 << 1)
#define MCAP_STS_REG_READ_CMP_MASK	(1 << 4)
#define MCAP_STS_REG_READ_COUNT_MASK	(7 << 5)
#define MCAP_STS_FIFO_OVERFLOW_MASK	(1 << 8)
#define MCAP_STS_FIFO_OCCUPANCY_MASK	(15 << 12)
#define MCAP_STS_CFG_MCAP_REQ_MASK	(1 << 24)

/* Maximum FIFO Depth */
#define MCAP_FIFO_DEPTH		16

/* PCIe Extended Capability Id */
#define MCAP_EXT_CAP_ID		0xB

/* Error Values */
#define EMCAPREQ	120
#define EMCAPRESET	121
#define EMCAPMODRESET	122
#define EMCAPFULLRESET	123
#define EMCAPWRITE	124
#define EMCAPREAD	125
#define EMCAPCFG	126
#define EMCAPBUSWALK	127
#define EMCAPCFGACC	128

#define EMCAP_EOS_RETRY_COUNT 10
#define EMCAP_EOS_LOOP_COUNT 100
#define EMCAP_NOOP_VAL	0x2000000

/* Bitfile Type */
#define EMCAP_CONFIG_FILE	 0
#define EMCAP_PARTIALCONFIG_FILE 1
#undef DEBUG

#ifndef DEBUG
#define pr_dbg(fmt, ...)	do {} while (0)
#else
#define pr_dbg	printf
#endif

#define pr_info printf
#define pr_err	printf

/* MCAP Device Information */
struct mcap_dev {
	struct pci_dev *pdev;
	struct pci_access *pacc;
	unsigned int reg_base;
	u32 is_multiplebit;
};

#define MCapRegWrite(mdev, offset, value) \
	pci_write_long(mdev->pdev, mdev->reg_base + offset, value)

#define MCapRegRead(mdev, offset) \
	pci_read_long(mdev->pdev, mdev->reg_base + offset)

#define IsResetSet(mdev) \
	(MCapRegRead(mdev, MCAP_CONTROL) & \
		MCAP_CTRL_RESET_MASK ? 1 : 0)

#define IsModuleResetSet(mdev) \
	(MCapRegRead(mdev, MCAP_CONTROL) & \
		MCAP_CTRL_MOD_RESET_MASK ? 1 : 0)

#define IsConfigureMCapReqSet(mdev) \
	(MCapRegRead(mdev, MCAP_STATUS) & \
		MCAP_STS_CFG_MCAP_REQ_MASK ? 1 : 0)

#define IsErrSet(mdev) \
	(MCapRegRead(mdev, MCAP_STATUS) & \
		MCAP_STS_ERR_MASK ? 1 : 0)

#define IsRegReadComplete(mdev) \
	(MCapRegRead(mdev, MCAP_STATUS) & \
		MCAP_STS_REG_READ_CMP_MASK ? 1 : 0)

#define IsFifoOverflow(mdev) \
	(MCapRegRead(mdev, MCAP_STATUS) & \
		MCAP_STS_FIFO_OVERFLOW_MASK ? 1 : 0)

#define GetRegReadCount(mdev) \
	((MCapRegRead(mdev, MCAP_STATUS) & \
		MCAP_STS_REG_READ_COUNT_MASK) >> 5)

/* Function Prototypes */
struct mcap_dev *MCapLibInit(int device_id);
void MCapLibFree(struct mcap_dev *mdev);
void MCapDumpRegs(struct mcap_dev *mdev);
void MCapDumpReadRegs(struct mcap_dev *mdev);
int MCapReset(struct mcap_dev *mdev);
int MCapModuleReset(struct mcap_dev *mdev);
int MCapFullReset(struct mcap_dev *mdev);
int MCapShowDevice(struct mcap_dev *mdev, int verbose);
int MCapConfigureFPGA(struct mcap_dev *mdev, char *file_path, u32 bitfile_type);
int MCapReadRegisters(struct mcap_dev *mdev, u32 *data);
int MCapAccessConfigSpace(struct mcap_dev *mdev, int argc, char **argv);
