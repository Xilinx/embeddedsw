/******************************************************************************
*
* (c) Copyright 2010-14 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/******************************************************************************/
/**
*
* @file intg.c
*
* DESCRIPTION:
*
* Provides integration test entry point for the XNandPs8 component.
*
* @note		None
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ----   ----------------------------------------------------------------
* 1.0   sb    11/28/14 First release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "intg.h"
#include <stdio.h>

/************************** Constant Definitions *****************************/

/**
 * Main menu selections.
 * @{
 */
#define MENU_MAIN_TEST			1
#define MENU_MAIN_UTIL			2
#define MENU_MAIN_EXIT			99
/*@}*/

/**
 * Test sub-menu selections.
 * @{
 */
#define MENU_TEST_ALL				0
#define MENU_TEST_ERASE_READ			1
#define MENU_TEST_FLASH_RW			2
#define MENU_TEST_RANDOM_RW			3
#define MENU_TEST_SPAREBYTES_RW			4
#define MENU_TEST_PARTIAL_RW			5
#define MENU_TEST_ECC				6
#define MENU_TEST_BBT				7
#define MENU_TEST_MARK_BLOCK_BAD		8
#define MENU_TEST_EXIT				99
/*@}*/

/**
 * Create unique bitmasks from the test numbers.
 * @{
 */
#define INTG_TEST_ALL				0xFFFFFFFF
#define INTG_TEST_ERASE_READ			(1 << MENU_TEST_ERASE_READ)
#define INTG_TEST_FLASH_RW			(1 << MENU_TEST_FLASH_RW)
#define INTG_TEST_RANDOM_RW			(1 << MENU_TEST_RANDOM_RW)
#define INTG_TEST_SPAREBYTES_RW			(1 << MENU_TEST_SPAREBYTES_RW)
#define INTG_TEST_PARTIAL_RW			(1 << MENU_TEST_PARTIAL_RW)
#define INTG_TEST_ECC				(1 << MENU_TEST_ECC)
#define INTG_TEST_BBT				(1 << MENU_TEST_BBT)
#define INTG_TEST_MARK_BLOCK_BAD		(1 << MENU_TEST_MARK_BLOCK_BAD)
/*@}*/

/**
 * Utility sub-menu selections.
 * @{
 */
#define MENU_UTIL_DATA_INTERFACE		1
#define MENU_UTIL_FLASH_DETAILS			2
#define MENU_UTIL_EXIT				99
/*@}*/

typedef enum TimingMode {
	Mode0 = 0U,
	Mode1,
	Mode2,
	Mode3,
	Mode4,
	Mode5
}NandPs8_TimingMode;

/*
 * Uncomment the following constant definition if UART16550 is standard output
 * device. If UartLite is standard output device, comment the definition
 */
//#define UART16550

/*
 * Uncomment the following constant definition if UARTLITE is standard output
 * device. If UART16550 is standard output device, comment the definition
 */
//#define UARTLITE

#ifdef UART16550
#include "xuartns550_l.h"
#define UART_BASEADDR XPAR_RS232_UART_BASEADDR
#elif UARTLITE
#include "xuartlite_l.h"
#define UART_BASEADDR XPAR_UARTLITE_BASEADDR
#else
#include "xuartps_hw.h"
#define UART_BASEADDR XPS_UART1_BASEADDR	/**< UART-1 Base Address */
#endif

//#define printf xil_printf
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
/*
 * Buffers used during read and write transactions.
 */
u8 ReadBuffer[TEST_BUF_SIZE] __attribute__ ((aligned(64)));/**< Read buffer */
u8 WriteBuffer[TEST_BUF_SIZE] __attribute__ ((aligned(64)));/**< write buffer */
/**
 * Nand driver instance for the Nand device.
 */
XNandPs8 NandInstance;
XNandPs8 *NandInstPtr = &NandInstance;
XNandPs8_Config *Config;
/************************** Function Prototypes ******************************/

static unsigned int GetUserInput(char* Prompt, char* Response,
				 unsigned int
				 MaxChars);
static int  GetMainMenuCommand(char* CmdLine);
static void RunTestMenu(char* CmdLine);
static void RunUtilMenu(char* CmdLine);
static int ChangeTimingMode(XNandPs8 *NandInstPtr,
				XNandPs8_DataInterface NewIntf,
				XNandPs8_TimingMode NewMode);

