/*
 * Copyright (c) 2016-2022 Xilinx, Inc. and Contributors. All rights reserved.
 * Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	time.h
 * @brief	Time primitives for libmetal.
 */

#ifndef __METAL_TIME__H__
#define __METAL_TIME__H__

#include <stdint.h>
#include <metal/sys.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup time TIME Interfaces
 *  @{
 */

/**
 * @brief      get timestamp
 *             This function returns the timestampe as unsigned long long
 *             value.
 *
 * @return     timestamp
 */
unsigned long long metal_get_timestamp(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_TIME__H__ */

