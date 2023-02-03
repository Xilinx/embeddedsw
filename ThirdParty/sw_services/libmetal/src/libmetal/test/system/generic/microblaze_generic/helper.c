/*
 * Copyright (c) 2017-2022 Xilinx, Inc. and Contributors. All rights reserved.
 * Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"

/* Main hw machinery initialization entry point, called from main()*/
/* return 0 on success */
int init_system(void)
{
	init_platform();
	return 0;
}
