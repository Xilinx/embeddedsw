/*
 * Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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

#ifndef PM_CLOCK_H_
#define PM_CLOCK_H_

#include "pm_pll.h"

typedef struct PmClockClass PmClockClass;
typedef struct PmNode PmNode;

/*********************************************************************
 * Macros
 ********************************************************************/
#define INVALID_DIV_ID(divId)	\
		((divId != PM_CLOCK_DIV0_ID) && (divId != PM_CLOCK_DIV1_ID))
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
int PmClockRequest(PmNode* const node);
int PmClockIsActive(PmNode* const node);
int PmClockMuxSetParent(PmClock* const clock, const u32 select);
int PmClockMuxGetParent(PmClock* const clock, u32 *const select);
int PmClockGateSetState(PmClock* const clock, const u8 enable);
int PmClockGateGetState(PmClock* const clock, u8* const enable);
int PmClockDividerSetVal(PmClock* const clock, const u32 divId, const u32 val);
int PmClockDividerGetVal(PmClock* const clock, const u32 divId, u32* const val);
int PmClockCheckPermission(const PmClock* const clock, const u32 ipiMask);

void PmClockInit(void);
void PmClockRelease(PmNode* const node);
void PmClockConstructList(void);
void PmClockRestore(PmNode* const node);
void PmClockSave(PmNode* const node);
PmClock* PmClockGetById(const u32 clockId);
void PmClockRestoreDdr();

#endif
