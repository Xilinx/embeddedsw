/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xttcps.c
* @addtogroup ttcps Overview
* @{
*
* This file contains the implementation of the XTtcPs driver. This driver
* controls the operation of one timer counter in the Triple Timer Counter (TTC)
* module in the Ps block. Refer to xttcps.h for more detailed description
* of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------
* 1.00a drg/jz 01/21/10 First release
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.01	pkp	   01/30/16 Modified XTtcPs_CfgInitialize to add XTtcps_Stop
*						to stop the timer before configuring
* 3.2   mus    10/28/16 Modified XTtcPs_CalcIntervalFromFreq to calculate
*                       32 bit interval count for zynq ultrascale+mpsoc
* 3.5   srm    10/06/17 Updated XTtcPs_GetMatchValue and XTtcPs_SetMatchValue
*                       APIs to use correct match register width for zynq
*                       (i.e. 16 bit) and zynq ultrascale+mpsoc (i.e. 32 bit).
*                       It fixes CR# 986617
* 3.6   srm    04/25/18 Corrected the Match register initialization in
						XTtcPs_CfgInitialize API.
* 3.7   mus    09/20/18 Modified XTtcPs_CalcIntervalFromFreq API to use
*						XTTCPS_MAX_INTERVAL_COUNT instead of hardcoding
*						MAX interval count to 16 bit value(i.e.65532),
*						which is incorrect for  zynq ultrascale+mpsoc
*						(i.e. max interval count is 32 bit).
* 3.10  aru    05/06/19 Added assert check for driver instance and freq
*			parameter in  XTtcPs_CalcIntervalFromFreq().
* 3.10  aru    05/30/19 Added interrupt handler to clear ISR
* 3.18  gm    06/26/23	Added PM Request node support.
* 3.18  gm    07/17/23	Added PM Release node support.
* 3.18  ml    09/08/23  Updated code by using ternary operator
*                       to fix MISRA-C violation for Rule 10.3
* 3.18  ml    09/08/23  Replaced TRUE with Numerical value to fix
*                       MISRA-C violation for Rule 10.5
* 3.18  ml    09/07/23  Removed XTtcPs_ClearInterruptStatus function call to avoid the
*                       the same operation for 2 times.
* 3.18  ml    09/07/23  Added U to numerical to fix MISRA-C violation for Rule 10.4
* 3.18  ml    09/08/23  Typecast with u32 to fix MISRA-C violation for Rule 12.2 and 10.7
* 3.18  ml    09/08/23  Added comments to fix HIS COMF violations.
* 3.21  ml    04/03/25  Added support for interrupt handling when a single interrupt
*                       is used for all three counters.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xttcps.h"
#include "xparameters.h"
#if defined  (XPM_SUPPORT)
#include "pm_defs.h"
#include "pm_api_sys.h"
#include "xil_types.h"
#include "pm_client.h"
#include "xpm_init.h"
#include "xdebug.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void StubStatusHandler(const void *CallBackRef, u32 StatusEvent);
static u32 GetIndexFromBaseAddr(u32 BaseAddress);
/************************** Variable Definitions *****************************/

static XTtcPs_StatusHandlerTableEntry StatusHandlerTable[XPAR_XTTCPS_NUM_INSTANCES];
extern XTtcPs_Config XTtcPs_ConfigTable[XPAR_XTTCPS_NUM_INSTANCES];

#if defined  (XPM_SUPPORT)
/*
 * Instance - counters list
 * Ttc0     - 0 to 2
 * Ttc1     - 3 to 5
 * Ttc2     - 6 to 8
 * Ttc3     - 9 to 11
 *
 * TtcNodeState is an array which holds the present state of the counter
 * in it's corresponding index value. Index value will be incremented
 * during ttc counter request node and decremented during release node.
 *
 */
static u32 TtcNodeState[XPAR_XTTCPS_NUM_INSTANCES];

static u32 GetTtcNodeAddress(u16 DeviceId)
{
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XTTCPS_NUM_INSTANCES; Index++) {
		if (XTtcPs_ConfigTable[Index].DeviceId == DeviceId) {
			return XTtcPs_ConfigTable[Index].BaseAddress;
		}
	}
	return 0;
}
#endif

