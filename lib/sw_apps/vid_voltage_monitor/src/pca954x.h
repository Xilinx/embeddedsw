/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file pca954x.h
 * @brief PCA9541 two-master I2C bus arbiter and PCA9848 8-channel mux.
 */
#ifndef PCA954X_H
#define PCA954X_H

#include "xil_types.h"
#include "xstatus.h"

XStatus Pca9541_CheckOwnership(u8 *const Owned, u8 *const Ctl);
XStatus Pca9541_Acquire(void);
XStatus Pca9541_Release(void);
XStatus Pca9848_Select(const u8 ChannelMask);

#endif /* PCA954X_H */
