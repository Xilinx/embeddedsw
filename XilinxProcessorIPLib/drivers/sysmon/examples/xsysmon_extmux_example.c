/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xsysmon_extmux_example.c
*
* This file contains a design example using the driver functions
* of the System Monitor/ADC driver. This example shows the usage of the
* driver/device in interrupt mode with external mux and XADC in Simulateneous
* Sequencer mode. This example hasn't been tested with the analog inputs
* connected through external mux. It is provided to illustrate the usage of
* external mux.
*
* @note
*
* This code assumes that no Operating System is being used.
*
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------
* 5.00a sdm  08/30/11 First release
* 5.03a bss  04/25/13 Modified SysMonIntrExample function to set
*		      Sequencer Mode as Safe mode instead of Single
*		      channel mode before configuring Sequencer registers.
*		      CR #703729
* 7.3   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 7.8   cog  07/20/23 Added support for SDT flow
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xsysmon.h"
#include "xstatus.h"
#ifndef SDT
#include "xintc.h"
#else
#include "xscugic.h"
#endif
#include "stdio.h"
#include "xil_exception.h"
#include "xil_printf.h"

/************************** Constant Definitions ****************************/
#ifndef SDT
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SYSMON_DEVICE_ID	XPAR_SYSMON_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define INTR_ID			XPAR_INTC_0_SYSMON_0_VEC_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define SYSMON_DEVICE_ID	0
#define INTC_DEVICE_ID		0
#define INTC		XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#if (XSM_IP_TYPE == XADC)
#define INTR_ID			(32U + 29U)
#else
#define INTR_ID			(32U + 89U)
#endif
#endif
#define printf xil_printf 	/* Small foot-print printf function */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

static int SysMonIntrExample(INTC* IntcInstPtr,
			XSysMon* SysMonInstPtr,
			u16 SysMonDeviceId,
			u16 SysMonIntrId);


static void SysMonInterruptHandler(void *CallBackRef);

static int SysMonSetupInterruptSystem(INTC* IntcInstancePtr,
				      XSysMon *SysMonPtr,
				      u16 IntrId );

/************************** Variable Definitions ****************************/

static XSysMon SysMonInst;		/* System Monitor driver instance */
static INTC InterruptController;			/* Instance of the XIntc/SCUGIC driver */

volatile static int EosFlag = FALSE;	/* EOS interrupt */

