/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_PLDOMAIN_H_
#define XPM_PLDOMAIN_H_

#include "xpm_pldomain_plat.h"


#ifdef __cplusplus
extern "C" {
#endif


/* TRIM Types */
#define XPM_PL_TRIM_VGG          (0x1U)
#define XPM_PL_TRIM_CRAM         (0x2U)
#define XPM_PL_TRIM_BRAM         (0x3U)
#define XPM_PL_TRIM_URAM         (0x4U)

/************************** Function Prototypes ******************************/
XStatus XPmPlDomain_RetriggerPlHouseClean(void);
XStatus ReduceCfuClkFreq(void);
XStatus RestoreCfuClkFreq(void);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* XPM_PLDOMAIN_H_ */
