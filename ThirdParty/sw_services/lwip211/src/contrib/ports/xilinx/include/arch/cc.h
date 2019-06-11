/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * Copyright (C) 2007 - 2019 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __ARCH_CC_H__
#define __ARCH_CC_H__

#include <stdio.h>
#include <stdlib.h>

#include "lwipopts.h"

/** if you want to use the struct timeval provided
 * by your system, set this to 0 and include <sys/time.h> in cc.h */
#define LWIP_TIMEVAL_PRIVATE 0
#include <sys/time.h>
#include "cpu.h"

/* As per new design of lwip-2.0.2,
 * LWIP_NO_STDINT_H should be defined as 1 in cc.h */
#define LWIP_NO_STDINT_H 1

typedef unsigned   char    u8_t;
typedef signed     char    s8_t;
typedef unsigned   short   u16_t;
typedef signed     short   s16_t;
typedef unsigned   int    u32_t;
typedef signed     int    s32_t;
typedef unsigned   long long    u64_t;
typedef signed     long long    s64_t;

#define S16_F "d"
#define U16_F "d"
#define S32_F "d"
#define U32_F "x"

#define X16_F "x"
#define X32_F "x"

#define LWIP_RAND rand

typedef unsigned long mem_ptr_t;

#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#define LWIP_PLATFORM_ASSERT(x)
#define LWIP_PLATFORM_DIAG(x) do { xil_printf x; } while(0)

#endif /* __ARCH_CC_H__ */
