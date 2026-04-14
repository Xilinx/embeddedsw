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


#ifndef __VVPATH_H__
#define __VVPATH_H__

#include "vvdevice.h"
#include "vvbparser.h"

int VsiVvdeviceInputPathCreate
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t bufIo
);

int VsiVvdeviceOutPutPathCreate
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t bufIo
);

int VsiVvdevicePathEnable
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t *bufIo,
	int bufCount
);

int VsiVvdevicePathDisable
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t *bufIo,
	int bufCount
);

int VsiVvdevicePathRelease
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t bufIo
);

int VsiVvdevicePathStart
(
	VvbenchInstance_t *pVvInstance,
	VvbenchVvdev_t *caseCtx,
	int instanceId,
	CamDeviceBufChainId_t *pBufIo,
	int bufCount

);

#endif //__VVPATH_H__
