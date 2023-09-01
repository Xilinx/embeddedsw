/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xparameters.h"
#include "xipipsu.h"

/*
* The configuration table for devices
*/

XIpiPsu_Config XIpiPsu_ConfigTable[] = {

	{
		XPAR_XIPIPSU_0_DEVICE_ID,
		XPAR_XIPIPSU_0_BASE_ADDRESS,
		XPAR_XIPIPSU_0_BIT_MASK,
		XPAR_XIPIPSU_0_BUFFER_INDEX,
		XPAR_XIPIPSU_0_INT_ID,
		XPAR_XIPIPSU_NUM_TARGETS,
		{

			{
				XPAR_PSU_IPI_0_BIT_MASK,
				XPAR_PSU_IPI_0_BUFFER_INDEX
			},
			{
				XPAR_PSU_IPI_1_BIT_MASK,
				XPAR_PSU_IPI_1_BUFFER_INDEX
			},
			{
				XPAR_PSU_IPI_2_BIT_MASK,
				XPAR_PSU_IPI_2_BUFFER_INDEX
			},
			{
				XPAR_PSU_IPI_3_BIT_MASK,
				XPAR_PSU_IPI_3_BUFFER_INDEX
			},
			{
				XPAR_PSU_IPI_4_BIT_MASK,
				XPAR_PSU_IPI_4_BUFFER_INDEX
			},
			{
				XPAR_PSU_IPI_5_BIT_MASK,
				XPAR_PSU_IPI_5_BUFFER_INDEX
			},
			{
				XPAR_PSU_IPI_6_BIT_MASK,
				XPAR_PSU_IPI_6_BUFFER_INDEX
			}
		}
	}
};
