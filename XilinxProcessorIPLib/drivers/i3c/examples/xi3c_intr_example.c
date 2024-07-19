
/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 * @file xi3c_intr_example.c
 *
 * Design example to use the I3C device as master in interrupt mode.
 *
 * It makes the slave static address as their dynamic address.
 * It sends and also receives data from slave.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00 gm   02/9/24 First release
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

#define I3C_DATALEN		90
#define I3C_SLAVE_ADDR		0x45

/************************** Function Prototypes *******************************/

#ifndef SDT
int I3cMasterIntrExample(u16 DeviceId);
static int SetupInterruptSystem(XI3c *InstancePtr);
#else
int I3cMasterIntrExample(UINTPTR BaseAddress);
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

	xil_printf("I3C Master interrupt Example Test \r\n");

	/*
	 * Run the I3c intr example in master mode, specify the Device
	 * ID that is specified in xparameters.h.
	 */
#ifndef SDT
	Status = I3cMasterIntrExample(XI3C_DEVICE_ID);
#else
        Status = I3cMasterIntrExample(XI3C_BASEADDRESS);
#endif
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
int I3cMasterIntrExample(u16 DeviceId)
#else
int I3cMasterIntrExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XI3c_Config *CfgPtr;
	u8 TxData[I3C_DATALEN];
	u8 RxData[I3C_DATALEN];
	XI3c_Cmd Cmd;
	u16 Index;
	u8 MaxLen[2];

	MaxLen[0] = (I3C_DATALEN & 0xFF00) >> 8;
	MaxLen[1] = (I3C_DATALEN & 0x00FF);

#ifndef SDT
	CfgPtr = XI3c_LookupConfig(DeviceId);
#else
	CfgPtr = XI3c_LookupConfig(BaseAddress);
#endif
	if (NULL == CfgPtr) {
		return XST_FAILURE;
	}
	XI3c_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);

	XI3C_BusInit(InstancePtr);

	/*
	 * Setup the Interrupt System.
	 */
#ifndef SDT
	Status = SetupInterruptSystem(InstancePtr);
#else
	Status = XSetupInterruptSystem(InstancePtr, &XI3c_MasterInterruptHandler,
				       CfgPtr->IntrId, CfgPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XI3c_SetStatusHandler(InstancePtr, Handler);

	/*
	 * Set Static address as dynamic address
	 */
	Cmd.NoRepeatedStart = 1;        /**< Disable repeated start */
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.Rw = 0;
	Cmd.CmdType = 1;
	Status = XI3c_SendTransferCmd(InstancePtr, &Cmd,
				      (u8)XI3C_CCC_BRDCAST_SETAASA);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Set Max Write length
	 */
	TransferComplete = FALSE;
	Cmd.NoRepeatedStart = 0;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.Rw = 0;
	Cmd.CmdType = 1;
	Status = XI3c_SendTransferCmd(InstancePtr, &Cmd, (u8)XI3C_CCC_SETMWL);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Cmd.SlaveAddr = (u8)I3C_SLAVE_ADDR;
	Cmd.NoRepeatedStart = 1;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.CmdType = 1;                /**< SDR mode */
	Status = XI3c_MasterSend(InstancePtr, &Cmd, MaxLen, 2);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	while (!TransferComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	/*
	 * Set Max read length
	 */
	TransferComplete = FALSE;
	Cmd.NoRepeatedStart = 0;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.Rw = 0;
	Cmd.CmdType = 1;
	Status = XI3c_SendTransferCmd(InstancePtr, &Cmd, (u8)XI3C_CCC_SETMRL);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Cmd.SlaveAddr = (u8)I3C_SLAVE_ADDR;
	Cmd.NoRepeatedStart = 1;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.CmdType = 1;
	Status = XI3c_MasterSend(InstancePtr, &Cmd, MaxLen, 2);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	while (!TransferComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	/*
	 * Fill data to buffer
	 */

	for (Index = 0; Index < I3C_DATALEN; Index++) {
		TxData[Index] = Index;		/** < Test data */
		RxData[Index] = 0;
	}

	/*
	 * Send
	 */
	TransferComplete = FALSE;
	Cmd.SlaveAddr = (u8)I3C_SLAVE_ADDR;
	Cmd.NoRepeatedStart = 1;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.CmdType = 1;
	Status = XI3c_MasterSend(InstancePtr, &Cmd, TxData, I3C_DATALEN);
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
	Cmd.SlaveAddr = I3C_SLAVE_ADDR;
	Cmd.NoRepeatedStart = 1;
	Cmd.Tid = 0;
	Cmd.Pec = 0;
	Cmd.CmdType = 1;
	Status = XI3c_MasterRecv(InstancePtr, &Cmd, RxData, I3C_DATALEN);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	while (!TransferComplete) {
		if (0 != TotalErrorCount) {
			return XST_FAILURE;
		}
	}

	for (Index = 0; Index < I3C_DATALEN; Index++) {
		if(TxData[Index] != RxData[Index]) {
			xil_printf("Data miss match at index 0x%x\r\n", Index);
			return XST_FAILURE;
		}
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
			       (XInterruptHandler) XI3c_MasterInterruptHandler,
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
				 (Xil_InterruptHandler)XI3c_MasterInterruptHandler,
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
