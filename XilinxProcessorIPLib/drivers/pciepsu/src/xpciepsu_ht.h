/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
* @file xpciepsu_ht.c
*
* This file contains the API definitions for set and get of bar address
* with vendor, device id and bar numbers
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 0.1	bs	08/21/2018	First release
* </pre>
*
*******************************************************************************/
#ifndef SRC_XPCIEPSU_HT_H_
#define SRC_XPCIEPSU_HT_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************** Include Files *******************************/

#include "stdlib.h"
#include "stdio.h"
#include "xil_types.h"

/****************************** Type Definitions ******************************/
#define MAX_BARS	6

typedef struct entry_s {
	u8 bus;
	u8 device;
	u8 function;
	u32 vendorIdDeviceId;
	u64 barAddress[MAX_BARS];
	struct entry_s *next;
} XEntry_t;

/***************************** Function Prototypes ****************************/

void ht_set(u32 venDevId, u64 bar_address[], u8 Bus, u8 Device,
	    u8 Function);
u64 ht_get_bar_addr(u8 Bus, u8 Device, u8 Function, u8 bar_no);
u32 ht_get_device_vendor_id(u8 Bus, u8 Device, u8 Function);
void ht_print_all();
void ht_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* SRC_XPCIEPSU_HT_H_ */
