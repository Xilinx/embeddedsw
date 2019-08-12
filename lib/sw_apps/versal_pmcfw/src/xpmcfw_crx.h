/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* IMDDRIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF DDRRCHANTABILITY,
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEDDRNT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_crx.h
*
* This is the file which contains code for clock and reset functionality
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XCRX_H
#define XCRX_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xil_io.h"

#include "xpmcfw_default.h"

/************************** Constant Definitions *****************************/

#define XCRX_APU_CPU_0		(1U)
#define XCRX_APU_CPU_1		(2U)
#define XCRX_APU_CPU_DUAL	(3U)

#define XCRX_CPU_HALT		(1U)
#define XCRX_CPU_NO_HALT	(2U)

#define XCRX_RPU_CPU_0		(0x10U)
#define XCRX_RPU_CPU_1		(0x20U)
#define XCRX_RPU_CPU_L		(0x30U)

#define XCRX_APU_AA64		(0U)
#define XCRX_APU_AA32		(1U)

#define XCRX_CPU_VINITHI_HIVEC	(0U)
#define XCRX_CPU_VINITHI_LOVEC	(1U)

#define XCRX_RPU_1_BASE_OFFSET	(0x100U)

/**
 * Aarch32 or Aarch64 CPU definitions
 */
#define FPD_APU_CONFIG_0_AA64N32_MASK_CPU0 (0x1U)
#define FPD_APU_CONFIG_0_AA64N32_MASK_CPU1 (0x2U)

#define FPD_APU_CONFIG_0_VINITHI_MASK_CPU0  (u32)(0x100U)
#define FPD_APU_CONFIG_0_VINITHI_MASK_CPU1  (u32)(0x200U)


/**************************** Type Definitions *******************************/
typedef struct {
	u64 RvbarAddr;  /*< Reset Vector Address */
	u32 CpuNo;	/*< CPU No to indicate APU/RPU core number */
	u32 VInitHi;	/*< Vector Init High mapping HiVec/LoVec */
	u32 AA64nAA32;	/*< A72 AA64 or AA32 state */
	u32 Halt;	/*< Half configuration of CPU */
} XCrx;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
XStatus XCrx_Init();
void XCrx_RelApu0(XCrx *InstancePtr);
void XCrx_RelApu1(XCrx *InstancePtr);
void XCrx_RelRpu(XCrx *InstancePtr);
void XCrx_WakeUpPsm();
void XCrx_RelPsm();

#ifdef __cplusplus
}
#endif

#endif  /* XCRX_H */
