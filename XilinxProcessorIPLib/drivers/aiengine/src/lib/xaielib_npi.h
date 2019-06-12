/*******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 1.1  Hyun    04/04/2019  Add the unlock and lock definitions
* 1.2  Wendy   02/28/2020  Add NPI interrupt registers definition
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

#define XAIE_NPI_PCSR_LOCK				((XAIE_NPI_BASEADDR) + 0X0000000C)
#define XAIE_NPI_PCSR_LOCK_STATE_LSB			0
#define XAIE_NPI_PCSR_LOCK_STATE_UNLOCK_CODE		0xF9E8D7C6
#define XAIE_NPI_PCSR_LOCK_STATE_LOCK_CODE		0x0

#define XAIE_NPI_ISR					((XAIE_NPI_BASEADDR) + 0x30U)
#define XAIE_NPI_ITR					((XAIE_NPI_BASEADDR) + 0x34U)
#define XAIE_NPI_IMR0					((XAIE_NPI_BASEADDR) + 0x38U)
#define XAIE_NPI_IER0					((XAIE_NPI_BASEADDR) + 0x3CU)
#define XAIE_NPI_IDR0					((XAIE_NPI_BASEADDR) + 0x40U)
#define XAIE_NPI_IMR1					((XAIE_NPI_BASEADDR) + 0x44U)
#define XAIE_NPI_IER1					((XAIE_NPI_BASEADDR) + 0x48U)
#define XAIE_NPI_IDR1					((XAIE_NPI_BASEADDR) + 0x4CU)
#define XAIE_NPI_IMR2					((XAIE_NPI_BASEADDR) + 0x50U)
#define XAIE_NPI_IER2					((XAIE_NPI_BASEADDR) + 0x54U)
#define XAIE_NPI_IDR2					((XAIE_NPI_BASEADDR) + 0x58U)
#define XAIE_NPI_IMR3					((XAIE_NPI_BASEADDR) + 0x5CU)
#define XAIE_NPI_IER3					((XAIE_NPI_BASEADDR) + 0x60U)
#define XAIE_NPI_IDR3					((XAIE_NPI_BASEADDR) + 0x64U)

/************************** Function Prototypes  *****************************/

u8 XAieLib_NpiShimReset(u8 Reset);
u8 XAieLib_NpiAieArrayReset(u8 Reset);

#endif		/* end of protection macro */

/** @} */
