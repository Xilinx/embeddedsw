;/******************************************************************************
;* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
;* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
;* SPDX-License-Identifier: MIT
;******************************************************************************/
;/*****************************************************************************/
;/**
;* @file xil-crt0.S
;*
;* <pre>
;* MODIFICATION HISTORY:
;*
;* Ver   Who  Date     Changes
;* ----- ---- -------- ---------------------------------------------------
;* 7.3  dp  06/25/20   Initial version for armclang
;* 9.5  vmt 28/11/25   Added Cortex-R52 processor support for armclang
;* 9.5  vmt 04/09/26   Replace __main with explicit __scatterload_rt2,
;*                     __cpp_initialize__aeabi_, and main calls for
;*                     FuSa libc compatibility.
;* </pre>
;*
;* @note
;*
;* None.
;*
;******************************************************************************/
#include "xparameters.h"
#include "bspconfig.h"

	IMPORT |Image$$BSS_SECTION$$Base|
#if !defined (ARMR52)
	IMPORT Xil_InitializeExistingMPURegConfig
#endif
	EXPORT _startup
	IMPORT __cpu_init
	IMPORT __scatterload_rt2
	IMPORT __cpp_initialize__aeabi_
	IMPORT main
#ifdef SLEEP_TIMER_BASEADDR
	IMPORT XTime_StartTTCTimer
#endif
	AREA |.text|, CODE
	REQUIRE8 {TRUE}
	PRESERVE8 {TRUE}
	ENTRY

_startup
	bl	__cpu_init ;Initialize the CPU first (BSP provides this)
	bl      __scatterload_rt2
	bl      __cpp_initialize__aeabi_

	;configure the timer if TTC is present
#ifdef SLEEP_TIMER_BASEADDR
	bl XTime_StartTTCTimer
#endif
#if !defined (ARMR52)
	bl 	Xil_InitializeExistingMPURegConfig ;Initialize MPU config
#endif
	;make sure argc and argv are valid
	mov	r0, #0
	mov	r1, #0

#ifdef XCLOCKING
	bl	Xil_ClockInit
#endif
	bl	main ;Jump to main C code

	bl	exit

exit	;should never get here
	b exit

	END
