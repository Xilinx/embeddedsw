/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilfpga_versal.h
 * @addtogroup xfpga_apis XilFPGA APIs
 * @{
 *
 * The XILFPGA library for Versal provides the interface to the application to
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
 * @{
 * @cond xilfpga_internal
 * <pre>
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
 * </pre>
 *
 * @note
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
 * @param BitstreamAddr	Bitstream image base address for Normal PDI LOAD.
 *			Image Id for Deferred PDI LOAD.
 * @param KeyAddr	Unused.
 * @param Size		Unused.
 * @param Flags		Flags are used to specify the type of Bitstream file.
 *			* BIT(0) - Bitstream type
 *                                     * 0 - Normal PDI Load
 *                                     * 1 - Deferred PDI Load
 *
 */
typedef struct {
		UINTPTR BitstreamAddr;
		UINTPTR	KeyAddr;
		u32 Size;
		u32 Flags;
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