extern char inbyte ();		/**< Inbyte returns the byte received by device. */
s32 FlashInit(u16 NandDeviceId);
#ifdef AUTOMATIC_TEST_MODE
int Automode_Tests(int TestLoops);
int CodeCoverage_Tests(int TestLoops);
#endif

/*****************************************************************************/
/*
 * Retrieve a line of input from the user. A line is defined as all characters
 * up to a new line.
 *
 * This basically implements a fgets function. The standalone EDK stdin
 * is apparently buggy because it behaves differently when new line is
 * entered by itself vs. when it is entered after a number of regular chars
 * have been entered. Characters entered are echoed back.
 *
 * @param   Prompt   -  Printed before string is accepted to ask the user to
 *					  enter something.
 * @param   Response -  User entered string with new line stripped
 * @param   MaxChars -  Maximum number of characters to read
 *
 * @return  Number of characters read (excluding newlines)
 *
 *****************************************************************************/
unsigned int GetUserInput(char* Prompt, char* Response, unsigned int MaxChars)
{
	int Finished, i;

	/* display prompt */
	if (Prompt) printf(Prompt);

	Finished = 0;
	i = 0;

	while(!Finished && (i < MaxChars - 1)) {
		/* flush out any output pending in stdout */
		fflush(stdout);

		/* wait for a character to arrive */
		Response[i] = inbyte();

		/* normal chars, add them to the string and keep going */
		if ((Response[i] >= 0x20) && (Response[i] <=0x7E)) {
			printf("%c", Response[i++]);
			continue;
		}

		/* control chars */
		switch(Response[i]) {
			/* carriage return */
			case 0x0D:
			/* Case Fall through */
			/* Line Feed */
			case 0x0A:
				Response[i++] = '\n';
				Finished = 1;
				printf("\r\n");
				break;

			/* backspace */
			case 0x08:
				if (i != 0) {
					/* erase previous character and move
					cursor back one space */
					printf("\b \b");
					Response[--i] = 0;
				}
				break;

			/* ignore all other control chars */
			default:
				continue;
		}
	}

	Response[i] = 0;
	return i;
}

/*****************************************************************************/
/**
*
* Intg_Entry
*
* Executes all unit tests for the device
*
* param		None
*
* @return	None
*
* @note		None
*
******************************************************************************/
void Intg_Entry(void)
{
	char CmdLine[132];
	s32 Status = XST_FAILURE;
#ifdef AUTOMATIC_TEST_MODE
        int   TestLoops;
        int   TestFailures;
	u32 t_mode = 0U;
	u32 RegVal = 0U;
	u32 feature;
	u32 ddr_mode[2];
#endif

	/* print banner */
	printf("\r\n\r\nNand Driver Integration Test\r\n");
	printf("Created on %s\n\n\r", __DATE__);
	printf("=====================================================\r\n");

	CT_Init();

	Status = FlashInit(NAND_DEVICE_ID);
	if(Status != XST_SUCCESS){
		printf("Nand Flash Initialization Failed\r\n");
		goto Out;
	}
	printf("Flash initialization done\r\n");
#ifndef AUTOMATIC_TEST_MODE
	/* prompt user for menu selections */
	while(1) {
		switch(GetMainMenuCommand(CmdLine)) {
			case MENU_MAIN_TEST:
				RunTestMenu(CmdLine);
				break;

			case MENU_MAIN_UTIL:
				RunUtilMenu(CmdLine);
				break;

			case MENU_MAIN_EXIT:
				printf("\r\nByebye! Thanks for using Nand ");
				printf("and Driver integration tests :)\r\n");
				return;

			default:
				printf("Invalid selections\r\n");
		}
	}
#else
	/* set defaults */
	TestLoops = 1;
	TestFailures = 0;

	/*
	 * Setting Interface
	 */
	NandInstPtr->DataInterface = SDR;

	for (t_mode = Mode0; t_mode <= Mode5; t_mode++){
		NandInstPtr->DataInterface = SDR;

		/*
		 * Set Timing mode
		 */
		Status = ChangeTimingMode(NandInstPtr, SDR, t_mode);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
		/*
		 * Run the test cases
		 */
		TestFailures += Automode_Tests(TestLoops);
	}

	for (t_mode = Mode0; t_mode <= Mode5; t_mode++){

		NandInstPtr->DataInterface = NVDDR;
		/*
		 * Set Timing mode
		 */
		Status = ChangeTimingMode(NandInstPtr, NVDDR, t_mode);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
		/*
		 * Run the test cases
		 */
		TestFailures += Automode_Tests(TestLoops);
	}

	/*
	 * Run Code Coverage Tests
	 */
	TestFailures += CodeCoverage_Tests(TestLoops);

	if (TestFailures) {
		XIL_FAIL(TestFailures);
	} else {
		XIL_PASS();
	}
#endif

Out:
	exit(0);
}

