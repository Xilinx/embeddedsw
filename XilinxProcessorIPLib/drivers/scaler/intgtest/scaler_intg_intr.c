/*****************************************************************************
*
*       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2014 Xilinx Inc.
*       All rights reserved.
*
*****************************************************************************/
/*****************************************************************************/
/**
*
* @file scaler_intg_intr.c
*
* This file contains an integration test on Scaler driver in interrupt mode.
*
* @note
*
* This code assumes that Zynq702 is the processor in the hardware system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------ -------- ---------------------------------------------
* 7.0   adk   22/08/14 First release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "scaler_intgtest.h"

/************************** Constant Definitions *****************************/


/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

static int SetupInterruptSystem(XScaler *ScalerInstPtr);

/* void Handler(void *CallBackRef, u32 Event, unsigned int EventData);*/
void Handler(void *CallBackRef, u32 Event);
static int ScalerInterruptTest(int TestLoops);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* This function setups the interrupt system so interrupts can occur for the
* SCALER core. The function is application-specific since the actual system may
* or may not have an interrupt controller. The SCALER device could be directly
* connected to a processor without an interrupt controller. The user should
* modify this function to fit the application.
*
* @param	ScalerInstPtr contains a pointer to the instance of the XScaler
*		which is going to be connected to the interrupt controller.
*
* @return	XST_SUCCESS if successful, or a specific error code defined in
*		"xstatus.h" if an error occurs.
*
* @note		This function assumes a Zynq702 system and no operating system
*		is used.
*
******************************************************************************/
static int SetupInterruptSystem(XScaler * ScalerInstPtr)
{
	int Status;

	/* The configuration parameters of the interrupt controller */
	XScuGic_Config *IntcConfig;

	/* Initialize the interrupt controller driver so that it is ready
	 * to use
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the Zynq702 processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
					&InterruptController);

	/* Connect the device driver handler that will be called when an interrupt
	 * for the device occurs, the handler defined above performs the specific
	 * interrupt processing for the device
	 */
	Status = XScuGic_Connect(&InterruptController,
				SCALER_INT_IRQ_ID,
				(Xil_ExceptionHandler)XScaler_IntrHandler,
				ScalerInstPtr);

	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Enable the interrupt for the Scaler device */
	XScuGic_Enable(&InterruptController, SCALER_INT_IRQ_ID);

	/* Enable interrupts in the Zynq702 */
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function tests the interrupt functionality of the Scaler driver.
*
* @param	TestLoops is the number of times to run the test.
*
* @return	the number of errors that occurred.
*
* @note		None.
*
****************************************************************************/
int Scaler_Intg_InterruptTest(int TestLoops)
{
	int Status = 0;

	CT_TestReset("Scaler interrupt mode test .. ..");

	while (TestLoops--) {
		CT_NotifyNextPass();
		Status = ScalerInterruptTest(TestLoops);
	}

	CT_CMP_NUM(int, Status, XST_SUCCESS);

	return (CT_GetTestFailures());
}

/*****************************************************************************/
/**
*
* This function implements the Scaler interrupt mode test functionality.
*
* @param	TestLoops is the number of times to run the test.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int ScalerInterruptTest(int TestLoops)
{
	int Status;
	/* u32 LoopCount;
	u32 IntrMask;
	u8 Bytes; */

	/* Initialize XScaler instance. */
	Status = Scaler_Initialize(&ScalerInst, (u16)SCALER_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Setup interrupt system */
	Status = SetupInterruptSystem(&ScalerInst);
	CT_CMP_NUM(int, Status, XST_SUCCESS);
	if (Status != XST_SUCCESS) {
		CT_Message("Failed test(s): %u\n", CT_GetTestFailures());
		return Status;
	}

	/* Set Handler, so the application specific processing can be
	 * performed for state change due to interrupt.
	 */
	XScaler_SetCallBack(&ScalerInst,
			(XScaler_CallBack)Handler, (void *)(&ScalerInst));

//	XScaler_SetCallBack(&ScalerInst, 5,
//	(XScaler_CallBack)Handler, (void *)(&ScalerInst));


	/* Enable interrupts for the device. */
	XScaler_IntrEnable(&ScalerInst);

	/* Disable interrupts for the device. */
	XScaler_IntrDisable(&ScalerInst);

	/* IntrMask = ;*/

	return (CT_GetTestFailures());
}

/*****************************************************************************/
/**
*
* This function handles application specific processing for the interrupt.
*
* @param	CallBackRef pointer to the provided argument.
* @param	Event is event that triggered the interrupt.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Handler(void *CallBackRef, u32 Event)
{
	xil_printf("Scaler Intr\n");

	/* Implement what you want to after interrupt */
}
