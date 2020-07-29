/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplm_loader.h
*
* This file contains the declarations of platform loader wrapper functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/20/2018 Initial release
* 1.01  ma   08/24/2019 Added code to force bootmode to SBI
*						for slave SLRs
* 1.02  ana  02/29/2020 Implemented KAT support for crypto engines
*       kc   03/23/2020 Minor code cleanup
* 1.03  kc   06/12/2020 Added IPI mask to PDI CDO commands to get
*						subsystem information
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLM_LOADER_H
#define XPLM_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplm_default.h"
#include "xplmi_hw.h"
#include "xloader.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_KAT_DONE        (0x000001F0U)
#define EFUSE_CACHE_MISC_CTRL   (0xF12500A0U)
#define EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK        (0X00008000U)

/*****************************************************************************/
/**
 * @brief	This function checks if the boot mode is jtag or not.
 *
 * @param	Void
 *
 * @return	TRUE if JTAG and FALSE otherwise
 *
 *****************************************************************************/
static inline u8 XLoader_IsJtagSbiMode(void)
{
	return (u8)(((XPlmi_In32(SLAVE_BOOT_SBI_MODE) &
				SLAVE_BOOT_SBI_MODE_JTAG_MASK) ==
				SLAVE_BOOT_SBI_MODE_JTAG_MASK) ?
				(TRUE) : (FALSE));
}

/************************** Function Prototypes ******************************/
int XPlm_LoaderInit(void);
int XPlm_LoadBootPdi(void *Arg);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_LOADER_H */
