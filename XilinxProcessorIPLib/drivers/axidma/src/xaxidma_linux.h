/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxidma_linux.h
* @addtogroup axidma_v9_12
* @{
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------
* 9.12  vak  08/21/20  Initial Release.
*****************************************************************************/

#ifndef XAXIDMA_LINUX_H
#define XAXIDMA_LINUX_H

#if defined(__LIBMETAL__) && !defined(__BAREMETAL__)

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef char char8;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint64_t u64;
typedef int sint32;

typedef intptr_t INTPTR;
typedef uintptr_t UINTPTR;

#ifndef TRUE
#define TRUE            1U
#endif

#ifndef FALSE
#define FALSE           0U
#endif

#ifndef NULL
#define NULL            0U
#endif

#define XIL_COMPONENT_IS_READY     0x11111111U
#define XIL_COMPONENT_IS_STARTED   0x22222222U

#ifndef xil_printf
#define xil_printf(args, ...)	printf((args), ##__VA_ARGS__)
#endif

#if defined (__aarch64__) || defined (__arch64__)
#define UPPER_32_BITS(n) ((u32)(((n) >> 16) >> 16))
#else
#define UPPER_32_BITS(n) 0U
#endif

#define LOWER_32_BITS(n) ((u32)(n))

#define DATA_SYNC

#define Xil_AssertNonvoid(opt)
#define Xil_AssertVoid(opt)

#define Xil_DCacheInvalidateRange(opt1, pt2)
#define Xil_DCacheFlushRange(opt1, opt2)

#endif
#endif /* XAXIDMA_LINUX_H */
