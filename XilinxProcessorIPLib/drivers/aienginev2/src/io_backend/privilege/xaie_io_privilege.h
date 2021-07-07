/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_io_privilege.h
* @{
*
* This file contains the data structures and routines for low level IO
* operations that requires privilege permissions for multiple backends.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Wendy   05/17/2021 Initial creation.
* </pre>
*
******************************************************************************/
#ifndef XAIE_IO_PRIVILEGE_H
#define XAIE_IO_PRIVILEGE_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaie_io_privilege.h"

AieRC _XAie_PrivilegeInitPart(XAie_DevInst *DevInst, XAie_PartInitOpts *Opts);
AieRC _XAie_PrivilegeTeardownPart(XAie_DevInst *DevInst);
AieRC _XAie_PrivilegeRequestTiles(XAie_DevInst *DevInst,
		XAie_BackendTilesArray *Args);

#endif /* XAIE_IO_PRIVILEGE_H */

/** @} */
