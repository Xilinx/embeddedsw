/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
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
* 2.2   kpt  08/14/2023 Renamed XPuf_IsRegistrationEnabled to XPuf_IsRegistrationDisabled
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
int XPuf_CheckGlobalVariationFilter(const XPuf_Data *PufData)
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
void XPuf_SetRoSwap(const XPuf_Data *PufData)
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
 * @brief	This function returns PUF registration disable status.
 *
 * @return	XPUF_PUF_REGIS_DIS  - When PUF registration is disabled.
 *		FALSE - When PUF registration is enabled or not supported incase of versal.
 *
 *****************************************************************************/
u32 XPuf_IsRegistrationDisabled(void)
{
	u32 PufEccCtrlValue = FALSE;

#if defined (VERSAL_NET)
	PufEccCtrlValue = XPuf_ReadReg(XPUF_EFUSE_CACHE_BASEADDR,
					XPUF_PUF_ECC_PUF_CTRL_OFFSET) & XPUF_PUF_REGIS_DIS;
#endif

	return PufEccCtrlValue;
}

/*****************************************************************************/
/**
 *
 * @brief	This function checks if changing IRO frequency before
 * 		PUF operation is required or not.
 *
 * @return	TRUE - Changing IRO frequency is required
 * 		FALSE - Changing IRO frequency is not required
 *
 * @note	In Versal, ROM always operates at PMC IRO frequency of 320 MHz.
 * 		For MP/HP devices, PLM updated the PMC IRO frequency to 400 MHz.
 * 		Since the IRO frequency at which PUF operates needs to be in
 * 		sync with ROM, so it is mandatory to update IRO frequency to 320 MHz
 * 		for Versal MP/HP devices before any PUF operation.
 * 		In case of Versal Net, ROM can also operate at 400 MHz IRO frequency.
 * 		So it might not always be required to change the frequency before
 * 		PUF operation.
 *****************************************************************************/
u32 XPuf_IsIroFreqChangeReqd(void)
{
	u32 Result = XPUF_IROFREQ_CHANGE_NOTREQD;
	u32 AnlgOscSw1Lp = XPuf_ReadReg(XPUF_EFUSE_CTRL_BASEADDR, XPUF_ANLG_OSC_SW_1LP_OFFSET);
	u32 IroTrimFuseSelect = AnlgOscSw1Lp & XPUF_IRO_TRIM_FUSE_SEL_BIT;

#if defined (VERSAL_NET)
	u32 RomRsvdRegVal = XPuf_ReadReg(XPUF_EFUSE_CACHE_BASEADDR,
			XPUF_EFUSE_CACHE_ROM_RSVD_OFFSET);
	u32 IroSwapVal = RomRsvdRegVal & XPUF_IRO_SWAP;

	if ((IroSwapVal != XPUF_IRO_SWAP) &&
		(IroTrimFuseSelect == XPUF_EFUSE_CTRL_IRO_TRIM_FAST)) {
		Result = XPUF_IROFREQ_CHANGE_REQD;
	}
	else {
		Result = XPUF_IROFREQ_CHANGE_NOTREQD;
	}
#else
	if (IroTrimFuseSelect == XPUF_EFUSE_CTRL_IRO_TRIM_FAST) {
		Result = XPUF_IROFREQ_CHANGE_REQD;
	}
#endif

	return Result;

}

/** @}
@endcond */
