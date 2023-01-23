/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsudma_hw.h
* @addtogroup Overview
* @{
*
* The xcsudma_hw.h header file contains identifiers and register-level driver
* functions (or macros) that can be used to access the Xilinx CSU_DMA core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   vnsld  22/10/14 First release
* 1.9	sk     02/11/21 Add description for XPS_CRP_BASEADDRESS and
* 			XCSUDMA_DONE_TIMEOUT_VAL macros.
* 1.11	sk     03/03/22 Replace driver version in addtogroup with Overview.
* 1.11	sk     03/03/22 Update Overview section based on review comments.
* 1.11	sk     03/03/22 Add cond INTERNAL to avoid internal macros.
* 1.12	sk     03/03/22 Added support for VERSAL NET.
* </pre>
*
******************************************************************************/

#ifndef XCSUDMA_HW_H_
#define XCSUDMA_HW_H_	/**< Prevent circular inclusions
			  *  by using protection macros	*/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/** @name Registers offsets
 * @{
 */
#define XCSUDMA_ADDR_OFFSET	0x000U	/**< Address Register Offset */
#define XCSUDMA_SIZE_OFFSET	0x004U	/**< Size Register Offset */
#define XCSUDMA_STS_OFFSET	0x008U	/**< Status Register Offset */
#define XCSUDMA_CTRL_OFFSET	0x00CU	/**< Control Register Offset */
#define XCSUDMA_I_STS_OFFSET	0x014U	/**< Interrupt Status Register
					  *  Offset */
/** @cond INTERNAL */
#define XCSUDMA_CRC_OFFSET	0x010U	/**< CheckSum Register Offset */
#define XCSUDMA_I_EN_OFFSET	0x018U	/**< Interrupt Enable Register
					  *  Offset */
#define XCSUDMA_I_DIS_OFFSET	0x01CU	/**< Interrupt Disable Register
					  *  Offset */
#define XCSUDMA_I_MASK_OFFSET	0x020U	/**< Interrupt Mask Register Offset */
#define XCSUDMA_CTRL2_OFFSET	0x024U	/**< Interrupt Control Register 2
					  *  Offset */
#define XCSUDMA_ADDR_MSB_OFFSET	0x028U	/**< Address's MSB Register Offset */
#define XCSUDMA_SAFETY_CHK_OFFSET 0xFF8U /**< Safety Check Field Offset */
#define XCSUDMA_FUTURE_ECO_OFFSET 0xFFCU /**< Future potential ECO Offset */
/** @endcond */
/*@}*/

/** @cond INTERNAL */
/** @name CSU Base address, CRP Base Address and CSU_DMA reset offset
 * @{
 */
#define XCSU_BASEADDRESS	0xFFCA0000U
						/**< CSU Base Address */
#define XCSU_DMA_RESET_OFFSET	0x0000000CU	/**< CSU_DMA Reset offset */

#ifndef XPS_CRP_BASEADDRESS
#define XPS_CRP_BASEADDRESS	0xF1260000U	/**< CRP Base Address */
#endif
/*@}*/

/** @name CRP PMC_DMA reset offset
 * @{
 */
#define XCRP_PMCDMA_RESET_OFFSET 0x00000328U	/**< PMC_DMA Reset offset */

/*@}*/
/** @name CSU_DMA Reset register bit masks
 * @{
 */
#define XCSUDMA_RESET_SET_MASK		0x00000001U	/**< Reset set mask */
#define XCSUDMA_RESET_UNSET_MASK	0x00000000U	/**< Reset unset mask*/
/*@}*/

/** @name Offset difference for Source and destination
 * @{
 */
#define XCSUDMA_OFFSET_DIFF	0x00000800U	/**< Offset difference for
						  *  source and
						  *  destination channels */
/*@}*/

/** @name Address register bit masks
 * @{
 */
#if defined (VERSAL_NET)
#define XCSUDMA_ADDR_MASK	0xFFFFFFFFU	/**< Address mask */
#else
#define XCSUDMA_ADDR_MASK	0xFFFFFFFCU	/**< Address mask */
#endif
#define XCSUDMA_ADDR_LSB_MASK	0x00000003U	/**< Address alignment check
						  *  mask */
/*@}*/
/** @endcond */

/** @name Size register bit masks and shifts
 * @{
 */
#define XCSUDMA_SIZE_MASK	0x1FFFFFFCU	/**< Mask for size */
#if defined (VERSAL_NET)
#define XCSUDMA_LAST_WORD_MASK	0x20000000U	/**< Last word check bit mask*/
#define XCSUDMA_SIZE_SHIFT	0U		/**< Shift for size */
#else
#define XCSUDMA_LAST_WORD_MASK	0x00000001U	/**< Last word check bit mask*/
#define XCSUDMA_SIZE_SHIFT	2U		/**< Shift for size */
#endif
/*@}*/

/** @cond INTERNAL */
/** @name Status register bit masks and shifts
 * @{
 */
#define XCSUDMA_STS_DONE_CNT_MASK	0x0000E000U	/**< Count done mask */
#define XCSUDMA_STS_FIFO_LEVEL_MASK	0x00001FE0U	/**< FIFO level mask */
#define XCUSDMA_STS_OUTSTDG_MASK	0x0000001EU	/**< No.of outstanding
							  *  read/write
							  *  commands mask */
#define XCSUDMA_STS_BUSY_MASK		0x00000001U	/**< Busy mask */
#define XCSUDMA_STS_DONE_CNT_SHIFT	13U		/**< Shift for Count
							  *  done */
#define XCSUDMA_STS_FIFO_LEVEL_SHIFT	5U		/**< Shift for FIFO
							  *  level */
#define XCUSDMA_STS_OUTSTDG_SHIFT	1U		/**< Shift for No.of
							  *  outstanding
							  *  read/write
							  *  commands */
/*@}*/

/** @name Control register bit masks and shifts
 * @{
 */
#define XCSUDMA_CTRL_SSS_FIFOTHRESH_MASK 0xFE000000U	/**< SSS FIFO threshold
							  *  value mask */
#define XCSUDMA_CTRL_APB_ERR_MASK	0x01000000U	/**< APB register
							  *  access error
							  *  mask */
#define XCSUDMA_CTRL_ENDIAN_MASK	0x00800000U	/**< Endianess mask */
#define XCSUDMA_CTRL_BURST_MASK		0x00400000U	/**< AXI burst type
							  *  mask */
#define XCSUDMA_CTRL_TIMEOUT_MASK	0x003FFC00U	/**< Time out value
							  *  mask */
#define XCSUDMA_CTRL_FIFO_THRESH_MASK	0x000003FCU	/**< FIFO threshold
							  *  mask */
#define XCSUDMA_CTRL_PAUSE_MEM_MASK	0x00000001U	/**< Memory pause
							  *  mask */
#define XCSUDMA_CTRL_PAUSE_STRM_MASK	0x00000002U	/**< Stream pause
							  *  mask */
#define XCSUDMA_CTRL_SSS_FIFOTHRESH_SHIFT 25U		/**< SSS FIFO threshold
							  *  shift */
#define XCSUDMA_CTRL_APB_ERR_SHIFT	24U		/**< APB error shift */
#define XCSUDMA_CTRL_ENDIAN_SHIFT	23U		/**< Endianess shift */
#define XCSUDMA_CTRL_BURST_SHIFT	22U		/**< AXI burst type
							  *  shift */
#define XCSUDMA_CTRL_TIMEOUT_SHIFT	10U		/**< Time out value
							  *  shift */
#define XCSUDMA_CTRL_FIFO_THRESH_SHIFT	2U		/**< FIFO thresh
							  *  shift */
/*@}*/

/** @name CheckSum register bit masks
 * @{
 */
#define XCSUDMA_CRC_RESET_MASK		0x00000000U	/**< Mask to reset
							  *  value of
							  *  check sum */
/*@}*/
/** @endcond */

/** @name Interrupt Enable/Disable/Mask/Status registers bit masks
 * @{
 */
#define XCSUDMA_IXR_FIFO_OVERFLOW_MASK	0x00000001U	/**< FIFO overflow
							  *  mask, it is valid
							  *  only to Destination
							  *  Channel */
#define XCSUDMA_IXR_INVALID_APB_MASK	0x00000040U	/**< Invalid APB access
							  *  mask */
#define XCSUDMA_IXR_FIFO_THRESHHIT_MASK	0x00000020U	/**< FIFO threshold hit
							  *  indicator mask */
#define XCSUDMA_IXR_TIMEOUT_MEM_MASK	0x00000010U	/**< Time out counter
							  *  expired to access
							  *  memory mask */
#define XCSUDMA_IXR_TIMEOUT_STRM_MASK	0x00000008U	/**< Time out counter
							  *  expired to access
							  *  stream mask */
#define XCSUDMA_IXR_AXI_WRERR_MASK	0x00000004U	/**< AXI Read/Write
							  *  error mask */
#define XCSUDMA_IXR_DONE_MASK		0x00000002U	/**< Done mask */
#define XCSUDMA_IXR_MEM_DONE_MASK	0x00000001U	/**< Memory done
							  *  mask, it is valid
							  *  only for source
							  *  channel*/
#define XCSUDMA_IXR_SRC_MASK		0x0000007FU
					/**< ((XCSUDMA_IXR_INVALID_APB_MASK)|
					(XCSUDMA_IXR_FIFO_THRESHHIT_MASK) |
					(XCSUDMA_IXR_TIMEOUT_MEM_MASK) |
					(XCSUDMA_IXR_TIMEOUT_STRM_MASK) |
					(XCSUDMA_IXR_AXI_WRERR_MASK) |
					(XCSUDMA_IXR_DONE_MASK) |
					(XCSUDMA_IXR_MEM_DONE_MASK)) */
					/**< All interrupt mask
					  *  for source */
#define XCSUDMA_IXR_DST_MASK		0x000000FEU
					/**< ((XCSUDMA_IXR_FIFO_OVERFLOW_MASK) |
					(XCSUDMA_IXR_INVALID_APB_MASK) |
					(XCSUDMA_IXR_FIFO_THRESHHIT_MASK) |
					(XCSUDMA_IXR_TIMEOUT_MEM_MASK) |
					(XCSUDMA_IXR_TIMEOUT_STRM_MASK) |
					(XCSUDMA_IXR_AXI_WRERR_MASK) |
					(XCSUDMA_IXR_DONE_MASK)) */
					/**< All interrupt mask
					  *  for destination */
/*@}*/

/** @cond INTERNAL */
/** @name Control register 2 bit masks and shifts
 * @{
 */
#define XCSUDMA_CTRL2_RESERVED_MASK	0x083F0000U	/**< Reserved bits
							  *  mask */
#define XCSUDMA_CTRL2_ACACHE_MASK	0X07000000U	/**< AXI CACHE mask */
#define XCSUDMA_CTRL2_ROUTE_MASK	0x00800000U	/**< Route mask */
#define XCSUDMA_CTRL2_TIMEOUT_EN_MASK	0x00400000U	/**< Time out counters
							  *  enable mask */
#define XCSUDMA_CTRL2_TIMEOUT_PRE_MASK	0x0000FFF0U	/**< Time out pre
							  *  mask */
#define XCSUDMA_CTRL2_MAXCMDS_MASK	0x0000000FU	/**< Maximum commands
							  *  mask */
#define XCSUDMA_CTRL2_RESET_MASK	0x0000FFF8U	/**< Reset mask */
#define XCSUDMA_CTRL2_ACACHE_SHIFT	24U		/**< Shift for
							  *  AXI R/W CACHE */
#define XCSUDMA_CTRL2_ROUTE_SHIFT	23U		/**< Shift for route */
#define XCSUDMA_CTRL2_TIMEOUT_EN_SHIFT	22U		/**< Shift for Timeout
							  *  enable feild */
#define XCSUDMA_CTRL2_TIMEOUT_PRE_SHIFT	4U		/**< Shift for Timeout
							  *  pre feild */
/*@}*/

/** @name MSB Address register bit masks and shifts
 * @{
 */
#define XCSUDMA_MSB_ADDR_MASK	0x0001FFFFU	/**< MSB bits of address
						  *  mask */
#define XCSUDMA_MSB_ADDR_SHIFT	32U		/**< Shift for MSB bits of
						  *  address */
/*@}*/
/** @endcond */

/** @name Software done timeout value
 * @{
 */
#define XCSUDMA_DONE_TIMEOUT_VAL 300000000U	/**< SW timeout loop value
						  *  for transfer completion */
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

#define XCsuDma_In32		Xil_In32	/**< Input operation */
#define XCsuDma_Out32		Xil_Out32	/**< Output operation */

/*****************************************************************************/
/**
*
* This macro reads the given register.
*
* @param	BaseAddress is the Xilinx base address of the CSU_DMA core.
* @param	RegOffset is the register offset of the register.
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XCsuDma_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XCsuDma_ReadReg(BaseAddress, RegOffset) \
		XCsuDma_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This macro writes the value into the given register.
*
* @param	BaseAddress is the Xilinx base address of the CSU_DMA core.
* @param	RegOffset is the register offset of the register.
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XCsuDma_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XCsuDma_WriteReg(BaseAddress, RegOffset, Data) \
		XCsuDma_Out32((BaseAddress) + (u32)(RegOffset), (u32)(Data))


#ifdef __cplusplus
}

#endif


#endif /* End of protection macro */
/** @} */
