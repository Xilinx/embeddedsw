/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xwdttb_winwdt_intr_example.c
*
* This file contains an example that demonstrates Q&A functionality of the
* Window Watchdog.
*
* @note
*
* This example assumes that the reset output of the WdtTb device is not
* connected to the reset of the processor. This example will not return
* if the interrupts are not working.
*
* MODIFICATION HISTORY:
*
* <pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 5.6   asa  01/13/23 First release of Q&A mode example.
* 5.7   sb   06/27/23 Correct the interrupt ID for Versal-net platform.
* 5.7   sb   07/12/23 Added support for system device-tree flow.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xwdttb.h"
#include "xil_exception.h"
#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#include <stdio.h>
#else
#include "xscugic.h"
#include "xil_printf.h"
#endif
#else
#include "xinterrupt_wrap.h"
#endif


/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#ifndef SDT
#define WDTTB_DEVICE_ID         XPAR_WDTTB_0_DEVICE_ID


#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID         XPAR_INTC_0_DEVICE_ID
#define WDTTB_IRPT_INTR         XPAR_INTC_0_WDTTB_0_VEC_ID
#else
#define INTC_DEVICE_ID         XPAR_SCUGIC_SINGLE_DEVICE_ID
#ifdef VERSAL_NET
#define WDTTB_IRPT_INTR       XPS_FPD_WWDT_2_INT_ID
#elif defined(versal)
#define WDTTB_IRPT_INTR       XPS_LPD_WWDT_0_INT_ID
#else
#define WDTTB_IRPT_INTR       XPAR_FABRIC_WDTTB_0_VEC_ID
#endif
#endif /* XPAR_INTC_0_DEVICE_ID */
#endif

/*
 * First window clock cycles count should be high enough to ensure
 * 3 answer bytes are calculated and written back.
 */
#define WIN_WDT_FW_COUNT		0x1C00000 /* First window clock cycles */
#define WIN_WDT_SW_COUNT		0x1C00000 /* Second window clock cycles */

#define WIN_WDT_SBC_COUNT		150		/* Selected byte count */
#define WIN_WDT_BSS_COUNT		2		/* Byte segment selected */

#define WIN_WDT_TOKEN_SEED		0x8
#define WIN_WDT_FEEDBACK		0x2
/*
 * Number of iterations should be high enough to ensure that FCV (fail counter
 * value) is decremented to zero and the watchdog is successfully disabled after
 * the example.
 * The default for FCV count is 5 and every good event decrements it by 1.
 */
#define WIN_NUM_ITERATIONS		10

/**************************** Type Definitions *******************************/

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC			XIntc
#define INTC_HANDLER		XIntc_InterruptHandler
#else
#define INTC			XScuGic
#define INTC_HANDLER		XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
/************************** Function Prototypes ******************************/

#ifndef SDT
int WinWdtQAModeExample(INTC *IntcInstancePtr,
			XWdtTb *WdtTbInstancePtr,
			u16 WdtTbDeviceId,
			u16 WdtTbIntrId);
#else
int WdtTbIntrExample(XWdtTb *WdtTbInstancePtr, UINTPTR BaseAddress);
#endif

static void WinWdtQAModeIntrHandler(void *CallBackRef);
#ifndef SDT
static int WinWdtQAModeSetupIntrSystem(INTC *IntcInstancePtr,
				       XWdtTb *WdtTbInstancePtr,
				       u16 WdtTbIntrId);
static void WinWdtQAModeDisableIntrSystem(INTC *IntcInstancePtr,
		u16 WdtTbIntrId);
#endif

/************************** Variable Definitions *****************************/

XWdtTb WdtTbInstance;	/* Instance of Time Base WatchDog Timer */
#ifndef SDT
INTC IntcInstance;	/* Instance of the Interrupt Controller */
#endif

static volatile int WdtExpired;

/*****************************************************************************/
/*****************************************************************************/
/**
* Main function to call the WdtTb (Window Watchdog) QA Mode Example.
*
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/

int main(void)
{
	int Status;

	/*
	 * Call the WdtTb (Window Watchdog) QA Mode, specify the parameters
	 * generated in xparameters.h
	 */
