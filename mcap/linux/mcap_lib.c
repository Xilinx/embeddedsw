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
* @file mcap_lib.c
* MCAP Interface Library functions
*
******************************************************************************/

#include "mcap_lib.h"

/* Library Specific Definitions */
#define MCAP_VENDOR_ID	0x10EE

#define MCAP_LOOP_COUNT	1000000

#define MCAP_SYNC_DWORD	0xFFFFFFFF
#define MCAP_SYNC_BYTE0 ((MCAP_SYNC_DWORD & 0xFF000000) >> 24)
#define MCAP_SYNC_BYTE1 ((MCAP_SYNC_DWORD & 0x00FF0000) >> 16)
#define MCAP_SYNC_BYTE2 ((MCAP_SYNC_DWORD & 0x0000FF00) >> 8)
#define MCAP_SYNC_BYTE3 ((MCAP_SYNC_DWORD & 0x000000FF) >> 0)

#define MCAP_RBT_FILE	".rbt"
#define MCAP_BIT_FILE	".bit"
#define MCAP_BIN_FILE	".bin"

static char *MCapFindTypeofFile(const char *s1, const char *s2)
{
	size_t l1, l2;

	l2 = strlen(s2);
	if (!l2)
		return (char *)s1;
	l1 = strlen(s1);
	while (l1 >= l2) {
		l1--;
		if (!strncasecmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}

	return NULL;
}

static u32 MCapProcessRBT(FILE *fptr, u32 *buf)
{
	char *raw = NULL;
	int i, read;
	size_t linelen;
	u32 count = 0, len = 0, result = 0;

	while ((read = getline(&raw, &linelen, fptr)) != -1) {
		if (raw[0] != '1' && raw[1] != '0')
			continue;

		for (i = 0; i < read - 1; i++) {
			if (raw[i] == '1' || raw[i] == '0') {
				result = (result << 1) | (raw[i] - 0x30);
				count++;
				if (count == 32) {
					*buf++ = result;
					len ++;
					result = count = 0;
					break;
				}
			}
		}
	}

	return len;
}

static u32 MCapProcessBIT(FILE *fptr, u32 *buf, int sz)
{
	int err;
	u8 value, len = 0;

	/*
	 * .bit files are not guaranteed to be aligned with
	 * the bitstream sync word on a 32-bit boundary. So,
	 * we need to check every byte here.
	 */
	while ((err = fread(&value, 1, 1, fptr)) == 1) {
		len++; if (value == MCAP_SYNC_BYTE0)
		  if ((err = fread(&value, 1, 1, fptr)) == 1) {
		    len++; if (value == MCAP_SYNC_BYTE1)
		      if ((err = fread(&value, 1, 1, fptr)) == 1) {
			len++; if (value == MCAP_SYNC_BYTE2)
			  if ((err = fread(&value, 1, 1, fptr)) == 1) {
				len++; if (value == MCAP_SYNC_BYTE3)
					break;
			  }
		      }
		}
	}

	if (err != 1 && !feof(fptr)) {
		pr_err("Failed to Read BIT file\n");
		return 0;
	}

	if (err != 1 && feof(fptr)) {
		pr_err("Failed to find SYNC Word in BIT file\n");
		return 0;
	}

	*buf++ = __bswap_32(MCAP_SYNC_DWORD);

	while ((err = fread(buf, sz - len, 1, fptr)) == 1)
		;

	if (err != 1 && !feof(fptr)) {
		pr_err("Failed to Read BIT file\n");
		return 0;
	}

	return (sz - len)/4 + 1;
}

static u32 MCapProcessBIN(FILE *fptr, u32 *buf, int sz)
{
	int err;

	err = fread(buf, sz, 1, fptr);
	if (err != 1 && !feof(fptr)) {
		pr_err("Failed to Read BIN file\n");
		return 0;
	}

	return sz/4;
}

static int MCapDoBusWalk(struct mcap_dev *mdev)
{
	struct pci_cap *c;

	c = pci_find_cap(mdev->pdev, MCAP_EXT_CAP_ID, PCI_CAP_EXTENDED);

	if (!c)
		return -EMCAPBUSWALK;

	mdev->reg_base = c->addr;

	return 0;
}

static int MCapClearRequestByConfigure(struct mcap_dev *mdev, u32 *restore)
{
	u32 set;
	int loop = MCAP_LOOP_COUNT;

	set = *restore = MCapRegRead(mdev, MCAP_CONTROL);

	if (IsConfigureMCapReqSet(mdev)) {
		/* Set 'Mode' and 'In Use by PCIe' bits */
		set |= (MCAP_CTRL_MODE_MASK | MCAP_CTRL_IN_USE_MASK);
		MCapRegWrite(mdev, MCAP_CONTROL, set);

		do {
			if (!(IsConfigureMCapReqSet(mdev)))
				break;
		} while (loop--);

		if (!loop) {
			pr_err("Failed to clear MCAP Request by config bit\n");
			MCapRegWrite(mdev, MCAP_CONTROL, *restore);
			return -EMCAPREQ;
		}
	}

	pr_dbg("Request by Configure bit cleared!!\n");

	return 0;
}

static int Checkforcompletion(struct mcap_dev *mdev)
{
	unsigned long retry_count = 0;
	u32 delay;
	int sr, i;

	sr = MCapRegRead(mdev, MCAP_STATUS);
	while (!(sr & MCAP_STS_EOS_MASK)) {

		usleep(2);
		for (i=0 ; i < EMCAP_EOS_LOOP_COUNT; i++) {
			MCapRegWrite(mdev, MCAP_DATA, EMCAP_NOOP_VAL);
		}
		sr = MCapRegRead(mdev, MCAP_STATUS);
		retry_count++;
		if (retry_count > EMCAP_EOS_RETRY_COUNT) {
			pr_err("Error: The MCAP EOS bit did not assert after");
			pr_err(" programming the specified programming file\n");
			return -EMCAPREQ;
		}
	}
	return 0;
}

static int MCapWritePartialBitStream(struct mcap_dev *mdev, u32 *data,
					int len, u8 bswap)
{
	u32 set, restore;
	int err, count = 0, i;

	if (!data || !len) {
		pr_err("Invalid Arguments\n");
		return -EMCAPWRITE;
	}

	err = MCapClearRequestByConfigure(mdev, &restore);
	if (err)
		return err;

	if (IsErrSet(mdev) || IsRegReadComplete(mdev) ||
		IsFifoOverflow(mdev)) {
		pr_err("Failed to initialize configuring FPGA\n");
		MCapRegWrite(mdev, MCAP_CONTROL, restore);
		return -EMCAPWRITE;
	}

	/* Set 'Mode', 'In Use by PCIe' and 'Data Reg Protect' bits */
	set = MCapRegRead(mdev, MCAP_CONTROL);
	set |= MCAP_CTRL_MODE_MASK | MCAP_CTRL_IN_USE_MASK |
		MCAP_CTRL_DATA_REG_PROT_MASK;

	/* Clear 'Reset', 'Module Reset' and 'Register Read' bits */
	set &= ~(MCAP_CTRL_RESET_MASK | MCAP_CTRL_MOD_RESET_MASK |
		 MCAP_CTRL_REG_READ_MASK | MCAP_CTRL_DESIGN_SWITCH_MASK);

	MCapRegWrite(mdev, MCAP_CONTROL, set);

	/* Write Data */
	if (!bswap) {
		for (count = 0; count < len; count++)
			MCapRegWrite(mdev, MCAP_DATA, data[count]);
	} else {
		for (count = 0; count < len; count++)
			MCapRegWrite(mdev, MCAP_DATA, __bswap_32(data[count]));
	}

	for (i = 0 ; i < EMCAP_EOS_LOOP_COUNT; i++) {
		MCapRegWrite(mdev, MCAP_DATA, EMCAP_NOOP_VAL);
	}

	if (IsErrSet(mdev) || IsFifoOverflow(mdev)) {
		pr_err("Failed to Write Bitstream\n");
		MCapRegWrite(mdev, MCAP_CONTROL, restore);
		MCapFullReset(mdev);
		return -EMCAPWRITE;
	}

	if (!mdev->is_multiplebit) {
		pr_info("Info: A partial reconfiguration clear file (-C) was");
		pr_info(" loaded without a partial reconfiguration file (-p)");
		pr_info(" as result the MCAP Control register was not restored");
		pr_info(" to its original value\n\r");
	}

	return 0;
}

static int MCapWriteBitStream(struct mcap_dev *mdev, u32 *data,
			      int len, u8 bswap)
{
	u32 set, restore;
	int err, count = 0;

	if (!data || !len) {
		pr_err("Invalid Arguments\n");
		return -EMCAPWRITE;
	}

	err = MCapClearRequestByConfigure(mdev, &restore);
	if (err)
		return err;

	if (IsErrSet(mdev) || IsRegReadComplete(mdev) ||
		IsFifoOverflow(mdev)) {
		pr_err("Failed to initialize configuring FPGA\n");
		MCapRegWrite(mdev, MCAP_CONTROL, restore);
		return -EMCAPWRITE;
	}

	if (!mdev->is_multiplebit) {
		/* Set 'Mode', 'In Use by PCIe' and 'Data Reg Protect' bits */
		set = MCapRegRead(mdev, MCAP_CONTROL);
		set |= MCAP_CTRL_MODE_MASK | MCAP_CTRL_IN_USE_MASK |
			MCAP_CTRL_DATA_REG_PROT_MASK;

		/* Clear 'Reset', 'Module Reset' and 'Register Read' bits */
		set &= ~(MCAP_CTRL_RESET_MASK | MCAP_CTRL_MOD_RESET_MASK |
			 MCAP_CTRL_REG_READ_MASK | MCAP_CTRL_DESIGN_SWITCH_MASK);

		MCapRegWrite(mdev, MCAP_CONTROL, set);
	}

	/* Write Data */
	if (!bswap) {
		for (count = 0; count < len; count++)
			MCapRegWrite(mdev, MCAP_DATA, data[count]);
	} else {
		for (count = 0; count < len; count++)
			MCapRegWrite(mdev, MCAP_DATA, __bswap_32(data[count]));
	}

	/* Check for Completion */
	err = Checkforcompletion(mdev);
	if (err)
		return -EMCAPCFG;

	if (IsErrSet(mdev) || IsFifoOverflow(mdev)) {
		pr_err("Failed to Write Bitstream\n");
		MCapRegWrite(mdev, MCAP_CONTROL, restore);
		MCapFullReset(mdev);
		return -EMCAPWRITE;
	}

	/* Enable PCIe BAR reads/writes in the PCIe hardblock */
	restore |= MCAP_CTRL_DESIGN_SWITCH_MASK;

	MCapRegWrite(mdev, MCAP_CONTROL, restore);

	return 0;
}

void MCapLibFree(struct mcap_dev *mdev)
{
	if (mdev) {
		pci_cleanup(mdev->pacc);
		free(mdev);
	}
}

struct mcap_dev *MCapLibInit(int device_id)
{
	struct pci_dev *dev;
	struct mcap_dev *mdev;

	/* Allocate MCAP device */
	mdev = malloc(sizeof(struct mcap_dev));
	if (!mdev)
		return NULL;

	/* Get the pci_access structure */
	mdev->pacc = pci_alloc();

	mdev->is_multiplebit = 0;

	/* Initialize the PCI library */
	pci_init(mdev->pacc);

	/* Get the list of devices */
	pci_scan_bus(mdev->pacc);

	for (dev = mdev->pacc->devices; dev; dev = dev->next) {
		/* Fill in header info we need */
		pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES |
			      PCI_FILL_CLASS);

		if (dev->vendor_id == MCAP_VENDOR_ID &&
			dev->device_id == device_id) {
			pr_info("Xilinx MCAP device found\n");
			mdev->pdev = dev;
		} else {
			continue;
		}
	}

	if (!mdev->pdev) {
		pr_err("Xilinx MCAP device not found .. Exiting ...\n");
		goto free_resources;
	}

	/* Get the MCAP Register base */
	if (MCapDoBusWalk(mdev)) {
		pr_err("Unable to get the Register Base\n");
		goto free_resources;
	}

	return mdev;

free_resources:
	MCapLibFree(mdev);

	return NULL;
}

