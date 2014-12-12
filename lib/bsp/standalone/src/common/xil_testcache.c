/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc. All rights reserved.
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
* @file xil_testcache.c
*
* Contains utility functions to test cache.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a hbm  07/28/09 Initial release
* 4.1   asa  05/09/14 Ensured that the address uses for cache test is aligned
*				      cache line.
* </pre>
*
* @note
*
* This file contain functions that all operate on HAL.
*
******************************************************************************/
#ifdef __ARM__
#include "xil_cache.h"
#include "xil_testcache.h"
#include "xil_types.h"
#include "xpseudo_asm.h"
#ifdef __aarch64__
#include "xreg_cortexa53.h"
#else
#include "xreg_cortexr5.h"
#endif

#include "xil_types.h"

extern void xil_printf(const char8 *ctrl1, ...);

#define DATA_LENGTH 128

#ifdef __aarch64__
static INTPTR Data[DATA_LENGTH] __attribute__ ((aligned(64)));
#else
static INTPTR Data[DATA_LENGTH] __attribute__ ((aligned(32)));
#endif

/**
* Perform DCache range related API test such as Xil_DCacheFlushRange and
* Xil_DCacheInvalidateRange. This test function writes a constant value
* to the Data array, flushes the range, writes a new value, then invalidates
* the corresponding range.
*
* @return
*
*     - 0 is returned for a pass
*     - -1 is returned for a failure
*/
s32 Xil_TestDCacheRange(void)
{
	s32 Index;
	s32 Status = 0;
	u32 CtrlReg;
	INTPTR Value;

	xil_printf("-- Cache Range Test --\n\r");

	for (Index = 0; Index < DATA_LENGTH; Index++)
		Data[Index] = 0xA0A00505;

	xil_printf("    initialize Data done:\r\n");

	Xil_DCacheFlushRange((INTPTR)Data, DATA_LENGTH * sizeof(INTPTR));

	xil_printf("    flush range done\r\n");

	dsb();
	#ifdef __aarch64__
			CtrlReg = mfcp(SCTLR_EL3);
			CtrlReg &= ~(XREG_CONTROL_DCACHE_BIT);
			mtcp(SCTLR_EL3,CtrlReg);
	#else
			CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
			CtrlReg &= ~(XREG_CP15_CONTROL_C_BIT);
			mtcp(XREG_CP15_SYS_CONTROL, CtrlReg);
	#endif
	dsb();

	Status = 0;

	for (Index = 0; Index < DATA_LENGTH; Index++) {
		Value = Data[Index];
		if (Value != 0xA0A00505) {
			Status = -1;
			xil_printf("Data[%d] = %x\r\n", Index, Value);
			break;
		}
	}

	if (!Status) {
		xil_printf("	Flush worked\r\n");
	}
	else {
		xil_printf("Error: flush dcache range not working\r\n");
	}
	dsb();
	#ifdef __aarch64__
			CtrlReg = mfcp(SCTLR_EL3);
			CtrlReg |= (XREG_CONTROL_DCACHE_BIT);
			mtcp(SCTLR_EL3,CtrlReg);
		#else
			CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
			CtrlReg |= (XREG_CP15_CONTROL_C_BIT);
			mtcp(XREG_CP15_SYS_CONTROL, CtrlReg);
		#endif
	dsb();
	for (Index = 0; Index < DATA_LENGTH; Index++)
		Data[Index] = 0xA0A0C505;



	Xil_DCacheFlushRange((INTPTR)Data, DATA_LENGTH * sizeof(INTPTR));

	for (Index = 0; Index < DATA_LENGTH; Index++)
		Data[Index] = Index + 3;

	Xil_DCacheInvalidateRange((INTPTR)Data, DATA_LENGTH * sizeof(INTPTR));

	xil_printf("    invalidate dcache range done\r\n");
	dsb();
	#ifdef __aarch64__
			CtrlReg = mfcp(SCTLR_EL3);
			CtrlReg &= ~(XREG_CONTROL_DCACHE_BIT);
			mtcp(SCTLR_EL3,CtrlReg);
	#else
			CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
			CtrlReg &= ~(XREG_CP15_CONTROL_C_BIT);
			mtcp(XREG_CP15_SYS_CONTROL, CtrlReg);
	#endif
	dsb();
	for (Index = 0; Index < DATA_LENGTH; Index++)
		Data[Index] = 0xA0A0A05;
	dsb();
	#ifdef __aarch64__
			CtrlReg = mfcp(SCTLR_EL3);
			CtrlReg |= (XREG_CONTROL_DCACHE_BIT);
			mtcp(SCTLR_EL3,CtrlReg);
	#else
			CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
			CtrlReg |= (XREG_CP15_CONTROL_C_BIT);
			mtcp(XREG_CP15_SYS_CONTROL, CtrlReg);
	#endif
	dsb();

	Status = 0;

	for (Index = 0; Index < DATA_LENGTH; Index++) {
		Value = Data[Index];
		if (Value != 0xA0A0A05) {
			Status = -1;
			xil_printf("Data[%d] = %x\r\n", Index, Value);
			break;
		}
	}


	if (!Status) {
		xil_printf("    Invalidate worked\r\n");
	}
	else {
		xil_printf("Error: Invalidate dcache range not working\r\n");
	}
	xil_printf("-- Cache Range Test Complete --\r\n");
	return Status;

}