static u32 GetIndexFromBaseAddr(u32 BaseAddress) {
        u32 Index;

        for (Index = 0U; Index < XPAR_XTTCPS_NUM_INSTANCES; Index++) {
                if ((XTtcPs_ConfigTable[Index].BaseAddress == BaseAddress) ||
                     !BaseAddress) {
                        break;
                }
        }
        return Index;
}

/*****************************************************************************/
/**
*
* Initializes a specific XTtcPs instance such that the driver is ready to use.
* This function initializes a single timer counter in the triple timer counter
* function block.
*
* The state of the device after initialization is:
*  - Overflow Mode
*  - Internal (pclk) selected
*  - Counter disabled
*  - All Interrupts disabled
*  - Output waveforms disabled
*
* @param	InstancePtr is a pointer to the XTtcPs instance.
* @param	ConfigPtr is a reference to a structure containing information
*		about a specific TTC device.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. The caller is responsible for keeping the address
*		mapping from EffectiveAddr to the device physical base address
*		unchanged once this function is invoked. Unexpected errors may
*		occur if the address mapping changes after this function is
*		called. If address translation is not used, then use
*		ConfigPtr->BaseAddress for this parameter, passing the physical
*		address instead.
*
* @return
*
* 		- XST_SUCCESS if the initialization is successful.
*		- XST_DEVICE_IS_STARTED if the device is started. It must be
*		  stopped to re-initialize.
*
* @note		Device has to be stopped first to call this function to
*		initialize it.
*
******************************************************************************/
s32 XTtcPs_CfgInitialize(XTtcPs *InstancePtr, XTtcPs_Config *ConfigPtr,
			 u32 EffectiveAddr)
{
	s32 Status;
	u32 IsStartResult;
#if defined  (XPM_SUPPORT)
	u32 TtcNodeAddr;
#endif
#ifdef SDT
	u16 Count;
#endif

	/*
	 * Assert to validate input arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

#if defined  (XPM_SUPPORT)
	TtcNodeAddr = GetTtcNodeAddress((ConfigPtr->DeviceId / 3) * 3);

	Status = XPm_RequestNode(XpmGetNodeId((UINTPTR)TtcNodeAddr), PM_CAP_ACCESS, MAX_QOS, REQUEST_ACK_BLOCKING);
	if (XST_SUCCESS != Status) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Ttc: XPm_RequestNode failed\r\n");
		return Status;
	}

	Status = XPm_ResetAssert(XpmGetResetId((UINTPTR)TtcNodeAddr), XILPM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Ttc: XPm_ResetAssert() ERROR=0x%x \r\n", Status);
		return Status;
	}

	TtcNodeState[ConfigPtr->DeviceId]++;
#endif

	/*
	 * Set some default values
	 */
#ifndef SDT
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
#endif
	InstancePtr->Config.BaseAddress = EffectiveAddr;
	InstancePtr->Config.InputClockHz = ConfigPtr->InputClockHz;
	InstancePtr->StatusHandler = StubStatusHandler;
#if defined(XIL_INTERRUPT) && !defined(SDT)
	InstancePtr->Config.IntrId = ConfigPtr->IntrId;
	InstancePtr->Config.IntrParent = ConfigPtr->IntrParent;
#endif

#ifdef SDT
	for (Count = 0; Count < XTTCPS_NUM_COUNTERS; Count++) {
		InstancePtr->Config.IntrId[Count] = ConfigPtr->IntrId[Count];
	}
	InstancePtr->Config.IntrParent = ConfigPtr->IntrParent;
#endif

	IsStartResult = XTtcPs_IsStarted(InstancePtr) ? 1U : 0U;
	/*
	 * If the timer counter has already started, return an error
	 * Device should be stopped first.
	 */
	if (IsStartResult == 1U) {
		Status = XST_DEVICE_IS_STARTED;
	} else {

		/*
		 * stop the timer before configuring
		 */
		XTtcPs_Stop(InstancePtr);
		/*
		 * Reset the count control register to it's default value.
		 */
		XTtcPs_WriteReg(InstancePtr->Config.BaseAddress,
				XTTCPS_CNT_CNTRL_OFFSET,
				XTTCPS_CNT_CNTRL_RESET_VALUE);

		/*
		 * Reset the rest of the registers to the default values.
		 */
		XTtcPs_WriteReg(InstancePtr->Config.BaseAddress,
				XTTCPS_CLK_CNTRL_OFFSET, 0x00U);
		XTtcPs_WriteReg(InstancePtr->Config.BaseAddress,
				XTTCPS_INTERVAL_VAL_OFFSET, 0x00U);
		XTtcPs_WriteReg(InstancePtr->Config.BaseAddress,
				XTTCPS_MATCH_0_OFFSET, 0x00U);
		XTtcPs_WriteReg(InstancePtr->Config.BaseAddress,
				XTTCPS_MATCH_1_OFFSET, 0x00U);
		XTtcPs_WriteReg(InstancePtr->Config.BaseAddress,
				XTTCPS_MATCH_2_OFFSET, 0x00U);
		XTtcPs_WriteReg(InstancePtr->Config.BaseAddress,
				XTTCPS_IER_OFFSET, 0x00U);
		XTtcPs_WriteReg(InstancePtr->Config.BaseAddress,
				XTTCPS_ISR_OFFSET, XTTCPS_IXR_ALL_MASK);

		InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

		/*
		 * Reset the counter value
		 */
		XTtcPs_ResetCounterValue(InstancePtr);
		Status = XST_SUCCESS;
	}
	return Status;
}

