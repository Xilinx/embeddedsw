/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/

/*********************************************************************
 * SRAM memories slaves definitions and data structures
 *********************************************************************/

#ifndef PM_SRAM_H_
#define PM_SRAM_H_

#include "pm_slave.h"

/*********************************************************************
 * Macros
 ********************************************************************/
/* Instances of SRAM */
#define PM_SRAM_OCM0        0U
#define PM_SRAM_OCM1        1U
#define PM_SRAM_OCM2        2U
#define PM_SRAM_OCM3        3U
#define PM_SRAM_TCM0A       4U
#define PM_SRAM_TCM0B       5U
#define PM_SRAM_TCM1A       6U
#define PM_SRAM_TCM1B       7U
#define PM_SRAM_L2          8U
#define PM_SRAM_INST_MAX    9U

/* Power states of SRAM */
#define PM_SRAM_STATE_OFF   0U
#define PM_SRAM_STATE_RET   1U
#define PM_SRAM_STATE_ON    2U
#define PM_SRAM_STATE_MAX   3U

/* Transitions of sram */
#define PM_SRAM_TR_ON_TO_RET    0U
#define PM_SRAM_TR_RET_TO_ON    1U
#define PM_SRAM_TR_ON_TO_OFF    2U
#define PM_SRAM_TR_OFF_TO_ON    3U
#define PM_SRAM_TR_MAX          4U

/*********************************************************************
 * Structure definitions
 ********************************************************************/
typedef struct PmSlaveSram {
	PmSlave slv;
} PmSlaveSram;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmSlaveSram pmSlaveOcm0_g;
extern PmSlaveSram pmSlaveOcm1_g;
extern PmSlaveSram pmSlaveOcm2_g;
extern PmSlaveSram pmSlaveOcm3_g;
extern PmSlaveSram pmSlaveL2_g;
extern PmSlaveSram pmSlaveTcm0A_g;
extern PmSlaveSram pmSlaveTcm0B_g;
extern PmSlaveSram pmSlaveTcm1A_g;
extern PmSlaveSram pmSlaveTcm1B_g;

#endif
