/*******************************************************************
* Copyright (C) 2022 Xilinx, Inc. All Rights Reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
* Description: Enabled Supply List
*
*******************************************************************/

#ifndef XSYSMONPSV_SUPPLYLIST
#define XSYSMONPSV_SUPPLYLIST

#ifdef __cplusplus
extern "c" {
#endif

/*
* The supply configuration table for sysmon
*/

typedef enum {
	EndList,
	NO_SUPPLIES_CONFIGURED = XPAR_XSYSMONPSV_0_NO_MEAS,
} XSysMonPsv_Supply;

#ifdef __cplusplus
}
#endif

#endif
