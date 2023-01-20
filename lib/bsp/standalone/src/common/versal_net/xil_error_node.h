/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xil_error_node.h
*
* This is the file which contains node IDs information for versal net error events.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who 	 Date        Changes
* ----- -------- -------- -----------------------------------------------------
* 8.0   bm       07/06/2022 Initial release
*       dc       07/12/2022 Added XIL_EVENT_ERROR_MASK_DEV_STATE_CHANGE
* 8.1   kal      01/05/2023 Added XIL_EVENT_ERROR_PCR_LOG_UPDATE
*       rama     01/19/2023 Add XilSem errors to SW error events
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XIL_ERROR_NODE_H
#define XIL_ERROR_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/
/**@name Versalnet Event Node IDs
 * @defgroup xileventnodes Event Node IDs
 * @ingroup xilnodeids
 * @{
 */

#define XIL_NODETYPE_EVENT_ERROR_ID_ENCODING	(0x28100000U)

 /**
 * Error Event Node Ids
 */
#define XIL_NODETYPE_EVENT_ERROR_PMC_ERR1	(0x28100000U)
#define XIL_NODETYPE_EVENT_ERROR_PMC_ERR2	(0x28104000U)
#define XIL_NODETYPE_EVENT_ERROR_PMC_ERR3	(0x28108000U)
#define XIL_NODETYPE_EVENT_ERROR_PSM_ERR1	(0x2810C000U)
#define XIL_NODETYPE_EVENT_ERROR_PSM_ERR2	(0x28110000U)
#define XIL_NODETYPE_EVENT_ERROR_PSM_ERR3	(0x28114000U)
#define XIL_NODETYPE_EVENT_ERROR_PSM_ERR4	(0x28118000U)
#define XIL_NODETYPE_EVENT_ERROR_SW_ERR		(0x2811C000U)
/**
 * @}
 */

/**@name Versalnet Error event Mask
 * @defgroup xilerroreventmask Error Event Mask
 * @ingroup xilnodeids
 * @{
 * @defgroup pmcerr1 Error Event Mask for PMC ERR1
 * @ingroup xilerroreventmask
 * @{
 * @brief Error Events belong to PMC ERR1 Node
 */

/** Error event mask for PMC Boot Correctable Error.
 * Set by ROM code during ROM execution during Boot. */
#define XIL_EVENT_ERROR_MASK_BOOT_CR		(0x00000001U)

/** Error event mask for PMC Boot Non-Correctable Error.
 * Set by ROM code during ROM execution during Boot. */
#define XIL_EVENT_ERROR_MASK_BOOT_NCR		(0x00000002U)

/** Error event mask for PMC Firmware Boot Correctable Error.
 * Set by PLM during firmware execution during Boot. */
#define XIL_EVENT_ERROR_MASK_FW_CR		(0x00000004U)

/** Error event mask for PMC Firmware Boot Non-Correctable Error.
 * Set by PLM during firmware execution during Boot. */
#define XIL_EVENT_ERROR_MASK_FW_NCR		(0x00000008U)

/** Error event mask for General Software Correctable Error.
 * Set by any processors after Boot. */
#define XIL_EVENT_ERROR_MASK_GSW_CR		(0x00000010U)

/** Error event mask for General Software Non-Correctable Error.
 * Set by any processors after Boot. */
#define XIL_EVENT_ERROR_MASK_GSW_NCR		(0x00000020U)

/** Error event mask for CFU Error. */
#define XIL_EVENT_ERROR_MASK_CFU		(0x00000040U)

/** Error event mask for CFRAME Error. */
#define XIL_EVENT_ERROR_MASK_CFRAME		(0x00000080U)

/** Error event mask for PSM Correctable Error,
 * Summary from PSM Error Management. */
#define XIL_EVENT_ERROR_MASK_PMC_PSM_CR		(0x00000100U)

/** Error event mask for PSM Non-Correctable Error,
 * Summary from PSM Error Management. */
#define XIL_EVENT_ERROR_MASK_PMC_PSM_NCR	(0x00000200U)

/** Error event mask for DDRMC MB Correctable ECC Error. */
#define XIL_EVENT_ERROR_MASK_DDRMB_CR		(0x00000400U)

/** Error event mask for DDRMC MB Non-Correctable ECC Error. */
#define XIL_EVENT_ERROR_MASK_DDRMB_NCR		(0x00000800U)

/** Error event mask for NoC Type1 Correctable Error. */
#define XIL_EVENT_ERROR_MASK_NOCTYPE1_CR	(0x00001000U)

