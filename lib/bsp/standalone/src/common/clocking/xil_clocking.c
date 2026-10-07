/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_clocking.c
*
* The xil_clocking.c file contains clocking related functions and macros.
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 7.2 sd  02/06/20 First release of clocking
* 7.2 sd  03/20/20 Added checking for isolation case
* 9.0 ml  03/03/23 Add description to fix doxygen warnings.
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
******************************************************************************/

#include "xil_clocking.h"
/************************** Variable Definitions *****************************/

#if (defined  (XPAR_XCRPSU_0_DEVICE_ID) || defined(XPAR_PSU_CRL_APB_BASEADDR)) && defined (XCLOCKING)
XClock ClockInstance;		/* Instance of clock Controller */
XClockPs_Config *ConfigPtr;

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* This API initialize the Clock controller driver.
*
* @return	Status to indicate success/failure.
*

******************************************************************************/
XStatus Xil_ClockInit(void)
{
	XStatus Status = XST_FAILURE;

	/* Lookup clock configurations */
#ifndef SDT
	ConfigPtr = XClock_LookupConfig(XPAR_XCLOCKPS_DEVICE_ID);
#else
	ConfigPtr = XClock_LookupConfig(XPAR_PSU_CRL_APB_BASEADDR);
#endif

	/* Initialize the Clock controller driver */
	Status = XClock_CfgInitialize(&ClockInstance, ConfigPtr);
	return Status;
}

/*****************************************************************************/
/**
*
* This function enables output clock based on clock ID.
*
* @param[in]	ClockId is the identifier for output clock.
*
* @return	Status to indicate success/failure.
*

******************************************************************************/
XStatus Xil_ClockEnable(XClock_OutputClks ClockId)
{

	XStatus Status = XST_FAILURE;

	Status = XClock_EnableClock(ClockId);
	return Status;
}

/*****************************************************************************/
/**
*
* This function disables output clock based on clock ID.
*
* @param[in] 	ClockId is the identifier for output clock.
*
* @return	Status to indicate success/failure.
*

******************************************************************************/
XStatus Xil_ClockDisable(XClock_OutputClks ClockId)
{
	XStatus Status = XST_FAILURE;

	Status = XClock_DisableClock(ClockId);
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to fetch rate of output clock.
*
* @param[in] 	ClockId is the identifier for output clock.
* @param[in] 	Rate is a pointer to variable storing rate.
*
* @return	Status to indicate success/failure.
*

******************************************************************************/
XStatus Xil_ClockGetRate(XClock_OutputClks ClockId, XClockRate *Rate)
{
	XStatus Status = XST_FAILURE;

	Xil_AssertNonvoid(Rate != NULL);

	Status = XClock_GetRate(ClockId, Rate);
	if (XST_SUCCESS == Status) {
		xdbg_printf(XDBG_DEBUG_GENERAL, "Operating rate =  %lx\n",*Rate);
	} else {
		xdbg_printf(XDBG_DEBUG_ERROR, "Failed: Fetching rate\r\n");
	}
	return Status;
}
/*****************************************************************************/
/**
*
* This function is used to set rate for output clock.
*
* @param[in] 	ClockId is the identifier for output clock.
* @param[in] 	Rate is the clock rate to set.
* @param[in] 	SetRate is a pointer to variable holding rate that is set.
*
* @return	Status to indicate success/failure.
*

******************************************************************************/
XStatus Xil_ClockSetRate(XClock_OutputClks ClockId, XClockRate Rate,
							XClockRate *SetRate)
{
	XStatus Status = XST_FAILURE;

	Xil_AssertNonvoid(SetRate != NULL);

	if (Rate == 0) {
		return XST_FAILURE;
	}
	Status = XClock_SetRate(ClockId, Rate, SetRate);
	if (XST_SUCCESS != Status) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Failed Setting rate\n");
	}
	return Status;
}

#else

/*****************************************************************************/
/**
*
* This function is used to fetch rate of output clock.
*
* @param[in] 	ClockId is the identifier for output clock.
* @param[in] 	Rate is pointer to variable storing rate.
*
* @return	XST_FAILURE to indicate failure.
*

******************************************************************************/
XStatus Xil_ClockGetRate(XClock_OutputClks ClockId, XClockRate *Rate)
{
	(void) ClockId;
	(void) Rate;
	return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function is used to set rate for output clock.
*
* @param[in] 	ClockId is the identifier for output clock.
* @param[in] 	Rate is the clock rate to set.
* @param[in] 	SetRate is a pointer to variable holding rate that is set.
*
* @return	XST_FAILURE to indicate failure.
*

******************************************************************************/
XStatus Xil_ClockSetRate(XClock_OutputClks ClockId, XClockRate Rate,
							XClockRate *SetRate) {
	(void) ClockId;
	(void) Rate;
	(void) SetRate;
	return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This API initialize the Clock controller driver.
*
* @return	XST_SUCCESS to indicate success.
*

******************************************************************************/
XStatus Xil_ClockInit(void)
{
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function enables output clock based on clock ID.
*
* @param[in] 	ClockId is the identifier for output clock.
*
* @return	XST_SUCCESS to indicate success.
*

******************************************************************************/
XStatus Xil_ClockEnable(XClock_OutputClks ClockId)
{
	(void) ClockId;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function disables output clock based on clock ID.
*
* @param[in] 	ClockId is the identifier for output clock.
*
* @return	XST_SUCCESS to indicate success.
*

******************************************************************************/
XStatus Xil_ClockDisable(XClock_OutputClks ClockId)
{
	(void) ClockId;
	return XST_SUCCESS;
}

/** @} */
#endif /* XCLOCKING */