#ifdef AUTOMATIC_TEST_MODE
/*****************************************************************************/
/**
*
* Auto Mode Tests
*
* Executes all unit tests for the device
*
* param		TestLoops: Number of times a test should run.
*
* @return	Total Test failures
*
* @note		None
*
******************************************************************************/
int Automode_Tests(int TestLoops)
{
	volatile int failures = 0;

	failures += Intg_EraseReadTest(NandInstPtr, TestLoops);
	failures += Intg_FlashRWTest(NandInstPtr, TestLoops);
	failures += Intg_RandomRWTest(NandInstPtr, TestLoops);
	failures += Intg_SpareBytesRWTest(NandInstPtr, TestLoops);
	failures += Intg_PartialRWTest(NandInstPtr, TestLoops);
	failures += Intg_EccTest(NandInstPtr, TestLoops);

	return failures;
}

/*****************************************************************************/
/**
*
* Code Coverage Tests
*
* Executes all code coverage tests for the device
*
* param		TestLoops: Number of times a test should run.
*
* @return	Total Test failures
*
* @note		None
*
******************************************************************************/
int CodeCoverage_Tests(int TestLoops)
{
	volatile int failures = 0;

	xil_printf("\tRunning Code Coverage Tests Now\r\n");

	XNandPs8_DisableDmaMode(NandInstPtr);
	failures += Automode_Tests(TestLoops);
	XNandPs8_EnableDmaMode(NandInstPtr);
	failures += Intg_BbtTest(NandInstPtr, TestLoops);
	failures += Intg_MarkBlockBadTest(NandInstPtr, TestLoops);
	failures += Intg_CodeCoverageTest(NandInstPtr, TestLoops);

	return failures;
}
#endif

/*****************************************************************************/
/**
 *
 * Prompt the user for selections in the main menu. The user is allowed to
 * enter one command at a time.
 *
 * @param	CmdLine - Storage to use for User input.
 *
 * @return	Numeric command entered.
 *
 * @note	None.
 *
 *****************************************************************************/
static int GetMainMenuCommand(char* CmdLine)
{
	/* prompt user */
	printf("\r\n\r\n\r\nMain Menu:\r\n");
	printf("==========\r\n");
	printf("%d  - Test menu\r\n", MENU_MAIN_TEST);
	printf("%d  - Util menu\r\n", MENU_MAIN_UTIL);
	printf("%d - Exit\r\n\r\n", MENU_MAIN_EXIT);

	*CmdLine = '\n';
	while(*CmdLine == '\n') {
		printf("Enter selection: ");
		GetUserInput(0, CmdLine, 131);
	}
	return(atoi(CmdLine));
}


/*****************************************************************************/
/**
 *
 * Prompt the user for a sequence of tests to run in the test sub-menu
 *
 * @param	CmdLine - Storage to use for User input
 *
 * @return	None
 *
 * @note	None
 *
 *****************************************************************************/
