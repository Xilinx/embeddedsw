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

/*********************************************************************
 * All functions, data and definitions needed for
 * managing PM slaves' states.
 *********************************************************************/

#ifndef PM_SLAVE_H_
#define PM_SLAVE_H_

#include "pm_defs.h"
#include "pm_common.h"
#include "pm_node.h"

/* Forward declarations */
typedef struct PmMaster PmMaster;
typedef struct PmRequirement PmRequirement;
typedef struct PmSlave PmSlave;

typedef int (*const PmSlaveFsmHandler)(PmSlave* const slave,
					   const PmStateId nextState);

/*********************************************************************
 * Macros
 ********************************************************************/
/* FPD GIC Proxy group base addresses */
#define FPD_GICP_GROUP0_BASE_ADDR   0xFF418000U
#define FPD_GICP_GROUP1_BASE_ADDR   0xFF418014U
#define FPD_GICP_GROUP2_BASE_ADDR   0xFF418028U
#define FPD_GICP_GROUP3_BASE_ADDR   0xFF41803CU
#define FPD_GICP_GROUP4_BASE_ADDR   0xFF418050U

/* FPD GIC Proxy register offsets */
#define FPD_GICP_STATUS_OFFSET      0x0U
#define FPD_GICP_MASK_OFFSET        0x4U
#define FPD_GICP_IRQ_ENABLE_OFFSET  0x8U
#define FPD_GICP_IRQ_DISABLE_OFFSET 0xCU

/* FPD GIC Proxy group indentifiers */
#define FPD_GICP_GROUP0     0U
#define FPD_GICP_GROUP1     1U
#define FPD_GICP_GROUP2     2U
#define FPD_GICP_GROUP3     3U
#define FPD_GICP_GROUP4     4U
#define FPD_GICP_GROUP_MAX  5U

#define FPD_GICP_ALL_IRQ_MASKED_IN_GROUP   0xFFFFFFFFU

/* FPD GIC Proxy pmu irq masks */
#define FPD_GICP_PMU_IRQ_GROUP0 0x1U
#define FPD_GICP_PMU_IRQ_GROUP1 0x2U
#define FPD_GICP_PMU_IRQ_GROUP2 0x4U
#define FPD_GICP_PMU_IRQ_GROUP3 0x8U
#define FPD_GICP_PMU_IRQ_GROUP4 0x10U

/* FPD GIC Proxy irq masks */

/* GIC Proxy group 0 */
#define FPD_GICP_CAN1_WAKE_IRQ_MASK	(1 << 24)
#define FPD_GICP_CAN0_WAKE_IRQ_MASK	(1 << 23)
#define FPD_GICP_UART1_WAKE_IRQ_MASK	(1 << 22)
#define FPD_GICP_UART0_WAKE_IRQ_MASK	(1 << 21)
#define FPD_GICP_SPI1_WAKE_IRQ_MASK	(1 << 20)
#define FPD_GICP_SPI0_WAKE_IRQ_MASK	(1 << 19)
#define FPD_GICP_I2C1_WAKE_IRQ_MASK	(1 << 18)
#define FPD_GICP_I2C0_WAKE_IRQ_MASK	(1 << 17)
#define FPD_GICP_GPIO_WAKE_IRQ_MASK	(1 << 16)
#define FPD_GICP_SPI_WAKE_IRQ_MASK	(1 << 15)
#define FPD_GICP_NAND_WAKE_IRQ_MASK	(1 << 14)

/* GIC Proxy group 1 */
#define FPD_GICP_ETH3_WAKE_IRQ_MASK	(1 << 31)
#define FPD_GICP_ETH2_WAKE_IRQ_MASK	(1 << 29)
#define FPD_GICP_ETH1_WAKE_IRQ_MASK	(1 << 27)
#define FPD_GICP_ETH0_WAKE_IRQ_MASK	(1 << 25)
#define FPD_GICP_SD1_WAKE_IRQ_MASK	(1 << 19)
#define FPD_GICP_SD0_WAKE_IRQ_MASK	(1 << 18)
#define FPD_GICP_TTC3_WAKE_IRQ_MASK	(1 << 13)
#define FPD_GICP_TTC2_WAKE_IRQ_MASK	(1 << 10)
#define FPD_GICP_TTC1_WAKE_IRQ_MASK	(1 << 7)
#define FPD_GICP_TTC0_WAKE_IRQ_MASK	(1 << 4)
#define FPD_GICP_IPI_APU_WAKE_IRQ_MASK	(1 << 3)

