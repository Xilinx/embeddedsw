/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xiicps_slave_monitor_example.c
 *
 * @brief Demonstrates checking the availability of a slave in monitor mode.
 *
 * This example runs on zynqmp / versal / vek385 evaluation boards using the
 * IIC device as master to check slave's availability.
 *
 * @note
 * Set the slave address to 0x3FB, which tests the device's ability
 * to handle 10-bit address.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00a jz  01/30/10 First release
 * 3.18  gm  07/14/23 Added SDT support.
 * 3.21  an  03/28/25 Added VEK385 board i2c slave address
 * 3.24  vn  08/26/26 Fix Doxygen warnings
 * 3.24  tb  09/30/26 Align Doxygen comments with AMD standards
 *
 * </pre>
 *
 ****************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xiicps.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define INTC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID /**< SCUGIC device ID. */
#endif

/** @name Example configuration
 * @{
 */
#define IIC_SCLK_RATE		100000 /**< IIC serial clock rate in Hz. */
#define SLV_MON_LOOP_COUNT 0x000FFFFF	/**< Slave Monitor Loop Count. */
/** @} */

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/

int IicPsSlaveMonitorExample();

static void Handler(void *CallBackRef, u32 Event);
#ifndef SDT
static int IicPsSlaveMonitor(u16 Address, u16 DeviceId, u32 Int_Id);
static int SetupInterruptSystem(XIicPs *IicPsPtr, u32 Int_Id);
static int IicPsConfig(u16 DeviceId, u32 Int_Id);
#else
static int IicPsSlaveMonitor(u16 Address, UINTPTR BaseAddress);
static int IicPsConfig(UINTPTR BaseAddress);
#endif
static int IicPsFindDevice(u16 Addr);

/************************** Variable Definitions ******************************/

XIicPs	Iic;			/**< Instance of the IIC Device. */
#ifndef SDT
XScuGic InterruptController;	/**< Instance of the Interrupt Controller. */
#endif

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
volatile u8 TransmitComplete; /**< Flag to check completion of Transmission. */
volatile u8 ReceiveComplete;	/**< Flag to check completion of Reception. */
volatile u32 TotalErrorCount;	/**< Total Error Count Flag. */
volatile u32 SlaveResponse;		/**< Slave Response Flag. */

/**
 * Holds the slave addresses to search for. The application can also add its
 * own slave Address in the below array list.
 */
u16 SlvAddr[] = {0x54, 0x55, 0x74, 0x72, 0};
XIicPs IicInstance;		/**< The instance of the IIC device. */
#ifdef SDT
/** Configuration table of the IIC devices. */
extern XIicPs_Config XIicPs_ConfigTable[XPAR_XIICPS_NUM_INSTANCES];
#endif

