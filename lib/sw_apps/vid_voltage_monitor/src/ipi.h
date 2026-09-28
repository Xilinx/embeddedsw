/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file ipi.h
 * @brief IPI channel setup for the XilPM client (polled mode).
 */
#ifndef IPI_H
#define IPI_H

#include <xipipsu.h>
#include <xstatus.h>

XStatus IpiInit(XIpiPsu *const InstancePtr);

#endif /* IPI_H */
