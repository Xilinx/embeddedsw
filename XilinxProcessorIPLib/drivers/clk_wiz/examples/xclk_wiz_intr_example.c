/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xclk_wiz_intr_example.c
*
* This file contains a design example using the XClk_Wiz driver with interrupts
* it will generate interrupt for clok glitch, clock overflow and underflow
* The user should have setup with 2 clocking wizard instances, one instance act
* as clocking monitor (Enable clock monitor in GUI), In another instance enable
* dynamic clock reconfiguration. In the present example XCLK_WIZ_DYN_DEVICE_ID
* assigned to clock wizard 1. Modify this value as per your dynamic clock
* reconfiguration Clocking wizard
*
* @note		This example requires an interrupt controller connected to the
*		processor and the MIPI CLK_WIZ  in the system.
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0 ram 2/12/16 Initial version for Clock Wizard
* 1.1 ms  01/23/17 Modified xil_printf statement in main function to
*                  ensure that "Successfully ran" and "Failed" strings are
*                  available in all examples. This is a fix for CR-965028.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xclk_wiz.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
 #include <stdio.h>
#else
 #include "xscugic.h"
 #include "xil_printf.h"
#endif


/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
*/
#define XCLK_WIZ_DEVICE_ID		XPAR_CLK_WIZ_0_DEVICE_ID

#ifdef XPAR_INTC_0_DEVICE_ID
 #define XINTC_CLK_WIZ_INTERRUPT_ID	XPAR_INTC_0_CLK_WIZ_0_VEC_ID
 #define XINTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
#else
 #define XINTC_CLK_WIZ_INTERRUPT_ID	XPAR_FABRIC_AXI_CLK_WIZ_0_INTERRUPT_INTR
 #define XINTC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif /* XPAR_INTC_0_DEVICE_ID */

/*
* change the XCLK_WIZ_DYN_DEVICE_ID value as per the Clock wizard
* whihc is setting as dynamic reconfiguration. In the present
* example clokc wizard 1 configured as clock wizard 1 as dynamic
* reconfigurable parameter
*/
#define XCLK_WIZ_DYN_DEVICE_ID		XPAR_CLK_WIZ_1_DEVICE_ID

/*
* The following constants are part of clock dynamic reconfiguration
* They are only defined here such that a user can easily change
* needed parameters
*/

#define CLK_LOCK			1

/*FIXED Value */
#define VCO_FREQ			600
#define CLK_WIZ_VCO_FACTOR		(VCO_FREQ * 10000)

 /*Input frequency in MHz */
#define DYNAMIC_INPUT_FREQ		100
#define DYNAMIC_INPUT_FREQ_FACTOR	(DYNAMIC_INPUT_FREQ * 10000)

/*
 * Output frequency in MHz. User need to change this value to
 * generate grater/lesser interrupt as per input frequency
 */
#define DYNAMIC_OUTPUT_FREQ		175
#define DYNAMIC_OUTPUT_FREQFACTOR	(DYNAMIC_OUTPUT_FREQ * 10000)

#define CLK_WIZ_RECONFIG_OUTPUT		DYNAMIC_OUTPUT_FREQ
#define CLK_FRAC_EN			1



#ifdef XPAR_INTC_0_DEVICE_ID
 #define XINTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
 #define INTC		XIntc
 #define INTC_HANDLER	XIntc_InterruptHandler
#else
 #define XINTC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID
 #define INTC		XScuGic
 #define INTC_HANDLER	XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 ClkWiz_IntrExample(INTC *IntcInstancePtr, u32 DeviceId);
int SetupInterruptSystem(INTC *IntcInstancePtr, XClk_Wiz *ClkWizPtr);
void XClk_Wiz_IntrHandler(void *InstancePtr);
void XClk_Wiz_InterruptEnable(XClk_Wiz *InstancePtr, u32 Mask);
int Clk_Wiz_Reconfig(XClk_Wiz_Config *CfgPtr_Dynamic);
int Wait_For_Lock(XClk_Wiz_Config *CfgPtr_Dynamic);

/* Interrupt helper functions */
void ClkWiz_ClkOutOfRangeEventHandler(void *CallBackRef, u32 Mask);
void ClkWiz_ClkGlitchEventHandler(void *CallBackRef, u32 Mask);
void ClkWiz_ClkStopEventHandler(void *CallBackRef, u32 Mask);

