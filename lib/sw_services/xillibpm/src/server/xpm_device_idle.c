/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include "xpm_device_idle.h"
#include "xpm_common.h"

static XPmDevice_SoftResetInfo DeviceRstData[] = {
#ifdef XPAR_PSU_OSPI_0_DEVICE_ID
	{
		.DeviceId = XPM_DEVID_OSPI,
		.SoftRst = NULL,
		.IdleHook = NodeOspiIdle,
		.IdleHookArgs = XPAR_PSU_OSPI_0_DEVICE_ID,
	},
#endif
#ifdef XPAR_PSU_QSPI_0_DEVICE_ID
	{
		.DeviceId = XPM_DEVID_QSPI,
		.SoftRst = NULL,
		.IdleHook = NodeQspiIdle,
		.IdleHookArgs = XPAR_PSU_QSPI_0_DEVICE_ID,
	},
#endif
};

#if defined(XPAR_PSU_QSPI_0_DEVICE_ID)
/**
 * NodeQspiIdle() - Idle the QSPI node
 *
 * @DeviceId:	 Device ID of QSPI node
 * @BaseAddress: QSPI base address
 */
void NodeQspiIdle(u16 DeviceId, u32 BaseAddress)
{
	int Status;
	XQspiPsu_Config *ConfigPtr;
	XQspiPsu QspiInst;

	ConfigPtr = XQspiPsu_LookupConfig(DeviceId);
	if (NULL == ConfigPtr) {
		goto done;
	}

	Status = XQspiPsu_CfgInitialize(&QspiInst, ConfigPtr, BaseAddress);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XQspiPsu_Idle(&QspiInst);

done:
	return;
}
#endif

#if defined(XPAR_PSU_OSPI_0_DEVICE_ID)
/**
 * NodeQspiIdle() - Idle the OSPI node
 *
 * @DeviceId:	 Device ID of OSPI node
 * @BaseAddress: OSPI base address
 */
void NodeOspiIdle(u16 DeviceId, u32 BaseAddress)
{
	int Status;
	XOspiPsv_Config *ConfigPtr;
	XOspiPsv OspiInst;

	/* Warning Fix */
	(void)(BaseAddress)

	ConfigPtr = XOspiPsv_LookupConfig(DeviceId);
	if (NULL == ConfigPtr) {
		goto done;
	}

	Status = XOspiPsv_CfgInitialize(&OspiInst, ConfigPtr);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XOspiPsv_Idle(&OspiInst);

done:
	return;
}
#endif