#ifndef SDT
	Status = WinWdtQAModeExample(&IntcInstance,
				     &WdtTbInstance,
				     WDTTB_DEVICE_ID,
				     WDTTB_IRPT_INTR);
#else
	Status = WdtTbIntrExample(&WdtTbInstance, XPAR_XWDTTB_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("\n\rWindow WDT QA Mode example failed.\n\r");
		return XST_FAILURE;
	}

	xil_printf("\n\rSuccessfully ran Window WDT QA Mode example.\n\r");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function demonstrates the functioning of the Window Watchdog QA mode.
*
* This function sets up WWDT in QA mode, enables interrupt subsystem. It
* programs the first (closed) and second (window) and sets up an interrupt to
* raise an interrupt in the open window
* In the closed window the function writes three correct responses to the
* relevant Token_Resp register. Once the interrupt is received in the open
* window, it writes the 4th correct response to the Token_resp register.
* The last (fourth) write ends the second (open window).
*
* This function assumes that the reset output of the Window Watchdog Timer
* is not connected to the reset of the processor. The function allows the
* Window Watchdog Timer to timeout such that a reset will occur if it is
* connected.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC
*		driver.
* @param	WdtTbInstancePtr is a pointer to the instance of WdtTb driver.
* @param	WdtTbDeviceId is the Device ID of the WdtTb Device and is
*		typically XPAR_<WDTTB_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	WdtTbIntrId is the Interrupt Id of the WatchDog and is
*		typically XPAR_<INTC_instance>_<WDTTB_instance>_WDT_
*		INTERRUPT_VEC_ID value from xparameters.h.
*
* @return
*		- XST_SUCCESS if interrupt example run successfully.
*		- XST_FAILURE, if reset has occurred.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
int WinWdtQAModeExample(INTC *IntcInstancePtr,
			XWdtTb *WdtTbInstancePtr,
			u16 WdtTbDeviceId,
			u16 WdtTbIntrId)
