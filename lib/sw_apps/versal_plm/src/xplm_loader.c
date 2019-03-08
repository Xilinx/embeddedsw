/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
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
* @file xplm_loader.c
*
* This file contains the wrapper functions for platfrom loader
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/20/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_loader.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XilPdi PdiInstance;

/*****************************************************************************/
/**
* It initializes the loader structures and registers  the CDO loader
* commands
*
* @param	None
* @return	None
*
*****************************************************************************/
int XPlm_LoaderInit()
{
	int Status;

	Status = XLoader_Init();

	return Status;
}

/*****************************************************************************/
/**
* It loads the boot PDI
*
* @param	None
* @return	None
*
*****************************************************************************/
int XPlm_LoadBootPdi(void *arg)
{
	int Status;
	u32 BootMode;

	/**
	 * 1. Read Boot mode register and multiboot offset register
	 * 2. Load subsystem present in PDI
	 */
	XilPdi* PdiPtr = &PdiInstance;
	BootMode = XLoader_GetBootMode();

	/**
	 * In case of JTAG boot mode and jtag mode is not SBI,
	 * no PDI loading is present
	 */
	if ((BootMode == XLOADER_PDI_SRC_JTAG) &&
	    (!XLoader_IsJtagSbiMode()))
	{
		Status = XST_SUCCESS;
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "PDI Load: Started\n\r");

	Status = XLoader_PdiInit(PdiPtr, BootMode, 0U);
	if (Status != XST_SUCCESS)
	{
		goto END;
	}

    Status = XLoader_LoadAndStartSubSystemPdi(PdiPtr);
	if (Status != XST_SUCCESS)
	{
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "PDI Load: Done\n\r");
END:
	return Status;
}
