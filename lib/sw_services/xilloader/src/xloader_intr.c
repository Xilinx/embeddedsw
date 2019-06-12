/******************************************************************************
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
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
 * This function initializes the loader instance and registers loader
 * commands with PLM
 *
 * @param None
 *
 * @return	returns XST_SUCCESS on success
 *
 *****************************************************************************/
int XLoader_IntrInit()
{
	/**
	 * Register the SBI RDY interrupt to enable the PDI loading from
	 * SBI interface.
	 * TODO
	 * When we enable SMAP_ABORT or any errors, then we need checks for
	 * SBI DATA RDY mask before loading the PDI.
	 */
	XPlmi_RegisterHandler(XPLMI_SBI_DATA_RDY, XLoader_SbiLoadPdi, (void *)0);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief This function is the interrupt handler for SBI data ready.
 * In this handler, PDI is loadeed through SBI interface.
 * SBI interface setting for JTAG/SMAP/AXI/HSDP should be set before
 * this handler
 *
 * @param Data Not used as of now, present as a part of general interrupt
 *             handler definition.
 *
 * @return Status of LoadPdi
 *****************************************************************************/
int XLoader_SbiLoadPdi(void *Data)
{
	int Status;
	u32 PdiSrc;
	u64 PdiAddr;
	XilPdi* PdiPtr = &SubsystemPdiIns;

	(void ) Data;

	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	/**
	 * Disable the SBI RDY interrupt so that PDI load does not
	 * interrupt itself
	 */
	XPlmi_PlmIntrDisable(XPLMI_SBI_DATA_RDY);

	/** store the command fields in resume data */
	PdiSrc = XLOADER_PDI_SRC_SBI;
	PdiAddr = 0U;

	XPlmi_Printf(DEBUG_INFO, "SBI PDI Load: Started\n\r");

	PdiPtr->PdiType = XLOADER_PDI_TYPE_PARTIAL;
	Status = XLoader_LoadPdi(PdiPtr, PdiSrc, PdiAddr);
	if (Status != XST_SUCCESS)
	{
		/* Update the error code */
		XPlmi_ErrMgr(Status);
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "SBI PDI Load: Done\n\r");
END:

	XLoader_ClearIntrSbiDataRdy();

	/**
	 * Enable the SBI RDY interrupt to get the next PDI
	 */
	XPlmi_PlmIntrEnable(XPLMI_SBI_DATA_RDY);

	return Status;
}

/*****************************************************************************/
/**
 * @brief This function clears the previous SBI data ready
 * and enables IRQ for next interrupt
 *
 * @param None
 *
 * @return None
 *****************************************************************************/
void XLoader_ClearIntrSbiDataRdy()
{
	/** Clear the SBI interrupt */
	XPlmi_UtilRMW(SLAVE_BOOT_SBI_IRQ_STATUS,
		     SLAVE_BOOT_SBI_IRQ_STATUS_DATA_RDY_MASK,
		     SLAVE_BOOT_SBI_IRQ_STATUS_DATA_RDY_MASK);
	XPlmi_UtilRMW(SLAVE_BOOT_SBI_IRQ_ENABLE,
		     SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK,
		     SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK);

}

