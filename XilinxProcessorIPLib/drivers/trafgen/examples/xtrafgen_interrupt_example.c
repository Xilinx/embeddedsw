/******************************************************************************
*
* Copyright (C) 2013 - 2014 Xilinx, Inc.  All rights reserved.
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
 * @file xtrafgen_interrupt_example.c
 *
 * This file demonstrates how to use the xtrafgen driver on the Xilinx AXI
 * Traffic Generator core. The AXI Traffic Generator IP is designed to 
 * generate AXI4 traffic which can be used to stress different modules/
 * interconnect connected in the system. It has three internal RAMS: MASTER
 * RAM, COMMAND RAM, PARAMETER RAM.  MASTER RAM is used to load/store data from 
 * this memory for write/read transactions.  And the commands to be issued are
 * loaded into COMMAND and PARAMETER RAMs.
 *
 * This example demonstrates by programming known data to Master RAM and
 * commands to Command and Param RAM. Initiating the master logic will take 
 * the data from Master RAM (from a location) and generate data for slave 
 * transactions which will be stored in Master RAM at a different location
 * specified by commands. The test passes when the master logic interrupt
 * asserts and verifies for data to be same.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a srt  01/25/13 First release
 * 4.1   ms   01/23/17 Modified xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings
 *                     are available in all examples. This is a fix for
 *                     CR-965028.
 *       ms   04/05/17 Added tabspace for return statements in functions for
 *                     proper documentation while generating doxygen.
 * </pre>
 *
 * ***************************************************************************
 */

/***************************** Include Files *********************************/
#include "xtrafgen.h"
#include "xparameters.h"
#include "xil_exception.h"

#ifdef XPAR_UARTNS550_0_BASEADDR
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define TRAFGEN_DEV_ID	XPAR_XTRAFGEN_0_DEVICE_ID

#ifdef XPAR_V6DDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_V6DDR_0_S_AXI_BASEADDR
#elif XPAR_S6DDR_0_S0_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_S6DDR_0_S0_AXI_BASEADDR
#elif XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif XPAR_MIG7SERIES_0_BASEADDR
#define DDR_BASE_ADDR XPAR_MIG7SERIES_0_BASEADDR
#endif

#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
                        DEFAULT SET TO 0x01000000
#define MEM_BASE_ADDR	0x01000000
#else
#define MEM_BASE_ADDR	(DDR_BASE_ADDR + 0x1000000)
#endif

#define AXI_ADDRESS		MEM_BASE_ADDR
#define TEST_LENGTH		0x8
#define MSTRRAM_INDEX	(TEST_LENGTH) * 4

#ifdef XPAR_INTC_0_DEVICE_ID
#define ERR_INTR_ID              XPAR_INTC_0_TRAFGEN_0_ERR_OUT_VEC_ID
#define CMP_INTR_ID              XPAR_INTC_0_TRAFGEN_0_IRQ_OUT_VEC_ID
#else
#define ERR_INTR_ID              XPAR_FABRIC_TRAFGEN_0_ERR_OUT_VEC_ID
#define CMP_INTR_ID              XPAR_FABRIC_TRAFGEN_0_IRQ_OUT_VEC_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#else
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
 #define INTC           XIntc
 #define INTC_HANDLER   XIntc_InterruptHandler
#else
 #define INTC           XScuGic
 #define INTC_HANDLER   XScuGic_InterruptHandler
#endif

#undef DEBUG

/************************** Function Prototypes ******************************/
#ifdef XPAR_UARTNS550_0_BASEADDR
static void Uart550_Setup(void);
#endif

int XTrafGenInterruptExample(XTrafGen *InstancePtr, u16 DeviceId);
void InitDefaultCommands(XTrafGen_Cmd *CmdPtr);
static void MasterCompleteIntrHandler(void *Callback);
static void ErrIntrHandler(void *Callback);
static int SetupIntrSystem(INTC * IntcInstancePtr,
                           XTrafGen * InstancePtr, u16 CmpIntrId, u16 ErrIntrId);
static void DisableIntrSystem(INTC * IntcInstancePtr,
                                        u16 CmpIntrId, u16 ErrIntrId);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XTrafGen XTrafGenInstance;

/*
 * Instance of the Interrupt Controller
 */
static INTC Intc;

/*
 * Test Data to write into Master RAM
 */
u32 MasterRamData[TEST_LENGTH] = {0x00000000,
				0x11111111,
				0x22222222,
				0x33333333,
				0x44444444,
				0x55555555,
				0x66666666,
				0x77777777,
			};
/*
 * Data read from Master RAM
 */
u32 VerifyRamData[TEST_LENGTH];

/*
 * Flags interrupt handlers use to notify the application context the events.
 */
