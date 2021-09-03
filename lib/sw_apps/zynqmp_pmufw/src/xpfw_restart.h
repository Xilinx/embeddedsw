/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPFW_RESTART_H_
#define XPFW_RESTART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "pm_master.h"

#define FSBL_STORE_ADDR					(XPAR_MICROBLAZE_DDR_RESERVE_SA + 0x80000U)
#define FSBL_LOAD_ADDR					0xFFFC0000U
#define FSBL_IMAGE_SIZE					(170U*1024U)
#define SHA3_HASH_LENGTH_IN_WORDS		12U

#define FSBL_STATE_PROC_SHIFT			(0x1U)

#define FSBL_RUNNING_ON_A53				(0x1U << FSBL_STATE_PROC_SHIFT)
#define FSBL_RUNNING_ON_R5_0			(0x2U << FSBL_STATE_PROC_SHIFT)
#define FSBL_RUNNING_ON_R5_L			(0x3U << FSBL_STATE_PROC_SHIFT)

#define FSBL_STATE_PROC_INFO_MASK		(0x3U << FSBL_STATE_PROC_SHIFT)
#define FSBL_ENCRYPTION_STS_MASK		(0x8U)

#define XPFW_OCM_IS_USED				(0x1U)
#define XPFW_FSBL_IS_COPIED				(0x2U)

/* Structure for FSBL copy and APU restart */
typedef struct FSBL_Store_Restore_Info_Struct {
	u32 FSBLImageHash[SHA3_HASH_LENGTH_IN_WORDS];
	u8 OcmAndFsblInfo;
}FSBL_Store_Restore_Info_Struct;

extern FSBL_Store_Restore_Info_Struct FSBL_Store_Restore_Info;

s32 XPfw_RecoveryInit(void);
void XPfw_RecoveryHandler(u8 ErrorId);
void XPfw_RecoveryAck(PmMaster *Master);
void XPfw_RecoveryStop(PmMaster *Master);
void XPfw_RecoveryRestart(PmMaster *Master);

#if defined(USE_DDR_FOR_APU_RESTART) && defined(ENABLE_SECURE)
s32 XPfw_StoreFsblToDDR(void);
s32 XPfw_RestoreFsblToOCM(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XPFW_RESTART_H_ */
