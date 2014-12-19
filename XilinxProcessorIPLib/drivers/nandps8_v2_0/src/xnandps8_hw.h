/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
*
* @file xnandps8_hw.h
*
* This file contains identifiers and low-level macros/functions for the Arasan
* NAND flash controller driver.
*
* See xnandps8.h for more information.
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date        Changes
* ----- ----   ----------  -----------------------------------------------
* 1.0   nm     05/06/2014  First Release
* 2.0   sb     11/04/2014  Changed XNANDPS8_ECC_SLC_MLC_MASK to
*			   XNANDPS8_ECC_HAMMING_BCH_MASK.
* </pre>
*
******************************************************************************/

#ifndef XNANDPS8_HW_H		/* prevent circular inclusions */
#define XNANDPS8_HW_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/************************** Register Offset Definitions **********************/

#define XNANDPS8_PKT_OFFSET		0x00U	/**< Packet Register */
#define XNANDPS8_MEM_ADDR1_OFFSET	0x04U	/**< Memory Address
						  Register 1 */
#define XNANDPS8_MEM_ADDR2_OFFSET	0x08U	/**< Memory Address
						  Register 2 */
#define XNANDPS8_CMD_OFFSET		0x0CU	/**< Command Register */
#define XNANDPS8_PROG_OFFSET		0x10U	/**< Program Register */
#define XNANDPS8_INTR_STS_EN_OFFSET	0x14U	/**< Interrupt Status
						     Enable Register */
#define XNANDPS8_INTR_SIG_EN_OFFSET	0x18U	/**< Interrupt Signal
						     Enable Register */
#define XNANDPS8_INTR_STS_OFFSET	0x1CU	/**< Interrupt Status
						  Register */
#define XNANDPS8_READY_BUSY_OFFSET	0x20U	/**< Ready/Busy status
						  Register */
#define XNANDPS8_FLASH_STS_OFFSET	0x28U	/**< Flash Status Register */
#define XNANDPS8_TIMING_OFFSET		0x2CU	/**< Timing Register */
#define XNANDPS8_BUF_DATA_PORT_OFFSET	0x30U	/**< Buffer Data Port
						  Register */
#define XNANDPS8_ECC_OFFSET		0x34U	/**< ECC Register */
#define XNANDPS8_ECC_ERR_CNT_OFFSET	0x38U	/**< ECC Error Count
						  Register */
#define XNANDPS8_ECC_SPR_CMD_OFFSET	0x3CU	/**< ECC Spare Command
						     Register */
#define XNANDPS8_ECC_CNT_1BIT_OFFSET	0x40U	/**< Error Count 1bit
						  Register */
#define XNANDPS8_ECC_CNT_2BIT_OFFSET	0x44U	/**< Error Count 2bit
						  Register */
#define XNANDPS8_ECC_CNT_3BIT_OFFSET	0x48U	/**< Error Count 3bit
						  Register */
#define XNANDPS8_ECC_CNT_4BIT_OFFSET	0x4CU	/**< Error Count 4bit
						  Register */
#define XNANDPS8_CPU_REL_OFFSET		0x58U	/**< CPU Release Register */
#define XNANDPS8_ECC_CNT_5BIT_OFFSET	0x5CU	/**< Error Count 5bit
						  Register */
#define XNANDPS8_ECC_CNT_6BIT_OFFSET	0x60U	/**< Error Count 6bit
						  Register */
#define XNANDPS8_ECC_CNT_7BIT_OFFSET	0x64U	/**< Error Count 7bit
						  Register */
#define XNANDPS8_ECC_CNT_8BIT_OFFSET	0x68U	/**< Error Count 8bit
						  Register */
#define XNANDPS8_DATA_INTF_OFFSET	0x6CU	/**< Data Interface Register */
#define XNANDPS8_DMA_SYS_ADDR0_OFFSET	0x50U	/**< DMA System Address 0
						  Register */
#define XNANDPS8_DMA_SYS_ADDR1_OFFSET	0x24U	/**< DMA System Address 1
						  Register */
#define XNANDPS8_DMA_BUF_BND_OFFSET	0x54U	/**< DMA Buffer Boundary
						  Register */
