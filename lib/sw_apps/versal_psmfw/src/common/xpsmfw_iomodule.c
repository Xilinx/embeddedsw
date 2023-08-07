/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
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
* 1.01  rama 08/07/2023 Made IOModule as global variable for STL support
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
#include "xpsmfw_plat.h"

#define XPSMFW_MB_MSR_BIP_MASK		(0x8U)

XIOModule IOModule;

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

#if defined(XPAR_XIPIPSU_0_DEVICE_ID) || defined(XPAR_XIPIPSU_0_BASEADDR)
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
	XStatus Status = XST_FAILURE;

	Status = XPsmfw_PwrUpHandler();
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling Power up interrupt\r\n");
	}
}

static void XPsmfw_InterruptPwrDwnHandler(void)
{
	XStatus Status = XST_FAILURE;

	Status = XPsmfw_PwrDwnHandler();
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling Power down interrupt\r\n");
	}
}

static void XPsmfw_InterruptWakeupHandler(void)
{
	XStatus Status = XST_FAILURE;

	Status = XPsmfw_WakeupHandler();
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling wakeup interrupt\r\n");
	}
}

static void XPsmfw_InterruptPwrCtlHandler(void)
{
	XStatus Status = XST_FAILURE;

	Status = XPsmfw_PwrCtlHandler();
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling power control interrupt\r\n");
	}
}

static void XPsmfw_InterruptGicP2Handler(void)
{
	XStatus Status = XST_FAILURE;

	Status = XPsmFw_GicP2Handler();
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error in handling GIC interrupt\r\n");
	}
}

/*****************************************************************************/
/**
 * @brief This is a function handler for all exceptions. It writes PSM
 * global register for an error.
 *
 * @param	Data Pointer
 *
 * @return	None
 *
 *****************************************************************************/
static void XPsmfw_ExceptionHandler(void *Data)
{
	/* Writing PSM Non-Correctable bit to ERR1_TRIG reg */
	XPsmFw_Write32(PSM_GLOBAL_REG_ERR1_TRIG, PSM_GLOBAL_REG_ERR1_TRIG_PSM_B_NCR_MASK);

	/* Just in case if control reaches here */
	while (TRUE) {
		;
	}
}

/*****************************************************************************/
/**
 * @brief This function enables the exceptions.
 *
 * @return	None
 *
 *****************************************************************************/
static void XPsmfw_ExceptionInit(void)
{
	u16 Index;

	/* Register exception handlers */
	for (Index = XIL_EXCEPTION_ID_FIRST; Index <= XIL_EXCEPTION_ID_LAST; Index++) {
		Xil_ExceptionRegisterHandler(Index, XPsmfw_ExceptionHandler, (void *)NULL);
	}
}

/* Structure for Top level interrupt table */
static const struct HandlerTable g_TopLevelInterruptTable[] = {
	{
		PSM_IOMODULE_IRQ_PENDING_IPI_SHIFT,
		PSM_IOMODULE_IRQ_PENDING_IPI_MASK,
		XPsmFw_InterruptIpiHandler
	},
	{
		PSM_IOMODULE_IRQ_PENDING_PWR_UP_REQ_SHIFT,
		PSM_IOMODULE_IRQ_PENDING_PWR_UP_REQ_MASK,
		XPsmfw_InterruptPwrUpHandler
	},
	{
		PSM_IOMODULE_IRQ_PENDING_PWR_DWN_REQ_SHIFT,
		PSM_IOMODULE_IRQ_PENDING_PWR_DWN_REQ_MASK,
		XPsmfw_InterruptPwrDwnHandler
	},
	{
		PSM_IOMODULE_IRQ_PENDING_WAKE_UP_REQ_SHIFT,
		PSM_IOMODULE_IRQ_PENDING_WAKE_UP_REQ_MASK,
		XPsmfw_InterruptWakeupHandler
	},
	{
		PSM_IOMODULE_IRQ_PENDING_PWR_CNT_REQ_SHIFT,
		PSM_IOMODULE_IRQ_PENDING_PWR_CNT_REQ_MASK,
		XPsmfw_InterruptPwrCtlHandler
	},
	{
		PSM_IOMODULE_IRQ_PENDING_SW_RST_REQ_SHIFT,
		PSM_IOMODULE_IRQ_PENDING_SW_RST_REQ_MASK,
		NULL
	},
	{
		PSM_IOMODULE_IRQ_PENDING_GICP_INT_SHIFT,
		PSM_IOMODULE_IRQ_PENDING_GICP_INT_MASK,
		XPsmfw_InterruptGicP2Handler
	},
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
	u32 l_index;

	/*
	* Connect a device driver handler that will be called when an interrupt
	* for the device occurs, the device driver handler performs the specific
	* interrupt processing for the device
	*/
	for(l_index = 0U; l_index < ARRAYSIZE(g_TopLevelInterruptTable); l_index++) {
		IntrNumber = g_TopLevelInterruptTable[l_index].Shift;
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
	XPsmfw_ExceptionInit();

	/*
	* Register the IO module interrupt handler with the exception table.
	*/
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XIOModule_DeviceInterruptHandler,
				     (void*) NULL);

	/*
	* Enable exceptions and interrupt
	*/
	Xil_ExceptionEnable();

	/*
	 * Clear Break in progress to get interrupts
	 */
	mtmsr(mfmsr() & (~XPSMFW_MB_MSR_BIP_MASK));

	return XST_SUCCESS;
}

#ifdef PSM_ENABLE_STL
/******************************************************************************/
/**
*
* This function registers the STL interrupt handler to the processor.
*
* @param InteruptNumber		Interrupt number
* @param StlInterruptHandler	Pointer to STL interrupt handler which needs to
* 				register
*
* @return   XST_SUCCESS or error code.
*
* @note     None.
*
****************************************************************************/
int XPsmFw_RegisterStlInterruptHandler(u8 InteruptNumber,
				       XInterruptHandler StlInterruptHandler)
{
	int Status = XST_FAILURE;

	if ((NULL == StlInterruptHandler) ||
	    (XPAR_IOMODULE_INTC_MAX_INTR_SIZE <= InteruptNumber)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	XIOModule_Disable(&IOModule, InteruptNumber);

	Status = XIOModule_Connect(&IOModule, InteruptNumber,
				   StlInterruptHandler, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XIOModule_Enable(&IOModule, InteruptNumber);

done:
	return Status;
}

/******************************************************************************/
/**
*
* This function restores the default interrupt handler.
*
* @param InteruptNumber		Interrupt number
*
* @return   XST_SUCCESS or error code.
*
* @note     None.
*
****************************************************************************/
int XPsmFw_RestoreInterruptHandler(u8 InteruptNumber)
{
	int Status = XST_FAILURE;

	if (XPAR_IOMODULE_INTC_MAX_INTR_SIZE <= InteruptNumber) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	XIOModule_Disable(&IOModule, InteruptNumber);

	Status = XIOModule_Connect(&IOModule, InteruptNumber,
				   (XInterruptHandler)XPsmFw_IntrHandler, NULL);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XIOModule_Enable(&IOModule, InteruptNumber);

done:
	return Status;
}
#endif

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
				if (NULL != g_TopLevelInterruptTable[l_index].Handler) {
					/* Call interrupt handler */
					g_TopLevelInterruptTable[l_index].Handler();
				}

				/* ACK the interrupt */
				XPsmFw_Write32(PSM_IOMODULE_IRQ_ACK,
				        g_TopLevelInterruptTable[l_index].Mask);
			}
	}
}
