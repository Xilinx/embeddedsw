/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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