#define XNANDPS8_SLV_DMA_CONF_OFFSET	0x80U	/**< Slave DMA Configuration
						  Register */

/** @name Packet Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_PKT_PKT_SIZE_MASK		0x000007FFU /**< Packet Size */
#define XNANDPS8_PKT_PKT_CNT_MASK		0x00FFF000U /**< Packet Count*/
#define XNANDPS8_PKT_PKT_CNT_SHIFT		12U /**< Packet Count Shift */
/* @} */

/** @name Memory Address Register 1 bit definitions and masks
 *  @{
 */
#define XNANDPS8_MEM_ADDR1_COL_ADDR_MASK	0x0000FFFFU /**< Column Address
							     Mask */
#define XNANDPS8_MEM_ADDR1_PG_ADDR_MASK		0xFFFF0000U /**< Page, Block
							     Address Mask */
#define XNANDPS8_MEM_ADDR1_PG_ADDR_SHIFT	16U /**< Page Shift */
/* @} */

/** @name Memory Address Register 2 bit definitions and masks
 *  @{
 */
#define XNANDPS8_MEM_ADDR2_MEM_ADDR_MASK	0x000000FFU /**< Memory Address
								*/
#define XNANDPS8_MEM_ADDR2_BUS_WIDTH_MASK	0x01000000U /**< Bus Width */
#define XNANDPS8_MEM_ADDR2_NFC_BCH_MODE_MASK	0x0E000000U /**< BCH Mode
							     Value */
#define XNANDPS8_MEM_ADDR2_MODE_MASK		0x30000000U /**< Flash
							     Connection Mode */
#define XNANDPS8_MEM_ADDR2_CHIP_SEL_MASK	0xC0000000U /**< Chip Select */
#define XNANDPS8_MEM_ADDR2_CHIP_SEL_SHIFT	30U	/**< Chip select
							shift */
#define XNANDPS8_MEM_ADDR2_BUS_WIDTH_SHIFT	24U	/**< Bus width shift */
#define XNANDPS8_MEM_ADDR2_NFC_BCH_MODE_SHIFT	25U
/* @} */

/** @name Command Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_CMD_CMD1_MASK			0x000000FFU /**< 1st Cycle
							     Command */
#define XNANDPS8_CMD_CMD2_MASK			0x0000FF00U /**< 2nd Cycle
							     Command */
#define XNANDPS8_CMD_PG_SIZE_MASK		0x03800000U /**< Page Size */
#define XNANDPS8_CMD_DMA_EN_MASK		0x0C000000U /**< DMA Enable
							     Mode */
#define XNANDPS8_CMD_ADDR_CYCLES_MASK		0x70000000U /**< Number of
							     Address Cycles */
#define XNANDPS8_CMD_ECC_ON_MASK		0x80000000U /**< ECC ON/OFF */
#define XNANDPS8_CMD_CMD2_SHIFT			8U /**< 2nd Cycle Command
						    Shift */
#define XNANDPS8_CMD_PG_SIZE_SHIFT		23U /**< Page Size Shift */
#define XNANDPS8_CMD_DMA_EN_SHIFT		26U /**< DMA Enable Shift */
#define XNANDPS8_CMD_ADDR_CYCLES_SHIFT		28U /**< Number of Address
						     Cycles Shift */
#define XNANDPS8_CMD_ECC_ON_SHIFT		31U /**< ECC ON/OFF */
/* @} */

/** @name Program Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_PROG_RD_MASK			0x00000001U /**< Read */
#define XNANDPS8_PROG_MUL_DIE_MASK		0x00000002U /**< Multi Die */
#define XNANDPS8_PROG_BLK_ERASE_MASK		0x00000004U /**< Block Erase */
#define XNANDPS8_PROG_RD_STS_MASK		0x00000008U /**< Read Status */
#define XNANDPS8_PROG_PG_PROG_MASK		0x00000010U /**< Page Program */
#define XNANDPS8_PROG_MUL_DIE_RD_MASK		0x00000020U /**< Multi Die Rd */
#define XNANDPS8_PROG_RD_ID_MASK		0x00000040U /**< Read ID */
#define XNANDPS8_PROG_RD_PRM_PG_MASK		0x00000080U /**< Read Param
							     Page */
