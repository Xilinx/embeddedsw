/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file pm_init.c
 * @brief PM subsystem bring-up (IPI + XilPM) for the example.
 */
#include "pm_api_sys.h"
#include "pm_init.h"
#include "xil_printf.h"

/**
 * @brief Initialize the IPI channel and the XilPM client library.
 *
 * @param IpiInst IPI instance to configure and hand to XilPM.
 *
 * @return XST_SUCCESS on success, XST_FAILURE on error.
 */
XStatus PmInit(XIpiPsu *const IpiInst)
{
	XStatus Status = XST_FAILURE;

	Status = IpiInit(IpiInst);
	if (XST_SUCCESS != Status) {
		xil_printf("IpiInit() failed with error: %d\r\n", Status);
		goto done;
	}

	Status = XPm_InitXilpm(IpiInst);
	if (XST_SUCCESS != Status) {
		xil_printf("XPm_InitXilpm() failed with error: %d\r\n", Status);
		goto done;
	}

	Status = XPm_InitFinalize();
	if (XST_SUCCESS != Status) {
		xil_printf("XPm_InitFinalize() failed with error: %d\r\n", Status);
		goto done;
	}

done:
	return Status;
}