/** Error event mask for NoC Type1 Non-Correctable Error. */
#define XIL_EVENT_ERROR_MASK_NOCTYPE1_NCR	(0x00002000U)

/** Error event mask for NoC User Error. */
#define XIL_EVENT_ERROR_MASK_NOCUSER		(0x00004000U)

/** Error event mask for MMCM Lock Error. */
#define XIL_EVENT_ERROR_MASK_MMCM		(0x00008000U)

/** Error event mask for ME Correctable Error. */
#define XIL_EVENT_ERROR_MASK_AIE_CR		(0x00010000U)

/** Error event mask for ME Non-Correctable Error. */
#define XIL_EVENT_ERROR_MASK_AIE_NCR		(0x00020000U)

/** Error event mask for DDRMC MC Correctable ECC Error. */
#define XIL_EVENT_ERROR_MASK_DDRMC_CR		(0x00040000U)

/** Error event mask for DDRMC MC Non-Correctable ECC Error. */
#define XIL_EVENT_ERROR_MASK_DDRMC_NCR		(0x00080000U)

/** Error event mask for GT Correctable Error. */
#define XIL_EVENT_ERROR_MASK_GT_CR		(0x00100000U)

/** Error event mask for GT Non-Correctable Error. */
#define XIL_EVENT_ERROR_MASK_GT_NCR		(0x00200000U)

/** Error event mask for PL Sysmon Correctable Error. */
#define XIL_EVENT_ERROR_MASK_PLSMON_CR		(0x00400000U)

/** Error event mask for PL Sysmon Non-Correctable Error. */
#define XIL_EVENT_ERROR_MASK_PLSMON_NCR		(0x00800000U)

/** Error event mask for User defined PL generic error. */
#define XIL_EVENT_ERROR_MASK_PL0		(0x01000000U)

/** Error event mask for User defined PL generic error. */
#define XIL_EVENT_ERROR_MASK_PL1		(0x02000000U)

/** Error event mask for User defined PL generic error. */
#define XIL_EVENT_ERROR_MASK_PL2		(0x04000000U)

/** Error event mask for User defined PL generic error. */
#define XIL_EVENT_ERROR_MASK_PL3		(0x08000000U)

/** Error event mask for NPI Root Error. */
#define XIL_EVENT_ERROR_MASK_NPIROOT		(0x10000000U)

/** Error event mask for SSIT Error from Slave SLR1,
 * Only used in Master SLR. */
#define XIL_EVENT_ERROR_MASK_SSIT3		(0x20000000U)

/** Error event mask for SSIT Error from Slave SLR2,
 * Only used in Master SLR. */
#define XIL_EVENT_ERROR_MASK_SSIT4		(0x40000000U)

/** Error event mask for SSIT Error from Slave SLR3,
 * Only used in Master SLR. */
#define XIL_EVENT_ERROR_MASK_SSIT5		(0x80000000U)
/**
 * @}
 */

/**
 * @defgroup pmcerr2 Error Event Mask for PMC ERR2
 * @ingroup xilpmerroreventmask
 * @{
 * @brief Error Events belong to PMC ERR2 Node
 */
/** Error event mask for General purpose PMC error,
 * can be triggered by any of the following peripherals:,
 * - PMC Global Regsiters,- PMC Clock & Reset (CRP),- PMC IOU Secure SLCR,
 * - PMC IOU SLCR,- BBRAM Controller,- PMC Analog Control Registers,
 * - RTC Control Registers. */
#define XIL_EVENT_ERROR_MASK_PMCAPB		(0x00000001U)

/** Error event mask for PMC ROM Validation Error. */
#define XIL_EVENT_ERROR_MASK_PMCROM		(0x00000002U)

/** Error event mask for PMC PPU0 MB TMR Fatal Error. */
#define XIL_EVENT_ERROR_MASK_MB_FATAL0		(0x00000004U)

/** Error event mask for PMC PPU1 MB TMR Fatal Error. */
#define XIL_EVENT_ERROR_MASK_MB_FATAL1		(0x00000008U)

/** Error event mask for PMC Correctable Errors:,
 * PPU0 RAM correctable error.,PPU1 instruction RAM correctable error.,
 * PPU1 data RAM correctable error. */
#define XIL_EVENT_ERROR_MASK_PMC_CR		(0x00000020U)

/** Error event mask for PMC Non-Correctable Errors:,
 * PPU0 RAM non-correctable error.,PPU1 instruction RAM non-correctable error.,
 * PPU1 data RAM non-correctable error.,PRAM non-correctable error. */
