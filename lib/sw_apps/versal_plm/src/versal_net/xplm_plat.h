/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal_net/xplm_plat.h
*
* This file contains versal_net specific declarations PLM module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       bm   07/13/2022 Added compatibility check for In-Place PLM Update
*       ma   07/29/2022 Replaced XPAR_XIPIPSU_0_DEVICE_ID macro with
*                       XPLMI_IPI_DEVICE_ID
* 1.01  ng   11/11/2022 Fixed doxygen file name error
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLM_PLAT_H
#define XPLM_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* @brief	This function configures the NPLL equal to slave SLR ROM NPLL
*		frequency. It is only required for versal master SLR devices.
*
* @return	XST_SUCCESS
*
*****************************************************************************/
static inline int XPlm_ConfigureDefaultNPll(void)
{
	/* Not Applicable for Versal Net */
	return XST_SUCCESS;
}

/************************** Function Prototypes ******************************/
#ifdef XPLMI_IPI_DEVICE_ID
u32 XPlm_SetPsmAliveStsVal(u32 Val);
u32 XPlm_UpdatePsmCounterVal(u32 Val);
#endif /* XPLMI_IPI_DEVICE_ID */
void XPlm_EnablePlatformSlaveErrors(void);
int XPlm_CompatibilityCheck(u32 PdiAddr);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_PLAT_H */
