/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include <errno.h>
#include <reent.h>
#include "xil_types.h"

/************************** Function Prototypes ******************************/

sint32 * __errno (void);

/*****************************************************************************/
/**
* The errno variable is stored in the reentrancy structure.  This
* function returns its address for use by the macro errno defined in
* errno.h.
******************************************************************************/

sint32 *__errno (void)
{
  return &_REENT->_errno;
}