volatile int Done;
volatile int Error;

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry of the traffic generator test. 
*
* @param	None
*
* @return
*		- XST_SUCCESS if tests pass
* 		- XST_FAILURE if fails.
*
* @note		None
*
******************************************************************************/
int main()
{
	int Status;

	xil_printf("Entering main\n\r");

	Status = XTrafGenInterruptExample(&XTrafGenInstance, TRAFGEN_DEV_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Traffic Generator Interrupt Example Test Failed\n\r");
		xil_printf("--- Exiting main() ---\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Traffic Generator Interrupt Example\n\r");
	xil_printf("--- Exiting main() ---\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function demonstrates the usage Traffic Generator
* It does the following:
*       - Set up the output terminal if UART16550 is in the hardware build
*       - Initialize the AXI Traffic Generator device
*	- Initialize Master RAM
*       - Initialize commands and add them to list
*       - Program internal command and parameter RAMs
*	- Start Master Logic
*       - Wait for the master logic to finish
*       - Check for errors
*       - Read Master RAM and verify data
*       - Return test status and exit
*
* @param	InstancePtr is a pointer to the instance of the
*		XTrafGen component.
* @param	DeviceId is Device ID of the Axi Traffic Generator Device,
*		typically XPAR_<TRAFGEN_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
******************************************************************************/
int XTrafGenInterruptExample(XTrafGen *InstancePtr, u16 DeviceId)
{

	XTrafGen_Config *Config;
	XTrafGen_Cmd Cmd;
	XTrafGen_Cmd *CmdPtr = &Cmd;
	u32 MasterRamIndex = 0;
	int Status = XST_SUCCESS;
	int Index;

	/* Initial setup for Uart16550 */
#ifdef XPAR_UARTNS550_0_BASEADDR

	Uart550_Setup();

#endif

	/* Initialize the Device Configuration Interface driver */
	Config = XTrafGen_LookupConfig(DeviceId);
	if (!Config) {
		xil_printf("No config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}

	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XTrafGen_CfgInitialize(InstancePtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed\n\r");
		return Status;
	}

	/* Program Master RAM with Test Data */
	XTrafGen_AccessMasterRam(InstancePtr, MasterRamIndex,
			sizeof(MasterRamData), XTG_WRITE, MasterRamData);

	/* Initialize default command fields */
	InitDefaultCommands(CmdPtr);

	/* Add Valid Command for Write Region */
	CmdPtr->CRamCmd.Address = AXI_ADDRESS;
	CmdPtr->CRamCmd.MasterRamIndex = 0x0;
	CmdPtr->CRamCmd.Length = TEST_LENGTH - 1;
	CmdPtr->RdWrFlag = XTG_WRITE;
	CmdPtr->CRamCmd.ValidCmd = 1;
	CmdPtr->CRamCmd.MyDepend = 0;
	CmdPtr->CRamCmd.OtherDepend = XTrafGen_GetLastValidIndex(InstancePtr,
						XTG_WRITE) + 1;
	Status = XTrafGen_AddCommand(InstancePtr, CmdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("AddCommand() failed\n\r");
		return Status;
	}

	/* Add Valid Command for Read Region */
	CmdPtr->CRamCmd.Address = AXI_ADDRESS;
	CmdPtr->CRamCmd.MasterRamIndex = MSTRRAM_INDEX;
	CmdPtr->CRamCmd.Length = 7;
	CmdPtr->CRamCmd.Size = 0x2;
	CmdPtr->RdWrFlag = XTG_READ;
	CmdPtr->CRamCmd.ValidCmd = 1;
	CmdPtr->CRamCmd.MyDepend = 0;
	/* Make this command dependent on Write logic command 1 */
	CmdPtr->CRamCmd.OtherDepend = XTrafGen_GetLastValidIndex(InstancePtr,
						XTG_WRITE) + 1;
	Status = XTrafGen_AddCommand(InstancePtr, CmdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("AddCommand() failed\n\r");
		return Status;
	}

	/* Add second valid command to Write Region  */
	CmdPtr->CRamCmd.Address = AXI_ADDRESS + MSTRRAM_INDEX;
	CmdPtr->CRamCmd.MasterRamIndex = MSTRRAM_INDEX;
	CmdPtr->CRamCmd.Length = 0;
	CmdPtr->RdWrFlag = XTG_WRITE;
	CmdPtr->CRamCmd.ValidCmd = 1;
	CmdPtr->CRamCmd.MyDepend = 0;
	/* Make this command dependent on Read logic command 1 */
	CmdPtr->CRamCmd.OtherDepend = XTrafGen_GetLastValidIndex(InstancePtr,
						XTG_READ) + 1;
	Status = XTrafGen_AddCommand(InstancePtr, CmdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("AddCommand() failed\n\r");
		return Status;
	}

	/* Add invalid command at the end of Write Queue  */
	CmdPtr->CRamCmd.Address = AXI_ADDRESS;
	CmdPtr->CRamCmd.MasterRamIndex = 0x0;
	CmdPtr->RdWrFlag = XTG_WRITE;
	CmdPtr->CRamCmd.Size = 0;
	CmdPtr->CRamCmd.ValidCmd = 0;
	CmdPtr->CRamCmd.MyDepend = 0;
	/* Make this command dependent on Read logic command 1 */
	CmdPtr->CRamCmd.OtherDepend = XTrafGen_GetLastValidIndex(InstancePtr,
						XTG_READ) + 1;;
	Status = XTrafGen_AddCommand(InstancePtr, CmdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("AddCommand() failed\n\r");
		return Status;
	}

	/* Add invalid command at the end of Read Queue  */
	CmdPtr->CRamCmd.MasterRamIndex = 0x0;
	CmdPtr->RdWrFlag = XTG_READ;
	CmdPtr->CRamCmd.Length = 0;
	CmdPtr->CRamCmd.Size = 0x0;
	CmdPtr->CRamCmd.ValidCmd = 0;
	/* Make this command dependent on Write logic command 2 */
	CmdPtr->CRamCmd.OtherDepend = XTrafGen_GetLastValidIndex(InstancePtr,
						XTG_WRITE) + 1;
	Status = XTrafGen_AddCommand(InstancePtr, CmdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("AddCommand() failed\n\r");
		return Status;
	}

	/* Display Command list */
#ifdef DEBUG
	XTrafGen_PrintCmds(InstancePtr);
#endif

	/* Program all prepared commands */
	Status = XTrafGen_WriteCmdsToHw(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("WriteCmdsToHw() failed\n\r");
		return Status;
	}

	/* Enable Complete Interrupt bit */
	XTrafGen_EnableMasterCmpInterrupt(InstancePtr);

	/* Set up Interrupt system  */
	Status = SetupIntrSystem(&Intc, InstancePtr, CMP_INTR_ID, ERR_INTR_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed intr setup\r\n");
		return Status;
	}

	/* Initialize flags before start transfer test  */
	Done = 0;
	Error = 0;

	/* Start Master Logic */
	XTrafGen_StartMasterLogic(InstancePtr);

	while (!Done && !Error);

	if (Error) {
		xil_printf("Errors in transfer\r\n");
		return XST_FAILURE;
	}

	/* Read Master RAM */
	XTrafGen_AccessMasterRam(InstancePtr, MSTRRAM_INDEX,
			sizeof(MasterRamData), XTG_READ, VerifyRamData);

	/* Verify Data */
	for (Index = 0 ; Index < TEST_LENGTH - 1; Index++) {
		if (VerifyRamData[Index] != MasterRamData[Index]) {
			xil_printf("Data Mismatch\n\r");
			return XST_FAILURE;
		}
	}

	DisableIntrSystem(&Intc, CMP_INTR_ID, ERR_INTR_ID);
	return Status;
}


#ifdef XPAR_UARTNS550_0_BASEADDR
/*****************************************************************************/
/*
*
* Uart16550 setup routine, need to set baudrate to 9600 and data bits to 8
*
* @param	None
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void Uart550_Setup(void)
{

	XUartNs550_SetBaud(XPAR_UARTNS550_0_BASEADDR,
			XPAR_XUARTNS550_CLOCK_HZ, 9600);

	XUartNs550_SetLineControlReg(XPAR_UARTNS550_0_BASEADDR,
			XUN_LCR_8_DATA_BITS);
}
#endif

/*****************************************************************************/
/*
*
* Initialize default command fields
*
* @param	XTrafGen_Cmd is a pointer to command structure
*
* @return	XST_SUCCESS if successful
*
* @note		None
*
******************************************************************************/
void InitDefaultCommands(XTrafGen_Cmd *CmdPtr)
{
	/* Command RAM default command values */
	CmdPtr->CRamCmd.LastAddress = 0;
	CmdPtr->CRamCmd.Prot = 0;
	CmdPtr->CRamCmd.Id = 0;
	CmdPtr->CRamCmd.Size = 0x2;
	CmdPtr->CRamCmd.Burst = 0x1;
	CmdPtr->CRamCmd.Lock = 0;
	CmdPtr->CRamCmd.Length = 0;
	CmdPtr->CRamCmd.MyDepend = 0;
	CmdPtr->CRamCmd.OtherDepend = 0;
	CmdPtr->CRamCmd.MasterRamIndex = 0;
	CmdPtr->CRamCmd.Qos = 0;
	CmdPtr->CRamCmd.User = 0;
	CmdPtr->CRamCmd.Cache = 0;
	CmdPtr->CRamCmd.ExpectedResp = 0x7;

	/* Parameter RAM default command values */
	CmdPtr->PRamCmd.AddrMode = 0;
	CmdPtr->PRamCmd.IdMode = 0;
	CmdPtr->PRamCmd.IntervalMode = 0;
	CmdPtr->PRamCmd.OpCntl0 = 0;
	CmdPtr->PRamCmd.OpCntl1 = 0;
	CmdPtr->PRamCmd.OpCntl2 = 0;
	CmdPtr->PRamCmd.Opcode = 0;
}

/*****************************************************************************/
/*
*
* This is the Master Logic Complete Interrupt handler function.
*
* This clears the master complete interrupt and set the done flag.
*
* @param	Callback is a pointer to Instance of Traffic Generator device.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void MasterCompleteIntrHandler(void *Callback)
{
	XTrafGen *InstancePtr = (XTrafGen *) Callback;

	XTrafGen_ClearMasterCmpInterrupt(InstancePtr);

	Done = 1;
}

/*****************************************************************************/
/*
*
* This is the Error Interrupt handler function.
*
* This clears the errors and set the error flag.
*
* @param	Callback is a pointer to Instance of Traffic Generator device.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void ErrIntrHandler(void *Callback)
{
	XTrafGen *InstancePtr = (XTrafGen *) Callback;
	u32 Value;

	Value = XTrafGen_ReadErrors(InstancePtr);
	if (Value) {
		XTrafGen_ClearErrors(InstancePtr, Value);
	}
		
	Error = 1;
}

/*****************************************************************************/
/*
*
* This function setups the interrupt system so interrupts can occur for the
* Traffic Generator, it assumes INTC component exists in the hardware system.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC.
* @param	InstancePtr is a pointer to the instance of the Traffic
*		Generator Device.
* @param	CmpIntrId is the Master Complete Interrupt ID.
* @param	ErrIntrId is the Error Interrupt ID.
*
* @return
* 		- XST_SUCCESS if successful,
* 		- XST_FAILURE if not successful
*
* @note		None.
*
******************************************************************************/
static int SetupIntrSystem(INTC * IntcInstancePtr,
			XTrafGen * InstancePtr, u16 CmpIntrId, u16 ErrIntrId)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID

	/* Initialize the interrupt controller and connect the ISRs */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed init intc\r\n");
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstancePtr, CmpIntrId,
			(XInterruptHandler) MasterCompleteIntrHandler, InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed master complete connect intc\r\n");
		return XST_FAILURE;
	}
	
	Status = XIntc_Connect(IntcInstancePtr, ErrIntrId,
			(XInterruptHandler) ErrIntrHandler, InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed err connect intc\r\n");
		return XST_FAILURE;
	}

	/* Start the interrupt controller */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to start intc\r\n");
		return XST_FAILURE;
	}

	XIntc_Enable(IntcInstancePtr, CmpIntrId);
	XIntc_Enable(IntcInstancePtr, ErrIntrId);

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

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
				IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, CmpIntrId, 0xA0, 0x3);

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, ErrIntrId, 0xA0, 0x3);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, CmpIntrId,
				(Xil_InterruptHandler)MasterCompleteIntrHandler,
				InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XScuGic_Connect(IntcInstancePtr, ErrIntrId,
				(Xil_InterruptHandler)ErrIntrHandler,
				InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	XScuGic_Enable(IntcInstancePtr, CmpIntrId);
	XScuGic_Enable(IntcInstancePtr, ErrIntrId);
#endif

	/* Enable interrupts from the hardware */
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
		(Xil_ExceptionHandler)INTC_HANDLER,
		(void *)IntcInstancePtr);

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function disables the interrupts for Traffic Generator.
*
* @param	IntcInstancePtr is the pointer to the INTC component instance
* @param	CmpIntrId is interrupt ID associated w/ Master logic
*		compleetion
* @param	ErrIntrId is interrupt ID associated w/ Master or Slave
*		errors.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void DisableIntrSystem(INTC * IntcInstancePtr,
				u16 CmpIntrId, u16 ErrIntrId)
{
#ifdef XPAR_INTC_0_DEVICE_ID
	/* Disconnect the interrupts for the Master complete and error */
	XIntc_Disconnect(IntcInstancePtr, CmpIntrId);
	XIntc_Disconnect(IntcInstancePtr, ErrIntrId);
#else
	XScuGic_Disconnect(IntcInstancePtr, CmpIntrId);
	XScuGic_Disconnect(IntcInstancePtr, ErrIntrId);
#endif
}
