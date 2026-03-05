// Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
/* Header for cam_common_api - implementation provided by libvisp.a */
#ifndef CAM_COMMON_API_H
#define CAM_COMMON_API_H

#include <stdint.h>
#include <stdbool.h>

/* Common handle type used across cam_device APIs */
typedef void *CamCommonHandle_t;

/* Result codes */
typedef int32_t RESULT;
#define RET_SUCCESS 0
#define RET_FAILURE -1

#endif /* CAM_COMMON_API_H */
