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
* @file xplm_sem_init.c
*
* This file contains the startup tasks related code for PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  rm   09/22/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_sem_init.h"
#include "xplmi_scheduler.h"
#include "xilsem.h"
#include "xplm_default.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief This function call all the init functions of all xilsem 
 * modules. As a part of init functions, modules can initiate scan on both cfi 
 * npi, scan will be decided based on the CIPS params in xparameters.h.
 *
 * @param	None
 *
 * @return	Status as SUCCESS or FAILURE
 *
 *****************************************************************************/
int XSem_Init()
{
	int Status = XST_FAILURE;

#if !defined(XSEM_CFRSCAN_EN) && !defined(XSEM_NPISCAN_EN)
	Status = XST_SUCCESS;
#endif

#ifdef XSEM_CFRSCAN_EN
	Status = XSem_CfrInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

#ifdef XSEM_NPISCAN_EN
	Status = XSem_NpiInit();
	if (Status != XST_SUCCESS) {
		goto END;
	} else {
		Status = XPlmi_SchedulerAddTask(0x0U, XSem_NpiRunScan, 100U);
	}
END:

#endif
	return Status;
}

