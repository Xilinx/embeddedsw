/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * Copyright (c) 2014-2022 Vivante Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

/* VeriSilicon 2020 */

/**
 *   @file offline_trace.h
 *
 *   This file defines the API for the tracing facility of the embedded lib.
 *
 *   WARNING:    Due to use of variadic macros which were introduced into C in
 *               c99 this can not be used with c++ code in pedantic mode.
 *
 *****************************************************************************/
/**
 * @defgroup module_tracer Trace System
 *
 * @brief The trace system used by Dream Chip.
 *
 * Example use of the trace system:
 *
 * - Create a file like tracer_cfg.c
 *
 * @code
 * CREATE_OFFLINETRACER(MODULE_NAME);
 * @endcode
 *
 * - In your source file import the tracer you like to use
 *
 * @code
 * USE_TRACER(MODULE_INFO);
 *
 * void foo()
 * {
 *     OFFLINE_TRACE(MODULE_INFO+instanceid,__func__, "enter %s\n", info);
 *     OFFLINE_TRACE(MODULE_INFO+instanceid,__func__, "exit %s\n", info);
 * }
 * @endcode
 *
 * @{
 *
 *****************************************************************************/
#ifndef OFFLINE_TRACE_H_
#define OFFLINE_TRACE_H_

/* must be defined even for release */
#include "linux_compat.h"
#include "types.h"
#include <return_codes.h>
#ifdef __cplusplus
extern "C"
{
#endif

void setOffLineTraceCasePrefix(const char* pCasePrefix);


#ifdef ISP_OFFLINE_TEST
#define MAX_SUPPORT_INSTANCE 4

extern char glog_type[3][6];
typedef struct offline_tracer_s {
	FILE *fp;
	char *name;
	uint32_t instanceId;
	int8_t enabled;
	uint32_t index;
} Offline_Tracer;

void enableOffLineTrace(Offline_Tracer *);
void disableOffLineTrace(Offline_Tracer *);
void setOffLineTraceFile(Offline_Tracer *, FILE *);
void OpenOffLineTrace(Offline_Tracer *off_Tracer, uint32_t instanceId, const char *path_suffix);
void CloseOffLineTrace(Offline_Tracer *off_Tracer);
void offLinetrace(Offline_Tracer *off_Tracer, const char* log_type, const char* func_name,
		  const char *sFormat, ...);
void setoffLineTraceFrameid(uint32_t instanceId, uint32_t i);


/*****************************************************************
*
*   This macro creates a offlineTracer. Every offineTracer has its
*   own output like stdout or a file. A tracer may be enabled and disabled.
*
*   \warning THIS MACRO MUST BE USED IN GLOBAL SCOPE.
*
*   @param    name          Name of the offline tracer.
*
******************************************************************/
#define CREATE_OFFLINE_TRACE(name) \
	Offline_Tracer offtrace__##name[MAX_SUPPORT_INSTANCE] = {0}; \
	Offline_Tracer *name = &offtrace__##name[0]; \
	CHAR tracer_##name[] = #name

/******************************************************************
*
*   If offline tracer was created in another compile unit this macro
*   makes the named tracer available in the current unit.
*
*   @param      Name of the offline tracer
*
******************************************************************/
#define USE_OFFLINE_TRACE(name)\
	extern Offline_Tracer offtrace__##name[MAX_SUPPORT_INSTANCE];\
	extern Offline_Tracer *name;\
	extern CHAR tracer_##name[]


/******************************************************************
*
*  Initial the offline tracer *fp
*
*  @param      First parameter is name of offline tracer.
*              Second paramter is the multi-instance index.
*              Third paramter is the log file storage path.
*              Fourth paramter is the File *fp pointer
*
*  @return     No return value.
*
*******************************************************************/
#define OPEN_OFFLINE_TRACE(T,I,C) \
	((Offline_Tracer *)((T)+(I)))->name = tracer_##T; \
	OpenOffLineTrace(T,I,C)

/*******************************************************************
*
*   this macro is used to close the offline trace *fp.
*
*   @param      T Name of the offline tracer
*
********************************************************************/
#define CLOSE_OFFLINE_TRACE(T) CloseOffLineTrace(T)

/********************************************************************
*              Send output to an offline tracer.
*
*  @param      First parameter is name of offline tracer.
*              Secone parameter is the log type.
*              Third paramter is the function name.
*              Use variable argument list like printf.
*
*  @return     No return value.
*
********************************************************************/
#define OFFLINE_TRACE(T,...) offLinetrace(T,glog_type[0],##__VA_ARGS__)

#define OFFLINE_TRACE_ERR(T,...) offLinetrace(T,glog_type[1],##__VA_ARGS__)

#define OFFLINE_TRACE_WARN(T,...) offLinetrace(T,glog_type[2],##__VA_ARGS__)

/********************************************************************
*              Send array output to an offline tracer.
*
*  @param      First parameter is name of offline tracer.
*              Second paramter is the function name.
*              variable argument  is the name of array varible
               isFloat is whether if it is float type
*
*  @return     No return value.
*
********************************************************************/
#define OFFLINE_TRACE_ARY(T, F, prefix,varible, isFloat) \
	do { \
		for (int i = 0; i < sizeof(varible)/sizeof(typeof(varible[0])); i++) \
		{ \
			if (!isFloat) { \
				OFFLINE_TRACE(T, F, "%s %s[%d] %d\n", prefix, #varible, i, *(varible+i)); \
			} else { \
				OFFLINE_TRACE(T, F, "%s %s[%d] %f\n", prefix, #varible, i, *(varible+i)); \
			} \
		} \
	}while(0)

/*********************************************************************
*
*              Enable an offline tracer.
*
*  @param      T name of offline tracer.
*  @return     No return value.
*
*********************************************************************/
#define ENABLE_OFFLINE_TRACE(T) enableOffLineTrace(T)


/*********************************************************************
*              Disable an offline tracer.
*
*  @param      T   name of offline tracer.
*  @return     No return value.
*
*********************************************************************/
#define DISABLE_OFFLINE_TRACE(T) disableOffLineTrace(T)

/**********************************************************************
*
*              set the offline tracer *fp
*
*  @param      I   index of isp.
*  @param      F   A valid FILE*.
*
*  @return     No return value.
*
**********************************************************************/
#define SET_OFFLINE_TRACE_FILE(I, F) setOffLineTraceFile(I, F)

/**********************************************************************
*
*              set the Current frame index
*
*  @param      ispId isp id .
*  @param      I   frame Index.
*
*  @return     No return value.
*
**********************************************************************/
#define SET_FRAMEID_TRACE(ispId,I) setoffLineTraceFrameid(ispId,I)

#else
/* The macro for CREATE_TRACER can not just expand to nothing. Otherwise we
* would have a single semicolon which C does not allow outside of function
* bodies. For this reason we expand to this extern declaration. As the
* external variable is never used linking should not be a problem. */
#define CREATE_OFFLINE_TRACE(name)        int32_t name
#define OPEN_OFFLINE_TRACE(...)           (void)0
#define CLOSE_OFFLINE_TRACE(T)            (void)0
#define USE_OFFLINE_TRACE(name)           extern int32_t name
#define OFFLINE_TRACE(...)                (void)0
#define OFFLINE_TRACE_ERR(...)            (void)0
#define OFFLINE_TRACE_WARN(...)           (void)0
#define ENABLE_OFFLINE_TRACE(T)           (void)0
#define DISABLE_OFFLINE_TRACE(T)          (void)0
#define SET_OFFLINE_TRACE_FILE(I, F)      (void)0
#define SET_FRAMEID_TRACE(ispId,F)        (void)0
#define OFFLINE_TRACE_ARY(T, F, prefix,varible, isFloat)    (void)0

#endif /* ISP_OFFLINE_TEST */

#ifdef __cplusplus
}
#endif

/* @} module_tracer*/

#endif /*OFFLINE_TRACE_H_*/
