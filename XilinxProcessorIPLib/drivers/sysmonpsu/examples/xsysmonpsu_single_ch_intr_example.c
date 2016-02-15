/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xsysmonpsu_single_ch_intr_example.c
*
* This file contains a design example using the driver functions of the
* System Monitor driver.
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
* 1.0   kvn    12/15/15 First release
*              02/15/16 Corrected order of Enabling / Disabling of
*                       interrupts.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xsysmonpsu.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xscugic.h"
#include "xil_exception.h"


/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#define SYSMON_DEVICE_ID	XPAR_XSYSMONPSU_0_DEVICE_ID
#define SCUGIC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTR_ID			XPAR_XSYSMONPSU_INTR


/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

int SysMonPsuSingleChannelIntrExample(XScuGic* XScuGicInstancePtr,
			XSysMonPsu* SysMonInstPtr,
			u16 SysMonDeviceId,
			u16 SysMonIntrId);


static void SysMonPsuInterruptHandler(void *CallBackRef);

static int SysMonPsuSetupInterruptSystem(XScuGic* XScuGicInstancePtr,
				      XSysMonPsu *SysMonPtr,
				      u16 IntrId );

/************************** Variable Definitions ****************************/

static XSysMonPsu SysMonInst; 	  /* System Monitor driver instance */
static XScuGic InterruptController; /* Instance of the XScuGic driver. */

/* Shared variables used to test the callbacks. */
volatile static int EocFlag = FALSE;	  	/* EOC interrupt */
volatile static int VccintIntr = FALSE;	  	/* VCCINT alarm interrupt */


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
	Status = SysMonPsuSingleChannelIntrExample(&InterruptController,
				   &SysMonInst,
				   SYSMON_DEVICE_ID,
				   INTR_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Sysmon Single Channel Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Sysmon Single Channel Interrupt Example Test\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a test on the System Monitor device using the
* driver APIs.
*
* The function does the following tasks:
*	- Initiate the System Monitor device driver instance
*	- Run self-test on the device
*	- Reset the device
*	- Set up alarm for VCCINT
*	- Set up the configuration registers for single channel continuous mode
*	for VCCINT channel
*	- Setup interrupt system
*	- Enable interrupts
*	- Wait until the VCCINT alarm interrupt occurs
*
* @param	XScuGicInstancePtr is a pointer to the Interrupt Controller
*		driver Instance.
* @param	SysMonInstPtr is a pointer to the XSysMon driver Instance.
* @param	SysMonDeviceId is the XPAR_<SYSMON_instance>_DEVICE_ID value
*		from xparameters.h.
* @param	SysMonIntrId is
*		XPAR_<SYSMON_instance>_VEC_ID
*		value from xparameters_ps.h
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		This function may never return if no interrupt occurs.
*
****************************************************************************/
int SysMonPsuSingleChannelIntrExample(XScuGic* XScuGicInstancePtr,
					XSysMonPsu* SysMonInstPtr,
					u16 SysMonDeviceId,
					u16 SysMonIntrId)
{
	int Status;
	XSysMonPsu_Config *ConfigPtr;
	u16 VccintData;
	u64 IntrStatus;

	/* Initialize the SysMon driver. */
	ConfigPtr = XSysMonPsu_LookupConfig(SysMonDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XSysMonPsu_CfgInitialize(SysMonInstPtr, ConfigPtr, ConfigPtr->BaseAddress);

	/* Self Test the System Monitor device. */
	Status = XSysMonPsu_SelfTest(SysMonInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the ADCCLK frequency equal to 1/32 of System clock for the System
	 * Monitor in the Configuration Register 2.
	 */
	XSysMonPsu_SetAdcClkDivisor(SysMonInstPtr, 32, XSYSMON_PS);

	/* Set the sequencer in Single channel mode. */
	XSysMonPsu_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_SINGCHAN, XSYSMON_PS);

	/*
	 * Set the configuration registers for single channel continuous mode
	 * of operation for the VCCINT channel.
	 */
	Status=  XSysMonPsu_SetSingleChParams(SysMonInstPtr, XSM_CH_SUPPLY1,
						FALSE, FALSE, FALSE, XSYSMON_PS);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Disable all the alarms in the Configuration Register 1. */
	XSysMonPsu_SetAlarmEnables(SysMonInstPtr, 0x0, XSYSMON_PS);

	/*
	 * Set up Alarm threshold registers for the VCCINT
	 * High limit and lower limit so that the alarm does not occur.
	 */
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP1_UPPER, 0xFFFF, XSYSMON_PS);
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP1_LOWER, 0x0, XSYSMON_PS);


	/* Setup the interrupt system. */
	Status = SysMonPsuSetupInterruptSystem(XScuGicInstancePtr,
					    SysMonInstPtr,
					    SysMonIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/* Clear any bits set in the Interrupt Status Register. */
	IntrStatus = XSysMonPsu_IntrGetStatus(SysMonInstPtr);
	XSysMonPsu_IntrClear(SysMonInstPtr, IntrStatus);


	/* Enable EOC interrupt and Alarm 1 interrupt for on-chip VCCINT. */
	XSysMonPsu_IntrEnable(SysMonInstPtr,
			((u64)XSYSMONPSU_IER_1_EOC_MASK << 32) |
			XSYSMONPSU_IER_0_PS_ALM_1_MASK);


	/* Wait till the End of Conversion occurs. */
	EocFlag = FALSE; 		/* Clear the EOC Flag */
	while (EocFlag != TRUE);

	/* Read the ADC converted Data from the data registers for VCCINT. */
	VccintData = XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_SUPPLY1, XSYSMON_PS);

	/* Disable EOC interrupt and Alarm 1 interrupt for on-chip VCCINT. */
	XSysMonPsu_IntrDisable(SysMonInstPtr,
				((u64)XSYSMONPSU_IER_1_EOC_MASK << 32) |
				XSYSMONPSU_IER_0_PS_ALM_1_MASK);

	/*
	 * Set up Alarm threshold registers for the VCCINT
	 * High limit and lower limit so that the alarm occurs.
	 */
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP1_UPPER,
						VccintData - 0x007F, XSYSMON_PS);
	XSysMonPsu_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_SUP1_LOWER,
						VccintData - 0x007F, XSYSMON_PS);

	VccintIntr = FALSE;	/* Clear the flag */

	/* Enable Alarm 1 for VCCINT */
	XSysMonPsu_SetAlarmEnables(SysMonInstPtr, XSYSMONPSU_CFR_REG1_ALRM_SUP1_MASK,
			XSYSMON_PS);

	/* Enable EOC interrupt and Alarm 1 interrupt for on-chip VCCINT. */
	XSysMonPsu_IntrEnable(SysMonInstPtr,
			((u64)XSYSMONPSU_IER_1_EOC_MASK << 32) |
			XSYSMONPSU_IER_0_PS_ALM_1_MASK);

	/* Wait until an Alarm 1 interrupt occurs. */
	while (1) {

		if (VccintIntr == TRUE) {
			/*
			 * Alarm 1 - VCCINT alarm interrupt has occurred.
			 * The required processing should be put here.
			 */
			break;
		}
	}


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
static void SysMonPsuInterruptHandler(void *CallBackRef)
{
	u64 IntrStatusValue;
	XSysMonPsu *SysMonPtr = (XSysMonPsu *)CallBackRef;

	/* Get the interrupt status from the device and check the value. */
	IntrStatusValue = XSysMonPsu_IntrGetStatus(SysMonPtr);

	if (IntrStatusValue & ((u64)XSYSMONPSU_IER_1_EOC_MASK << 32)) {
		/*
		 * Set End of Conversion  interrupt flag so the code
		 * in application context can be aware of this interrupt.
		 */
		EocFlag = TRUE;
		/*XSysMon_GetStatus(SysMonPtr);*/ /* TODO Clear the latched status */
	}

	if (IntrStatusValue & XSYSMONPSU_IER_0_PS_ALM_1_MASK) {
		/*
		 * Set VCCINT interrupt flag so the code in application context
		 * can be aware of this interrupt.
		 */
		VccintIntr = TRUE;
	}

	/* Clear all bits in Interrupt Status Register. */
	XSysMonPsu_IntrClear(SysMonPtr, IntrStatusValue);

	/* Disable EOC interrupt and Alarm 1 interrupt for on-chip VCCINT. */
	XSysMonPsu_IntrDisable(SysMonPtr,
			((u64)XSYSMONPSU_IER_1_EOC_MASK << 32) |
			XSYSMONPSU_IER_0_PS_ALM_1_MASK);

 }

