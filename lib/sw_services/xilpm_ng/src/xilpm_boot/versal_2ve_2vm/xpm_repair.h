/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#ifndef XPM_REPAIR_H_
#define XPM_REPAIR_H_

#include "xpm_bisr.h"
#include "xpm_regs.h"
#include "xpm_powerdomain.h"
#include "xpm_repair_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

maybe_unused static inline XStatus XPmRepair_Vdu(u32 EfuseTagAddr, u32 TagSize,
                        u32 TagOptional, u32 *TagDataAddr)
{
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagOptional;
	(void)TagDataAddr;

	return XST_INVALID_PARAM;
}

maybe_unused static inline XStatus XPmRepair_Bfrb(u32 EfuseTagAddr, u32 TagSize,
			u32 TagOptional, u32 *TagDataAddr)
{
	(void)EfuseTagAddr;
	(void)TagSize;
	(void)TagOptional;
	(void)TagDataAddr;

	return XST_INVALID_PARAM;
}

/************************** Function Prototypes ******************************/
XStatus XPmRepair_Lpx(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr);
XStatus XPmRepair_Fpx(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr);
XStatus XPmRepair_Hnicx_Nthub(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr);
XStatus XPmRepair_Cpm5n(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr);
XStatus XPmRepair_Ddrmc5_Crypto(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional,
	 u32 *TagDataAddr);
XStatus XPmRepair_Ddrmc5_Main(u32 EfuseTagAddr, u32 TagSize,
                u32 TagOptional, u32 *TagDataAddr);
XStatus XPmRepair_Hnicx_Dpu(u32 EfuseTagAddr, u32 TagSize,
            u32 TagOptional, u32 *TagDataAddr);
XStatus XPmRepair_Hnicx_Lcs(u32 EfuseTagAddr, u32 TagSize,
            u32 TagOptional, u32 *TagDataAddr);
#ifdef __cplusplus
}
#endif

#endif /* XPM_REPAIR_H_ */
