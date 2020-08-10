/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xcsi_hw.h
* @addtogroup csi_v1_5
* @{
*
* Hardware register & masks definition file. It defines the register interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 06/15/15 Initial release
* 1.1 sss 08/17/16 Added 64 bit support
* 1.2 vsa 03/02/17 Add Word Count corruption interrupt bit support
* 1.5 vsa 08/10/20 Add YUV 420 8bit support
* </pre>
*
*****************************************************************************/

#ifndef XCSI_HW_H_
#define XCSI_HW_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files **********************************/

#include "xil_types.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/* Register offset definitions. Register accesses are 32-bit.
 */
/** @name Device registers
 *  Register sets of MIPI CSI2 Rx Core
 *  @{
 */

#define XCSI_CCR_OFFSET		0x00000000	/**< Core Configuration
						  *  Register */
#define XCSI_PCR_OFFSET		0x00000004	/**< Protocol Configuration
						  *  Register */
#define XCSI_CSR_OFFSET		0x00000010	/**< Core Status Register */
#define XCSI_GIER_OFFSET	0x00000020	/**< Global Interrupt
						  *  Register */
#define XCSI_ISR_OFFSET		0x00000024	/**< Interrupt Status
						  *  Register */
#define XCSI_IER_OFFSET		0x00000028	/**< Interrupt Enable
						  *  Register */
#define XCSI_SPKTR_OFFSET	0x00000030	/**< Generic Short Packet
						  *  Register */
#define XCSI_VCX_FE_OFFSET	0x00000034	/**< VCx Frame Error
						  *  Register */
#define XCSI_CLKINFR_OFFSET	0x0000003C	/**< Clock Lane Info
						  *  Register */

/** Lane Information Registers **/
#define XCSI_L0INFR_OFFSET	0x00000040	/**< Lane 0 Info Register */
#define XCSI_L1INFR_OFFSET	0x00000044	/**< Lane 1 Info Register */
#define XCSI_L2INFR_OFFSET	0x00000048	/**< Lane 2 Info Register */
#define XCSI_L3INFR_OFFSET	0x0000004C	/**< Lane 3 Info Register */

/** Image Information Register for each Virtual Channel **/
#define XCSI_VC0INF1R_OFFSET	0x00000060	/**< Virtual Channel 0 Image
						  *  Information 1 Register */
#define XCSI_VC0INF2R_OFFSET	0x00000064	/**< Virtual Channel 0 Image
						  *  Information 2 Register */
#define XCSI_VC1INF1R_OFFSET	0x00000068	/**< Virtual Channel 1 Image
						  *  Information 1 Register */
#define XCSI_VC1INF2R_OFFSET	0x0000006C	/**< Virtual Channel 1 Image
						  *  Information 2 Register */
#define XCSI_VC2INF1R_OFFSET	0x00000070	/**< Virtual Channel 2 Image
						  *  Information 1 Register */
#define XCSI_VC2INF2R_OFFSET	0x00000074	/**< Virtual Channel 2 Image
						  *  Information 2 Register */
#define XCSI_VC3INF1R_OFFSET	0x00000078	/**< Virtual Channel 3 Image
						  *  Information 1 Register */
#define XCSI_VC3INF2R_OFFSET	0x0000007C	/**< Virtual Channel 3 Image
						  *  Information 2 Register */
#define XCSI_VC4INF1R_OFFSET	0x00000080	/**< Virtual Channel 4 Image
						  *  Information 1 Register */
#define XCSI_VC4INF2R_OFFSET	0x00000084	/**< Virtual Channel 4 Image
						  *  Information 2 Register */
#define XCSI_VC5INF1R_OFFSET	0x00000088	/**< Virtual Channel 5 Image
						  *  Information 1 Register */
#define XCSI_VC5INF2R_OFFSET	0x0000008C	/**< Virtual Channel 5 Image
						  *  Information 2 Register */
#define XCSI_VC6INF1R_OFFSET	0x00000090	/**< Virtual Channel 6 Image
						  *  Information 1 Register */
#define XCSI_VC6INF2R_OFFSET	0x00000094	/**< Virtual Channel 6 Image
						  *  Information 2 Register */
#define XCSI_VC7INF1R_OFFSET	0x00000098	/**< Virtual Channel 7 Image
						  *  Information 1 Register */
#define XCSI_VC7INF2R_OFFSET	0x0000009C	/**< Virtual Channel 7 Image
						  *  Information 2 Register */
#define XCSI_VC8INF1R_OFFSET	0x000000A0	/**< Virtual Channel 8 Image
						  *  Information 1 Register */
#define XCSI_VC8INF2R_OFFSET	0x000000A4	/**< Virtual Channel 8 Image
						  *  Information 2 Register */
#define XCSI_VC9INF1R_OFFSET	0x000000A8	/**< Virtual Channel 9 Image
						  *  Information 1 Register */
#define XCSI_VC9INF2R_OFFSET	0x000000AC	/**< Virtual Channel 9 Image
						  *  Information 2 Register */
#define XCSI_VC10INF1R_OFFSET	0x000000B0	/**< Virtual Channel 10 Image
						  *  Information 1 Register */
#define XCSI_VC10INF2R_OFFSET	0x000000B4	/**< Virtual Channel 10 Image
						  *  Information 2 Register */
#define XCSI_VC11INF1R_OFFSET	0x000000B8	/**< Virtual Channel 11 Image
						  *  Information 1 Register */
#define XCSI_VC11INF2R_OFFSET	0x000000BC	/**< Virtual Channel 11 Image
						  *  Information 2 Register */
#define XCSI_VC12INF1R_OFFSET	0x000000C0	/**< Virtual Channel 12 Image
						  *  Information 1 Register */
#define XCSI_VC12INF2R_OFFSET	0x000000C4	/**< Virtual Channel 12 Image
						  *  Information 2 Register */
#define XCSI_VC13INF1R_OFFSET	0x000000C8	/**< Virtual Channel 13 Image
						  *  Information 1 Register */
#define XCSI_VC13INF2R_OFFSET	0x000000CC	/**< Virtual Channel 13 Image
						  *  Information 2 Register */
#define XCSI_VC14INF1R_OFFSET	0x000000D0	/**< Virtual Channel 14 Image
						  *  Information 1 Register */
#define XCSI_VC14INF2R_OFFSET	0x000000D4	/**< Virtual Channel 14 Image
						  *  Information 2 Register */
#define XCSI_VC15INF1R_OFFSET	0x000000D8	/**< Virtual Channel 15 Image
						  *  Information 1 Register */
#define XCSI_VC15INF2R_OFFSET	0x000000DC	/**< Virtual Channel 15 Image
						  *  Information 2 Register */
/*@}*/

/** @name Bitmasks and offsets of XCSI_CCR_OFFSET register
 *
 * This register is used for the enabling/disabling and resetting
 * the core of CSI2 Rx Controller
 * @{
 */

#define XCSI_CCR_SOFTRESET_MASK	0x00000002	/**< Soft Reset the core */
#define XCSI_CCR_COREENB_MASK	0x00000001	/**< Enable/Disable core */

#define XCSI_CCR_SOFTRESET_SHIFT	1	/**< Shift bits for
						  *  Soft reset */
#define XCSI_CCR_COREENB_SHIFT 		0	/**< Shift bits for
						  *  Core Enable*/
/*@}*/

/** @name Bitmasks and offset of XCSI_PCR_OFFSET register
 *
 * This register reports the number of lanes configured during core generation
 * and number of lanes actively used.
 * @{
 */
#define XCSI_PCR_MAXLANES_MASK	0x00000018	/**< Maximum lanes in core */
#define XCSI_PCR_ACTLANES_MASK	0x00000003	/**< Active  lanes in core */

#define XCSI_PCR_MAXLANES_SHIFT	3	/**< Shift bits for Max Lanes */
#define XCSI_PCR_ACTLANES_SHIFT	0	/**< Shift bits for Active Lanes */

/*@}*/

/** @name Bitmasks and offsets of XCSI_CSR_OFFSET register
 *
 * This register captures the core's status.
 * @{
 */
#define XCSI_CSR_PKTCOUNT_MASK	0xFFFF0000	/**< 16-bit Packet Counter */
#define XCSI_CSR_SPFIFOFULL_MASK	0x00000008	/**< Short Packet FIFO
							  *  Full */
#define XCSI_CSR_SPFIFONE_MASK	0x00000004	/**< Short Packet FIFO
						  *  Not Empty */
#define XCSI_CSR_SLBF_MASK	0x00000002	/**< Stream Line Buffer Full */
#define XCSI_CSR_RIPCD_MASK	0x00000001	/**< Reset in Progress OR
						  *  Core Disabled */

#define XCSI_CSR_PKTCOUNT_SHIFT		16	/**< Shift bits for
						  *  Packet Counter */
#define XCSI_CSR_SPFIFOFULL_SHIFT	3	/**< Shift bits for Short Packet
						  *  FIFO Full */
#define XCSI_CSR_SPFIFONE_SHIFT		2	/**< Shift bits for Short Packet
						  *   Not Empty */
#define XCSI_CSR_SLBF_SHIFT		1	/**< Shift bits for Stream Line
						  *  Buffer Full */
#define XCSI_CSR_RIPCD_SHIFT		0	/**< Bit Shift for
						  *  Reset in Progress */
/*@}*/


/** @name Bitmasks and offsets of XCSI_GIER_OFFSET register
 *
 * This register contains the global interrupt enable bit.
 * @{
 */
#define XCSI_GIER_GIE_MASK	0x00000001	/**< Global Interrupt
						  *  Enable bit */
#define XCSI_GIER_GIE_SHIFT	0	/**< Shift bits for Global Interrupt
					  *  Enable */

#define XCSI_GIER_SET	1	/**< Enable the Global Interrupts */
#define XCSI_GIER_RESET 0	/**< Disable the Global Interrupts */

/*@}*/


/** @name Bitmasks and offsets of XCSI_ISR_OFFSET register
 *
 * This register contains the interrupt status.
 * @{
 */
#define XCSI_ISR_FR_MASK	0x80000000	/**< Frame Received */
#define XCSI_ISR_VCXFE_MASK	0x40000000	/**< VCX Frame Error */
#define XCSI_ISR_SKEWCALCHS_MASK	0x20000000	/**< Skewcalhs Error */
#define XCSI_ISR_YUV420_MASK	0x10000000	/**< YUV420 Word count Error */
#define XCSI_ISR_WC_MASK	0x00400000	/**< Word count corruption */
#define XCSI_ISR_ILC_MASK	0x00200000	/**< Incorrect Lanes
						  *  Configured */
#define XCSI_ISR_SPFIFOF_MASK	0x00100000	/**< Short Packet FIFO FULL */
#define XCSI_ISR_SPFIFONE_MASK	0x00080000	/**< Short Packet FIFO
						  *  Not Empty */
#define XCSI_ISR_SLBF_MASK	0x00040000	/**< Stream Line Buffer
						  *  Full */
#define XCSI_ISR_STOP_MASK	0x00020000	/**< Detect Stop State */
#define XCSI_ISR_SOTERR_MASK	0x00002000	/**< SoT Error */
#define XCSI_ISR_SOTSYNCERR_MASK	0x00001000	/**< SoT Sync Error */
#define XCSI_ISR_ECC2BERR_MASK		0x00000800	/**< ECC 2 bit Error */
#define XCSI_ISR_ECC1BERR_MASK	0x00000400	/**< ECC 1 bit Error */
#define XCSI_ISR_CRCERR_MASK	0x00000200	/**< CRC Error */
#define XCSI_ISR_DATAIDERR_MASK	0x00000100	/**< Unknown data ID
						  * packet Error */
#define XCSI_ISR_VC3FSYNCERR_MASK	0x00000080	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 3 */
#define XCSI_ISR_VC3FLVLERR_MASK	0x00000040	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 3 */
#define XCSI_ISR_VC2FSYNCERR_MASK	0x00000020	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 2 */
#define XCSI_ISR_VC2FLVLERR_MASK	0x00000010	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 2 */
#define XCSI_ISR_VC1FSYNCERR_MASK	0x00000008	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 1 */
#define XCSI_ISR_VC1FLVLERR_MASK	0x00000004	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 1 */
#define XCSI_ISR_VC0FSYNCERR_MASK	0x00000002	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 0 */
#define XCSI_ISR_VC0FLVLERR_MASK	0x00000001	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 0 */

#define XCSI_ISR_ALLINTR_MASK		0xF07FFFFF	/**< All interrupts
							  *  mask */

#define XCSI_ISR_FR_SHIFT	31	/**< Shift bits for
					  *  Frame received interrupt */
#define XCSI_ISR_VCXFE_SHIFT	30	/**< Shift bits for
					  *  VCx Frame Error interrupt */
#define XCSI_ISR_SKEWCALCHS_SHIFT	29	/**< Shift bits for
					  *  skeecalchs Error interrupt */
#define XCSI_ISR_YUV420_SHIFT	28	/**< Shift bits for
					  *  YUV420 word count interrupt */
#define XCSI_ISR_WC_SHIFT	22	/**< Shift bits for Word Count
					  *  corruption */
#define XCSI_ISR_ILC_SHIFT	21	/**< Shift bits for Incorrect Lanes
					  *  configured */
#define XCSI_ISR_SPFIFOF_SHIFT	20	/**< Shift bits for Short Packet
					  *  FIFO Full */
#define XCSI_ISR_SPFIFONE_SHIFT	19	/**< Shift bits for Short Packet
					  *  FIFO Not Empty */
#define XCSI_ISR_SLBF_SHIFT	18	/**< Shift bits for Stream Line Buffer
					  *  Full */
#define XCSI_ISR_STOP_SHIFT	17	/**< Shift bits for Stop State */
#define XCSI_ISR_SOTERR_SHIFT	13	/**< Shift bits for Start of
					  *  Transmission Error */
#define XCSI_ISR_SOTSYNCERR_SHIFT	12	/**< Shift bits for Start of
						  *  Transmission Sync Error */
#define XCSI_ISR_ECC2BERR_SHIFT	11	/**< Shift bits for 2 bit ECC error */
#define XCSI_ISR_ECC1BERR_SHIFT	10	/**< Shift bits for 1 bit ECC error */
#define XCSI_ISR_CRCERR_SHIFT	9	/**< Shift bits for Packet CRC error */
#define XCSI_ISR_DATAIDERR_SHIFT	8	/**< Shift bits for Unsupported
						  *  Data ID error */
#define XCSI_ISR_VC3FSYNCERR_SHIFT	7	/**< Shift bits for Virtual
						  *  Channel 3 Frame
						  *  Synchronisation Error */
#define XCSI_ISR_VC3FLVLERR_SHIFT	6	/**< Shift bits for Virtual
						  *  Channel 3 Frame
						  *  Level Error */
#define XCSI_ISR_VC2FSYNCERR_SHIFT	5	/**< Shift bits for Virtual
						  *  Channel 2 Frame
						  *  Synchronisation Error */
#define XCSI_ISR_VC2FLVLERR_SHIFT	4	/**< Shift bits for Virtual
						  *  Channel 2 Frame
						  *  Level Error */
#define XCSI_ISR_VC1FSYNCERR_SHIFT	3	/**< Shift bits for Virtual
						  *  Channel 1 Frame
						  *  Synchronisation Error */
#define XCSI_ISR_VC1FLVLERR_SHIFT	2	/**< Shift bits for Virtual
						  *  Channel 1 Frame
						  *  Level Error */
#define XCSI_ISR_VC0FSYNCERR_SHIFT	1	/**< Shift bits for Virtual
						  *  Channel 0 Frame
						  *  Synchronisation Error */
#define XCSI_ISR_VC0FLVLERR_SHIFT	0	/**< Shift bits for Virtual
						  *  Channel 0 Frame
						  *  Level Error */
/*@}*/

/** @name BitMasks for grouped interrupts
 *
 * The interrupts are grouped into DPHY Level Errors, Protocol Decoding Errors,
 * Packet Level Errors, Normal Errors, Frame Received interrupt and
 * Short Packet related. These are used in XCsi_InterruptHandler() to
 * determine the particular callback
 * @{
 */
#define XCSI_INTR_PROT_MASK 	(XCSI_ISR_VC3FSYNCERR_MASK | \
				 XCSI_ISR_VC3FLVLERR_MASK | \
				 XCSI_ISR_VC2FSYNCERR_MASK | \
				 XCSI_ISR_VC2FLVLERR_MASK | \
				 XCSI_ISR_VC1FSYNCERR_MASK | \
				 XCSI_ISR_VC1FLVLERR_MASK | \
				 XCSI_ISR_VC0FSYNCERR_MASK | \
				 XCSI_ISR_VC0FLVLERR_MASK | \
				 XCSI_ISR_VCXFE_MASK)

