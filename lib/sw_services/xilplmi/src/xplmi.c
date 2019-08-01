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
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi.c
*
* This file contains the PLMI module register functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/07/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi.h"
#include "xplmi_err.h"
#include "xplmi_sysmon.h"

/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/
typedef int (*XPlmiInit)(void);

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/******************************************************************************/
/**
* @brief This function  will initialize the PLMI module.
* @param    None
* @return   None.
****************************************************************************/
int XPlmi_Init(void )
{
	int Status;

	Status = XPlmi_SetUpInterruptSystem();
	if (Status != XST_SUCCESS)
	{
		goto END;
	}

	XPlmi_GenericInit();
END:
	return Status;
}

/**
 * It contains all the PS LPD init functions to be run for every module that
 * is present as a part of PLM.
 */
XPlmiInit LpdInitList[] = {
#ifdef DEBUG_UART_PS
	XPlmi_InitUart,
#endif
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
	XPlmi_IpiInit,
#endif
	XPlmi_SysMonInit,
	XPlmi_PsEmInit,
};

/*****************************************************************************/
/**
 * @brief This function calls all the PS LPD init functions of all the different
 * modules. As a part of init functions, modules can register the
 * command handlers, interrupt handlers with the interface layer.
 *
 * @param	None
 *
 * @return	Status as defined in xplm_status.h
 *
 *****************************************************************************/
void XPlmi_LpdInit(void)
{
	u32 Index;
	int Status;

	for (Index = 0; Index <
	     sizeof(LpdInitList) / sizeof(*LpdInitList); Index++) {
		Status = LpdInitList[Index]();
		if (Status != XST_SUCCESS) {
			Status = XPLMI_UPDATE_STATUS(XPLM_ERR_LPD_MOD, Status);
			break;
		}
	}
	
	if (XST_SUCCESS == Status) {
		LpdInitialized |= LPD_INITIALIZED;
	}
}
