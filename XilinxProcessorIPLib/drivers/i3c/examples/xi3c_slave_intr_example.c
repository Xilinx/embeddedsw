
/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 * @file xi3c_slave_intr_example.c
 *
 * Design example to use the I3C device as slave in interrupt mode.
 *
 * It performs the send and receive operations in slave mode.
 *
 * Note:
 * Master need to check for slave devices availability and then assign address.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.2 gm   02/18/25  Add support for Slave mode
 *
 * </pre>
 *
 ****************************************************************************/

#include "xil_printf.h"
#include "xi3c.h"
#include "xi3c_hw.h"

#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif
#endif

#ifndef SDT
#define XI3C_DEVICE_ID          XPAR_XI3C_0_DEVICE_ID
#else
#define XI3C_BASEADDRESS        XPAR_XI3C_0_BASEADDR
#endif

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
#define I3C_INTR_ID	XPAR_INTC_0_I3C_0_VEC_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define I3C_INTR_ID		XPAR_FABRIC_AXI_I3C_0_INTERRUPT_INTR
#define INTC			XScuGic
#define INTC_HANDLER		XScuGic_InterruptHandler
#endif
#endif

/*
 * Length should be less than half of fifo depth for slave mode
 */
#define I3C_DATALEN		64

/************************** Function Prototypes *******************************/

#ifndef SDT
int I3cSlaveIntrExample(u16 DeviceId);
static int SetupInterruptSystem(XI3c *InstancePtr);
#else
int I3cSlaveIntrExample(UINTPTR BaseAddress);
#endif

void Handler(u32 Event);

/************************** Variable Definitions ******************************/
XI3c Xi3c_Instance;
XI3c *InstancePtr = &Xi3c_Instance;

#ifndef SDT
INTC Intc;      /* The instance of the Interrupt Controller Driver */
#endif

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
volatile u32 TransferComplete;
volatile u32 TotalErrorCount;

/******************************************************************************/
/**
*
* Main function to call the interrupt example.
*
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
*******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("I3C Slave interrupt Example Test \r\n");

	/*
	 * Run the I3c intr example in slave mode, specify the Device
	 * ID that is specified in xparameters.h.
	 */
#ifndef SDT
	Status = I3cSlaveIntrExample(XI3C_DEVICE_ID);
#else
        Status = I3cSlaveIntrExample(XI3C_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("I3C Slave Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran I3C Slave Interrupt Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the I3c device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XI3c driver.
*
* This function sends and receives data through the I3C.
*
* This function uses interrupt driver mode of the I3C.
*
* @param	DeviceId is the Device ID of the I3c Device and is the
*		XPAR_<I3C_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
*******************************************************************************/
#ifndef SDT
int I3cSlaveIntrExample(u16 DeviceId)
#else
int I3cSlaveIntrExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XI3c_Config *CfgPtr;
	u8 TxData[I3C_DATALEN];
	u8 RxData[I3C_DATALEN];
	u16 Index;

#ifndef SDT
	CfgPtr = XI3c_LookupConfig(DeviceId);
#else
	CfgPtr = XI3c_LookupConfig(BaseAddress);
#endif
	if (NULL == CfgPtr) {
		return XST_FAILURE;
	}
	XI3c_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);

	/*
	 * Setup the Interrupt System.
	 */
#ifndef SDT
	Status = SetupInterruptSystem(InstancePtr);
#else
	Status = XSetupInterruptSystem(InstancePtr, &XI3c_SlaveInterruptHandler,
				       CfgPtr->IntrId, CfgPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XI3c_SetStatusHandler(InstancePtr, Handler);

	/*
	 * Fill data to buffer
	 */
	for (Index = 0; Index < I3C_DATALEN; Index++) {
		TxData[Index] = Index + 1;		/** < Test data */
		RxData[Index] = 0;
	}

	/*
	 * Wait for address assignment
	 */
	while (!XI3c_IsDyncAddrAssigned(InstancePtr));

	/*
	 * Master need to send SETMRL CCC to set max read length
	 */
	TransferComplete = FALSE;
	Status = XI3c_SlaveRecv(InstancePtr, RxData);

	if (Status != XST_SUCCESS) {
		return Status;
	}

	while (!TransferComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	/*
	 * Master need to send SETMWL CCC to set max write length
	 */
	TransferComplete = FALSE;
	Status = XI3c_SlaveRecv(InstancePtr, RxData);

	if (Status != XST_SUCCESS) {
		return Status;
	}

	while (!TransferComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	/*
	 * Send
	 */
	TransferComplete = FALSE;
	Status = XI3c_SlaveSend(InstancePtr, TxData, I3C_DATALEN);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	while (!TransferComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	/*
	 * Recv
	 */
	TransferComplete = FALSE;
	Status = XI3c_SlaveRecv(InstancePtr, RxData);
	if (Status != XST_SUCCESS)
		return Status;

	while (!TransferComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	xil_printf("Slave Recv data: \r\n");
	for (Index = 0; Index < I3C_DATALEN; Index++) {
		xil_printf("0x%x  ", RxData[Index]);
		if (Index != 0 && Index % 10 == 0)
			xil_printf("\n");
	}

	return XST_SUCCESS;
}

#ifndef SDT
/*****************************************************************************/
/**
* This function setups the interrupt system so interrupts can occur for the
* I3C device. The function is application-specific since the actual system may
* or may not have an interrupt controller. The I3C device could be directly
* connected to a processor without an interrupt controller. The user should
* modify this function to fit the application.
*
* @param	I3cInstPtr contains a pointer to the instance of the I3C device
*		which is going to be connected to the interrupt controller.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int SetupInterruptSystem(XI3c *I3cInstPtr)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID

	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 */
	Status = XIntc_Initialize(&Intc, INTC_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(&Intc, I3C_INTR_ID,
			       (XInterruptHandler) XI3c_SlaveInterruptHandler,
			       I3cInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts.
	 */
	Status = XIntc_Start(&Intc, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupts for the I3C device.
	 */
	XIntc_Enable(&Intc, I3C_INTR_ID);

#else

	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&Intc, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(&Intc, I3C_INTR_ID,
				       0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(&Intc, I3C_INTR_ID,
				 (Xil_InterruptHandler)XI3c_SlaveInterruptHandler,
				 I3cInstPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the I3C device.
	 */
	XScuGic_Enable(&Intc, I3C_INTR_ID);

#endif

	/*
	 * Initialize the exception table and register the interrupt
	 * controller handler with the exception table
	 */
	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)INTC_HANDLER, &Intc);

	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function is the handler which updates transfer status up on events
* from the I3C.  It is called from an interrupt context such that the amount
* of processing performed should be minimized.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void Handler(u32 Event)
{
	if (Event == 0)
		TransferComplete = TRUE;
	else
		TotalErrorCount++;

}