#define XCSI_ISR_VC15FSYNCERR_MASK	0x00800000	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 15 */
#define XCSI_ISR_VC15FLVLERR_MASK	0x00400000	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 15 */
#define XCSI_ISR_VC14FSYNCERR_MASK	0x00200000	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 14 */
#define XCSI_ISR_VC14FLVLERR_MASK	0x00100000	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 14 */
#define XCSI_ISR_VC13FSYNCERR_MASK	0x00080000	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 13 */
#define XCSI_ISR_VC13FLVLERR_MASK	0x00040000	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 13 */
#define XCSI_ISR_VC12FSYNCERR_MASK	0x00020000	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 12 */
#define XCSI_ISR_VC12FLVLERR_MASK	0x00010000	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 12 */
#define XCSI_ISR_VC11FSYNCERR_MASK	0x00008000	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 11 */
#define XCSI_ISR_VC11FLVLERR_MASK	0x00004000	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 11 */
#define XCSI_ISR_VC10FSYNCERR_MASK	0x00002000	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 10 */
#define XCSI_ISR_VC10FLVLERR_MASK	0x00001000	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 10 */
#define XCSI_ISR_VC9FSYNCERR_MASK	0x00000800	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 9 */
#define XCSI_ISR_VC9FLVLERR_MASK	0x00000400	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 9 */
#define XCSI_ISR_VC8FSYNCERR_MASK	0x00000200	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 8 */
#define XCSI_ISR_VC8FLVLERR_MASK	0x00000100	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 8 */
#define XCSI_ISR_VC7FSYNCERR_MASK	0x00000080	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 7 */
#define XCSI_ISR_VC7FLVLERR_MASK	0x00000040	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 7 */
#define XCSI_ISR_VC6FSYNCERR_MASK	0x00000020	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 6 */
#define XCSI_ISR_VC6FLVLERR_MASK	0x00000010	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 6 */
#define XCSI_ISR_VC5FSYNCERR_MASK	0x00000008	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 5 */
#define XCSI_ISR_VC5FLVLERR_MASK	0x00000004	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 5 */
#define XCSI_ISR_VC4FSYNCERR_MASK	0x00000002	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 4 */
#define XCSI_ISR_VC4FLVLERR_MASK	0x00000001	/**< Frame Level Error
							  *  on Virtual
							  *  Channel 4 */

