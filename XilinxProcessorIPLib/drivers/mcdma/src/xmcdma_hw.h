/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XMCDMA_HW_H_
/** @brief Include guard for the MCDMA hardware header. */
#define XMCDMA_HW_H_

/*****************************************************************************/
/**
*
* @file xmcdma_hw.h
* @addtogroup mcdma Overview
* @{
* @brief Defines hardware register offsets and masks for MCDMA.
*
* Hardware definition file for the MCDMA register interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   adk     18/07/17 Initial version.
* 1.2   mj      05/03/18 Defined XMCDMA_BD_SW_ID_OFFSET
* 1.2   rsp     08/17/18 Remove unused XMCDMA_BD_LEN_MASK
* </pre>
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Registers offsets
 * @{
 */
#define XMCDMA_CCR_OFFSET	0x00	/**< Common control register offset. */
#define XMCDMA_CSR_OFFSET	0x04	/**< Common status register offset. */
#define XMCDMA_CHEN_OFFSET	0x08	/**< Channel enable register offset. */
#define XMCDMA_CHSER_OFFSET	0x0C	/**< Channel service register offset. */
#define XMCDMA_ERR_OFFSET	0x10	/**< Error register offset. */
#define XMCDMA_CHOBS1_OFFSET	0x440	/**< Channel observer 1 register offset. */
#define XMCDMA_CHOBS2_OFFSET	0x444	/**< Channel observer 2 register offset. */
#define XMCDMA_CHOBS3_OFFSET	0x448	/**< Channel observer 3 register offset. */
#define XMCDMA_CHOBS4_OFFSET	0x44C	/**< Channel observer 4 register offset. */
#define XMCDMA_CHOBS5_OFFSET	0x450	/**< Channel observer 5 register offset. */
#define XMCDMA_CHOBS6_OFFSET	0x454	/**< Channel observer 6 register offset. */
#define XMCDMA_SGCACHE_OFFSET	0x4B0	/**< Scatter-gather cache register offset. */

/* MM2S Common Register offset */
#define XMCDMA_TXSCHD_TYPE_OFFSET 0x14	/**< Transmit scheduler type register offset. */
#define XMCDMA_TX_WRR_REG_OFFSET  0x18	/**< Transmit weighted round-robin register 0 offset. */
#define XMCDMA_TX_WRR_REG1_OFFSET 0x1C	/**< Transmit weighted round-robin register 1 offset. */
#define XMCDMA_TXCH_SERV_OFFSET	  0x20	/**< Transmit channel service register offset. */
#define XMCDMA_TXAXCACHE_OFFSET	  0x24	/**< Transmit AXI cache register offset. */
#define XMCDMA_TXINT_SER_OFFSET   0x28	/**< Transmit interrupt service register offset. */

/* S2MM Common Register offset */
#define XMCDMA_CPKTDROP_OFFSET	0x14	/**< Common packet drop register offset. */
#define XMCDMA_RXCH_SER_OFFSET	0x18	/**< Receive channel service register offset. */
#define XMCDMA_RXAXCACHE_OFFSET	0x1C	/**< Receive AXI cache register offset. */
#define XMCDMA_RXINT_SER_OFFSET	0x20	/**< Receive interrupt service register offset. */

/* Per Channel Register offset's */
#define XMCDMA_CR_OFFSET    		0x40	/**< Per-channel control register offset. */
#define XMCDMA_SR_OFFSET		0x44	/**< Per-channel status register offset. */
#define XMCDMA_CDESC_OFFSET		0x48	/**< Current descriptor pointer register offset. */
#define XMCDMA_CDESC_MSB_OFFSET		0x4C	/**< Current descriptor pointer MSB register offset. */
#define XMCDMA_TDESC_OFFSET		0x50	/**< Tail descriptor pointer register offset. */
#define XMCDMA_TDESC_MSB_OFFSET		0x54	/**< Tail descriptor pointer MSB register offset. */
#define XMCDMA_PKTDROP_OFFSET		0x58	/**< Packet drop count register offset. */
#define XMCDMA_TX_PKTCNT_STAT_OFFSET	0x58	/**< Transmit packet count status register offset. */
#define XMCDMA_RX_PKTCNT_STAT_OFFSET	0x5C	/**< Receive packet count status register offset. */

#define XMCDMA_NXTCHAN_OFFSET	0x40	/**< Next channel register block stride. */
#define XMCDMA_NXTOBS_OFFSET	0x4	/**< Next observer register stride. */
#define XMCDMA_RX_OFFSET 	0x500	/**< Receive path register base offset. */

#define XMCDMA_CCR_RUNSTOP_MASK		0x00000001	/**< Common control run/stop bit mask. */
#define XMCDMA_CCR_RESET_MASK		0x00000004	/**< Common control reset bit mask. */

#define XMCDMA_CSR_HALTED_MASK		0x00000001	/**< Common status halted bit mask. */
#define XMCDMA_CSR_IDLE_MASK		0x00000002	/**< Common status idle bit mask. */

