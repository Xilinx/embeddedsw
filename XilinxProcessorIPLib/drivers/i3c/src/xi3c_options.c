/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3c.c
* @addtogroup Overview
* @{
*
* Handles init functions.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---  -------- ---------------------------------------------
* 1.00  gm   03/18/24 Add Scl clock configuration support
* 1.1   gm   10/07/24 Set Data Hold Time based on IP revision value.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xi3c.h"
#include "xi3c_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/***************************************************************************/
/**
* @brief
* Sets I3C Scl clock frequency.
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	SclkHz is Scl clock to be configured in Hz.
* @param        Mode is the mode of operation I2C/I3C.
*
* @return       The return value is XST_SUCCESS if successful.
*
* @note         None.
*
******************************************************************************/
s32 XI3c_SetSClk(XI3c *InstancePtr, u32 SclkHz, u8 Mode)
{
	u32 THigh;
	u32 TLow;
	u32 THold;
	u32 OdTHigh;
	u32 OdTLow;
	u32 CorePeriodNs;
	u32 TcasMin;
	u32 TsuStart;
	u32 TsuStop;
	u32 ThdStart;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(SclkHz > 0U);

	THigh = XI3C_CEIL_DIV(InstancePtr->Config.InputClockHz, SclkHz) >> 1;
	TLow = THigh;

	/* Hold time : 40% of TLow time */
	THold = (TLow * 4)/10;
	CorePeriodNs = XI3C_CEIL_DIV(1000000000, InstancePtr->Config.InputClockHz);

	/*
	 * Data Hold Time.
	 * For initial IP (revision number = 0), minimum Data Hold Time is 5.
	 * For updated IP (revision number > 0), minimum Data Hold Time is 6.
	 * Updated IP now supports achieving high data rate with low reference
	 * frequency.
	 */
	if (XI3c_GetRevisionNumber(InstancePtr) == 0)
		THold = (THold < 5) ? 5 : THold;
	else
		THold = (THold < 6) ? 6 : THold;

	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_SCL_HIGH_TIME_OFFSET,
		      ((THigh-2) & XI3C_18BITS_MASK));
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_SCL_LOW_TIME_OFFSET,
		      ((TLow-2)  & XI3C_18BITS_MASK));
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_SDA_HOLD_TIME_OFFSET,
		      ((THold-2) & XI3C_18BITS_MASK));

	if(!Mode) {
		/*
		 * I2C
		 */
		XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_OD_SCL_HIGH_TIME_OFFSET,
			      ((THigh-2) & XI3C_18BITS_MASK));
		XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_OD_SCL_LOW_TIME_OFFSET,
			      ((TLow-2)  & XI3C_18BITS_MASK));
		/*
		 * Minimum clock period after start 600ns
		 */
		TcasMin = XI3C_CEIL_DIV(600000, CorePeriodNs);
	} else {
		/*
		 * I3C OD
		 */

		/*
		 * Minimum OdTLow period 500ns
		 */
		OdTLow = XI3C_CEIL_DIV(500000, CorePeriodNs);

		/*
		 * Minimum OdTHigh period 41ns
		 */
		OdTHigh = XI3C_CEIL_DIV(41000, CorePeriodNs);

		OdTLow = (TLow < OdTLow) ?OdTLow : TLow;
		OdTHigh = (THigh > OdTHigh) ?OdTHigh : THigh;

		XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_OD_SCL_HIGH_TIME_OFFSET,
			      ((OdTHigh-2) & XI3C_18BITS_MASK));
		XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_OD_SCL_LOW_TIME_OFFSET,
			      ((OdTLow-2)  & XI3C_18BITS_MASK));
		/*
		 * Minimum clock period after start 260ns
		 */
		TcasMin = XI3C_CEIL_DIV(260000, CorePeriodNs);
	}

	ThdStart = (THigh > TcasMin) ? THigh: TcasMin;
	TsuStart = (TLow > TcasMin) ? TLow: TcasMin;
	TsuStop = (TLow > TcasMin) ? TLow: TcasMin;

	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_TSU_START_OFFSET,
		      ((TsuStart-2) & XI3C_18BITS_MASK));
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_THD_START_OFFSET,
		      ((ThdStart-2)  & XI3C_18BITS_MASK));
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_TSU_STOP_OFFSET,
		      ((TsuStop-2) & XI3C_18BITS_MASK));

	return XST_SUCCESS;
}