#define XIL_EVENT_ERROR_MASK_PMC_NCR		(0x00000040U)

/** Error event mask for PMC Temperature Shutdown Alert and Power Supply
 * Failure Detection Errors from PMC Sysmon alarm[0].
 * Indicates an alarm condition on any of SUPPLY0 to SUPPLY31. */
#define XIL_EVENT_ERROR_MASK_PMCSMON0		(0x00000080U)

/** Error event mask for PMC Temperature Shutdown Alert and Power Supply
 * Failure Detection Errors from PMC Sysmon alarm[1].
 * Indicates an alarm condition on any of SUPPLY32 to SUPPLY63. */
#define XIL_EVENT_ERROR_MASK_PMCSMON1		(0x00000100U)

/** Error event mask for PMC Temperature Shutdown Alert and Power Supply
 * Failure Detection Errors from PMC Sysmon alarm[2].
 * Indicates an alarm condition on any of SUPPLY64 to SUPPLY95. */
#define XIL_EVENT_ERROR_MASK_PMCSMON2		(0x00000200U)

/** Error event mask for PMC Temperature Shutdown Alert and Power Supply
 * Failure Detection Errors from PMC Sysmon alarm[3].
 * Indicates an alarm condition on any of SUPPLY96 to SUPPLY127. */
#define XIL_EVENT_ERROR_MASK_PMCSMON3		(0x00000400U)

/** Error event mask for PMC Temperature Shutdown Alert and Power Supply
 * Failure Detection Errors from PMC Sysmon alarm[4].
 * Indicates an alarm condition on any of SUPPLY128 to SUPPLY159. */
#define XIL_EVENT_ERROR_MASK_PMCSMON4		(0x00000800U)

/** Error event mask for PMC Temperature Shutdown Alert and Power Supply
 * Failure Detection Errors from PMC Sysmon alarm[8].
 * Indicates an over-temperature alarm. */
#define XIL_EVENT_ERROR_MASK_PMCSMON8		(0x00008000U)

/** Error event mask for PMC Temperature Shutdown Alert and Power Supply
 * Failure Detection Errors from PMC Sysmon alarm[9].
 * Indicates a device temperature alarm. */
#define XIL_EVENT_ERROR_MASK_PMCSMON9		(0x00010000U)

/** Error event mask for CFI Non-Correctable Error. */
#define XIL_EVENT_ERROR_MASK_CFI		(0x00020000U)

/** Error event mask for CFRAME SEU CRC Error. */
#define XIL_EVENT_ERROR_MASK_SEUCRC		(0x00040000U)

/** Error event mask for CFRAME SEU ECC Error. */
#define XIL_EVENT_ERROR_MASK_SEUECC		(0x00080000U)

/** Error event mask for PMX WDT Error. */
#define XIL_EVENT_ERROR_MASK_PMX_WWDT		(0x00100000U)

/** Error event mask for RTC Alarm Error. */
#define XIL_EVENT_ERROR_MASK_RTCALARM		(0x00400000U)

/** Error event mask for PMC NPLL Lock Error,
 * This error can be unmasked after the NPLL is locked to alert when the
 * NPLL loses lock. */
#define XIL_EVENT_ERROR_MASK_NPLL		(0x00800000U)

/** Error event mask for PMC PPLL Lock Error,
 * This error can be unmasked after the PPLL is locked to alert when the
 * PPLL loses lock. */
#define XIL_EVENT_ERROR_MASK_PPLL		(0x01000000U)

/** Error event mask for Clock Monitor Errors.,
 * Collected from CRP's CLKMON_STATUS register. */
#define XIL_EVENT_ERROR_MASK_CLKMON		(0x02000000U)

/** Error event mask for PMC interconnect correctable error. */
#define XIL_EVENT_ERROR_MASK_INT_PMX_CORR_ERR		(0x08000000U)

/** Error event mask for PMC interconnect uncorrectable error. */
#define XIL_EVENT_ERROR_MASK_INT_PMX_UNCORR_ERR		(0x10000000U)

/** Error event mask for For Master SLR: SSIT Error from Slave SLR1.,
 * For Slave SLRs: SSIT Error0 from Master SLR. */
#define XIL_EVENT_ERROR_MASK_SSIT0		(0x20000000U)

/** Error event mask for For Master SLR: SSIT Error from Slave SLR2.,
 * For Slave SLRs: SSIT Error1 from Master SLR. */
#define XIL_EVENT_ERROR_MASK_SSIT1		(0x40000000U)

