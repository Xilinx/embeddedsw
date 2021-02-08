/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/
#include "xpsmfw_stl.h"

#ifdef PSM_ENABLE_STL

/****************************************************************************/
/**
 * @brief	Hook function to run startup PSM STLs
 *
 * @return	XST_SUCCESS or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_StartUpStlHook(void)
{
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief	Hook function to run PSM STLs periodically
 *
 * @return	XST_SUCCESS or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_PeriodicStlHook(void)
{
	return XST_SUCCESS;
}
#endif /* PSM_ENABLE_STL */
