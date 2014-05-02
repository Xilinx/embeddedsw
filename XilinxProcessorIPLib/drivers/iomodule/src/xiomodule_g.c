/******************************************************************************
*
* (c) Copyright 2011-2014 Xilinx, Inc. All rights reserved.
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
* @file xiomodule_g.c
*
* This file contains a configuration table that specifies the configuration of
* interrupt controller devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sa   07/15/11 First release
* </pre>
*
* @internal
*
* This configuration table contains entries that are modified at runtime
* by the driver. The EDK tools populate the table with default values for the
* vector table and the options flag. These default values can be, and are,
* overwritten at runtime by the driver.  This is a deviation from most drivers'
* configuration tables in that most are created statically by the tools and
* are never modified during runtime.  Most tables reflect only the hardware
* configuration of the device. This IOModule configuration table contains
* software information in addition to hardware configuration.  The IOModule
* configuration table should be considered an exception to the usage of the
* configuration table rather than the norm.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xiomodule.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each iomodule device
 * in the system. The XIOModule driver must know when to acknowledge the
 * interrupt. The entry which specifies this as a bit mask where each bit
 * corresponds to a specific interrupt.  A bit set indicates to ack it before
 * servicing it. Generally, acknowledge before service is used when the
 * interrupt signal is edge-sensitive, and after when the signal is
 * level-sensitive.
 *
 * Refer to the XIOModule_Config data structure in xiomodule.h for
 * details on how this table should be initialized.
 */
XIOModule_Config XIOModule_ConfigTable[XPAR_XIOMODULE_NUM_INSTANCES] = {
	{
	 XPAR_IOMODULE_0_DEVICE_ID,	    /* Unique ID of device */
	 XPAR_IOMODULE_0_REG_BASEADDR,	    /* Unique identifier */
	 XPAR_IOMODULE_0_IO_BASEADDR,	    /* IO Bus Base Address */
	 XPAR_IOMODULE_0_INTC_HAS_FAST,	    /* Fast interrupt enabled */
	 XPAR_IOMODULE_0_INTC_BASE_VECTORS, /* Relocatable base vector */
	 XPAR_IOMODULE_0_ACK_BEFORE,	    /* Ack before or after service */
	 0,				    /* Device options */
	 100000000,			    /* Input clock frequency (Hz) */
	 115200,			    /* Current baud rate */
	 {0, 0, 0, 0},			    /* PIT timer used */
	 {0, 0, 0, 0},			    /* PIT timer counter size */
	 {0, 0, 0, 0},			    /* PIT prescaler */
	 {0, 0, 0, 0},			    /* PIT readable */
	 {0, 0, 0, 0}			    /* GPO initial value */
	 }
	,
	{
	 XPAR_IOMODULE_1_DEVICE_ID,	    /* Unique ID of device */
	 XPAR_IOMODULE_1_REG_BASEADDR,	    /* Unique identifier */
	 XPAR_IOMODULE_1_IO_BASEADDR,	    /* IO Bus Base Address */
	 XPAR_IOMODULE_1_INTC_HAS_FAST,	    /* Fast interrupt enabled */
	 XPAR_IOMODULE_1_INTC_BASE_VECTORS, /* Relocatable base vector */
	 XPAR_IOMODULE_1_ACK_BEFORE,	    /* Ack before or after service */
	 0,				    /* Device options */
	 100000000,			    /* Input clock frequency (Hz) */
	 115200,			    /* Current baud rate */
	 {0, 0, 0, 0},			    /* PIT timer used */
	 {0, 0, 0, 0},			    /* PIT timer counter size */
	 {0, 0, 0, 0},			    /* PIT prescaler */
	 {0, 0, 0, 0},			    /* PIT readable */
	 {0, 0, 0, 0}			    /* GPO initial value */
	 }
};
