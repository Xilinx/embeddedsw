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
*
* @file usleep.c
*
* This function provides a microsecond delay using the Global Timer register in
* the ARM Cortex A53 MP core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 5.00 	pkp  	 05/29/14 First release
* 5.04	pkp		 01/28/16 Modified the usleep API to configure Time Stamp
*						  generator only when disable using frequency from
*						  xparamters.h instead of hardcoding
* 5.05	pkp		 13/04/16 Modified usleep routine to call XTime_StartTimer
*						  which enables timer only when it is disabled and
*						  read counter value directly from register instead
*						  of calling XTime_GetTime for optimization
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "sleep.h"
#include "xtime_l.h"
#include "xparameters.h"
#include "xpseudo_asm.h"
#include "xreg_cortexa53.h"

/* Global Timer is always clocked at half of the CPU frequency */
#define COUNTS_PER_USECOND  (COUNTS_PER_SECOND/1000000 )

/*****************************************************************************/
/**
*
* This API gives a delay in microseconds
*
* @param	useconds requested
*
* @return	0 if the delay can be achieved, -1 if the requested delay
*		is out of range
*
* @note		None.
*
****************************************************************************/
s32 usleep(u32 useconds)
{
	XTime tEnd, tCur;
	/* Start global timer counter, it will only be enabled if it is disabled */
	XTime_StartTimer();

	tCur = mfcp(CNTPCT_EL0);
	tEnd = tCur + (((XTime) useconds) * COUNTS_PER_USECOND);
	do
	{
		tCur = mfcp(CNTPCT_EL0);
	} while (tCur < tEnd);

	return 0;
}
