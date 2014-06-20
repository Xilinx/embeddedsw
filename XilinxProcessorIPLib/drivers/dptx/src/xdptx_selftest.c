/*******************************************************************************
 *
 * Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdptx_selftest.c
 *
 * This file contains a diagnostic self-test function for the XDptx driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a als  05/17/14 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdptx.h"
#include "xstatus.h"

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function runs a self-test on the XDptx driver/device. The test attempts
 * to intialize the DisplayPort TX core, train the main link at the highest
 * common capabilities between the core and the sink, and checks the status
 * of the link after training.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *              - XST_SUCCESS if the self-test passed. The main link has been
 *                trained and established successfully.
 *              - XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDptx_SelfTest(XDptx *InstancePtr)
{
        XDptx_Config *ConfigPtr;
        u32 Status;

        /* Verify arguments. */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        /* Obtain the capabilities of the sink by reading the DPCD. */
        Status = XDptx_GetSinkCapabilities(InstancePtr);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        /* Configure the main link attributes. */
        Status = XDptx_CfgMainLinkMax(InstancePtr);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        /* Attempt to establish a link at the maximum common capabilities
         * between the DisplayPort TX core and the sink. */
        XDptx_EstablishLink(InstancePtr);

        /* Return whether or not the link has been successfully trained. */
        Status = XDptx_CheckLinkStatus(InstancePtr,
                                        InstancePtr->LinkConfig.LaneCount);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        return XST_SUCCESS;
}
