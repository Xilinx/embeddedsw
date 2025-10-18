/***************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file versal_2vp_p/xplm_plat.h
*
* This file contains versal_2vp_p specific declarations PLM module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   sd  10/13/25 Initial release
*
* </pre>
*
* @note
*
***************************************************************************************************/

#ifndef XPLM_PLAT_H
#define XPLM_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xplmi_hw.h"

/************************************ Constant Definitions ****************************************/

#define XPlm_OcpHandler		NULL /**< Ocp Handler */

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/**************************************************************************************************/
/**
 * @brief	This function is not applicable for versal_2vp.
 *
 * @param	PdiAddr is the address of the PDI which has new PLM.
 *
 * @return
 * 		- XST_SUCCESS.
 *
 **************************************************************************************************/
static inline int XPlm_CompatibilityCheck(u32 PdiAddr)
{
	/* Not Applicable for Versal_2vp */
	return XST_SUCCESS;
}

/**************************************************************************************************/
/**
 * @brief	This function is not applicable for versal_2vp.
 *
 * @param	PdiAddr is the address of the PDI.
 *
 * @return
 * 		- XST_SUCCESS.
 *
 **************************************************************************************************/
static inline int XPlm_CheckPsmPresenceInOD(u32 PdiAddr)
{
	/* Not Applicable for Versal_2vp */
	return XST_SUCCESS;
}

/************************************ Function Prototypes *****************************************/
#ifdef XPLMI_IPI_DEVICE_ID
u32 XPlm_SetPsmAliveStsVal(u32 Val);
u32 XPlm_UpdatePsmCounterVal(u32 Val);
#endif /* XPLMI_IPI_DEVICE_ID */
int XPlm_ConfigureDefaultNPll(void);
void XPlm_EnablePlatformSlaveErrors(void);

/************************************ Variable Definitions ****************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_PLAT_H */
