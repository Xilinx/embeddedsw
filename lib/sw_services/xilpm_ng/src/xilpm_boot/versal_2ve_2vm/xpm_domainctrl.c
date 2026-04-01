/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#if defined(XILPM_NG_DOMAIN_CONTROL_GPIO)

#include "xpm_domainctrl.h"

#include "xpm_regs.h"
#include "xpm_common.h"
#include "xpm_debug.h"
#include "xpm_powerdomain.h"
#include "xpm_device.h"
#include "xpm_api.h"

/**
 * @brief 	Helper function to set domain control for a given power domain
 *
 * @param 	DomainCtrl: Pointer to the domain control object to be associated with the power domain
 * @param 	PowerDomainId: Node Id of the power domain for which the domain control is being set
 *
 * @return 	XST_SUCCESS if successful else XST_FAILURE or error code
 */
static XStatus SetDomainCtrl(XPm_DomainCtrl *DomainCtrl, u32 PowerDomainId) {
	XStatus Status = XST_FAILURE;
	XPm_PowerDomain *PwrDomain;

	if (NULL == DomainCtrl) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	PwrDomain = (XPm_PowerDomain *)XPmPower_GetById(PowerDomainId);
	if (NULL == PwrDomain) {
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	/* save the Domain Control object */
	PwrDomain->DomainCtrl = DomainCtrl;

	Status = XST_SUCCESS;

done:
	return Status;

}

/****************************************************************************/
/**
 * @brief  Initialize domainctrl node base class
 *
 * @param  DomainCtrl: Pointer to power domainctrl struct
 * @param  DomainCtrlId: Node Id assigned to a Power DomainCtrl node
 * @param  Args: Arguments for power domain control
 * @param  NumArgs: Number of arguments for power domain
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note Args is dependent on the domainctrl type. Passed arguments will be
 *		 different for mode type.
 *
 ****************************************************************************/
XStatus XPmDomainCtrl_Init(XPm_DomainCtrl *DomainCtrl, u32 DomainCtrlId, const u32 *Args, u32 NumArgs) {
	XStatus Status = XST_FAILURE;
	u32 Mode, TotalModes, CmdLen, ArgIndex = 1U;
	u8 ModeId;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Invalid Power Domain Control NodeId */
	if ((u32)XPM_NODEIDX_POWER_MAX <= NODEINDEX(DomainCtrlId)) {
		DbgErr = XPM_INT_ERR_INVALID_NODE_IDX;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/**
	 * sanity check the cdo structure:
	 * - only GPIO Type is supported for Domain Control (0x2)
	 * - # of arguments > 3 (Domain Control Id, Type, Parent Id, ...)
	 * - 3rd argument must be GPIO Device Node
	 */
	if ((NumArgs <= 3U) ||
	    (XPM_DOMAIN_CONTROL_GPIO != Args[ArgIndex]) ||
	    ((PM_DEV_GPIO != Args[ArgIndex + 1U]) &&
	    (PM_DEV_GPIO_PMC != Args[ArgIndex + 1U]))) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/**
	 * extract and save the necessary information for given Domain Control:
	 * arg0: Power Domain Control NodeId (0x453404F, 0x4534050U, ...)
	 * arg1: Type (0x2 for GPIO)
	 * arg2: Parent Device NodeId (to control power domains on HW level)
	 * arg3: # of modes supported for this Domain Control Id (0x2 - 0:OFF; 1:ON)
	 * arg4: [Length | Mode0] - Length of command for mode 0 and GPIO config for mode 0 (register offset, mask, value)
	 * arg5: [Length | Mode1] - Length of command for mode 1 and GPIO config for mode 1 (register offset, mask, value)
	 *
	 * Example:
	 * pm_add_node 0x4534053 0x2 0x18224023 0x2 0x300 0x0040 0x100000 0x000000
	 *					    0x301 0x0040 0x100000 0x100000
	*/
	/* check whether GPIO Device Node is present in the system */
	DomainCtrl->GpioNodeId = Args[++ArgIndex];
	if (NULL == XPmDevice_GetById(DomainCtrl->GpioNodeId)) {
		DbgErr = XPM_INT_ERR_DEVICE_INIT;
		Status = XPM_ERR_DEVICE_INIT;
		goto done;
	}

	/* max supported modes = 2 (ON/OFF) */
	TotalModes = Args[++ArgIndex];
	if (MAX_DOMAIN_CONTROL_MODES < TotalModes) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* extract and store GPIO HW information ( for each mode ) */
	for (Mode = 0U; Mode < TotalModes; Mode++) {
		/** check whether argument count is correct:
		 * We expect 4 more arguments after each mode
		 * 	1. Command Length | Mode
		 * 	2. GPIO Register Offset
		 * 	3. GPIO Mask
		 * 	4. GPIO Value
		*/
		if ((ArgIndex + 4U) >= NumArgs) {
			DbgErr = XPM_INT_ERR_INVALID_PARAM;
			Status = XST_INVALID_PARAM;
			goto done;
		}

		ArgIndex++;
		CmdLen = (u8)((Args[ArgIndex] >> GPIO_DOMAIN_CTRL_CMDLEN_SHIFT) & GPIO_DOMAIN_CTRL_MODE_MASK);
		if (MAX_GPIO_CTRL_CMD_LEN != CmdLen) {
			DbgErr = XPM_INT_ERR_INVALID_ARGS;
			Status = XST_INVALID_PARAM;
			goto done;
		}
		ModeId = (u8)(Args[ArgIndex] & GPIO_DOMAIN_CTRL_MODE_MASK);
		if (ModeId >= MAX_DOMAIN_CONTROL_MODES) {
			DbgErr = XPM_INT_ERR_INVALID_PARAM;
			Status = XST_INVALID_PARAM;
			goto done;
		}

		DomainCtrl->GpioCtrl[ModeId].Offset = (u16)Args[++ArgIndex];
		DomainCtrl->GpioCtrl[ModeId].Mask = Args[++ArgIndex];
		DomainCtrl->GpioCtrl[ModeId].Value = Args[++ArgIndex];
	}

	/* Find and store the Domain Control object in it's respective power domain struct */
	switch (DomainCtrlId) {
	case PM_POWER_FPD_DOMAIN_CTRL:
		Status = SetDomainCtrl(DomainCtrl, PM_POWER_FPD);
		break;
	case PM_POWER_PLD_DOMAIN_CTRL:
		Status = SetDomainCtrl(DomainCtrl, PM_POWER_PLD);
		break;
	default:
		DbgErr = XPM_INT_ERR_INVALID_NODE_IDX;
		Status = XST_INVALID_PARAM;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief  DomainCtrl control
 *
 * @param  DomainCtrlId: Domain Control ID
 * @param  State: Domain power state
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 ****************************************************************************/
XStatus XPmDomainCtrl_Control(const XPm_PowerDomain *PwrDomain, u32 State) {
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_DomainCtrl *DomainCtrl;
	u32 DomainId = 0U, BaseAddress = 0U;
	u8 Mode;

	if (NULL == PwrDomain) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}
	DomainCtrl = PwrDomain->DomainCtrl;

	if (NULL == PwrDomain->DomainCtrl) {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XST_INVALID_PARAM;
		goto done;
	}
	DomainId = PwrDomain->Power.Node.Id;

	/* Get the GPIO device base address */
	Status = XPm_GetDeviceBaseAddr(DomainCtrl->GpioNodeId, &BaseAddress);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* select domain control mode */
	if (XPM_POWER_STATE_ON == State) {
		Mode = 1U;
		PmDbg("Turning %x domain on now\r\n", DomainId);
	}
	else if (XPM_POWER_STATE_OFF == State) {
		Mode = 0U;
		PmDbg("Turning %x domain off now\r\n", DomainId);
	}
	else {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/**
	 * Check whether requested Power State is achieved or not by
	 * reading the corresponding power rail of given Power Domain
	 * note: XST_SUCCESS: Rail ON, Error code: Rail OFF
	 *
	 * - XPM_NODEIDX_POWER_FPD_DOMAIN_CTRL (FPD) => PM_POWER_VCCINT_PSFP
	 * - XPM_NODEIDX_POWER_PLD_DOMAIN_CTRL (PLD) => PM_POWER_VCCINT_PL
	 *
	 * TBD: This can further be optimized by performing power check before
	 * triggering HW sequencer
	*/
	/* Set GPIO to trigger external HW Sequencer for Power Transition */
	XPm_RMW32(BaseAddress + DomainCtrl->GpioCtrl[Mode].Offset,
		DomainCtrl->GpioCtrl[Mode].Mask, DomainCtrl->GpioCtrl[Mode].Value);

	Status = XST_SUCCESS;

	/**
	 * TODO: FIXME: Add a robust time-out/N-retry mechanism to check whether requested power state is achieved or not.
	 * This is because external HW sequencer may take some time to perform the power transition and
	 * Note: we only have to check for POWER OFF ( as in PowerUp Sequence, we already check power when performing initialization )
	 */
	/* check power for PSFP Power Rail */
	/* check power for PL Power Rail */

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

#endif /* XILPM_NG_DOMAIN_CONTROL_GPIO */
