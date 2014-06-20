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
 * @file xdptx_intr_example.c
 *
 * Contains a design example using the XDptx driver with interrupts. Upon hot-
 * plug-detect (DisplayPort cable is plugged/unplugged or the monitor is turned
 * on/off), the main link will be trained.
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
#include "xil_printf.h"
#include "xparameters.h"
#include "xstatus.h"
#if defined(__MICROBLAZE__)
#include "xintc.h"
#elif defined(__arm__)
#include "xscugic.h"
#else
#error "Unknown processor type."
#endif

/**************************** Constant Definitions ****************************/

#define DPTX_DEVICE_ID XPAR_DISPLAYPORT_0_DEVICE_ID
#if defined(__MICROBLAZE__)
#define DP_INTERRUPT_ID         XPAR_AXI_INTC_1_DISPLAYPORT_0_AXI_INT_INTR
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#elif defined(__arm__)
#define DP_INTERRUPT_ID         XPAR_FABRIC_DISPLAYPORT_0_AXI_INT_INTR
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif

/**************************** Function Prototypes *****************************/

static u32 Dptx_SetupInterruptHandler(XDptx *InstancePtr,
                                                void *IntrHandler, u32 IntrId);
static void Dptx_InterruptHandler(XDptx *InstancePtr);
static void Dptx_HpdEventHandler(void *InstancePtr);
static void Dptx_HpdPulseHandler(void *InstancePtr);

/**************************** Variable Definitions ****************************/

#if defined(__MICROBLAZE__)
static XIntc IntcInstance;
#elif defined(__arm__)
static XScuGic IntcInstance;
#endif

/**************************** Function Definitions ****************************/

int main(void)
{
        u32 Status;

        /* Do platform initialization here. This is hardware system specific -
         * it is up to the user to implement this function. */
        Dptx_PlatformInit();
        /******************/

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

        /* Setup interrupt handling in the system. */
        Status = Dptx_SetupInterruptHandler(&DptxInstance,
                                &Dptx_InterruptHandler, DP_INTERRUPT_ID);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        /* Do not return in order to allow interrupt handling to run. */
        while (1);
        return XST_SUCCESS;
}

static u32 Dptx_SetupInterruptHandler(XDptx *InstancePtr,
                                                void *IntrHandler, u32 IntrId)
{
        u32 Status;
#if defined(__arm__)
        XScuGic_Config *IntcConfig;
#endif

        /* Set the HPD interrupt handlers. */
        XDptx_SetHpdEventHandler(InstancePtr, &Dptx_HpdEventHandler,
                                                                InstancePtr);
        XDptx_SetHpdPulseHandler(InstancePtr, &Dptx_HpdPulseHandler,
                                                                InstancePtr);

        /* Initialize interrupt controller driver. */
#if defined(__MICROBLAZE__)
        Status = XIntc_Initialize(&IntcInstance, INTC_DEVICE_ID);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
#elif defined(__arm__)
        IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
        Status = XScuGic_CfgInitialize(&IntcInstance, IntcConfig,
                                                IntcConfig->CpuBaseAddress);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XScuGic_SetPriorityTriggerType(&IntcInstance, IntrId, 0xA0, 0x1);
#endif

        /* Connect the device driver handler that will be called when an
         * interrupt for the device occurs, the handler defined above performs
         * the specific interrupt processing for the device. */
#if defined(__MICROBLAZE__)
        Status = XIntc_Connect(&IntcInstance, IntrId,
                                (XInterruptHandler)IntrHandler, InstancePtr);
#elif defined(__arm__)
        Status = XScuGic_Connect(&IntcInstance, IntrId,
                                (Xil_InterruptHandler)IntrHandler, InstancePtr);
#endif
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        /* Start the interrupt controller. */
#if defined(__MICROBLAZE__)
        Status = XIntc_Start(&IntcInstance, XIN_REAL_MODE);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XIntc_Enable(&IntcInstance, IntrId);
#elif defined(__arm__)
        XScuGic_Enable(&IntcInstance, IntrId);
#endif

        /* Initialize the exception table. */
        Xil_ExceptionInit();

        /* Register the interrupt controller handler with the exception table. */
#if defined(__MICROBLAZE__)
        Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                (Xil_ExceptionHandler)XIntc_InterruptHandler, &IntcInstance);
#elif defined(__arm__)
        Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
                (Xil_ExceptionHandler)XScuGic_InterruptHandler, &IntcInstance);
#endif

        /* Enable exceptions. */
        Xil_ExceptionEnable();

#if defined(__MICROBLAZE__)
        /* Enable interrupts in the MicroBlaze processor. */
        microblaze_enable_interrupts();
#endif

        return XST_SUCCESS;
}

static void Dptx_InterruptHandler(XDptx *InstancePtr)
{
        XDptx_HpdInterruptHandler(InstancePtr);
}

static void Dptx_HpdEventHandler(void *InstancePtr)
{
        XDptx *XDptx_InstancePtr = (XDptx *)InstancePtr;

        if (XDptx_IsConnected(XDptx_InstancePtr)) {
                xil_printf("+===> HPD connection event detected.\n");
                Dptx_Run(XDptx_InstancePtr, USE_LANE_COUNT, USE_LINK_RATE);
        }
        else {
                xil_printf("+===> HPD disconnection event detected.\n\n");
        }
}

static void Dptx_HpdPulseHandler(void *InstancePtr)
{
        XDptx *XDptx_InstancePtr = (XDptx *)InstancePtr;

        xil_printf("===> HPD pulse detected.\n");

        Dptx_Run(XDptx_InstancePtr, USE_LANE_COUNT, USE_LINK_RATE);
}
