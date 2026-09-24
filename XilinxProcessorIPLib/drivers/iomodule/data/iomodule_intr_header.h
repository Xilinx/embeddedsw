/**
 * @file iomodule_intr_header.h
 * @brief Declarations for the IOModule interrupt example APIs.
 */
/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/**
 * @brief Run the IO Module interrupt example.
 *
 * @param[in] IOModuleInstancePtr is a pointer to the IO Module instance.
 * @param[in] DeviceId is the device ID of the IO Module instance.
 *
 * @return XST_SUCCESS if example ran successfully, XST_FAILURE otherwise.
 */
XStatus IOModuleIntrExample(XIOModule *IOModuleInstancePtr,
			    u16 DeviceId);
/**
 * @brief Set up the IO Module interrupt system.
 *
 * @param[in] IOModuleInstancePtr is a pointer to the IO Module instance.
 * @param[in] DeviceId is the device ID of the IO Module instance.
 *
 * @return XST_SUCCESS if setup succeeded, XST_FAILURE otherwise.
 */
XStatus IOModuleInterruptSetup(XIOModule *IOModuleInstancePtr,
                               u16 DeviceId);

#ifdef __cplusplus
}
#endif
