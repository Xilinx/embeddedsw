/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xycrcb2rgb_hw.h
* @addtogroup ycrcb2rgb_v7_2
* @{
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx YCrCb to RGB Color Space
* Converter (YCRCB2RGB) core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 7.0   adk    01/31/14 First release.
*                       Added the register offsets and bit masks for the
*                       registers.
*                       Added backward compatibility macros.
* </pre>
*
******************************************************************************/

#ifndef XYCRCB2RGB_HW_H_
#define XYCRCB2RGB_HW_H_	/**< Prevent circular inclusions by using
				  *  protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Control Registers
 *
 * Control Registers offset for YCRCB2RGB core.
 * @{
 */
#define XYCC_CONTROL_OFFSET	0x000	/**< Control offset */
#define XYCC_STATUS_OFFSET	0x004	/**< Status offset */
#define XYCC_ERROR_OFFSET	0x008	/**< Error offset */
#define XYCC_IRQ_EN_OFFSET	0x00C	/**< IRQ Enable offset */
#define XYCC_VERSION_OFFSET	0x010	/**< Version offset */
#define XYCC_SYSDEBUG0_OFFSET	0x014	/**< System Debug 0 offset */
#define XYCC_SYSDEBUG1_OFFSET	0x018	/**< System Debug 1 offset */
#define XYCC_SYSDEBUG2_OFFSET	0x01C	/**< System Debug 2 offset */
/*@}*/

/** @name Timing Control Registers
 *
 * Timing control registers offset for YCRCB2RGB core.
 * @{
 */
#define XYCC_ACTIVE_SIZE_OFFSET		0x020	/**< Active Size (V x H)
						  *  offset */
/*@}*/

/** @name Core Specific Registers
 *
 * Core specific registers offset
 * @{
 */
#define XYCC_RGBMAX_OFFSET	0x100	/**< RGB Clipping offset */
#define XYCC_RGBMIN_OFFSET	0x104	/**< RGB Clamping offset */
#define XYCC_ROFFSET_OFFSET	0x108	/**< R Offset offset */
#define XYCC_GOFFSET_OFFSET	0x10C	/**< G Offset offset */
#define XYCC_BOFFSET_OFFSET	0x110	/**< B Offset offset */
#define XYCC_ACOEF_OFFSET	0x114	/**< A Coefficient offset */
#define XYCC_BCOEF_OFFSET	0x118	/**< B Coefficient offset */
#define XYCC_CCOEF_OFFSET	0x11C	/**< C Coefficient offset */
#define XYCC_DCOEF_OFFSET	0x120	/**< D Coefficient offset */
/*@}*/

/** @name Control Register Bit Masks
 *
 * Control Register bit definition for YCRCB2RGB core.
 * @{
 */
#define XYCC_CTL_SW_EN_MASK	0x00000001	/**< Software Enable Mask */
#define XYCC_CTL_RUE_MASK	0x00000002	/**< Register Update
						  *  Enable Mask */
#define XYCC_CTL_BPE_MASK	0x00000010	/**< Bypass Mask */
#define XYCC_CTL_TPE_MASK	0x00000020	/**< Test Pattern Mask */
#define XYCC_CTL_AUTORESET_MASK	0x40000000	/**< Software Reset -
						  *  Auto-synchronize
						  *  to SOF Mask */
#define XYCC_CTL_RESET_MASK	0x80000000	/**< Software Reset -
						  *  Instantaneous Mask */
/*@}*/

/** @name Slave Error Bit Masks
 * @{
 */
#define XYCC_ERR_EOL_EARLY_MASK	0x000000001	/**< Error: End of line
						  *  Early Mask */
#define XYCC_ERR_EOL_LATE_MASK	0x000000002	/**< Error: End of line
						  *  Late Mask */
#define XYCC_ERR_SOF_EARLY_MASK	0x000000004	/**< Error: Start of frame
						  *  Early Mask */
#define XYCC_ERR_SOF_LATE_MASK	0x000000008	/**< Error: Start of frame
						  *  Late Mask */
/*@}*/

/** @name Interrupt Register Bit Masks
 *
 * Interrupt Register bit definition for YCRCB2RGB core. It is applicable for
 * STATUS and IRQ_ENABLE Registers.
 * @{
 */
#define XYCC_IXR_PROCS_MASK	0x00000001	/**< Process Started Mask */
#define XYCC_IXR_EOF_MASK	0x00000002	/**< End-Of-Frame Mask */
#define XYCC_IXR_SE_MASK	0x00010000	/**< Slave Error Mask */
#define XYCC_IXR_ALLINTR_MASK	0x00010003	/**< Interrupt All
						  *  Error Mask (ORing
						  *  (of All Interrupt
						  *  Mask) */
/*@}*/

/** @name Version Register Bit Masks and Shifts
 * @{
 */
#define XYCC_VER_REV_NUM_MASK	0x000000FF	/**< Revision Number Mask */
#define XYCC_VER_PID_MASK	0x00000F00	/**< Patch ID Mask */
#define XYCC_VER_REV_MASK	0x0000F000	/**< Revision Mask */
#define XYCC_VER_MINOR_MASK	0x00FF0000	/**< Minor Mask */
#define XYCC_VER_MAJOR_MASK	0xFF000000	/**< Major Mask */
#define XYCC_VER_MAJOR_SHIFT	24		/**< Major Shift */
#define XYCC_VER_MINOR_SHIFT	16		/**< Minor Shift */
#define XYCC_VER_REV_SHIFT	12		/**< Revision Shift */
#define XYCC_VER_INTERNAL_SHIFT	8		/**< Internal Shift */
/*@}*/

/** @name Active Size Register Bit Masks and Shifts
 * @{
 */
#define XYCC_ACTSIZE_NUM_PIXEL_MASK	0x00001FFF /**< The number of pixels
						     *  in source image */
#define XYCC_ACTSIZE_NUM_LINE_MASK	0x1FFF0000 /**< The number of lines in
						     *  source image */
#define XYCC_ACTSIZE_NUM_LINE_SHIFT	16	    /**< Shift for number
						      *  of lines */
/*@}*/

/** @name RGBMAX Register Bit Mask
 * @{
 */
#define XYCC_RGBMAX_MASK	0x0000FFFF	/**< Clipping Mask */
/*@}*/

/** @name RGBMIN Register Bit Mask
 * @{
 */
#define XYCC_RGBMIN_MASK	0x0000FFFF	/**< Clamping Mask */
/*@}*/

/** @name ROFFSET Register Bit Mask
 * @{
 */
#define XYCC_ROFFSET_MASK	0xFFFFFFFF	/**< Red offset compensation
						  *  Mask */
/*@}*/

/** @name GOFFSET Register Bit Mask
 * @{
 */
#define XYCC_GOFFSET_MASK	0xFFFFFFFF	/**< Green offset compensation
						  *  Mask */
/*@}*/

/** @name BOFFSET Register Bit Mask
 * @{
 */
#define XYCC_BOFFSET_MASK	0xFFFFFFFF	/**< Blue offset compensation
						  *  Mask */
/*@}*/

/** @name A ,B, C , D Coefficient Register Bit Mask
 * @{
 */
#define XYCC_COEF_MASK		0x0003FFFF	/**< Coefficients Mask */
/*@}*/

/** @name General purpose Bit Mask and Shifts
 * @{
 */
#define XYCC_8_BIT_MASK		0x000000FF	/**< 8-bit Mask	*/
#define XYCC_16_BIT_MASK	0x0000FFFF	/**< 16-Bit Mask */
#define XYCC_16_BIT_COEF_SHIFT	16		/**< 16-Bit Coefficient
						  *  Shift */
/*@}*/

/** @name Data widths in bits per color.
 * @{
 */
#define XYCC_DATA_WIDTH_8	8	/**< 8-bit Data Width. */
#define XYCC_DATA_WIDTH_10	10	/**< 10-bit Data Width. */
#define XYCC_DATA_WIDTH_12	12	/**< 12-bit Data Width. */
#define XYCC_DATA_WIDTH_16	16	/**< 16-bit Data Width. */
/*@}*/

/** @name Backward Compatibility Macros
 *
 * To support backward compatibility, following macros definition are
 * re-defined.
 * @{
 */
#define YCC_CONTROL		XYCC_CONTROL_OFFSET
#define YCC_STATUS		XYCC_STATUS_OFFSET
#define YCC_ERROR		XYCC_ERROR_OFFSET
#define YCC_IRQ_EN		XYCC_IRQ_EN_OFFSET
#define YCC_VERSION		XYCC_VERSION_OFFSET
#define YCC_SYSDEBUG0		XYCC_SYSDEBUG0_OFFSET
#define YCC_SYSDEBUG1 		XYCC_SYSDEBUG1_OFFSET
#define YCC_SYSDEBUG2		XYCC_SYSDEBUG2_OFFSET
#define YCC_ACTIVE_SIZE		XYCC_ACTIVE_SIZE_OFFSET
#define XYCC_RGBMAX			XYCC_RGBMAX_OFFSET
#define XYCC_RGBMIN			XYCC_RGBMIN_OFFSET
#define XYCC_ROFFSET		XYCC_ROFFSET_OFFSET
#define XYCC_GOFFSET		XYCC_GOFFSET_OFFSET
#define XYCC_BOFFSET		XYCC_BOFFSET_OFFSET
#define XYCC_ACOEF			XYCC_ACOEF_OFFSET
#define XYCC_BCOEF			XYCC_BCOEF_OFFSET
#define XYCC_CCOEF			XYCC_CCOEF_OFFSET
#define XYCC_DCOEF			XYCC_DCOEF_OFFSET

#define YCC_CTL_EN_MASK		XYCC_CTL_EN_MASK
#define YCC_CTL_RUE_MASK 	XYCC_CTL_RUE_MASK
#define YCC_RST_RESET		XYCC_CTL_RESET_MASK
#define YCC_RST_AUTORESET	XYCC_CTL_AUTORESET_MASK

#define YCC_In32		XYCrCb2Rgb_In32
#define YCC_Out32		XYCrCb2Rgb_Out32

#define XYCC_ReadReg		XYCrCb2Rgb_ReadReg
#define XYCC_WriteReg		XYCrCb2Rgb_WriteReg

/*@}*/

/** @name Interrupt Registers
 *
 * Interrupt status register generates a interrupt if the corresponding bits
 * of interrupt enable register bits are set.
 * @{
 */
#define XYCC_ISR_OFFSET		XYCC_STATUS_OFFSET	/**< Interrupt Status
							  *  Offset */
#define XYCC_IER_OFFSET		XYCC_IRQ_EN_OFFSET	/**< Interrupt Enable
							  *  Offset */
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

#define XYCrCb2Rgb_In32		Xil_In32	/**< Input operation */
#define XYCrCb2Rgb_Out32	Xil_Out32	/**< Output operation */

/*****************************************************************************/
/**
*
* This function macro reads the given register.
*
* @param	BaseAddress is the base address of the YCRCB2RGB core.
* @param	RegOffset is the register offset of the register defined at
*		top of this file.
*
* @return	32-bit value of the register.
*
* @note		C-style signature:
*		u32 XYCrCb2Rgb_ReadReg(u32 BaseAddress, u32 RegOffset).
*
******************************************************************************/
#define XYCrCb2Rgb_ReadReg(BaseAddress, RegOffset) \
	XYCrCb2Rgb_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This function macro writes the given register.
*
* @param	BaseAddress is base address of the YCRCB2RGB core.
* @param	RegOffset is the register offset of the register (defined at
*		top of this file).
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XYCrCb2Rgb_WriteReg(u32 BaseAddress, u32 RegOffset,
*						u32 Data)
*
******************************************************************************/
#define XYCrCb2Rgb_WriteReg(BaseAddress, RegOffset, Data) \
	XYCrCb2Rgb_Out32((BaseAddress) + (u32)(RegOffset), (Data))

/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif	/* End of protection macro */
/** @} */
