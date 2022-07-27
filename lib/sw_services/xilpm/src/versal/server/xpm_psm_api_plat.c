/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "sleep.h"
#include "xplmi.h"
#include "xplmi_modules.h"
#include "xpm_defs.h"
#include "xpm_psm_api.h"
#include "xpm_psm.h"
#include "xpm_common.h"
#include "xpm_core.h"
#include "xpm_device.h"
#include "xpm_ipi.h"
#include "xpm_regs.h"
#include "xpm_subsystem.h"
#include "xpm_requirement.h"
#include "xpm_cpmdomain.h"

u32 ProcDevList[PROC_DEV_MAX] = {
	[ACPU_0] = PM_DEV_ACPU_0,
	[ACPU_1] = PM_DEV_ACPU_1,
	[RPU0_0] = PM_DEV_RPU0_0,
	[RPU0_1] = PM_DEV_RPU0_1,
};

XStatus ReleaseDeviceLpd(void)
{
        XStatus Status = XST_FAILURE;
        XPm_Core *Core;

        /* Release PMC */
        Status = XPmDevice_Release(PM_SUBSYS_PMC, PM_DEV_IPI_PMC, XPLMI_CMD_SECURE);
        if (XST_SUCCESS != Status) {
                PmErr("Error %d in  XPmDevice_Release(PM_SUBSYS_PMC, PM_DEV_IPI_PMC)\r\n");
                goto done;
        }
#ifdef DEBUG_UART_PS
        Status = XPmDevice_Release(PM_SUBSYS_PMC, NODE_UART, XPLMI_CMD_SECURE);
        if (XST_SUCCESS != Status) {
                PmErr("PMC Error %d in  XPmDevice_Release(PM_SUBSYS_DEFAULT, PM_DEV_UART_0)\r\n");
                goto done;
        }
#endif
        /* Power down PSM core */
        Core = (XPm_Core *)XPmDevice_GetById(PM_DEV_PSM_PROC);
        if (NULL != Core->CoreOps->PowerDown) {
                Status = Core->CoreOps->PowerDown(Core);
                if (XST_SUCCESS != Status) {
                        PmErr("Error %d in PSM core PowerDown\r\n");
                        goto done;
                }
        }

        /* Release PSM */
        Status = XPmDevice_Release(PM_SUBSYS_PMC, PM_DEV_PSM_PROC, XPLMI_CMD_SECURE);
        if (XST_SUCCESS != Status) {
                PmErr("Error %d in  XPmDevice_Release(PM_SUBSYS_DEFAULT, PM_DEV_PSM_PROC)\r\n");
        }

done:
        return Status;
}

/****************************************************************************/
/**
 * @brief This Function sends a CCIX_EN IPI to PSM if it design used is a
 * valid CPM CCIX design.
 *
 * @param PowerId	NodeId of CPM Power Domain
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 * @note none
 *
 ****************************************************************************/
XStatus XPm_CCIXEnEvent(u32 PowerId)
{
	u32 Payload[PAYLOAD_ARG_CNT];
	XStatus Status = XST_FAILURE;
	const XPm_CpmDomain *Cpm;
	u32 RegAddr;
	u32 RegVal;

	switch (PowerId) {
	case PM_POWER_CPM5:
		RegAddr	= CPM5_PCIE_ATTRIB_0_TDVSEC_NXT_PTR;
		Status = XST_SUCCESS;
		break;
	case PM_POWER_CPM:
		RegAddr	= PCIE_ATTRIB_0_TDVSEC_NXT_PTR;
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPM_INVALID_PWRDOMAIN;
		break;
	}

	if (XST_SUCCESS != Status) {
		goto done;
	}

	Cpm = (XPm_CpmDomain *)XPmPower_GetById(PowerId);
	if (NULL == Cpm) {
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	PmIn32(RegAddr, RegVal);

	if (RegVal == DVSEC_PCSR_START_ADDR)  {
		Payload[0] = PSM_API_CCIX_EN;
		Payload[1] = PowerId;
		Payload[2] = Cpm->CpmSlcrBaseAddr;

		Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);
	} else {
		Status = XST_SUCCESS;
	}

done:
	return Status;
}