/** Error event mask for For Master SLR: SSIT Error from Slave SLR3.,
 * For Slave SLRs: SSIT Error2 from Master SLR. */
#define XIL_EVENT_ERROR_MASK_SSIT2		(0x80000000U)
/**
 * @}
 */

/**
 * @defgroup pmcerr3 Error Event Mask for PMC ERR3
 * @ingroup xilpmerroreventmask
 * @{
 * @brief Error Events belong to PMC ERR3 Node
 */
/** Error event mask for PMC IOU correctable error. */
#define XIL_EVENT_ERROR_MASK_IOU_CR		(0x00000001U)

/** Error event mask for PMC IOU uncorrectable error. */
#define XIL_EVENT_ERROR_MASK_IOU_NCR		(0x00000002U)

/** Error event mask for DFX unexpected activation. */
#define XIL_EVENT_ERROR_MASK_DFX_UXPT_ACT		(0x00000004U)

/** Error event mask for DICE CDI SEED parity. */
#define XIL_EVENT_ERROR_MASK_DICE_CDI_PAR		(0x00000008U)

/** Error event mask for Device identity private key parity. */
#define XIL_EVENT_ERROR_MASK_DEVIK_PRIV		(0x00000010U)

/** Error event mask for Next SW CDI SEED parity. */
#define XIL_EVENT_ERROR_MASK_NXTSW_CDI_PAR		(0x00000020U)

/** Error event mask for Device attestation private key parity. */
#define XIL_EVENT_ERROR_MASK_DEVAK_PRIV		(0x00000040U)

/** Error event mask for DME public key X component's parity. */
#define XIL_EVENT_ERROR_MASK_DME_PUB_X		(0x00000080U)

/** Error event mask for DME public key Y component's parity. */
#define XIL_EVENT_ERROR_MASK_DME_PUB_Y		(0x00000100U)

/** Error event mask for DEVAK public key X component's parity. */
#define XIL_EVENT_ERROR_MASK_DEVAK_PUB_X		(0x00000200U)

/** Error event mask for DEVAK public key Y component's parity. */
#define XIL_EVENT_ERROR_MASK_DEVAK_PUB_Y		(0x00000400U)

/** Error event mask for DEVIK public key X component's parity. */
#define XIL_EVENT_ERROR_MASK_DEVIK_PUB_X		(0x00000800U)

/** Error event mask for DEVIK public key Y component's parity. */
#define XIL_EVENT_ERROR_MASK_DEVIK_PUB_Y		(0x00001000U)

/** Error event mask for PCR parity. */
#define XIL_EVENT_ERROR_MASK_PCR_PAR		(0x00002000U)
/**
 * @}
 */

/**
 * @defgroup psmerr1 Error Event Mask for PSM ERR1
 * @ingroup xilpmerroreventmask
 * @{
 * @brief Error Events belong to PSM ERR1 Node
 */
/** Error event mask for PS Software can write to trigger register to
 * generate this Correctable Error. */
#define XIL_EVENT_ERROR_MASK_PS_SW_CR		(0x00000001U)

/** Error event mask for PS Software can write to trigger register to
 * generate this Non-Correctable Error. */
#define XIL_EVENT_ERROR_MASK_PS_SW_NCR		(0x00000002U)

/** Error event mask for PSM Firmware can write to trigger register to
 * generate this Correctable Error. */
#define XIL_EVENT_ERROR_MASK_PSM_B_CR		(0x00000004U)

/** Error event mask for PSM Firmware can write to trigger register to
 * generate this Non-Correctable Error. */
#define XIL_EVENT_ERROR_MASK_PSM_B_NCR		(0x00000008U)

/** Error event mask for Or of MB Fatal1, Fatal2, Fatal3 Error. */
#define XIL_EVENT_ERROR_MASK_MB_FATAL		(0x00000010U)

/** Error event mask for PSM Correctable. */
#define XIL_EVENT_ERROR_MASK_PSM_CR		(0x00000020U)

/** Error event mask for PSM Non-Correctable. */
#define XIL_EVENT_ERROR_MASK_PSM_NCR		(0x00000040U)

/** Error event mask for PSMX CHK error. */
#define XIL_EVENT_ERROR_MASK_PSMX_CHK		(0x00000080U)

/** Error event mask for APLL1 lock error. The error can be unmasked
 * after the PLL is locked to alert when the PLL loses lock. */
#define XIL_EVENT_ERROR_MASK_APLL1_LOCK		(0x00000100U)

/** Error event mask for APLL2 lock error. The error can be unmasked
 * after the PLL is locked to alert when the PLL loses lock. */
