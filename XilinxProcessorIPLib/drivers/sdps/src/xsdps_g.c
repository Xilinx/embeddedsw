/******************************************************************************
*
* Copyright (C) 2013 - 2019 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xsdps_g.c
* @addtogroup sdps_v3_8
* @{
*
* This file contains a configuration table that specifies the configuration of
* SD devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.00a hk/sg  10/17/13 Initial release
* 3.6   mn     07/06/18 Add initialization macros in sdps
*       mn     07/13/18 Add initializer macro for HasEMIO
*
* </pre>
*
******************************************************************************/



#include "xparameters.h"
#include "xsdps.h"

/*
* The configuration table for devices
*/

XSdPs_Config XSdPs_ConfigTable[] =
{
	{
		XPAR_XSDPS_0_DEVICE_ID,
		XPAR_XSDPS_0_BASEADDR,
		XPAR_XSDPS_0_SDIO_CLK_FREQ_HZ,
		XPAR_XSDPS_0_HAS_CD,
		XPAR_XSDPS_0_HAS_WP,
		XPAR_XSDPS_0_BUS_WIDTH,
		XPAR_XSDPS_0_MIO_BANK,
		XPAR_XSDPS_0_HAS_EMIO,
		XPAR_XSDPS_0_IS_CACHE_COHERENT
	}
};
/** @} */