#define XCSI_INTR_VCFE_MASK 	(XCSI_ISR_VC4FSYNCERR_MASK | \
				 XCSI_ISR_VC4FLVLERR_MASK | \
				 XCSI_ISR_VC5FSYNCERR_MASK | \
				 XCSI_ISR_VC5FLVLERR_MASK | \
				 XCSI_ISR_VC6FSYNCERR_MASK | \
				 XCSI_ISR_VC6FLVLERR_MASK | \
				 XCSI_ISR_VC7FSYNCERR_MASK | \
				 XCSI_ISR_VC7FLVLERR_MASK | \
				 XCSI_ISR_VC8FSYNCERR_MASK | \
				 XCSI_ISR_VC8FLVLERR_MASK | \
				 XCSI_ISR_VC9FSYNCERR_MASK | \
				 XCSI_ISR_VC9FLVLERR_MASK | \
				 XCSI_ISR_VC10FSYNCERR_MASK | \
				 XCSI_ISR_VC10FLVLERR_MASK | \
				 XCSI_ISR_VC11FSYNCERR_MASK | \
				 XCSI_ISR_VC11FLVLERR_MASK | \
				 XCSI_ISR_VC12FSYNCERR_MASK | \
				 XCSI_ISR_VC12FLVLERR_MASK | \
				 XCSI_ISR_VC13FSYNCERR_MASK | \
				 XCSI_ISR_VC13FLVLERR_MASK | \
				 XCSI_ISR_VC14FSYNCERR_MASK | \
				 XCSI_ISR_VC14FLVLERR_MASK | \
				 XCSI_ISR_VC15FSYNCERR_MASK | \
				 XCSI_ISR_VC15FLVLERR_MASK)