#define XMCDMA_IRQ_PKTDROP_MASK		0x00000010	/**< Packet drop interrupt mask. */
#define XMCDMA_IRQ_IOC_MASK		0x00000020	/**< Interrupt on complete mask. */
#define XMCDMA_IRQ_DELAY_MASK		0x00000040	/**< Delay interrupt mask. */
#define XMCDMA_IRQ_ERROR_MASK		0x00000080	/**< Error interrupt mask. */
#define XMCDMA_IRQ_ALL_MASK		0x000000F0	/**< All interrupt sources mask. */

#define XMCDMA_DELAY_MASK		0xFF000000	/**< Interrupt delay counter field mask. */
#define XMCDMA_COALESCE_MASK		0x00FF0000	/**< Interrupt coalesce counter field mask. */
#define XMCDMA_PKTDROP_COALESCE_MASK	0x0000FF00	/**< Packet drop coalesce field mask. */

#define XMCDMA_COALESCE_SHIFT		16	/**< Bit shift for the coalesce counter field. */
#define XMCDMA_DELAY_SHIFT		24	/**< Bit shift for the delay counter field. */

#define XMCDMA_CHEN_MASK		0x000000FF	/**< Channel enable field mask. */

#define XMCDMA_CHID_MASK		0x000000FF	/**< Channel identifier field mask. */

#define XMCDMA_ERR_INTERNAL_MASK	0x00000001	/**< Internal error bit mask. */
#define XMCDMA_ERR_SLAVE_MASK		0x00000002	/**< Slave error bit mask. */
#define XMCDMA_ERR_DECODE_MASK		0x00000004	/**< Decode error bit mask. */
#define XMCDMA_ERR_SG_INT_MASK		0x00000010	/**< Scatter-gather internal error bit mask. */
#define XMCDMA_ERR_SG_SLV_MASK		0x00000020	/**< Scatter-gather slave error bit mask. */
#define XMCDMA_ERR_SG_DEC_MASK		0x00000040	/**< Scatter-gather decode error bit mask. */

#define XMCDMA_PKTDROP_CNT_MASK		0xFFFFFFFF	/**< Packet drop count field mask. */

#define XMCDMA_AXUSER_MASK		0x00000F00	/**< AXI user sideband field mask. */
#define XMCDMA_AXCACHE_MASK		0x0000000F	/**< AXI cache attribute field mask. */

#define XMCDMA_TX_WRRCH0_MASK		0x0000000F	/**< Transmit WRR weight mask for channel 0. */
#define XMCDMA_TX_WRRCH1_MASK		0x000000F0	/**< Transmit WRR weight mask for channel 1. */
#define XMCDMA_TX_WRRCH2_MASK		0x00000F00	/**< Transmit WRR weight mask for channel 2. */
#define XMCDMA_TX_WRRCH3_MASK		0x0000F000	/**< Transmit WRR weight mask for channel 3. */
#define XMCDMA_TX_WRRCH4_MASK		0x000F0000	/**< Transmit WRR weight mask for channel 4. */
#define XMCDMA_TX_WRRCH5_MASK		0x00F00000	/**< Transmit WRR weight mask for channel 5. */
#define XMCDMA_TX_WRRCH6_MASK		0x0F000000	/**< Transmit WRR weight mask for channel 6. */
#define XMCDMA_TX_WRRCH7_MASK		0xF0000000	/**< Transmit WRR weight mask for channel 7. */

/**
 * @brief Returns the WRR weight field mask for a transmit channel.
 *
 * @param[in] Chan_id is the zero-based WRR channel index within the register.
 */
#define XMCDMA_TX_WRRCH_MASK(Chan_id)	WRR_MASK((Chan_id * 4 + 3), Chan_id *4)
/**
 * @brief Returns the bit shift for a transmit channel WRR weight field.
 *
 * @param[in] Chan_id is the zero-based WRR channel index within the register.
 */
#define XMCDMA_TX_WRRCH_SHIFT(Chan_id)	Chan_id * 4

#define XMCDMA_SGAWCACHE_MASK		0x000F0000	/**< Scatter-gather AXI write cache field mask. */
#define XMCDMA_SGARCACHE_MASK		0x0000000F	/**< Scatter-gather AXI read cache field mask. */
#define XMCDMA_SGAWCACHE_SHIFT		16	/**< Bit shift for scatter-gather write cache field. */

/** @} */

/** @name Buffer Descriptor offsets
 *  @brief The first 13 words are used by hardware.
 *  All words after the 13rd word are for software use only.
 *  @{
 */
