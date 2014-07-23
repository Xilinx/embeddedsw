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
 * @file xdptx_intr_timer_example.c
 *
 * Contains a design example using the XDptx driver with interrupts and. Upon Hot-
 * Plug-Detect (HPD - DisplayPort cable is plugged/unplugged or the monitor is
 * turned on/off), the main link will be trained.
 *
 * @note        This example requires an interrupt controller connected to the
 *              processor and the DisplayPort TX core in the system.
 * @note        This example requires an AXI timer in the system.
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
#include "xtmrctr.h"
#ifdef XPAR_INTC_0_DEVICE_ID
/* For MicroBlaze systems. */
#include "xintc.h"
#else
/* For ARM/Zynq SoC systems. */
#include "xscugic.h"
#endif /* XPAR_INTC_0_DEVICE_ID */

/**************************** Constant Definitions ****************************/

/* The following constants map to the XPAR parameters created in the
 * xparameters.h file. */
#ifdef XPAR_INTC_0_DEVICE_ID
#define DP_INTERRUPT_ID         XPAR_AXI_INTC_1_DISPLAYPORT_0_AXI_INT_INTR
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#else
#define DP_INTERRUPT_ID         XPAR_FABRIC_DISPLAYPORT_0_AXI_INT_INTR
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif /* XPAR_INTC_0_DEVICE_ID */

/****************************** Type Definitions ******************************/

/* Depending on whether the system is a MicroBlaze or ARM/Zynq SoC system,
 * different drivers and associated types will be used. */
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC            XIntc
#define INTC_HANDLER    XIntc_InterruptHandler
#else
#define INTC            XScuGic
#define INTC_HANDLER    XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */

/**************************** Function Prototypes *****************************/

static u32 Dptx_SetupInterruptHandler(XDptx *InstancePtr);
static void Dptx_HpdEventHandler(void *InstancePtr);
static void Dptx_HpdPulseHandler(void *InstancePtr);

static void Dptx_CustomWaitUs(void *InstancePtr, u32 MicroSeconds);

/**************************** Variable Definitions ****************************/

INTC IntcInstance;              /* The interrupt controller instance. */
XTmrCtr TimerCounterInst;       /* The timer counter instance. */

/**************************** Function Definitions ****************************/

