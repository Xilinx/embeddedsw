/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_intr.c
*
* This file contains the code related to the interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/25/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xloader.h"
#include "xplmi_proc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief	This function initializes the loader instance and registers loader
 * commands with PLM.
 *
 * @param	None
 *
 * @return	Returns XST_SUCCESS
 *
 *****************************************************************************/
int XLoader_IntrInit()
{
	int Status = XST_FAILURE;

	/*
	 * Register the SBI RDY interrupt to enable the PDI loading from
	 * SBI interface.
	 * TODO
	 * When we enable SMAP_ABORT or any errors, then we need checks for
	 * SBI DATA RDY mask before loading the PDI.
	 */
	XPlmi_RegisterHandler(XPLMI_SBI_DATA_RDY, XLoader_SbiLoadPdi, (void *)0U);
	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is the interrupt handler for SBI data ready.
 * In this handler, PDI is loadeed through SBI interface.
 * SBI interface setting for JTAG/SMAP/AXI/HSDP should be set before
 * this handler.
 *
 * @param	Data Not used as of now, present as a part of general interrupt
 *             handler definition.
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_SbiLoadPdi(void *Data)
{
	int Status = XST_FAILURE;
	u32 PdiSrc;
	u64 PdiAddr;
	u32 RegVal;
	XilPdi* PdiPtr = &SubsystemPdiIns;
	(void)Data;

	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	/*
	 * Disable the SBI RDY interrupt so that PDI load does not
	 * interrupt itself
	 */
	XPlmi_PlmIntrDisable(XPLMI_SBI_DATA_RDY);

	/* Store the command fields in resume data */
	RegVal = XPlmi_In32(SLAVE_BOOT_SBI_CTRL) &
			SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK;
	if (RegVal == 0U) {
		PdiSrc = XLOADER_PDI_SRC_SMAP;
	}
	else {
		PdiSrc = XLOADER_PDI_SRC_SBI;
	}

	PdiAddr = 0U;

	XPlmi_Printf(DEBUG_GENERAL, "SBI PDI Load: Started\n\r");
	PdiPtr->PdiType = XLOADER_PDI_TYPE_PARTIAL;
	PdiPtr->IpiMask = 0U;
	Status = XLoader_LoadPdi(PdiPtr, PdiSrc, PdiAddr);
	if (Status != XST_SUCCESS) {
		/* Update the error code */
		XPlmi_ErrMgr(Status);
		goto END;
	}
	XPlmi_Printf(DEBUG_GENERAL, "SBI PDI Load: Done\n\r");

END:
	XLoader_ClearIntrSbiDataRdy();
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function clears the previous SBI data ready
 * and enables IRQ for next interrupt.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XLoader_ClearIntrSbiDataRdy()
{
	/* Clear the SBI interrupt */
	XPlmi_UtilRMW(SLAVE_BOOT_SBI_IRQ_STATUS,
		SLAVE_BOOT_SBI_IRQ_STATUS_DATA_RDY_MASK,
		SLAVE_BOOT_SBI_IRQ_STATUS_DATA_RDY_MASK);
	XPlmi_UtilRMW(SLAVE_BOOT_SBI_IRQ_ENABLE,
		SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK,
		SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK);

	/* Clear and Enable GIC interrupt */
	XPlmi_PlmIntrClear(XPLMI_SBI_DATA_RDY);
	XPlmi_PlmIntrEnable(XPLMI_SBI_DATA_RDY);
}
