/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xpsmfw_iomodule.c
*
* This file contains IO Module interrupt handling functions
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

/***************************** Include Files *********************************/
#include "xpsmfw_iomodule.h"
#include "xpsmfw_power.h"
#include "xpsmfw_reset.h"
#include "xpsmfw_ipi_manager.h"
#include "psm_global.h"

#define XPSMFW_MB_MSR_BIP_MASK		(0x8U)
#define XPSMFW_MB_MSR_IE_MASK		(0x2U)

static XIOModule IOModule;

static void XPsmFw_IpiHandler(void)
{
	u32 Mask;
	int Status = XST_FAILURE;

	Mask = XPsmFw_Read32(IPI_PSM_ISR);
#ifdef XPAR_PSU_IPI_PSM_DEVICE_ID
	Status = XPsmFw_DispatchIpiHandler(Mask);
#else
	XPsmFw_Printf(DEBUG_ERROR, "PSM IPI channel is not enabled\r\n");
#endif
	XPsmFw_Write32(IPI_PSM_ISR, Mask);

	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling IPI interrupt\r\n");
	}
}

static void XPsmfw_InterruptPwrUpHandler(void)
{
	u32 PwrUpStatus, PwrUpIntMask;
	XStatus Status;

	PwrUpStatus = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRUP_STATUS);
	PwrUpIntMask = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRUP_INT_MASK);
	Status = XPsmFw_DispatchPwrUpHandler(PwrUpStatus, PwrUpIntMask);
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling Power up interrupt\r\n");
	}
}

static void XPsmfw_InterruptPwrDwnHandler(void)
{
	u32 PwrDwnStatus, PwrDwnIntMask, PwrUpStatus, PwrUpIntMask;
	XStatus Status;

	PwrDwnStatus = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRDWN_STATUS);
	PwrDwnIntMask = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRDWN_INT_MASK);
	PwrUpStatus = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRUP_STATUS);
	PwrUpIntMask = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_PWRUP_INT_MASK);
	Status = XPsmFw_DispatchPwrDwnHandler(PwrDwnStatus, PwrDwnIntMask,
			PwrUpStatus, PwrUpIntMask);
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling Power down interrupt\r\n");
	}
}

static void XPsmfw_InterruptWakeupHandler(void)
{
	u32 WakeupStatus, WakeupIntMask;
	XStatus Status;

	WakeupStatus = XPsmFw_Read32(PSM_GLOBAL_REG_WAKEUP_IRQ_STATUS);
	WakeupIntMask = XPsmFw_Read32(PSM_GLOBAL_REG_WAKEUP_IRQ_MASK);
	Status = XPsmFw_DispatchWakeupHandler(WakeupStatus, WakeupIntMask);
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling wakeup interrupt\r\n");
	}
}

static void XPsmfw_InterruptPwrCtlHandler(void)
{
	u32 PwrCtlStatus, PwrCtlIntMask;
	XStatus Status;

	PwrCtlStatus = XPsmFw_Read32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_STATUS);
	PwrCtlIntMask = XPsmFw_Read32(PSM_GLOBAL_REG_PWR_CTRL_IRQ_MASK);
	Status = XPsmFw_DispatchPwrCtlHandler(PwrCtlStatus, PwrCtlIntMask);
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling power control interrupt\r\n");
	}
}

static void XPsmfw_InterruptSwRstHandler(void)
{
	u32 SwRstStatus, SwRstIntMask;
	XStatus Status;

	SwRstStatus = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_SWRST_STATUS);
	SwRstIntMask = XPsmFw_Read32(PSM_GLOBAL_REG_REQ_SWRST_INT_MASK);
	Status = XPsmFw_DispatchSwRstHandler(SwRstStatus, SwRstIntMask);
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling software reset interrupt\r\n");
	}
}

/* Structure for Top level interrupt table */
static struct HandlerTable g_TopLevelInterruptTable[] = {
	{PSM_IOMODULE_IRQ_PENDING_IPI_MASK, XPsmFw_IpiHandler},
	{PSM_IOMODULE_IRQ_PENDING_PWR_UP_REQ_MASK, XPsmfw_InterruptPwrUpHandler},
	{PSM_IOMODULE_IRQ_PENDING_PWR_DWN_REQ_MASK, XPsmfw_InterruptPwrDwnHandler},
	{PSM_IOMODULE_IRQ_PENDING_WAKE_UP_REQ_MASK, XPsmfw_InterruptWakeupHandler},
	{PSM_IOMODULE_IRQ_PENDING_PWR_CNT_REQ_MASK, XPsmfw_InterruptPwrCtlHandler},
	{PSM_IOMODULE_IRQ_PENDING_SW_RST_REQ_MASK, XPsmfw_InterruptSwRstHandler},
};