/****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* System Monitor.  The function is application-specific since the actual
* system may or may not have an interrupt controller. The System Monitor
* device could be directly connected to a processor without an interrupt
* controller. The user should modify this function to fit the application.
*
* @param	XScuGicInstancePtr is a pointer to the Interrupt Controller
*		driver Instance
* @param	SysMonPtr is a pointer to the driver instance for the System
* 		Monitor device which is going to be connected to the interrupt
*		controller
* @param	IntrId is XPAR_<SYSMON_instance>_VEC_ID
*		value from xparameters.h
*
* @return	XST_SUCCESS if successful, or XST_FAILURE.
*
* @note		None.
*
*
****************************************************************************/
static int SysMonPsuSetupInterruptSystem(XScuGic* XScuGicInstancePtr,
				      XSysMonPsu *SysMonPtr,
				      u16 IntrId )
{
	int Status;

	XScuGic_Config *XScuGicConfig; /* Config for interrupt controller */

	/* Initialize the interrupt controller driver */
	XScuGicConfig = XScuGic_LookupConfig(SCUGIC_DEVICE_ID);
	if (NULL == XScuGicConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(XScuGicInstancePtr, XScuGicConfig,
					XScuGicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect the interrupt controller interrupt handler to the
	 * hardware interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler) XScuGic_InterruptHandler,
				XScuGicInstancePtr);


	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler
	 * performs the specific interrupt processing for the device
	 */
	Status = XScuGic_Connect(XScuGicInstancePtr, IntrId,
				  (Xil_ExceptionHandler) SysMonPsuInterruptHandler,
				  (void *) SysMonPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Enable the interrupt for the device */
	XScuGic_Enable(XScuGicInstancePtr, IntrId);


	/* Enable interrupts */
	 Xil_ExceptionEnable();

	return XST_SUCCESS;
}