#define XIL_EVENT_ERROR_MASK_APLL2_LOCK		(0x00000200U)

/** Error event mask for RPLL Lock Errors. The error can be unmasked
 * after the PLL is locked to alert when the PLL loses lock. */
#define XIL_EVENT_ERROR_MASK_RPLL_LOCK		(0x00000400U)

/** Error event mask for FLXPLL Lock Errors. The error can be unmasked
 * after the PLL is locked to alert when the PLL loses lock. */
#define XIL_EVENT_ERROR_MASK_FLXPLL_LOCK		(0x00000800U)

/** Error event mask for INT_PSM correctable error. */
#define XIL_EVENT_ERROR_MASK_INT_PSM_CR		(0x00001000U)

/** Error event mask for INT_PSM non-correctable error. */
#define XIL_EVENT_ERROR_MASK_INT_PSM_NCR		(0x00002000U)

/** Error event mask for Consolidated Error from the two USB2 blocks. */
#define XIL_EVENT_ERROR_MASK_USB2		(0x00004000U)

/** Error event mask for LPX unexpected dfx activation error. */
#define XIL_EVENT_ERROR_MASK_LPX_UXPT_ACT	(0x00008000U)

/** Error event mask for INT_LPD correctable error. */
#define XIL_EVENT_ERROR_MASK_INT_LPD_CR		(0x00020000U)

/** Error event mask for INT_LPD non-correctable error. */
#define XIL_EVENT_ERROR_MASK_INT_LPD_NCR		(0x00040000U)

/** Error event mask for INT_OCM correctable error. */
#define XIL_EVENT_ERROR_MASK_INT_OCM_CR		(0x00080000U)

/** Error event mask for INT_OCM non-correctable error. */
#define XIL_EVENT_ERROR_MASK_INT_OCM_NCR		(0x00100000U)

/** Error event mask for INT_FPD correctable error. */
#define XIL_EVENT_ERROR_MASK_INT_FPD_CR		(0x00200000U)

/** Error event mask for INT_FPD non-correctable error. */
#define XIL_EVENT_ERROR_MASK_INT_FPD_NCR		(0x00400000U)

/** Error event mask for INT_IOU correctable Error. */
#define XIL_EVENT_ERROR_MASK_INT_IOU_CR		(0x00800000U)

/** Error event mask for INT_IOU non-correctable Error. */
#define XIL_EVENT_ERROR_MASK_INT_IOU_NCR		(0x01000000U)

/** Error event mask for RPU lockstep error for ClusterA. */
#define XIL_EVENT_ERROR_MASK_RPUA_LOCKSTEP	(0x02000000U)

/** Error event mask for RPU lockstep error for ClusterB. */
#define XIL_EVENT_ERROR_MASK_RPUB_LOCKSTEP	(0x04000000U)

/** Error event mask for APU GIC AXI error (slverr, decerr). */
#define XIL_EVENT_ERROR_MASK_APU_GIC_AXI		(0x08000000U)

/** Error event mask for APU GIC ECC error. */
#define XIL_EVENT_ERROR_MASK_APU_GIC_ECC		(0x10000000U)

/** Error event mask for CPM correctable error. */
#define XIL_EVENT_ERROR_MASK_CPM_CR		(0x20000000U)

/** Error event mask for CPM non-correctable error. */
#define XIL_EVENT_ERROR_MASK_CPM_NCR		(0x40000000U)

/** Error event mask for CPI error. */
#define XIL_EVENT_ERROR_MASK_CPI			(0x80000000U)
/**
 * @}
 */


/**
 * @defgroup psmerr2 Error Event Mask for PSM ERR2
 * @ingroup xilpmerroreventmask
 * @brief Error Events belong to PSM ERR2 Node
 * @{
 */
/** Error event mask for FPD WDT0 error. */
#define XIL_EVENT_ERROR_MASK_FPD_WDT0		(0x00000001U)

/** Error event mask for FPD WDT1 error. */
#define XIL_EVENT_ERROR_MASK_FPD_WDT1		(0x00000002U)

/** Error event mask for FPD WDT2 error. */
#define XIL_EVENT_ERROR_MASK_FPD_WDT2		(0x00000004U)

/** Error event mask for FPD WDT3 error. */
#define XIL_EVENT_ERROR_MASK_FPD_WDT3		(0x00000008U)

/** Error event mask for Memory Errors for Splitter0. */
#define XIL_EVENT_ERROR_MASK_MEM_SPLITTER0	(0x00000010U)

