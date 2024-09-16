/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
#include "xparameters.h"
#include "xuartpsv_hw.h"

#ifdef __cplusplus
extern "C" {
#endif
char inbyte(void);
#ifdef __cplusplus
}
#endif

char inbyte(void)
{
	return XUartPsv_RecvByte(STDIN_BASEADDRESS);
}
