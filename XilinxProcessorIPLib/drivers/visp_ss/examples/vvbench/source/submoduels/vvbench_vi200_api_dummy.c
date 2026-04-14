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


#define LOGTAG "VI200DUMMY"
#include "cJSON.h"
#include "vlog.h"
#include "vvbench_vi200_api.h"

int VsiVvbenchParseVi200Info
(
	cJSON *vi200Info,
	VvbenchInstanceVi200Cfg_t *vi200Ctrl
)
{
	return 0;
}

int VsiVvbenchVi200Func
(
	CamDeviceVi200Handle_t hVi200,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	return 0;
}

int VsiVvbenchVi200MetadataGetBufferSize
(
	CamDeviceVi200Handle_t hVi200,
	uint32_t *pBufSize
)
{
	return 0;
}

int VsiVvbenchVi200TpgFunc
(
	CamDeviceVi200Handle_t hVi200,
	VvbenchVvdev_t *caseCtx,
	int index
)
{
	return 0;
}
