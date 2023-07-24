/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xsysmon_single_ch_intr_example.c
*
* This file contains a design example using the driver functions of the
* System Monitor/ADC driver.
* This example here shows the usage of the driver/device in single channel
* interrupt mode to handle End of Conversion (EOC) and VCCINT alarm interrupts.
*
*
* @note
*
* This code assumes that no Operating System is being used.
*
* The value of the on-chip Vccint voltage is read from the device and then the
* alarm thresholds are set in such a manner that the alarm occurs.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a sv     09/04/07 First release
* 4.00a ktn    10/22/09 Updated the example to use HAL Processor APIs/macros.
*		        Updated the example to use macros that have been
*		        renamed to remove _m from the name of the macro.
* 7.3   vns    15/04/16 Updated example to support ZynqMP
*       ms     01/23/17 Added xil_printf statement in main function to
*                       ensure that "Successfully ran" and "Failed" strings
*                       are available in all examples. This is a fix for
*                       CR-965028.
* 7.8   cog    07/20/23 Added support for SDT flow
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xsysmon.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xil_exception.h"
#include "xil_printf.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif

/************************** Constant Definitions ****************************/
#ifndef SDT
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define SYSMON_DEVICE_ID	XPAR_SYSMON_0_DEVICE_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID	/* Interrupt Controller */
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define INTR_ID			XPAR_INTC_0_SYSMON_0_VEC_ID
#else	/* SCUGIC Interrupt Controller */
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTR_ID		XPAR_FABRIC_SYSTEM_MANAGEMENT_WIZ_0_IP2INTC_IRPT_INTR
#endif /* XPAR_INTC_0_DEVICE_ID */

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC		XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif
#else
#define SYSMON_DEVICE_ID	0
#define INTC_DEVICE_ID		0
#if (XSM_IP_TYPE == XADC)
#define INTR_ID			(32U + 29U)
#else
#define INTR_ID			(32U + 89U)
#endif
#define INTC		XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

int SysMonSingleChannelIntrExample(INTC* IntcInstancePtr,
			XSysMon* SysMonInstPtr,
			u16 SysMonDeviceId,
			u16 SysMonIntrId);


static void SysMonInterruptHandler(void *CallBackRef);

static int SysMonSetupInterruptSystem(INTC* IntcInstancePtr,
				      XSysMon *SysMonPtr,
				      u16 IntrId );

/************************** Variable Definitions ****************************/

#ifndef TESTAPP_GEN
static XSysMon SysMonInst; 	  /* System Monitor driver instance */
static INTC InterruptController; /* Instance of the XIntc driver. */
#endif

/*
 * Shared variables used to test the callbacks.
 */
volatile static int EocFlag = FALSE;	  	/* EOC interrupt */
volatile static int VccintIntr = FALSE;	  	/* VCCINT alarm interrupt */


#ifndef TESTAPP_GEN
/****************************************************************************/
/**
*
* Main function that invokes the Single Channel Interrupt example.
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

	/*
	 * Run the SysMonitor interrupt example, specify the parameters that
	 * are generated in xparameters.h.
	 */
	Status = SysMonSingleChannelIntrExample(&InterruptController,
				   &SysMonInst,
				   SYSMON_DEVICE_ID,
				   INTR_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Sysmon single ch interrupt Example Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran Sysmon single ch interrupt Example\r\n");
	return XST_SUCCESS;

}
#endif /* TESTAPP_GEN */

