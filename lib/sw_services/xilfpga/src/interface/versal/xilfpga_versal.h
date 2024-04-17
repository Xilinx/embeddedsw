/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilfpga_versal.h
 * @addtogroup xilfpga_versal XilFPGA APIs for Versal Adaptive SoCs
 * @{
 * @details
 *
 * <pre>
 * The XILFPGA library for AMD Versal Adaptive SoCs provides the interface to the application to
 * configure the programmable logic (PL) though the PS.
 *
 * - Supported Features:
 *    - Partial Bitstream loading.
 *
 * #  Xilfpga_PL library Interface modules	{#xilfpgapllib}
 *	Xilfpga_PL library uses the below major components to configure the PL
 *	through PS.
 *  -  xilmailbox library is used to transfer the actual Bit stream file from
 *     the PS to PL.
 *
 * @note XilFPGA library is capable of loading only .pdi format files into PL.
 * The library does not support other file formats.
 *
 *
 * ##   Initialization & Writing Bitstream	{#xilinit}
 *
 * Use the u32 XFpga_PL_BitSream_Load(); function to initialize the driver
 * and load the Bitstream.
 *
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ---- ---- --------  --------------------------------------------------------
 * 5.2  Nava 12/05/19  Added Versal platform support.
 * 5.2  Nava 02/14/20  Removed unwanted header file inclusion.
 * 5.3  Nava 06/16/20  Modified the date format from dd/mm to mm/dd.
 * 6.0  Nava 12/14/20  In XFpga_PL_BitStream_Load() API the argument
 *                     AddrPtr_Size is being used for multiple purposes.
 *                     Use of the same variable for multiple purposes can
 *                     make it more difficult for a person to read (or)
 *                     understand the code and also it leads to a safety
 *                     violation. fixes this  issue by adding a separate
 *                     function arguments to read KeyAddr and
 *                     Size(Bitstream size).
 * 6.0  Nava 05/17/21  Removed unused structure definition.
 * 6.3  Nava 08/05/22  Added doxygen tags.
 * 6.5  Nava 08/18/23  Resolved the doxygen issues.
 * 6.6  AC	 04/04/24  Resolved the doxygen issues.
 * </pre>
 *
 *
 ******************************************************************************/

#ifndef XILFPGA_VERSAL_H
#define XILFPGA_VERSAL_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**
 * Structure to store the PL Write Image details.
 *
 */
typedef struct {
		UINTPTR BitstreamAddr; /**< Bitstream image base address for Normal PDI LOAD */
		UINTPTR	KeyAddr; /**< Unused */
		u32 Size; /**< Unused */
		u32 Flags; /**< Specifies the type of bitstream file: BIT(0) - Bitstream type; 0 - Normal PDI Load , 1 - Deferred PDI Load */
}XFpga_Write;

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XILFPGA_VERSAL_H */
/** @} */
