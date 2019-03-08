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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_err.c
*
* This file contains error management for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  kc   02/12/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_status.h"
#include "xplmi_debug.h"
#include "xplmi_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * This function is called in PLM error cases.
 *
 * @param ErrorStatus is the error code which is written to the
 *		  error status register
 *
 * @return none
 *
 *****************************************************************************/
void XPlmi_ErrMgr(int Status)
{
	/* Print the PMCFW error */
	XPlmi_Printf(DEBUG_GENERAL, "PLM Error Status: 0x%08lx\n\r",
			Status);
	XPlmi_Out32(PMC_GLOBAL_PMC_FW_ERR, Status);
	XPlmi_DumpRegisters();

	while(1);
}

/*****************************************************************************/
/**
 * This function dumps the registers which can help debugging
 *
 * @param
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XPlmi_DumpRegisters()
{

	XPlmi_Printf(DEBUG_GENERAL, "====Register Dump============\n\r");

	XPlmi_Printf(DEBUG_GENERAL, "IDCODE: 0x%08x\n\r",
		      XPlmi_In32(PMC_TAP_IDCODE));
	XPlmi_Printf(DEBUG_GENERAL, "Version: 0x%08x\n\r",
		      XPlmi_In32(PMC_TAP_VERSION));
	XPlmi_Printf(DEBUG_GENERAL, "Bootmode User: 0x%08x\n\r",
		      XPlmi_In32(CRP_BOOT_MODE_USER));
	XPlmi_Printf(DEBUG_GENERAL, "Bootmode POR: 0x%08x\n\r",
		      XPlmi_In32(CRP_BOOT_MODE_POR));
	XPlmi_Printf(DEBUG_GENERAL, "Reset Reason: 0x%08x\n\r",
		      XPlmi_In32(CRP_RESET_REASON));
	XPlmi_Printf(DEBUG_GENERAL, "Multiboot: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT));
	XPlmi_Printf(DEBUG_GENERAL, "PMC PWR Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_PWR_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC GSW Err: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_PMC_GSW_ERR));
	XPlmi_Printf(DEBUG_GENERAL, "PMC FW Error: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_PMC_FW_ERR));
	XPlmi_Printf(DEBUG_GENERAL, "PMC ERR OUT1 Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC ERR OUT2 Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP0 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP0_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP1 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP1_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP2 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP2_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP3 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP3_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP4 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP4_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP5 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP5_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP6 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP6_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP7 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP7_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICPPMC IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS));

	XPlmi_Printf(DEBUG_GENERAL, "====Register Dump============\n\r");
}