/****************************************************************************/
/**
*
* This function runs a test on the System Monitor/ADC device using the
* driver APIs.
*
* The function does the following tasks:
*	- Initiate the System Monitor/ADC device driver instance
*	- Run self-test on the device
*	- Reset the device
*	- Set up alarm for VCCINT
*	- Set up the configuration registers for single channel continuous mode
*	for VCCINT channel
*	- Setup interrupt system
*	- Enable interrupts
*	- Wait until the VCCINT alarm interrupt occurs
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance.
* @param	SysMonInstPtr is a pointer to the XSysMon driver Instance.
* @param	SysMonDeviceId is the XPAR_<SYSMON_ADC_instance>_DEVICE_ID value
*		from xparameters.h.
* @param	SysMonIntrId is
*		XPAR_<INTC_instance>_<SYSMON_ADC_instance>_VEC_ID
*		value from xparameters.h
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		This function may never return if no interrupt occurs.
*
****************************************************************************/
int SysMonSingleChannelIntrExample(INTC* IntcInstancePtr,
					XSysMon* SysMonInstPtr,
					u16 SysMonDeviceId,
					u16 SysMonIntrId)
{
	int Status;
	XSysMon_Config *ConfigPtr;
	u16 VccintData;
	u32 IntrStatus;

	/*
	 * Initialize the SysMon driver.
	 */
	ConfigPtr = XSysMon_LookupConfig(SysMonDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XSysMon_CfgInitialize(SysMonInstPtr, ConfigPtr, ConfigPtr->BaseAddress);

	/*
	 * Self Test the System Monitor/ADC device.
	 */
	Status = XSysMon_SelfTest(SysMonInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the ADCCLK frequency equal to 1/32 of System clock for the System
	 * Monitor/ADC in the Configuration Register 2.
	 */
	XSysMon_SetAdcClkDivisor(SysMonInstPtr, 32);

	/*
	 * Set the sequencer in Single channel mode.
	 */
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_SINGCHAN);

	/*
	 * Set the configuration registers for single channel continuous mode
	 * of operation for the VCCINT channel.
	 */
	Status=  XSysMon_SetSingleChParams(SysMonInstPtr, XSM_CH_VCCINT,
						FALSE, FALSE, FALSE);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Disable all the alarms in the Configuration Register 1.
	 */
	XSysMon_SetAlarmEnables(SysMonInstPtr, 0x0);

	/*
	 * Set up Alarm threshold registers for the VCCINT
	 * High limit and lower limit so that the alarm does not occur.
	 */
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCINT_UPPER, 0xFFFF);
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCINT_LOWER, 0x0);


	/*
	 * Setup the interrupt system.
	 */
	Status = SysMonSetupInterruptSystem(IntcInstancePtr,
					    SysMonInstPtr,
					    SysMonIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Clear any bits set in the Interrupt Status Register.
	 */
	IntrStatus = XSysMon_IntrGetStatus(SysMonInstPtr);
	XSysMon_IntrClear(SysMonInstPtr, IntrStatus);


	/*
	 * Enable EOC interrupt and Alarm 1 interrupt for on-chip VCCINT.
	 */
	XSysMon_IntrEnable(SysMonInstPtr, XSM_IPIXR_EOC_MASK |
						XSM_IPIXR_VCCINT_MASK);

	/*
	 * Enable global interrupt of System Monitor.
	 */
	XSysMon_IntrGlobalEnable(SysMonInstPtr);


	/*
	 * Wait till the End of Conversion occurs.
	 */
	EocFlag = FALSE; 		/* Clear the EOC Flag */
	while (EocFlag != TRUE);


	/*
	 * Read the ADC converted Data from the data registers for VCCINT.
	 */
	VccintData = XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_VCCINT);


	/*
	 * Set up Alarm threshold registers for the VCCINT
	 * High limit and lower limit so that the alarm occurs.
	 */
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCINT_UPPER,
						VccintData - 0x007F);
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCINT_LOWER,
						VccintData - 0x007F);


	VccintIntr = FALSE;	/* Clear the flag */


	/*
	 * Enable Alarm 1 for VCCINT
	 */
	XSysMon_SetAlarmEnables(SysMonInstPtr, XSM_CFR1_ALM_VCCINT_MASK);

	/*
	 * Wait until an Alarm 1 interrupt occurs.
	 */
	while (1) {

		if (VccintIntr == TRUE) {
			/*
			 * Alarm 1 - VCCINT alarm interrupt has occurred.
			 * The required processing should be put here.
			 */
			break;
		}
	}

	/*
	 * Disable global interrupt of System Monitor.
	 */
	XSysMon_IntrGlobalDisable(SysMonInstPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the Interrupt Service Routine for the System Monitor device.
* It will be called by the processor whenever an interrupt is asserted
* by the device.
*
* There are 8 different interrupt types supported
*	- Over Temperature
*	- ALARM 0
*	- ALARM 1
*	- ALARM 2
*	- End of Sequence
*	- End of Conversion
*	- Over Temperature DeActivated
*	- ALARM 0 DeActivated
*
* This function only handles the EOC interrupt and ALARM 1 interrupt.
* User of this code should modify the code to meet needs of the application.
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
static void SysMonInterruptHandler(void *CallBackRef)
{
	u32 IntrStatusValue;
	XSysMon *SysMonPtr = (XSysMon *)CallBackRef;

	/*
	 * Get the interrupt status from the device and check the value.
	 */
	IntrStatusValue = XSysMon_IntrGetStatus(SysMonPtr);

	if (IntrStatusValue & XSM_IPIXR_EOC_MASK) {
		/*
		 * Set End of Conversion  interrupt flag so the code
		 * in application context can be aware of this interrupt.
		 */
		EocFlag = TRUE;
		XSysMon_GetStatus(SysMonPtr); /* Clear the latched status */
	}

	if (IntrStatusValue & XSM_IPIXR_VCCINT_MASK) {
		/*
		 * Set VCCINT interrupt flag so the code in application context
		 * can be aware of this interrupt.
		 */
		VccintIntr = TRUE;
	}

	/*
	 * Clear all bits in Interrupt Status Register.
	 */
	XSysMon_IntrClear(SysMonPtr, IntrStatusValue);
 }

/****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* System Monitor/ADC.  The function is application-specific since the actual
* system may or may not have an interrupt controller. The System Monitor/ADC
* device could be directly connected to a processor without an interrupt
* controller. The user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance
* @param	SysMonPtr is a pointer to the driver instance for the System
* 		Monitor device which is going to be connected to the interrupt
*		controller
* @param	IntrId is XPAR_<INTC_instance>_<SYSMON_ADC_instance>_VEC_ID
*		value from xparameters.h
*
* @return	XST_SUCCESS if successful, or XST_FAILURE.
*
* @note		None.
*
*
****************************************************************************/
static int SysMonSetupInterruptSystem(INTC* IntcInstancePtr,
				      XSysMon *SysMonPtr,
				      u16 IntrId )
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID
#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif
	/*
	 * Connect the handler that will be called when an interrupt
	 * for the device occurs, the handler defined above performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(IntcInstancePtr,
				IntrId,
				(XInterruptHandler) SysMonInterruptHandler,
				SysMonPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts. Specify real mode so that the System
	 * Monitor/ACD device can cause interrupts through the interrupt
	 * controller.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif
	/*
	 * Enable the interrupt for the System Monitor/ADC device
	 */
	XIntc_Enable(IntcInstancePtr, IntrId);
#else /* SCUGIC */

#ifndef TESTAPP_GEN
	XScuGic_Config *IntcConfig;
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
#endif

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, IntrId,
					0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, IntrId,
				 (Xil_ExceptionHandler)SysMonInterruptHandler,
				 SysMonPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Sysmon device.
	 */
	XScuGic_Enable(IntcInstancePtr, IntrId);
#endif
#ifndef TESTAPP_GEN
	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
					(Xil_ExceptionHandler) INTC_HANDLER,
				IntcInstancePtr);
	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

#endif /* TESTAPP_GEN */

	return XST_SUCCESS;
}
