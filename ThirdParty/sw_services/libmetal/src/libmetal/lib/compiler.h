/*
 * Copyright (c) 2015-2022 Xilinx, Inc. and Contributors. All rights reserved.
 * Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	compiler.h
 * @brief	Compiler specific primitives for libmetal.
 */

#ifndef __METAL_COMPILER__H__
#define __METAL_COMPILER__H__

#if defined(__GNUC__)
# include <metal/compiler/gcc/compiler.h>
#elif defined(__ICCARM__)
# include <metal/compiler/iar/compiler.h>
#elif defined(__CC_ARM)
# error "MDK-ARM ARMCC compiler requires the GNU extensions to work correctly"
#else
# error "Missing compiler support"
#endif

#endif /* __METAL_COMPILER__H__ */
