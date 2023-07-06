/******************************************************************************
* Copyright (C) 2013 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xtrafgen_polling_example.c
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
 * specified by commands. The test passes when the master logic completes and  
 * verifies for data to be same.
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

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#ifndef SDT
#define TRAFGEN_DEV_ID	XPAR_XTRAFGEN_0_DEVICE_ID
#endif

#ifdef XPAR_V6DDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_V6DDR_0_S_AXI_BASEADDR
#elif XPAR_S6DDR_0_S0_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_S6DDR_0_S0_AXI_BASEADDR
#elif XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif XPAR_MIG_7SERIES_1_BASEADDR
#define DDR_BASE_ADDR XPAR_MIG_7SERIES_1_BASEADDR
#elif XPAR_MIG7SERIES_0_BASEADDR
#define DDR_BASE_ADDR XPAR_MIG7SERIES_0_BASEADDR
#elif XPAR_MIG_0_BASEADDRESS
#define DDR_BASE_ADDR XPAR_MIG_0_BASEADDRESS
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

#undef DEBUG

/************************** Function Prototypes ******************************/
#ifndef SDT
int XTrafGenPollingExample(XTrafGen *InstancePtr, u16 DeviceId);
#else
int XTrafGenPollingExample(XTrafGen *InstancePtr, UINTPTR BaseAddress);
#endif
void InitDefaultCommands(XTrafGen_Cmd *CmdPtr);
#ifdef XPAR_UARTNS550_0_BASEADDR
static void Uart550_Setup(void);
#endif

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XTrafGen XTrafGenInstance;

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

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry of the traffic generator test. 
*
* @param        None
*
* @return
*				- XST_SUCCESS if tests pass
*               - XST_FAILURE if fails.
*
* @note         None.
*
******************************************************************************/
int main()
{
	int Status;

	xil_printf("Entering main\n\r");

#ifndef SDT
	Status = XTrafGenPollingExample(&XTrafGenInstance, TRAFGEN_DEV_ID);
#else
	Status = XTrafGenPollingExample(&XTrafGenInstance, XPAR_XTRAFGEN_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Traffic Generator Polling Example Test Failed\n\r");
		xil_printf("--- Exiting main() ---\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Traffic Generator Polling Example\n\r");
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
*
*
* @param	InstancePtr is a pointer to the instance of the
*			XTrafGen component.
* @param	DeviceId is Device ID of the Axi Traffic Generator Device,
*			typically XPAR_<TRAFGEN_instance>_DEVICE_ID value from
*			xparameters.h.
*
* @return
*			-XST_SUCCESS to indicate success
*			-XST_FAILURE to indicate failure
*
******************************************************************************/
#ifndef SDT
int XTrafGenPollingExample(XTrafGen *InstancePtr, u16 DeviceId)
#else
int XTrafGenPollingExample(XTrafGen *InstancePtr, UINTPTR BaseAddress)
#endif
{

	XTrafGen_Config *Config;
	XTrafGen_Cmd Cmd;
	XTrafGen_Cmd *CmdPtr = &Cmd;
	u32 MasterRamIndex = 0;
	u32 Done;
	u32 Error;
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

	/* Start Master Logic */
	XTrafGen_StartMasterLogic(InstancePtr);

	while (1) {
		Done = XTrafGen_IsMasterLogicDone(InstancePtr);
		if (Done) {
			break;
		}

		Error = XTrafGen_ReadErrors(InstancePtr);
		if (Error) {
			XTrafGen_ClearErrors(InstancePtr, Error);
			return XST_FAILURE;
		}
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

	return XST_SUCCESS;
}


#ifdef XPAR_UARTNS550_0_BASEADDR
/*****************************************************************************/
/*
*
* Uart16550 setup routine, need to set baudrate to 9600 and data bits to 8
*
* @param        None
*
* @return       None
*
* @note         None.
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
* @param        XTrafGen_Cmd is a pointer to command structure
*
* @return       None.
*
* @note         None.
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

