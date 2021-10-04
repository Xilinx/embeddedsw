/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiomodule_options.c
* @addtogroup iomodule_v2_13
* @{
*
* Contains option functions for the XIOModule driver. These functions allow the
* user to configure an instance of the XIOModule driver.  This file requires
* other files of the component to be linked in also.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------
* 1.00a sa   07/15/11 First release
* 2.4   mi   09/20/16 Fixed compilation warnings
* 2.13	sk   10/04/21 Update functions return type to fix misra-c violation.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xiomodule.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/*
 * The following data type maps an option to a register mask such that getting
 * and setting the options may be table driven.
 */
typedef struct {
	u32 Option;
	u32 Mask;
} Mapping;


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/*
 * Create the table which contains options which are to be processed to get/set
 * the options. These options are table driven to allow easy maintenance and
 * expansion of the options.
 */
static Mapping OptionsTable[] = {
	{XTC_INT_MODE_OPTION, 0},
	{XTC_AUTO_RELOAD_OPTION, XTC_CSR_AUTO_RELOAD_MASK}
};

/* Create a constant for the number of entries in the table */
#define XTC_NUM_OPTIONS   (sizeof(OptionsTable) / sizeof(Mapping))


/*****************************************************************************/
/**
*
* Set the options for the interrupt controller driver.
*
* @param	InstancePtr is a pointer to the XIOModule instance to be
*               worked on.
* @param	Options to be set. The available options are described in
*		xiomodule.h.
*
* @return
* 		- XST_SUCCESS if the options were set successfully
* 		- XST_INVALID_PARAM if the specified option was not valid
*
* @note		None.
*
****************************************************************************/
s32 XIOModule_SetOptions(XIOModule * InstancePtr, u32 Options)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Make sure option request is valid
	 */
	if ((Options == XIN_SVC_SGL_ISR_OPTION) ||
	    (Options == XIN_SVC_ALL_ISRS_OPTION)) {
		InstancePtr->CfgPtr->Options = Options;
		return XST_SUCCESS;
	}
	else {
		return XST_INVALID_PARAM;
	}
}

/*****************************************************************************/
/**
*
* Return the currently set options.
*
* @param	InstancePtr is a pointer to the XIOModule instance to be
*               worked on.
*
* @return	The currently set options. The options are described in
*               xiomodule.h.
*
* @note		None.
*
****************************************************************************/
u32 XIOModule_GetOptions(XIOModule * InstancePtr)
{
	/*
	 * Assert the arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return InstancePtr->CfgPtr->Options;
}


/*****************************************************************************/
/**
*
* Enables the specified options for the specified timer . This function
* sets the options without regard to the current options of the driver. To
* prevent a loss of the current options, the user should call
* XIOModule_Timer_GetOptions() prior to this function and modify the retrieved
* options to pass into this function to prevent loss of the current options.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	TimerNumber is the timer of the device to operate on.
*		Each device may contain multiple timers. The timer
*		number is a zero based number with a range of
*		0 to (XTC_DEVICE_TIMER_COUNT - 1).
* @param	Options contains the desired options to be set or cleared.
*		Setting the option to '1' enables the option, clearing the to
*		'0' disables the option. The options are bit masks such that
*		multiple options may be set or cleared. The options are
*		described in xiomodule.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XIOModule_Timer_SetOptions(XIOModule * InstancePtr, u8 TimerNumber,
                                u32 Options)
{
	u32 CounterControlReg = 0;
	u32 TimerOffset = TimerNumber << XTC_TIMER_COUNTER_SHIFT;
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(TimerNumber <= XTC_DEVICE_TIMER_COUNT);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Loop through the Options table, turning the enable on or off
	 * depending on whether the bit is set in the incoming Options flag.
	 */
	for (Index = 0; Index < XTC_NUM_OPTIONS; Index++) {
		if (Options & OptionsTable[Index].Option) {
			/*
			 * Turn the option on
			 */
			CounterControlReg |= OptionsTable[Index].Mask;
		}
		else {
			/*
			 * Turn the option off
			 */
			CounterControlReg &= ~OptionsTable[Index].Mask;
		}


	}

	/*
	 * Write out the updated value to the actual register
	 */
	XIOModule_WriteReg(InstancePtr->BaseAddress,
			   TimerOffset + XTC_TCSR_OFFSET,
			   CounterControlReg);
	InstancePtr->CurrentTCSR[TimerNumber] = CounterControlReg;
}

/*****************************************************************************/
/**
*
* Get the options for the specified timer.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	TimerNumber is the timer of the device to operate on
*		Each device may contain multiple timer. The timer
*		number is a zero based number with a range of
*		0 to (XTC_DEVICE_TIMER_COUNT - 1).
*
* @return
*
* The currently set options. An option which is set to a '1' is enabled and
* set to a '0' is disabled. The options are bit masks such that multiple
* options may be set or cleared. The options are described in xiomodule.h.
*
* @note		None.
*
******************************************************************************/
u32 XIOModel_Timer_GetOptions(XIOModule * InstancePtr, u8 TimerNumber)
{
	u32 Options = 0;
	u32 CounterControlReg;
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(TimerNumber < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Get the current contents of the control status register to allow
	 * the current options to be determined
	 */
	CounterControlReg = InstancePtr->CurrentTCSR[TimerNumber];

	/*
	 * Loop through the Options table, turning the enable on or off
	 * depending on whether the bit is set in current register settings.
	 */
	for (Index = 0; Index < XTC_NUM_OPTIONS; Index++) {
		if (CounterControlReg & OptionsTable[Index].Mask) {
			Options |= OptionsTable[Index].Option;	/* turn on */
		}
		else {
			Options &= ~OptionsTable[Index].Option;	/* turn off */
		}
	}

	return Options;
}
/** @} */
