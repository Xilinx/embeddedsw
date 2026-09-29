/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcoresightpsdcc.c
* @brief Implements the minimum required XCoreSightPs DCC functions.
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
* 1.13  ml     07/21/25 Fixed GCC warnings
* 1.14  vn     07/30/26 Fix Doxygen warnings
* 1.14  tb     09/10/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/**
* @addtogroup coresightps_dcc_api CORESIGHTPS_DCC APIs
* @{
*/

/***************************** Include Files *********************************/
#if defined (__MICROBLAZE__) || defined (__riscv)
#warning "The driver is supported only for ARM architecture"
#else

#include "xil_types.h"
#include "xpseudo_asm.h"
#include "xcoresightpsdcc.h"

#ifdef __ICCARM__
#define INLINE /**< Compiler inline keyword. */
#else
#define INLINE __inline /**< Compiler inline keyword. */
#endif

/** @name DCC status bits
 * @{
 */
#define XCORESIGHTPS_DCC_STATUS_RX (1 << 30) /**< RX is full. */
#define XCORESIGHTPS_DCC_STATUS_TX (1 << 29) /**< TX is full. */
/** @} */

static INLINE u32 XCoresightPs_DccGetStatus(void);

/****************************************************************************/
/**
* @brief Sends a single byte using the DCC.
*
* This function is blocking.
* It waits until the transmitter is not full,
* then writes the byte to the transmit register.
*
* @param[in]	BaseAddress Dummy parameter to match other stdio device
*		function prototypes.
* @param[in]	Data Byte of data to send.
*
******************************************************************************/
void XCoresightPs_DccSendByte(u32 BaseAddress, u8 Data)
{
	(void) BaseAddress;
	while (XCoresightPs_DccGetStatus() & XCORESIGHTPS_DCC_STATUS_TX) {
		dsb();
	}
#ifdef __aarch64__
	u64 Data64 = (u64)Data;
	__asm volatile ("msr dbgdtrtx_el0, %0" : : "r" (Data64));
	Data = (u8)Data64;
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
* @brief Receives a single byte using the DCC.
*
* This function is blocking.
* It waits until the receiver is not empty,
* then reads the byte from the receive register.
*
* @param[in]	BaseAddress Dummy parameter to match other stdio device
*		function prototypes.
*
* @return	The byte of data received.
*
******************************************************************************/
u8 XCoresightPs_DccRecvByte(u32 BaseAddress)
{
	u64 Data = 0U;
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

	return (u8)Data;
}

/****************************************************************************/
/**
* @brief Reads the DCC status register.
*
* @return	The contents of the status register.
*
******************************************************************************/
static INLINE u32 XCoresightPs_DccGetStatus(void)
{
	u64 Status = 0U;

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
	return (u32)Status;
}

#ifdef XPAR_STDIN_IS_CORESIGHTPS_DCC
/****************************************************************************/
/**
* @brief Sends one byte to the CoreSight DCC stdout device.
*
* @param[in]	c Byte to send.
*
******************************************************************************/
void outbyte(char c)
{
	XCoresightPs_DccSendByte(STDOUT_BASEADDRESS, c);
}

/****************************************************************************/
/**
* @brief Receives one byte from the CoreSight DCC stdin device.
*
* @return	The byte received.
*
******************************************************************************/
char inbyte(void)
{
	return XCoresightPs_DccRecvByte(STDIN_BASEADDRESS);
}
#endif
#endif
/** @} */