static void RunTestMenu(char* CmdLine)
{
	char* Token;
	char* tmp;
	int   QuitToMain = 0;
	int   TestLoops;
	int   TestFailures;
	u32 RunTestMask;

	/*
	 * Flash Initialization Function
	 */

	while(!QuitToMain) {
		/* set defaults */
		TestLoops = 1;
		RunTestMask = 0;
		TestFailures = 0;

		/* prompt user */
		printf("\r\n\r\n\r\nTest Menu:\r\n");
		printf("==========\r\n");
		printf("%d - Run all tests\r\n", MENU_TEST_ALL);
		printf("%d - Flash Erase Read Test\r\n", MENU_TEST_ERASE_READ);
		printf("%d - Flash Read Write Test \r\n", MENU_TEST_FLASH_RW);
		printf("%d - Random Block Flash Read Write Test\r\n", MENU_TEST_RANDOM_RW);
		printf("%d - Spare Bytes Read Write Test \r\n",
				MENU_TEST_SPAREBYTES_RW);
		printf("%d - Partial Page Read Write Test\r\n",
				MENU_TEST_PARTIAL_RW);
		printf("%d - ECC Test.\r\n", MENU_TEST_ECC);
		printf("%d - BBT Scan Test.\r\n", MENU_TEST_BBT);
		printf("%d - Mark Block Bad Test.\r\n", MENU_TEST_MARK_BLOCK_BAD);
		printf("%d - Exit to main menu\r\n\r\n", MENU_TEST_EXIT);
		printf("More than one test can be specified\r\n");
		printf("Adding l <number> sets the number of test loops\n\n");

		/* wait for input */
		*CmdLine = '\n';
		while(*CmdLine == '\n') {
			printf("Enter test(s) to execute: ");
			GetUserInput(0, CmdLine, 131);
		}

		/* parse input line */
		Token = strtok_r(CmdLine, " ", &tmp);
		while(Token) {
			if ((*Token == 'l') || (*Token == 'L')) {
				TestLoops = atoi(Token+1);
			}
			else {
				switch(atoi(Token)) {
					case MENU_TEST_ALL:
						RunTestMask |= INTG_TEST_ALL;
						break;

					case MENU_TEST_FLASH_RW:
						RunTestMask |= INTG_TEST_FLASH_RW;
						break;

					case MENU_TEST_ERASE_READ:
						RunTestMask |= INTG_TEST_ERASE_READ;
						break;

					case MENU_TEST_RANDOM_RW:
						RunTestMask |= INTG_TEST_RANDOM_RW;
						break;

					case MENU_TEST_SPAREBYTES_RW:
						RunTestMask |=
						INTG_TEST_SPAREBYTES_RW;
						break;

					case MENU_TEST_PARTIAL_RW:
						RunTestMask |=
						INTG_TEST_PARTIAL_RW;
						break;

					case MENU_TEST_ECC:
						RunTestMask |=
						INTG_TEST_ECC;
						break;

					case MENU_TEST_BBT:
						RunTestMask |=
						INTG_TEST_BBT;
						break;

					case MENU_TEST_MARK_BLOCK_BAD:
						RunTestMask |=
						INTG_TEST_MARK_BLOCK_BAD;
						break;

					case MENU_TEST_EXIT:
						QuitToMain = 1;
						break;

					default:
						printf("Unknown test id	%s\r\n",
							Token);
				}
			}

			Token = strtok_r(0, " ", &tmp);
		}

		/*
		 * Execute selected tests
		 */
		if (QuitToMain == 1) break;

		printf("\r\n\r\n");
		if (RunTestMask & INTG_TEST_ERASE_READ) {
			TestFailures += Intg_EraseReadTest(NandInstPtr, TestLoops);
		}

		if (RunTestMask & INTG_TEST_FLASH_RW) {
			TestFailures += Intg_FlashRWTest(NandInstPtr, TestLoops);
		}

		if (RunTestMask & INTG_TEST_RANDOM_RW) {
			TestFailures += Intg_RandomRWTest(NandInstPtr, TestLoops);
		}

		if (RunTestMask & INTG_TEST_SPAREBYTES_RW) {
			TestFailures += Intg_SpareBytesRWTest(NandInstPtr, TestLoops);
		}

		if (RunTestMask & INTG_TEST_PARTIAL_RW) {
			TestFailures += Intg_PartialRWTest(NandInstPtr, TestLoops);
		}

		if (RunTestMask & INTG_TEST_ECC) {
			TestFailures += Intg_EccTest(NandInstPtr, TestLoops);
		}

		if (RunTestMask & INTG_TEST_BBT) {
			TestFailures += Intg_BbtTest(NandInstPtr, TestLoops);
		}

		if (RunTestMask & INTG_TEST_MARK_BLOCK_BAD) {
			TestFailures += Intg_MarkBlockBadTest(NandInstPtr, TestLoops);
		}

		printf("************************************************\r\n");
		if (TestFailures) {
			printf("* %d test FAILURE(s) recorded\r\n", \
			TestFailures);
		} else {
			printf("* Tests pass\r\n");
		}
		printf("*********************************************\r\n");
	}
}

