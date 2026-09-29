/******************************************************************************
* Copyright (C) 2013 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xtrafgen_interrupt_example.c
 * @brief Demonstrates interrupt-mode use of the xtrafgen driver.
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
 * commands to Command and Param RAM. Initiating the master logic takes
 * the data from Master RAM (from a location) and generates data for slave
 * transactions that are stored in Master RAM at a different location
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
 * 4.5   sd   07/18/23 Fix the disable interrupt
 * 4.7   ar   03/07/24 Fix the interrupt ID sequence in the SDT flow.
 * 4.9  vlt  12/18/25 Update Doxygen comments to include SDT flow details.
 * 4.10 vn   07/30/26 Fix Doxygen warnings
 * 4.10 vn   09/29/26 Align Doxygen comments with AMD standards
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

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif
#endif

#ifdef SDT
#include "xinterrupt_wrap.h"
#endif
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#ifndef SDT
#define TRAFGEN_DEV_ID	XPAR_XTRAFGEN_0_DEVICE_ID /**< Device ID. */
#endif

#ifdef XPAR_V6DDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_V6DDR_0_S_AXI_BASEADDR /**< DDR base address. */
#elif XPAR_S6DDR_0_S0_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_S6DDR_0_S0_AXI_BASEADDR /**< DDR base address. */
#elif XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
/** DDR base address. */
#define DDR_BASE_ADDR	XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif XPAR_MIG7SERIES_0_BASEADDR
#define DDR_BASE_ADDR XPAR_MIG7SERIES_0_BASEADDR /**< DDR base address. */
#endif

#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
                        DEFAULT SET TO 0x01000000
/** Base address of the memory used by the example. */
#define MEM_BASE_ADDR	0x01000000
#else
/** Base address of the memory used by the example. */
#define MEM_BASE_ADDR	(DDR_BASE_ADDR + 0x1000000)
#endif

/** AXI address used in the commands. */
#define AXI_ADDRESS		MEM_BASE_ADDR
#define TEST_LENGTH		0x8 /**< Number of words in the test data. */
/** Master RAM index used in the commands. */
#define MSTRRAM_INDEX	(TEST_LENGTH) * 4

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
/** Error interrupt ID. */
#define ERR_INTR_ID              XPAR_INTC_0_TRAFGEN_0_ERR_OUT_VEC_ID
/** Master Logic Complete interrupt ID. */
#define CMP_INTR_ID              XPAR_INTC_0_TRAFGEN_0_IRQ_OUT_VEC_ID
#else
/** Error interrupt ID. */
#define ERR_INTR_ID              XPAR_FABRIC_TRAFGEN_0_ERR_OUT_VEC_ID
/** Master Logic Complete interrupt ID. */
#define CMP_INTR_ID              XPAR_FABRIC_TRAFGEN_0_IRQ_OUT_VEC_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID /**< INTC device ID. */
#else
/** INTC device ID. */
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif
#endif

#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
 #define INTC           XIntc /**< Interrupt controller type. */
 #define INTC_HANDLER   XIntc_InterruptHandler /**< Interrupt handler. */
#else
 #define INTC           XScuGic /**< Interrupt controller type. */
 #define INTC_HANDLER   XScuGic_InterruptHandler /**< Interrupt handler. */
#endif
#endif

#undef DEBUG

/************************** Function Prototypes ******************************/
#ifdef XPAR_UARTNS550_0_BASEADDR
static void Uart550_Setup(void);
#endif

#ifndef SDT
int XTrafGenInterruptExample(XTrafGen *InstancePtr, u16 DeviceId);
static int SetupIntrSystem(INTC * IntcInstancePtr,
                           XTrafGen * InstancePtr, u16 CmpIntrId, u16 ErrIntrId);
static void DisableIntrSystem(INTC * IntcInstancePtr,
                                        u16 CmpIntrId, u16 ErrIntrId);
#else
int XTrafGenInterruptExample(XTrafGen *InstancePtr, UINTPTR BaseAddress);
#endif
void InitDefaultCommands(XTrafGen_Cmd *CmdPtr);
static void MasterCompleteIntrHandler(void *Callback);
static void ErrIntrHandler(void *Callback);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XTrafGen XTrafGenInstance; /**< Instance of the Traffic Generator. */

/*
 * Instance of the Interrupt Controller
 */
#ifndef SDT
static INTC Intc;
#endif

/**
 * Test Data to write into Master RAM.
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
/**
 * Data read from Master RAM.
 */
u32 VerifyRamData[TEST_LENGTH];

/*
 * Flags interrupt handlers use to notify the application context the events.
 */
/** Flag set by the Master Logic Complete interrupt handler. */
volatile int Done;
volatile int Error; /**< Flag set by the Error interrupt handler. */

