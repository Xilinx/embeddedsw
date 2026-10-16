/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_runtime_power.h"
#include "xpm_core.h"
#include "xpm_requirement.h"
#include "xpm_device.h"
#include "xpm_runtime_device.h"
/****************************************************************************/
/**
 * @brief  Force power down of a power domain node and its children
 *
 * @param  NodeId: Node ID of the power domain to be forced down
 *
 * @return XST_SUCCESS on success, XPM_PM_INVALID_NODE for an invalid or
 *         PMC power domain node, or an error code otherwise
 *
 ****************************************************************************/
maybe_unused static XStatus XPmPower_ForcePwrDwn(u32 NodeId);

maybe_unused static XStatus XPmPower_ForcePwrDwn(u32 NodeId)
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

/****************************************************************************/
/**
 * @brief  Hold FPD on across a subsystem level request
 *
 * Takes one reference on FPD so that releasing devices during a subsystem
 * restart, forced power down or subsystem shutdown cannot switch it off.
 * The reference goes through the power state machine, so that taking an
 * unused FPD to used also requests its power parent, as for any other user.
 * When FPD is not on, nothing is held: it has no users that the scope could
 * release, and a power up inside the scope is counted by the power state
 * machine itself.
 *
 * @return FPD if a reference was taken, NULL otherwise. Pass the result to
 *	   XPmPower_ReleaseFpd() at the end of the scope.
 *
 ****************************************************************************/
XPm_Power *XPmPower_HoldFpd(void)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Fpd = XPmPower_GetById(PM_POWER_FPD);

	/* Nothing to hold unless FPD is on */
	if ((NULL == Fpd) || ((u8)XPM_POWER_STATE_ON != Fpd->Node.State)) {
		Fpd = NULL;
		goto done;
	}

	/* Take the reference through the power state machine */
	Status = Fpd->HandleEvent(&Fpd->Node, (u32)XPM_POWER_EVENT_PWR_UP);
	if (XST_SUCCESS != Status) {
		PmErr("FPD hold failed: 0x%x\r\n", Status);
		/* No reference was taken, so there is nothing to release */
		Fpd = NULL;
	}

done:
	return Fpd;
}

/****************************************************************************/
/**
 * @brief  Drop the FPD reference taken by XPmPower_HoldFpd()
 *
 * FPD is never powered off here. If this is its last reference while it is
 * on, FPD is left on and unused, and its power parent is released as the power
 * state machine does when a domain goes from used to unused, so that the next
 * request of FPD requests the parent again and the parent use count stays
 * balanced.
 *
 * Assumption of use: FPD is not the only user of its power parent. Otherwise
 * releasing the last hold makes the parent's state machine power the parent
 * down while FPD stays on. LPD always has other users, such as the ASU and
 * RPU IPIs.
 *
 * @param Fpd	Result of XPmPower_HoldFpd(); NULL if no reference was taken
 *
 ****************************************************************************/
void XPmPower_ReleaseFpd(XPm_Power *Fpd)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *Parent = NULL;

	if (NULL == Fpd) {
		/* XPmPower_HoldFpd() took no reference */
		goto done;
	}

	if (0U == Fpd->UseCount) {
		/* Unbalanced release: there is no reference left to drop */
		PmErr("FPD hold already released\r\n");
	} else if ((1U == Fpd->UseCount) &&
		   ((u8)XPM_POWER_STATE_ON == Fpd->Node.State) &&
		   (NULL != Fpd->Parent)) {
		/*
		 * Last reference: leave FPD on and unused, and release its
		 * parent as the power state machine does from used to unused.
		 */
		Fpd->UseCount = 0U;
		Parent = Fpd->Parent;
		Status = Parent->HandleEvent(&Parent->Node, (u32)XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			PmErr("FPD parent release failed: 0x%x\r\n", Status);
		}
	} else {
		/* Otherwise only drop this reference, without a state change */
		Fpd->UseCount--;
	}

done:
	return;
}
