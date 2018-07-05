/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


#ifndef PM_CLOCK_H_
#define PM_CLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_pll.h"

/*********************************************************************
 * Macros
 ********************************************************************/
#define INVALID_DIV_ID(divId)	\
		(((divId) != PM_CLOCK_DIV0_ID) && ((divId) != PM_CLOCK_DIV1_ID))
/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmClock - Basic clock structure
 * @derived	Pointer to the derived clock structure
 * @class	Pointer to the clock class
 * @id		Clock identifier
 */
typedef struct PmClock {
	void* const derived;
	const PmClockClass* const class;
	const u8 id;
} PmClock;

/*********************************************************************
 * Function declarations
 ********************************************************************/
s32 PmClockRequest(PmNode* const node);
s32 PmClockIsActive(PmNode* const node);
s32 PmClockMuxSetParent(PmClock* const clockPtr, const u32 select);
s32 PmClockMuxGetParent(PmClock* const clockPtr, u32 *const select);
s32 PmClockGateSetState(PmClock* const clockPtr, const u8 enable);
s32 PmClockGateGetState(PmClock* const clockPtr, u8* const enable);
s32 PmClockDividerSetVal(PmClock* const clockPtr, const u32 divId, const u32 val);
s32 PmClockDividerGetVal(PmClock* const clockPtr, const u32 divId, u32* const val);
s32 PmClockCheckPermission(const PmClock* const clockPtr, const u32 ipiMask);

void PmClockInit(void);
void PmClockRelease(PmNode* const node);
void PmClockConstructList(void);
void PmClockRestore(PmNode* const node);
void PmClockSave(PmNode* const node);
PmClock* PmClockGetById(const u32 clockId);
void PmClockRestoreDdr(void);


#ifdef __cplusplus
}
#endif

#endif /* PM_CLOCK_H_ */
