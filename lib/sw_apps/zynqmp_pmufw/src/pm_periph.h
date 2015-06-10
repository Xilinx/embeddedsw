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
#ifndef PM_PERIPH_H_
#define PM_PERIPH_H_

#include "pm_slave.h"

/*********************************************************************
 * Macros
 ********************************************************************/
/*
 * Standard slave states (used for generic slaves with trivial on/off)
 * These slaves have no machanisms for controlling their own state, and their
 * off state is controlled by the power parent state.
 */
#define PM_STD_SLAVE_STATE_OFF	0U
#define PM_STD_SLAVE_STATE_ON	1U

/* Always-on slaves, have only one state */
#define PM_AON_SLAVE_STATE	0U

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmSlaveTtc - Structure used for timer peripheral
 * @slv Base slave structure
 */
typedef struct PmSlaveTtc {
	PmSlave slv;
} PmSlaveTtc;

/**
 * PmSlaveSata - Structure used for Sata peripheral
 * @slv Base slave structure
 */
typedef struct PmSlaveSata {
	PmSlave slv;
} PmSlaveSata;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmSlaveTtc pmSlaveTtc0_g;
extern PmSlaveTtc pmSlaveTtc1_g;
extern PmSlaveTtc pmSlaveTtc2_g;
extern PmSlaveTtc pmSlaveTtc3_g;
extern PmSlaveSata pmSlaveSata_g;
extern PmSlave pmSlaveGpuPP0_g;
extern PmSlave pmSlaveGpuPP1_g;
extern PmSlave pmSlaveUart0_g;
extern PmSlave pmSlaveUart1_g;
extern PmSlave pmSlaveSpi0_g;
extern PmSlave pmSlaveSpi1_g;
extern PmSlave pmSlaveI2C0_g;
extern PmSlave pmSlaveI2C1_g;
extern PmSlave pmSlaveSD0_g;
extern PmSlave pmSlaveSD1_g;
extern PmSlave pmSlaveCan0_g;
extern PmSlave pmSlaveCan1_g;
extern PmSlave pmSlaveEth0_g;
extern PmSlave pmSlaveEth1_g;
extern PmSlave pmSlaveEth2_g;
extern PmSlave pmSlaveEth3_g;
extern PmSlave pmSlaveAdma_g;
extern PmSlave pmSlaveGdma_g;
extern PmSlave pmSlaveDP_g;
extern PmSlave pmSlaveNand_g;
extern PmSlave pmSlaveQSpi_g;
extern PmSlave pmSlaveGpio_g;
extern PmSlave pmSlaveAFI_g;

#endif
