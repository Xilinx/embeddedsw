/******************************************************************************
*
* Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
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
* @file xdualsplitter_hw.h
* @addtogroup dual_splitter_v1_1
* @{
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx Dual Splitter core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- --------------------------------------------------
* 1.00  sha 07/21/14 Initial release.
* </pre>
*
******************************************************************************/

#ifndef XDUALSPLITTER_HW_H_
#define XDUALSPLITTER_HW_H_	/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Core registers offsets
* @{
*/
#define XDUSP_GENR_CTL_OFFSET		0x0000	/**< General Control register
						  *  offset */
#define XDUSP_GENR_ERR_OFFSET		0x0008	/**< General Error register
						  *  offset */
#define XDUSP_IRQ_EN_OFFSET		0x000C	/**< IRQ Enable register
						  *  offset */
#define XDUSP_TIME_CTL_OFFSET		0x0020	/**< Time Control register
						  *  offset */
#define XDUSP_CORE_CTL_OFFSET		0x0100	/**< Core Control register
						  *  offset */
/*@}*/

/** @name General control register bit masks
* @{
*/
#define XDUSP_GENR_CTL_EN_MASK		0x00000001	/**< Enable mask */
#define XDUSP_GENR_CTL_RUE_MASK		0x00000002	/**< Register update
							  *  enable mask */
#define XDUSP_GENR_CTL_RST_MASK		0x80000000	/**< Reset mask */
/*@}*/

/** @name Error register bit masks
* @{
*/
#define XDUSP_ERR_EOL_EARLY_MASK	0x00000001	/**< Error: End of line
							  *  early mask */
#define XDUSP_ERR_EOL_LATE_MASK		0x00000002	/**< Error: End of line
							  *  late mask */
#define XDUSP_ERR_SOF_EARLY_MASK	0x00000004	/**< Error: Start of
							  *  frame early
							  * mask */
#define XDUSP_ERR_SOF_LATE_MASK		0x00000008	/**< Error: Start of
							  *  frame late mask */
#define XDUSP_ALL_ERR_MASK		(XDUSP_ERR_EOL_EARLY_MASK | \
					 XDUSP_ERR_EOL_LATE_MASK | \
					 XDUSP_ERR_SOF_EARLY_MASK | \
					 XDUSP_ERR_SOF_LATE_MASK) /**< All
								   *  error
								   * mask */
/*@}*/

/** @name Time control register bit masks and shifts
* @{
*/
#define XDUSP_TIME_CTL_WIDTH_MASK	0x0000FFFF	/**< Image width
							  *  mask */
#define XDUSP_TIME_CTL_HEIGHT_MASK	0xFFFF0000	/**< Image height
							  *  mask */
#define XDUSP_TIME_CTL_HEIGHT_SHIFT	16		/**< Image height
							  *  shift */
/*@}*/

/** @name Core control register masks and shifts
* @{
*/
#define XDUSP_CORE_CTL_IN_SAMPLES_MASK	0x000000FF	/**< Input
							  *  samples
							  *  mask */
#define XDUSP_CORE_CTL_OUT_SAMPLES_MASK	0x0000FF00	/**< Output
							  *  samples
							  *  mask */
#define XDUSP_CORE_CTL_IMG_SEG_MASK	0x00FF0000	/**< No of
							  *  image
							  *  segments
							  *  mask */
#define XDUSP_CORE_CTL_OVRLAP_SEG_MASK	0xFF000000	/**< No of
							  *  over-
							  *  lapping
							  *  segments
							  *  mask */
#define XDUSP_CORE_CTL_OUT_SAMPLES_SHIFT	8	/**< Output samples
							  *  shift */
#define XDUSP_CORE_CTL_IMG_SEG_SHIFT	16	/**< No of image
						  *  segments shift */
#define XDUSP_CORE_CTL_OVRLAP_SEG_SHIFT	24	/**< No of overlapping
						  *  segments shift */
/*@}*/

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/** @name Device register I/O APIs
* @{
*/
#define XDualSplitter_In32	Xil_In32	/**< Input operation. */
#define XDualSplitter_Out32	Xil_Out32	/**< Output operation. */

/*****************************************************************************/
/**
*
* This macro reads a value from a Dual Splitter core's register.
* A 32 bit read is performed. If the component is implemented in a smaller
* width, only the least significant data is read from the register. The most
* significant data will be read as 0.
*
* @param	BaseAddress is the base address of the XDualSplitter core.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file).
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XDualSplitter_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XDualSplitter_ReadReg(BaseAddress, RegOffset) \
	XDualSplitter_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value into a Dual Splitter core's register.
* A 32 bit write is performed. If the component is implemented in a smaller
* width, only the least significant data is written.
*
* @param	BaseAddress is the base address of the XDualSplitter core.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file) to be written.
* @param	Data is the 32-bit value to write into the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XDualSplitter_WriteReg(u32 BaseAddress, u32 RegOffset,
*		u32 Data)
*
******************************************************************************/
#define XDualSplitter_WriteReg(BaseAddress, RegOffset, Data) \
	XDualSplitter_Out32((BaseAddress) + (u32)(RegOffset), (Data))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
