/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplm_plat.c
*
* This file contains the PLMI versal_net platform specific code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_pm.h"
#include "xparameters.h"
#include "xplmi_update.h"
#include "xplmi.h"
#include "xpm_psm.h"

/************************** Constant Definitions *****************************/
#define XPLMI_PSM_COUNTER_VER 		(1U)
#define XPLMI_PSM_COUNTER_LCVER		(1U)
#define XPLMI_PSM_KEEP_ALIVE_STS_VER 	(1U)
#define XPLMI_PSM_KEEP_ALIVE_STS_LCVER	(1U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief	This function enables SLVERR for PMC related modules
*
* @return	None
*
*****************************************************************************/
void XPlm_EnablePlatformSlaveErrors(void)
{
	/* TODO Add versalnet specific slave errors */
	return;
}

#ifdef XPAR_XIPIPSU_0_DEVICE_ID
/*****************************************************************************/
/**
* @brief	This function updates the keep alive status variable
*
* @param	Val to set the status as started or not started or error
*
* @return	PsmKeepAliveStatus
*
*****************************************************************************/
u32 XPlm_SetPsmAliveStsVal(u32 Val)
{
	static u32 PsmKeepAliveStatus __attribute__ ((aligned(4U)))
			= XPLM_PSM_ALIVE_NOT_STARTED;
	EXPORT_XILPSM_DS(PsmKeepAliveStatus, XPM_PSM_KEEP_ALIVE_STS_DS_ID,
		XPLMI_PSM_KEEP_ALIVE_STS_VER, XPLMI_PSM_KEEP_ALIVE_STS_LCVER,
		sizeof(PsmKeepAliveStatus), (u32)(UINTPTR)&PsmKeepAliveStatus);

	if(Val != XPLM_PSM_ALIVE_RETURN) {
		/* Update the Keep Alive Status */
		PsmKeepAliveStatus = Val;
	}

	return PsmKeepAliveStatus;
}

/*****************************************************************************/
/**
* @brief	This function updates the counter value
*
* @param	Val to Increment or Clear the CounterVal variable
*
* @return	CounterVal
*
*****************************************************************************/
u32 XPlm_UpdatePsmCounterVal(u32 Val)
{
	static u32 CounterVal __attribute__ ((aligned(4U))) = 0U;
	EXPORT_XILPSM_DS(CounterVal, XPM_PSM_COUNTER_DS_ID, XPLMI_PSM_COUNTER_VER,
		XPLMI_PSM_COUNTER_LCVER, sizeof(CounterVal), (u32)(UINTPTR)&CounterVal);

	if(Val == XPLM_PSM_COUNTER_INCREMENT) {
		/* Increment the counter value */
		CounterVal++;
	}else if(Val == XPLM_PSM_COUNTER_CLEAR){
		/* Clear the counter value */
		CounterVal = 0U;
	} else{
		/* To avoid Misra-C violation  */
	}

	return CounterVal;
}
#endif
