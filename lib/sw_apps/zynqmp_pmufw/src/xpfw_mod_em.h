/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPFW_MOD_EM_H_
#define XPFW_MOD_EM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define EM_IPI_HANDLER_ID		0xEU
#define EM_API_ID_MASK			0xFFFFU

#define EM_MOD_API_ID_OFFSET	0x0U
#define EM_ERROR_ID_OFFSET		0x1U
#define EM_ERROR_ACTION_OFFSET	0x2U
#define PMU_BRAM_CE_LOG_OFFSET	0x3U
#define EM_ERROR_LOG_MAX		0x4U

/* EM API IDs */
#define SET_EM_ACTION			0x01U
#define REMOVE_EM_ACTION		0x02U
#define SEND_ERRORS_OCCURRED	0x03U

/* EM error codes */
#define PERMISSION_DENIED		0xFU

extern u32 ErrorLog[EM_ERROR_LOG_MAX];
void ModEmInit(void);
void RpuLsHandler(u8 ErrorId);
void SwdtHandler(u8 ErrorId);
void NullHandler(u8 ErrorId);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_MOD_EM_H_ */
