/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_hooks.c
*
* This is the file which contains PMCFW hook functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   04/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xpmcfw_hooks.h"
#include "xilcdo.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPMCFW_PMC_CDO_ADDR		(XPMCFW_PMCRAM_BASEADDR)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/*****************************************************************************/
/**
 * This is a hook function where user can include the functionality to be run
 * before bitstream download
 *
 * @param none
 *
 * @return error status based on implemented functionality (SUCCESS by default)
 *
 ******************************************************************************/
XStatus XPmcFw_HookBeforeBSDownload(void )
{
	XStatus Status = XPMCFW_SUCCESS;

	/**
	 * Add the code here
	 */


	return Status;
}

/*****************************************************************************/
/**
 * This is a hook function where user can include the functionality to be run
 * after bitstream download
 *
 * @param none
 *
 * @return error status based on implemented functionality (SUCCESS by default)
 *
 ******************************************************************************/
XStatus XPmcFw_HookAfterBSDownload(void )
{
	XStatus Status = XPMCFW_SUCCESS;

	/**
	 * Add the code here
	 */

	return Status;
}

/*****************************************************************************/
/**
 * This is a hook function where user can include the functionality to be run
 * before starting the subsystems
 *
 * @param none
 *
 * @return error status based on implemented functionality (SUCCESS by default)
 *
 ******************************************************************************/
XStatus XPmcFw_HookBeforeHandoff(void )
{
	XStatus Status = XPMCFW_SUCCESS;

	/**
	 * Add the code here
	 */

	return Status;
}

/*****************************************************************************/
/**
 * This is a hook function where user can include the functionality to be run
 * before PMCFW fallback happens
 *
 * @param none
 *
 * @return error status based on implemented functionality (SUCCESS by default)
 *
 ******************************************************************************/
XStatus XPmcFw_HookBeforeFallback(void)
{
	XStatus Status = XPMCFW_SUCCESS;

	/**
	 * Add the code here
	 */

	return Status;
}

/*****************************************************************************/
/**
 * This function facilitates users to define different variants of psu_init()
 * functions based on different configurations in Vivado. The default call to
 * psu_init() can then be swapped with the alternate variant based on the
 * requirement.
 *
 * @param none
 *
 * @return error status based on implemented functionality (SUCCESS by default)
 *
  *****************************************************************************/

XStatus XPmcFw_HookPmcPsuInit(void)
{
	XStatus Status;

	/* Add the code here */
	Status = XilCdo_ProcessCdo((u32 *)XPMCFW_PMC_CDO_ADDR);

	if (XPMCFW_SUCCESS != Status) {
		XPmcFw_Printf(DEBUG_GENERAL, "XPMCFW_ERR_PMC_CDO_INIT\n\r");
		/**
		 * Need to check a way to communicate both PMCFW code
		 * and PSU init error code
		 */
		Status = XPMCFW_UPDATE_ERR(XPMCFW_ERR_PMC_CDO_INIT, Status);
		goto END;
	}

END:
	return Status;
}
