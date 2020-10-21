/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * GPU Pixel Processors slaves data structures
 *********************************************************************/

#ifndef PM_GPP_H_
#define PM_GPP_H_

#ifdef __cplusplus
extern "C" {
#endif

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
extern PmSlave pmSlaveGpu_g;

#ifdef __cplusplus
}
#endif

#endif /* PM_GPP_H_ */
