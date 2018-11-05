/******************************************************************************
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/

#ifndef _CSU_H_
#define _CSU_H_

#ifdef __cplusplus
extern "C" {
#endif

#define	CSU_BASE							((u32)0XFFCA0000U)
#define	CSU_IDCODE							( ( CSU_BASE ) + ((u32)0X00000040U) )
#define	CSU_VERSION							( ( CSU_BASE ) + ((u32)0X00000044U) )

#define	CSU_PCAP_STATUS_REG					( ( CSU_BASE ) + ((u32)0x00003010U) )
#define	CSU_PCAP_STATUS_PL_INIT_SHIFT_VAL	2
#define	CSU_PCAP_STATUS_PL_INIT_MASK_VAL	0x4U

#define	CSU_VERSION_EMPTY_SHIFT				20
#define	CSU_VERSION_PL_STATE_SHIFT			29

#ifdef __cplusplus
}
#endif


#endif /* _CSU_H_ */