int MCapReset(struct mcap_dev *mdev)
{
	u32 set, restore;
	int err;

	err = MCapClearRequestByConfigure(mdev, &restore);
	if (err)
		return err;

	/* Set 'Mode', 'In Use by PCIe' and 'Reset' bits */
	set = MCapRegRead(mdev, MCAP_CONTROL);
	set |= MCAP_CTRL_MODE_MASK | MCAP_CTRL_IN_USE_MASK |
		MCAP_CTRL_RESET_MASK;
	MCapRegWrite(mdev, MCAP_CONTROL, set);

	if (IsErrSet(mdev) || !(IsResetSet(mdev))) {
		pr_err("Failed to Reset\n");
		MCapRegWrite(mdev, MCAP_CONTROL, restore);
		return -EMCAPRESET;
	}

	MCapRegWrite(mdev, MCAP_CONTROL, restore);
	pr_info("Reset Done!!\n");

	return 0;
}

int MCapModuleReset(struct mcap_dev *mdev)
{
	u32 set, restore;
	int err;

	err = MCapClearRequestByConfigure(mdev, &restore);
	if (err)
		return err;

	/* Set 'Mode', 'In Use by PCIe' and 'Module Reset' bits */
	set = MCapRegRead(mdev, MCAP_CONTROL);
	set |= MCAP_CTRL_MODE_MASK | MCAP_CTRL_IN_USE_MASK |
		MCAP_CTRL_MOD_RESET_MASK;
	MCapRegWrite(mdev, MCAP_CONTROL, set);

	if (IsErrSet(mdev) || !(IsModuleResetSet(mdev))) {
		pr_err("Failed to Reset Module\n");
		MCapRegWrite(mdev, MCAP_CONTROL, restore);
		return -EMCAPMODRESET;
	}

	MCapRegWrite(mdev, MCAP_CONTROL, restore);
	pr_info("Module Reset Done!!\n");

	return 0;
}

