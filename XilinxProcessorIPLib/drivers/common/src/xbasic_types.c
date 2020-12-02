/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbasic_types.c
* @addtogroup common_v1_1
* @{
*
* This file contains basic functions for Xilinx software IP.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rpm  11/07/03 Added XNullHandler function as a stub interrupt handler
* 1.00a xd   11/03/04 Improved support for doxygen.
* 1.00a bss  13/01/12 Removed a compiler warning in XNullHandler;
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xbasic_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/**
 * This variable allows testing to be done easier with asserts. An assert
 * sets this variable such that a driver can evaluate this variable
 * to determine if an assert occurred.
 */
unsigned int XAssertStatus;

/**
 * This variable allows the assert functionality to be changed for testing
 * such that it does not wait infinitely. Use the debugger to disable the
 * waiting during testing of asserts.
 */
int XWaitInAssert = TRUE;

/* The callback function to be invoked when an assert is taken */
static XAssertCallback XAssertCallbackRoutine = (XAssertCallback) NULL;

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* Implements assert. Currently, it calls a user-defined callback function
* if one has been set.  Then, it potentially enters an infinite loop depending
* on the value of the XWaitInAssert variable.
*
* @param    File is the name of the filename of the source
* @param    Line is the linenumber within File
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XAssert(char *File, int Line)
{
	/* if the callback has been set then invoke it */
	if (XAssertCallbackRoutine != NULL) {
		(*XAssertCallbackRoutine) (File, Line);
	}

	/* if specified, wait indefinitely such that the assert will show up
	 * in testing
	 */
	while (XWaitInAssert) {
	}
}

/*****************************************************************************/
/**
*
* Sets up a callback function to be invoked when an assert occurs. If there
* was already a callback installed, then it is replaced.
*
* @param    Routine is the callback to be invoked when an assert is taken
*
* @return   None.
*
* @note     This function has no effect if NDEBUG is set
*
******************************************************************************/
void XAssertSetCallback(XAssertCallback Routine)
{
	XAssertCallbackRoutine = Routine;
}


/*****************************************************************************/
/**
*
* Null handler function. This follows the XInterruptHandler signature for
* interrupt handlers. It can be used to assign a null handler (a stub) to an
* interrupt controller vector table.
*
* @param    NullParameter is an arbitrary void pointer and not used.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XNullHandler(void *NullParameter)
{
 (void) NullParameter;
}
/** @} */
