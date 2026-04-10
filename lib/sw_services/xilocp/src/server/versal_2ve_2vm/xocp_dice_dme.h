/***************************************************************************************************
* Copyright (c) 2025 - 2026, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_dice_dme.h
*
* This file contains the xilocp DME challenge signature declarations for versal_2ve_2vm.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.5   tvp  06/05/25 Initial release
* 1.7   rpu  02/18/26 Fixed Doxygen warnings
* </pre>
*
* @note
*
***************************************************************************************************/

#ifndef XOCP_DICE_DME_H
#define XOCP_DICE_DME_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/

#include "xplmi_config.h"
#ifdef PLM_OCP

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/

/************************************ Function Prototypes *****************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/**************************************************************************************************/
/**
 * @brief	This function generates error as DME response generation is not supported from
 * 		ROM/PLM.
 *
 * @param	NonceAddr holds the address of 32 bytes buffer Nonce,which shall be used to fill one
 * 		of the member of DME structure.
 *
 * @param	DmeStructResAddr is the address to the 224 bytes buffer, which is used to store the
 * 		response to DME challenge request of type XOcp_DmeResponse.
 *
 * @return
 *		- XST_FAILURE - As DME response generation not supported.
 *
 **************************************************************************************************/
static inline int XOcp_GenerateDmeResponseImpl(u64 NonceAddr, u64 DmeStructResAddr)
{
	(void)NonceAddr;
	(void)DmeStructResAddr;

	return XST_FAILURE;
}
#endif /* PLM_OCP */

#ifdef __cplusplus
}
#endif
#endif /* XOCP_DICE_DME_H_ */
