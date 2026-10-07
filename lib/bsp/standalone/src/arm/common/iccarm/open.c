/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file open.c
 * @brief File open stub implementation for standalone BSP.
 */


/*
 *
 * This is a template implementation of the "__open" function used by
 * the standard library.  Replace it with a system-specific
 * implementation.
 *
 * The "__open" function opens the file named "filename" as specified
 * by "mode".
 * open -- open a file descriptor. We don't have a filesystem, so
 *         we return an error.
 *
 */

#include <yfuns.h>
#include "xil_types.h"

sint32 __open(const char8 * filename, sint32 mode);

/**
 * @brief Stub for __open(); always returns 0.
 * @param[in] filename Filename string (unused).
 * @param[in] mode Open mode (unused).
 * @return Always 0.
 */
sint32 __open(const char8 * filename, sint32 mode)
{
return 0;
}
