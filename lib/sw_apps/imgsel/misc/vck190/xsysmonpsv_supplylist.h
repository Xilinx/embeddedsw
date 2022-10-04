/*******************************************************************
* Copyright (C) 2022 Xilinx, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
* Description: Enabled Supply List
*
*******************************************************************/

#ifndef XSYSMONPSV_SUPPLYLIST
#define XSYSMONPSV_SUPPLYLIST

/*
* The supply configuration table for sysmon
*/

typedef enum {
	EndList,
	NO_SUPPLIES_CONFIGURED = XPAR_XSYSMONPSV_0_NO_MEAS,
} XSysMonPsv_Supply;

#endif
