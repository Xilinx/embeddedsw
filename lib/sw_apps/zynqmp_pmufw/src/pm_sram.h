/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * SRAM memories slaves definitions and data structures
 *********************************************************************/

#ifndef PM_SRAM_H_
#define PM_SRAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_slave.h"
#include "pm_common.h"

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
struct PmSlaveTcm {
	PmSlaveSram sram;
	void (*const eccInit)(const PmSlaveTcm* const tcm);
	u32 base;
	u32 size;
	u8 id;
};

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

#ifdef __cplusplus
}
#endif

#endif /* PM_SRAM_H_ */
