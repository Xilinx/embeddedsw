/*
 * Copyright (C) 2014 - 2019 Xilinx, Inc.  All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
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
void PmDdrExitSr(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PM_DDR_H_ */
