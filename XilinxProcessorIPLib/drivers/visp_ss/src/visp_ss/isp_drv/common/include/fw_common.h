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
 * @file    firmware_common.h
 *
 * @brief   Defines common structure can be used in firmware sdk.
 *
 *****************************************************************************/
#ifndef __FW_COMMON__
#define __FW_COMMON__

#include <types.h>

typedef struct FwCommonIspFuSaMisVal_s {
	uint32_t fusaEcc1MisVal;
	uint32_t fusaEcc2MisVal;
	uint32_t fusaEcc3MisVal;
	uint32_t fusaEcc4MisVal;
	uint32_t fusaEcc5MisVal;
	uint32_t fusaEcc6MisVal;
	uint32_t fusaDupMisVal;
	uint32_t fusaParityMisVal;
	uint32_t fusaLv1MisVal;
} FwCommonIspFuSaMisVal_t;

/*****************************************************************************/
/**
 * @brief   Cam Device FUSA event ID.
 *
 *****************************************************************************/
typedef enum FwCommonEventId_e {
	FW_COMMON_EVENT_INVALID = 0x0000,    /**< Invalid event (only for an internal evaluation) */
	FW_COMMON_EVENT_FUSA = 0x0001,    /**< FUSA events */
	DUMMY_FW_COMMON_EVENT = 0xDEADFEED,
} FwCommonEventId_t;


#endif
