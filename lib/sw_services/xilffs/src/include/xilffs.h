/******************************************************************************
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilffs.h
 * @addtogroup xilffs Overview
 * @{
 * @details
 *
 * This file contains declarations specific to AMD's unique requirements and
 * functionalities for xilffs.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 5.2   ht   10/10/23    Added code for versioning of library.
 * 5.6   an   11/12/25    Update the FatFS version to R0.16
 * 5.7   an   08/26/26    Updated Library version for 2026.2
 *
 *</pre>
 *
 *@note
 *****************************************************************************/
#ifndef XILFFS_H
#define XILFFS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_util.h"
#include "xil_io.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/
/* Library version info */
#define XILFFS_MAJOR_VERSION	5U	/**< Major version of xilffs library */
#define XILFFS_MINOR_VERSION	7U	/**< Minor version of xilffs library */

/****************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 *
 * @brief	This function returns the version number of xilffs library.
 *
 * @return	32-bit version number
 *
******************************************************************************/
static __attribute__((always_inline)) INLINE
u32 Xilffs_GetLibVersion(void)
{
	return (XIL_BUILD_VERSION(XILFFS_MAJOR_VERSION, XILFFS_MINOR_VERSION));
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XILFFS_H */
