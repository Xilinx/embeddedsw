
/*******************************************************************
*
* Copyright (C) 2010-2017 Xilinx, Inc. All rights reserved.
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

*******************************************************************************/
#include "xparameters.h"
#include "xv_sdirxss.h"

/*
* List of Sub-cores included in the Subsystem for Device ID 0
* Sub-core device id will be set by its driver in xparameters.h
*/

#define XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_PRESENT	1
#define XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_ABSOLUTE_BASEADDR	(XPAR_V_SMPTE_UHDSDI_RX_SS_BASEADDR + XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_S_AXI_CTRL_BASEADDR)


/*
* List of Sub-cores excluded from the subsystem for Device ID 0
*	- Excluded sub-core device id is set to 255
*	- Excluded sub-core baseaddr is set to 0
*/



XV_SdiRxSs_Config XV_SdiRxSs_ConfigTable[XPAR_XV_SDIRXSS_NUM_INSTANCES] =
{
	{
		XPAR_V_SMPTE_UHDSDI_RX_SS_DEVICE_ID,
		XPAR_V_SMPTE_UHDSDI_RX_SS_BASEADDR,
		(XVidC_PixelsPerClock) XPAR_V_SMPTE_UHDSDI_RX_SS_PIXELS_PER_CLOCK,
		XPAR_V_SMPTE_UHDSDI_RX_SS_LINE_RATE,

		{
			XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_PRESENT,
			XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_DEVICE_ID,
			XPAR_V_SMPTE_UHDSDI_RX_SS_V_SMPTE_UHDSDI_RX_ABSOLUTE_BASEADDR
		},
	}
};
