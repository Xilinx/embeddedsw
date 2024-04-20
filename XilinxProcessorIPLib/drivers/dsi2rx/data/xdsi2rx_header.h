/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XDSI_HEADER_H		/* prevent circular inclusions */
#define XDSI_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/***************************** Include Files *********************************/


/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 DsiSelfTestExample(u32 DeviceId);

#endif
