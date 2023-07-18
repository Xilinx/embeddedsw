/******************************************************************************
*
* Copyright (C) 2004 - 2014 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/* Note: This file depends on the following files having been included prior to self being included.
   1. portab.h
*/

#ifndef BL_SREC_H
#define BL_SREC_H

#define SREC_MAX_BYTES        255  /* Maximum record length */
#define SREC_DATA_MAX_BYTES   123  /* Maximum of 123 data bytes */

#define SREC_TYPE_0  0
#define SREC_TYPE_1  1
#define SREC_TYPE_2  2
#define SREC_TYPE_3  3
#define SREC_TYPE_5  5
#define SREC_TYPE_7  7
#define SREC_TYPE_8  8
#define SREC_TYPE_9  9


typedef struct srec_info_s {
    int8    type;
    uint8*  addr;
    uint8*  sr_data;
    uint8   dlen;
} srec_info_t;

uint8   decode_srec_line (uint8 *sr_buf, srec_info_t *info);

#endif /* BL_SREC_H */
