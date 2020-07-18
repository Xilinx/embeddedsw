/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_gic.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ---- ---- -------- ------------------------------
* 1.00	av	19/02/2020	Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#include "xpsmfw_default.h"
#include "xpsmfw_gic.h"
#include "xpsmfw_dvsec_common.h"
#include "psm_global.h"

#define CHECK_BIT(reg, mask)	(((reg) & (mask)) == (mask))

static struct GicP2HandlerTable_t GicHandlerTable[] = {
	{PSM_GLOBAL_GICP2_IRQ_STATUS_CPM_CORR_ERR_MASK, XPsmFw_DvsecWrite},
	{PSM_GLOBAL_GICP2_IRQ_STATUS_CPM_MISC_MASK, XPsmFw_DvsecRead},
};

/**
 * XPsmFw_DispatchGicP2Handler() - GIC Proxy 2 interrupt handler
 *
 * @GICP2Status   GICP2 status register value
 * @GICP2IntMask  GICP2 interrupt mask register value
 *
 * @return         XST_SUCCESS or error code
 */
XStatus XPsmFw_DispatchGicP2Handler(u32 GicP2Status, u32 GicP2IntMask)
{
	XStatus Status = XST_SUCCESS;
	u32 Idx;

	for (Idx = 0U; Idx < ARRAYSIZE(GicHandlerTable); Idx++) {
		if ((CHECK_BIT(GicP2Status, GicHandlerTable[Idx].Mask) != 0) &&
		     (CHECK_BIT(GicP2IntMask, GicHandlerTable[Idx].Mask) == 0)) {
			/* Call gic handler */
			Status = GicHandlerTable[Idx].Handler();
		}

		/* Ack the service */
		XPsmFw_Write32(PSM_GLOBAL_GICP2_IRQ_STATUS,
			       GicHandlerTable[Idx].Mask);
		XPsmFw_Write32(PSM_GLOBAL_GICP_PSM_IRQ_STATUS,
			       PSM_GLOBAL_GICP_GICP2_MASK);
	}

	return Status;
}

/**
 * XPsmFw_GicIrqEnable() - Enables GICP2 PSM Irq's
 */
void XPsmFw_GicP2IrqEnable(void)
{
	u32 IntMask = PSM_GLOBAL_GICP2_IRQ_STATUS_CPM_CORR_ERR_MASK |
		      PSM_GLOBAL_GICP2_IRQ_STATUS_CPM_MISC_MASK     |
		      PSM_GLOBAL_GICP2_IRQ_STATUS_PL_MASK;

	XPsmFw_Write32(PSM_GLOBAL_GICP2_INT_EN, IntMask);

	/* Enable GIC PSM irq */
	XPsmFw_Write32(PSM_GLOBAL_GICP_PSM_IRQ_EN, PSM_GLOBAL_GICP_GICP2_MASK);
}
