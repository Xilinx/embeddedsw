/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xadcps_intr_example.c
*
* This file contains a design example using the driver functions of the
* XADC/ADC driver. This example here shows the usage of the driver/device in
* interrupt mode to handle on-chip temperature and voltage alarm interrupts.
*
*
* @note
*
* This code assumes that no Operating System is being used.
*
* The values of the on-chip temperature and the on-chip Vccaux voltage are read
* from the device and then the alarm thresholds are set in such a manner that
* the alarms occur.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a bss    01/20/12 First release
* 2.2   ms     01/23/17 Modified xil_printf statement in main function to
*                       ensure that "Successfully ran" and "Failed" strings
*                       are available in all examples. This is a fix for
*                       CR-965028.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xadcps.h"
#include "xscugic.h"
#include "xil_exception.h"

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define XADC_DEVICE_ID		XPAR_XADCPS_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTR_ID				XPAR_XADCPS_INT_ID
#define printf				xil_printf
#endif

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

int XAdcIntrExample(XScuGic *IntcInstancePtr,
			XAdcPs *XAdcInstPtr,
			u16 XAdcDeviceId,
			u16 XAdcIntrId,
			int *Temp);

static void XAdcInterruptHandler(void *CallBackRef);

static int XAdcSetupInterruptSystem(XScuGic *IntcInstancePtr,
				      XAdcPs *XAdcPtr,
				      u16 IntrId );

/************************** Variable Definitions ****************************/

#ifndef TESTAPP_GEN
static XAdcPs XAdcInst; 	  	/* XADC driver instance */
static XScuGic InterruptController; 	/* Instance of the GIC driver */
#endif

/*
 * Shared variables used to test the callbacks.
 */
static volatile  int TemperatureIntr = FALSE; 	/* Temperature alarm intr */
static volatile  int VccpauxIntr = FALSE;	/* VCCPAUX alarm interrupt */

#ifndef TESTAPP_GEN
/****************************************************************************/
/**
*
* Main function that invokes the XADC Interrupt example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
*****************************************************************************/
int main(void)
{

	int Status;
	int Temp;

	/*
	 * Run the XAdc interrupt example, specify the parameters that
	 * are generated in xparameters.h.
	 */
	Status = XAdcIntrExample(&InterruptController,
				   &XAdcInst,
				   XADC_DEVICE_ID,
				   INTR_ID,
				   &Temp);
	if (Status != XST_SUCCESS) {
		printf("Interrupt example Failed\r\n");
		return XST_FAILURE;
	}
	printf("Successfully ran Interrupt example\r\n");
	return XST_SUCCESS;

}
#endif /* TESTAPP_GEN */

