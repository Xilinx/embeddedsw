/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xtpm_cmd.h
 *
 * This is the header file which contains TPM commands specific definitions for PLM.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.3   pre  03/09/26 Initial release
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

#ifndef XTPM_CMD_H
#define XTPM_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_TPM

void XTpm_CmdsInit(void);

#endif	/* PLM_TPM */

#ifdef __cplusplus
}
#endif

#endif	/* XTPM_CMD_H */