#if defined  (XPM_SUPPORT)
static u32 CheckTtcNodeState(u16 TtcNodeId)
{
	u8 IdOffset;
	u16 TtcBaseNodeId;
	u32 State = FALSE;

	TtcBaseNodeId = ((TtcNodeId / 3) * 3);

	for (IdOffset = TtcBaseNodeId; IdOffset < (TtcBaseNodeId + 3); IdOffset++) {
		if (TtcNodeState[IdOffset] == 0) {
			continue;
		} else {
			if (IdOffset == TtcNodeId) {
				if ((TtcNodeState[IdOffset] - 1) == 0) {
					continue;
				}
			}
			State = TRUE;
			break;
		}
	}
	return State;
}
#endif

/*****************************************************************************/
/**
*
* This routine releases resources of XTtcPs instance/driver.
*
* @param	None
* @return	- XST_SUCCESS if node release was successful
*		- XST_FAILURE if node release was fail, Node won't be released
*		  if any other counter/counters in that TTC in use.
*
* @note		None.
*
******************************************************************************/
u32 XTtcPs_Release(XTtcPs *InstancePtr)
{
	u32 Status = XST_SUCCESS;
#if defined (XPM_SUPPORT)
	u32 TtcNodeAddr;
#endif

	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined (XPM_SUPPORT)
	if (InstancePtr->Config.DeviceId >= XPAR_XTTCPS_NUM_INSTANCES) {
		Status = XST_FAILURE;
	} else {
		/* Stop ttc */
		XTtcPs_Stop(InstancePtr);

		/* Clear interrupt status */
		XTtcPs_ClearInterruptStatus(InstancePtr,
					    XTtcPs_GetInterruptStatus(InstancePtr));

		/* Disable interrupts */
		XTtcPs_DisableInterrupts(InstancePtr, XTTCPS_IXR_ALL_MASK);

		/* Release node, if no other counter in use under that ttc node */
		if (TRUE == CheckTtcNodeState(InstancePtr->Config.DeviceId)) {
			Status = XST_FAILURE;
		} else {
			TtcNodeAddr = GetTtcNodeAddress((InstancePtr->Config.DeviceId / 3) * 3);
			Status = XPm_ReleaseNode(XpmGetNodeId((UINTPTR)TtcNodeAddr));
			TtcNodeState[InstancePtr->Config.DeviceId]--;
		}
	}
#endif
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to set the match registers. There are three match
* registers.
*
* The match 0 register is special. If the waveform output mode is enabled, the
* waveform will change polarity when the count matches the value in the match 0
* register. The polarity of the waveform output can also be set using the
* XTtcPs_SetOptions() function.
*
* @param	InstancePtr is a pointer to the XTtcPs instance.
* @param	MatchIndex is the index to the match register to be set.
*		    Valid values are: 0 - 2.
* @param	Value is the 16-bit value to be set in the match register.
*           Valid Values are: (For Zynq):
*                             0 - ((2^16)-1)
*                             (For Zynq UltraScale + MpSoc) and Versal:
*                             0 - ((2^32) - 1)
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XTtcPs_SetMatchValue(XTtcPs *InstancePtr, u8 MatchIndex, XMatchRegValue Value)
{
	/*
	 * Validate input arguments and in case of error conditions assert.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(MatchIndex < (u8)XTTCPS_NUM_MATCH_REG);

	/*
	 * Write the value to the correct match register with MatchIndex
	 */
	XTtcPs_WriteReg(InstancePtr->Config.BaseAddress,
			XTtcPs_Match_N_Offset(MatchIndex), Value);
}

/*****************************************************************************/
/**
*
* This function is used to get the value of the match registers. There are
* three match registers.
*
* @param	InstancePtr is a pointer to the XTtcPs instance.
* @param	MatchIndex is the index to the match register to be set.
*           There are three match registers are there.
*		    Valid values are: 0 - 2.
*
* @return	The match register value
*
* @note		None
*
****************************************************************************/
XMatchRegValue XTtcPs_GetMatchValue(XTtcPs *InstancePtr, u8 MatchIndex)
{
	u32 MatchReg;

	/*
	 * Validate input arguments and in case of error conditions assert.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(MatchIndex < XTTCPS_NUM_MATCH_REG);

	MatchReg = XTtcPs_ReadReg(InstancePtr->Config.BaseAddress,
				  XTtcPs_Match_N_Offset(MatchIndex));

	return (XMatchRegValue) MatchReg;
}

/*****************************************************************************/
/**
*
* This function sets the prescaler enable bit and if needed sets the prescaler
* bits in the control register.
*
* @param	InstancePtr is a pointer to the XTtcPs instance.
* @param	PrescalerValue is a number from 0-16 that sets the prescaler
*		    to use.
*		    If the parameter is 0 - 15, use a prescaler on the clock of
*		    2^(PrescalerValue+1), or 2-65536.
*		    If the parameter is XTTCPS_CLK_CNTRL_PS_DISABLE, do not use a
*		    prescaler.
*
*		    Valid values are: 0 - 15
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XTtcPs_SetPrescaler(XTtcPs *InstancePtr, u8 PrescalerValue)
{
	u32 ClockReg;

	/*
	 * Assert to validate input arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(PrescalerValue <= XTTCPS_CLK_CNTRL_PS_DISABLE);

	/*
	 * Read the clock control register
	 */
	ClockReg = XTtcPs_ReadReg(InstancePtr->Config.BaseAddress,
				  XTTCPS_CLK_CNTRL_OFFSET);

	/*
	 * Clear all of the prescaler control bits in the register
	 */
	ClockReg &=
		~(XTTCPS_CLK_CNTRL_PS_VAL_MASK | XTTCPS_CLK_CNTRL_PS_EN_MASK);

	if (PrescalerValue < XTTCPS_CLK_CNTRL_PS_DISABLE) {
		/*
		 * Set the prescaler value and enable prescaler
		 */
		ClockReg |= (u32)(((u32)PrescalerValue << (u32)XTTCPS_CLK_CNTRL_PS_VAL_SHIFT) &
				  (u32)XTTCPS_CLK_CNTRL_PS_VAL_MASK);
		ClockReg |= (u32)XTTCPS_CLK_CNTRL_PS_EN_MASK;
	}

	/*
	 * Write the register with the new values.
	 */
	XTtcPs_WriteReg(InstancePtr->Config.BaseAddress,
			XTTCPS_CLK_CNTRL_OFFSET, ClockReg);
}

/*****************************************************************************/
/**
*
* This function gets the input clock prescaler
*
* @param	InstancePtr is a pointer to the XTtcPs instance.
*
* <pre>
* @return	The value(n) from which the prescalar value is calculated
*		    as 2^(n+1). Some example values are given below :
*
* 	Value		Prescaler
* 	0		2
* 	1		4
* 	N		2^(n+1)
* 	15		65536
* 	16		1
*
*           Valid values are: 0 - 16
* </pre>
*
* @note		None.
*
****************************************************************************/
u8 XTtcPs_GetPrescaler(XTtcPs *InstancePtr)
{
	u8 Status;
	u32 ClockReg;

	/*
	 * Assert to validate input arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the clock control register
	 */
	ClockReg = XTtcPs_ReadReg(InstancePtr->Config.BaseAddress,
				  XTTCPS_CLK_CNTRL_OFFSET);

	if (0U == (ClockReg & XTTCPS_CLK_CNTRL_PS_EN_MASK)) {
		/*
		 * Prescaler is disabled. Return the correct flag value
		 */
		Status = (u8)XTTCPS_CLK_CNTRL_PS_DISABLE;
	} else {

		Status = (u8)((ClockReg & (u32)XTTCPS_CLK_CNTRL_PS_VAL_MASK) >>
			      (u32)XTTCPS_CLK_CNTRL_PS_VAL_SHIFT);
	}
	return Status;
}

/*****************************************************************************/
/**
*
* This function calculates the interval value as well as the prescaler value
* for a given frequency.
*
* @param	InstancePtr is a pointer to the XTtcPs instance.
* @param	Freq is the requested output frequency for the device.
*           valid values are: 1 - (2^32)-1
* @param	Interval is the interval value for the given frequency,
*		    it is the output value for this function.
* @param	Prescaler is the prescaler value for the given frequency,
*		    it is the output value for this function.
*
* @return	None.
*
* @note
*  Upon successful calculation for the given frequency, Interval and Prescaler
*  carry the settings for the timer counter; Upon unsuccessful calculation,
*  Interval and Prescaler are set to 0xFF(FF) for their maximum values to
*  signal the caller of failure. Therefore, caller needs to check the return
*  interval or prescaler values for whether the function has succeeded.
*
****************************************************************************/
void XTtcPs_CalcIntervalFromFreq(XTtcPs *InstancePtr, u32 Freq,
				 XInterval *Interval, u8 *Prescaler)
{
	u8 TmpPrescaler;
	UINTPTR TempValue;
	u32 InputClock;

	/*
	 * Assert to validate input arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Freq > 0U);

	InputClock = InstancePtr->Config.InputClockHz;
	/*
	 * Find the smallest prescaler that will work for a given frequency. The
	 * smaller the prescaler, the larger the count and the more accurate the
	 *  PWM setting.
	 */
	TempValue = InputClock / Freq;

	if (TempValue < 4U) {
		/*
		 * The frequency is too high, it is too close to the input
		 * clock value. Use maximum values to signal caller.
		 */
		*Interval = XTTCPS_MAX_INTERVAL_COUNT;
		*Prescaler = 0xFFU;
		return;
	}

	/*
	 * First, do we need a prescaler or not?
	 */
	if (((UINTPTR)XTTCPS_MAX_INTERVAL_COUNT) > TempValue) {
		/*
		 * We do not need a prescaler, so set the values appropriately
		 */
		*Interval = (XInterval)TempValue;
		*Prescaler = XTTCPS_CLK_CNTRL_PS_DISABLE;
		return;
	}


	for (TmpPrescaler = 0U; TmpPrescaler < XTTCPS_CLK_CNTRL_PS_DISABLE;
	     TmpPrescaler++) {
		TempValue =	InputClock / (Freq * ((u32)1U << (TmpPrescaler + 1U)));

		/*
		 * The first value less than 2^16 is the best bet
		 */
		if (((UINTPTR)XTTCPS_MAX_INTERVAL_COUNT) > TempValue) {
			/*
			 * Set the values appropriately
			 */
			*Interval = (XInterval)TempValue;
			*Prescaler = TmpPrescaler;
			return;
		}
	}

	/* Can not find interval values that work for the given frequency.
	 * Return maximum values to signal caller.
	 */
	*Interval = XTTCPS_MAX_INTERVAL_COUNT;
	*Prescaler = 0XFFU;
	return;
}

/*****************************************************************************/
/**
 *
 * Handles interrupts by resetting the counter value
 * and clearing the status register
 *
 * @param	InstancePtr is a pointer to the XTtcPs instance.
 *
 * @return
 *		- XST_SUCCESS if successful.
 *
 * @note	None.
 *
 ******************************************************************************/

u32 XTtcPs_InterruptHandler(XTtcPs *InstancePtr)
{
	u32 XTtcPsStatusReg,Index;
	UINTPTR BaseAddr;

	BaseAddr = InstancePtr->Config.BaseAddress & COUNTER_BASE_ADDRESS_MASK;

	Index = GetIndexFromBaseAddr(BaseAddr);

        /*
         * Check the interrupt status register (ISR) of counter 0
         * If an interrupt is set, call the assigned handler
         */
	if((XTtcPsStatusReg = Xil_In32( BaseAddr + XTTCPS_ISR_OFFSET)) != 0){
		StatusHandlerTable[Index].StatusHandler(StatusHandlerTable[Index].StatusRef, XTtcPsStatusReg);
	}

        /*
         * Check the interrupt status register (ISR) of counter 1
         * If an interrupt is set, call the assigned handler
         */
	if((XTtcPsStatusReg = Xil_In32( BaseAddr + 4 + XTTCPS_ISR_OFFSET)) != 0){
		StatusHandlerTable[Index + 1].StatusHandler(StatusHandlerTable[Index + 1].StatusRef, XTtcPsStatusReg);
	}

        /*
         * Check the interrupt status register (ISR) of counter 2
         * If an interrupt is set, call the assigned handler
         */
	if((XTtcPsStatusReg = Xil_In32( BaseAddr + 8 + XTTCPS_ISR_OFFSET)) != 0){
		StatusHandlerTable[Index + 2].StatusHandler(StatusHandlerTable[Index + 2].StatusRef, XTtcPsStatusReg);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * Sets the status callback function, the status handler, which the driver
 * calls when it encounters conditions that should be reported to upper
 * layer software. The handler executes in an interrupt context, so it must
 * minimize the amount of processing performed. One of the following status
 * events is passed to the status handler.
 *
 * </pre>
 * @param	InstancePtr is a pointer to the XTtcPs instance.
 * @param	CallBackRef is the upper layer callback reference passed back
 *		when the callback function is invoked.
 * @param	FuncPointer is the pointer to the callback function.
 *
 * @return	None.
 *
 * @note
 *
 * The handler is called within interrupt context, so it should do its work
 * quickly and queue potentially time-consuming work to a task-level thread.
 *
 ******************************************************************************/
void XTtcPs_SetStatusHandler(XTtcPs *InstancePtr, void *CallBackRef,
			     XTtcPs_StatusHandler FuncPointer)
{
	u32 Index;
	/*
	 * Validate input arguments and in case of error conditions assert.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FuncPointer != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Index = GetIndexFromBaseAddr(InstancePtr->Config.BaseAddress);
	StatusHandlerTable[Index].StatusHandler = FuncPointer;
	StatusHandlerTable[Index].StatusRef  = CallBackRef;
}

/*****************************************************************************/
/**
 *
 * This is a stub for the status callback. The stub is here in case the upper
 * layers forget to set the handler.
 *
 * @param	CallBackRef is a pointer to the upper layer callback reference
 * @param	StatusEvent is the event that just occurred.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static void StubStatusHandler(const void *CallBackRef, u32 StatusEvent)
{
	(void) CallBackRef;
	(void) StatusEvent;

	Xil_AssertVoidAlways();
}
/** @} */
