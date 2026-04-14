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


#ifndef __VVBASE_H__
#define __VVBASE_H__

#define FILE_LEN 256
#define FILE_NAME 128
#define VERSION_LEN 10

#ifndef ALIGN_UP
	#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align)-1))
#endif

#define ALIGN_16BYTE(width)         (((width) + (0xF)) & (~(0xF)))
/******************************************************************************
 * VvbenchCtx_t
 *****************************************************************************/
typedef struct VvbenchCtx_s {
	const char *cfgJsonFile;
	const char *caseJsonFile;
	const char *swSimuName;
} VvbenchCtx_t;

typedef struct VvbenchCfg_s {
	unsigned int alignMask;
	int streamDuration;
	int useTerminal;
	char swSimuName[FILE_NAME];
	char rawName[FILE_NAME];
} VvbenchCfg_t;

typedef enum VvbenchRunMode_e {
	INVALID_MODE = -1,
	DEFAULT_CASE_LIST,
	LOAD_IMAGE_CASE_LIST,
	LOAD_IMAGE_CASE,
	LOAD_IMAGE_CASE_BY_TOOL
} VvbenchRunMode_t;

#endif //__VVBASE_H__