#define XNANDPS8_PROG_RST_MASK			0x00000100U /**< Reset */
#define XNANDPS8_PROG_GET_FEATURES_MASK		0x00000200U /**< Get Features */
#define XNANDPS8_PROG_SET_FEATURES_MASK		0x00000400U /**< Set Features */
#define XNANDPS8_PROG_RD_UNQ_ID_MASK		0x00000800U /**< Read Unique
							     ID */
#define XNANDPS8_PROG_RD_STS_ENH_MASK		0x00001000U /**< Read Status
							     Enhanced */
#define XNANDPS8_PROG_RD_INTRLVD_MASK		0x00002000U /**< Read
							     Interleaved */
#define XNANDPS8_PROG_CHNG_RD_COL_ENH_MASK	0x00004000U /**< Change Read
								Column
								Enhanced */
#define XNANDPS8_PROG_COPY_BACK_INTRLVD_MASK	0x00008000U /**< Copy Back
								Interleaved */
#define XNANDPS8_PROG_RD_CACHE_START_MASK	0x00010000U /**< Read Cache
							     Start */
#define XNANDPS8_PROG_RD_CACHE_SEQ_MASK		0x00020000U /**< Read Cache
							     Sequential */
#define XNANDPS8_PROG_RD_CACHE_RAND_MASK	0x00040000U /**< Read Cache
								Random */
#define XNANDPS8_PROG_RD_CACHE_END_MASK		0x00080000U /**< Read Cache
							     End */
#define XNANDPS8_PROG_SMALL_DATA_MOVE_MASK	0x00100000U /**< Small Data
							     Move */
#define XNANDPS8_PROG_CHNG_ROW_ADDR_MASK	0x00200000U /**< Change Row
								Address */
#define XNANDPS8_PROG_CHNG_ROW_ADDR_END_MASK	0x00400000U /**< Change Row
								Address End */
#define XNANDPS8_PROG_RST_LUN_MASK		0x00800000U /**< Reset LUN */
#define XNANDPS8_PROG_PGM_PG_CLR_MASK		0x01000000U /**< Enhanced
							     Program Page
							     Register Clear */
#define XNANDPS8_PROG_VOL_SEL_MASK		0x02000000U /**< Volume Select */
#define XNANDPS8_PROG_ODT_CONF_MASK		0x04000000U /**< ODT Configure */
/* @} */

/** @name Interrupt Status Enable Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_INTR_STS_EN_BUFF_WR_RDY_STS_EN_MASK	0x00000001U /**< Buffer
								     Write Ready
								     Status
								     Enable */
#define XNANDPS8_INTR_STS_EN_BUFF_RD_RDY_STS_EN_MASK	0x00000002U /**< Buffer
								     Read Ready
								     Status
								     Enable */
#define XNANDPS8_INTR_STS_EN_TRANS_COMP_STS_EN_MASK	0x00000004U /**< Transfer
								     Complete
								     Status
								     Enable */
#define XNANDPS8_INTR_STS_EN_MUL_BIT_ERR_STS_EN_MASK	0x00000008U /**< Multi
								     Bit Error
								     Status
								     Enable */
#define XNANDPS8_INTR_STS_EN_ERR_INTR_STS_EN_MASK	0x00000010U /**< Single
								     Bit Error
								     Status
								     Enable,
								     BCH Detect
								     Error
								     Status
								     Enable */
#define XNANDPS8_INTR_STS_EN_DMA_INT_STS_EN_MASK	0x00000040U /**< DMA
								     Status
								     Enable */
#define XNANDPS8_INTR_STS_EN_ERR_AHB_STS_EN_MASK	0x00000080U /**< Error
								     AHB Status
								     Enable */
/* @} */

/** @name Interrupt Signal Enable Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_INTR_SIG_EN_BUFF_WR_RDY_STS_EN_MASK	0x00000001U /**< Buffer
								     Write Ready
								     Signal
								     Enable */
#define XNANDPS8_INTR_SIG_EN_BUFF_RD_RDY_STS_EN_MASK	0x00000002U /**< Buffer
								     Read Ready
								     Signal
								     Enable */
