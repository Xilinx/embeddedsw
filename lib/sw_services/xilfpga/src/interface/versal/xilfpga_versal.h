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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 *
 *****************************************************************************/
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

#include "xilfpga.h"
#include "xil_util.h"

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
