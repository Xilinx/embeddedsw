/******************************************************************************
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
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
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xilfpga.h
 * @addtogroup xfpga_apis XilFPGA APIs
 * @{
 *
 * The XILFPGA library provides the interface to the application to configure
 * the programmable logic (PL) though the PS.
 *
 * The below Features are supported in ZynqMP platform.
 * - Supported Features:
 *    - Full Bitstream loading.
 *    - Partial Bitstream loading.
 *    - Encrypted Bitstream loading.
 *    - Authenticated Bitstream loading.
 *    - Authenticated and Encrypted Bitstream loading.
 *    - Partial Bitstream loading.
 *
 * #  Xilfpga_PL library Interface modules	{#xilfpgapllib}
 *	Xilfpga_PL library uses the below major components to configure
 * the PL through PS.
 *
 *  - Xilsecure_library provides APIs to access secure hardware on the
 * ZynqMP devices. This library includes:
 *	 - SHA-3 engine hash functions
 *	 - AES for symmetric key encryption
 *	 - RSA for authentication
 *
 * These algorithms are needed to support to load the Encrypted and
 * Authenticated Bitstream into PL.
 *
 * @note XilFPGA library is capable of loading only .bin format files into PL On
 * ZynqMP platform.
 * The library does not support other file formats.
 *
 *
 * ##   Initialization & Writing Bitstream	{#xilinit}
 *
 * Use the u32 XFpga_PL_BitSream_Load(); function to initialize the PL
 * and load the Bitstream.
 *
 * @{
 * @cond xilfpga_internal
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 4.2   Nava  08/06/16 Refactor the xilfpga library to support
 *                      different PL programming Interfaces.
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef XILFPGA_H
#define XILFPGA_H
#include "xil_io.h"
#include "xil_types.h"
#include "xil_printf.h"
#include "xparameters.h"
#if defined(PLATFORM_ZYNQMP) || (PSU_PMU)
#include "xilfpga_pcap.h"
#include "xsecure.h"
#endif

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/
#define XFPGA_SUCCESS                   (0x0U)
#define XFPGA_FAILURE                   (0x1U)
#define XFPGA_VALIDATE_ERROR            (0x2U)
#define XFPGA_PRE_CONFIG_ERROR          (0x3U)
#define XFPGA_WRITE_BITSTREAM_ERROR     (0x4U)
#define XFPGA_POST_CONFIG_ERROR         (0x5U)
/** @endcond*/
/************************** Function Prototypes ******************************/
u32 XFpga_PL_BitStream_Load(UINTPTR BitstreamImageAddr,
			UINTPTR KeyAddr, u32 flags);
u32 XFpga_InterfaceStatus(void);
u32 XFpga_GetPlConfigReg(u32 ConfigReg, u32 *RegData);

#endif  /* XILFPGA_H */
