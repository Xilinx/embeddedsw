/******************************************************************************
*
*       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2014 Xilinx Inc.
*       All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file	scaler_intg_main.c
*
* This file contains the integration test functions for the SCALER device.
*
* This file contains a list of functions providing the menu driven test
* functionality for various integration tests.
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ------------------------------------------------
* 7.0   adk   22/08/14 First release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "scaler_intgtest.h"
#include <stdlib.h>

/************************** Constant Definitions *****************************/


/************************** Constant Definitions *****************************/

/*
 * Main menu selections.
 */
#define MENU_MAIN_TEST		1   /**<Menu test */
#define MENU_MAIN_EXIT		99  /**<Menu Exit */

/*
 * Test sub-menu selections.
 */
#define MENU_TEST_ALL		0	/**<Menu test all */
#define MENU_TEST_BASIC		1	/**<Menu test basic */
//#define MENU_TEST_POLLED	2	/**<Menu test polled */
#define MENU_TEST_INTR		2	/**<Menu test interrupt */
#define MENU_TEST_EXIT		99	/**<Menu test exit */

/* Create unique bit masks from the test numbers. */
#define SCALER_INTG_TEST_ALL	0xFFFFFFFF /**<SCALER Menu test all */
#define SCALER_INTG_TEST_BASIC	(1 << MENU_TEST_BASIC) /**<Scaler Menu test
							 * basic */
#define SCALER_INTG_TEST_POLLED	(1 << MENU_TEST_POLLED)	/**<Scaler Menu test
							  * polled */
#define SCALER_INTG_TEST_INTR	(1 << MENU_TEST_INTR) /**<Scaler menu test
							* Interrupt */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

static int GetMainMenuCommand(char *CmdLine);
static void RunTestMenu(char *CmdLine);

/************************** Variable Definitions *****************************/

char CmdLine[132];	/**< To read Menu Command arguments */

/*****************************************************************************/
/**
*
* Main entry point for integration tests on Scaler.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main()
{
	int Status;
#ifdef AUTOMATIC_TEST_MODE
	int TestLoops;
	int TestFailures;
#endif

	printf("\nScaler COMPONENT INTEGRATION TEST\n");
	printf("Created on %s\n\n", __DATE__);
	printf("================================\n");

	Status = Scaler_Initialize(&ScalerInst, (u16)SCALER_0_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		return Status;
	}

#ifndef AUTOMATIC_TEST_MODE
	/*
	 * Prompt user for menu selections.
	 */
	while (1) {
		switch (GetMainMenuCommand(CmdLine)) {
			case MENU_MAIN_TEST:
				RunTestMenu(CmdLine);
				break;

			case MENU_MAIN_EXIT:
				printf("\nBye\n");
				return (0);

			default:
				printf("Invalid selections.\n");
		}
	}
#else
	/* set defaults */
	TestLoops = 1;
	TestFailures = 0;

	TestFailures += Scaler_Intg_SelfTest(TestLoops);
/*	TestFailures += Scaler_Intg_PolledTest(TestLoops); */
	TestFailures += Scaler_Intg_InterruptTest(TestLoops);

	if (TestFailures) {
		XIL_FAIL(TestFailures);
	}
	else {
		XIL_PASS();
	}
#endif

	return 0;
}

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
* @note		None.
*
******************************************************************************/
static int GetMainMenuCommand(char *CmdLine)
{
	/* Prompt user. */
	printf("\nMain Menu:\n");
	printf("1  - Test menu\n");
/*	printf("2  - Util menu\n");*/
/*	printf("3  - One of, experimental & misc functions\n");*/
	printf("99 - Exit\n");

	/*
	 * Wait for input.
	 */
	while (!CT_GetUserInput("Enter selection: ", CmdLine,
				sizeof(CmdLine) - 1));
	return (atoi(CmdLine));
}

/*****************************************************************************/
/**
*
* Prompt the user for a sequence of tests to run in the test sub-menu.
*
* @param	CmdLine - Storage to use for User input.
*
* @return	None.
*
* @note		None.
*
 *****************************************************************************/
static void RunTestMenu(char *CmdLine)
{
	char *Token;
	char *tmp;
	int QuitToMain = 0;
	int TestLoops;
	int TestFailures;
	u32 RunTestMask;

	while (!QuitToMain) {
		/*
		 * Set defaults.
		 */
		TestLoops = 1;
		RunTestMask = 0;
		TestFailures = 0;

		/*
		 * Prompt user.
		 */
		printf("\nTest Menu:\n");
		printf("0  - All tests\n");
		printf("1  - Basic Self test\n");
		/* printf("2  - Polled mode tests\n"); */
		printf("2  - Interrupt mode tests\n");
		printf("99 - Exit to main menu\n");
		printf("Adding l<number> sets the number of test loops\n");

		/*
		 * Wait for input.
		 */
		while (!CT_GetUserInput("Enter selection: ", CmdLine,
					sizeof(CmdLine) - 1));

		/*
		 * Parse input line.
		 */
		Token = strtok_r(CmdLine, " ", &tmp);
		while (Token) {
			if ((*Token == 'l') || (*Token == 'L')) {
				TestLoops = atoi(Token + 1);
			}
			else {
				switch (atoi(Token)) {

				case MENU_TEST_ALL:
					RunTestMask |= SCALER_INTG_TEST_ALL;
					break;

				case MENU_TEST_BASIC:
					RunTestMask |= SCALER_INTG_TEST_BASIC;
					break;

			/*	case MENU_TEST_POLLED:
					RunTestMask |= SCALER_INTG_TEST_POLLED;
					break; */

				case MENU_TEST_INTR:
					RunTestMask |= SCALER_INTG_TEST_INTR;
					break;

				case MENU_TEST_EXIT:
					QuitToMain = 1;
					break;

				default:
					printf("Unknown test id %s\n", Token);
				}
			}

			Token = strtok_r(0, " ", &tmp);
		}

		if(!QuitToMain) {
			/*
			 * Execute selected tests.
			 */
			printf("\n********************************************\
					*******\n");
			printf("* Test sequence mask = %08X, TestLoops = %d\n",
					(int) RunTestMask, TestLoops);
			printf("**********************************************\
					*******\n");
			if (RunTestMask & SCALER_INTG_TEST_BASIC) {
				TestFailures +=
					Scaler_Intg_SelfTest(TestLoops);
			}

			/*if (RunTestMask & SCALER_INTG_TEST_POLLED) {
				TestFailures +=
					Scaler_Intg_PolledTest(TestLoops);
			}*/

			if (RunTestMask & SCALER_INTG_TEST_INTR) {
				TestFailures +=
					Scaler_Intg_InterruptTest(TestLoops);
			}

			printf("\n********************************************\
					******\n");
			if (TestFailures) {
				printf("* %d test FAILURES recorded\n", \
					TestFailures);
			}
			else {
				printf("* Tests pass.\n");
			}
			printf("\n********************************************\
					******\n");
		}
	}
}
