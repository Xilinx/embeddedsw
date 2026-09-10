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
 * @brief Initialize a GPIO domain-control object from topology arguments.
 *
 * Validates the domain-control node and its GPIO device, decodes the supported
 * mode commands into @p DomainCtrl, and attaches the object to the FPD or PLD
 * power-domain object selected by @p DomainCtrlId.
 *
 * @param DomainCtrl Pointer to the domain-control object to populate.
 * @param DomainCtrlId Domain-control node ID that selects the owning domain.
 * @param Args Topology argument array containing the control type, GPIO node,
 *             mode count, and GPIO register commands.
 * @param NumArgs Number of words available in @p Args.
 *
 * @return XST_SUCCESS when the object is decoded and attached;
 *         XST_INVALID_PARAM for malformed arguments, an unsupported node or
 *         mode, or an invalid node index; XPM_ERR_DEVICE_INIT when the GPIO
 *         device is absent; or XPM_INVALID_PWRDOMAIN when the owning power
 *         domain is absent.
 *
 * @note @p DomainCtrl and @p Args must point to writable object storage and a
 *       readable argument array, respectively.
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
	ArgIndex++;
	DomainCtrl->GpioNodeId = Args[ArgIndex];
	if (NULL == XPmDevice_GetById(DomainCtrl->GpioNodeId)) {
		DbgErr = XPM_INT_ERR_DEVICE_INIT;
		Status = XPM_ERR_DEVICE_INIT;
		goto done;
	}

	/* max supported modes = 2 (ON/OFF) */
	ArgIndex++;
	TotalModes = Args[ArgIndex];
	if (MAX_DOMAIN_CONTROL_MODES < TotalModes) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * ArgIndex remains on the preceding field after each mode, so the first
	 * increment in the loop advances directly to the next command header.
	 */
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

		/* Decode and validate the command header for this mode */
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

		/* Store the register command under its decoded mode ID */
		ArgIndex++;
		DomainCtrl->GpioCtrl[ModeId].Offset = (u16)Args[ArgIndex];
		ArgIndex++;
		DomainCtrl->GpioCtrl[ModeId].Mask = Args[ArgIndex];
		ArgIndex++;
		DomainCtrl->GpioCtrl[ModeId].Value = Args[ArgIndex];
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
		break;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief Request an external GPIO-controlled power-domain transition.
 *
 * Resolves the GPIO controller associated with @p PwrDomain, selects its ON or
 * OFF command from @p State, writes the configured register field, and waits a
 * fixed interval for the external hardware sequencer. The function does not
 * poll the rail to verify that the requested state was reached.
 *
 * @param PwrDomain Pointer to the power domain whose control object supplies
 *                  the GPIO device and mode commands.
 * @param State Requested XPM_POWER_STATE_ON or XPM_POWER_STATE_OFF state.
 *
 * @return XST_SUCCESS after issuing a valid GPIO command; XPM_INVALID_PWRDOMAIN
 *         when @p PwrDomain is NULL; XST_INVALID_PARAM when no control object
 *         is attached or @p State is unsupported; or an error returned by
 *         XPm_GetDeviceBaseAddr() when the GPIO base address cannot be found.
 *
 * @note A successful return means the command was issued, not that the rail
 *       transition was measured as complete.
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

	/* A power domain without a control object cannot issue GPIO commands */
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

	/*
	 * TODO: Replace this fixed delay with a time-out/retry loop that polls
	 * the power rail state. The external HW sequencer needs time to
	 * complete the transition.
	 */
	usleep(150000);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

#endif /* XILPM_NG_DOMAIN_CONTROL_GPIO */
