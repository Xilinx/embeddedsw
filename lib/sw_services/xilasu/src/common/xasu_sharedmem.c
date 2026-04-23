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
			SubsystemId = CommChannelInfo->Channel[ChannelIndex].SubsystemId;
			goto END;
		}
	}

END:
	return SubsystemId;
}
/** @} */
