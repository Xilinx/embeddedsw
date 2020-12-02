/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/* The errno variable is stored in the reentrancy structure.  This
   function returns its address for use by the macro errno defined in
   errno.h.  */

#include <errno.h>
#include <reent.h>
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
	__attribute__((weak)) sint32 * __errno (void);
}
#endif

__attribute__((weak)) sint32 *
__errno (void)
{
  return &_REENT->_errno;
}
