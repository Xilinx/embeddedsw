/******************************************************************************
* Copyright (c) 2022 - 2023, Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2025, Advanced Micro Devices, Inc.  All rights reserved.
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
