/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaietile_lock.h
* @{
*
* Header file lock acquire/release functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/14/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* 1.2  Nishad  12/05/2018  Renamed ME attributes to AIE
* </pre>
*
******************************************************************************/
#ifndef XAIETILE_LOCK_H
#define XAIETILE_LOCK_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/
#define XAIETILE_LOCK_ACQ_SUCCESS		1U
#define XAIETILE_LOCK_ACQ_FAILED			0U
#define XAIETILE_LOCK_ACQ_VALINVALID		0xFFU
#define XAIETILE_LOCK_ACQ_VAL0			0U
#define XAIETILE_LOCK_ACQ_VAL1			1U

#define XAIETILE_LOCK_REL_SUCCESS		1U
#define XAIETILE_LOCK_REL_FAILED			0U
#define XAIETILE_LOCK_REL_VALINVALID		0xFFU
#define XAIETILE_LOCK_REL_VAL0			0U
#define XAIETILE_LOCK_REL_VAL1			1U

/**************************** Type Definitions *******************************/

/***************************** Macro Definitions *****************************/

/************************** Function Prototypes  *****************************/
u8 XAieTile_LockAcquire(XAieGbl_Tile *TileInstPtr, u8 LockId, u8 LockVal, u32 TimeOut);
u8 XAieTile_LockRelease(XAieGbl_Tile *TileInstPtr, u8 LockId, u8 LockVal, u32 TimeOut);

#endif            /* end of protection macro */
/** @} */
