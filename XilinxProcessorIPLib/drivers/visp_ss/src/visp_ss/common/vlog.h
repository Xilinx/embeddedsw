/******************************************************************************\
|* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2023 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

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
#ifdef VLOG_ENABLE_ANSI_COLORS
#define VLOG_COLOR_RESET "\033[0m"
#define VLOG_COLOR_DEBUG "\033[1;30;37m"
#define VLOG_COLOR_INFO  "\033[1;30;32m"
#define VLOG_COLOR_WARN  "\033[1;30;33m"
#define VLOG_COLOR_ERROR "\033[1;30;31m"
#else
#define VLOG_COLOR_RESET ""
#define VLOG_COLOR_DEBUG ""
#define VLOG_COLOR_INFO  ""
#define VLOG_COLOR_WARN  ""
#define VLOG_COLOR_ERROR ""
#endif

#define __VLOG_INT(format, ...) "[%s] " format VLOG_COLOR_RESET "%s", LOGTAG, __VA_ARGS__

#define LOGD(...)\
	if (VsiVvbenchGetLogLevel() <= VVLOG_LEVEL_DEBUG) { \
		printf("[%.6lf]",timeStampGet()); \
		printf(VLOG_COLOR_DEBUG "DEBUG  : " __VLOG_INT(__VA_ARGS__, "\n")); \
	} // white

#define LOGI(...)\
	if (VsiVvbenchGetLogLevel() <= VVLOG_LEVEL_INFO) { \
		printf("[%.6lf]",timeStampGet()); \
		printf(VLOG_COLOR_INFO "INFO  : " __VLOG_INT(__VA_ARGS__, "\n")); \
	} // green

#define LOGW(...)\
	if (VsiVvbenchGetLogLevel() <= VVLOG_LEVEL_WARNING) { \
		printf("[%.6lf]",timeStampGet()); \
		printf(VLOG_COLOR_WARN "WARN  : " __VLOG_INT(__VA_ARGS__, "\n")); \
	} // yellow

#define LOGE(...)\
	if (VsiVvbenchGetLogLevel() <= VVLOG_LEVEL_ERROR) { \
		printf("[%.6lf]",timeStampGet()); \
		printf(VLOG_COLOR_ERROR "ERROR  : " __VLOG_INT(__VA_ARGS__, "\n")); \
	} // red

#endif  // _VLOG_H_