#define XMCDMA_BD_NDESC_OFFSET		0x00  /**< Next descriptor pointer. */
#define XMCDMA_BD_NDESC_MSB_OFFSET	0x04  /**< Next descriptor pointer. */
#define XMCDMA_BD_BUFA_OFFSET		0x08  /**< Buffer address. */
#define XMCDMA_BD_BUFA_MSB_OFFSET	0x0C  /**< Buffer address. */
#define XMCDMA_BD_RESERVED_OFFSET	0x10  /**< Reserved field. */
#define XMCDMA_BD_CTRL_OFFSET		0x14  /**< Control/buffer length. */
#define XMCDMA_BD_STS_OFFSET		0x18  /**< Status. */
#define XMCDMA_BD_SIDEBAND_STS_OFFSET	0x1C  /**< Side Band Status. */
#define XMCDMA_BD_USR0_OFFSET		0x20  /**< User IP specific word0. */
#define XMCDMA_BD_USR1_OFFSET		0x24  /**< User IP specific word1. */
#define XMCDMA_BD_USR2_OFFSET		0x28  /**< User IP specific word2. */
#define XMCDMA_BD_USR3_OFFSET		0x2C  /**< User IP specific word3. */
#define XMCDMA_BD_USR4_OFFSET		0x30  /**< User IP specific word3. */

#define XMCDMA_BD_HAS_DRE_OFFSET	0x34 /**< First unused field by h/w. */
#define XMCDMA_BD_HAS_CTRLSTS_OFFSET	0x38	/**< BD offset for status/control stream flag. */
#define XMCDMA_BD_SW_ID_OFFSET		0x3C  /**< Sw ID. */

/** @} */

#define XMCDMA_BD_CTRL_SBAND_OFFSET	0x18  /**< Status. */

#define XMCDMA_BD_CTRL_SOF_MASK		0x80000000	/**< Buffer descriptor start-of-frame control mask. */
#define XMCDMA_BD_CTRL_EOF_MASK		0x40000000	/**< Buffer descriptor end-of-frame control mask. */
#define XMCDMA_BD_CTRL_ALL_MASK		0xC0000000	/**< Buffer descriptor SOF and EOF control mask. */

#define XMCDMA_BD_CTRL_SBAND_SHIFT	24	/**< Bit shift for BD sideband control fields. */
#define XMCDMA_BD_STS_COMPLETE_MASK	0x80000000 /**< Completed. */
#define XMCDMA_BD_STS_DEC_ERR_MASK	0x40000000 /**< Decode error. */
#define XMCDMA_BD_STS_SLV_ERR_MASK	0x20000000 /**< Slave error. */
#define XMCDMA_BD_STS_INT_ERR_MASK	0x10000000 /**< Internal err. */
#define XMCDMA_BD_STS_ALL_ERR_MASK	0x70000000 /**< All errors. */
#define XMCDMA_BD_STS_RXSOF_MASK	0x08000000 /**< First rx pkt. */
#define XMCDMA_BD_STS_RXEOF_MASK	0x04000000 /**< Last rx pkt. */
#define XMCDMA_BD_STS_ALL_MASK		0xFC000000 /**< All status bits. */

#define XMCDMA_BD_SIDEBAND_STS_TID_MASK	   0xFF000000	/**< BD sideband status TID field mask. */
#define XMCDMA_BD_SIDEBAND_STS_TDEST_MASK  0x000F0000	/**< BD sideband status TDEST field mask. */
#define XMCDMA_BD_SIDEBAND_STS_TUSER_MASK  0x0000FFFF	/**< BD sideband status TUSER field mask. */

#define XMCDMA_MAX_TRANSFER_LEN		0x03FFFFFF	/**< Maximum buffer transfer length mask. */

#define XMCDMA_BD_HW_NUM_BYTES		52  /**< Number of bytes hw used. */

#define XMCDMA_BD_START_CLEAR		8   /**< Offset to start clear. */
#define XMCDMA_BD_BYTES_TO_CLEAR	44  /**< BD-specific bytes to clear. */
#define XMCDMA_BD_HAS_DRE_MASK		0xF00 /**< Whether has DRE mask. */
#define XMCDMA_BD_WORDLEN_MASK		0xFF  /**< Whether has DRE mask. */

#define XMCDMA_BD_HAS_DRE_SHIFT		8     /**< Whether has DRE shift. */
#define XMCDMA_BD_WORDLEN_SHIFT		0     /**< Whether has DRE shift. */

#define XMCDMA_LAST_APPWORD		4	/**< Last application word index in the BD. */
/***************** Macros (Inline Functions) Definitions *********************/

#define XMcdma_In32		Xil_In32	/**< Input operation. */
#define XMcdma_Out32	Xil_Out32	/**< Output operation. */

/*****************************************************************************/
/**
*
* @brief Reads the given register.
*
* @param[in]	BaseAddress is the base address of the MCDMA core.
* @param[in]	RegOffset is the register offset of the register.
*
* @return	The 32-bit value of the register.
*
* @note
*		C-style signature:
*		u32 XMcdma_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XMcdma_ReadReg(BaseAddress, RegOffset) \
	XMcdma_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* @brief Writes the given register.
*
* @param[in]	BaseAddress is the base address of the MCDMA core.
* @param[in]	RegOffset is the register offset of the register.
* @param[in]	Data is the 32-bit value to write to the register.
*
* @note
*		C-style signature:
*		void XMcdma_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XMcdma_WriteReg(BaseAddress, RegOffset, Data) \
	XMcdma_Out32(((BaseAddress) + (RegOffset)), (Data))

#ifdef __cplusplus
}

#endif
#endif /* XMCDMA_HW_H_ */
/** @} */
