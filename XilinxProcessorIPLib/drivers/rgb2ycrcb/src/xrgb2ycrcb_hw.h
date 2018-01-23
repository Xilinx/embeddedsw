/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xrgb2ycrcb_hw.h
* @addtogroup rgb2ycrcb_v7_1
* @{
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx RGB to YCrCb color space
* converter (RGB2YCRCB) core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------------
* 7.0   adk    01/28/14 First release.
*                       Added the register offsets and bit masks for the
*                       registers.
*                       Added backward compatibility macros.
*</pre>
*
******************************************************************************/

#ifndef XRGB2YCRCB_HW_H_
#define XRGB2YCRCB_HW_H_	/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Control Registers
 *
 * Control registers offset for RGB2YCRCB core.
 * @{
 */
#define XRGB_CONTROL_OFFSET		0x000	/**< Control offset */
#define XRGB_STATUS_OFFSET		0x004	/**< Status offset */
#define XRGB_ERROR_OFFSET		0x008	/**< Error offset */
#define XRGB_IRQ_EN_OFFSET		0x00C	/**< IRQ Enable	offset */
#define XRGB_VERSION_OFFSET		0x010	/**< Version offset */
#define XRGB_SYSDEBUG0_OFFSET		0x014	/**< System Debug 0 offset */
#define XRGB_SYSDEBUG1_OFFSET		0x018	/**< System Debug 1 offset */
#define XRGB_SYSDEBUG2_OFFSET		0x01C	/**< System Debug 2 offset */
/*@}*/

/** @name Timing Control Registers
 *
 * Timing control registers offset for RGB2YCRCB core.
 * @{
 */
#define XRGB_ACTIVE_SIZE_OFFSET		0x020	/**< Active Size (V x H)
						  *  offset */
/*@}*/

/** @name Core Specific Registers
 *
 * Core specific registers offset.
 * @{
 */
#define XRGB_YMAX_OFFSET	0x100	/**< Luma Clipping offset */
#define XRGB_YMIN_OFFSET	0x104	/**< Luma Clamping offset */
#define XRGB_CBMAX_OFFSET	0x108	/**< Cb Clipping offset */
#define XRGB_CBMIN_OFFSET	0x10C	/**< Cb Clamping offset */
#define XRGB_CRMAX_OFFSET	0x110	/**< Cr Clipping offset */
#define XRGB_CRMIN_OFFSET	0x114	/**< Cr Clamping offset */
#define XRGB_YOFFSET_OFFSET	0x118	/**< Luma Offset offset */
#define XRGB_CBOFFSET_OFFSET	0x11C	/**< Cb Offset offset */
#define XRGB_CROFFSET_OFFSET	0x120	/**< Cr Offset offset */
#define XRGB_ACOEF_OFFSET	0x124	/**< A Coefficient offset */
#define XRGB_BCOEF_OFFSET	0x128	/**< B Coefficient offset */
#define XRGB_CCOEF_OFFSET	0x12C	/**< C Coefficient offset */
#define XRGB_DCOEF_OFFSET	0x130	/**< D Coefficient offset */
/*@}*/

/** @name Control Register Bit Masks
 *
 * Control Register bit definition for RGB2YCRCB core.
 * @{
 */
#define XRGB_CTL_SW_EN_MASK	0x00000001	/**< Software Enable Mask */
#define XRGB_CTL_RUE_MASK	0x00000002	/**< Register Update
						  *  Enable Mask */
#define XRGB_CTL_BPE_MASK	0x00000010	/**< Bypass Mask */
#define XRGB_CTL_TPE_MASK	0x00000020	/**< Test Pattern Mask */
#define XRGB_CTL_AUTORESET_MASK	0x40000000	/**< Software Reset -
						  *  Auto-synchronize
						  *  to SOF Mask */
#define XRGB_CTL_RESET_MASK	0x80000000	/**< Software Reset -
						  *  Instantaneous Mask */
/*@}*/

/** @name Slave Error Bit Masks
 * @{
 */
#define XRGB_ERR_EOL_EARLY_MASK	0x000000001	/**<Error: End of line Early
						  * Mask */
#define XRGB_ERR_EOL_LATE_MASK	0x000000002	/**<Error: End of line Late
						  * Mask */
#define XRGB_ERR_SOF_EARLY_MASK	0x000000004	/**<Error: Start of frame
						  * Early Mask */
#define XRGB_ERR_SOF_LATE_MASK	0x000000008	/**<Error: Start of frame
						  * Late Mask */
/*@}*/

/** @name Interrupt Register Bit Masks
 *
 * Interrupt Register bit definition for RGB2YCRCB core. It is applicable for
 * STATUS and IRQ_ENABLE Registers.
 * @{
 */
#define XRGB_IXR_PROC_STARTED_MASK	0x00000001	/**< Process Started
							  *  Mask */
#define XRGB_IXR_EOF_MASK		0x00000002	/**< End-Of-Frame
							  *  Mask */
#define XRGB_IXR_SE_MASK		0x00010000	/**< Slave Error
							  *  Mask */
#define XRGB_IXR_ALLINTR_MASK		0x00010003	/**< Interrupt All
							  *  Error Mask (ORing
							  *  (of All Interrupt
							  *  Mask) */
/*@}*/

/** @name Version Register Bit Masks and Shifts
 *
 * Version Register bit definition for RGB2YCRCB core.
 * @{
 */
#define XRGB_VER_REV_NUM_MASK	0x000000FF	/**< Revision Number Mask */
#define XRGB_VER_PID_MASK	0x00000F00	/**< Patch ID Mask */
#define XRGB_VER_REV_MASK	0x0000F000	/**< Revision Mask */
#define XRGB_VER_MINOR_MASK	0x00FF0000	/**< Minor Mask */
#define XRGB_VER_MAJOR_MASK	0xFF000000	/**< Major Mask */
#define XRGB_VER_MAJOR_SHIFT	24		/**< Major Shift */
#define XRGB_VER_MINOR_SHIFT	16		/**< Minor Shift */
#define XGMA_VER_REV_SHIFT	12		/**< Revision Shift */
#define XRGB_VER_INTERNAL_SHIFT	8		/**< Internal Shift */
/*@}*/

/** @name ActiveSize Register Bit Masks and Shift
 * @{
 */
#define XRGB_ACTSIZE_NUM_PIXEL_MASK	0x00001FFF /**< The number of pixels
						     *  in source image */
#define XRGB_ACTSIZE_NUM_LINE_MASK	0x1FFF0000 /**< The number of lines in
						     *  source image */
#define XRGB_ACTSIZE_NUM_LINE_SHIFT	16	    /**< Shift for  number of
						      *  lines */
/*@}*/

/** @name YMax Register Bit Mask
 * @{
 */
#define XRGB_YMAX_MASK	0x0000FFFF	/**< Luma clipping value Mask */
/*@}*/

/** @name YMin Register Bit Mask
 * @{
 */
#define XRGB_YMIN_MASK	0x0000FFFF	/**< Luma clamping value Mask */
/*@}*/

/** @name CBMax Register Bit Mask
 * @{
 */
#define XRGB_CBMAX_MASK	0x0000FFFF	/**< Chroma Cb clipping value Mask */
/*@}*/

/** @name CBMin Register Bit Mask
 * @{
 */
#define XRGB_CBMIN_MASK	0x0000FFFF	/**< Chroma Cb clamping value mask */
/*@}*/

/** @name CRMax Register Bit Mask
 * @{
 */
#define XRGB_CRMAX_MASK	0x0000FFFF	/**< Chroma Cr clipping value Mask */
/*@}*/

/** @name CRMin Register Bit Mask
 * @{
 */
#define XRGB_CRMIN_MASK	0x0000FFFF	/**< Chroma Cr clamping value Mask */
/*@}*/

/** @name YOffset Register Bit Mask
 * @{
 */
#define XRGB_YOFFSET_MASK	0x0001FFFF	/**< Luma offset compensation
						  * value Mask */
/*@}*/

/** @name CbOffset Register Bit Mask
 * @{
 */
#define XRGB_CBOFFSET_MASK	0x0001FFFF	/**< Chroma(Cb) offset
						  *  compensation value Mask */
/*@}*/

/** @name CrOffset Register Bit Mask
 * @{
 */
#define XRGB_CROFFSET_MASK	0x0001FFFF	/**< Chroma(Cr) offset
						  *  compensation value Mask */
/*@}*/

/** @name ACOEF, BCOEF, CCOEF, DCOEF Register Bit Mask
 * @{
 */
#define XRGB_COEFF_MASK		0x0001FFFF	/**< Matrix Conversion
						  *  Coefficient value mask */
/*@}*/

/** @name General purpose Bit Mask and Shifts
 * @{
 */
#define XRGB_8_BIT_MASK		0x000000FF	/**< 8-Bit mask */
#define XRGB_16_BIT_MASK	0x0000FFFF	/**< 16-Bit mask */
#define XRGB_16_BIT_COEF_SHIFT	16		/**< 16-Bit Coefficient
						  *  shift */
/*@}*/

/** @name Data widths in bits per color.
 * @{
 */
#define XRGB_DATA_WIDTH_8	8	/**< 8-bit Data Width */
#define XRGB_DATA_WIDTH_10	10	/**< 10-bit Data Width */
#define XRGB_DATA_WIDTH_12	12	/**< 12-bit Data Width */
#define XRGB_DATA_WIDTH_16	16	/**< 16-bit Data Width */
/*@}*/

/**@name Backward Compatibility Macros
 *
 * To support backward compatibility, following macro definitions are
 * re-defined.
 * @{
 */
#define RGB_CONTROL		XRGB_CONTROL_OFFSET
#define RGB_STATUS		XRGB_STATUS_OFFSET
#define RGB_ERROR		XRGB_ERROR_OFFSET
#define RGB_IRQ_EN		XRGB_IRQ_EN_OFFSET
#define RGB_VERSION		XRGB_VERSION_OFFSET
#define RGB_SYSDEBUG0		XRGB_SYSDEBUG0_OFFSET
#define RGB_SYSDEBUG1		XRGB_SYSDEBUG1_OFFSET
#define RGB_SYSDEBUG2		XRGB_SYSDEBUG2_OFFSET
#define RGB_ACTIVE_SIZE		XRGB_ACTIVE_SIZE_OFFSET
#define RGB_YMAX		XRGB_YMAX_OFFSET
#define RGB_YMIN		XRGB_YMIN_OFFSET
#define RGB_CBMAX		XRGB_CBMAX_OFFSET
#define RGB_CBMIN		XRGB_CBMIN_OFFSET
#define RGB_CRMAX		XRGB_CRMAX_OFFSET
#define RGB_CRMIN		XRGB_CRMIN_OFFSET
#define RGB_YOFFSET		XRGB_YOFFSET_OFFSET
#define RGB_CBOFFSET		XRGB_CBOFFSET_OFFSET
#define RGB_CROFFSET		XRGB_CROFFSET_OFFSET
#define RGB_ACOEF		XRGB_ACOEF_OFFSET
#define RGB_BCOEF		XRGB_BCOEF_OFFSET
#define RGB_CCOEF		XRGB_CCOEF_OFFSET
#define RGB_DCOEF		XRGB_DCOEF_OFFSET
#define RGB_CTL_EN_MASK		XRGB_CTL_EN_MASK
#define RGB_CTL_RUE_MASK	XRGB_CTL_RUE_MASK
#define RGB_RST_RESET		XRGB_CTL_RESET_MASK
#define RGB_RST_AUTORESET	XRGB_CTL_AUTORESET_MASK
#define RGB_In32		XRgb2YCrCb_In32
#define RGB_Out32		XRgb2YCrCb_Out32
/* @}*/

/** @name Interrupt Registers
 *
 * Interrupt status register generates a interrupt if the corresponding bits
 * of interrupt enable register bits are set.
 * @{
 */
#define XRGB_ISR_OFFSET	XRGB_STATUS_OFFSET	/**< Interrupt Status Offset */
#define XRGB_IER_OFFSET	XRGB_IRQ_EN_OFFSET	/**< Interrupt Enable Offset */
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

#define XRgb2YCrCb_In32		Xil_In32	/**< Input operation */
#define XRgb2YCrCb_Out32	Xil_Out32	/**< Output operation */

/*****************************************************************************/
/**
*
* This function macro reads the given register.
*
* @param	BaseAddress is the Xilinx base address of the RGB2YCRCB core.
* @param	RegOffset is the register offset of the register (defined at
*		top of this file).
*
* @return	32-bit value of the register.
*
* @note		C-style signature:
*		u32 XRgb2YCrCb_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XRgb2YCrCb_ReadReg(BaseAddress, RegOffset) \
		XRgb2YCrCb_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This function macro writes the given register.
*
* @param	BaseAddress is the Xilinx base address of the RGB2YCRCB core.
* @param	RegOffset is the register offset of the register (defined at
*		top of this file).
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XRgb2YCrCb_WriteReg(u32 BaseAddress, u32 RegOffset,
*						u32 Data)
*
******************************************************************************/
#define XRgb2YCrCb_WriteReg(BaseAddress, RegOffset, Data) \
		XRgb2YCrCb_Out32((BaseAddress) + (u32)(RegOffset),(u32)(Data))

/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
