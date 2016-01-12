/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
* @file xilskey_bbram_ultrascale_input.h
*
* This file contains macros which needs to configured by user based on the
* options selected by user operations will be performed.
*
* @note
*
*  		User configurable parameters for Ultrascale BBRAM
*  	----------------------------------------------------------------------
* 	#define 	XSK_BBRAM_AES_KEY
*	"349de4571ae6d88de23de65489acf67000ff5ec901ae3d409aabbce4549812dd"
* 	#define	XSK_BBRAM_AES_KEY_SIZE_IN_BITS	256
*
*	In Ultrascale GPIO pins are used for connecting MASTER_JTAG pins to
*	access BBRAM.
*	Following are the GPIO pins and user can change these pins
*	#define XSK_BBRAM_AXI_GPIO_JTAG_TDO	(0)
*	#define XSK_BBRAM_AXI_GPIO_JTAG_TDI	(0)
*	#define XSK_BBRAM_AXI_GPIO_JTAG_TMS	(1)
*	#define XSK_BBRAM_AXI_GPIO_JTAG_TCK	(2)
*
*	#define XSK_BBRAM_GPIO_INPUT_CH		(2)
*	This macro is for providing channel number of ALL INPUTS connected(TDO)
*	#define XSK_BBRAM_GPIO_OUTPUT_CH	(1)
*	This macro is for providing channel number of ALL OUTPUTS connected
*	(TDI, TCK, TMS)
*
*	NOTE: All inputs and outputs of GPIO can be configured in single
*	channel also
*	i.e XSK_BBRAM_GPIO_INPUT_CH = XSK_BBRAM_GPIO_OUTPUT_CH = 1 or 2.
*	Among (TDI, TCK, TMS) Outputs of GPIO cannot be connected to different
*	GPIO channels all the 3 signals should be in same channel.
*	TDO can be a other channel of (TDI, TCK, TMS) or the same.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ----    -------- ------------------------------------------------------
* 5.0   vns     09/01/16 First Release.
*
* </pre>
*
******************************************************************************/

#ifndef XILSKEY_INPUT_H
#define XILSKEY_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/

/**
 * Following defines should be given in decimal/hexa-decimal values.
 * These are to be defined for Ultrascale Microblaze
 * AXI GPIO pin numbers connected to MASTER JTAG primitive and corresponding
 * channel numbers for GPIO pins
 */
#define	XSK_BBRAM_AXI_GPIO_JTAG_TDO	(0)	/**< MASTER JTAG GPIO
						  *  pin for TDO */
#define	XSK_BBRAM_AXI_GPIO_JTAG_TDI	(0)	/**< MASTER JTAG GPIO
						  *  pin for TDI */
#define	XSK_BBRAM_AXI_GPIO_JTAG_TMS	(1)	/**< MASTER JTAG GPIO
						  *  pin for TMS */
#define	XSK_BBRAM_AXI_GPIO_JTAG_TCK	(2)	/**< MASTER JTAG GPIO
						  *  pin for TCK */

#define	XSK_BBRAM_GPIO_INPUT_CH		(2)	/**< GPIO Channel of TDO
						  *  pin connected */
#define	XSK_BBRAM_GPIO_OUTPUT_CH	(1)	/**< GPIO Channel of TDI,
						  *  TMS and TCK pin
						  *  connected */

/**
 *
 * This is the 256 bit key to be programmed into BBRAM.
 * This should entered by user in HEX.
 */
#define 	XSK_BBRAM_AES_KEY	"349de4571ae6d88de23de65489acf67000ff5ec901ae3d409aabbce4549812dd"

#define		XSK_BBRAM_AES_KEY_SIZE_IN_BITS	256

/*
 * End of definitions for BBRAM
 */

/************************** Function Prototypes *****************************/
/****************************************************************************/
#ifdef __cplusplus
}
#endif

#endif	/*XILSKEY_INPUT_H*/
