/*
 * Copyright (c) 2016-2022 Xilinx, Inc. and Contributors. All rights reserved.
 * Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "metal-test.h"

int main(void)
{
	int status;

	status = metal_tests_run(NULL);

	return status;
}
