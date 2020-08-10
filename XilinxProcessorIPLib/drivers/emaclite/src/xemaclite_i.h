/******************************************************************************
* Copyright (C) 2004 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
* @file xemaclite_i.h
* @addtogroup emaclite_v4_6
* @{
*
* This header file contains internal identifiers, which are those shared
* between the files of the driver. It is intended for internal use only.
*
* NOTES:
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.01a ecm  05/21/04 First release
* 1.11a mta  03/21/07 Updated to new coding style
* 1.13a sv   02/1/08  Added macros to Get/Set Tx/Rx status
* 3.00a ktn  10/22/09 The macros have been renamed to remove _m from the name.
*		      The macros changed in this file are
*		      XEmacLite_mGetTxActive changed to XEmacLite_GetTxActive,
*		      XEmacLite_mSetTxActive changed to XEmacLite_SetTxActive.
*
* </pre>
******************************************************************************/

#ifndef XEMACLITE_I_H		/* prevent circular inclusions */
#define XEMACLITE_I_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/

#include "xemaclite.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* Get the TX active location to check status. This is used to check if
* the TX buffer is currently active. There isn't any way in the hardware
* to implement this but the register is fully populated so the driver can
* set the bit in the send routine and the ISR can clear the bit when
* the handler is complete. This mimics the correct operation of the hardware
* if it was possible to do this in hardware.
*
* @param	BaseAddress is the base address of the device
*
* @return	Contents of active bit in register.
*
* @note		C-Style signature:
* 		u32 XEmacLite_GetTxActive(u32 BaseAddress)
*
*****************************************************************************/
#define XEmacLite_GetTxActive(BaseAddress)			\
		 (XEmacLite_ReadReg((BaseAddress), XEL_TSR_OFFSET))

/****************************************************************************/
/**
*
* Set the TX active location to update status. This is used to set the bit
* indicating which TX buffer is currently active. There isn't any way in the
* hardware to implement this but the register is fully populated so the driver
* can set the bit in the send routine and the ISR can clear the bit when
* the handler is complete. This mimics the correct operation of the hardware
* if it was possible to do this in hardware.
*
* @param	BaseAddress is the base address of the device
* @param	Mask is the data to be written
*
* @return	None
*
* @note		C-Style signature:
* 		void XEmacLite_SetTxActive(u32 BaseAddress, u32 Mask)
*
*****************************************************************************/
#define XEmacLite_SetTxActive(BaseAddress, Mask) 		\
	(XEmacLite_WriteReg((BaseAddress), XEL_TSR_OFFSET, (Mask)))

/************************** Variable Definitions ****************************/

extern XEmacLite_Config XEmacLite_ConfigTable[];

/************************** Function Prototypes ******************************/

void XEmacLite_AlignedWrite(void *SrcPtr, UINTPTR *DestPtr, unsigned ByteCount);
void XEmacLite_AlignedRead(UINTPTR *SrcPtr, void *DestPtr, unsigned ByteCount);

void StubHandler(void *CallBackRef);


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
