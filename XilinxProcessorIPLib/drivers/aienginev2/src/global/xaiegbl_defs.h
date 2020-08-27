/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaiegbl_defs.h
* @{
*
* This file contains the generic definitions for the AIE drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/23/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* 1.2  Hyun    10/10/2018  Added the mask write API
* 1.3  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.4  Hyun    01/08/2019  Add the mask poll function
* 1.5  Tejus   08/01/2019  Restructure code for AIE
* 1.6  Dishita 04/17/2020  Fix compiler warning
* 1.7  Dishita 05/07/2020  Removed Reset related macros
* 1.8  Tejus   06/09/2020  Remove NPI apis.
* </pre>
*
******************************************************************************/
#ifndef XAIEGBL_DEFS_H
#define XAIEGBL_DEFS_H

/***************************** Include Files *********************************/
#include "xaielib.h"

/************************** Constant Definitions *****************************/
#define XAIE_DEV_GEN_AIE		1U

#define XAIE_SUCCESS			XAIELIB_SUCCESS
#define XAIE_FAILURE			XAIELIB_FAILURE
#define XAIE_COMPONENT_IS_READY		XAIELIB_COMPONENT_IS_READY

#define XAIE_NULL			(void *)0U
#define XAIE_ENABLE			1U
#define XAIE_DISABLE			0U

#define XAIEGBL_TILE_TYPE_AIETILE	0U
#define XAIEGBL_TILE_TYPE_SHIMNOC	1U
#define XAIEGBL_TILE_TYPE_SHIMPL	2U
#define XAIEGBL_TILE_TYPE_RESERVED	3U
#define XAIEGBL_TILE_TYPE_MAX		4U

#define XAIEGBL_CMDIO_COMMAND_SETSTACK	XAIELIB_CMDIO_COMMAND_SETSTACK
#define XAIEGBL_CMDIO_COMMAND_LOADSYM	XAIELIB_CMDIO_COMMAND_LOADSYM
#define XAIEGBL_TILE_BASE_ADDRESS        XAIELIB_TILE_BASE_ADDRESS

#define XAie_print			XAieLib_print
#define XAie_usleep			XAieLib_usleep

#define XAie_SetField(Val, Lsb, Mask)	(((u32)Val << (Lsb)) & Mask)
#define XAie_GetField(Val, Lsb, Mask)	(((u32)Val & Mask) >> Lsb)

#define XAieGbl_Read32		XAieLib_Read32
#define XAieGbl_Write32		XAieLib_Write32
#define XAieGbl_MaskWrite32	XAieLib_MaskWrite32
#define XAieGbl_WriteCmd	XAieLib_WriteCmd
#define XAieGbl_MaskPoll	XAieLib_MaskPoll

/************************** Variable Definitions *****************************/
/************************** Function Prototypes  *****************************/
#endif		/* end of protection macro */
/** @} */
