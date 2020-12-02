/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

#ifndef PM_PERIPH_H_
#define PM_PERIPH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_slave.h"
#include "pm_gpp.h"

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmSlave pmSlaveTtc0_g;
extern PmSlave pmSlaveTtc1_g;
extern PmSlave pmSlaveTtc2_g;
extern PmSlave pmSlaveTtc3_g;
extern PmSlave pmSlaveSata_g;
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
extern PmSlave pmSlaveIpiApu_g;
extern PmSlave pmSlaveIpiRpu0_g;
extern PmSlave pmSlaveIpiRpu1_g;
extern PmSlave pmSlaveIpiPl0_g;
extern PmSlave pmSlaveIpiPl1_g;
extern PmSlave pmSlaveIpiPl2_g;
extern PmSlave pmSlaveIpiPl3_g;
extern PmSlave pmSlavePcie_g;
extern PmSlave pmSlavePcap_g;
extern PmSlave pmSlaveRtc_g;
extern PmSlave pmSlavePl_g;
extern PmSlave pmSlaveFpdWdt_g;

/**
 * PmWakeEventEth - Ethernet wake event, derived from PmWakeEvent
 * @wake		Basic PmWakeEvent structure
 * @baseAddr		Base Address of Ethernet
 * @receiveQptr		Receive queue pointer of ethernet
 * @receiveQ1ptr	Receive queue-1 pointer of ethernet
 * @wakeEnabled		Flag to check whether ethernet wakeup is enabled or not
 * @subClass		Pointer to the class specific to the derived structure
 * @subWake		Pointer to wake structure of derived class
 */
typedef struct PmWakeEventEth {
	PmWakeEvent wake;
	const u32 baseAddr;
	u32 receiveQptr;
	u32 receiveQ1ptr;
	u32 receiveHighptr;
	bool wakeEnabled;
	PmWakeEventClass* const subClass;
	PmWakeEvent* const subWake;
} PmWakeEventEth;

#ifdef __cplusplus
}
#endif

#endif /* PM_PERIPH_H_ */
