/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPFW_MOD_WDT_H_
#define XPFW_MOD_WDT_H_

#ifdef __cplusplus
extern "C" {
#endif

void ModWdtInit(void);
void XPfw_WdtSetVal(u32 TimeOutVal);
void InitCsuPmuWdt(void);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_MOD_WDT_H_ */
