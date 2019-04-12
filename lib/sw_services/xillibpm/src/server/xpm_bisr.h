/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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

#ifndef XPM_BISR_H_
#define XPM_BISR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpm_common.h"

#define LPD_TAG_ID 	0x03
#define FPD_TAG_ID 	0x04
#define CPM_TAG_ID 	0x05
#define MEA_TAG_ID 	0x08
#define MEB_TAG_ID 	0x09
#define MEC_TAG_ID 	0x0A
#define DDRMC_TAG_ID 	0x0B
#define GTY_TAG_ID 	0x0C
#define DCMAC_TAG_ID	0x0D
#define ILKN_TAG_ID	0x0E
#define MRMAC_TAG_ID	0x0F
#define SDFEC_TAG_ID	0x10
#define BRAM_TAG_ID	0x11
#define URAM_TAG_ID 	0x12

#define PCSR_UNLOCK_VAL		(0xF9E8D7C6U)
XStatus XPmBisr_Repair(u32 TagId);

#ifdef __cplusplus
}
#endif

#endif /* XPM_BISR_H_ */