/****************************************************************************/
/**
*
* Main function that invokes the Interrupt example.
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
	Status = SysMonIntrExample(&InterruptController, &SysMonInst,
				   SYSMON_DEVICE_ID, INTR_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Sysmon extmux Example Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran Sysmon extmux Example\r\n");
	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function runs a test on the XADC device using the driver APIs.
*
* The function does the following tasks:
*	- Initiate the XADC device driver instance
*	- Run self-test on the device
*	- Reset the device
*	- Set up sequencer registers to continuously monitor the auxiliary
*	  channel pairs avaibale in XADC
*	- Setup interrupt system
*	- Enable interrupts
*	- Set up configuration registers to start the sequencer in simultaneous
*	  sampling mode
*	- Wait until End of sequence interrupt occurs and read the conversion
*	  data
*
* @param	IntcInstPtr is a pointer to the Interrupt Controller
*		driver Instance.
* @param	SysMonInstPtr is a pointer to the XSysMon driver Instance.
* @param	SysMonDeviceId is the XPAR_<SYSMON_ADC_instance>_DEVICE_ID value
*		from xparameters.h.
* @param	SysMonIntrId is
*		XPAR_<INTC_instance>_<SYSMON_ADC_instance>_VEC_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		This function may never return if no interrupt occurs.
*
****************************************************************************/
static int SysMonIntrExample(INTC* IntcInstPtr, XSysMon* SysMonInstPtr,
			     u16 SysMonDeviceId, u16 SysMonIntrId)
{
	int Status;
	XSysMon_Config *ConfigPtr;
	u32 IntrStatus;
	u32 IntrEnable;
	u32 AdcData[8];
	int Index;

	printf("\r\nXADC External MUX Example. \r\n");

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
	 * Disable the Channel Sequencer before configuring the Sequencer.
	 */
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_SAFE);

	/*
	 * Enable the following auxiliary channel pairs in the Sequencer
	 * registers:
	 *	- Auxiliary Channel 0 & 8
	 *	- Auxiliary Channel 1 & 9
	 *	- Auxiliary Channel 2 & 10
	 *	- Auxiliary Channel 3 & 11
	 */
	Status = XSysMon_SetSeqChEnables(SysMonInstPtr, 0x0F0000);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the ADCCLK frequency equal to 1/32 of System clock for the System
	 * Monitor/ADC in the Configuration Register 2.
	 */
	XSysMon_SetAdcClkDivisor(SysMonInstPtr, 32);

	/*
	 * Setup the interrupt system.
	 */
	Status = SysMonSetupInterruptSystem(IntcInstPtr, SysMonInstPtr,
					    SysMonIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable global interrupt of System Monitor.
	 */
	XSysMon_IntrGlobalEnable(SysMonInstPtr);

	/*
	 * Clear any pending interrupts.
	 */
	IntrStatus = XSysMon_IntrGetStatus(SysMonInstPtr);
	XSysMon_IntrClear(SysMonInstPtr, IntrStatus);

	/*
	 * Enable EOS interrupts.
	 */
	XSysMon_IntrEnable(SysMonInstPtr, XSM_IPIXR_EOS_MASK);
	IntrEnable = XSysMon_IntrGetEnabled(SysMonInstPtr);
	if ((IntrEnable & XSM_IPIXR_EOS_MASK) != XSM_IPIXR_EOS_MASK) {
		return XST_FAILURE;
	}

	/*
	 * Enable external Mux and connect to Aux CH0 and Aux CH8.
	 */
	XSysMon_SetExtenalMux(SysMonInstPtr, 0x10); /* 0b'10000 to CH[4:0] */

	/*
	 * Enable simultaneous sequencer mode.
	 */
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_SIMUL);

	while (EosFlag != TRUE);
	EosFlag = FALSE;			/* Clear the Flag */
	XSysMon_GetStatus(SysMonInstPtr);	/* Clear the latched status */

	/*
	 * Read the ADC converted Data from the data registers.
	 */
	/* Read ADC data for channels 0 - 3 */
	for (Index = 0; Index < 4; Index++) {
		AdcData[Index] = XSysMon_GetAdcData(SysMonInstPtr,
					XSM_CH_AUX_MIN + Index);
	}

	/* Read ADC data for channels 8 - 11 */
	for (Index = 0; Index < 4; Index++) {
		AdcData[Index + 4] = XSysMon_GetAdcData(SysMonInstPtr,
					XSM_CH_AUX_MIN + Index + 8);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the Interrupt Service Routine for the XADC device.
* It will be called by the processor when an interrupt is asserted by the
* device.
*
* There are 10 different interrupts supported
*	- Over Temperature
*	- ALARM 0
*	- ALARM 1
*	- ALARM 2
*	- End of Sequence
*	- End of Conversion
*	- JTAG Locked
*	- JATG Modified
*	- Over Temperature deactivate
*	- ALARM 0 deactivate
*
* This function only handles EOS interrupts.
* User of this code may need to modify the code to meet the needs of the
* application.
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

	if (IntrStatusValue & XSM_IPIXR_EOS_MASK) {
		/*
		 * Set End of Conversion  interrupt flag so the code
		 * in application context can be aware of this interrupt.
		 */
		EosFlag = TRUE;
		XSysMon_GetStatus(SysMonPtr); /* Clear the latched status */
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
* @param	IntcInstPtr is a pointer to the Interrupt Controller driver
*		Instance.
* @param	SysMonPtr is a pointer to the driver instance for the System
* 		Monitor device which is going to be connected to the interrupt
*		controller.
* @param	IntrId is XPAR_<INTC_instance>_<SYSMON_ADC_instance>_VEC_ID
*		value from xparameters.h
*
* @return	XST_SUCCESS if successful, or XST_FAILURE.
*
* @note		None.
*
*
****************************************************************************/
static int SysMonSetupInterruptSystem(INTC* IntcInstPtr, XSysMon *SysMonPtr,
				      u16 IntrId )
{
	int Status;

#ifndef SDT
	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 */
	Status = XIntc_Initialize(IntcInstPtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the handler that will be called when an interrupt
	 * for the device occurs, the handler defined above performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(IntcInstPtr,
				IntrId,
				(XInterruptHandler) SysMonInterruptHandler,
				SysMonPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts. Specify real mode so that the System
	 * Monitor/ACD device can cause interrupts through the interrupt
	 * controller.
	 */
	Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the System Monitor/ADC device.
	 */
	XIntc_Enable(IntcInstPtr, IntrId);
#else /* SCUGIC */

	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstPtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(IntcInstPtr, IntrId,
					0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstPtr, IntrId,
				 (Xil_ExceptionHandler)SysMonInterruptHandler,
				 SysMonPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Sysmon device.
	 */
	XScuGic_Enable(IntcInstPtr, IntrId);
#endif

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler) INTC_HANDLER,
				IntcInstPtr);

	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
