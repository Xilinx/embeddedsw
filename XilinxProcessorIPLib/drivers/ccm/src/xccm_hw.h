/******************************************************************************
*
* (c) Copyright 2014 Xilinx, Inc. All rights reserved.
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
* @file xccm_hw.h
* @addtogroup ccm_v6_0
* @{
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx Color Correction Matrix (CCM)
* core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- -------------------------------------------------------
* 6.0   adk     03/06/14 First release.
*                        Added the register offsets and bit masks for the
*                        registers.
*                        Added backward compatibility macros.
* </pre>
*
******************************************************************************/

#ifndef XCCM_HW_H_
#define XCCM_HW_H_	/**< Prevent circular inclusions
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
#define XCCM_CONTROL_OFFSET	0x000	/**< Control offset */
#define XCCM_STATUS_OFFSET	0x004	/**< Status offset */
#define XCCM_ERROR_OFFSET	0x008	/**< Error offset */
#define XCCM_IRQ_EN_OFFSET	0x00C	/**< IRQ Enable offset */
#define XCCM_VERSION_OFFSET	0x010	/**< Version offset */
#define XCCM_SYSDEBUG0_OFFSET	0x014	/**< System Debug 0 offset */
#define XCCM_SYSDEBUG1_OFFSET	0x018	/**< System Debug 1 offset */
#define XCCM_SYSDEBUG2_OFFSET	0x01C	/**< System Debug 2 offset */
#define XCCM_ACTIVE_SIZE_OFFSET	0x020	/**< Active Size (V x H) offset */

#define XCCM_K11_OFFSET		0x100	/**< K11 Coefficient offset */
#define XCCM_K12_OFFSET		0x104	/**< K12 Coefficient offset */
#define XCCM_K13_OFFSET		0x108	/**< K13 Coefficient offset */
#define XCCM_K21_OFFSET		0x10C	/**< K21 Coefficient offset */
#define XCCM_K22_OFFSET		0x110	/**< K22 Coefficient offset */
#define XCCM_K23_OFFSET		0x114	/**< K23 Coefficient offset */
#define XCCM_K31_OFFSET		0x118	/**< K31 Coefficient offset */
#define XCCM_K32_OFFSET		0x11C	/**< K32 Coefficient offset */
#define XCCM_K33_OFFSET		0x120	/**< K33 Coefficient offset */
#define XCCM_ROFFSET_OFFSET	0x124	/**< Red Offset offset */
#define XCCM_GOFFSET_OFFSET	0x128	/**< Green Offset offset */
#define XCCM_BOFFSET_OFFSET	0x12C	/**< Blue Offset offset */
#define XCCM_CLIP_OFFSET	0x130	/**< Clip Offset offset */
#define XCCM_CLAMP_OFFSET	0x134	/**< Clamp Offset offset */
/*@}*/

/** @name Control register bit masks
 * @{
 */
#define XCCM_CTL_SW_EN_MASK	0x00000001	/**< Enable mask */
#define XCCM_CTL_RUE_MASK	0x00000002	/**< Register Update Enable
						  *  mask */
#define XCCM_CTL_BPE_MASK	0x00000010	/**< Bypass Enable mask */
#define XCCM_CTL_TPE_MASK	0x00000020	/**< Test Pattern Enable
						  *  mask */
#define XCCM_CTL_AUTORESET_MASK	0x40000000	/**< Software Auto Reset
						  *  mask */
#define XCCM_CTL_RESET_MASK	0x80000000	/**< Software Reset mask */
/*@}*/

/** @name Interrupt register bit masks. It is applicable for
 *	  Status and IRQ_ENABLE Registers
 * @{
 */
#define XCCM_IXR_PROCS_STARTED_MASK 0x00000001	/**< Process Started mask */
#define XCCM_IXR_EOF_MASK	0x00000002	/**< End-Of-Frame mask */
#define XCCM_IXR_SE_MASK	0x00010000	/**< Slave Error mask */
#define XCCM_IXR_ALLINTR_MASK	0x00010003	/**< Interrupt all error mask
						  *  (ORing of all interrupt
						  *  mask) */
/*@}*/

/** @name Error register bit masks
 * @{
 */
#define	XCCM_ERR_EOL_EARLY_MASK	0x00000001	/**< End of Line Early mask */
#define XCCM_ERR_EOL_LATE_MASK 	0x00000002	/**< End of Line Late mask */
#define XCCM_ERR_SOF_EARLY_MASK	0x00000004	/**< Start of Frame Early
						  *  mask */
#define XCCM_ERR_SOF_LATE_MASK	0x00000008	/**< Start of Frame Late
						  *  mask */
/*@}*/

/** @name Version register bit masks and shifts
 * @{
 */
#define XCCM_VER_REV_NUM_MASK	0x000000FF	/**< Version Revision Number
						  *  mask */
#define XCCM_VER_PID_MASK	0x00000F00	/**< Version Patch ID mask */
#define XCCM_VER_REV_MASK	0x0000F000	/**< Version Revision mask */
#define XCCM_VER_MINOR_MASK	0x00FF0000	/**< Version Minor mask */
#define XCCM_VER_MAJOR_MASK	0xFF000000	/**< Version Major mask */
#define XCCM_VER_INTERNAL_SHIFT	0x00000008	/**< Version Internal shift */
#define XCCM_VER_REV_SHIFT	0x0000000C	/**< Version Revision shift */
#define XCCM_VER_MINOR_SHIFT	0x00000010	/**< Version Minor shift */
#define XCCM_VER_MAJOR_SHIFT	0x00000018	/**< Version Major shift */
/*@}*/