int MCapFullReset(struct mcap_dev *mdev)
{
	u32 set, restore;
	int err;

	err = MCapClearRequestByConfigure(mdev, &restore);
	if (err)
		return err;

	/* Set 'Mode', 'In Use by PCIe' and 'Module Reset' bits */
	set = MCapRegRead(mdev, MCAP_CONTROL);
	set |= MCAP_CTRL_MODE_MASK | MCAP_CTRL_IN_USE_MASK |
		MCAP_CTRL_RESET_MASK | MCAP_CTRL_MOD_RESET_MASK;
	MCapRegWrite(mdev, MCAP_CONTROL, set);

	if (IsErrSet(mdev) || !(IsModuleResetSet(mdev)) ||
		!(IsResetSet(mdev))) {
		pr_err("Failed to Full Reset\n");
		MCapRegWrite(mdev, MCAP_CONTROL, restore);
		return -EMCAPFULLRESET;
	}

	MCapRegWrite(mdev, MCAP_CONTROL, restore);
	pr_info("Full Reset Done!!\n");

	return 0;
}

static int MCapReadDataRegisters(struct mcap_dev *mdev, u32 *data)
{
	u32 set, restore, read_cnt;
	int err;

	if (!data) {
		pr_err("Invalid Arguments\n");
		return -EMCAPREAD;
	}

	err = MCapClearRequestByConfigure(mdev, &restore);
	if (err)
		return err;

	/* Set 'Mode', 'In Use by PCIe' and 'Data Reg Protect' bits */
	set = MCapRegRead(mdev, MCAP_CONTROL);
	set |= MCAP_CTRL_MODE_MASK | MCAP_CTRL_IN_USE_MASK |
		MCAP_CTRL_REG_READ_MASK;

	/* Clear 'Reset', 'Module Reset' and 'Register Read' bits */
	set &= ~(MCAP_CTRL_RESET_MASK | MCAP_CTRL_MOD_RESET_MASK);

	MCapRegWrite(mdev, MCAP_CONTROL, set);
	read_cnt = GetRegReadCount(mdev);

	if (!(read_cnt) || !(IsRegReadComplete(mdev))) {
		MCapRegWrite(mdev, MCAP_CONTROL, restore);
		return EMCAPREAD;
	}

	if (IsErrSet(mdev) || IsFifoOverflow(mdev)) {
		pr_err("Read Register Set Configuration Failed\n");
		MCapRegWrite(mdev, MCAP_CONTROL, restore);
		return -EMCAPREAD;
	}

	switch (read_cnt) {
	case 7: case 6: case 5: case 4:
		data[3] = MCapRegRead(mdev, MCAP_READ_DATA_3);
		/* Fall-through */
	case 3:
		data[2] = MCapRegRead(mdev, MCAP_READ_DATA_2);
		/* Fall-through */
	case 2:
		data[1] = MCapRegRead(mdev, MCAP_READ_DATA_1);
		/* Fall-through */
	case 1:
		data[0] = MCapRegRead(mdev, MCAP_READ_DATA_0);
		break;
	}

	MCapRegWrite(mdev, MCAP_CONTROL, restore);
	pr_dbg("Read Data Registers Complete!\n");

	return 0;
}