/** Error event mask for Consolidated Errror indicating AXI parity Error for Splitter0. */
#define XIL_EVENT_ERROR_MASK_AXI_PAR_SPLITTER0	(0x00000020U)

/** Error event mask for Memory Errors for Splitter1. */
#define XIL_EVENT_ERROR_MASK_MEM_SPLITTER1	(0x00000040U)

/** Error event mask for Consolidated Errror indicating AXI parity Error for Splitter1. */
#define XIL_EVENT_ERROR_MASK_AXI_PAR_SPLITTER1	(0x00000080U)

/** Error event mask for Memory Errors for Splitter2. */
#define XIL_EVENT_ERROR_MASK_MEM_SPLITTER2	(0x00000100U)

/** Error event mask for Consolidated Errror indicating AXI parity Error for Splitter2. */
#define XIL_EVENT_ERROR_MASK_AXI_PAR_SPLITTER2	(0x00000200U)

/** Error event mask for Memory Errors for Splitter3. */
#define XIL_EVENT_ERROR_MASK_MEM_SPLITTER3    	(0x00000400U)

/** Error event mask for Consolidated Errror indicating AXI parity Error for Splitter3. */
#define XIL_EVENT_ERROR_MASK_AXI_PAR_SPLITTER3	(0x00000800U)

/** Error event mask for APU Cluster 0 error. */
#define XIL_EVENT_ERROR_MASK_APU_CLUSTER0 	(0x00001000U)

/** Error event mask for APU Cluster 1 error. */
#define XIL_EVENT_ERROR_MASK_APU_CLUSTER1	(0x00002000U)

/** Error event mask for APU Cluster 2 error. */
#define XIL_EVENT_ERROR_MASK_APU_CLUSTER2	(0x00004000U)

/** Error event mask for APU Cluster 3 error. */
#define XIL_EVENT_ERROR_MASK_APU_CLUSTER3	(0x00008000U)

/** Error event mask for WWDT0 LPX Error. */
#define XIL_EVENT_ERROR_MASK_LPD_WWDT0		(0x00010000U)

/** Error event mask for WWDT0 LPX Error. */
#define XIL_EVENT_ERROR_MASK_LPD_WWDT1		(0x00020000U)

/** Error event mask for ADMA Lockstep Error. */
#define XIL_EVENT_ERROR_MASK_ADMA_LOCKSTEP	(0x00040000U)

/** Error event mask for IPI Error */
#define XIL_EVENT_ERROR_MASK_IPI			(0x00080000U)

/** Error event mask for OCM Bank0 Corr Error. */
#define XIL_EVENT_ERROR_MASK_OCM_BANK0_CR	(0x00100000U)

/** Error event mask for OCM Bank1 Corr Error. */
#define XIL_EVENT_ERROR_MASK_OCM_BANK1_CR	(0x00200000U)

/** Error event mask for OCM Bank0 UnCorr Error. */
#define XIL_EVENT_ERROR_MASK_OCM_BANK0_NCR	(0x00400000U)

/** Error event mask for OCM Bank1 UnCorr Error. */
#define XIL_EVENT_ERROR_MASK_OCM_BANK1_NCR	(0x00800000U)

/** Error event mask for LPXAFIFS Corr Error. */
#define XIL_EVENT_ERROR_MASK_LPXAFIFS_CR		(0x01000000U)

/** Error event mask for LPXAFIFS UnCorr Error. */
#define XIL_EVENT_ERROR_MASK_LPXAFIFS_NCR	(0x02000000U)

/** Error event mask for LPX Glitch Detector0 glitch detected. */
#define XIL_EVENT_ERROR_MASK_LPX_GLITCH_DETECT0	(0x04000000U)

/** Error event mask for LPX Glitch Detector1 glitch detected. */
#define XIL_EVENT_ERROR_MASK_LPX_GLITCH_DETECT1	(0x08000000U)

/** Error event mask for Firewall write errors from NOC NMUs. */
#define XIL_EVENT_ERROR_MASK_FWALL_WR_NOC_NMU	(0x10000000U)

/** Error event mask for Firewall read error from NOC NMU. */
#define XIL_EVENT_ERROR_MASK_FWALL_RD_NOC_NMU	(0x20000000U)

/** Error event mask for Firewall error from NOC NSU. */
#define XIL_EVENT_ERROR_MASK_FWALL_NOC_NSU	(0x40000000U)

/** Error event mask for Bit[18] from R52 Core A0, Err event. */
#define XIL_EVENT_ERROR_MASK_B18_R52_A0		(0x80000000U)
/**
 * @}
 */

