/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xtmr_manager_g.c
* @addtogroup tmr_manager_v1_0
* @{
*
* This file contains a configuration table that specifies the configuration of
* TMR Manager devices in the system. Each device in the system should have an
* entry in the table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xtmr_manager.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * The configuration table for TMR Manager devices
 */
XTMR_Manager_Config XTMR_Manager_ConfigTable[XPAR_XTMR_MANAGER_NUM_INSTANCES] =
{
    {
	XPAR_TMRMANAGER_0_DEVICE_ID,		  /* Unique ID of device */
	XPAR_TMRMANAGER_0_BASEADDR,		  /* Device base address */
	XPAR_TMRMANAGER_0_BRK_DELAY_RST_VALUE,	  /* Break delay reset value */
	XPAR_TMRMANAGER_0_MASK_RST_VALUE,	  /* Mask reset value */
	XPAR_TMRMANAGER_0_MAGIC1,		  /* Magic byte 1 */
	XPAR_TMRMANAGER_0_MAGIC2,		  /* Magic byte 2 */
	XPAR_TMRMANAGER_0_UE_IS_FATAL,		  /* UE is fatal */
	XPAR_TMRMANAGER_0_UE_WIDTH,		  /* UE width */
	XPAR_TMRMANAGER_0_NO_OF_COMPARATORS,	  /* Number of comparators */
	XPAR_TMRMANAGER_0_COMPARATORS_MASK,	  /* Comparators mask used */
	XPAR_TMRMANAGER_0_WATCHDOG,		  /* Software watchdog used */
	XPAR_TMRMANAGER_0_WATCHDOG_WIDTH,	  /* Software watchdog width */
	XPAR_TMRMANAGER_0_SEM_INTERFACE,	  /* SEM interface */
	XPAR_TMRMANAGER_0_SEM_HEARTBEAT_WATCHDOG, /* SEM heartbeat watchdog */
	XPAR_TMRMANAGER_0_SEM_HEARTBEAT_WATCHDOG_WIDTH,
						  /* SEM HB watchdog width */
	XPAR_TMRMANAGER_0_BRK_DELAY_WIDTH,	  /* Break delay width */
	XPAR_TMRMANAGER_0_TMR,			  /* Use TMR or lockstep */
	XPAR_TMRMANAGER_0_TEST_COMPARATOR,	  /* Test comparator */
	XPAR_TMRMANAGER_0_STRICT_MISCOMPARE,	  /* Strict miscompare */
	XPAR_TMRMANAGER_0_USE_DEBUG_DISABLE,	  /* Debug disable used */
	XPAR_TMRMANAGER_0_USE_TMR_DISABLE	  /* TMR disable used */
    },
};


/** @} */
