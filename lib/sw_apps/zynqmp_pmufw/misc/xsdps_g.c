/******************************************************************************
 *
 * Copyright (C) 2010 - 2017 Xilinx, Inc.  All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 ******************************************************************************/

#include "xparameters.h"
#include "xsdps.h"

/*
* The configuration table for devices
*/

XSdPs_Config XSdPs_ConfigTable[XPAR_XSDPS_NUM_INSTANCES] =
{
	{
		XPAR_PSU_SD_1_DEVICE_ID,
		XPAR_PSU_SD_1_BASEADDR,
		XPAR_PSU_SD_1_SDIO_CLK_FREQ_HZ,
		XPAR_PSU_SD_1_HAS_CD,
		XPAR_PSU_SD_1_HAS_WP,
		XPAR_PSU_SD_1_BUS_WIDTH,
		XPAR_PSU_SD_1_MIO_BANK,
		XPAR_PSU_SD_1_HAS_EMIO,
		XPAR_PSU_SD_1_IS_CACHE_COHERENT
	}
};
