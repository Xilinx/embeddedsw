
/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3cpsx_options.c
* @{
*
* Contains functions for the configuration of the XIccPs driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------
* 1.00  sd  06/10/22 First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xi3cpsx.h"
#include "xi3cpsx_pr.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/
s32 XI3cPsx_SetSClk(XI3cPsx *InstancePtr)
{
	u32 CoreRate;
	u32 CorePeriod;
	u32 SclTiming;
	UINTPTR BaseAddress;
	u8 Hcnt;
	u8 Lcnt;

	CoreRate = InstancePtr->Config.InputClockHz;
	BaseAddress = InstancePtr->Config.BaseAddress;

	CorePeriod = XI3CPSX_CEIL_DIV(1000000000, CoreRate);

	Hcnt = XI3CPSX_CEIL_DIV(I3C_BUS_THIGH_MAX_NS, CorePeriod) - 1;
	if (Hcnt < SCL_I3C_TIMING_CNT_MIN)
		Hcnt = SCL_I3C_TIMING_CNT_MIN;

	Lcnt = XI3CPSX_CEIL_DIV(CoreRate, I3C_BUS_TYP_I3C_SCL_RATE) - Hcnt;
	if (Lcnt < SCL_I3C_TIMING_CNT_MIN)
		Lcnt = SCL_I3C_TIMING_CNT_MIN;

	SclTiming = SCL_I3C_TIMING_HCNT(Hcnt) | Lcnt & XI3CPSX_SCL_I3C_PP_TIMING_I3C_PP_LCNT_MASK;
	SclTiming = 0x00050005;
	XI3cPsx_WriteReg(BaseAddress, XI3CPSX_SCL_I3C_PP_TIMING, SclTiming);
	SclTiming = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_SCL_I3C_PP_TIMING);

	Lcnt = XI3CPSX_CEIL_DIV(I3C_BUS_TLOW_OD_MIN_NS, CorePeriod);
	SclTiming = SCL_I3C_TIMING_HCNT(Hcnt) | SCL_I3C_TIMING_LCNT(Lcnt);

	/* RE-VISIT - Hard coding for the time being */
	XI3cPsx_WriteReg(BaseAddress, XI3CPSX_SCL_I3C_OD_TIMING, 0x00200020);

	return 0;
}