/**
 * @defgroup psmerr3 Error Event Mask for PSM ERR3
 * @ingroup xilpmerroreventmask
 * @brief Error Events belong to PSM ERR3 Node
 * @{
 */
/** Error event mask for Bit[18] from R52 Core A1, Err event. */
#define XIL_EVENT_ERROR_MASK_B18_R52_A1		(0x00000001U)

/** Error event mask for Bit[18] from R52 Core B0, Err event. */
#define XIL_EVENT_ERROR_MASK_B18_R52_B0		(0x00000002U)

/** Error event mask for Bit[18] from R52 Core B1, Err event. */
#define XIL_EVENT_ERROR_MASK_B18_R52_B1		(0x00000004U)

/** Error event mask for R52 A0 Core Correctable Error. */
#define XIL_EVENT_ERROR_MASK_R52_A0_CR		(0x00000008U)

/** Error event mask for R52 A0 Core TFatal Error. */
#define XIL_EVENT_ERROR_MASK_R52_A0_TFATAL	(0x00000010U)

/** Error event mask for R52 A0 Core Timeout Error. */
#define XIL_EVENT_ERROR_MASK_R52_A0_TIMEOUT	(0x00000020U)

/** Error event mask for Bit[24:20] pf ERREVNT for RPUA0. */
#define XIL_EVENT_ERROR_MASK_B24_B20_RPUA0	(0x00000040U)

/** Error event mask for Bit[25] of ERREVNT for RPUA0. */
#define XIL_EVENT_ERROR_MASK_B25_RPUA0		(0x00000080U)

/** Error event mask for R52 A1 Core Correctable Error. */
#define XIL_EVENT_ERROR_MASK_R52_A1_CR		(0x00000100U)

/** Error event mask for R52 A1 Core TFatal Error. */
#define XIL_EVENT_ERROR_MASK_R52_A1_TFATAL	(0x00000200U)

/** Error event mask for R52 A1 Core Timeout Error. */
#define XIL_EVENT_ERROR_MASK_R52_A1_TIMEOUT    	(0x00000400U)

/** Error event mask for Bit[24:20] pf ERREVNT for RPUA1. */
#define XIL_EVENT_ERROR_MASK_B24_B20_RPUA1	(0x00000800U)

/** Error event mask for Bit[25] of ERREVNT for RPUA1. */
#define XIL_EVENT_ERROR_MASK_B25_RPUA1		(0x00001000U)

/** Error event mask for R52 A1 Core Correctable Error. */
#define XIL_EVENT_ERROR_MASK_R52_B0_CR		(0x00002000U)

/** Error event mask for R52 A1 Core TFatal Error. */
#define XIL_EVENT_ERROR_MASK_R52_B0_TFATAL	(0x00004000U)

/** Error event mask for R52 A1 Core Timeout Error. */
#define XIL_EVENT_ERROR_MASK_R52_B0_TIMEOUT    	(0x00008000U)

/** Error event mask for Bit[24:20] pf ERREVNT for RPUB0. */
#define XIL_EVENT_ERROR_MASK_B24_B20_RPUB0	(0x00010000U)

/** Error event mask for Bit[25] of ERREVNT for RPUB0. */
#define XIL_EVENT_ERROR_MASK_B25_RPUB0		(0x00020000U)

/** Error event mask for R52 A1 Core Correctable Error. */
#define XIL_EVENT_ERROR_MASK_R52_B1_CR		(0x00040000U)

/** Error event mask for R52 A1 Core TFatal Error. */
#define XIL_EVENT_ERROR_MASK_R52_B1_TFATAL	(0x00080000U)

/** Error event mask for R52 A1 Core Timeout Error. */
#define XIL_EVENT_ERROR_MASK_R52_B1_TIMEOUT    	(0x00100000U)

/** Error event mask for Bit[24:20] pf ERREVNT for RPUB1. */
#define XIL_EVENT_ERROR_MASK_B24_B20_RPUB1	(0x00200000U)

/** Error event mask for Bit[25] of ERREVNT for RPUB1.*/
#define XIL_EVENT_ERROR_MASK_B25_RPUB1		(0x00400000U)

/** Error event mask for PCIL ERR FOR RPU Clusters. */
#define XIL_EVENT_ERROR_MASK_PCIL_RPU		(0x01000000U)

/** Error event mask for FPXAFIFS Corr Error. */
#define XIL_EVENT_ERROR_MASK_FPXAFIFS_CR		(0x02000000U)

