/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xavbuf_clk.h
 *
 * This header file contains the identifiers and low-level driver functions (or
 * macros) that can be used to configure PLL to generate required frequency.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   mh  06/24/17 Initial release.
 * 2.1   tu  12/29/17 LPD and FPD offsets adjusted
 * </pre>
 *
*******************************************************************************/

#ifndef XAVBUF_CLK_H_
#define XAVBUF_CLK_H_

/******************************* Include Files ********************************/
#include "xavbuf_hw.h"
#include "xstatus.h"
#include "sleep.h"

/****************************** Type Definitions ******************************/
/**
 * This enum enumerates various PLL
 */
enum PLL{
	APLL  = 0,
	DPLL  = 1,
	VPLL  = 2,
	IOPLL = 3,
	RPLL  = 4
};

/**
 * This typedef enumerates various variables used to configure Pll
 */
typedef struct {
	u64 BaseAddress;
	u64 Fractional;
	u64 RefClkFreqhz;
	u32 Divider;
	u8 Offset;
	u8 ClkDividBy2;
	u8 ExtDivider0;
	u8 ExtDivider1;
	u8 ExtDividerCnt;
	u8 DomainSwitchDiv;
	u8 FracIntegerFBDIV;
	u8 IntegerFBDIV;
	u8 InputRefClk;
	u8 Fpd;
	u8 Pll;
}XAVBuf_Pll;

/**************************** Function Prototypes *****************************/
int XAVBuf_SetPixelClock(u64 FreqHz);
int XAVBuf_SetAudioClock(u64 FreqHz);
#endif /* XAVBUF_CLK_H_ */
