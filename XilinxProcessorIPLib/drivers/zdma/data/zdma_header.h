/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file zdma_header.h
 * @brief Declares the ZDMA example entry points.
 */

#ifndef ZDMA_HEADER_H		/* prevent circular inclusions */
/** @brief Include guard for the ZDMA example header. */
#define ZDMA_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
/*****************************************************************************/
/**
 * @brief Runs the ZDMA self-test example.
 *
 * @param[in] DeviceId is the unique device ID of the ZDMA.
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
int XZDma_SelfTestExample(u16 DeviceId);
#ifdef XPAR_SCUGIC_0_DEVICE_ID
/*****************************************************************************/
/**
 * @brief Runs the ZDMA simple-mode interrupt example.
 *
 * @param[in] IntcInstPtr is a pointer to the interrupt controller instance.
 * @param[in] ZdmaInstPtr is a pointer to the XZDma instance.
 * @param[in] DeviceId is the unique device ID of the ZDMA.
 * @param[in] IntrId is the interrupt ID of the ZDMA.
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
int XZDma_SimpleExample(XScuGic *IntcInstPtr, XZDma *ZdmaInstPtr,
			u16 DeviceId, u16 IntrId);
#else
#ifdef XPAR_INTC_0_DEVICE_ID
/*****************************************************************************/
/**
 * @brief Runs the ZDMA simple-mode interrupt example.
 *
 * @param[in] IntcInstPtr is a pointer to the interrupt controller instance.
 * @param[in] ZdmaInstPtr is a pointer to the XZDma instance.
 * @param[in] DeviceId is the unique device ID of the ZDMA.
 * @param[in] IntrId is the interrupt ID of the ZDMA.
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
int XZDma_SimpleExample(XIntc *IntcInstPtr, XZDma *ZdmaInstPtr,
			u16 DeviceId, u16 IntrId);
#endif
#endif
#else
/*****************************************************************************/
/**
 * @brief Runs the ZDMA self-test example.
 *
 * @param[in] BaseAddress is the base address of the ZDMA instance.
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
int XZDma_SelfTestExample(UINTPTR BaseAddress);
/*****************************************************************************/
/**
 * @brief Runs the ZDMA simple-mode interrupt example.
 *
 * @param[in] ZdmaInstPtr is a pointer to the XZDma instance.
 * @param[in] BaseAddress is the base address of the ZDMA instance.
 *
 * @return
 *		- XST_SUCCESS if the example completes successfully.
 *		- XST_FAILURE if the example fails.
 *
 *****************************************************************************/
int XZDma_SimpleExample(XZDma *ZdmaInstPtr,
			UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif

#endif
