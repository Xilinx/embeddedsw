/******************************************************************************
* Copyright (C) 2015 - 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
******************************************************************************/

#ifndef XPFW_MOD_EM_H_
#define XPFW_MOD_EM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define EM_IPI_HANDLER_ID		0xEU
#define EM_API_ID_MASK			0xFFFFU

#define EM_MOD_API_ID_OFFSET	0x0U
#define EM_ERROR_ID_OFFSET		0x1U
#define EM_ERROR_ACTION_OFFSET	0x2U
#define PMU_BRAM_CE_LOG_OFFSET	0x3U
#define EM_ERROR_LOG_MAX		0x4U

/* EM API IDs */
#define SET_EM_ACTION			0x01U
#define REMOVE_EM_ACTION		0x02U
#define SEND_ERRORS_OCCURRED	0x03U

/* EM error codes */
#define PERMISSION_DENIED		0xFU

extern u32 ErrorLog[EM_ERROR_LOG_MAX];
void ModEmInit(void);
void RpuLsHandler(u8 ErrorId);
void SwdtHandler(u8 ErrorId);
void NullHandler(u8 ErrorId);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_MOD_EM_H_ */
