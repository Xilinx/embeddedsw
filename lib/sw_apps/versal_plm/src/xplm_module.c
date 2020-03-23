/******************************************************************************
* Copyright (C) 2018-2020 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_module.c
*
* This file contains the module initialization code for PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  kc   08/28/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_module.h"
#include "xplm_default.h"
#include "xplmi_sysmon.h"
#include "xpm_api.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef int (*ModuleInit)(void);

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XPlm_PlmiInit(void);
static int XPlm_ErrInit(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * It contains the all the init functions to be run for every module that
 * is present as a part of PLM.
 */
static const ModuleInit ModuleList[] =
{
	XPlm_PlmiInit,
	XPlm_ErrInit,
	XPlm_PmInit,
	XPlm_LoaderInit,
};

/*****************************************************************************/
/**
 * @brief This function initializes the Error module and registers the
 * error commands of the CDO.
 *
 * @param	None
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
static int XPlm_ErrInit(void)
{
	int Status = XST_FAILURE;

	XPlmi_EmInit(XPm_SystemShutdown);
	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief This function initializes the PLMI module and registers the
 * general commands of the CDO.
 *
 * @param	None
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
static int XPlm_PlmiInit(void)
{
	int Status = XST_FAILURE;
	Status = XPlmi_Init();
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function call all the init functions of all the different
 * modules. As a part of init functions, modules can register the
 * command handlers, interrupt handlers with the interface layer.
 *
 * @param	None
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
int XPlm_ModuleInit(void *Arg)
{
	int Status = XST_FAILURE;
	u32 Index;

	(void )Arg;
	for (Index = 0; Index < sizeof ModuleList / sizeof *ModuleList; Index++)
	{
		Status = ModuleList[Index]();
		if (Status != XST_SUCCESS)
		{
			goto END;
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}
