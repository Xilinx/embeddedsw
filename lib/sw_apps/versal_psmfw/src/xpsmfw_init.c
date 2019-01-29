/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xpsmfw_init.c
*
* This file contains PSM Firmware initialization functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#include "xpsmfw_init.h"
#include "xpsmfw_default.h"

int XPsmFw_Init()
{
	int Status = XST_SUCCESS;
	
#ifdef XPAR_PSU_IPI_PSM_DEVICE_ID
	XPsmfw_IpiManagerInit();
	
	/* FIXME: Clear IPI0 status and enable IPI interrupts. Do it else where*/
	XPsmFw_Write32(IPI_PSM_ISR, MASK32_ALL_HIGH);
#endif
	return Status;
}

int XPsmFw_FPD_MBISR()
{
	int Status = XST_FAILURE;
	
	XPsmFw_UtilRMW(PSM_LOCAL_MBISR_CNTRL, PSM_LOCAL_MBISR_ENABLE_FPD, (~PSM_LOCAL_MBISR_ENABLE_FPD));
	XPsmFw_UtilRMW(PSM_LOCAL_MBISR_CNTRL, PSM_LOCAL_MBISR_TRG_FPD, PSM_LOCAL_MBISR_TRG_FPD);
	XPsmFw_UtilRMW(PSM_LOCAL_MBISR_CNTRL, PSM_LOCAL_MBISR_ENABLE_FPD, PSM_LOCAL_MBISR_ENABLE_FPD);

	Status = XPsmFw_UtilPollForMask(PSM_LOCAL_MBISR_STATUS, PSM_LOCAL_MBISR_DONE_STATUS, 0x10000U);
	if (Status != XST_SUCCESS ) {
		goto END;
	}
	
	Status = XPsmFw_UtilPollForMask(PSM_LOCAL_MBISR_STATUS, PSM_LOCAL_MBISR_PASS_STATUS, 0x10000U);
	if (Status != XST_SUCCESS ) {
		goto END;
	}

END:
	return Status;
}

int XPsmFw_ScanClear(u32 ScanClearAddr)
{
	int Status = XST_FAILURE;
	
	/* Trigger scan clear */
	XPsmFw_UtilRMW(ScanClearAddr, PSM_LOCAL_SCAN_CLEAR_TRIGGER, PSM_LOCAL_SCAN_CLEAR_TRIGGER);
	
	Status = XPsmFw_UtilPollForMask(ScanClearAddr, PSM_LOCAL_SCAN_CLEAR_DONE_STATUS, 0x10000U);
	if ( Status != XST_SUCCESS ) {
		goto END;
	}
	
	Status = XPsmFw_UtilPollForMask(ScanClearAddr, PSM_LOCAL_SCAN_CLEAR_PASS_STATUS, 0x10000U);
	if ( Status != XST_SUCCESS ) {
		goto END;
	}

END:
	return Status;
}