void MCapDumpReadRegs(struct mcap_dev *mdev)
{
	u32 data[4];
	u32 status;

	status = MCapReadDataRegisters(mdev, data);
	if (status == EMCAPREAD)
		return;

	if (status) {
		pr_err("Failed Reading Registers.. This may be");
		pr_err(" due to inappropriate FPGA configuration.");
		pr_err(" Make sure you downloaded the correct bitstream\n");
		return;
	}
	pr_info("Register Read Data 0:\t0x%08x\n", data[0]);
	pr_info("Register Read Data 1:\t0x%08x\n", data[1]);
	pr_info("Register Read Data 2:\t0x%08x\n", data[2]);
	pr_info("Register Read Data 3:\t0x%08x\n", data[3]);
}

void MCapDumpRegs(struct mcap_dev *mdev)
{
	pr_info("Extended Capability:\t0x%08x\n",
			MCapRegRead(mdev, MCAP_EXT_CAP_HEADER));
	pr_info("Vendor Specific Header:\t0x%08x\n",
			MCapRegRead(mdev, MCAP_VEND_SPEC_HEADER));
	pr_info("FPGA JTAG ID:\t\t0x%08x\n",
			MCapRegRead(mdev, MCAP_FPGA_JTAG_ID));
	pr_info("FPGA Bit-Stream Version:0x%08x\n",
			MCapRegRead(mdev, MCAP_FPGA_BIT_VERSION));
	pr_info("Status:\t\t\t0x%08x\n",
			MCapRegRead(mdev, MCAP_STATUS));
	pr_info("Control:\t\t0x%08x\n",
			MCapRegRead(mdev, MCAP_CONTROL));
	pr_info("Data:\t\t\t0x%08x\n",
			MCapRegRead(mdev, MCAP_DATA));

	MCapDumpReadRegs(mdev);
}

