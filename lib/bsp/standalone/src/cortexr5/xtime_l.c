/******************************************************************************
*
* Copyright (C) 2014 - 2015 Xilinx, Inc. All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xtime_l.c
*
* This file contains low level functions to get/set time from the Global Timer
* register in the ARM Cortex R5 core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- ---------------------------------------------------
* 5.00 	pkp    08/29/14 First release
* 5.04  pkp	   02/19/16 XTime_StartTimer API is added to configure TTC3 timer
*						when present. XTime_GetTime is modified to give 64bit
*						output using timer overflow when TTC3 present.
*						XTime_SetTime is modified to configure TTC3 counter
*						value when present.
* </pre>
*
* @note		None.
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xtime_l.h"
#include "xpseudo_asm.h"
#include "xil_assert.h"
#include "xil_io.h"

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/
#define RST_LPD_IOU2 0xFF5E0238U
#define RST_LPD_IOU2_TTC3_RESET_MASK	0X00004000U
/************************** Variable Definitions *****************************/
u32 time_high = 0;
/************************** Function Prototypes ******************************/

/* Function definitions are applicable only when TTC3 is present*/
#ifdef SLEEP_TIMER_BASEADDR
/****************************************************************************
*
* Start the TTC timer.
*
* @param	None.
*
* @return	None.
*
* @note		In multiprocessor environment reference time will reset/lost for
*		all processors, when this function called by any one processor.
*
****************************************************************************/
void XTime_StartTimer(void)
{
	u32 reg_lpd_rst, reg_timer_prescalar, reg_timer_cntrl, reg_timer_isr;

	reg_lpd_rst = Xil_In32(RST_LPD_IOU2);
	/* Bring Timer out of reset when under reset*/
	if ((reg_lpd_rst & RST_LPD_IOU2_TTC3_RESET_MASK) != 0 ) {
		reg_lpd_rst = reg_lpd_rst & (~RST_LPD_IOU2_TTC3_RESET_MASK);
		Xil_Out32(RST_LPD_IOU2, reg_lpd_rst);
		goto Config_Timer;
	}

	reg_timer_cntrl = Xil_In32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_CNTR_CNTRL_OFFSET);
	/* check if Timer is disabled */
	if ((reg_timer_cntrl & SLEEP_TIMER_COUNTER_CONTROL_DIS_MASK)!=0)
		goto Config_Timer;
	else
		return;

	reg_timer_prescalar = Xil_In32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_CLK_CNTRL_OFFSET);
	reg_timer_isr = Xil_In32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_IER_OFFSET);

	/* check if the timer is configured with proper functionalty for sleep */
	if ((((reg_timer_cntrl & SLEEP_TIMER_COUNTER_CONTROL_DIS_MASK) == 0) &
			(((reg_timer_prescalar & SLEEP_TIMER_CLOCK_CONTROL_PS_EN_MASK) != 0) |
				((reg_timer_isr & SLEEP_TIMER_INTERRUPT_REGISTER_OV_MASK) == 0))) != 0)
					goto Config_Timer;
	else
		return;

Config_Timer:
	/* Disable the timer to configure */
	reg_timer_cntrl = Xil_In32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_CNTR_CNTRL_OFFSET);
	reg_timer_cntrl = reg_timer_cntrl | SLEEP_TIMER_COUNTER_CONTROL_DIS_MASK;
	Xil_Out32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_CNTR_CNTRL_OFFSET, reg_timer_cntrl);

	/* Disable the prescalar */
	reg_timer_prescalar = Xil_In32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_CLK_CNTRL_OFFSET);
	reg_timer_prescalar = reg_timer_prescalar & (~SLEEP_TIMER_CLOCK_CONTROL_PS_EN_MASK);
	Xil_Out32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_CLK_CNTRL_OFFSET , reg_timer_prescalar);

	/* Enable the Overflow interrupt */
	reg_timer_isr = Xil_In32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_IER_OFFSET);
	reg_timer_isr = reg_timer_isr | SLEEP_TIMER_INTERRUPT_REGISTER_OV_MASK;
	Xil_Out32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_IER_OFFSET, reg_timer_isr);

	/* Enable the Timer */
	reg_timer_cntrl = SLEEP_TIMER_COUNTER_CONTROL_RST_MASK & (~SLEEP_TIMER_COUNTER_CONTROL_DIS_MASK);
	Xil_Out32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_CNTR_CNTRL_OFFSET, reg_timer_cntrl);

}
/****************************************************************************
*
* Set the time in the Timer Counter Register.
*
* @param	Value to be written to the Timer Counter Register.
*
* @return	None.
*
* @note		In multiprocessor environment reference time will reset/lost for
*		all processors, when this function called by any one processor.
*
****************************************************************************/
void XTime_SetTime(XTime Xtime_Global)
{
	u32 reg;
	/* Disable the timer to configure */
	reg = Xil_In32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_CNTR_CNTRL_OFFSET);
	reg = reg | SLEEP_TIMER_COUNTER_CONTROL_DIS_MASK;
	Xil_Out32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_CNTR_CNTRL_OFFSET, reg);

	/* Write the lower 32bit value to timer counter register */
	Xil_Out32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_CNTR_VAL_OFFSET, (u32)Xtime_Global);
	/* store upper 32bit value to variable high*/
	time_high = (u32)((u32)(Xtime_Global>>32U));

	/* Enable the Timer */
	reg = Xil_In32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_CNTR_CNTRL_OFFSET);;
	reg = reg & (~SLEEP_TIMER_COUNTER_CONTROL_DIS_MASK);
	Xil_Out32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_CNTR_CNTRL_OFFSET, reg);
}

/****************************************************************************
*
* Get the time from the Timer Counter Register.
*
* @param	Pointer to the location to be updated with the time.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XTime_GetTime(XTime *Xtime_Global)
{
	u32 time_low;
	time_low = Xil_In32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_CNTR_VAL_OFFSET);
	/* If timer overflows increment the high counter */
	if(((Xil_In32(SLEEP_TIMER_BASEADDR + SLEEP_TIMER_ISR_OFFSET)) & SLEEP_TIMER_INTERRUPT_REGISTER_OV_MASK)!= 0){
		time_high++;
	}

	*Xtime_Global = (((XTime) time_high) << 32U) | (XTime) time_low;
}
#endif