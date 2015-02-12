/******************************************************************************
*
* (c) Copyright 2010-2014 Xilinx, Inc. All rights reserved.
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
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
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
/*****************************************************************************/
/**
*
* @file ct_standalone.c
*
* Implements Code test (CT) utility driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who  Date     Changes
* --- ---- -------- -----------------------------------------------
* 1.0 rmm  12/23/03 First release
* 1.1 xd   03/16/04 Changed to support PPC405 without OS.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include "ct.h"
#include "intg.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

unsigned CT_TotalFailures;	/**< Total failures flag */
unsigned CT_TestFailures;	/**< Test failures flag */
unsigned CT_TestPass;		/**< Test pass flag */

/*****************************************************************************/
/**
*
* Initialize the CT subsystem.
*
* @return
*
* Nothing
*
******************************************************************************/
void CT_Init(void)
{
	CT_TestFailures = 0;
	CT_TotalFailures = 0;
	CT_TestPass = 0;
}


/*****************************************************************************/
/**
*
* Reset for a new test and display a test specific message. This involves:
*
*   - Zero out test failure counter.
*   - Print a message specified by the fmt parameter and its associated
*     arguments.
*
* @param    fmt is a "printf" style format string.
* @param    ... is a variable number of arguments that match "fmt"
*
* @return   Nothing
*
******************************************************************************/
void CT_TestReset(char *fmt, ...)
{
	va_list args;

	CT_TestFailures = 0;
	CT_TestPass = 1;

	/* Print out the format and its associated argument list */
	va_start(args, fmt);
	printf(fmt, args);
	CT_Message("\r\n=====================================================");

	va_end(args);
	CT_Message("\r\n");
}


/*****************************************************************************/
/**
*
* Display current test pass number to user
*
* @return
*
* Nothing
*
******************************************************************************/
void CT_NotifyNextPass(void)
{
	printf("\r\n=====>Pass %d\r\n\r\n", CT_TestPass);
	CT_TestPass++;
}


/*****************************************************************************/
/**
*
* Log a test failure. This involves incrementing test failure and total test
* failure counters, and displaying test specific message.
*
* @param    fmt is a "printf" style format string.
* @param    ... is a variable number of arguments that match "fmt"
*
* @return   Nothing
*
******************************************************************************/
void CT_LogFailure(char* fmt, ...)
{
	va_list args;

	/*
	 * Increment failure counters
	 */
	CT_TestFailures++;
	CT_TotalFailures++;

	/*
	 * Print out the format and its associated argument list.
	 */
	va_start(args, fmt);

	printf(fmt, args);
	va_end(args);
}


/*****************************************************************************/
/**
*
* Log a test failure. This involves incrementing test failure and total test
* failure counters, and displaying test specific message.
*
* param    fmt is a "printf" style format string.
* param    ... is a variable number of arguments that match "fmt"
*
* @return   Nothing
*
******************************************************************************/
void CT_LogFailure2()
{
	/*
	 * Increment failure counters
	 */
	CT_TestFailures++;
	CT_TotalFailures++;
}


/*****************************************************************************/
/**
*
* Return the number of test failures since CT_TestReset() was called.
*
* @return   Number of failures logged.
*
******************************************************************************/
unsigned CT_GetTestFailures(void)
{
	return(CT_TestFailures);
}


/*****************************************************************************/
/**
*
* Return the number of test failures since CT_Init() was called.
*
* @return   Total number of failures logged.
*
******************************************************************************/
unsigned CT_GetTotalFailures(void)
{
	return(CT_TotalFailures);
}


/*****************************************************************************/
/**
 * Return status of the Xilinx driver assert mechanism.
 *
 * @return
 *
 * 1 if asserts are disabled, 0 otherwise.
 *****************************************************************************/
int CT_IsAssertDisabled(void)
{
#ifdef NDEBUG
	return(1);
#else
	return(0);
#endif
}


/*****************************************************************************/
/**
*
* Print a message based on the given format string and parameters.
*
* @param fmt is a "printf" style format string.
* @param ... is a variable number of arguments that match "fmt"
*
* @return
*
* Nothing
*
******************************************************************************/
void CT_Message(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);

	printf(fmt, args);

	va_end(args);
}


/*****************************************************************************/
/**
*
* Set a series of bytes in memory to a specific value
*
* @param Address is the start address in memory to write
* @param Value is the value to set memory to
* @param Bytes is the number of bytes to set
*
* @return
*
* Nothing
*
******************************************************************************/
void CT_MemSet(void *Address, char Value, unsigned Bytes)
{
	char* Ptr = Address;

	while(Bytes--)
	{
	*Ptr++ = Value;
	}
}