/**
* Perform DCache all related API test such as Xil_DCacheFlush and
* Xil_DCacheInvalidate. This test function writes a constant value
* to the Data array, flushes the DCache, writes a new value, then invalidates
* the DCache.
*
* @return
*     - 0 is returned for a pass
*     - -1 is returned for a failure
*/
s32 Xil_TestDCacheAll(void)
{
	s32 Index;
	s32 Status;
	INTPTR Value;
	u32 CtrlReg;

	xil_printf("-- Cache All Test --\n\r");

	for (Index = 0; Index < DATA_LENGTH; Index++)
		Data[Index] = 0x50500A0A;
	xil_printf("    initialize Data done:\r\n");

	Xil_DCacheFlush();
	xil_printf("    flush all done\r\n");
	dsb();
	#ifdef __aarch64__
		CtrlReg = mfcp(SCTLR_EL3);
		CtrlReg &= ~(XREG_CONTROL_DCACHE_BIT);
		mtcp(SCTLR_EL3,CtrlReg);
	#else
		CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
		CtrlReg &= ~(XREG_CP15_CONTROL_C_BIT);
		mtcp(XREG_CP15_SYS_CONTROL, CtrlReg);
	#endif
	dsb();
	Status = 0;

	for (Index = 0; Index < DATA_LENGTH; Index++) {
		Value = Data[Index];

		if (Value != 0x50500A0A) {
			Status = -1;
			xil_printf("Data[%d] = %x\r\n", Index, Value);
			break;
		}
	}

	if (!Status) {
		xil_printf("    Flush all worked\r\n");
	}
	else {
		xil_printf("Error: Flush dcache all not working\r\n");
	}
	dsb();
	#ifdef __aarch64__
		CtrlReg = mfcp(SCTLR_EL3);
		CtrlReg |= (XREG_CONTROL_DCACHE_BIT);
		mtcp(SCTLR_EL3,CtrlReg);
	#else
		CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
			CtrlReg |= (XREG_CP15_CONTROL_C_BIT);
			mtcp(XREG_CP15_SYS_CONTROL, CtrlReg);
	#endif
	dsb();
	for (Index = 0; Index < DATA_LENGTH; Index++)
		Data[Index] = 0x505FFA0A;

	Xil_DCacheFlush();


	for (Index = 0; Index < DATA_LENGTH; Index++)
		Data[Index] = Index + 3;

	Xil_DCacheInvalidate();

	xil_printf("    invalidate all done\r\n");
	dsb();
	#ifdef __aarch64__
		CtrlReg = mfcp(SCTLR_EL3);
		CtrlReg &= ~(XREG_CONTROL_DCACHE_BIT);
		mtcp(SCTLR_EL3,CtrlReg);
	#else
		CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
		CtrlReg &= ~(XREG_CP15_CONTROL_C_BIT);
		mtcp(XREG_CP15_SYS_CONTROL, CtrlReg);
	#endif
	dsb();
	for (Index = 0; Index < DATA_LENGTH; Index++)
		Data[Index] = 0x50CFA0A;
	dsb();
	#ifdef __aarch64__
		CtrlReg = mfcp(SCTLR_EL3);
		CtrlReg |= (XREG_CONTROL_DCACHE_BIT);
		mtcp(SCTLR_EL3,CtrlReg);
	#else
		CtrlReg = mfcp(XREG_CP15_SYS_CONTROL);
		CtrlReg |= (XREG_CP15_CONTROL_C_BIT);
		mtcp(XREG_CP15_SYS_CONTROL, CtrlReg);
	#endif
	dsb();
	Status = 0;

	for (Index = 0; Index < DATA_LENGTH; Index++) {
		Value = Data[Index];
		if (Value != 0x50CFA0A) {
			Status = -1;
			xil_printf("Data[%d] = %x\r\n", Index, Value);
			break;
		}
	}

	if (!Status) {
		xil_printf("    Invalidate all worked\r\n");
	}
	else {
			xil_printf("Error: Invalidate dcache all not working\r\n");
	}

	xil_printf("-- DCache all Test Complete --\n\r");

	return Status;
}


/**
* Perform Xil_ICacheInvalidateRange() on a few function pointers.
*
* @return
*
*     - 0 is returned for a pass
*     The function will hang if it fails.
*/
s32 Xil_TestICacheRange(void)
{

	Xil_ICacheInvalidateRange((INTPTR)Xil_TestICacheRange, 1024);
	Xil_ICacheInvalidateRange((INTPTR)Xil_TestDCacheRange, 1024);
	Xil_ICacheInvalidateRange((INTPTR)Xil_TestDCacheAll, 1024);

	xil_printf("-- Invalidate icache range done --\r\n");

	return 0;
}

/**
* Perform Xil_ICacheInvalidate().
*
* @return
*
*     - 0 is returned for a pass
*     The function will hang if it fails.
*/
s32 Xil_TestICacheAll(void)
{
	Xil_ICacheInvalidate();
	xil_printf("-- Invalidate icache all done --\r\n");
	return 0;
}
#endif
