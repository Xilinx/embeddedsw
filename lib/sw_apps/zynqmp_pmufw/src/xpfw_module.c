/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpfw_module.h"

XStatus XPfw_ModuleInit(XPfw_Module_t *ModPtr,u8 ModId)
{
	XStatus Status;

	if (ModPtr != NULL) {
		ModPtr->ModId = ModId;
		ModPtr->CfgInitHandler = NULL;
		ModPtr->EventHandler = NULL;
		ModPtr->IpiHandler = NULL;
		ModPtr->IpiId = 0U;
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}

	return Status;

}
