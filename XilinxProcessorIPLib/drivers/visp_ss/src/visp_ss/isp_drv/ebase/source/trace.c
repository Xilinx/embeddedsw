/******************************************************************************\
|* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) <2020> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2020 */

/**
 *   @file trace.c
 *
 *	This file defines the implementation for the tracing facility of the
 *	embedded lib.
 *
 *****************************************************************************/


#include "trace.h"

#include <stdarg.h>
#include <stdlib.h>

#include <time.h>

#include "dct_assert.h"

#ifndef NDEBUG

#define MAX_TRACE_MODULE 0xFFFFFFFFFFFFFFFF
#if ARMR52
#define TIME_STAMP_ENABLE 1
#define XTRACE_MEGA (1000000ULL)
static int glb_level =TRACE_OFF;
#else
static int glb_level = 0x6;
#endif
static uint64_t glb_module = 0xFFFFFFFFFFFFFFFF;

static Tracer	*tracerListHead = NULL;

// use macro instead of variable, or build error as "variable length array folded to constant array as an extension"
#define BUFFSIZE  1024

/*	For some stupid reason beyond my imagination, gcc's stdio.h doesn't	*/
/*	support vsnprintf() in strict c99 mode. Only happens in cygwin.		*/
#if defined(__GNUC__) && defined(__CYGWIN__) && !defined(PICO)
	int vsnprintf(char *, size_t, const char *, __VALIST);
#endif


#ifdef ISP_NO_USE_FUNCTION
int getTraceLevel(void)
{
	return glb_level;
}
void setTraceLevel(int new_level)
{
	if (TRACE_OFF != new_level)
		new_level = (~(((uint32_t)(new_level)) - 1u)) & MAX_LEVEL;
	glb_level = new_level;
}

int getTraceModule(Tracer *t)
{
	DCT_ASSERT(t);
	return t->module;
}
void setTraceModule(Tracer *t, int new_module)
{
	if (new_module < TRACE_MODULE_MAX)
		t->module = (uint64_t)1 << new_module;
}

void enableTracer(Tracer *t)
{
	DCT_ASSERT(t);
	t->enabled = 1;
}


void disableTracer(Tracer *t)
{
	DCT_ASSERT(t);
	t->enabled = 0;
}


void setTracerFile(Tracer* t, FILE* f)
{
	DCT_ASSERT(t);
	t->fp = f;
}

void flushTracer(const Tracer *t)
{
	if (t->fp)
		(void) fflush(t->fp);
}


#endif

static void addToList(Tracer* tracer)
{
	if (tracerListHead)
		tracer->next = tracerListHead;
	tracer->linked = 1;
	tracerListHead = tracer;
}


#if ARMR52
__attribute__((weak)) Tracer *getTracerList(void)
{
	return tracerListHead;
}

__attribute__((weak)) void setTraceModule(Tracer *t, int new_module)
{
    if (new_module < TRACE_MODULE_MAX)
    {
        t->module = (uint64_t)1 << new_module;
    }
}

// /**
//  * @brief Get 64-bit cycle count (ARM Cortex-R52)
//  *
//  * @return Raw cycle count of PMCCNTR
//  */
__attribute__((weak)) u64 GetRawCycleCount(void) {
	volatile u64 cval;

	__asm__ __volatile__("mrrc p15, 0, %Q0, %R0, c9" : "=r" (cval) :: "memory");
	return cval;
}

/**
 * @brief Get current time in micro seconds.
 *
 * @return time in micro seconds.
 */
__attribute__((weak)) u64 getCurrentTime() {
    return (GetRawCycleCount() / 875U); // Approximate CPU freq is 875 MHz
}

__attribute__((weak)) void getTimeStamp(TimeStamp* ts)
{
   volatile uint64_t timeUs = getCurrentTime();
    ts->secFrac = timeUs % XTRACE_MEGA;
    ts->sec = timeUs / XTRACE_MEGA;
}


__attribute__((weak)) void trace( Tracer* tracer, const CHAR* sFormat,  ...)
{
#if(TIME_STAMP_ENABLE)
    TimeStamp ts = { .sec = 0 , .secFrac = 0};
#endif


    va_list args;
    if (!tracer->linked)
    {
        addToList(tracer);
    }

    if (tracer->enabled && ((tracer->level & glb_level) || (tracer->level == ALWAYS)))
    {
            // Backend not initialized - fallback to original xil_vprintf
#if(TIME_STAMP_ENABLE)
            getTimeStamp(&ts);
            xil_printf("{ R0 - %llu.%06llu } ", (ts.sec), (ts.secFrac));
#else
            xil_printf("{ R0 } ");
#endif
            va_start(args, sFormat);
         //   if (useFloat) {
           //     vprintf(sFormat, args);  // Use vprintf for floats
            //} else {
                xil_vprintf(sFormat, args);  // Use xil_vprintf for non-floats
            //}
            va_end(args);
            return;

    }

}
#else
Tracer *getTracerList(void)
{
	return tracerListHead;
}

void trace(Tracer* tracer, const CHAR* sFormat, ...)
{
#if 0
	va_list args;
	if (!tracer->linked)
		addToList(tracer);

	if ((tracer->level & glb_level) && (tracer->enabled != 0)) {
		va_start(args, sFormat);
		xil_vprintf(sFormat, args);
		va_end(args);
	}
#endif
}
#endif
#endif	/* NDEBUG */