#define XCSI_INTR_PKTLVL_MASK 	(XCSI_ISR_ECC2BERR_MASK |	\
				 XCSI_ISR_ECC1BERR_MASK |	\
				 XCSI_ISR_CRCERR_MASK   |	\
				 XCSI_ISR_DATAIDERR_MASK)

#define XCSI_INTR_DPHY_MASK 	(XCSI_ISR_SOTERR_MASK 	|	\
				 XCSI_ISR_SOTSYNCERR_MASK)

#define XCSI_INTR_SPKT_MASK 	(XCSI_ISR_SPFIFOF_MASK |	\
				 XCSI_ISR_SPFIFONE_MASK)

#define XCSI_INTR_FRAMERCVD_MASK	(XCSI_ISR_FR_MASK)

#define XCSI_INTR_VCXFE_MASK		(XCSI_ISR_VCXFE_MASK)

#define XCSI_INTR_ERR_MASK 	(XCSI_ISR_SKEWCALCHS_MASK |\
				 XCSI_ISR_YUV420_MASK	|\
				 XCSI_ISR_WC_MASK	|\
				 XCSI_ISR_ILC_MASK 	|\
				 XCSI_ISR_SLBF_MASK 	|\
				 XCSI_ISR_STOP_MASK)

/*@}*/