/************************** Variable Definitions *****************************/
XClk_Wiz ClkWiz_Mon;   /* The instance of the ClkWiz_Mon */
XClk_Wiz ClkWiz_Dynamic; /* The instance of the ClkWiz_Dynamic */
//XIntc InterruptController;  /* The instance of the Interrupt Controller */

volatile u8 Clk_Outof_Range_Flag = 1;
volatile u8 Clk_Glitch_Flag = 1;
volatile u8 Clk_Stop_Flag = 1;

INTC Intc;
/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This is the Wait_For_Lock function, it will wait for lock to settle change
* frequency value
*
* @param	CfgPtr_Dynamic provides pointer to clock wizard dynamic config
*
* @return
*		- Error 0 for pass scenario
*		- Error > 0 for failure scenario
*
* @note		None
*
******************************************************************************/
int Wait_For_Lock(XClk_Wiz_Config *CfgPtr_Dynamic)
{
	u32 Count = 0;
	u32 Error = 0;

	while(!(*(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK)) {
		if(Count == 10000) {
			Error++;
			break;
		}
		Count++;
        }
    return Error;
}

/******************************************************************************/
/**
*
* For Microblaze we use an assembly loop that is roughly the same regardless of
* optimization level, although caches and memory access time can make the delay
* vary.  Just keep in mind that after resetting or updating the PHY modes,
* the PHY typically needs time to recover.
*
* @param	Number of seconds to sleep
*
* @return	None
*
* @note		None
*
******************************************************************************/
void Delay(u32 Seconds)
{
#if defined (__MICROBLAZE__) || defined(__PPC__)
	static s32 WarningFlag = 0;

	/* If MB caches are disabled or do not exist, this delay loop could
	 * take minutes instead of seconds (e.g., 30x longer).  Print a warning
	 * message for the user (once).  If only MB had a built-in timer!
	 */
	if (((mfmsr() & 0x20) == 0) && (!WarningFlag)) {
		WarningFlag = 1;
	}

#define ITERS_PER_SEC   (XPAR_CPU_CORE_CLOCK_FREQ_HZ / 6)
    asm volatile ("\n"
			"1:               \n\t"
			"addik r7, r0, %0 \n\t"
			"2:               \n\t"
			"addik r7, r7, -1 \n\t"
			"bneid  r7, 2b    \n\t"
			"or  r0, r0, r0   \n\t"
			"bneid %1, 1b     \n\t"
			"addik %1, %1, -1 \n\t"
			:: "i"(ITERS_PER_SEC), "d" (Seconds));
#else
    sleep(Seconds);
#endif
}

/*****************************************************************************/
/**
*
* This is the Clk_Wiz_Reconfig function, it will reconfigure frequencies as
* per input array
*
* @param	CfgPtr_Dynamic provides pointer to clock wizard dynamic config
* @param	Findex provides the index for Frequency divide register
* @param	Sindex provides the index for Frequency phase register
*
* @return
*		-  Error 0 for pass scenario
*		-  Error > 0 for failure scenario
*
* @note	 None
*
******************************************************************************/
int Clk_Wiz_Reconfig(XClk_Wiz_Config *CfgPtr_Dynamic)
{
    u32 Count = 0;
    u32 Error = 0;
    u32 Fail  = 0;
    u32 Frac_en = 0;
    u32 Frac_divide = 0;
    u32 Divide = 0;
    float Freq = 0.0;

    Fail = Wait_For_Lock(CfgPtr_Dynamic);
    if(Fail) {
	Error++;
        xil_printf("\n ERROR: Clock is not locked for default frequency" \
	" : 0x%x\n\r", *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
     }

    /* SW reset applied */
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x00) = 0xA;

    if(*(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK) {
	Error++;
        xil_printf("\n ERROR: Clock is locked : 0x%x \t expected "\
	  "0x00\n\r", *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
    }

    /* Wait cycles after SW reset */
    for(Count = 0; Count < 2000; Count++);

    Fail = Wait_For_Lock(CfgPtr_Dynamic);
    if(Fail) {
	  Error++;
          xil_printf("\n ERROR: Clock is not locked after SW reset :"
	      "0x%x \t Expected  : 0x1\n\r",
	      *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
    }

    /* Calculation of Input Freq and Divide factors*/
    Freq = ((float) CLK_WIZ_VCO_FACTOR/ DYNAMIC_INPUT_FREQ_FACTOR);

    Divide = Freq;
    Freq = (float)(Freq - Divide);

    Frac_divide = Freq * 10000;

    if(Frac_divide % 10 > 5) {
	   Frac_divide = Frac_divide + 10;
    }
    Frac_divide = Frac_divide/10;

    if(Frac_divide > 1023 ) {
	   Frac_divide = Frac_divide / 10;
    }

    if(Frac_divide) {
	   /* if fraction part exists, Frac_en is shifted to 26
	    * for input Freq */
	   Frac_en = (CLK_FRAC_EN << 26);
    }
    else {
	   Frac_en = 0;
    }

    /* Configuring Multiply and Divide values */
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x200) = \
	Frac_en | (Frac_divide << 16) | (Divide << 8) | 0x01;
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x204) = 0x00;

    /* Calculation of Output Freq and Divide factors*/
    Freq = ((float) CLK_WIZ_VCO_FACTOR / DYNAMIC_OUTPUT_FREQFACTOR);

    Divide = Freq;
    Freq = (float)(Freq - Divide);

    Frac_divide = Freq * 10000;

    if(Frac_divide%10 > 5) {
	Frac_divide = Frac_divide + 10;
    }
    Frac_divide = Frac_divide / 10;

    if(Frac_divide > 1023 ) {
        Frac_divide = Frac_divide / 10;
    }

    if(Frac_divide) {
	/* if fraction part exists, Frac_en is shifted to 18 for output Freq */
	Frac_en = (CLK_FRAC_EN << 18);
    }
    else {
	Frac_en = 0;
    }

    /* Configuring Multiply and Divide values */
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x208) =
	    Frac_en | (Frac_divide << 8) | (Divide);
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x20C) = 0x00;

    /* Load Clock Configuration Register values */
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x25C) = 0x07;

    if(*(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK) {
	Error++;
        xil_printf("\n ERROR: Clock is locked : 0x%x \t expected "
	    "0x00\n\r", *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
     }

     /* Clock Configuration Registers are used for dynamic reconfiguration */
     *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x25C) = 0x02;

    Fail = Wait_For_Lock(CfgPtr_Dynamic);
    if(Fail) {
	Error++;
        xil_printf("\n ERROR: Clock is not locked : 0x%x \t Expected "\
	": 0x1\n\r", *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
    }
	return Error;
}

/*****************************************************************************/
/**
*
* This is the main function for XClk_Wiz interrupt example. If the
* ClkWiz_IntrExample function which sets up the system succeeds, this function
* will wait for the interrupts. Notify the events
*
* @param	None.
*
* @return
*		- XST_FAILURE if the interrupt example was unsuccessful.
*
* @note		Unless setup failed, main will never return since
*		ClkWiz_IntrExample is blocking (it is waiting on interrupts
*		for Hot-Plug-Detect (HPD) events.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main()
{
	u32 Status;

	xil_printf("------------------------------------------\n\r");
	xil_printf("CLK_WIZ Monitor interrupt example\n\r");
	xil_printf("(c) 2016 by Xilinx\n\r");
	xil_printf("-------------------------------------------\n\r\n\r");

	Status = ClkWiz_IntrExample(&Intc, XCLK_WIZ_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("CLK_WIZ Monitor interrupt example Failed");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran CLK_WIZ Monitor interrupt example\n\r");

	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the CLK_WIZ device. This function is application specific since the
* actual system may or may not have an interrupt controller. The CLK_WIZ
* could be directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param	ClkWizPtr contains a pointer to the instance of the CLK_WIZ
*		component which is going to be connected to the interrupt
*		controller.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
****************************************************************************/
int SetupInterruptSystem(INTC *IntcInstancePtr, XClk_Wiz *ClkWizPtr)
{

	int Status;


	/* Setup call back handlers */
	XClk_Wiz_SetCallBack(ClkWizPtr, XCLK_WIZ_HANDLER_CLK_OUTOF_RANGE,
				ClkWiz_ClkOutOfRangeEventHandler, ClkWizPtr);
	XClk_Wiz_SetCallBack(ClkWizPtr, XCLK_WIZ_HANDLER_CLK_GLITCH,
				ClkWiz_ClkGlitchEventHandler, ClkWizPtr);
	XClk_Wiz_SetCallBack(ClkWizPtr, XCLK_WIZ_HANDLER_CLK_STOP,
				ClkWiz_ClkStopEventHandler, ClkWizPtr);

#ifdef XPAR_INTC_0_DEVICE_ID
	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	Status = XIntc_Initialize(IntcInstancePtr, XINTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(IntcInstancePtr, XINTC_CLK_WIZ_INTERRUPT_ID,\
			   (XInterruptHandler)XClk_Wiz_IntrHandler, \
			   (void *)ClkWizPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the CLK_WIZ can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the CLK_WIZ.
	 */
	XIntc_Enable(IntcInstancePtr, XINTC_CLK_WIZ_INTERRUPT_ID);

#else
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(XINTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, XINTC_CLK_WIZ_INTERRUPT_ID,
					0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, XINTC_CLK_WIZ_INTERRUPT_ID,
			(XInterruptHandler)XClk_Wiz_IntrHandler, (void *)ClkWizPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Enable the interrupt for the GPIO device.*/
	XScuGic_Enable(IntcInstancePtr, XINTC_CLK_WIZ_INTERRUPT_ID);
#endif
	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, \
			 (Xil_ExceptionHandler)INTC_HANDLER, \
			 IntcInstancePtr);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the main entry point for the interrupt example using the
* XClk_Wiz driver. This function will set up the system with interrupts
* handlers.
*
* @param	DeviceId is the unique device ID of the CLK_WIZ
*		Subsystem core.
*
* @return
*		- XST_FAILURE if the system setup failed.
*		- XST_SUCCESS should never return since this function, if setup
*		was successful, is blocking.
*
* @note		If system setup was successful, this function is blocking in
*		order to illustrate interrupt handling taking place for HPD
*		events.
*
******************************************************************************/
u32 ClkWiz_IntrExample(INTC *IntcInstancePtr, u32 DeviceId)
{
	XClk_Wiz_Config *CfgPtr_Mon;
	XClk_Wiz_Config *CfgPtr_Dynamic;
	ULONG Exit_Count = 0;
	u32 Status = XST_SUCCESS;

	CfgPtr_Mon = XClk_Wiz_LookupConfig(DeviceId);
	if (!CfgPtr_Mon) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the CLK_WIZ driver so that it is ready to use.
	 */
	Status = XClk_Wiz_CfgInitialize(&ClkWiz_Mon, CfgPtr_Mon,
					CfgPtr_Mon->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Check the given clock wizard is enabled with clock monitor
	 * This test applicable only for clock monitor
	 */
	if(CfgPtr_Mon->EnableClkMon == 0) {
		xil_printf("Interrupt test only applicable for "
			"clock monitor\r\n");
		return XST_SUCCESS;
	}

	/*
	 * Get the CLK_WIZ Dynamic reconfiguration driver instance
	 */
	CfgPtr_Dynamic = XClk_Wiz_LookupConfig(XCLK_WIZ_DYN_DEVICE_ID);
	if (!CfgPtr_Dynamic) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the CLK_WIZ Dynamic reconfiguration driver
	 */
	Status = XClk_Wiz_CfgInitialize(&ClkWiz_Dynamic, CfgPtr_Dynamic,
		 CfgPtr_Dynamic->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the CLK_WIZ to the interrupt subsystem such that interrupts can
	 * occur. This function is application specific.
	 */

	Status = SetupInterruptSystem(IntcInstancePtr, &ClkWiz_Mon);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Calling Clock wizard dynamic reconfig */
	Clk_Wiz_Reconfig(CfgPtr_Dynamic);

	/* Enable interrupts after setup interrupt */
	XClk_Wiz_InterruptEnable(&ClkWiz_Mon, XCLK_WIZ_IER_ALLINTR_MASK);

	do {
		Delay(1);
		Exit_Count++;
		if(Exit_Count > 3) {
			xil_printf("ClKMon Interrupt test failed, " \
				"Please check design\r\n");
			return XST_FAILURE;
		}
	}
	while((Clk_Outof_Range_Flag == 1) && (Clk_Glitch_Flag == 1) \
		&& (Clk_Stop_Flag == 1));
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is called when a clock out of range is received by
* the CLK_WIZ  Subsystem core.
*
* @param	CallBackRef contains a callback reference from the driver.
*		In this case it is the instance pointer for the ClkWiz_Mon driver.
*
* @param	Mask of interrupt which caused this event
*
* @return	None
*
* @note		None
*
******************************************************************************/
void ClkWiz_ClkOutOfRangeEventHandler(void *CallBackRef, u32 Mask)
{
	if (Mask & XCLK_WIZ_ISR_CLK0_MAXFREQ_MASK) {
		xil_printf(" User Clock 0  frequency is greater "
			"than the specifications \r\n");
	}
	if (Mask & XCLK_WIZ_ISR_CLK1_MAXFREQ_MASK) {
		xil_printf(" User Clock 1  frequency is greater "
			"than the specifications \r\n");
	}
	if (Mask & XCLK_WIZ_ISR_CLK2_MAXFREQ_MASK) {
		xil_printf(" User Clock 2  frequency is greater "
			"than the specifications \r\n");
	}
	if (Mask & XCLK_WIZ_ISR_CLK3_MAXFREQ_MASK) {
		xil_printf(" User Clock 3  frequency is greater"
			"than the specifications \r\n");
	}
	if (Mask & XCLK_WIZ_ISR_CLK0_MINFREQ_MASK) {
		xil_printf(" User Clock 0  frequency is lesser "
			"than the specifications \r\n");
	}
	if (Mask & XCLK_WIZ_ISR_CLK1_MINFREQ_MASK) {
		xil_printf(" User Clock 1  frequency is lesser "
			"than the specifications \r\n");
	}
	if (Mask & XCLK_WIZ_ISR_CLK2_MINFREQ_MASK) {
		xil_printf(" User Clock 2  frequency is lesser "
			"than the specifications \r\n");
	}
	if (Mask & XCLK_WIZ_ISR_CLK3_MINFREQ_MASK) {
		xil_printf(" User Clock 3  frequency is lesser "
			"than the specifications \r\n");
	}
	Clk_Outof_Range_Flag = 0;
}

/*****************************************************************************/
/**
*
* This function is called when a clock glitch event is received by
* the CLK_WIZ Subsystem core.
*
* @param	CallBackRef contains a callback reference from the driver.
*		In this case it is the instance pointer for the ClkWiz_Mon driver.
*
* @param	Mask of interrupt which caused this event
*
* @return	None
*
* @note		None
*
******************************************************************************/
void ClkWiz_ClkGlitchEventHandler(void *CallBackRef, u32 Mask)
{
	if (Mask & XCLK_WIZ_ISR_CLK0_GLITCH_MASK) {
		xil_printf("Glitch occurred in the user clock 0 \r\n");
	}
	if (Mask & XCLK_WIZ_ISR_CLK1_GLITCH_MASK) {
		xil_printf("Glitch occurred in the user clock 1 \r\n");
	}
	if (Mask & XCLK_WIZ_ISR_CLK2_GLITCH_MASK) {
		xil_printf("Glitch occurred in the user clock 2 \r\n");
	}
	if (Mask & XCLK_WIZ_ISR_CLK3_GLITCH_MASK) {
		xil_printf("Glitch occurred in the user clock 3 \r\n");
	}
	Clk_Glitch_Flag = 0;
}

/*****************************************************************************/
/**
*
* This function is called when a clock stop event is received by
* the CLK_WIZ Subsystem core.
*
* @param	CallBackRef is a pointer to the XClk_Wiz instance.
*
* @param	Mask of interrupt which caused this event
*
* @return	None
*
* @note		None
*
******************************************************************************/
void ClkWiz_ClkStopEventHandler(void *CallBackRef, u32 Mask)
{
	if (Mask & XCLK_WIZ_ISR_CLK0_STOP_MASK) {
		xil_printf("Clock stop on User clock 0\r\n");
	}
	if (Mask & XCLK_WIZ_ISR_CLK1_STOP_MASK) {
		xil_printf("Clock stop on User clock 1\r\n");
	}
	if (Mask & XCLK_WIZ_ISR_CLK2_STOP_MASK) {
		xil_printf("Clock stop on User clock 2\r\n");
	}
	if (Mask & XCLK_WIZ_ISR_CLK3_STOP_MASK) {
		xil_printf("Clock stop on User clock 3\r\n");
	}
	Clk_Stop_Flag = 0;
}
