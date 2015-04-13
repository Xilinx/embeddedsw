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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */
#ifndef PM_PERIPH_H_
#define PM_PERIPH_H_

#include "pm_slave.h"

/*********************************************************************
 * Macros
 ********************************************************************/
/*
 * Standard slave states (used for generic slaves with trivial on/off)
 * These slaves have no machanisms for controlling their own state, and their
 * off state is controlled by the power parent state.
 */
#define PM_STD_SLAVE_STATE_OFF  0U
#define PM_STD_SLAVE_STATE_ON   1U

/* Always-on slaves, have only one state */
#define PM_AON_SLAVE_STATE      0U

/*********************************************************************
 * Structure definitions
 ********************************************************************/
typedef struct PmSlaveTtc {
	PmSlave slv;
} PmSlaveTtc;

typedef struct PmSlaveSata {
	PmSlave slv;
} PmSlaveSata;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmSlaveTtc pmSlaveTtc0_g;
extern PmSlaveSata pmSlaveSata_g;

#endif
