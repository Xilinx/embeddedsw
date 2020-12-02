/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


#ifndef PM_EXTERN_H_
#define PM_EXTERN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "pm_common.h"

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmSlave pmSlaveExternDevice_g;

/*********************************************************************
 * Function declarations
 ********************************************************************/
s32 PmExternWakeMasters(void);

#ifdef __cplusplus
}
#endif

#endif /* PM_EXTERN_H_ */