/** @name Bitmasks and offsets of XCSI_IER_OFFSET register
 *
 * This register contains the interrupt enable masks
 * @{
 */
#define XCSI_IER_FR_MASK	0x80000000	/**< Frame Received */
#define XCSI_IER_VCXFE_MASK	0x40000000	/**< VCX Frame Error */
#define XCSI_IER_SKEWCALHS_MASK	0x20000000	/**< Skewcalchs State */
#define XCSI_IER_YUV420_MASK	0x10000000	/**< YUV420 Word Count Error */
#define XCSI_IER_WC_MASK	0x00400000	/**< Word Count Corruption */
#define XCSI_IER_ILC_MASK	0x00200000	/**< Incorrect Lanes
						  *  Configured */
#define XCSI_IER_SPFIFOF_MASK	0x00100000	/**< Short Packet FIFO FULL */
#define XCSI_IER_SPFIFONE_MASK	0x00080000	/**< Short Packet FIFO
						   *  Not Empty */
#define XCSI_IER_SLBF_MASK	0x00040000	/**< Stream Line Buffer Full */
#define XCSI_IER_STOP_MASK	0x00020000	/**< Detect Stop State */
#define XCSI_IER_SOTERR_MASK	0x00002000	/**< SoT Error */
#define XCSI_IER_SOTSYNCERR_MASK	0x00001000	/**< SoT Sync Error */
#define XCSI_IER_ECC2BERR_MASK	0x00000800	/**< ECC 2 bit Error */
#define XCSI_IER_ECC1BERR_MASK	0x00000400	/**< ECC 1 bit Error */
#define XCSI_IER_CRCERR_MASK	0x00000200	/**< CRC Error */
#define XCSI_IER_DATAIDERR_MASK	0x00000100	/**< Unknown data ID packet
						  *  Error */
