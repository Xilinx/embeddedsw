/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * This file contains interface of the PM configuration object parser.
 *********************************************************************/

#ifndef PM_CONFIG_H_
#define PM_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "pm_common.h"

/*********************************************************************
 * Function declarations
 ********************************************************************/

s32 PmConfigLoadObject(const u32 address, const u32 callerIpi);

bool PmConfigObjectIsLoaded(void);

#ifdef __cplusplus
}
#endif

#endif /* PM_CONFIG_H_ */
