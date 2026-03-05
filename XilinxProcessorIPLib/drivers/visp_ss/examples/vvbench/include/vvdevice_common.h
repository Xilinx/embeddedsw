/******************************************************************************\
|* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2023 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#include "vvdevice.h"
#include <types.h>
#include "cJSON.h"
#ifndef __VVDEV_COMMON_H_
#define __VVDEV_COMMON_H_

#define ISO_STRENGTH_NUM 20
#define TABLE_NUM 2
#define RAW_CHN_NUM 4

enum auto_type {
	TYPE_LINEAR = 0,
	TYPE_HDR
};

/* Forward declaration - actual type provided by libvisp.a */
typedef void *TCommonMapHandle_t;

typedef struct DbContext_s {
	TCommonMapHandle_t handleMap;
} DbContext_t;

typedef void *CamDeviceHandle_t;
typedef void *CamDeviceVi200Handle_t;

#endif //__VVDEV_COMMON_H_
