
/******************************************************************************
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 * @file xi3cpsx_intr_example.c
 *
 * Design example to use the I3C device as master in interrupt-driven mode.
 *
 * It perform read operation with repeated start by first sending register
 * address without TOC and receives data from slave.
 * This example runs on versal net evaluation board.
 *
 * LSM6DSO sensor interfaced on I3CPS0 of versal net evaluation board.
 * Refer data sheet of LSM6DSO sensor for slave device registers details.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00 sd   06/21/22 First release
 * 1.3  sd   11/17/23 Added support for system device-tree flow
 * 1.4  gm   10/06/24 Added return statements and remove resetfifos
 * 1.6  an   07/08/25 Fixed GCC warnings
 *
 * </pre>
 *
 ****************************************************************************/

#include "xil_printf.h"
#include "xi3cpsx.h"
#include "xi3cpsx_hw.h"
#include "xi3cpsx_pr.h"
#include "xscugic.h"
#include "xil_exception.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

XI3cPsx Xi3cPsx_Instance;
XScuGic InterruptController;	/* Instance of the Interrupt Controller */
XI3cPsx *InstancePtr = &Xi3cPsx_Instance;

#ifndef SDT
int I3cPsxMasterIntrExample(u16 DeviceId);
static int SetupInterruptSystem(XI3cPsx *InstancePtr);
#else
int I3cPsxMasterIntrExample(UINTPTR BaseAddress);
#endif

#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define I3C_INT_VEC_ID		XPAR_PSVXL_I3C_0_INTR
#ifndef SDT
#define I3C_DEVICE_ID		XPAR_XI3CPSX_0_DEVICE_ID
#else
#define I3C_DEVICE_ID		XPAR_XI3CPSX_0_BASEADDR
#endif

/*
 * LSM6DSO sensor registers
 */
#define I3C_WHO_AM_I		0x0F
#define CTRL3_C			0x12
#define CTRL9_XL		0x18

#define I3C_DATALEN		10

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
volatile u32 TransferComplete;
volatile u32 TotalErrorCount;

/************************** Function Prototypes *******************************/

