/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xpuf_plat.c
*
* This file contains platform specific APIs for PUF. In case the API is not
* supported for the platform it will be treated as a dummy call.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 2.0   har  07/04/2022 Initial release
*       kpt  08/31/2022 Fixed logical error in XPuf_CheckGlobalVariationFilter
* 2.1   am   02/13/2023 Fixed MISRA C violations
*
* </pre>
*
* @note
*
*******************************************************************************/
/****************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_io.h"
#include "xpuf_plat.h"
#include "xpuf.h"
#include "xpuf_hw.h"

/*************************** Constant Definitions *****************************/
/** @cond xpuf_internal
@{
*/
#define XPUF_SHUT_GLB_VAR_FLTR_ENABLED_SHIFT	(31)
		/**< Shift for Global Variation Filter bit in shutter value */

/***************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
 *
 * @brief	This function checks Global Variation Filter option
 *
 * @param	PufData - Pointer to XPuf_Data structure which includes options
 *			  to configure PUF
 *
 * @return	Status
 *
 *****************************************************************************/
int XPuf_CheckGlobalVariationFilter(XPuf_Data *PufData)
{
	int Status = XST_FAILURE;

#if (defined(versal) && !defined(VERSAL_NET))
	/**
	 * Error code shall be returned if MSB of PUF shutter value does not
	 * match with Global Variation Filter option
	 */
	if (((PufData->ShutterValue >> XPUF_SHUT_GLB_VAR_FLTR_ENABLED_SHIFT) ^
		PufData->GlobalVarFilter) == TRUE) {
		Status = XPUF_SHUTTER_GVF_MISMATCH;
	}
	else {
		Status = XST_SUCCESS;
	}
#else
	(void)PufData;
	Status = XST_SUCCESS;
#endif

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function configures the RO_SWAP register in PUF as per the
 * 		user provided value.
 *
 * @param       PufData - Pointer to XPuf_Data structure which includes options
 *		to configure PUF
 *
 *****************************************************************************/
void XPuf_SetRoSwap(XPuf_Data *PufData)
{
#if defined (VERSAL_NET)
	/**
	 * Update PUF Ring Oscillator Swap setting
	 */
	Xil_Out32(XPUF_PMC_GLOBAL_BASEADDR + XPUF_PMX_GLOBAL_PUF_RO_SWP_OFFSET,
		PufData->RoSwapVal);
#else
	(void)PufData;
#endif
}

/*****************************************************************************/
/**
 *
 * @brief	This function checks if registration is enabled or not.
 *
 * @return	XST_SUCCESS - Registration is enabled
 * 		XPUF_ERROR_REGISTRATION_INVALID - Registration is disabled
 *
 *****************************************************************************/
int XPuf_IsRegistrationEnabled(u32 PufEccCtrlValue)
{
	int Status = XST_FAILURE;

#if defined (VERSAL_NET)
	if ((PufEccCtrlValue & XPUF_PUF_REGIS_DIS) == XPUF_PUF_REGIS_DIS) {
		Status = XST_FAILURE;
	}
	else {
		Status = XST_SUCCESS;
	}
#else
	(void)PufEccCtrlValue;
	Status = XST_SUCCESS;
#endif

	return Status;
}

/** @}
@endcond */
