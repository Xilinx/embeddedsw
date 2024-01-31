/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcoresightpsdcc.c
* @addtogroup coresightps_dcc Overview
* @{
*
* Functions in this file are the minimum required functions for the
* XCoreSightPs driver.
*
* @note 	None.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date		Changes
* ----- -----  -------- -----------------------------------------------
* 1.00  kvn    02/14/15 First release
* 1.1   kvn    06/12/15 Add support for Zynq Ultrascale+ MP.
*       kvn    08/18/15 Modified Makefile according to compiler changes.
* 1.2   kvn    10/09/15 Add support for IAR Compiler.
* 1.3   asa    07/01/16 Made changes to ensure that the file does not compile
*                       for MB BSPs. Instead it throws up a warning. This
*                       fixes the CR#953056.
* 1.5   sne    01/19/19 Fixed MISRA-C Violations CR#1025101.
* 1.9   ht     07/05/23 Added support for system device-tree flow.
* 1.10  mus    10/06/23 Fix compilation error for Microblaze RISC-V processor.
* 1.10  ml     11/15/23 Fix compilation errors reported with -std=c2x compiler flag
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#if defined (__MICROBLAZE__) || defined (__riscv)
#warning "The driver is supported only for ARM architecture"
#else

#include "xil_types.h"
#include "xpseudo_asm.h"
#include "xcoresightpsdcc.h"

#ifdef __ICCARM__
#define INLINE
#else
#define INLINE __inline
#endif

/* DCC Status Bits */
#define XCORESIGHTPS_DCC_STATUS_RX (1 << 30)
#define XCORESIGHTPS_DCC_STATUS_TX (1 << 29)

static INLINE u32 XCoresightPs_DccGetStatus(void);

/****************************************************************************/
/**
*
* This functions sends a single byte using the DCC. It is blocking in that it
* waits for the transmitter to become non-full before it writes the byte to
* the transmit register.
*
* @param	BaseAddress is a dummy parameter to match the function proto
*		of functions for other stdio devices.
* @param	Data is the byte of data to send
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCoresightPs_DccSendByte(u32 BaseAddress, u8 Data)
{
	(void) BaseAddress;
	while (XCoresightPs_DccGetStatus() & XCORESIGHTPS_DCC_STATUS_TX) {
		dsb();
	}
#ifdef __aarch64__
	__asm volatile ("msr dbgdtrtx_el0, %0" : : "r" (Data));
#elif defined (__GNUC__) || defined (__ICCARM__)
	__asm volatile("mcr p14, 0, %0, c0, c5, 0"
			     : : "r" (Data));
#else
	{
		volatile register u32 Reg __asm("cp14:0:c0:c5:0");
		Reg = Data;
	}
#endif
	isb();

}

/****************************************************************************/
/**
*
* This functions receives a single byte using the DCC. It is blocking in that
* it waits for the receiver to become non-empty before it reads from the
* receive register.
*
* @param	BaseAddress is a dummy parameter to match the function proto
*		of functions for other stdio devices.
*
* @return	The byte of data received.
*
* @note		None.
*
******************************************************************************/
u8 XCoresightPs_DccRecvByte(u32 BaseAddress)
{
	u8 Data = 0U;
	(void) BaseAddress;

	while (!(XCoresightPs_DccGetStatus() & XCORESIGHTPS_DCC_STATUS_RX)) {
		dsb();
	}

#ifdef __aarch64__
	__asm volatile ("mrs %0, dbgdtrrx_el0" : "=r" (Data));
#elif defined (__GNUC__) || defined (__ICCARM__)
	__asm volatile ("mrc p14, 0, %0, c0, c5, 0"
			     : "=r" (Data));
#else
	{
		volatile register u32 Reg __asm("cp14:0:c0:c5:0");
		Data = Reg;
	}
#endif
	isb();

	return Data;
}

/****************************************************************************/
/**INLINE
*
* This functions read the status register of the DCC.
*
* @param	BaseAddress is the base address of the device
*
* @return	The contents of the Status Register.
*
* @note		None.
*
******************************************************************************/
static INLINE u32 XCoresightPs_DccGetStatus(void)
{
	u32 Status = 0U;

#ifdef __aarch64__
	__asm volatile ("mrs %0, mdccsr_el0" : "=r" (Status));
#elif defined (__GNUC__) || defined (__ICCARM__)
	__asm volatile ("mrc p14, 0, %0, c0, c1, 0"
			     : "=r" (Status) : : "cc");
#else
	{
		volatile register u32 Reg __asm("cp14:0:c0:c1:0");
		Status = Reg;
	}
#endif
	return Status;
}

#ifdef XPAR_STDIN_IS_CORESIGHTPS_DCC
void outbyte(char c)
{
	XCoresightPs_DccSendByte(STDOUT_BASEADDRESS, c);
}

char inbyte(void)
{
	return XCoresightPs_DccRecvByte(STDIN_BASEADDRESS);
}
#endif
#endif
/** @} */