int MCapConfigureFPGA(struct mcap_dev *mdev, char *file_path, u32 bitfile_type)
{
	FILE *fptr;
	u32 *data;
	u32 binsz, wrdatasz;
	int err = 0;
	u8 bswap = 0;

	/* Get the size */
	fptr = fopen(file_path, "rb");
	if (fptr == NULL)
		return -EMCAPCFG;
	fseek(fptr, 0L, SEEK_END);
	binsz = ftell(fptr);
	fseek(fptr, 0L, SEEK_SET);

	/* Allocate the buffer */
	data = malloc(binsz);
	if (data == NULL)
		return -EMCAPCFG;

	/* Process files and Read the data */
	if (MCapFindTypeofFile(file_path, MCAP_RBT_FILE)) {

		/* Read the RBT file */
		wrdatasz = MCapProcessRBT(fptr, data);

	} else if (MCapFindTypeofFile(file_path, MCAP_BIT_FILE)) {

		/* Read the BIT file */
		wrdatasz = MCapProcessBIT(fptr, data, binsz);
		bswap = 1;

	} else if (MCapFindTypeofFile(file_path, MCAP_BIN_FILE)) {

		/* Read the BIN file */
		wrdatasz = MCapProcessBIN(fptr, data, binsz);
		bswap = 1;

	} else {
		pr_err("Unknown File Format\n");
		goto free_resources;
	}

	/* Program FPGA */
	if (bitfile_type == EMCAP_PARTIALCONFIG_FILE) {
		err = MCapWritePartialBitStream(mdev, data, wrdatasz, bswap);
		if (err)
			return -EMCAPCFG;
		pr_info("FPGA Partial Configuration Done!!\n");
	} else if (bitfile_type == EMCAP_CONFIG_FILE) {
		err = MCapWriteBitStream(mdev, data, wrdatasz, bswap);
		if (err)
			return -EMCAPCFG;
		pr_info("FPGA Configuration Done!!\n");
	}

free_resources:
	if (data)
		free(data);
	fclose(fptr);

	return err;
}

