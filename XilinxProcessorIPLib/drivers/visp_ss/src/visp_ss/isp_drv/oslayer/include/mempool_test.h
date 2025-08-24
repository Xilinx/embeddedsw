/******************************************************************************\
|* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2024 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#ifndef __MEMPOOL_TEST_H__
#define __MEMPOOL_TEST_H__

#include "oslayer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Quick memory pool test - call this early in your application
 * @return OSLAYER_OK on success, OSLAYER_ERROR on failure
 */
int32_t osMemPoolQuickTest(void);

/**
 * @brief Comprehensive memory pool stress test
 * @return OSLAYER_OK on success, OSLAYER_ERROR on failure
 */
int32_t osMemPoolStressTest(void);

#ifdef __cplusplus
}
#endif

#endif /* __MEMPOOL_TEST_H__ */
