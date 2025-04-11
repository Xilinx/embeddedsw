/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_feature_check.h"
#include "pm_api_version.h"
#include "xpm_err.h"
#include "xplmi_modules.h"
#include "xpm_common.h"

/****************************************************************************/
/**
 * @brief  This function returns supported version of the given API.
 *
 * @param  ApiId	API ID to check
 * @param  Version	pointer to array of 4 words
 *  - version[0] - EEMI API version number
 *  - version[1] - lower 32-bit bitmask of IOCTL or QUERY ID
 *  - version[2] - upper 32-bit bitmask of IOCTL or Query ID
 *  - Only PM_FEATURE_CHECK version 2 supports 64-bit bitmask
 *  - i.e. version[1] and version[2]
 * @return XST_SUCCESS if successful else XST_NO_FEATURE.
 *
 * @note   Remove CDO-only commands from versioning as it is for internal
 * use only, so no need to consider for versioing.
 *
 ****************************************************************************/
XStatus XPm_FeatureCheck(const u32 ApiId, u32 *const Version)
{
	XStatus Status = XST_FAILURE;

	if (NULL == Version) {
		Status = XPM_ERR_VERSION;
		goto done;
	}
	XPlmi_Module* PmModule = XPlmi_GetModule(XPLMI_MODULE_XILPM_ID);
	if (NULL == PmModule) {
		PmErr("PmModule is NULL\n\r");
		goto done;
	}

	if ((PmModule->CmdCnt <= ApiId) || (NULL == PmModule->AccessPermBufferPtr) ||
	    (XPLMI_GET_ALL_IPI_MASK(XPLMI_NO_IPI_ACCESS) == PmModule->AccessPermBufferPtr[ApiId])) {
		PmErr("API 0x%x is not accessible!\n\r", ApiId);
		Status = XPM_PM_NO_ACCESS;
		goto done;
	}

	switch (ApiId) {
	case PM_API(PM_GET_API_VERSION):
	case PM_API(PM_GET_NODE_STATUS):
	case PM_API(PM_ABORT_SUSPEND):
	case PM_API(PM_REQUEST_WAKEUP):
	case PM_API(PM_SET_WAKEUP_SOURCE):
	case PM_API(PM_SYSTEM_SHUTDOWN):
	case PM_API(PM_SET_REQUIREMENT):
	case PM_API(PM_SET_MAX_LATENCY):
	case PM_API(PM_RESET_ASSERT):
	case PM_API(PM_RESET_GET_STATUS):
	case PM_API(PM_INIT_FINALIZE):
	case PM_API(PM_GET_CHIPID):
	case PM_API(PM_CLOCK_ENABLE):
	case PM_API(PM_CLOCK_DISABLE):
	case PM_API(PM_CLOCK_GETSTATE):
	case PM_API(PM_CLOCK_SETDIVIDER):
	case PM_API(PM_CLOCK_GETDIVIDER):
	case PM_API(PM_CLOCK_SETPARENT):
	case PM_API(PM_CLOCK_GETPARENT):
	case PM_API(PM_PLL_SET_PARAMETER):
	case PM_API(PM_PLL_GET_PARAMETER):
	case PM_API(PM_PLL_SET_MODE):
	case PM_API(PM_PLL_GET_MODE):
	case PM_API(PM_PINCTRL_REQUEST):
	case PM_API(PM_PINCTRL_RELEASE):
	case PM_API(PM_PINCTRL_GET_FUNCTION):
	case PM_API(PM_PINCTRL_SET_FUNCTION):
	case PM_API(PM_PINCTRL_CONFIG_PARAM_GET):
	case PM_API(PM_PINCTRL_CONFIG_PARAM_SET):
		*Version = XST_API_BASE_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_FEATURE_CHECK):
		*Version = XST_API_PM_FEATURE_CHECK_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_REGISTER_NOTIFIER):
		*Version = XST_API_REG_NOTIFIER_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_SELF_SUSPEND):
		*Version = XST_API_SELF_SUSPEND_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_FORCE_POWERDOWN):
		*Version = XST_API_FORCE_POWERDOWN_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_REQUEST_NODE):
		*Version = XST_API_REQUEST_NODE_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_RELEASE_NODE):
		*Version = XST_API_RELEASE_NODE_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_GET_OP_CHARACTERISTIC):
		Version[0] = XST_API_GET_OP_CHAR_VERSION;
		Version[1] = (u32)(PM_GET_OP_CHAR_FEATURE_BITMASK);
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_QUERY_DATA):
		Version[0] = XST_API_QUERY_DATA_VERSION;
		Version[1] = (u32)(PM_QUERY_FEATURE_BITMASK);
		Version[2] = (u32)(PM_QUERY_FEATURE_BITMASK >> 32);
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_BISR):
	case PM_API(PM_APPLY_TRIM):
		*Version = XST_API_BASE_VERSION;
		Status = XST_SUCCESS;
		break;
	case PM_API(PM_IOCTL):
		Version[0] = XST_API_PM_IOCTL_VERSION;
		Version[1] = (u32)(PM_IOCTL_FEATURE_BITMASK);
		Version[2] = (u32)(PM_IOCTL_FEATURE_BITMASK >> 32);
		Status = XST_SUCCESS;
		break;
	default:
		*Version = 0U;
		Status = XPM_NO_FEATURE;
		break;
	}

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}