int main(void)
{
        u32 Status;

        /* Do platform initialization here. This is hardware system specific -
         * it is up to the user to implement this function. */
        Dptx_PlatformInit();
        /******************/

        /* Set a custom timer handler for improved delay accuracy on MicroBlaze
         * systems since the driver does not assume/have a dependency on the
         * system having a timer in the FPGA.
         * Note: This only has an affect for MicroBlaze systems since the Zynq
         * SoC contains a timer, which is used when the driver calls the sleep
         * function. */
        XDptx_SetUserTimerHandler(&DptxInstance, &Dptx_CustomWaitUs,
                                                        &TimerCounterInst);

        Status = Dptx_SetupExample(&DptxInstance, DPTX_DEVICE_ID);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        XDptx_EnableTrainAdaptive(&DptxInstance, TRAIN_ADAPTIVE);
        XDptx_SetHasRedriverInPath(&DptxInstance, TRAIN_HAS_REDRIVER);

        /* Setup interrupt handling in the system. */
        Status = Dptx_SetupInterruptHandler(&DptxInstance);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        /* Do not return in order to allow interrupt handling to run. */
        while (1);
        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets up the interrupt system such that interrupts caused by
 * Hot-Plug-Detect (HPD) events and pulses are handled. This function is
 * application-specific for systems that have an interrupt controller connected
 * to the processor. The user should modify this function to fit the
 * application.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return      - XST_SUCCESS if the interrupt system was successfully set up.
 *              - XST_FAILURE otherwise.
 *
 * @note        An interrupt controller must be present in the system, connected
 *              to the processor and the DisplayPort TX core.
 *
*******************************************************************************/
static u32 Dptx_SetupInterruptHandler(XDptx *InstancePtr)
{
        u32 Status;

        /* Set the HPD interrupt handlers. */
        XDptx_SetHpdEventHandler(InstancePtr, &Dptx_HpdEventHandler,
                                                                InstancePtr);
        XDptx_SetHpdPulseHandler(InstancePtr, &Dptx_HpdPulseHandler,
                                                                InstancePtr);

        /* Initialize interrupt controller driver. */
#ifdef XPAR_INTC_0_DEVICE_ID
        Status = XIntc_Initialize(&IntcInstance, INTC_DEVICE_ID);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
#else
        XScuGic_Config *IntcConfig;

        IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
        Status = XScuGic_CfgInitialize(&IntcInstance, IntcConfig,
                                                IntcConfig->CpuBaseAddress);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XScuGic_SetPriorityTriggerType(&IntcInstance, DP_INTERRUPT_ID,
                                                                0xA0, 0x1);
#endif /* XPAR_INTC_0_DEVICE_ID */

        /* Connect the device driver handler that will be called when an
         * interrupt for the device occurs, the handler defined above performs
         * the specific interrupt processing for the device. */
#ifdef XPAR_INTC_0_DEVICE_ID
        Status = XIntc_Connect(&IntcInstance, DP_INTERRUPT_ID,
                (XInterruptHandler)XDptx_HpdInterruptHandler, InstancePtr);
#else
        Status = XScuGic_Connect(&IntcInstance, DP_INTERRUPT_ID,
                (Xil_InterruptHandler)XDptx_HpdInterruptHandler, InstancePtr);
#endif /* XPAR_INTC_0_DEVICE_ID */
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }

        /* Start the interrupt controller. */
#ifdef XPAR_INTC_0_DEVICE_ID
        Status = XIntc_Start(&IntcInstance, XIN_REAL_MODE);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XIntc_Enable(&IntcInstance, DP_INTERRUPT_ID);
#else
        XScuGic_Enable(&IntcInstance, DP_INTERRUPT_ID);
#endif /* XPAR_INTC_0_DEVICE_ID */

        /* Initialize the exception table. */
        Xil_ExceptionInit();

        /* Register the interrupt controller handler with the exception table. */
        Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                        (Xil_ExceptionHandler)INTC_HANDLER, &IntcInstance);

        /* Enable exceptions. */
        Xil_ExceptionEnable();

        return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function is called when a Hot-Plug-Detect (HPD) event is received by the
 * DisplayPort TX core. The XDPTX_INTERRUPT_STATUS_HPD_EVENT_MASK bit of the
 * core's XDPTX_INTERRUPT_STATUS register indicates that an HPD event has
 * occurred.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return      None.
 *
 * @note        Use the XDptx_SetHpdEventHandler driver function to set this
 *              function as the handler for HPD pulses.
 *
*******************************************************************************/
static void Dptx_HpdEventHandler(void *InstancePtr)
{
        XDptx *XDptx_InstancePtr = (XDptx *)InstancePtr;

        if (XDptx_IsConnected(XDptx_InstancePtr)) {
                xil_printf("+===> HPD connection event detected.\n");

                Dptx_Run(XDptx_InstancePtr);
        }
        else {
                xil_printf("+===> HPD disconnection event detected.\n\n");
        }
}

/******************************************************************************/
/**
 * This function is called when a Hot-Plug-Detect (HPD) pulse is received by the
 * DisplayPort TX core. The XDPTX_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK bit
 * of the core's XDPTX_INTERRUPT_STATUS register indicates that an HPD event has
 * occurred.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return      None.
 *
 * @note        Use the XDptx_SetHpdPulseHandler driver function to set this
 *              function as the handler for HPD pulses.
 *
*******************************************************************************/
static void Dptx_HpdPulseHandler(void *InstancePtr)
{
        XDptx *XDptx_InstancePtr = (XDptx *)InstancePtr;

        xil_printf("===> HPD pulse detected.\n");

        Dptx_Run(XDptx_InstancePtr);
}

/******************************************************************************/
/**
 * This function is used to override the driver's default sleep functionality.
 * For MicroBlaze systems, the XDptx_WaitUs driver function's default behavior
 * is to use the MB_Sleep function from microblaze_sleep.h, which is implemented
 * in software and only has millisecond accuracy. For this reason, using a
 * hardware timer is preferrable. For ARM/Zynq SoC systems, the SoC's timer is
 * used - XDptx_WaitUs will ignore this custom timer handler.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
 *
 * @return      None.
 *
 * @note        Use the XDptx_SetUserTimerHandler driver function to set this
 *              function as the handler for when the XDptx_WaitUs driver
 *              function is called.
 *
*******************************************************************************/
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
                        (XDptx_InstancePtr->Config.SAxiClkHz / 1000000)));

        XTmrCtr_Stop(XDptx_InstancePtr->UserTimerPtr, 0);
}
