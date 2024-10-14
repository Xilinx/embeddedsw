/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/
#include "xpsmfw_stl.h"
#include "xpsmfw_ipi_manager.h"

#ifdef PSM_ENABLE_STL
#include "xstl_psminterface.h"
#include "xpsmfw_iomodule.h"
#include "xpsmfw_power.h"
static void XPsmfw_NotifyStlErrEvent(u32 StlId);
volatile u32 PeriodicStlTrigger;
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
	PeriodicStlTrigger = 0U;
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

	/* Execute Periodic STLs, If any failure reported by the STL, the return
	 * value contains the STL ID, else it will be XST_SUCCESS
	 */
	Status = XStl_PsmPeriodicTask();
	if (XST_SUCCESS != Status) {
		goto END;
	}
	/* Disable interrupts */
	XIOModule_Disable(&IOModule, PSM_IOMODULE_IRQ_STATUS_WAKE_UP_REQ_SHIFT);
	XIOModule_Disable(&IOModule, PSM_IOMODULE_IRQ_STATUS_GICP_INT_SHIFT);

	/* Execute STL interrupt injection */
	Status = XStl_PsmIntrInjHook();

	/* Enable interrupts */
	XIOModule_Enable(&IOModule, PSM_IOMODULE_IRQ_STATUS_WAKE_UP_REQ_SHIFT);
	XIOModule_Enable(&IOModule, PSM_IOMODULE_IRQ_STATUS_GICP_INT_SHIFT);
END:
	if (XST_SUCCESS != Status) {
		/* Get STL ID from the status */
		StlIdx = ((u32)Status >> 4U);
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
	PsmToPlmEvent.EventInfo.StlEvent = 1U;
	/* Update STL ID in RTCA and trigger IPI to PLM */
	PsmToPlmEvent.StlId = StlId;
	return XPsmFw_IpiTrigger(IPI_PSM_IER_PMC_MASK);
}
#endif /* PSM_ENABLE_STL */
