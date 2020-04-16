/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
 * 5.2  Nava 05/12/19  Added Versal platform support.
 * 5.2  Nava 14/02/20  Removed unwanted header file inclusion.
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
 * @BitstreamAddr	Bitstream image base address for Normal PDI LOAD.
 *			Image Id for Deferred PDI LOAD.
 * @AddrPtr_Size	Unused
 * @Flags		Flags are used to specify the type of Bitstream file.
 *			* BIT(0) - Bitstream type
 *                                     * 0 - Normal PDI Load
 *                                     * 1 - Deferred PDI Load
 *
 */
typedef struct {
		UINTPTR BitstreamAddr;
		UINTPTR	AddrPtr_Size;
		u32 Flags;
}XFpga_Write;

/**
 * Structure to store the PL Image details.
 *
 * @ReadbackAddr	Address which is used to store the PL readback data.
 * @ConfigReg		Configuration register value to be returned (or)
 * 			The number of Fpga configuration frames to read
 */
typedef struct {
		UINTPTR ReadbackAddr;
		u32 ConfigReg_NumFrames;
}XFpga_Read;

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XILFPGA_VERSAL_H */
/** @} */