/*****************************************************************************/
/**
 *
 * Prompt the user for a selection in the utility sub-menu
 *
 * @param	CmdLine - Storage to use for User input
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
static void RunUtilMenu(char* CmdLine)
{
	int QuitToMain = 0;
	int Status = XST_FAILURE;

	while(!QuitToMain) {
		/* prompt user */
		printf("\r\n\r\n\r\nUtil Menu:\r\n");
		printf("==========\r\n");
		printf("%d - Change Data Interface Timing mode- 0 for SDR, 1 for NVDDR"
				"\r\n",MENU_UTIL_DATA_INTERFACE);
		printf("%d - Print Flash Details\r\n",
				MENU_UTIL_FLASH_DETAILS);

		/* wait for input */
		*CmdLine = '\n';
		while(*CmdLine == '\n') {
			printf("Enter selection: ");
			GetUserInput(0, CmdLine, 131);
		}

		/* execute selection */
		switch(atoi(CmdLine)) {
			case MENU_UTIL_DATA_INTERFACE:
				printf("Type 0 for SDR, 1 for NVDDR\r\n");
				/* wait for input */
				*CmdLine = '\n';
				while(*CmdLine == '\n') {
					printf("Enter selection: ");
					GetUserInput(0, CmdLine, 131);
				}
				switch(atoi(CmdLine)) {
					case SDR:
						printf("Enter Timing Mode : 0 - 5 for SDR Interface : ");
						/* wait for input */
						*CmdLine = '\n';
						while(*CmdLine == '\n') {
							GetUserInput(0, CmdLine, 131);
						}
						if((u8)atoi(CmdLine) >= 0 && (u8)atoi(CmdLine) <= 5){
							Status = ChangeTimingMode(NandInstPtr,SDR,(u8)atoi(CmdLine));
							if (Status != XST_SUCCESS) {
								printf("Data Interface / Timing Mode Change"
										" failed\r\n");
							}

						}
						else{
							printf("Invalid Input\n\r");
						}
						break;

					case NVDDR:
						printf("Enter Timing Mode : 0 - 5 for NVDDR Interface : ");
						/* wait for input */
						*CmdLine = '\n';
						while(*CmdLine == '\n') {
							GetUserInput(0, CmdLine, 131);
						}
						if((u8)atoi(CmdLine) >= 0 && (u8)atoi(CmdLine) <= 5){
							Status = ChangeTimingMode(NandInstPtr,NVDDR,(XNandPs8_TimingMode)atoi(CmdLine));
							if (Status != XST_SUCCESS) {
								printf("Data Interface / Timing Mode Change"
										" failed\r\n");
							}
						}
						else{
							printf("Invalid Input\n\r");
						}
						break;

					default:
						printf("Invalid selection\r\n");
				}
				break;

			case MENU_UTIL_FLASH_DETAILS:
				xil_printf("Bytes Per Page: 0x%x\r\n",
						NandInstPtr->Geometry.BytesPerPage);
				xil_printf("Spare Bytes Per Page: 0x%x\r\n",
						NandInstPtr->Geometry.SpareBytesPerPage);
				xil_printf("Pages Per Block: 0x%x\r\n",
						NandInstPtr->Geometry.PagesPerBlock);
				xil_printf("Blocks Per LUN: 0x%x\r\n",
						NandInstPtr->Geometry.BlocksPerLun);
				xil_printf("Number of LUNs: 0x%x\r\n",
						NandInstPtr->Geometry.NumLuns);
				xil_printf("Number of bits per cell: 0x%x\r\n",
						NandInstPtr->Geometry.NumBitsPerCell);
				xil_printf("Number of ECC bits: 0x%x\r\n",
						NandInstPtr->Geometry.NumBitsECC);
				xil_printf("Block Size: 0x%x\r\n",
						NandInstPtr->Geometry.BlockSize);
				xil_printf("Number of Target Blocks: 0x%x\r\n",
						NandInstPtr->Geometry.NumTargetBlocks);
				xil_printf("Number of Target Pages: 0x%x\r\n",
						NandInstPtr->Geometry.NumTargetPages);
				break;

			case MENU_UTIL_EXIT:
				QuitToMain = 1;
				break;

			default:
				printf("Invalid selection\r\n");
		}
	}
}