#define XCSI_IER_VC3FSYNCERR_MASK	0x00000080	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 3 */
#define XCSI_IER_VC3FLVLERR_MASK	0x00000040	/**< Frame Level Error on
							  *  Virtual
							  *  Channel 3 */
#define XCSI_IER_VC2FSYNCERR_MASK	0x00000020	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 2 */
#define XCSI_IER_VC2FLVLERR_MASK	0x00000010	/**< Frame Level Error on
							  *  Virtual
							  *  Channel 2 */
#define XCSI_IER_VC1FSYNCERR_MASK	0x00000008	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 1 */
#define XCSI_IER_VC1FLVLERR_MASK	0x00000004	/**< Frame Level Error on
							  *  Virtual
							  *  Channel 1 */
#define XCSI_IER_VC0FSYNCERR_MASK	0x00000002	/**< Frame Sync Error
							  *  on Virtual
							  *  Channel 0 */
#define XCSI_IER_VC0FLVLERR_MASK	0x00000001	/**< Frame Level Error on
							  *  Virtual
							  *  Channel 0 */

#define XCSI_IER_ALLINTR_MASK		0xF07FFFFF /**< All interrupts mask */

#define XCSI_IER_FR_SHIFT	31	/**< Shift bits for Frame received
					  *  interrupt*/
