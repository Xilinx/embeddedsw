/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_ss_aieml.h
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
#ifndef XAIE_SS_AIEML_H
#define XAIE_SS_AIEML_H

/***************************** Include Files *********************************/
#include "xaie_helper.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes  *****************************/

AieRC _XAieMl_AieTile_StrmSwCheckPortValidity(StrmSwPortType Slave,
		u8 SlvPortNum, StrmSwPortType Master, u8 MstrPortNum);
AieRC _XAieMl_MemTile_StrmSwCheckPortValidity(StrmSwPortType Slave,
                u8 SlvPortNum, StrmSwPortType Master, u8 MstrPortNum);
AieRC _XAieMl_ShimTile_StrmSwCheckPortValidity(StrmSwPortType Slave,
                u8 SlvPortNum, StrmSwPortType Master, u8 MstrPortNum);

#endif /* XAIE_SS_AIEML_H */
/** @} */