/****************************************************************************/
/**
*
* This functions receives a single byte using the UART. It is non-blocking in
* that it returns if there is no data received.
*
* @param	Data will have the received data after this function returns
*		XST_SUCCESS
*
* @return	XST_SUCCESS if received a byte, XST_FAILURE otherwise
*
* @note		None.
*
******************************************************************************/
int UART_RecvByte(u8 *Data)
{
#ifdef UART16550
	if (!XUartNs550_IsReceiveData(UART_BASEADDR)) {
		return XST_FAILURE;
	} else {
		*Data = XUartNs550_ReadReg(UART_BASEADDR, XUN_RBR_OFFSET);
		return XST_SUCCESS;
	}

#elif UARTLITE
	if(XUartLite_IsReceiveEmpty(UART_BASEADDR)) {
		return XST_FAILURE;
	} else {
		*Data = (u8)Xil_In32(UART_BASEADDR + XUL_RX_FIFO_OFFSET);
		return XST_SUCCESS;
	}
#else
	if(!XUartPs_IsReceiveData(UART_BASEADDR)) {
		return XST_FAILURE;
	} else {
		*Data = (u8)XUartPs_ReadReg(UART_BASEADDR,
				XUARTPS_FIFO_OFFSET);
		return XST_SUCCESS;
	}
#endif
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function initialize the Nand flash.
*
* @param	NandDeviceId is is the XPAR_<NAND_instance>_DEVICE_ID value
*		from xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note
*		None
*
****************************************************************************/
s32 FlashInit(u16 NandDeviceId){

	s32 Status = XST_FAILURE;

	Config = XNandPs8_LookupConfig(NandDeviceId);
	if (Config == NULL) {
		Status = XST_FAILURE;
		goto Out;
	}

	/*
	 * Initialize the flash driver.
	 */
	Status = XNandPs8_CfgInitialize(NandInstPtr, Config,
			Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		goto Out;
	}

Out:
	return Status;

}

/*****************************************************************************/
/**
*
* This function changes the data interface and timing mode.
*
* @param	NandInstPtr is a pointer to the XNandPs8 instance.
* @param	NewIntf is the new data interface.
* @param	NewMode is the new timing mode.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None
*
******************************************************************************/
static int ChangeTimingMode(XNandPs8 *NandInstPtr,
				XNandPs8_DataInterface NewIntf,
				XNandPs8_TimingMode NewMode){

	s32 Status = XST_FAILURE;
	u32 t_mode = NewMode;
	u32 RegVal = 0U;
	u32 feature = 0U;
	u32 ddr_mode[2] = {0U};

	if (NewIntf == SDR){
		NandInstPtr->DataInterface = SDR;

		/*
		 * Set Timing mode
		 */
		Status = XNandPs8_SetFeature(NandInstPtr, 0, 0x1, &t_mode);
		if (Status != XST_SUCCESS) {
			goto Out;
		}

		/*
		 * Setting Interface and Timing Mode
		 */
		NandInstPtr->TimingMode = t_mode;
	}
	else{
		NandInstPtr->DataInterface = NVDDR;
		ddr_mode[0] = t_mode | 0x10;

		/*
		 * Set Timing mode
		 */
		Status = XNandPs8_SetFeature(NandInstPtr, 0, 0x1, &ddr_mode[0]);
		if (Status != XST_SUCCESS) {
			goto Out;
		}

		/*
		 * Setting Interface and Timing Mode
		 */
		NandInstPtr->TimingMode = ddr_mode[0];
	}

	/*
	 * Setting the Data Interface Register
	 */
	RegVal = ((NewMode % 6U) << ((NewIntf == NVDDR) ? 3U : 0U)) |
			((u32)NewIntf << XNANDPS8_DATA_INTF_DATA_INTF_SHIFT);
	XNandPs8_WriteReg(NandInstPtr->Config.BaseAddress,
				XNANDPS8_DATA_INTF_OFFSET, RegVal);
	/*
	 * Get the Timing mode
	 */
	Status = XNandPs8_GetFeature(NandInstPtr, 0, 0x1, &feature);
	if (Status != XST_SUCCESS) {
		goto Out;
	}
	xil_printf("\tCurrent Interface type : %d, Timing mode is "
			"%d\r\n\n",NandInstPtr->DataInterface,t_mode);

	/*
	 * Check if set_feature was successful
	 */
	if (feature != NandInstPtr->TimingMode) {
		xil_printf("Interface / Mode Changed failed %x = %x\r\n",
				feature,NandInstPtr->TimingMode);
		Status = XST_FAILURE;
		goto Out;
	}

Out:
	return Status;
}

/*****************************************************************************/
/**
 *
 * Main entry function for the whole integration test.
 *
 * param	None.
 *
 * @return	0
 *
 * @note	None.
 *
 *****************************************************************************/
int main()
{
	Intg_Entry();
	return 0;
}