#define XCSI_IER_SKEWCALHS_SHIFT	29	/**< Shift bits for skewcalhs status */
#define XCSI_IER_YUV420_SHIFT	28	/**< Shift bits for YUV 420 Word Count error */
#define XCSI_IER_WC_SHIFT	22	/**< Shift bits for Word count
					 *  corruption */
#define XCSI_IER_ILC_SHIFT	21	/**< Shift bits for Incorrect Lanes
					  *  configured */
#define XCSI_IER_SPFIFOF_SHIFT	20	/**< Shift bits for Short Packet
					  *  FIFO Full */
#define XCSI_IER_SPFIFONE_SHIFT	19	/**< Shift bits for Short Packet
					  *  FIFO Not Empty */
#define XCSI_IER_SLBF_SHIFT	18	/**< Shift bits for Stream Line Buffer
					  *  Full */
#define XCSI_IER_STOP_SHIFT	17	/**< Shift bits for Stop State */
#define XCSI_IER_SOTERR_SHIFT	13	/**< Shift bits for Start of
					  *  Transmission Error */
#define XCSI_IER_SOTSYNCERR_SHIFT	12	/**< Shift bits for Start of
						  *  Transmission Sync Error */
#define XCSI_IER_ECC2BERR_SHIFT		11	/**< Shift bits for 2 bit
						  *  ECC error */
#define XCSI_IER_ECC1BERR_SHIFT		10	/**< Shift bits for 1 bit
						  *  ECC error */
#define XCSI_IER_CRCERR_SHIFT	9	/**< Shift bits for Packet CRC error */
#define XCSI_IER_DATAIDERR_SHIFT	8	/**< Shift bits for
						  *  Unsupported Data ID
						  *  error */
#define XCSI_IER_VC3FSYNCERR_SHIFT	7	/**< Shift bits for Virtual
						  *  Channel 3 Frame
						  *  Synchronisation Error */
#define XCSI_IER_VC3FLVLERR_SHIFT	6	/**< Shift bits for Virtual
						  *  Channel 3 Frame Level
						  *  Error */
#define XCSI_IER_VC2FSYNCERR_SHIFT	5	/**< Shift bits for Virtual
						  *  Channel 2 Frame
						  *  Synchronisation Error */
#define XCSI_IER_VC2FLVLERR_SHIFT	4	/**< Shift bits for Virtual
						  *  Channel 2 Frame Level
						  *  Error */
#define XCSI_IER_VC1FSYNCERR_SHIFT	3	/**< Shift bits for Virtual
						  *  Channel 1 Frame
						  *  Synchronisation Error */
#define XCSI_IER_VC1FLVLERR_SHIFT	2	/**< Shift bits for Virtual
						  *  Channel 1 Frame Level
						  *  Error */
#define XCSI_IER_VC0FSYNCERR_SHIFT	1	/**< Shift bits for Virtual
						  *  Channel 0 Frame
						  *  Synchronisation Error */
#define XCSI_IER_VC0FLVLERR_SHIFT	0	/**< Shift bits for Virtual
						  *  Channel 0 Frame Level
						  *  Error */
/*@}*/

/** @name Bitmasks and offsets of XCSI_SPKTR_OFFSET register
 *
 * This register contains the masks for getting the 16 bit data,
 * virtual channel and data type
 *
 * @{
 */
#define XCSI_SPKTR_DATA_MASK	0x00FFFF00	/**< 16 bit short packet
						  *  data Received */
#define XCSI_SPKTR_VC_MASK	0x000000C0	/**< Virtual channel number */
#define XCSI_SPKTR_DT_MASK	0x0000003F	/**< Data Type */


