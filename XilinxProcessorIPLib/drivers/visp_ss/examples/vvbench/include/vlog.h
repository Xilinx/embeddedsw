// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2026 Vivantec Corporation
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
 ******************************************************************************/


#ifndef _VLOG_H_
#define _VLOG_H_

#include <stdio.h>
#include <stdlib.h>
#include <oslayer.h>

enum {
	VVLOG_LEVEL_VERBOSE = 0,
	VVLOG_LEVEL_DEBUG,
	VVLOG_LEVEL_INFO,
	VVLOG_LEVEL_WARNING,
	VVLOG_LEVEL_ERROR,
};

extern int vvbenchLogLevel;

static inline int VsiVvbenchGetLogLevel()
{
	if (vvbenchLogLevel != -1)
		return vvbenchLogLevel;

	vvbenchLogLevel = VVLOG_LEVEL_DEBUG;
	return vvbenchLogLevel;
}

static inline float32_t timeStampGet()
{
	float32_t timeStamp;

	osTimeStampNs(&timeStamp);

	return timeStamp;
}

// c++11 workaround empty variadic macro
#define __VLOG_INT(format, ...) "[%s] " format "\033[0m%s", LOGTAG, __VA_ARGS__

#define LOGD(...)\
	if (VsiVvbenchGetLogLevel() <= VVLOG_LEVEL_DEBUG) { \
		printf("[%.6lf]",timeStampGet()); \
		printf("\033[1;30;37mDEBUG  : " __VLOG_INT(__VA_ARGS__, "\n")); \
	} // white

#define LOGI(...)\
	if (VsiVvbenchGetLogLevel() <= VVLOG_LEVEL_INFO) { \
		printf("[%.6lf]",timeStampGet()); \
		printf("\033[1;30;32mINFO  : " __VLOG_INT(__VA_ARGS__, "\n")); \
	} // green

#define LOGW(...)\
	if (VsiVvbenchGetLogLevel() <= VVLOG_LEVEL_WARNING) { \
		printf("[%.6lf]",timeStampGet()); \
		printf("\033[1;30;33mWARN  : " __VLOG_INT(__VA_ARGS__, "\n")); \
	} // yellow

#define LOGE(...)\
	if (VsiVvbenchGetLogLevel() <= VVLOG_LEVEL_ERROR) { \
		printf("[%.6lf]",timeStampGet()); \
		printf("\033[1;30;31mERROR  : " __VLOG_INT(__VA_ARGS__, "\n")); \
	} // red

#endif  // _VLOG_H_
