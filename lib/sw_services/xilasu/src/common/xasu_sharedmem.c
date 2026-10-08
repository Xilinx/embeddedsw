/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_sharedmem.c
 *
 * This file contains the IPI communication channel function definitions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vas  04/07/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_sharedmem_apis Shared Memory APIs
 * @{
*/

/************************************ Include Files **********************************************/
#include "xasu_sharedmem.h"

/********************************* Constant Definitions ******************************************/
/*
 * XAsu_GetModuleInfoSlot() in xasu_sharedmem.h indexes the module info region for module IDs below
 * XASU_RTCA_MODULE_INFO_ENTRIES and the extension region for the rest, so the two regions together
 * have to cover every module ID, and each region has to hold the entries it claims. Nothing else
 * ties XASU_MAX_MODULES in xasu_def.h to the region sizes here, so raising it past the two regions,
 * or growing XAsu_CryptoAlgInfo so that either region runs into whatever RTCA field follows it,
 * fails the build rather than writing past the region.
 *
 * These live here rather than beside the function because that header is wrapped in extern "C" and
 * so can be included from a C++ translation unit, where _Static_assert is not valid. This file is
 * always compiled as C.
 */
_Static_assert(XASU_MAX_MODULES <= (2U * XASU_RTCA_MODULE_INFO_ENTRIES),
	       "XASU_MAX_MODULES exceeds the two RTCA module info regions");
_Static_assert((XASU_RTCA_MODULE_INFO_ENTRIES * sizeof(XAsu_CryptoAlgInfo)) <=
	       (XASU_RTCA_DBG_LOG_BUF_INFO_ADDR - XASU_RTCA_MODULE_INFO_BASEADDR),
	       "RTCA module info region overruns the debug log buffer info at RTCA 0x15C");
_Static_assert((XASU_RTCA_MODULE_INFO_ENTRIES * sizeof(XAsu_CryptoAlgInfo)) <=
	       (XASU_RTCA_PL_CFG_BASE_ADDR - XASU_RTCA_MODULE_INFO_EXT_BASEADDR),
	       "RTCA module info extension region overruns the PL configuration window");

/*********************************** Macros Definitions ******************************************/
#define XASU_INVALID_SUBSYS_ID		(0xFFFFFFFFU) /**< Invalid Subsystem ID */

/*********************************** Type Definitions ********************************************/

/********************************* Variable Definitions ******************************************/
/** Pointer to the IPI communication channel information received from user configuration */
XAsu_CommChannelInfo *CommChannelInfo = (XAsu_CommChannelInfo *)(UINTPTR)
	XASU_RTCA_COMM_CHANNEL_INFO_ADDR;

/****************************** Inline Function Definitions **************************************/

/********************************** Function Prototypes ******************************************/


/*************************************************************************************************/
/**
 * @brief	This function returns IPI Mask corresponding to the Channel Index.
 *
 * @param	ChannelIndex	IPI Channel Index.
 *
 * @return
 *	- IPI Bit Mask.
 *
 *************************************************************************************************/
u32 XAsu_GetIpiMask(u32 ChannelIndex)
{
	u32 IpiBitMask;

	if (ChannelIndex >= CommChannelInfo->NumOfIpiChannels) {
		IpiBitMask = 0U;
	} else {
		IpiBitMask = CommChannelInfo->Channel[ChannelIndex].IpiBitMask;
	}

	return IpiBitMask;
}

/*************************************************************************************************/
/**
 * @brief	This function provides subsystem ID based on IPI mask.
 *
 * @param	IpiMask		IPI mask.
 *
 * @return
 *	- Returns subsystem ID correspond to IPI mask.
 *	- XASU_INVALID_SUBSYS_ID if no matching IPI mask is found.
 *
 *************************************************************************************************/
u32 XAsu_GetSubsysIdFromIpiMask(u32 IpiMask)
{
	u32 ChannelIndex;
	u32 SubsystemId = XASU_INVALID_SUBSYS_ID;

	for (ChannelIndex = 0U; ChannelIndex < CommChannelInfo->NumOfIpiChannels; ++ChannelIndex) {
		if (CommChannelInfo->Channel[ChannelIndex].IpiBitMask == IpiMask) {
			if (CommChannelInfo->Channel[ChannelIndex].SubsystemId != 0U) {
				SubsystemId = CommChannelInfo->Channel[ChannelIndex].SubsystemId;
			}
			goto END;
		}
	}

END:
	return SubsystemId;
}
/** @} */
