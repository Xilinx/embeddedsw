/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
#include "xpsmfw_ipi_manager.h"
#include "psmx_global.h"

#define XPSMFW_MB_MSR_BIP_MASK		(0x8U)

static XIOModule IOModule;

static void XPsmFw_InterruptIpiHandler(void)
{
	u32 Mask;
	XStatus Status = XST_FAILURE;

	Mask = XPsmFw_Read32(IPI_PSM_ISR);

	/* If IPI is not from PMC print error and exit */
	if (IPI_PSM_ISR_PMC_MASK != (IPI_PSM_ISR_PMC_MASK & Mask)) {
		XPsmFw_Printf(DEBUG_ERROR, "Invalid IPI interrupt\r\n");
		goto done;
	}

#ifdef XPAR_XIPIPSU_0_DEVICE_ID
	Status = XPsmFw_DispatchIpiHandler(IPI_PSM_ISR_PMC_MASK);
#else
	XPsmFw_Printf(DEBUG_ERROR, "PSM IPI channel is not enabled\r\n");
#endif

done:
	/*Clear PSM IPI ISR */
	XPsmFw_Write32(IPI_PSM_ISR, Mask);

	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling IPI interrupt\r\n");
	}
}

static void XPsmfw_InterruptPwrUpHandler(void)
{
	u32 PwrUp0Status, PwrUp0IntMask, PwrUp1Status, PwrUp1IntMask;
	XStatus Status = XST_FAILURE;

	PwrUp0Status = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP0_STATUS);
	PwrUp0IntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP0_INT_MASK);
	if(0U != PwrUp0Status){
		Status = XPsmFw_DispatchPwrUp0Handler(PwrUp0Status, PwrUp0IntMask);
	}
	PwrUp1Status = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP1_STATUS);
	PwrUp1IntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP1_INT_MASK);
	if(0U != PwrUp1Status){
		Status = XPsmFw_DispatchPwrUp1Handler(PwrUp1Status, PwrUp1IntMask);
	}
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling Power up interrupt\r\n");
	}

}

static void XPsmfw_InterruptPwrDwnHandler(void)
{
	u32 PwrDwn0Status, PwrDwn0IntMask, PwrUp0Status, PwrUp0IntMask;
	u32 PwrDwn1Status, PwrDwn1IntMask, PwrUp1Status, PwrUp1IntMask;
	XStatus Status = XST_FAILURE;

	PwrDwn0Status = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRDWN0_STATUS);
	PwrDwn0IntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRDWN0_INT_MASK);
	PwrUp0Status = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP0_STATUS);
	PwrUp0IntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP0_INT_MASK);
	PwrDwn1Status = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRDWN1_STATUS);
	PwrDwn1IntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRDWN1_INT_MASK);
	PwrUp1Status = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP1_STATUS);
	PwrUp1IntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_REQ_PWRUP1_INT_MASK);
	if(0 != PwrDwn0Status){
		Status = XPsmFw_DispatchPwrDwn0Handler(PwrDwn0Status, PwrDwn0IntMask,
				PwrUp0Status, PwrUp0IntMask);
	}
	if(0 != PwrDwn1Status){
		Status = XPsmFw_DispatchPwrDwn1Handler(PwrDwn1Status, PwrDwn1IntMask,
				PwrUp1Status, PwrUp1IntMask);
	}
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling Power down interrupt\r\n");
	}
}

static void XPsmfw_InterruptWakeupHandler(void)
{
	u32 WakeupStatus, WakeupIntMask;
	XStatus Status = XST_FAILURE;

	WakeupStatus = XPsmFw_Read32(PSMX_GLOBAL_REG_WAKEUP0_IRQ_STATUS);
	WakeupIntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_WAKEUP0_IRQ_MASK);
	if(0 != WakeupStatus){
		Status = XPsmFw_DispatchAPUWakeupHandler(WakeupStatus, WakeupIntMask);
	}

	WakeupStatus = XPsmFw_Read32(PSMX_GLOBAL_REG_WAKEUP1_IRQ_STATUS);
	WakeupIntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_WAKEUP1_IRQ_MASK);
	if(0 != WakeupStatus){
		Status = XPsmFw_DispatchRPUWakeupHandler(WakeupStatus, WakeupIntMask);
	}
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling wakeup interrupt\r\n");
	}
}

static void XPsmfw_InterruptPwrCtlHandler(void)
{
	u32 PwrCtlStatus, PwrCtlIntMask;
	XStatus Status = XST_FAILURE;

	PwrCtlStatus = XPsmFw_Read32(PSMX_GLOBAL_REG_PWR_CTRL1_IRQ_STATUS);
	PwrCtlIntMask = XPsmFw_Read32(PSMX_GLOBAL_REG_PWR_CTRL1_IRQ_MASK);
	Status = XPsmFw_DispatchPwrCtlHandler(PwrCtlStatus, PwrCtlIntMask);
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling power control interrupt\r\n");
	}
}


/* Structure for Top level interrupt table */
static struct HandlerTable g_TopLevelInterruptTable[] = {
	{PSM_IOMODULE_IRQ_PENDING_IPI_MASK, XPsmFw_InterruptIpiHandler},
	{PSM_IOMODULE_IRQ_PENDING_PWR_UP_REQ_MASK, XPsmfw_InterruptPwrUpHandler},
	{PSM_IOMODULE_IRQ_PENDING_PWR_DWN_REQ_MASK, XPsmfw_InterruptPwrDwnHandler},
	{PSM_IOMODULE_IRQ_PENDING_WAKE_UP_REQ_MASK, XPsmfw_InterruptWakeupHandler},
	{PSM_IOMODULE_IRQ_PENDING_PWR_CNT_REQ_MASK, XPsmfw_InterruptPwrCtlHandler},
	{PSM_IOMODULE_IRQ_PENDING_SW_RST_REQ_MASK, NULL},
	{PSM_IOMODULE_IRQ_PENDING_GICP_INT_MASK, NULL},
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
XStatus XPsmFw_IoModuleInit(u16 DeviceId)
{
    XStatus Status = XST_FAILURE;

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

	XPsmFw_Printf(DEBUG_DETAILED, "IO Module init completed\r\n");

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
XStatus SetUpInterruptSystem(void)
{
	u32 IntrNumber;

	/*
     * Connect a device driver handler that will be called when an interrupt
     * for the device occurs, the device driver handler performs the specific
     * interrupt processing for the device
     */
	for (IntrNumber = 0; IntrNumber < XPAR_IOMODULE_INTC_MAX_INTR_SIZE;
	     IntrNumber++) {
		if (XST_SUCCESS != XIOModule_Connect(&IOModule, (u8)IntrNumber,
						     (XInterruptHandler)XPsmFw_IntrHandler,
						     (void *)IntrNumber)) {
			XPsmFw_Printf(DEBUG_ERROR, "%s: Error! IO Module connect failed\r\n", __func__);
		}

		XIOModule_Enable(&IOModule, (u8)IntrNumber);
	}

	if (XST_SUCCESS != XIOModule_Start(&IOModule)) {
		XPsmFw_Printf(DEBUG_ERROR, "%s: Error! IO Module start failed\r\n", __func__);
	}

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

	microblaze_enable_interrupts();

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
	XPsmFw_Printf(DEBUG_DETAILED,
	              "Interrupt number = 0x%x\r\n", IntrNumber);
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
