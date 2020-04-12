/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
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
* 1.5  Tejus   10/14/2019  Enable assertion for linux and simulation
* </pre>
*
******************************************************************************/
#ifndef XAIEGBL_DEFS_H
#define XAIEGBL_DEFS_H

/***************************** Include Files *********************************/
#include "xaielib.h"

/************************** Constant Definitions *****************************/
#define XAIE_SUCCESS			XAIELIB_SUCCESS
#define XAIE_FAILURE			XAIELIB_FAILURE
#define XAIE_COMPONENT_IS_READY		XAIELIB_COMPONENT_IS_READY

#define XAIE_NULL			(void *)0U
#define XAIE_ENABLE			1U
#define XAIE_DISABLE			0U
#define XAIE_RESETENABLE			1U
#define XAIE_RESETDISABLE		0U

#define XAIEGBL_CMDIO_COMMAND_SETSTACK	XAIELIB_CMDIO_COMMAND_SETSTACK
#define XAIEGBL_CMDIO_COMMAND_LOADSYM	XAIELIB_CMDIO_COMMAND_LOADSYM
#define XAIEGBL_TILE_BASE_ADDRESS        XAIELIB_TILE_BASE_ADDRESS

#define XAie_print			XAieLib_print
#define XAie_usleep			XAieLib_usleep
#define XAie_AssertNonvoid(Cond)	XAieLib_AssertNonvoid(Cond, __func__, __LINE__)
#define XAie_AssertVoid(Cond)		XAieLib_AssertVoid(Cond, __func__, __LINE__)

#define XAie_SetField(Val, Lsb, Mask)	(((u32)Val << Lsb) & Mask)
#define XAie_GetField(Val, Lsb, Mask)	(((u32)Val & Mask) >> Lsb)

#define XAieGbl_Read32                   XAieLib_Read32
#define XAieGbl_Read128                  XAieLib_Read128
#define XAieGbl_Write32                  XAieLib_Write32
#define XAieGbl_MaskWrite32              XAieLib_MaskWrite32
#define XAieGbl_Write128                 XAieLib_Write128
#define XAieGbl_WriteCmd                 XAieLib_WriteCmd
#define XAieGbl_MaskPoll                 XAieLib_MaskPoll
#define XAieGbl_LoadElf                  XAieLib_LoadElf
#define XAieGbl_LoadElfMem               XAieLib_LoadElfMem

#define XAieGbl_NPIRead32                XAieLib_NPIRead32
#define XAieGbl_NPIWrite32               XAieLib_NPIWrite32
#define XAieGbl_NPIMaskWrite32           XAieLib_NPIMaskWrite32
#define XAieGbl_NPIMaskPoll              XAieLib_NPIMaskPoll

#define XAieGbl_MemInst                  XAieLib_MemInst
#define XAieGbl_MemInit                  XAieLib_MemInit
#define XAieGbl_MemFinish                XAieLib_MemFinish
#define XAieGbl_MemAttach                XAieLib_MemAttach
#define XAieGbl_MemDetach                XAieLib_MemDetach
#define XAieGbl_MemAllocate              XAieLib_MemAllocate
#define XAieGbl_MemFree                  XAieLib_MemFree
#define XAieGbl_MemSyncForCPU            XAieLib_MemSyncForCPU
#define XAieGbl_MemSyncForDev            XAieLib_MemSyncForDev
#define XAieGbl_MemGetSize               XAieLib_MemGetSize
#define XAieGbl_MemGetVaddr              XAieLib_MemGetVaddr
#define XAieGbl_MemGetPaddr              XAieLib_MemGetPaddr
#define XAieGbl_MemRead32                XAieLib_MemRead32
#define XAieGbl_MemWrite32               XAieLib_MemWrite32

#define XAieGbl_IntrRegisterIsr          XAieLib_InterruptRegisterIsr
#define XAieGbl_IntrUnregisterIsr        XAieLib_InterruptUnregisterIsr

/************************** Variable Definitions *****************************/

/************************** Function Prototypes  *****************************/

#endif		/* end of protection macro */
/** @} */