#else
int WdtTbIntrExample(XWdtTb *WdtTbInstancePtr, UINTPTR BaseAddress)
#endif
{
	volatile u32 ClosedWindowQAOver;
	XWdtTb_Config *Config;
	u32 LoopCnt = 0;
	u32 ByteCount;
	u8 TokenVal;
	u8 Feedback;
	int Status;
	u8 Ans;

	/*
	 * Initialize the WDTTB driver so that it's ready to use look up
	 * configuration in the config table, then initialize it.
	 */
#ifndef SDT
	Config = XWdtTb_LookupConfig(WdtTbDeviceId);
#else
	Config = XWdtTb_LookupConfig(BaseAddress);
#endif
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XWdtTb_CfgInitialize(WdtTbInstancePtr, Config,
				      Config->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef SDT
	if (!WdtTbInstancePtr->Config.IsPl) {
#else
	if (!(strcmp(WdtTbInstancePtr->Config.Name, "xlnx,versal-wwdt-1.0"))) {
#endif
		/*Enable Window Watchdog Feature in WWDT */
		XWdtTb_ConfigureWDTMode(WdtTbInstancePtr, XWT_WWDT);
	}

	/*
	 * Perform a self-test to ensure that the WWDT is active and behaving
	 * normally.
	 */
	Status = XWdtTb_SelfTest(WdtTbInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the WdtTb to the interrupt subsystem so that interrupts
	 * can occur
	 */

#ifndef SDT
	Status = WinWdtQAModeSetupIntrSystem(IntcInstancePtr,
					     WdtTbInstancePtr,
					     WdtTbIntrId);
#else
	Status = XSetupInterruptSystem(WdtTbInstancePtr, &WinWdtQAModeIntrHandler,
				       Config->IntrId[0],
				       Config->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set window mode register space to writable,
	 * Write 1 to MWC bit (bit 0)
	 */
	XWdtTb_SetRegSpaceAccessMode(WdtTbInstancePtr, 1);

	/* Configure first and second window */
	XWdtTb_SetWindowCount(WdtTbInstancePtr, WIN_WDT_FW_COUNT,
			      WIN_WDT_SW_COUNT);

	/* Set interrupt position in second window*/
	XWdtTb_SetByteCount(WdtTbInstancePtr, WIN_WDT_SBC_COUNT);
	XWdtTb_SetByteSegment(WdtTbInstancePtr, WIN_WDT_BSS_COUNT);

	/*
	 * Disable Secondary Sequence Timer (SST), not used in this example
	 * Fail counter is always enabled in QA mode and PSM mode in irrelevant
	 * in QA mode.
	 */
	XWdtTb_DisableSst(WdtTbInstancePtr);

	/* Set Token Seed and FirstFeedback in Token FB register*/
	XWdtTb_SetSeedValue(WdtTbInstancePtr, WIN_WDT_TOKEN_SEED);
	XWdtTb_SetFeedbackVal(WdtTbInstancePtr, WIN_WDT_FEEDBACK);

	/* Write to Func_Ctrl register to enable Q&A mode */
	XWdtTb_EnableQAMode(WdtTbInstancePtr);

	/*
	 * Start the watchdog timer as a normal application would
	 */
	XWdtTb_Start(WdtTbInstancePtr);
	/*
	 * Set window mode register space to writable. The previous WEN bit
	 * toggle in XWdtTb_Start would have reset the MWC bit to 0.
	 * Write 1 to MWC bit (bit 0)
	 */
	XWdtTb_SetRegSpaceAccessMode(WdtTbInstancePtr, 1);

	XWdtTb_StartQASequence(WdtTbInstancePtr, WIN_WDT_FEEDBACK);

	WdtExpired = FALSE;
	/*
	 * At the start of first window in Q&A mode, the ACNT (Answer Count)
	 * value in ESR register must have a value of 3.
	 */
	if (XWdtTb_GetAnsByteCnt(WdtTbInstancePtr) != 0x3) {
		xil_printf("Answer Count(ACNT) is not 3 at the start of first window.\r\n");
		return XST_FAILURE;
	}

	while (LoopCnt < WIN_NUM_ITERATIONS) {
		xil_printf(".");
		ClosedWindowQAOver = FALSE;
		while (WdtExpired != TRUE) {
			if (ClosedWindowQAOver == FALSE) {
				/* First 3 Answers are written in closed/first window */
				for (ByteCount = 0U; ByteCount < 3U; ByteCount++) {
					if (XWdtTb_InSecondWindow(WdtTbInstancePtr) == FALSE) {
						TokenVal = XWdtTb_GetTokenVal(WdtTbInstancePtr);
						Feedback = XWdtTb_GetFeedbackVal(WdtTbInstancePtr);
						Ans = XWdtTb_GenAnswer(TokenVal, ByteCount, Feedback);
						XWdtTb_UpdateAnsByte(WdtTbInstancePtr, (u32)Ans);
						ClosedWindowQAOver = TRUE;
					}
				}
			}
		}

		if (XWdtTb_InSecondWindow(WdtTbInstancePtr) == TRUE) {
			/*
			 * In second window in Q&A mode, the ACNT (Answer Count)
			 * value in ESR register must have a value of 2 before the
			 * last Answer Byte is written.
			 */
			if (XWdtTb_GetAnsByteCnt(WdtTbInstancePtr) != 0x2) {
				xil_printf("Answer Count(ACNT) is not 2 in second window.\r\n");
				return XST_FAILURE;
			}
			TokenVal = XWdtTb_GetTokenVal(WdtTbInstancePtr);
			Feedback = XWdtTb_GetFeedbackVal(WdtTbInstancePtr);
			Ans = XWdtTb_GenAnswer(TokenVal, 0x3, Feedback);
			XWdtTb_UpdateAnsByte(WdtTbInstancePtr, (u32)Ans);
		}
		WdtExpired = FALSE;
		LoopCnt++;
	}

	/* Disable and disconnect the interrupt system */
#ifndef SDT
	WinWdtQAModeDisableIntrSystem(IntcInstancePtr, WdtTbIntrId);
#else
	XDisconnectInterruptCntrl(Config->IntrId[0], Config->IntrParent);
#endif
	/* Stop the timer */
	Status = XWdtTb_Stop(WdtTbInstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Watchdog cannot be stopped.\r\n");
		return XST_FAILURE;
	}
	XWdtTb_DisableQAMode(WdtTbInstancePtr);

	/* Check for last bad event */
	if (XWdtTb_GetLastEvent(WdtTbInstancePtr) == XWDTTB_NO_BAD_EVENT) {
		return XST_SUCCESS;
	} else {
		xil_printf("The example ends with bad events.\r\n");
		return XST_FAILURE;
	}
}

/*****************************************************************************/
/**
*
* This function setups the interrupt system such that WDT interrupt can occur
* for the WdtTb. This function is application specific since the actual
* system may or may not have an interrupt controller. The WdtTb device could be
* directly connected to a processor without an interrupt controller. The
* user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the instance of the Intc
*		driver.
* @param	WdtTbInstancePtr is a pointer to the instance of WdtTb driver.
* @param	WdtTbIntrId is the Interrupt Id of the WDT interrupt and is
*		typically
*		XPAR_<INTC_instance>_<WDTTB_instance>_WDT_INTERRUPT_VEC_ID
*		value from xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
static int WinWdtQAModeSetupIntrSystem(INTC *IntcInstancePtr,
				       XWdtTb *WdtTbInstancePtr,
				       u16 WdtTbIntrId)
{
	int Status;
#ifdef XPAR_INTC_0_DEVICE_ID

	/*
	 * Initialize the interrupt controller driver so that
	 * it's ready to use. Specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when the WDT
	 * interrupt for the device occurs, the device driver handler performs
	 * the specific interrupt processing for the device
	 */
	Status = XIntc_Connect(IntcInstancePtr, WdtTbIntrId,
			       (XInterruptHandler)WinWdtQAModeIntrHandler,
			       (void *)WdtTbInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/* Enable the WDT interrupt of the WdtTb Device */
	XIntc_Enable(IntcInstancePtr, WdtTbIntrId);
#endif

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


	XScuGic_SetPriorityTriggerType(IntcInstancePtr, WdtTbIntrId,
				       0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, WdtTbIntrId,
				 (Xil_ExceptionHandler)WinWdtQAModeIntrHandler,
				 WdtTbInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Enable the interrupt for the Timer device */
	XScuGic_Enable(IntcInstancePtr, WdtTbIntrId);

	/* Initialize the exception table */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)INTC_HANDLER,
				     IntcInstancePtr);

	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif


/*****************************************************************************/
/**
*
* This function is the Interrupt handler for the WDT Interrupt of the
* WdtTb device. It is called when reached to the interrupt programmed point
* and is called from an interrupt context.
*
* @param	CallBackRef is a pointer to the callback reference.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void WinWdtQAModeIntrHandler(void *CallBackRef)
{
	XWdtTb *WdtTbInstancePtr = (XWdtTb *)CallBackRef;

	/* Set the flag indicating that the WDT has expired */
	WdtExpired = TRUE;
	XWdtTb_IntrClear(WdtTbInstancePtr);
}

/*****************************************************************************/
/**
*
* This function disables the interrupts that occur for the WdtTb.
*
* @param	IntcInstancePtr is the pointer to the instance of INTC driver.
* @param	WdtTbIntrId is the WDT Interrupt Id and is typically
*		XPAR_<INTC_instance>_<WDTTB_instance>_WDT_INTERRUPT_VEC_ID
*		value from xparameters.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
static void WinWdtQAModeDisableIntrSystem(INTC *IntcInstancePtr,
		u16 WdtTbIntrId)
{
	/* Disconnect and disable the interrupt for the WdtTb */
#ifdef XPAR_INTC_0_DEVICE_ID
	XIntc_Disconnect(IntcInstancePtr, WdtTbIntrId);
#else
	/* Disconnect the interrupt */
	XScuGic_Disable(IntcInstancePtr, WdtTbIntrId);
	XScuGic_Disconnect(IntcInstancePtr, WdtTbIntrId);

#endif
}
#endif