void Handler(void *CallBackRef, u32 Event);

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

	xil_printf("I3C Master Interrupt Example Test \r\n");

	/*
	 * Run the I3c interrupt example in master mode, specify the Device
	 * ID that is specified in xparameters.h.
	 */
	Status = I3cPsxMasterIntrExample(I3C_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("I3C Master Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran I3C Master Interrupt Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the I3c device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XI3cPsx driver.
*
* This function sends data and expects to receive the same data through the I3C
*
* This function uses interrupt driver mode of the I3C.
*
* @param	DeviceId is the Device ID of the I3cPsx Device and is the
*		XPAR_<I3C_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
*******************************************************************************/
#ifndef SDT
int I3cPsxMasterIntrExample(u16 DeviceId)
#else
int I3cPsxMasterIntrExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XI3cPsx_Config *CfgPtr;
	u8 RxData[I3C_DATALEN];
	u8 RegAddr;
	int Index;
	u16 RxLen;
	XI3cPsx_Cmd Cmd;

#ifndef SDT
	CfgPtr = XI3cPsx_LookupConfig(DeviceId);
#else
	CfgPtr = XI3cPsx_LookupConfig(BaseAddress);
#endif
	if (NULL == CfgPtr) {
		return XST_FAILURE;
	}

	Status = XI3cPsx_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef SDT
	Status = SetupInterruptSystem(InstancePtr);
#else
	Status = XSetupInterruptSystem(InstancePtr, XI3cPsx_MasterInterruptHandler,
				       CfgPtr->IntrId,
				       CfgPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XI3cPsx_SetStatusHandler(InstancePtr, (void *)InstancePtr, Handler);

	/*
	 * Read I3C_WHO_AM_I register of sensor
	 * First write register address with repeated start(TOC bit not set)
	 * then issue read operation
	 */

	XI3cPsx_ResetFifos(InstancePtr);

	/*
	 * Transfer Argument
	 */
	RegAddr = I3C_WHO_AM_I;

	Cmd.TransArg = COMMAND_PORT_SDA_DATA_BYTE_1(RegAddr) |
			   COMMAND_PORT_SDA_BYTE_STRB_1 |
			   COMMAND_PORT_SHORT_DATA_ARG;

	/*
	 * Transfer command
	 */
	Cmd.TransCmd = (COMMAND_PORT_SPEED(0) |
			    COMMAND_PORT_DEV_INDEX(0) |
			    COMMAND_PORT_SDAP);

	/*
	 * Register address passed above as part of transfer argument
	 * So, no need to pass again.
	 */
	Status = XI3cPsx_MasterSend(InstancePtr, NULL, 0, Cmd);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read register data
	 */
	for (Index = 0; Index < I3C_DATALEN; Index++) {
		RxData[Index] = 0;
	}

	TransferComplete = FALSE;
	RxLen = 1;
	Cmd.TransArg = COMMAND_PORT_ARG_DATA_LEN(RxLen) | COMMAND_PORT_TRANSFER_ARG;
	Cmd.TransCmd =	(COMMAND_PORT_READ_TRANSFER |
				 COMMAND_PORT_SPEED(0) |
				 COMMAND_PORT_DEV_INDEX(0) |
				 COMMAND_PORT_TID(1) |
				 COMMAND_PORT_ROC |
				 COMMAND_PORT_TOC);
	Status = XI3cPsx_MasterRecv(InstancePtr, RxData, RxLen, &Cmd);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (!TransferComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	xil_printf("Data at I3C_WHO_AM_I(0x0F) is 0x%x\n", RxData[0]);

	/*
	 * Read CTRL3_C register of sensor
	 * First write register address with repeated start(TOC bit not set)
	 * then issue read operation
	 */

	/*
	 * Transfer Argument
	 */
	RegAddr = CTRL3_C;

	Cmd.TransArg = COMMAND_PORT_SDA_DATA_BYTE_1(RegAddr) |
			   COMMAND_PORT_SDA_BYTE_STRB_1 |
			   COMMAND_PORT_SHORT_DATA_ARG;

	/*
	 * Transfer command
	 */
	Cmd.TransCmd = (COMMAND_PORT_SPEED(0) |
			    COMMAND_PORT_DEV_INDEX(0) |
			    COMMAND_PORT_SDAP);

	/*
	 * Register address passed above as part of transfer argument
	 * So, no need to pass again.
	 */
	Status = XI3cPsx_MasterSend(InstancePtr, NULL, 0, Cmd);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read register data
	 */
	for (Index = 0; Index < I3C_DATALEN; Index++) {
		RxData[Index] = 0;
	}

	TransferComplete = FALSE;
	RxLen = 1;
	Cmd.TransArg = COMMAND_PORT_ARG_DATA_LEN(RxLen) | COMMAND_PORT_TRANSFER_ARG;
	Cmd.TransCmd =	(COMMAND_PORT_READ_TRANSFER |
				 COMMAND_PORT_SPEED(0) |
				 COMMAND_PORT_DEV_INDEX(0) |
				 COMMAND_PORT_TID(1) |
				 COMMAND_PORT_ROC |
				 COMMAND_PORT_TOC);
	Status = XI3cPsx_MasterRecv(InstancePtr, RxData, RxLen, &Cmd);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (!TransferComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	xil_printf("Data at CTRL3_C(0x12) is 0x%x\n", RxData[0]);

	/*
	 * Read CTRL9_XL register of sensor
	 * First write register address with repeated start(TOC bit not set)
	 * then issue read operation.
	 *
	 * Transfer Argument
	 */

	RegAddr = CTRL9_XL;

	Cmd.TransArg = COMMAND_PORT_SDA_DATA_BYTE_1(RegAddr) |
			   COMMAND_PORT_SDA_BYTE_STRB_1 |
			   COMMAND_PORT_SHORT_DATA_ARG;

	/*
	 * Transfer command
	 */
	Cmd.TransCmd = (COMMAND_PORT_SPEED(0) |
			    COMMAND_PORT_DEV_INDEX(0) |
			    COMMAND_PORT_SDAP);

	/*
	 * Register address passed above as part of transfer argument
	 * So, no need to pass again.
	 */
	Status = XI3cPsx_MasterSend(InstancePtr, NULL, 0, Cmd);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read register data
	 */
	for (Index = 0; Index < I3C_DATALEN; Index++) {
		RxData[Index] = 0;
	}

	TransferComplete = FALSE;
	RxLen = 1;
	Cmd.TransArg = COMMAND_PORT_ARG_DATA_LEN(RxLen) | COMMAND_PORT_TRANSFER_ARG;
	Cmd.TransCmd =	(COMMAND_PORT_READ_TRANSFER |
				 COMMAND_PORT_SPEED(0) |
				 COMMAND_PORT_DEV_INDEX(0) |
				 COMMAND_PORT_TID(1) |
				 COMMAND_PORT_ROC |
				 COMMAND_PORT_TOC);
	Status = XI3cPsx_MasterRecv(InstancePtr, RxData, RxLen, &Cmd);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (!TransferComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	xil_printf("Data at CTRL9_XL(0x18) is 0x%x\n", RxData[0]);

	return XST_SUCCESS;
}
#ifndef SDT
/******************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the I3C.  This function is application specific since the actual
* system may or may not have an interrupt controller.  The I3C could be
* directly connected to a processor without an interrupt controller.  The
* user should modify this function to fit the application.
*
* @param	IicPsPtr contains a pointer to the instance of the Iic
*		which is going to be connected to the interrupt controller.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*******************************************************************************/
static int SetupInterruptSystem(XI3cPsx *InstancePtr)
{
	int Status;
	XScuGic_Config *IntcConfig; /* Instance of the interrupt controller */

	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
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

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     &InterruptController);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(&InterruptController, I3C_INT_VEC_ID,
				 (Xil_InterruptHandler)XI3cPsx_MasterInterruptHandler,
				 (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Iic device.
	 */
	XScuGic_Enable(&InterruptController, I3C_INT_VEC_ID);

	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function is the handler which updates transfer status up on events
* from the I3cPsx.  It is called from an interrupt context such that the amount
* of processing performed should be minimized.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void Handler(void *CallBackRef, u32 Event)
{
	(void)CallBackRef;

	if (!Event == 0)
		TotalErrorCount++;

	TransferComplete = TRUE;
}