#define XNANDPS8_INTR_SIG_EN_TRANS_COMP_STS_EN_MASK	0x00000004U /**< Transfer
								     Complete
								     Signal
								     Enable */
#define XNANDPS8_INTR_SIG_EN_MUL_BIT_ERR_STS_EN_MASK	0x00000008U /**< Multi
								     Bit Error
								     Signal
								     Enable */
#define XNANDPS8_INTR_SIG_EN_ERR_INTR_STS_EN_MASK	0x00000010U /**< Single
								     Bit Error
								     Signal
								     Enable,
								     BCH Detect
								     Error
								     Signal
								     Enable */
#define XNANDPS8_INTR_SIG_EN_DMA_INT_STS_EN_MASK	0x00000040U /**< DMA
								     Signal
								     Enable */
#define XNANDPS8_INTR_SIG_EN_ERR_AHB_STS_EN_MASK	0x00000080U /**< Error
								     AHB Signal
								     Enable */
/* @} */

/** @name Interrupt Status Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_INTR_STS_BUFF_WR_RDY_STS_EN_MASK	0x00000001U /**< Buffer
								     Write
								     Ready */
#define XNANDPS8_INTR_STS_BUFF_RD_RDY_STS_EN_MASK	0x00000002U /**< Buffer
								     Read
								     Ready */
#define XNANDPS8_INTR_STS_TRANS_COMP_STS_EN_MASK	0x00000004U /**< Transfer
								     Complete */
#define XNANDPS8_INTR_STS_MUL_BIT_ERR_STS_EN_MASK	0x00000008U /**< Multi
								    Bit Error */
#define XNANDPS8_INTR_STS_ERR_INTR_STS_EN_MASK		0x00000010U /**< Single
								     Bit Error,
								     BCH Detect
								     Error */
#define XNANDPS8_INTR_STS_DMA_INT_STS_EN_MASK		0x00000040U /**< DMA
								     Interrupt
								     */
#define XNANDPS8_INTR_STS_ERR_AHB_STS_EN_MASK		0x00000080U /**< Error
								     AHB */
/* @} */

/** @name Interrupt bit definitions and masks
 *  @{
 */
#define XNANDPS8_INTR_BUFF_WR_RDY_STS_EN_MASK	0x00000001U /**< Buffer Write
								Ready Status
								Enable */
#define XNANDPS8_INTR_BUFF_RD_RDY_STS_EN_MASK	0x00000002U /**< Buffer Read
								Ready Status
								Enable */
#define XNANDPS8_INTR_TRANS_COMP_STS_EN_MASK	0x00000004U /**< Transfer
								Complete Status
								Enable */
#define XNANDPS8_INTR_MUL_BIT_ERR_STS_EN_MASK	0x00000008U /**< Multi Bit Error
								Status Enable */
#define XNANDPS8_INTR_ERR_INTR_STS_EN_MASK	0x00000010U /**< Single Bit Error
								Status Enable,
								BCH Detect Error
								Status Enable */
#define XNANDPS8_INTR_DMA_INT_STS_EN_MASK	0x00000040U /**< DMA Status
								Enable */
#define XNANDPS8_INTR_ERR_AHB_STS_EN_MASK	0x00000080U /**< Error AHB Status
								Enable */
/* @} */

/** @name ID2 Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_ID2_DEVICE_ID2_MASK		0x000000FFU /**< MSB Device ID */
/* @} */

/** @name Flash Status Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_FLASH_STS_FLASH_STS_MASK	0x0000FFFFU /**< Flash Status
							     Value */
/* @} */

/** @name Timing Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_TIMING_TCCS_TIME_MASK		0x00000003U /**< Change column
							     setup time */
#define XNANDPS8_TIMING_SLOW_FAST_TCAD_MASK	0x00000004U /**< Slow/Fast device
							     */
#define XNANDPS8_TIMING_DQS_BUFF_SEL_MASK	0x00000078U /**< Write/Read data
							     transaction value
							     */
#define XNANDPS8_TIMING_TADL_TIME_MASK		0x00007F80U /**< Address latch
							     enable to Data
							     loading time */
/* @} */

/** @name ECC Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_ECC_ADDR_MASK			0x0000FFFFU /**< ECC address */
#define XNANDPS8_ECC_SIZE_MASK			0x01FF0000U /**< ECC size */
#define XNANDPS8_ECC_HAMMING_BCH_MASK		0x02000000U /**< Hamming/BCH
							     support */
