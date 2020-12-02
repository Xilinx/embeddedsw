/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpfw_mod_stl.h"

#ifdef ENABLE_STL
#include "xstl_topmb.h"

void ModStlInit(void)
{
	/* This STL function is implemented in STL source
	 * and this has Module creation, Setting up IPI handlers,etc
	 * in its definition.
	 */
	XStl_PmuStlInit();
}
#else /* ENABLE_STL */
void ModStlInit(void) { }
#endif /* ENABLE_STL */