int MCapAccessConfigSpace(struct mcap_dev *mdev, int argc, char **argv)
{
	unsigned long wrval, rdval;
	int pos, access_type;

	pos = (int) strtol(argv[4], NULL, 16);
	access_type = tolower(argv[5][0]);

	if (argc == 6) {
		switch (access_type) {
		case 'b':
			rdval = pci_read_byte(mdev->pdev, pos);
			break;
		case 'h':
			rdval = pci_read_word(mdev->pdev, pos);
			break;
		case 'w':
			rdval = pci_read_long(mdev->pdev, pos);
			break;
		default:
			return -EMCAPCFGACC;
		}
		pr_info("Read 0x%08lx @ 0x%x\n", rdval, pos);
	}

	if (argc > 6) {
		wrval = strtoul(argv[6], 0, 0);
		switch (access_type) {
		case 'b':
			pci_write_byte(mdev->pdev, pos, wrval);
			break;
		case 'h':
			pci_write_word(mdev->pdev, pos, wrval);
			break;
		case 'w':
			pci_write_long(mdev->pdev, pos, wrval);
			break;
		default:
			return -EMCAPCFGACC;
		}
		pr_info("Written 0x%08lx @ 0x%x\n", wrval, pos);
	}

	return 0;
}

int MCapShowDevice(struct mcap_dev *mdev, int verbose)
{
	char command[80];
	u16 vendor_id, device_id;

	vendor_id = mdev->pdev->vendor_id;
	device_id = mdev->pdev->device_id;

	if (verbose == 1)
		sprintf(command, "lspci -vd %x:%x", vendor_id, device_id);
	if (verbose >= 2)
		sprintf(command, "lspci -vvd %x:%x", vendor_id, device_id);
	if (!verbose)
		sprintf(command, "lspci -d %x:%x", vendor_id, device_id);

	return system(command);
}