/* @} */

/** @name ECC Error Count Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_ECC_ERR_CNT_PKT_BND_ERR_CNT_MASK	0x000000FFU /**< Packet
								     bound error
								     count */
#define XNANDPS8_ECC_ERR_CNT_PG_BND_ERR_CNT_MASK	0x0000FF00U /**< Page
								     bound error
								     count */
/* @} */

/** @name ECC Spare Command Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_ECC_SPR_CMD_SPR_CMD_MASK		0x000000FFU /**< ECC
								     spare
								     command */
#define XNANDPS8_ECC_SPR_CMD_ECC_ADDR_CYCLES_MASK	0x70000000U /**< Number
								     of ECC/
								     spare
								     address
								     cycles */
/* @} */

/** @name Data Interface Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_DATA_INTF_SDR_MASK		0x00000007U /**< SDR mode */
#define XNANDPS8_DATA_INTF_NVDDR_MASK		0x00000038U /**< NVDDR mode */
#define XNANDPS8_DATA_INTF_NVDDR2_MASK		0x000001C0U /**< NVDDR2 mode */
#define XNANDPS8_DATA_INTF_DATA_INTF_MASK	0x00000600U /**< Data
							     Interface */
#define XNANDPS8_DATA_INTF_NVDDR_SHIFT		3U /**< NVDDR mode shift */
#define XNANDPS8_DATA_INTF_DATA_INTF_SHIFT	9U /**< Data Interface Shift */
/* @} */

/** @name DMA Buffer Boundary Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_DMA_BUF_BND_BND_MASK		0x00000007U /**< DMA buffer
							     boundary */
#define XNANDPS8_DMA_BUF_BND_4K			0x0U
#define XNANDPS8_DMA_BUF_BND_8K			0x1U
#define XNANDPS8_DMA_BUF_BND_16K		0x2U
#define XNANDPS8_DMA_BUF_BND_32K		0x3U
#define XNANDPS8_DMA_BUF_BND_64K		0x4U
#define XNANDPS8_DMA_BUF_BND_128K		0x5U
#define XNANDPS8_DMA_BUF_BND_256K		0x6U
#define XNANDPS8_DMA_BUF_BND_512K		0x7U
/* @} */

/** @name Slave DMA Configuration Register bit definitions and masks
 *  @{
 */
#define XNANDPS8_SLV_DMA_CONF_SDMA_TX_RX_MASK		0x00000001U /**< Slave
								     DMA
								     Transfer
								     Direction
								     */
#define XNANDPS8_SLV_DMA_CONF_DMA_TRANS_CNT_MASK	0x001FFFFEU /**< Slave
								     DMA
								     Transfer
								     Count */
#define XNANDPS8_SLV_DMA_CONF_DMA_BURST_SIZE_MASK	0x00E00000U /**< Slave
								     DMA
								     Burst
								     Size */
#define XNANDPS8_SLV_DMA_CONF_DMA_TMOUT_CNT_VAL_MASK	0x0F000000U /**< DMA
								     Timeout
								     Counter
								     Value */
#define XNANDPS8_SLV_DMA_CONF_SDMA_EN_MASK		0x10000000U /**< Slave
								     DMA
								     Enable */
/* @} */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* This macro reads the given register.
*
* @param	BaseAddress is the base address of controller registers.
* @param	RegOffset is the register offset to be read.
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XNandPs8_ReadReg(u32 BaseAddress, u32 RegOffset)
*
*****************************************************************************/
#define XNandPs8_ReadReg(BaseAddress, RegOffset)			\
			Xil_In32((BaseAddress) + (RegOffset))

/****************************************************************************/
/**
*
* This macro writes the given register.
*
* @param	BaseAddress is the the base address of controller registers.
* @param	RegOffset is the register offset to be written.
* @param	Data is the the 32-bit value to write to the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XNandPs8_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XNandPs8_WriteReg(BaseAddress, RegOffset, Data)			\
			Xil_Out32(((BaseAddress) + (RegOffset)), (Data))

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* XNANDPS8_HW_H end of protection macro */
