/******************************************************************************
*
* Copyright (C) 2004 - 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information  of Xilinx, Inc.
* and is protected under U.S. and  international copyright and other
* intellectual property  laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any  rights to the
* materials distributed herewith. Except as  otherwise provided in a valid
* license issued to you by  Xilinx, and to the maximum extent permitted by
* applicable law:
* (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL FAULTS, AND
* XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS, IMPLIED,
* OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and
* (2) Xilinx shall not be liable (whether in contract or tort,  including
* negligence, or under any other theory of liability) for any loss or damage of
* any kind or nature  related to, arising under or in connection with these
* materials, including for any direct, or any indirect,  special, incidental,
* or consequential loss or damage  (including loss of data, profits, goodwill,
* or any type of  loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was  reasonably foreseeable
* or Xilinx had been advised of the  possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe  performance, such as life-support or
* safety devices or  systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any  other applications
* that could lead to death, personal  injury, or severe property or environmental
* damage  (individually and collectively, "Critical  Applications").
* Customer assumes the sole risk and liability of any use of Xilinx products in
* Critical  Applications, subject only to applicable laws and  regulations
* governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file microblaze_interrupt_handler.c
*
* This file contains the standard interrupt handler for the MicroBlaze processor.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Date     Changes
* ----- -------- -----------------------------------------------
* 1.00b 10/03/03 First release
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/

#include "xil_exception.h"
#include "microblaze_interrupts_i.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

void __interrupt_handler (void) __attribute__ ((interrupt_handler));
void microblaze_register_handler(XInterruptHandler Handler, void *DataPtr);

/************************** Variable Definitions *****************************/

extern MB_InterruptVectorTableEntry MB_InterruptVectorTable;
/*****************************************************************************/
/**
*
* This function is the standard interrupt handler used by the MicroBlaze processor.
* It saves all volatile registers, calls the users top level interrupt handler.
* When this returns, it restores all registers, and returns using a rtid instruction.
*
* @param
*
* None
*
* @return
*
* None.
*
* @note
*
* None.
*
******************************************************************************/
void __interrupt_handler(void)
{
	/* The compiler saves all volatiles and the MSR */
	(void)MB_InterruptVectorTable.Handler(MB_InterruptVectorTable.CallBackRef);
	/* The compiler restores all volatiles and MSR, and returns from interrupt */
}

/****************************************************************************/
/*****************************************************************************/
/**
*
* Registers a top-level interrupt handler for the MicroBlaze. The
* argument provided in this call as the DataPtr is used as the argument
* for the handler when it is called.
*
* @param    Top level handler.
* @param    DataPtr is a reference to data that will be passed to the handler
*           when it gets called.

* @return   None.
*
* @note
*
* None.
*
****************************************************************************/
void microblaze_register_handler(XInterruptHandler Handler, void *DataPtr)
{
   MB_InterruptVectorTable.Handler = Handler;
   MB_InterruptVectorTable.CallBackRef = DataPtr;
}

