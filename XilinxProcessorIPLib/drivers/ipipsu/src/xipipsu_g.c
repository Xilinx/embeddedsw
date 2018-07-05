/******************************************************************************
*
* Copyright (C) 2015 - 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

#include "xparameters.h"
#include "xipipsu.h"

/*
* The configuration table for devices
*/

XIpiPsu_Config XIpiPsu_ConfigTable[] =
{

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