/*****************************************************************************/
/**
* @brief Runs the traffic generator interrupt example test.
*
* This function is the main entry of the traffic generator test.
*
* @return
*              - XST_SUCCESS if tests pass.
*              - XST_FAILURE if fails.
*
******************************************************************************/
int main()
{
	int Status;

	xil_printf("Entering main\n\r");

#ifndef SDT
	Status = XTrafGenInterruptExample(&XTrafGenInstance, TRAFGEN_DEV_ID);
#else
	Status = XTrafGenInterruptExample(&XTrafGenInstance, XPAR_XTRAFGEN_0_BASEADDR);
#endif
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
* @brief Demonstrates the usage of the Traffic Generator in interrupt mode.
*
* It does the following:
*       - Set up the output terminal if UART16550 is in the hardware build
*       - Initialize the AXI Traffic Generator device
*       - Initialize Master RAM
*       - Initialize commands and add them to list
*       - Program internal command and parameter RAMs
*       - Start Master Logic
*       - Wait for the master logic to finish
*       - Check for errors
*       - Read Master RAM and verify data
*       - Return test status and exit
*
* @param[in,out] InstancePtr Pointer to the instance of the
*                XTrafGen component.
* @if SDT
* @param[in]     BaseAddress Base address of the device.
* @else
* @param[in]     DeviceId Device ID of the Axi Traffic Generator Device,
*                typically XPAR_<TRAFGEN_instance>_DEVICE_ID value from
*                xparameters.h.
* @endif
*
* @return
*                -XST_SUCCESS to indicate success
*                -XST_FAILURE to indicate failure.
*
* @note          In XSCT/classic flow, DeviceId is used to look up the device
*                configuration.
*
*****************************************************************************/
#ifndef SDT
int XTrafGenInterruptExample(XTrafGen *InstancePtr, u16 DeviceId)
#else
int XTrafGenInterruptExample(XTrafGen *InstancePtr, UINTPTR BaseAddress)
#endif
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
#ifndef SDT
	Config = XTrafGen_LookupConfig(DeviceId);
#else
	Config = XTrafGen_LookupConfig(BaseAddress);
#endif
	if (!Config) {
#ifndef SDT
		xil_printf("No config found for %d\r\n", DeviceId);
#endif
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
#ifndef SDT
	Status = SetupIntrSystem(&Intc, InstancePtr, CMP_INTR_ID, ERR_INTR_ID);
#else
	Status = XSetupInterruptSystem(InstancePtr, &MasterCompleteIntrHandler,
			               InstancePtr->Config.IntId[MASTER_INTRID],
				       InstancePtr->Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	Status = XSetupInterruptSystem(InstancePtr, &ErrIntrHandler,
			               InstancePtr->Config.IntId[ERROR_INTRID],
				       InstancePtr->Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
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

#ifndef SDT
	DisableIntrSystem(&Intc, CMP_INTR_ID, ERR_INTR_ID);
#else
	XDisconnectInterruptCntrl(InstancePtr->Config.IntId[0],
				       InstancePtr->Config.IntrParent);
	XDisconnectInterruptCntrl(InstancePtr->Config.IntId[1],
				       InstancePtr->Config.IntrParent);
#endif
	return Status;
}


#ifdef XPAR_UARTNS550_0_BASEADDR
/*****************************************************************************/
/**
*
* @brief Sets up the UART16550 with a baud rate of 9600 and 8 data bits.
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
/**
*
* @brief Initializes the default command fields.
*
* @param[out]  CmdPtr Pointer to command structure.
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
/**
*
* @brief Handles the Master Logic Complete interrupt.
*
* Clears the master complete interrupt and sets the done flag.
*
* @param[in]   Callback Pointer to Instance of Traffic Generator device.
*
******************************************************************************/
static void MasterCompleteIntrHandler(void *Callback)
{
	XTrafGen *InstancePtr = (XTrafGen *) Callback;

	XTrafGen_ClearMasterCmpInterrupt(InstancePtr);

	Done = 1;
}

/*****************************************************************************/
/**
*
* @brief Handles the Error interrupt.
*
* Clears the errors and sets the error flag.
*
* @param[in]   Callback Pointer to Instance of Traffic Generator device.
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
/**
*
* @brief Sets up the interrupt system for the Traffic Generator.
*
* Sets up the interrupt system so interrupts can occur for the Traffic
* Generator. It assumes an INTC component exists in the hardware system.
*
* @param[in,out] IntcInstancePtr Pointer to the instance of the INTC.
* @param[in]     InstancePtr Pointer to the instance of the Traffic
*                Generator Device.
* @param[in]     CmpIntrId Master Complete Interrupt ID.
* @param[in]     ErrIntrId Error Interrupt ID.
*
* @return
*                - XST_SUCCESS if successful,.
*                - XST_FAILURE if not successful.
*
******************************************************************************/
#ifndef SDT
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
#endif

/*****************************************************************************/
/**
* @brief Disables the interrupts for the Traffic Generator.
*
* @param[in]   IntcInstancePtr Pointer to the INTC component instance.
* @param[in]   CmpIntrId Interrupt ID associated with Master logic
*              completion.
* @param[in]   ErrIntrId Interrupt ID associated with Master or Slave
*              errors.
*
******************************************************************************/
#ifndef SDT
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
#endif