/****************************************************************************/
/**
*
* This function runs a test on the XADC/ADC device using the
* driver APIs.
*
* The function does the following tasks:
*	- Initiate the XADC/ADC device driver instance
*	- Run self-test on the device
*	- Reset the device
*	- Set up alarms for on-chip temperature and VCCPAUX
*	- Set up sequence registers to continuously monitor on-chip temperature
*	and VCCPAUX
*	- Setup interrupt system
*	- Enable interrupts
*	- Set up configuration registers to start the sequence
*	- Wait until temperature alarm interrupt or VCCPAUX alarm interrupt
*	occurs
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance.
* @param	XAdcInstPtr is a pointer to the XAdcPs driver Instance.
* @param	XAdcDeviceId is the XPAR_<SYSMON_ADC_instance>_DEVICE_ID value
*		from xparameters.h.
* @param	XAdcIntrId is
*		XPAR_<INTC_instance>_<SYSMON_ADC_instance>_VEC_ID value from
*		xparameters.h
* @param	Temp is an output parameter, it is a pointer through which the
*		current temperature value is returned to the main function.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		This function may never return if no interrupt occurs.
*
****************************************************************************/
int XAdcIntrExample(XScuGic *IntcInstancePtr, XAdcPs *XAdcInstPtr,
			u16 XAdcDeviceId, u16 XAdcIntrId, int *Temp)
{
	int Status;
	XAdcPs_Config *ConfigPtr;
	u16 TempData;
	u16 VccpauxData;
	u32 IntrStatus;

	/*
	 * Initialize the XAdc driver.
	 */
	ConfigPtr = XAdcPs_LookupConfig(XAdcDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XAdcPs_CfgInitialize(XAdcInstPtr, ConfigPtr, ConfigPtr->BaseAddress);

	/*
	 * Self Test the XADC/ADC device.
	 */
	Status = XAdcPs_SelfTest(XAdcInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set XADC in default mode so that alarms are inactive
	 */
	XAdcPs_SetSequencerMode(XAdcInstPtr, XADCPS_SEQ_MODE_SAFE);

	/* Disable all alarms */
	XAdcPs_SetAlarmEnables(XAdcInstPtr,0x0000);

	/*
	 * Set up Alarm threshold registers for the on-chip temperature and
	 * VCCPAUX High limit and lower limit so that the alarms occur.
	 */
	TempData = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_TEMP);
	XAdcPs_SetAlarmThreshold(XAdcInstPtr, XADCPS_ATR_TEMP_UPPER,(TempData-0x07FF));
	XAdcPs_SetAlarmThreshold(XAdcInstPtr, XADCPS_ATR_TEMP_LOWER,(TempData+0x07FF));

	VccpauxData = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_VCCPAUX);
	XAdcPs_SetAlarmThreshold(XAdcInstPtr, XADCPS_ATR_VCCPAUX_UPPER,(VccpauxData-0x07FF));
	XAdcPs_SetAlarmThreshold(XAdcInstPtr, XADCPS_ATR_VCCPAUX_LOWER,(VccpauxData+0x07FF));

	/*
	 * Setup the interrupt system.
	 */
	Status = XAdcSetupInterruptSystem(IntcInstancePtr,
					    XAdcInstPtr,
					    XAdcIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Clear any bits set in the Interrupt Status Register.
	 */
	IntrStatus = XAdcPs_IntrGetStatus(XAdcInstPtr);
	XAdcPs_IntrClear(XAdcInstPtr, IntrStatus);


	/*
	 * Enable Alarm 0 interrupt for on-chip temperature and Alarm 5
	 * interrupt for on-chip VCCPAUX.
	 */
	XAdcPs_IntrEnable(XAdcInstPtr,
			(XADCPS_INTX_ALM5_MASK | XADCPS_INTX_ALM0_MASK));

	/*
	 * Enable Alarm 0 for on-chip temperature and Alarm 5 for on-chip
	 * VCCPAUX in the Configuration Register 1.
	 */
	XAdcPs_SetAlarmEnables(XAdcInstPtr, (XADCPS_CFR1_ALM_VCCPAUX_MASK
											| XADCPS_CFR1_ALM_TEMP_MASK));


	XAdcPs_SetSequencerMode(XAdcInstPtr, XADCPS_SEQ_MODE_INDEPENDENT);


	/*
	 * Wait until an Alarm 0 or Alarm 2 interrupt occurs.
	 */
	while (1) {
		if (TemperatureIntr == TRUE) {
			/*
			 * Alarm 0 - Temperature alarm interrupt has occurred.
			 * The required processing should be put here.
			 */
			break;
		}
		if (VccpauxIntr == TRUE) {
					/*
					 * Alarm 5 - VccPAUX alarm interrupt has occurred.
					 * The required processing should be put here.
					 */
					break;
		}

	}


	/* Set back the ADC in default mode */

	XAdcPs_SetSequencerMode(XAdcInstPtr, XADCPS_SEQ_MODE_SAFE);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the Interrupt Service Routine for the XADC device.
* It will be called by the processor whenever an interrupt is asserted
* by the device.
*
* There are 8 different interrupts supported
*	- Over Temperature
*	- ALARM 0 to ALARM 7
*
* This function only handles ALARM 0 and ALARM 2 interrupts. User of this
* code may need to modify the code to meet needs of the application.
*
* @param	CallBackRef is the callback reference passed from the Interrupt
*		controller driver, which in our case is a pointer to the
*		driver instance.
*
* @return	None.
*
* @note		This function is called within interrupt context.
*
******************************************************************************/
static void XAdcInterruptHandler(void *CallBackRef)
{
	u32 IntrStatusValue;
	XAdcPs *XAdcPtr = (XAdcPs *)CallBackRef;

	/*
	 * Get the interrupt status from the device and check the value.
	 */

	IntrStatusValue = XAdcPs_IntrGetStatus(XAdcPtr);

	if (IntrStatusValue & XADCPS_INTX_ALM0_MASK) {
		/*
		 * Set Temperature interrupt flag so the code
		 * in application context can be aware of this interrupt.
		 */
		TemperatureIntr = TRUE;

		/* Disable Temperature interrupt */
		XAdcPs_IntrDisable(XAdcPtr,XADCPS_INTX_ALM0_MASK);

	}
	if (IntrStatusValue & XADCPS_INTX_ALM5_MASK) {
			/*
			 * Set Temperature interrupt flag so the code
			 * in application context can be aware of this interrupt.
			 */
		VccpauxIntr = TRUE;

		/* Disable VccPAUX interrupt */
		XAdcPs_IntrDisable(XAdcPtr,XADCPS_INTX_ALM5_MASK);
	}

	/*
	 * Clear all bits in Interrupt Status Register.
	 */
	XAdcPs_IntrClear(XAdcPtr, IntrStatusValue);
 }

/****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* XADC/ADC.  The function is application-specific since the actual
* system may or may not have an interrupt controller. The XADC/ADC
* device could be directly connected to a processor without an interrupt
* controller. The user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance.
* @param	XAdcPtr is a pointer to the driver instance for the System
* 		Monitor device which is going to be connected to the interrupt
*		controller.
* @param	IntrId is XPAR_<INTC_instance>_<SYSMON_ADC_instance>_VEC_ID
*		value from xparameters.h.
*
* @return	XST_SUCCESS if successful, or XST_FAILURE.
*
* @note		None.
*
*
****************************************************************************/
static int XAdcSetupInterruptSystem(XScuGic *IntcInstancePtr,
				      XAdcPs *XAdcPtr,
				      u16 IntrId )
{

	int Status;
	XScuGic_Config *IntcConfig; /* Instance of the interrupt controller */


	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, IntrId,
				(Xil_InterruptHandler)XAdcInterruptHandler,
				(void *)XAdcPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the XADC device.
	 */
	XScuGic_Enable(IntcInstancePtr, IntrId);

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler) XScuGic_InterruptHandler,
				IntcInstancePtr);
	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();


	return XST_SUCCESS;

}
