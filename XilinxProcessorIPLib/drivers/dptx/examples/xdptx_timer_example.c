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
 * @file xdptx_timer_example.c
 *
 * Contains a design example using the XDptx driver with a user-defined hook
 * for delay. The reasoning behind this is that MicroBlaze sleep is not very
 * accurate without a hardware timer. For systems that have a hardware timer,
 * the user may override the default MicroBlaze sleep with a function that will
 * use the hardware timer.
 *
 * @note        For this example to display output, the user will need to
 *              implement initialization of the system (Dptx_PlatformInit) and,
 *              after training is complete, implement configuration of the video
 *              stream source in order to provide the DisplayPort core with
 *              input (Dptx_ConfigureVidgen - called in xdptx_example_common.c).
 *              See XAPP1178 for reference.
 * @note        The functions Dptx_PlatformInit and Dptx_ConfigureVidgen are
 *              declared extern in xdptx_example_common.h and are left up to the
 *              user to implement.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a als  06/17/14 Initial creation.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdptx.h"
#include "xdptx_example_common.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xtmrctr.h"

/**************************** Constant Definitions ****************************/

#define DPTX_DEVICE_ID XPAR_DISPLAYPORT_0_DEVICE_ID

/**************************** Function Prototypes *****************************/

static void Dptx_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);

/*************************** Variable Declarations ****************************/

XTmrCtr TimerCounterInst;

/**************************** Function Definitions ****************************/

int main(void)
{
        u32 Status;

        /* Do platform initialization here. This is hardware system specific -
         * it is up to the user to implement this function. */
        Dptx_PlatformInit();
        /*******************/

        /* Set a custom timer handler for improved delay accuracy.
         * Note: This only has an affect for MicroBlaze systems. */
        XDptx_SetUserTimerHandler(&DptxInstance, &Dptx_CustomWaitUs,
                                                        &TimerCounterInst);

        Status = Dptx_SetupExample(&DptxInstance, DPTX_DEVICE_ID);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

#if defined(TRAIN_ADAPTIVE)
        XDptx_EnableTrainAdaptive(&DptxInstance, 1);
#else
        XDptx_EnableTrainAdaptive(&DptxInstance, 0);
#endif
#if defined(TRAIN_HAS_REDRIVER)
        XDptx_SetHasRedriverInPath(&DptxInstance, 1);
#else
        XDptx_SetHasRedriverInPath(&DptxInstance, 0);
#endif

        /* A receiver must be connected at this point. */
        Dptx_Run(&DptxInstance, USE_LANE_COUNT, USE_LINK_RATE);

        /* Do not return in order to keep the program running. */
        while (1);
        return XST_SUCCESS;
}

static void Dptx_CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
{
        XDptx *XDptx_InstancePtr = (XDptx *)InstancePtr;
        u32 TimerVal;

        XTmrCtr_Start(XDptx_InstancePtr->UserTimerPtr, 0);

        /* Wait specified number of useconds. */
        do {
                TimerVal = XTmrCtr_GetValue(XDptx_InstancePtr->UserTimerPtr, 0);
        }
        while (TimerVal < (MicroSeconds *
                        (XDptx_InstancePtr->TxConfig.SAxiClkHz / 1000000)));

        XTmrCtr_Stop(XDptx_InstancePtr->UserTimerPtr, 0);
}
