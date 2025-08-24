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
 * @file return_codes.h
 *
 * @brief
 *   This header files contains general return codes .
 *
 *****************************************************************************/
#ifndef __RETURN_CODES_H__
#define __RETURN_CODES_H__

typedef int RESULT;

#define RET_SUCCESS             0   //!< this has to be 0, if clauses rely on it
#define RET_FAILURE             1   //!< general failure
#define RET_NOTSUPP             2   //!< feature not supported
#define RET_BUSY                3   //!< there's already something going on...
#define RET_CANCELED            4   //!< operation canceled
#define RET_OUTOFMEM            5   //!< out of memory
#define RET_OUTOFRANGE          6   //!< parameter/value out of range
#define RET_IDLE                7   //!< feature/subsystem is in idle state
#define RET_WRONG_HANDLE        8   //!< handle is wrong
#define RET_NULL_POINTER        9   //!< the/one/all parameter(s) is a(are) NULL pointer(s)
#define RET_NOTAVAILABLE       10   //!< profile not available
#define RET_DIVISION_BY_ZERO   11   //!< a divisor equals ZERO
#define RET_WRONG_STATE        12   //!< state machine in wrong state
#define RET_INVALID_PARM       13   //!< invalid parameter
#define RET_PENDING            14   //!< command pending
#define RET_WRONG_CONFIG       15   //!< given configuration is invalid
#define RET_TIME_OUT           16   //!< wait time out
#define RET_UNSUPPORT_ID   0xFFFF   //!< unsupport index number


#define UPDATE_RESULT( cur_res, new_res ) { RESULT __lres__ = (new_res); if (cur_res == RET_SUCCESS) cur_res = __lres__; } //!< Keeps first non-success result; cur_res must be a modifiable L-value; new_res can be any type of R-value including function call.

#define RETURN_RESULT_IF_DIFFERENT( cur_res, exp_res ) if ( exp_res != cur_res ) { return ( cur_res ); }

#endif /* __RETURN_CODES_H__ */
