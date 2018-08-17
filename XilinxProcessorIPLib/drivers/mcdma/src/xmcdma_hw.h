/******************************************************************************
*
* Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   adk 	18/07/17 Initial version.
* 1.2   mj      05/03/18 Defined XMCDMA_BD_SW_ID_OFFSET
* 1.2   rsp     08/17/18 Remove unused XMCDMA_BD_LEN_MASK
******************************************************************************/
#ifndef XMCDMA_HW_H_
#define XMCDMA_HW_H_		/**< Prevent circular inclusions
				  *  by using protection macros	*/
#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Registers offsets
 * @{
 */
#define XMCDMA_CCR_OFFSET	0x00
#define XMCDMA_CSR_OFFSET	0x04
#define XMCDMA_CHEN_OFFSET	0x08
#define XMCDMA_CHSER_OFFSET	0x0C
#define XMCDMA_ERR_OFFSET	0x10
#define XMCDMA_CHOBS1_OFFSET	0x440
#define XMCDMA_CHOBS2_OFFSET	0x444
#define XMCDMA_CHOBS3_OFFSET	0x448
#define XMCDMA_CHOBS4_OFFSET	0x44C
#define XMCDMA_CHOBS5_OFFSET	0x450
#define XMCDMA_CHOBS6_OFFSET	0x454
#define XMCDMA_SGCACHE_OFFSET	0x4B0

/* MM2S Common Register offset */
#define XMCDMA_TXSCHD_TYPE_OFFSET 0x14
#define XMCDMA_TX_WRR_REG_OFFSET  0x18
#define XMCDMA_TX_WRR_REG1_OFFSET 0x1C
#define XMCDMA_TXCH_SERV_OFFSET	  0x20
#define XMCDMA_TXAXCACHE_OFFSET	  0x24
#define XMCDMA_TXINT_SER_OFFSET   0x28

/* S2MM Common Register offset */
#define XMCDMA_CPKTDROP_OFFSET	0x14
#define XMCDMA_RXCH_SER_OFFSET	0x18
#define XMCDMA_RXAXCACHE_OFFSET	0x1C
#define XMCDMA_RXINT_SER_OFFSET	0x20

/* Per Channel Register offset's */
#define XMCDMA_CR_OFFSET    		0x40
#define XMCDMA_SR_OFFSET		0x44
#define XMCDMA_CDESC_OFFSET		0x48
#define XMCDMA_CDESC_MSB_OFFSET		0x4C
#define XMCDMA_TDESC_OFFSET		0x50
#define XMCDMA_TDESC_MSB_OFFSET		0x54
#define XMCDMA_PKTDROP_OFFSET		0x58
#define XMCDMA_TX_PKTCNT_STAT_OFFSET	0x58
#define XMCDMA_RX_PKTCNT_STAT_OFFSET	0x5C

#define XMCDMA_NXTCHAN_OFFSET	0x40
#define XMCDMA_NXTOBS_OFFSET	0x4
#define XMCDMA_RX_OFFSET 	0x500

#define XMCDMA_CCR_RUNSTOP_MASK		0x00000001
#define XMCDMA_CCR_RESET_MASK		0x00000004

#define XMCDMA_CSR_HALTED_MASK		0x00000001
#define XMCDMA_CSR_IDLE_MASK		0x00000002

#define XMCDMA_IRQ_PKTDROP_MASK		0x00000010
#define XMCDMA_IRQ_IOC_MASK		0x00000020
#define XMCDMA_IRQ_DELAY_MASK		0x00000040
#define XMCDMA_IRQ_ERROR_MASK		0x00000080
#define XMCDMA_IRQ_ALL_MASK		0x000000F0

#define XMCDMA_DELAY_MASK		0xFF000000
#define XMCDMA_COALESCE_MASK		0x00FF0000
#define XMCDMA_PKTDROP_COALESCE_MASK	0x0000FF00

#define XMCDMA_COALESCE_SHIFT		16
#define XMCDMA_DELAY_SHIFT		24

#define XMCDMA_CHEN_MASK		0x000000FF

#define XMCDMA_CHID_MASK		0x000000FF

#define XMCDMA_ERR_INTERNAL_MASK	0x00000001
#define XMCDMA_ERR_SLAVE_MASK		0x00000002
#define XMCDMA_ERR_DECODE_MASK		0x00000004
#define XMCDMA_ERR_SG_INT_MASK		0x00000010
#define XMCDMA_ERR_SG_SLV_MASK		0x00000020
#define XMCDMA_ERR_SG_DEC_MASK		0x00000040

#define XMCDMA_PKTDROP_CNT_MASK		0xFFFFFFFF

#define XMCDMA_AXUSER_MASK		0x00000F00
#define XMCDMA_AXCACHE_MASK		0x0000000F

#define XMCDMA_TX_WRRCH0_MASK		0x0000000F
#define XMCDMA_TX_WRRCH1_MASK		0x000000F0
#define XMCDMA_TX_WRRCH2_MASK		0x00000F00
#define XMCDMA_TX_WRRCH3_MASK		0x0000F000
#define XMCDMA_TX_WRRCH4_MASK		0x000F0000
#define XMCDMA_TX_WRRCH5_MASK		0x00F00000
#define XMCDMA_TX_WRRCH6_MASK		0x0F000000
#define XMCDMA_TX_WRRCH7_MASK		0xF0000000

#define XMCDMA_TX_WRRCH_MASK(Chan_id)	WRR_MASK((Chan_id * 4 + 3), Chan_id *4)
#define XMCDMA_TX_WRRCH_SHIFT(Chan_id)	Chan_id * 4

#define XMCDMA_SGAWCACHE_MASK		0x000F0000
#define XMCDMA_SGARCACHE_MASK		0x0000000F
#define XMCDMA_SGAWCACHE_SHIFT		16

/** @name Buffer Descriptor offsets
 *  The first 13 words are used by hardware.
 *  All words after the 13rd word are for software use only.
 *  @{
 */
#define XMCDMA_BD_NDESC_OFFSET		0x00  /**< Next descriptor pointer */
#define XMCDMA_BD_NDESC_MSB_OFFSET	0x04  /**< Next descriptor pointer */
#define XMCDMA_BD_BUFA_OFFSET		0x08  /**< Buffer address */
#define XMCDMA_BD_BUFA_MSB_OFFSET	0x0C  /**< Buffer address */
#define XMCDMA_BD_RESERVED_OFFSET	0x10  /**< Reserverd field */
#define XMCDMA_BD_CTRL_OFFSET		0x14  /**< Control/buffer length */
#define XMCDMA_BD_STS_OFFSET		0x18  /**< Status */
#define XMCDMA_BD_SIDEBAND_STS_OFFSET	0x1C  /**< Side Band Status */
#define XMCDMA_BD_USR0_OFFSET		0x20  /**< User IP specific word0 */
#define XMCDMA_BD_USR1_OFFSET		0x24  /**< User IP specific word1 */
#define XMCDMA_BD_USR2_OFFSET		0x28  /**< User IP specific word2 */
#define XMCDMA_BD_USR3_OFFSET		0x2C  /**< User IP specific word3 */
#define XMCDMA_BD_USR4_OFFSET		0x30  /**< User IP specific word3 */

#define XMCDMA_BD_HAS_DRE_OFFSET	0x34 /**< First unused field by h/w */
#define XMCDMA_BD_HAS_CTRLSTS_OFFSET	0x38
#define XMCDMA_BD_SW_ID_OFFSET		0x3C  /**< Sw ID */

/*@}*/
#define XMCDMA_BD_CTRL_SBAND_OFFSET	0x18  /**< Status */

#define XMCDMA_BD_CTRL_SOF_MASK		0x80000000
#define XMCDMA_BD_CTRL_EOF_MASK		0x40000000
#define XMCDMA_BD_CTRL_ALL_MASK		0xC0000000

#define XMCDMA_BD_CTRL_SBAND_SHIFT	24
#define XMCDMA_BD_STS_COMPLETE_MASK	0x80000000 /**< Completed */
#define XMCDMA_BD_STS_DEC_ERR_MASK	0x40000000 /**< Decode error */
#define XMCDMA_BD_STS_SLV_ERR_MASK	0x20000000 /**< Slave error */
#define XMCDMA_BD_STS_INT_ERR_MASK	0x10000000 /**< Internal err */
#define XMCDMA_BD_STS_ALL_ERR_MASK	0x70000000 /**< All errors */
#define XMCDMA_BD_STS_RXSOF_MASK	0x08000000 /**< First rx pkt */
#define XMCDMA_BD_STS_RXEOF_MASK	0x04000000 /**< Last rx pkt */
#define XMCDMA_BD_STS_ALL_MASK		0xFC000000 /**< All status bits */

#define XMCDMA_BD_SIDEBAND_STS_TID_MASK	   0xFF000000
#define XMCDMA_BD_SIDEBAND_STS_TDEST_MASK  0x000F0000
#define XMCDMA_BD_SIDEBAND_STS_TUSER_MASK  0x0000FFFF

#define XMCDMA_MAX_TRANSFER_LEN		0x03FFFFFF

#define XMCDMA_BD_HW_NUM_BYTES		52  /**< Number of bytes hw used */

#define XMCDMA_BD_START_CLEAR		8   /**< Offset to start clear */
#define XMCDMA_BD_BYTES_TO_CLEAR	44  /**< BD specific bytes to be
									  *  cleared */
#define XMCDMA_BD_HAS_DRE_MASK		0xF00 /**< Whether has DRE mask */
#define XMCDMA_BD_WORDLEN_MASK		0xFF  /**< Whether has DRE mask */

#define XMCDMA_BD_HAS_DRE_SHIFT		8     /**< Whether has DRE shift */
#define XMCDMA_BD_WORDLEN_SHIFT		0     /**< Whether has DRE shift */

#define XMCDMA_LAST_APPWORD		4
/***************** Macros (Inline Functions) Definitions *********************/

#define XMcdma_In32		Xil_In32	/**< Input operation */
#define XMcdma_Out32	Xil_Out32	/**< Output operation */

/*****************************************************************************/
/**
*
* This macro reads the given register.
*
* @param	BaseAddress is the Xilinx base address of the MCDMA core.
* @param	RegOffset is the register offset of the register.
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XMcdma_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XMcdma_ReadReg(BaseAddress, RegOffset) \
		XMcdma_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This macro writes the value into the given register.
*
* @param	BaseAddress is the Xilinx base address of the MCDMA core.
* @param	RegOffset is the register offset of the register.
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note		C-style signature:
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
