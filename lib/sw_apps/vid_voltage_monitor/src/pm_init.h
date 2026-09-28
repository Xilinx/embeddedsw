/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file pm_init.h
 * @brief PM subsystem bring-up (IPI + XilPM) for the example.
 */
#ifndef PM_INIT_H
#define PM_INIT_H

#include "ipi.h"

XStatus PmInit(XIpiPsu *const IpiInst);

#endif /* PM_INIT_H */
