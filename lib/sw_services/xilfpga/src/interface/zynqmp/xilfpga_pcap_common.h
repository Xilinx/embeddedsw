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
 * 5.2  Nava  14/02/20  Added Bitstream loading support by using IPI services.
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
 * @BitstreamAddr	Bitstream image base address.
 * @AddrPtr_Size        Aes key address which is used for Decryption (or)
 *                      In none Secure Bitstream used it is used store size
 *                      of Bitstream Image.
 * @Flags               Flags are used to specify the type of Bitstream file.
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
/* FPGA Configuration Registers Offsets */
#define CRC             0U  /* Status Register */
#define FAR1            1U  /* Frame Address Register */
#define FDRI            2U  /* FDRI Register */
#define FDRO            3U  /* FDRO Register */
#define CMD             4U  /* Command Register */
#define CTL0            5U  /* Control Register 0 */
#define MASK            6U  /* MASK Register */
#define STAT            7U  /* Status Register */
#define LOUT            8U  /* LOUT Register */
#define COR0            9U  /* Configuration Options Register 0 */
#define MFWR            10U /* MFWR Register */
#define CBC             11U /* CBC Register */
#define IDCODE          12U /* IDCODE Register */
#define AXSS            13U /* AXSS Register */
#define COR1            14U /* Configuration Options Register 1 */
#define WBSTAR          16U /* Warm Boot Start Address Register */
#define TIMER           17U /* Watchdog Timer Register */
#define BOOTSTS         22U /* Boot History Status Register */
#define CTL1            24U /* Control Register 1 */
/************************** Function Prototypes ******************************/
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XILFPGA_PCAP_COMMON_H */
/** @} */
