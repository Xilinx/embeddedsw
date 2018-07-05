/*
 * Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
 */

/*********************************************************************
 * SRAM memories slaves definitions and data structures
 *********************************************************************/

#ifndef PM_SRAM_H_
#define PM_SRAM_H_

#include "pm_slave.h"

typedef struct PmSlaveTcm PmSlaveTcm;

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmSlaveSram - Structure of a sram object, derived from slave
 * @slv         Base slave structure
 * @PwrDn   Pointer to a power down pmu-rom handler
 * @PwrUp   Pointer to a power up pmu-rom handler
 * @retCtrlAddr Address of the retention control register
 * @retCtrlMask Mask of the retention bits in control register
 */
typedef struct PmSlaveSram {
	PmSlave slv;
	PmTranHandler PwrDn;
	PmTranHandler PwrUp;
	const u32 retCtrlAddr;
	const u32 retCtrlMask;
} PmSlaveSram;

/**
 * PmSlaveTcm - TCM structure derived from SRAM slave
 * @sram	Base SRAM slave structure
 * @eccInit	ECC initialization handler (to call on OFF->ON transition)
 * @base	Base address of the memory bank
 * @size	Size of the memory bank
 * @id		ID of the TCM bank
 */
typedef struct PmSlaveTcm {
	PmSlaveSram sram;
	void (*const eccInit)(const PmSlaveTcm* const tcm);
	u32 base;
	u32 size;
	u8 id;
} PmSlaveTcm;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmSlaveSram pmSlaveOcm0_g;
extern PmSlaveSram pmSlaveOcm1_g;
extern PmSlaveSram pmSlaveOcm2_g;
extern PmSlaveSram pmSlaveOcm3_g;
extern PmSlaveSram pmSlaveL2_g;
extern PmSlaveTcm pmSlaveTcm0A_g;
extern PmSlaveTcm pmSlaveTcm0B_g;
extern PmSlaveTcm pmSlaveTcm1A_g;
extern PmSlaveTcm pmSlaveTcm1B_g;

#endif
