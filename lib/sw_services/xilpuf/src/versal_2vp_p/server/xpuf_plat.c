/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file src/versal_2vp_p/server/xpuf_plat.c
*
* This file contains platform specific APIs for PUF for VERSAL_2VP_P.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   sd   04/13/2026 Initial release
*
* </pre>
*
*******************************************************************************/
/**
 * @addtogroup xpuf_server_apis XilPuf Server APIs
 * @{
 */
/****************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_io.h"
#include "xpuf_plat.h"
#include "xpuf.h"
#include "xpuf_hw.h"

/**
 * @cond xpuf_internal
 * @{
 */

/*****************************************************************************/
/**
 *
 * @brief	This function checks Global Variation Filter option.
 *		VERSAL_2VP_P does not use the Global Variation Filter check,
 *		so this function always returns XST_SUCCESS.
 *
 * @param	PufData - Pointer to XPuf_Data structure which includes options
 *		to configure PUF
 *
 * @return
 *		- XST_SUCCESS always.
 *
 *****************************************************************************/
int XPuf_CheckGlobalVariationFilter(const XPuf_Data *PufData)
{
	(void)PufData;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function configures the RO_SWAP register in PUF as per the
 * 		user provided value.
 *
 * @param	PufData Pointer to XPuf_Data structure which includes options
 *		to configure PUF.
 *
 *****************************************************************************/
void XPuf_SetRoSwap(const XPuf_Data *PufData)
{
	/** - Update PUF Ring Oscillator Swap setting. */
	Xil_Out32(XPUF_PMC_GLOBAL_BASEADDR + XPUF_PMX_GLOBAL_PUF_RO_SWP_OFFSET,
		PufData->RoSwapVal);
}

/*****************************************************************************/
/**
 *
 * @brief	This function returns PUF registration disable status.
 *
 * @return
 *		- XPUF_PUF_REGIS_DIS When PUF registration is disabled.
 *		- FALSE when PUF registration is enabled.
 *
 *****************************************************************************/
u32 XPuf_IsRegistrationDisabled(void)
{
	return (XPuf_ReadReg(XPUF_EFUSE_CACHE_BASEADDR,
		XPUF_PUF_ECC_PUF_CTRL_OFFSET) & XPUF_PUF_REGIS_DIS);
}

/*****************************************************************************/
/**
 *
 * @brief	This function checks if changing IRO frequency before
 * 		PUF operation is required or not.
 *
 * @return
 * 		- XPUF_IROFREQ_CHANGE_REQD if change in IRO frequency is required.
 * 		- XPUF_IROFREQ_CHANGE_NOTREQD if change in IRO frequency is not required.
 *
 *****************************************************************************/
u32 XPuf_IsIroFreqChangeReqd(void)
{
	u32 Result = XPUF_IROFREQ_CHANGE_NOTREQD;
	u32 AnlgOscSw1Lp = XPuf_ReadReg(XPUF_EFUSE_CTRL_BASEADDR, XPUF_ANLG_OSC_SW_1LP_OFFSET);
	u32 IroTrimFuseSelect = AnlgOscSw1Lp & XPUF_IRO_TRIM_FUSE_SEL_BIT;
	u32 RomRsvdRegVal = XPuf_ReadReg(XPUF_EFUSE_CACHE_BASEADDR,
			XPUF_EFUSE_CACHE_ROM_RSVD_OFFSET);
	u32 IroSwapVal = RomRsvdRegVal & XPUF_IRO_SWAP;

	if ((IroSwapVal != XPUF_IRO_SWAP) &&
		(IroTrimFuseSelect == XPUF_EFUSE_CTRL_IRO_TRIM_FAST)) {
		Result = XPUF_IROFREQ_CHANGE_REQD;
	}

	return Result;
}

/**
 * @}
 * @endcond
 */
/** @} */
