/**
 * @file iomodule_header.h
 * @brief Declarations for the IOModule example APIs.
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

#ifndef SDT
/**
 * @brief Run the IO Module self-test example.
 *
 * @param[in] DeviceId is the device ID of the IO Module instance.
 *
 * @return XST_SUCCESS if the self-test passed, XST_FAILURE otherwise.
 */
XStatus IOModuleSelfTestExample(u16 DeviceId);
#else
XStatus IOModuleSelfTestExample(u32 DeviceId);
#endif

#ifdef SDT
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
#endif

#ifdef __cplusplus
}
#endif
