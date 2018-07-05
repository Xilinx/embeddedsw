/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * DDR slave definition
 *
 * Note: DDR does not have a structure derived from PmSlave, currently
 * derived structure is not needed.
 *********************************************************************/

#ifndef PM_DDR_H_
#define PM_DDR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_slave.h"

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmSlave pmSlaveDdr_g;

/*********************************************************************
 * Function declarations
 ********************************************************************/
void ddr_io_prepare(void);
s32 PmDdrPowerOffSuspendResume(void);
#ifdef ENABLE_DDR_SR_WR
s32 PmDdrEnterSr(void);
s32 PmDdrExitSr(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PM_DDR_H_ */
