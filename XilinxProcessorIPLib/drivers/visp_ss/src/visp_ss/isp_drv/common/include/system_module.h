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

#ifndef __SYSTEM_MODULE_H__
#define __SYSTEM_MODULE_H__

#include <types.h>

typedef enum IspSystemPortType_e {
	ISP_SYSTEM_PORT_TYPE_INVALID = 0,
	ISP_SYSTEM_PORT_TYPE_INPUT = 1,
	ISP_SYSTEM_PORT_TYPE_OUTPUT = 2,
	ISP_SYSTEM_PORT_TYPE_MAX
} IspSystemPortType_t;

typedef enum IspSystemBufferType_e {
	ISP_SYSTEM_BUFFER_TYPE_INVALID = 0,
	ISP_SYSTEM_BUFFER_TYPE_EMPTY = 1,
	ISP_SYSTEM_BUFFER_TYPE_FULL = 2,
	ISP_SYSTEM_BUFFER_TYPE_MAX
} IspSystemBufferType_t;

typedef struct IspSystemPortInfo_s {
	IspSystemPortType_t portType;
	IspSystemBufferType_t bufferType;
	uint8_t portId;
} IspSystemPortInfo_t;

typedef RESULT (* IspSystemEnqueueBuf_t)
(
	void *pHandle,
	void *pModuleHandle,
	IspSystemPortInfo_t *pPortInfo,
	void *pMediaBuf
);

typedef RESULT (* IspSystemDequeueBuf_t)
(
	void *pHandle,
	void *pModuleHandle,
	IspSystemPortInfo_t *pPortInfo,
	void **pMediaBuf
);


#endif  // __CAMERA_DEVICE_BUF_DEFS_H__
