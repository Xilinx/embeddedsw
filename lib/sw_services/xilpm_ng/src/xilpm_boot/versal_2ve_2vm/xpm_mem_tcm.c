/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xpm_mem.h"
#include "xpm_debug.h"
#include "xpm_rpucore.h"
#include "xpm_mem_tcm.h"
#include "xplmi_dma.h"

/****************************************************************************/
/**
 * @brief  Release the associated RPU reset and initialize TCM ECC.
 *
 * The owning RPU is reset and halted, its reset is released, and the complete
 * TCM address range is passed to the ECC initialization routine.
 *
 * @param  Device  Pointer to the TCM memory device
 *
 * @return XST_SUCCESS on success, XST_INVALID_PARAM for a NULL device or an
 *         unknown TCM ID, or an error from the RPU reset or ECC initialization
 *         operation
 *
 ****************************************************************************/
XStatus XPmMem_TcmResetRelease(const XPm_Device *Device)
{
	XStatus Status = XST_FAILURE;
	u32 RpuId = 0U;
	const XPm_MemDevice *Tcm = NULL;
	u32 Size = 0U;

	/* Validate the device before interpreting it as TCM memory. */
	if (NULL == Device) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Tcm = (const XPm_MemDevice *)Device;

	/* Resolve the RPU core that owns the supplied TCM bank. */
	Status = XPm_GetRpuByTcmId(Device->Node.Id, &RpuId);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to get RPU ID for TCM ID: 0x%x\n", Device->Node.Id);
		goto done;
	}
	/* Quiesce the owning core before making its TCM accessible. */
	Status = XPmRpuCore_ResetAndHalt(RpuId);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to halt RPU ID: 0x%x\n", RpuId);
		goto done;
	}
	/* Release reset so the TCM address range can be initialized. */
	Status = XPmRpuCore_ReleaseReset(RpuId);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to release reset for RPU ID: 0x%x\n", RpuId);
		goto done;
	}

	/* TCM is now accessible -- ECC initialize the memory */
	Size = Tcm->EndAddress - Tcm->StartAddress + 1U;
	/* Initialize ECC only when the inclusive address range is nonempty. */
	if (0U != Size) {
		Status = XPlmi_EccInit((u64)Tcm->StartAddress, Size);
		if (XST_SUCCESS != Status) {
			PmErr("ECC init failed for TCM 0x%x\r\n", Device->Node.Id);
			goto done;
		}
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Map a TCM device node ID to its owning RPU core node ID.
 *
 * @param  TcmId  TCM device node ID
 * @param  RpuId  Non-NULL pointer to receive the owning RPU core node ID
 *
 * @return XST_SUCCESS for a recognized TCM ID, or XST_INVALID_PARAM when
 *         TcmId is not recognized
 *
 ****************************************************************************/
XStatus XPm_GetRpuByTcmId(u32 TcmId, u32 *RpuId)
{
	XStatus Status = XST_FAILURE;
	/* Map each TCM bank group to its owning RPU instance. */
	switch (TcmId)
	{
	/*
	 * Every core owns three banks, distinguished by the trailing A/B/C in
	 * the node name; the digit before it selects the core within the
	 * cluster.  That is why three labels share one target in each group.
	 */
	case PM_DEV_TCM_A_0A:
	case PM_DEV_TCM_A_0B:
	case PM_DEV_TCM_A_0C:
		*RpuId = PM_DEV_RPU_A_0;
		Status = XST_SUCCESS;
		break;
	case PM_DEV_TCM_A_1A:
	case PM_DEV_TCM_A_1B:
	case PM_DEV_TCM_A_1C:
		*RpuId = PM_DEV_RPU_A_1;
		Status = XST_SUCCESS;
		break;
	/* Select one of the B-cluster RPU cores. */
	case PM_DEV_TCM_B_0A:
	case PM_DEV_TCM_B_0B:
	case PM_DEV_TCM_B_0C:
		*RpuId = PM_DEV_RPU_B_0;
		Status = XST_SUCCESS;
		break;
	case PM_DEV_TCM_B_1A:
	case PM_DEV_TCM_B_1B:
	case PM_DEV_TCM_B_1C:
		*RpuId = PM_DEV_RPU_B_1;
		Status = XST_SUCCESS;
		break;
	/* Select one of the C-cluster RPU cores. */
	case PM_DEV_TCM_C_0A:
	case PM_DEV_TCM_C_0B:
	case PM_DEV_TCM_C_0C:
		*RpuId = PM_DEV_RPU_C_0;
		Status = XST_SUCCESS;
		break;
	case PM_DEV_TCM_C_1A:
	case PM_DEV_TCM_C_1B:
	case PM_DEV_TCM_C_1C:
		*RpuId = PM_DEV_RPU_C_1;
		Status = XST_SUCCESS;
		break;
	/* Select one of the D-cluster RPU cores. */
	case PM_DEV_TCM_D_0A:
	case PM_DEV_TCM_D_0B:
	case PM_DEV_TCM_D_0C:
		*RpuId = PM_DEV_RPU_D_0;
		Status = XST_SUCCESS;
		break;
	case PM_DEV_TCM_D_1A:
	case PM_DEV_TCM_D_1B:
	case PM_DEV_TCM_D_1C:
		*RpuId = PM_DEV_RPU_D_1;
		Status = XST_SUCCESS;
		break;
	/* Select one of the E-cluster RPU cores. */
	case PM_DEV_TCM_E_0A:
	case PM_DEV_TCM_E_0B:
	case PM_DEV_TCM_E_0C:
		*RpuId = PM_DEV_RPU_E_0;
		Status = XST_SUCCESS;
		break;
	case PM_DEV_TCM_E_1A:
	case PM_DEV_TCM_E_1B:
	case PM_DEV_TCM_E_1C:
		*RpuId = PM_DEV_RPU_E_1;
		Status = XST_SUCCESS;
		break;
	default:
		/* Reject IDs that do not identify a TCM bank. */
		PmErr("Invalid TCM ID: 0x%x\n", TcmId);
		Status = XST_INVALID_PARAM;
		break;
	}
	return Status;
}
