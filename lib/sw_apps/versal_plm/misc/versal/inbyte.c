/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xparameters.h"
#include "xuartpsv_hw.h"

#ifdef __cplusplus
extern "C" {
#endif
char inbyte(void);
#ifdef __cplusplus
}
#endif 

char inbyte(void) {
	 return XUartPsv_RecvByte(STDIN_BASEADDRESS);
}
