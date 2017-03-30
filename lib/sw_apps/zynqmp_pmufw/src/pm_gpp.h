/*
 * Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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
 */

/*********************************************************************
 * GPU Pixel Processors slaves data structures
 *********************************************************************/

#ifndef PM_GPP_H_
#define PM_GPP_H_

#include "pm_slave.h"

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmSlaveGpp - Slave wrapper structure used for GPP
 * @slv		Base slave structure
 * @PwrDn	Pointer to power down PMU-ROM handler
 * @PwrUp	Pointer to power up PMU-ROM handler
 */
typedef struct PmSlaveGpp {
	PmSlave slv;
	PmTranHandler PwrDn;
	PmTranHandler PwrUp;
	u32 (*const reset)(void);
} PmSlaveGpp;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmSlaveGpp pmSlaveGpuPP0_g;
extern PmSlaveGpp pmSlaveGpuPP1_g;
extern PmSlaveGpp pmSlaveVcu_g;

#endif