/******************************************************************************/
/**
*
* @brief Calls the Slave Monitor example from the main function.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
*******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("IIC Slave Monitor Example Test \r\n");

	/*
	 * Run the Iic Slave Monitor example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
	Status = IicPsSlaveMonitorExample();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC Slave Monitor Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC Slave Monitor Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Performs the initial configuration for the IICPS Device.
*
* @if SDT
* @param[in]    BaseAddress Base address of the device.
* @else
* @param[in]    DeviceId Device ID of the IicPs Device.
* @param[in]    Int_Id Interrupt ID mapped to the device.
* @endif
*
* @return	XST_SUCCESS if pass, otherwise XST_FAILURE.
*
****************************************************************************/
#ifndef SDT
static int IicPsConfig(u16 DeviceId, u32 Int_Id)
#else
static int IicPsConfig(UINTPTR BaseAddress)
#endif
{
	int Status;
	XIicPs_Config *ConfigPtr;	/* Pointer to configuration data */

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
#ifndef SDT
	ConfigPtr = XIicPs_LookupConfig(DeviceId);
#else
	ConfigPtr = XIicPs_LookupConfig(BaseAddress);
#endif
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&IicInstance, ConfigPtr,
				      ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the Interrupt System.
	 */
#ifndef SDT
	Status = SetupInterruptSystem(&IicInstance, Int_Id);
#else
	Status = XSetupInterruptSystem(&IicInstance, (void *)XIicPs_MasterInterruptHandler,
				       ConfigPtr->IntrId,
				       ConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handlers for the IIC that will be called from the
	 * interrupt context when data has been sent and received, specify a
	 * pointer to the IIC driver instance as the callback reference so
	 * the handlers are able to access the instance data.
	 */
	XIicPs_SetStatusHandler(&IicInstance, (void *) &IicInstance, Handler);

	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&IicInstance, IIC_SCLK_RATE);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Checks the availability of a slave using slave monitor mode.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
*******************************************************************************/
int IicPsSlaveMonitorExample(void)
{
	int Status;
	int Index;

	for (Index = 0; SlvAddr[Index] != 0; Index++) {
		Status = IicPsFindDevice(SlvAddr[Index]);
		if (Status == XST_SUCCESS) {
			return XST_SUCCESS;
		}
	}
	return XST_FAILURE;
}
/*****************************************************************************/
/**
*
* @brief Checks the availability of a slave on one IIC instance.
*
* This function checks the availability of a slave using slave monitor mode.
*
* @param[in]    Address Slave address to monitor.
* @if SDT
* @param[in]    BaseAddress Base address of the device.
* @else
* @param[in]    DeviceId Device ID of the IicPs Device.
* @param[in]    Int_Id Interrupt ID mapped to the device.
* @endif
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
*******************************************************************************/
#ifndef SDT
static int IicPsSlaveMonitor(u16 Address, u16 DeviceId, u32 Int_Id)
#else
static int IicPsSlaveMonitor(u16 Address, UINTPTR BaseAddress)
#endif
{
	u32 Index;
	int Status;
	XIicPs *IicPtr;

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
#ifndef SDT
	Status = IicPsConfig(DeviceId, Int_Id);
#else
	Status = IicPsConfig(BaseAddress);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	IicPtr = &IicInstance;
	XIicPs_DisableAllInterrupts(IicPtr->Config.BaseAddress);
	XIicPs_EnableSlaveMonitor(&IicInstance, Address);

	TotalErrorCount = 0;
	SlaveResponse = FALSE;

	Index = 0;

	/*
	 * Wait for the Slave Monitor Interrupt, the interrupt processing
	 * works in the background, this function may get locked up in this
	 * loop if the interrupts are not working correctly or the slave
	 * never responds.
	 */
	while ((!SlaveResponse) && (Index < SLV_MON_LOOP_COUNT)) {
		Index++;

		/*
		 * Ignore any errors. The hardware generates NACK interrupts
		 * if the slave is not present.
		 */
		if (0 != TotalErrorCount) {
			xil_printf("Test error unexpected NACK\n");
			return XST_FAILURE;
		}
	}

	if (Index >= SLV_MON_LOOP_COUNT) {
		return XST_FAILURE;

	}

	XIicPs_DisableSlaveMonitor(&IicInstance);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Checks whether the slave is alive or not.
*
* @param[in]    Addr Address of the slave device.
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
*******************************************************************************/
static int IicPsFindDevice(u16 Addr)
{
	int Status;
#ifdef SDT
	u32 BaseAddress;
#endif

#ifndef SDT
	Status = IicPsSlaveMonitor(Addr, 0, XPAR_XIICPS_0_INTR);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
	Status = IicPsSlaveMonitor(Addr, 1, XPAR_XIICPS_1_INTR);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
	Status = IicPsSlaveMonitor(Addr, 0, XPAR_XIICPS_1_INTR);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
	Status = IicPsSlaveMonitor(Addr, 1, XPAR_XIICPS_0_INTR);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
#else
	BaseAddress = XIicPs_ConfigTable[0].BaseAddress;
	Status = IicPsSlaveMonitor(Addr, BaseAddress);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
	BaseAddress = XIicPs_ConfigTable[1].BaseAddress;
	Status = IicPsSlaveMonitor(Addr, BaseAddress);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
#endif
	return XST_FAILURE;
}

#ifndef SDT
/******************************************************************************/
/**
*
* @brief Sets up the interrupt system so that interrupts can occur for the IIC.
*
* @param[in]    IicPsPtr Contains a pointer to the instance of the Iic
*		which is going to be connected to the interrupt controller.
* @param[in]    Int_Id Interrupt ID mapped to the device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
*******************************************************************************/
static int SetupInterruptSystem(XIicPs *IicPsPtr, u32 Int_Id)
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
	Status = XScuGic_Connect(&InterruptController, Int_Id,
				 (Xil_InterruptHandler)XIicPs_MasterInterruptHandler,
				 (void *)IicPsPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Iic device.
	 */
	XScuGic_Enable(&InterruptController, Int_Id);


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
* @brief Handles data events from the IIC.
*
* This function is the handler which performs processing to handle data events
* from the IIC. It is called from an interrupt context such that the amount
* of processing performed should be minimized.
*
* This handler provides an example of how to handle data for the IIC and
* is application specific.
*
* @param[in]    CallBackRef Contains a callback reference from the driver, in
*		this case it is the instance pointer for the IIC driver.
* @param[in]    Event Contains the specific kind of event that has occurred.
*
*******************************************************************************/
void Handler(void *CallBackRef, u32 Event)
{
	(void)CallBackRef;

	/*
	 * All of the data transfer has been finished.
	 */

	if (0 != (Event & XIICPS_EVENT_COMPLETE_SEND)) {
		TransmitComplete = TRUE;
	} else if (0 != (Event & XIICPS_EVENT_COMPLETE_RECV)) {
		ReceiveComplete = TRUE;
	} else if (0 != (Event & XIICPS_EVENT_SLAVE_RDY)) {
		SlaveResponse = TRUE;
	} else if (0 != (Event & XIICPS_EVENT_ERROR)) {
		TotalErrorCount++;
	}
}
