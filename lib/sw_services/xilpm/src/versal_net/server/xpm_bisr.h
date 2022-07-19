/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_BISR_H_
#define XPM_BISR_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "xpm_common.h"

#define LPD_TAG_ID 	0x03
#define FPD_TAG_ID 	0x04
#define CPM_TAG_ID 	0x05
#define XRAM_TAG_ID	0x06
#define CPM5_TAG_ID 	0x07
#define MEA_TAG_ID 	0x08
#define MEB_TAG_ID 	0x09
#define MEC_TAG_ID 	0x0A
#define DDRMC_TAG_ID 	0x0B
#define GTY_TAG_ID 	0x0C
#define DCMAC_TAG_ID	0x0D
#define ILKN_TAG_ID	0x0E
#define MRMAC_TAG_ID	0x0F
#define SDFEC_TAG_ID	0x10
#define BRAM_TAG_ID	0x11
#define URAM_TAG_ID 	0x12
#define LAGUNA_TAG_ID   0x13
#define VDU_TAG_ID  0x15
#define HSC_TAG_ID	0x16U
#define CPM5_GTYP_TAG_ID	0x17
#define GTYP_TAG_ID	0x18
#define GTM_TAG_ID	0x19

XStatus XPmBisr_Repair(u32 TagId);
XStatus XPmBisr_NidbLeftMostLaneRepair(void);
XStatus XPmBisr_NidbLaneRepair(void);
XStatus XPmBisr_TriggerLpd(void);

#ifdef __cplusplus
}
#endif

#endif /* XPM_BISR_H_ */
