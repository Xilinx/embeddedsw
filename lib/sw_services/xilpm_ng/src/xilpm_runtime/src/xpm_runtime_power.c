/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_runtime_power.h"
#include "xpm_core.h"
#include "xpm_requirement.h"
#include "xpm_device.h"
#include "xpm_runtime_device.h"
XStatus XPmPower_ForcePwrDwn(u32 NodeId)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Power = NULL;
	XPm_Device *Device = NULL;;
	const XPm_Core *Core;
	u32 i;

	if ((u32)XPM_NODESUBCL_POWER_DOMAIN != NODESUBCLASS(NodeId)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	/*
	 * PMC power domain can not be powered off.
	 */
	if ((u32)XPM_NODEIDX_POWER_PMC == NODEINDEX(NodeId)) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	/**
	 * Check if any power domain is ON and its parent is requested power
	 * domain, then explicitly power down those such power domains.
	 */
	for (i = (u32)XPM_NODEIDX_POWER_PMC; i < (u32)XPM_NODEIDX_POWER_MAX; i++) {
		Power = XPmPower_GetByIndex(i);
		if ((NULL == Power) || (NULL == Power->Parent) ||
		    (Power->Parent->Node.Id != NodeId) ||
		    ((u8)XPM_POWER_STATE_ON != Power->Node.State) ||
		    ((u32)XPM_NODESUBCL_POWER_DOMAIN !=
		    NODESUBCLASS(Power->Node.Id))) {
			continue;
		}

		Status = XPmPower_ForcePwrDwn(Power->Node.Id);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/**
	 * This is a special use case where child power domain is ON but no
	 * device of that power domain is requested. So use count of child power
	 * domain is 0. Also parent power domain usecount does not consider this
	 * child power domain as no device is requested.
	 *
	 * So to force power domain in this case, increment child and parent
	 * both power domain and call power down of child power domain which
	 * powers off child power domain without affecting usecount.
	 */
	Power = XPmPower_GetById(NodeId);
	if ((NULL != Power) && (0U == Power->UseCount) &&
	    ((u8)XPM_POWER_STATE_ON == Power->Node.State)) {
		Status = Power->HandleEvent(&Power->Node, (u32)XPM_POWER_EVENT_PWR_UP);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = Power->HandleEvent(&Power->Node, (u32)XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/*
	 * Release devices belonging to the power domain.
	 */
	for (i = 1; i < (u32)XPM_NODEIDX_DEV_MAX; i++) {
		/*
		 * Note: XPmDevice_GetByIndex() assumes that the caller is
		 * responsible for validating the Node ID attributes other than
		 * node index.
		 */
		Device = XPmDevice_GetByIndex(i);
		if ((NULL == Device) ||
		    ((u32)XPM_DEVSTATE_UNUSED == Device->Node.State)) {
			continue;
		}

		/*
		 * Check power topology of this device to identify if it belongs
		 * to the power domain.
		 */
		Power = Device->Power;
		while (NULL != Power) {
			if (NodeId == Power->Node.Id) {
				/* Disable the direct wake in case of force power down */
				if ((u32)XPM_NODESUBCL_DEV_CORE ==
				    NODESUBCLASS(Device->Node.Id)) {
					Core = (XPm_Core *)XPmDevice_GetById(Device->Node.Id);
					if (NULL != Core) {
						DisableWake(Core);
					}
				}

				Status = XPmRequirement_ReleaseFromAllSubsystem(Device);
				if (XST_SUCCESS != Status) {
					Status = XPM_PM_INVALID_NODE;
					goto done;
				}
			}
			Power = Power->Parent;
		}
	}

done:
	return Status;
}