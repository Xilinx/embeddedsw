/******************************************************************************\
|* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2023 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")  *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets       *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#ifndef __VVBASE_H__
#define __VVBASE_H__

#define FILE_LEN 256
#define FILE_NAME 128
#define VERSION_LEN 10

#ifndef ALIGN_UP
	#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align)-1))
#endif

#define ALIGN_16BYTE(width)         (((width) + (0xF)) & (~(0xF)))
/******************************************************************************
 * VvbenchCtx_t
 *****************************************************************************/
typedef struct VvbenchCtx_s {
	const char *cfgJsonFile;
	const char *caseJsonFile;
	const char *swSimuName;
} VvbenchCtx_t;

typedef struct VvbenchCfg_s {
	unsigned int alignMask;
	int streamDuration;
	int useTerminal;
	char swSimuName[FILE_NAME];
	char rawName[FILE_NAME];
} VvbenchCfg_t;

typedef enum VvbenchRunMode_e {
	INVALID_MODE = -1,
	DEFAULT_CASE_LIST,
	LOAD_IMAGE_CASE_LIST,
	LOAD_IMAGE_CASE,
	LOAD_IMAGE_CASE_BY_TOOL
} VvbenchRunMode_t;

#endif //__VVBASE_H__