/** Error event mask for FPXAFIFS UnCorr Error. */
#define XIL_EVENT_ERROR_MASK_FPXAFIFS_NCR	(0x04000000U)

/** Error event mask for PSX_CMN_1 PD block consolidated ERROR. */
#define XIL_EVENT_ERROR_MASK_PSX_CMN_1		(0x08000000U)

/** Error event mask for PSX_CMN_2 PD block consolidated ERROR. */
#define XIL_EVENT_ERROR_MASK_PSX_CMN_2		(0x10000000U)

/** Error event mask for PSX_CMN_3 PD block consolidated ERROR. */
#define XIL_EVENT_ERROR_MASK_PSX_CMN_3		(0x20000000U)

/** Error event mask for PSX_CML PD block consolidated ERROR. */
#define XIL_EVENT_ERROR_MASK_PSX_CML		(0x40000000U)

/** Error event mask for FPD_INT_WRAP PD block consolidated ERROR. */
#define XIL_EVENT_ERROR_MASK_FPD_INT_WRAP	(0x80000000U)
/**
 * @}
 */

/**
 * @defgroup psmerr4 Error Event Mask for PSM ERR4
 * @ingroup xilpmerroreventmask
 * @brief Error Events belong to PSM ERR4 Node
 * @{
 */
/** Error event mask for FPD Reset Monitor ERROR. */
#define XIL_EVENT_ERROR_MASK_FPD_RST_MON		(0x00000001U)

/** Error event mask for LPD reset and Clock Monitor Error. */
#define XIL_EVENT_ERROR_MASK_LPD_RST_CLK_MON	(0x00000002U)

/** Error event mask for Fatal Error from all AFI FM. */
#define XIL_EVENT_ERROR_MASK_FATAL_AFI_FM	    (0x00000004U)

/** Error event mask for Non Fatal Error from AFI FM in LPX. */
#define XIL_EVENT_ERROR_MASK_NFATAL_AFI_FM_LPX      (0x00000008U)

/** Error event mask for Non Fatal Error from AFI FM0 in FPX. */
#define XIL_EVENT_ERROR_MASK_NFATAL_AFI_FM0_FPX	    (0x00000010U)

/** Error event mask for Non Fatal Error from AFI FM1 in FPX. */
#define XIL_EVENT_ERROR_MASK_NFATAL_AFI_FM1_FPX	    (0x00000020U)

/** Error event mask for Non Fatal Error from AFI FM2 in FPX. */
#define XIL_EVENT_ERROR_MASK_NFATAL_AFI_FM2_FPX	    (0x00000040U)

/** Error event mask for Non Fatal Error from AFI FM3 in FPX. */
#define XIL_EVENT_ERROR_MASK_NFATAL_AFI_FM3_FPX	    (0x00000080U)

/** Error event mask for Errors from RPU cluster A. */
#define XIL_EVENT_ERROR_MASK_RPU_CLUSTERA	    (0x00000100U)

/** Error event mask for Errors from RPU cluster B. */
#define XIL_EVENT_ERROR_MASK_RPU_CLUSTERB	    (0x00000200U)
/**
 * @}
 */

/**
 * @defgroup swerr Error Event Mask for Software error events
 * @ingroup xilpmerroreventmask
 * @{
 * @brief Error Events belong to SW ERR Node
 */
/** Error event mask for Software error events */
/** Health Boot Monitoring errors */
#define XIL_EVENT_ERROR_MASK_HB_MON_0		(0x00000001U)
#define XIL_EVENT_ERROR_MASK_HB_MON_1		(0x00000002U)
#define XIL_EVENT_ERROR_MASK_HB_MON_2		(0x00000004U)
#define XIL_EVENT_ERROR_MASK_HB_MON_3		(0x00000008U)
#define XIL_EVENT_ERROR_MASK_PLM_EXCEPTION	(0x00000010U)
#define XIL_EVENT_ERROR_MASK_DEV_STATE_CHANGE 	(0x00000020U)
#define XIL_EVENT_ERROR_PCR_LOG_UPDATE		(0x00000040U)
/** XilSem errors */
#define XIL_EVENT_ERROR_MASK_XSEM_CRAM_CE	(0x00000080U)
#define XIL_EVENT_ERROR_MASK_XSEM_CRAM_UE	(0x00000100U)
#define XIL_EVENT_ERROR_MASK_XSEM_NPI_UE	(0x00000200U)
/**
 * @}
 */

/**
 * @}
 */
#ifdef __cplusplus
}
#endif

#endif /* XIL_ERROR_NODE_H */
