/******************************************************************************
* Copyright (C) 2018 Xilinx, Inc. All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
* @file xplm_pm.c
*
* This file contains the wrapper code xilpm
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/20/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_pm.h"
#include "xplm_main.h"
#include "xillibpm_api.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
* It initializes libPM
*
* @param	None
* @return	None
*
*****************************************************************************/
int XPlm_PmInit()
{
	int Status;

	/**
	 * Initialize the libPM component. It registers the callback handlers,
	 * variables, events
	 */
	Status = XPm_Init(NULL, NULL);

	return Status;
}


/*****************************************************************************/
/**
* It initializes PM with
*
* @param	None
* @return	None
*
*****************************************************************************/
int XPlm_ProcessPlmCdo(struct metal_event *event, void *arg)
{
	int Status;
	XPlmi_Printf(DEBUG_DETAILED, "%s\n\r", __func__);

	/**
	 * Pass the device topology and policies sections to PM
	 *  - Pass the PLM CDO to CDO parser
	 *  - PM sets device topology
	 *  - PM sets the polices
	 *
	 * Use XPm_CreateSubsystem() to create the PLM subsystem.
	 *  - PM sets the PLM requirements
	 *  - Subsystem handle is updated by PM
	 *
	 * Register the PM callback for PLM subsystem
	 *	- TODO what can we have in this?
	 */

	/** Process the PLM CDO */
	/**
	 * TODO There is no way to find the PLM CDO length right now,
	 * with out reading the boot header.
	 * Passing the whole RAM length now and will break the loop
	 * when CMD_END is detected
	 */
	XPlmi_ProcessCdo((u32 *)XPLMI_PMCRAM_BASEADDR, XPLMI_PMCRAM_LEN);

#ifdef DEBUG_UART_PS
	XPlm_InitUart();
#endif

	Status = METAL_EVENT_HANDLED;
	return Status;
}
