
/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_ss_aie.h
* @{
*
* This file contains internal api implementations for AIE stream switch.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who         Date        Changes
* ----- ---------   ----------  -----------------------------------------------
* 1.0   Siddharth   12/09/2020  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_SS_AIE_H
#define XAIE_SS_AIE_H

/***************************** Include Files *********************************/
#include "xaie_helper.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/

AieRC _XAie_StrmSwCheckPortValidity(StrmSwPortType Slave, u8 SlvPortNum,
		StrmSwPortType Master, u8 MstrPortNum);

#endif /* XAIE_SS_AIE_H */
/** @} */
