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

/**
 * PmSlaveGpp - Structure used for GPP
 * @slv         Base slave structure
 * @PwrDn   Pointer to a power down pmu-rom handler
 * @PwrUp   Pointer to a power up pmu-rom handler
 */
typedef struct PmSlaveGpp {
	PmSlave slv;
	PmTranHandler PwrDn;
	PmTranHandler PwrUp;
} PmSlaveGpp;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmSlaveTtc pmSlaveTtc0_g;
extern PmSlaveTtc pmSlaveTtc1_g;
extern PmSlaveTtc pmSlaveTtc2_g;
extern PmSlaveTtc pmSlaveTtc3_g;
extern PmSlaveSata pmSlaveSata_g;
extern PmSlaveGpp pmSlaveGpuPP0_g;
extern PmSlaveGpp pmSlaveGpuPP1_g;
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
extern PmSlave pmSlaveIpiApu_g;
extern PmSlave pmSlaveIpiRpu0_g;
extern PmSlave pmSlaveGpu_g;
extern PmSlave pmSlavePcie_g;
extern PmSlave pmSlavePcap_g;
extern PmSlave pmSlaveRtc_g;
extern PmSlaveUsb pmSlaveUsb0_g;
extern PmSlaveUsb pmSlaveUsb1_g;

#endif
