/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilfpga_pcap_common.h
 * @addtogroup xfpga_apis XilFPGA APIs
 * @{
 *
 *
 * The XILFPGA library provides the interface to the application to configure
 * the programmable logic (PL) though the PS.
 *
 * - Supported Features:
 *    - Full Bitstream loading.
 *    - Partial Bitstream loading.
 *    - Encrypted Bitstream loading.
 *    - Authenticated Bitstream loading.
 *    - Authenticated and Encrypted Bitstream loading.
 *    - Partial Bitstream loading.
 *
 * #  Xilfpga_PL library Interface modules     {#xilfpgapllib}
 *     Xilfpga_PL library uses the below major components to configure the PL
 *     through PS.
 *  - CSU DMA driver is used to transfer the actual Bit stream file for the
 *    PS to PL after PCAP initialization
 *
 *  - Xilsecure_library provides APIs to access secure hardware on the Zynq&reg
 *    UltraScale+&tm MPSoC devices. This library includes:
 *      - SHA-3 engine hash functions
 *      - AES for symmetric key encryption
 *      - RSA for authentication
 *
 * These algorithms are needed to support to load the Encrypted and
 * Authenticated Bitstreams into PL.
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
 * ---- ----  --------  --------------------------------------------------------
 * 5.2  Nava  02/14/20  Added Bitstream loading support by using IPI services.
 * 5.3  Nava  06/16/20  Modified the date format from dd/mm to mm/dd.
 * 6.0  Nava  12/14/20  In XFpga_PL_BitStream_Load() API the argument
 *                      AddrPtr_Size is being used for multiple purposes.
 *                      Use of the same variable for multiple purposes can
 *                      make it more difficult for a person to read (or)
 *                      understand the code and also it leads to a safety
 *                      violation. fixes this  issue by adding a separate
 *                      function arguments to read KeyAddr and
 *                      Size(Bitstream size).
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

#ifndef XILFPGA_PCAP_COMMON_H
#define XILFPGA_PCAP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**
 * Structure to store the PL Write Image details.
 *
 * @param BitstreamAddr	Bitstream image base address.
 * @param KeyAddr	Aes key address which is used for Decryption
 * @param Size		Used to store size of Bitstream Image.
 * @param Flags		Flags are used to specify the type of Bitstream file.
 *                      * BIT(0) - Bitstream type
 *                                     * 0 - Full Bitstream
 *                                     * 1 - Partial Bitstream
 *                      * BIT(1) - Authentication using DDR
 *                                     * 1 - Enable
 *                                     * 0 - Disable
 *                      * BIT(2) - Authentication using OCM
 *                                     * 1 - Enable
 *                                     * 0 - Disable
 *                      * BIT(3) - User-key Encryption
 *                                     * 1 - Enable
 *                                     * 0 - Disable
 *                      * BIT(4) - Device-key Encryption
 *                                     * 1 - Enable
 *                                     * 0 - Disable
 *
 */
typedef struct {
		UINTPTR BitstreamAddr;
		UINTPTR	KeyAddr;
		u32 Size;
		u32 Flags;
}XFpga_Write;

/**
 * Structure to store the PL Image details.
 *
 * @param ReadbackAddr	Address which is used to store the PL readback data.
 * @param ConfigReg		Configuration register value to be returned (or)
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

#endif  /* XILFPGA_PCAP_COMMON_H */
/** @} */
