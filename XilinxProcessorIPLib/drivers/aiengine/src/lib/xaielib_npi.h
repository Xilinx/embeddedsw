/*******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*******************************************************************************/

/******************************************************************************/
/**
* @file xaielib_npi.h
* @{
*
* Header files for NPI module
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Jubaer  03/08/2019  Initial creation
* </pre>
*
*******************************************************************************/
#ifndef XAIELIB_NPI_H
#define XAIELIB_NPI_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/

#define XAIE_NPI_BASEADDR				0xF70A0000

#define XAIE_NPI_PCSR_MASK				((XAIE_NPI_BASEADDR) + 0X00000000)
#define XAIE_NPI_PCSR_MASK_SHIM_RESET_MSK		0x08000000
#define XAIE_NPI_PCSR_MASK_SHIM_RESET_LSB		27U
#define XAIE_NPI_PCSR_MASK_AIE_ARRAY_RESET_MASK		0x04000000
#define XAIE_NPI_PCSR_MASK_AIE_ARRAY_RESET_LSB		26U

#define XAIE_NPI_PCSR_CONTROL				((XAIE_NPI_BASEADDR) + 0X00000004)
#define XAIE_NPI_PCSR_CONTROL_SHIM_RESET_MSK		0x08000000
#define XAIE_NPI_PCSR_CONTROL_SHIM_RESET_LSB		27U
#define XAIE_NPI_PCSR_CONTROL_AIE_ARRAY_RESET_MASK	0x04000000
#define XAIE_NPI_PCSR_CONTROL_AIE_ARRAY_RESET_LSB	26U

/************************** Function Prototypes  *****************************/

u8 XAieLib_NpiShimReset(u8 Reset);
u8 XAieLib_NpiAieArrayReset(u8 Reset);

#endif		/* end of protection macro */

/** @} */
