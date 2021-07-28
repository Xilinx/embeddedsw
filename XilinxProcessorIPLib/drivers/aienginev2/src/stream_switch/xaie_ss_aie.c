/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_ss_aie.c
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
#include "xaie_feature_config.h"
#include "xaie_helper.h"

#ifdef XAIE_FEATURE_SS_ENABLE
/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This api is used to verify  if a stream switch connection is possible with
* the provided slave and master ports. Within AIE, a full crossbar exists.
* So, every valid slave port and valid master port can connect.
*
* @param        Slave: The type of the slave port.
* @param        SlvPortNum: The number of the slave port.
* @param        Master: The type of the master port.
* @param        MstrPortNum: The number of the master port.
*
* @return       XAIE_OK if a stream switch connection is possible.
*
* @note         Internal API for AIE. This API shouldn't be called directly.
*               It is invoked using a function pointer within the Stream
*               Module data structure.
*
*****************************************************************************/
AieRC _XAie_StrmSwCheckPortValidity(StrmSwPortType Slave, u8 SlvPortNum,
		StrmSwPortType Master, u8 MstrPortNum)
{
	(void)Slave;
	(void)SlvPortNum;
	(void)Master;
	(void)MstrPortNum;
	return XAIE_OK;
}

#endif /* XAIE_FEATURE_SS_ENABLE */
