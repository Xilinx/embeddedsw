/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * Implementation of notifications and event handling within
 * power management.
 *********************************************************************/

#ifndef PM_NOTIFIER_H_
#define PM_NOTIFIER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_master.h"

/*********************************************************************
 * Function declarations
 ********************************************************************/

s32 PmNotifierRegister(const PmMaster* const mst, const PmNode* const nd,
		       const u32 event, const u32 wake);

void PmNotifierUnregister(const PmMaster* const mst, const PmNode* const nd,
			  const u32 event);

void PmNotifierUnregisterAll(const PmMaster* const mst);

void PmNotifierEvent(const PmNode* const nd, const u32 event);

#ifdef __cplusplus
}
#endif

#endif /* PM_NOTIFIER_H_ */