#define XCSI_SPKTR_DATA_SHIFT	8	/**< Shift bits for Short Packet Data */
#define XCSI_SPKTR_VC_SHIFT	6	/**< Shift bits for Short Packet
					  *  Virtual Channel Data */
#define XCSI_SPKTR_DT_SHIFT	0	/**< Bit Shift for Short Packet
					  *  Data Type */
/*@}*/

/** @name Bitmasks and offsets of XCSI_CLKINFR_OFFSET register
 *
 * This register contains the masks for getting current status of clock lane
 * @{
 */
#define XCSI_CLKINFR_STOP_MASK	0x00000002	/**< Stop State on clock lane */

#define XCSI_CLKINFR_STOP_SHIFT		1	/**< Shift bits for Clock Lane
						  *  Stop bit */
/*@}*/

/** @name Bitmasks and offsets of XCSI_L(0..3)INFR register
 *
 * This register contains the masks for getting current status of data lanes.
 * @{
 */

#define XCSI_LXINFR_STOP_MASK	0x00000020	/**< Stop State on clock lane */
#define XCSI_LXINFR_SKEWCALHS_MASK	0x00000004	/**< SkewcalHs State on clock lane */
#define XCSI_LXINFR_SOTERR_MASK		0x00000002	/**< Detection of
							  *  ErrSoTHS */
#define XCSI_LXINFR_SOTSYNCERR_MASK	0x00000001	/**< Detection of
							  *  ErrSoTSyncHS */

#define XCSI_LXINFR_STOP_SHIFT	5	/**< Bit Shift for Data Lane
					  *  Stop State */
#define XCSI_LXINFR_SKEWCALHS_SHIFT	2	/**< Bit Shift for Data Lane
					  *  SkewCalHs State */
#define XCSI_LXINFR_SOTERR_SHIFT	1	/**< Bit Shift for Start of
						  *  Transmission Error */
#define XCSI_LXINFR_SOTSYNCERR_SHIFT	0	/**< Bit Shift for Start of
						  *  Transmission Sync Error */
/*@}*/

/** @name Bitmasks and offsets of XCSI_VC(0..3)INF(1/2)R register
 *
 * This register contains the masks to get line count, byte count from
 * Info Reg 1 and to get current data type being processed from Info Reg 2
 * for each virtual channel.
 * @{
 */

#define XCSI_VCXINF1R_LINECOUNT_MASK	0xFFFF0000	/**< Number of long
							  *  packets
							  *  written into
							  *  line buffer */
#define XCSI_VCXINF1R_BYTECOUNT_MASK	0x0000FFFF	/**< Byte count of
							  *  current packet in
							  *  process */
#define XCSI_VCXINF1R_LINECOUNT_SHIFT	16	/**< Shift bits for Virtual
						  *  Channel Line Count
						  *  bits */
#define XCSI_VCXINF1R_BYTECOUNT_SHIFT	0	/**< Shift bits for Virtual
						  *  Channel Byte Count
						  *  bits */

#define XCSI_VCXINF2R_DATATYPE_MASK	0x0000003F	/**< Data type of
							  *  current
							  *  packet */
#define XCSI_VCXINF2R_DATATYPE_SHIFT	0	/**< Bit Shift for Virtual
						  * Channel Data Type */
/*@}*/

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/*****************************************************************************/
/**
* Static inline function to read data from CSI register space
*
* @param	BaseAddress is the base address of CSI
* @param	RegOffset is the register offset.
*
* @return	Value of the register.
*
* @note		None
*
******************************************************************************/
static inline u32 XCsi_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
{
	return Xil_In32(BaseAddress + (u32)RegOffset);
}

/*****************************************************************************/
/**
* Static inline function to write data to CSI register space
*
* @param	BaseAddress is the base address of CSI
* @param	RegOffset is the register offset.
* @param	Data is the value to be written to the register.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XCsi_WriteReg(UINTPTR BaseAddress, u32 RegOffset, u32 Data)
{
	Xil_Out32(BaseAddress + (u32)RegOffset, (u32)Data);
}

#ifdef __cplusplus
}
#endif
#endif /* end of protection macro */
/** @} */