/**
* This function  initializes the IO module and enables the interrupts
*
* This function uses interrupt driven mode of the IO Module.
*
* @param        DeviceId is the XPAR_<IOModule_instance>_DEVICE_ID value from
*               xparameters.h
*
* @return      None
*
********************************************************************************/
int XPsmFw_IoModuleInit(u32 DeviceId)
{
    u32 Status;

    /*
     * Initialize the IO Module so that it's ready to use, specify the device
     * ID that is generated in xparameters.h
     */
    Status = XIOModule_Initialize(&IOModule, DeviceId);
    if (Status != XST_SUCCESS) {
		XPsmFw_Printf(DEBUG_ERROR, "IO Module: Init failed\r\n");
            goto END;
    }

	Status = XIOModule_SelfTest(&IOModule);
	if (Status != XST_SUCCESS) {
		XPsmFw_Printf(DEBUG_ERROR, "IO Module: Self test failed\r\n");
        goto END;
    }

	Status = SetUpInterruptSystem();
	if (Status != XST_SUCCESS) {
		XPsmFw_Printf(DEBUG_ERROR,
		        "IO Module: Connecting intr handler to IO Module failed\r\n");
        goto END;
    }

	XPsmFw_Printf(DEBUG_PRINT_ALWAYS, "IO Module init completed\r\n");

 	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
*
* This function connects the interrupt handler of the IO Module to the
* processor.
*
* @param    None.
*
* @return   None.
*
* @note     None.
*
****************************************************************************/
int SetUpInterruptSystem(void)
{
	u32 IntrNumber;

	/*
     * Connect a device driver handler that will be called when an interrupt
     * for the device occurs, the device driver handler performs the specific
     * interrupt processing for the device
     */
	for (IntrNumber = 0; IntrNumber < XPAR_IOMODULE_INTC_MAX_INTR_SIZE;
	     IntrNumber++) {
		XIOModule_Connect(&IOModule, IntrNumber,
		              (XInterruptHandler)XPsmFw_IntrHandler, (void *)IntrNumber);

		XIOModule_Enable(&IOModule, IntrNumber);
	}

	XIOModule_Start(&IOModule);

	/*
     * Initialize the exception table.
     */
	Xil_ExceptionInit();

	/*
     * Register the IO module interrupt handler with the exception table.
     */
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                 (Xil_ExceptionHandler)XIOModule_DeviceInterruptHandler,
                 (void*) 0);

	/*
     * Enable exceptions.
     */
	Xil_ExceptionEnable();

	microblaze_enable_exceptions();

	/* microblaze_enable_interrupts(); */

	/*
	 * FIXME: Currently, IE bit in MSR is not being set when
	 * microblaze_enable_interrupts() API is called. So set
	 * this bit manually to receive interrupt. Remove this
	 * hack when issue has been fixed.
	 */
	mtmsr(mfmsr() | XPSMFW_MB_MSR_IE_MASK);

	/*
	 * Clear Break in progress to get interrupts
	 */
	mtmsr(mfmsr() & (~XPSMFW_MB_MSR_BIP_MASK));

    return XST_SUCCESS;
}

/**
 * XPsmFw_IntrHandler() - Interrupt handler
 *
 * @IntrNumber		Interrupt number
 */
void XPsmFw_IntrHandler(void *IntrNumber)
{
	u32 l_IrqReg;
	u32 l_index;
	XPsmFw_Printf(DEBUG_PRINT_ALWAYS,
	              "Interrupt number = 0x%x\r\n", (u32)IntrNumber);
	l_IrqReg = XPsmFw_Read32(PSM_IOMODULE_IRQ_PENDING);

	for(l_index = 0U; l_index < ARRAYSIZE(g_TopLevelInterruptTable);
	    l_index++) {
			if ((l_IrqReg & g_TopLevelInterruptTable[l_index].Mask)
				    == g_TopLevelInterruptTable[l_index].Mask) {
				/* Call interrupt handler */
				g_TopLevelInterruptTable[l_index].Handler();

				/* ACK the interrupt */
				XPsmFw_Write32(PSM_IOMODULE_IRQ_ACK,
				        g_TopLevelInterruptTable[l_index].Mask);
			}
	}
}