/* GIC Proxy group 2 */
#define FPD_GICP_USB1_WAKE_IRQ_MASK (1 << 12)
#define FPD_GICP_USB0_WAKE_IRQ_MASK (1 << 11)

/* GIC Proxy group 4 */
#define FPD_GICP_SATA_WAKE_IRQ_MASK (1 << 5)

/* Mask definitions for slave's flags */
#define PM_SLAVE_FLAG_IS_SHAREABLE	0x1U

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmStateTran - Transition for a state in finite state machine
 * @latency     Transition latency in microseconds
 * @fromState   From which state the transition is taken
 * @toState     To which state the transition is taken
 */
typedef struct {
	const u32 latency;
	PmStateId fromState;
	PmStateId toState;
} PmStateTran;

/**
 * PmSlaveFsm - Finite state machine data for slaves
 * @state       Pointer to states array. Index in array is a state id, elements
 *              of array are power values in that state. For power island values
 *              are 0 and 1, for power domains values are in mV
 * @enterState  Pointer to a function that executes FSM actions to enter a state
 * @trans       Pointer to array of transitions of the FSM
 * @transCnt    Number of elements in transition array
 * @statesCnt   Number of states in state array
 */
typedef struct {
	const u32* const states;
	PmSlaveFsmHandler enterState;
	const PmStateTran* const trans;
	const u8 statesCnt;
	const u8 transCnt;
} PmSlaveFsm;

/**
 * PmGicProxyProperties - Information about FPD GIC Proxy groups
 * @baseAddr    Base address of the group
 * @pmuIrqBit   Bit (mask) of the interrupt which the group generates to the PMU
 */
typedef struct {
	const u32 baseAddr;
	const u32 pmuIrqBit;
} PmGicProxyProperties;

/**
 * PmWakeProperties - Wake-up event properties
 * @proxyIrqMask    As most of the interrupt routes go through FPD GIC Proxy,
 *                  this is the interrupt mask in GIC Proxy registers.
 * @proxyGroup      Group in FPD GIC Proxy
 */
typedef struct {
	const u32 proxyIrqMask;
	PmGicProxyProperties *proxyGroup;
} PmWakeProperties;

/**
 * PmSlave - Slave structure used for managing slave's states
 * @node        Pointer to the node structure of this slave
 * @reqs        Pointer to array of master requirements related to this slave
 * @wake        Wake event this slave can generate
 * @slvFsm      Slave finite state machine
 * @reqsCnt     Size of masterReq array
 * @flags       Slave's flags (bit 0: whether the slave is shareable (1) or
 *              exclusive (0) resource)
 */
typedef struct PmSlave {
	PmNode node;
	PmRequirement* const* reqs;
	const PmWakeProperties* wake;
	const PmSlaveFsm* slvFsm;
	u8 reqsCnt;
	u8 flags;
} PmSlave;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmGicProxyProperties gicProxyGroups_g[FPD_GICP_GROUP_MAX];

/*********************************************************************
 * Function declarations
 ********************************************************************/
int PmUpdateSlave(PmSlave* const slave);
int PmCheckCapabilities(PmSlave* const slave, const u32 capabilities);

bool PmSlaveRequiresPower(const PmSlave* const slave);

int PmSlaveVerifyRequest(const PmSlave* const slave);
int PmSlaveProcessWake(const u32 wakeMask);
void PmSlaveWakeEnable(PmSlave* const slave);
void PmSlaveWakeDisable(PmSlave* const slave);

u32 PmGetLatencyFromState(const PmSlave* const slave, const PmStateId state);
u32 PmSlaveGetUsersMask(const PmSlave* const slave);

u32 PmSlaveGetUsageStatus(const u32 slavenode, const PmMaster *const master);
u32 PmSlaveGetRequirements(const u32 slavenode, const PmMaster *const master);

/**
 * PmSlaveWakeClear() - Clear GIC Proxy wake interrupt of the slave
 * @slave       Slave whose interrupt should be cleared
 */
static inline void PmSlaveWakeClear(const PmSlave* const slave)
{
	/* Clear GIC Proxy IRQ by writing slave's mask to status register */
	XPfw_Write32(slave->wake->proxyGroup->baseAddr +
		     FPD_GICP_STATUS_OFFSET, slave->wake->proxyIrqMask);
}

#endif
