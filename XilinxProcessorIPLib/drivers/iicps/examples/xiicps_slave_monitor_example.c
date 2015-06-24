/******************************************************************************
*
* Copyright (C) 2010 - 2014 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
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
 * @file xiicps_slave_monitor_example.c
 *
 * A design example of using the device as master to check slave's
 * availability.
 *
 * @note
 * Please set the slave address to 0x3FB, which tests the device's ability
 *	to handle 10-bit address.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00a jz  01/30/10 First release
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

/************************** Constant Definitions ******************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IIC_DEVICE_ID		XPAR_XIICPS_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define IIC_INT_VEC_ID		XPAR_XIICPS_0_INTR

/*
 * The slave address to send to and receive from.
 */
#define IIC_SLAVE_ADDR		0x3FB
#define IIC_SCLK_RATE		100000

/*
 * This timeout interval is used in polling mode transfers.
 * Please increase the timeout limit on faster systems to avoid
 * unintended timeout failure.
 */
#define POLL_TIME_OUT	2000000  /**< Time out count for polled transfer*/

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/

int IicPsSlaveMonitorExample(u16 DeviceId);

static int SetupInterruptSystem(XIicPs *IicPsPtr);

void Handler(void *CallBackRef, u32 Event);

/************************** Variable Definitions ******************************/

XIicPs	Iic;			/* Instance of the IIC Device */
XScuGic InterruptController;	/* Instance of the Interrupt Controller */

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
volatile int SlaveReady;
volatile int TotalError;

/******************************************************************************/
/**
*
* Main function to call the Slave Monitor example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
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
	Status = IicPsSlaveMonitorExample(IIC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("IIC Slave Monitor Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC Slave Monitor Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function checks the availability of a slave using slave monitor mode.
*
* @param	DeviceId is the Device ID of the IicPs Device and is the
*		XPAR_<IICPS_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note 	None.
*
*******************************************************************************/
int IicPsSlaveMonitorExample(u16 DeviceId)
{
	int Status;
	int Timeout;
	XIicPs_Config *Config;

	/*
	 * Initialize the IIC driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XIicPs_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XIicPs_SelfTest(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the IIC to the interrupt subsystem such that interrupts can
	 * occur.  This function is application specific.
	 */
	Status = SetupInterruptSystem(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handlers for the IIC that will be called from the
	 * interrupt context when slave response to the address transfer.
	 */
	XIicPs_SetStatusHandler(&Iic, (void *) &Iic, Handler);

	/*
	 * Set 10-bit address mode.
	 */
	XIicPs_SetOptions(&Iic, XIICPS_10_BIT_ADDR_OPTION);

	/*
	 * Set the IIC serial clock rate.
	 */
	Status = XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);
	if (Status != XST_SUCCESS) {
	return XST_FAILURE;
	}

	/*
	 * Wait for the bus to be idle.
	 */
	while (XIicPs_BusIsBusy(&Iic)) {
		/* NOP */
	}

	XIicPs_EnableSlaveMonitor(&Iic, IIC_SLAVE_ADDR);

	TotalError= 0;
	SlaveReady = FALSE;

	Timeout = POLL_TIME_OUT;

	/*
	 * Wait for the Slave Monitor Interrupt, the interrupt processing
	 * works in the background, this function may get locked up in this
	 * loop if the interrupts are not working correctly or the slave
	 * never responds.
	 */
	while ((!SlaveReady) && (Timeout > 0)) {
	Timeout --;
		/*
		 * Ignore any errors. The hardware generates NACK interrupts
		 * if the slave is not present.
		 */
		if (0 != TotalError) {
			return XST_FAILURE;
		}

	}

	if (Timeout == 0) {
		return XST_FAILURE;
	}

	XIicPs_DisableSlaveMonitor(&Iic);

	/*
	 * Clear 10-bit address mode.
	 */
	XIicPs_ClearOptions(&Iic, XIICPS_10_BIT_ADDR_OPTION);

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This function is the handler which performs processing to handle data events
* from the IIC.  It is called from an interrupt context such that the amount
* of processing performed should be minimized.
*
* This handler provides an example of how to handle data for the IIC and
* is application specific.
*
* @param	CallBackRef contains a callback reference from the driver,
*		in this case it is the instance pointer for the IIC driver.
* @param	Event contains the specific kind of event that has occurred.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void Handler(void *CallBackRef, u32 Event)
{
	if (0 != (Event & XIICPS_EVENT_SLAVE_RDY)){
		SlaveReady = TRUE;
		return;
	}
	TotalError += 1;

	return;
}

/******************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the IIC.  This function is application specific since the actual
* system may or may not have an interrupt controller.  The IIC could be
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
static int SetupInterruptSystem(XIicPs *IicPsPtr)
{
	int Status;
	XScuGic_Config *IntcConfig;

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
	Status = XScuGic_Connect(&InterruptController, IIC_INT_VEC_ID,
			(Xil_InterruptHandler)XIicPs_MasterInterruptHandler,
			(void *)IicPsPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Iic device.
	 */
	XScuGic_Enable(&InterruptController, IIC_INT_VEC_ID);


	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
