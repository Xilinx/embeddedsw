/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/
#include "xpsmfw_stl.h"
#include "xpsmfw_ipi_manager.h"

#ifdef PSM_ENABLE_STL
#include "xstl_psminterface.h"

static void XPsmfw_NotifyStlErrEvent(u32 StlId);

/****************************************************************************/
/**
 * @brief	Hook function to run startup PSM STLs
 *
 * @return	XST_SUCCESS or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_StartUpStlHook(void)
{
	/* Update Version of PSM and run start-up STLs */
	XStl_PsmInit();
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief	Hook function to run PSM STLs periodically
 *
 * @return	XST_SUCCESS or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_PeriodicStlHook(void)
{
	XStatus Status = XST_FAILURE;
	u32 StlIdx = 0U;

	/* Execute Periodic STLs */
	Status = XStl_PsmPeriodicTask();
	if (XST_SUCCESS != Status) {
		/* Get STL ID from the status */
		StlIdx = (Status >> 4U);
		XPsmfw_NotifyStlErrEvent(StlIdx);
		Status = XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************/
/**
*
* This function reports failure to PLM
*
* @param	StlId to specify which STL got failure
*
* @return	. Status:XST_SUCCESS upon successful completion of error reporting
*                   XST_FAILURE if any failure detected during error reporting
*
*******************************************************************************/
static void XPsmfw_NotifyStlErrEvent(u32 StlId)
{
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};

	/* Frame the IPI payload buffer */
	Payload[0] = XSTL_HEADER((u32)1U, XSTL_ERR_NOTIFY_CMD);
	Payload[1] = StlId;

	(void)XPsmFw_IpiSend(IPI_PSM_IER_PMC_MASK, Payload);
}
#endif /* PSM_ENABLE_STL */