/** @name Active Size register masks and shift
 * @{
 */
#define XCCM_ACTSIZE_NUM_PIXEL_MASK 0x00001FFF /**< Number of Active pixels
						  *  per scan line (horizontal)
						  *  mask */
#define XCCM_ACTSIZE_NUM_LINE_MASK  0x1FFF0000 	/**< Number of Active lines per
						  *  frame (vertical) mask */
#define XCCM_ACTSIZE_NUM_LINE_SHIFT 16		/**< Shift for number of
						  *  lines */
/*@}*/

/** @name Matrix coefficient masks and shifts
 * @{
 */
#define XCCM_COEF_MASK		0x0003FFFF	/**< Matrix Coefficient mask */
#define XCCM_COEF_DECI_MASK	0x0001C000	/**< Mask of Decimal part */
#define XCCM_COEFF_FRAC_MASK	0x00003FFF 	/**< Mask of Fractional part */
#define XCCM_COEF_SHIFT		14		/**< Coefficient shift */
#define XCCM_COEF_SIGN_MASK	0x20000		/**< Mask for sign bit */

/*@}*/

/** @name Offsets masks and shifts
 * @{
 */
#define XCCM_OFFSET_MASK	0x0001FFFF	/**< Offset mask for Red, Green
						  *  Blue Offset registers */
#define XCCM_OFFSET_SIGN_SHIFT	15		/**< Shift for signed bit */
/*@}*/

/** @name Clip and Clamp masks
 * @{
 */
#define XCCM_CLIP_MASK		0x0000FFFF	/**< Clip register mask */
#define XCCM_CLAMP_MASK		0x0000FFFF	/**< Clamp register mask */
/*@}*/

/** @name General purpose macros
 * @{
 */
#define XCCM_SIGN_MUL		-1		/**< Macro for sign
						  *  multiplication */
#define XCCM_MAX_VALUE		0xFFFFFFFF	/**< 32 bit maximum value */
#define XCCM_SIGNBIT_MASK	0x10000000	/** Mask for sign bit of 32
						  *  bit number */
/*@}*/

/** @name Macros for backward compatibility
 * @{
 */
#define CCM_CONTROL		XCCM_CONTROL_OFFSET
#define CCM_STATUS		XCCM_STATUS_OFFSET
#define CCM_ERROR		XCCM_ERROR_OFFSET
#define CCM_IRQ_EN		XCCM_IRQ_EN_OFFSET
#define CCM_VERSION		XCCM_VERSION_OFFSET
#define CCM_SYSDEBUG0		XCCM_SYSDEBUG0_OFFSET
#define CCM_SYSDEBUG1		XCCM_SYSDEBUG1_OFFSET
#define CCM_SYSDEBUG2		XCCM_SYSDEBUG2_OFFSET
#define CCM_ACTIVE_SIZE		XCCM_ACTIVE_SIZE_OFFSET
#define CCM_K11			XCCM_K11_OFFSET
#define CCM_K12			XCCM_K12_OFFSET
#define CCM_K13			XCCM_K13_OFFSET
#define CCM_K21			XCCM_K21_OFFSET
#define CCM_K22			XCCM_K22_OFFSET
#define CCM_K23			XCCM_K23_OFFSET
#define CCM_K31			XCCM_K31_OFFSET
#define CCM_K32			XCCM_K32_OFFSET
#define CCM_K33			XCCM_K33_OFFSET
#define CCM_ROFFSET		XCCM_ROFFSET_OFFSET
#define CCM_GOFFSET		XCCM_GOFFSET_OFFSET
#define CCM_BOFFSET		XCCM_BOFFSET_OFFSET
#define CCM_CLIP		XCCM_CLIP_OFFSET
#define CCM_CLAMP		XCCM_CLAMP_OFFSET
#define CCM_CTL_EN_MASK		XCCM_CTL_SW_EN_MASK
#define CCM_CTL_RUE_MASK	XCCM_CTL_RUE_MASK
#define CCM_RST_RESET		XCCM_CTL_RESET_MASK
#define CCM_RST_AUTORESET	XCCM_CTL_AUTORESET_MASK
#define CCM_In32		XCcm_In32
#define CCM_Out32		XCcm_Out32
#define XCCM_ReadReg		XCcm_ReadReg
#define XCCM_WriteReg		XCcm_WriteReg
/*@}*/

/** @name Interrupt Enable and Status Registers Offsets
 * @{
 */
/**
* Interrupt status register generates a interrupt if the corresponding bits of
* interrupt enable register bits are set.
*/
#define XCCM_ISR_OFFSET		XCCM_STATUS_OFFSET
					/**< Interrupt status offset */
#define XCCM_IER_OFFSET		XCCM_IRQ_EN_OFFSET
					/**< Interrupt enable Offset */
/*@}*/
/***************** Macros (Inline Functions) Definitions *********************/

#define XCcm_In32		Xil_In32	/**< Input operation */
#define XCcm_Out32		Xil_Out32	/**< Output operation */

/*****************************************************************************/
/**
*
* This macro reads the given register.
*
* @param	BaseAddress is the Xilinx base address of the CCM core.
* @param	RegOffset is the register offset of the register.
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XCcm_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XCcm_ReadReg(BaseAddress, RegOffset) \
	XCcm_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This macro writes the given register.
*
* @param	BaseAddress is the Xilinx base address of the CCM core.
* @param	RegOffset is the register offset of the register.
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XCcm_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XCcm_WriteReg(BaseAddress, RegOffset, Data) \
	XCcm_Out32((BaseAddress) + (u32)(RegOffset), (u32)(Data))

/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}

#endif

#endif /* End of protection macro */
/** @} */
