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
* @file xcresample_hw.h
* @addtogroup cresample_v4_1
* @{
*
* This header file contains identifiers and register-level driver functions
* (or macros) that can be used to access the Xilinx Chroma Resampler core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- -------------------------------------------------------
* 4.0   adk     03/12/14 First release
*                        Added the register offsets and bit masks for the
*                        registers and added backward compatibility for macros.
*
* </pre>
*
******************************************************************************/

#ifndef XCRESAMPLE_HW_H_
#define XCRESAMPLE_HW_H_	/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name register offsets
 * @{
 */
/* General control registers */
#define XCRE_CONTROL_OFFSET		0x0000	/**< Control */
#define XCRE_STATUS_OFFSET		0x0004	/**< Status */
#define XCRE_ERROR_OFFSET		0x0008	/**< Error */
#define XCRE_IRQ_EN_OFFSET		0x000C	/**< IRQ enable */
#define XCRE_VERSION_OFFSET		0x0010	/**< Version */
#define XCRE_SYSDEBUG0_OFFSET		0x0014	/**< System debug 0 */
#define XCRE_SYSDEBUG1_OFFSET		0x0018	/**< System debug 1 */
#define XCRE_SYSDEBUG2_OFFSET		0x001C	/**< System debug 2 */
/*@}*/

/* Timing control registers */
#define XCRE_ACTIVE_SIZE_OFFSET		0x0020	/**< Horizontal and vertical
						  *  active frame size */
#define XCRE_ENCODING_OFFSET		0x0028	/**< Frame encoding */

/* Core specific registers */
/* Coefficient Registers for Horizontal Filter Phase 0 */
#define XCRE_COEF00_HPHASE0_OFFSET	0x0100	/**< Coefficient 00 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF01_HPHASE0_OFFSET	0x0104	/**< Coefficient 01 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF02_HPHASE0_OFFSET	0x0108  /**< Coefficient 02 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF03_HPHASE0_OFFSET	0x010C	/**< Coefficient 03 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF04_HPHASE0_OFFSET	0x0110	/**< Coefficient 04 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF05_HPHASE0_OFFSET	0x0114	/**< Coefficient 05 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF06_HPHASE0_OFFSET	0x0118	/**< Coefficient 06 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF07_HPHASE0_OFFSET	0x011C	/**< Coefficient 07 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF08_HPHASE0_OFFSET	0x0120	/**< Coefficient 08 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF09_HPHASE0_OFFSET	0x0124	/**< Coefficient 09 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF10_HPHASE0_OFFSET	0x0128	/**< Coefficient 10 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF11_HPHASE0_OFFSET	0x012C	/**< Coefficient 11 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF12_HPHASE0_OFFSET	0x0130	/**< Coefficient 12 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF13_HPHASE0_OFFSET	0x0134	/**< Coefficient 13 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF14_HPHASE0_OFFSET	0x0138	/**< Coefficient 14 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF15_HPHASE0_OFFSET	0x013C	/**< Coefficient 15 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF16_HPHASE0_OFFSET	0x0140	/**< Coefficient 16 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF17_HPHASE0_OFFSET	0x0144	/**< Coefficient 17 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF18_HPHASE0_OFFSET	0x0148	/**< Coefficient 18 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF19_HPHASE0_OFFSET	0x014C	/**< Coefficient 19 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF20_HPHASE0_OFFSET	0x0150	/**< Coefficient 20 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF21_HPHASE0_OFFSET	0x0154	/**< Coefficient 21 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF22_HPHASE0_OFFSET	0x0158	/**< Coefficient 22 of
						  *  horizontal phase 0
						  *  filter */
#define XCRE_COEF23_HPHASE0_OFFSET	0x015C	/**< Coefficient 23 of
						  *  horizontal phase 0
						  *  filter */
/* Coefficient Registers for Horizontal Filter Phase 1 */
#define XCRE_COEF00_HPHASE1_OFFSET	0x0160	/**< Coefficient 00 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF01_HPHASE1_OFFSET	0x0164	/**< Coefficient 01 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF02_HPHASE1_OFFSET	0x0168	/**< Coefficient 02 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF03_HPHASE1_OFFSET	0x016C	/**< Coefficient 03 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF04_HPHASE1_OFFSET	0x0170	/**< Coefficient 04 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF05_HPHASE1_OFFSET	0x0174	/**< Coefficient 05 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF06_HPHASE1_OFFSET	0x0178	/**< Coefficient 06 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF07_HPHASE1_OFFSET	0x017C	/**< Coefficient 07 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF08_HPHASE1_OFFSET	0x0180	/**< Coefficient 08 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF09_HPHASE1_OFFSET	0x0184	/**< Coefficient 09 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF10_HPHASE1_OFFSET	0x0188	/**< Coefficient 10 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF11_HPHASE1_OFFSET	0x018C	/**< Coefficient 11 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF12_HPHASE1_OFFSET	0x0190	/**< Coefficient 12 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF13_HPHASE1_OFFSET	0x0194	/**< Coefficient 13 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF14_HPHASE1_OFFSET	0x0198	/**< Coefficient 14 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF15_HPHASE1_OFFSET	0x019C	/**< Coefficient 15 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF16_HPHASE1_OFFSET	0x01A0	/**< Coefficient 16 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF17_HPHASE1_OFFSET	0x01A4	/**< Coefficient 17 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF18_HPHASE1_OFFSET	0x01A8	/**< Coefficient 18 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF19_HPHASE1_OFFSET	0x01AC	/**< Coefficient 19 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF20_HPHASE1_OFFSET	0x01B0	/**< Coefficient 20 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF21_HPHASE1_OFFSET	0x01B4	/**< Coefficient 21 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF22_HPHASE1_OFFSET	0x01B8	/**< Coefficient 22 of
						  *  horizontal phase 1
						  *  filter */
#define XCRE_COEF23_HPHASE1_OFFSET	0x01BC	/**< Coefficient 23 of
						  *  horizontal phase 1
						  *  filter */
/* Coefficient Registers for Vertical Filter Phase 0 */
#define XCRE_COEF00_VPHASE0_OFFSET	0x01C0	/**< Coefficient 00 of
						  *  vertical phase 0
						  *  filter */
#define XCRE_COEF01_VPHASE0_OFFSET	0x01C4	/**< Coefficient 01 of
						  *  vertical phase 0
						  *  filter */
#define XCRE_COEF02_VPHASE0_OFFSET	0x01C8	/**< Coefficient 02 of
						  *  vertical phase 0
						  *  filter */
#define XCRE_COEF03_VPHASE0_OFFSET	0x01CC	/**< Coefficient 03 of
						  *  vertical phase 0
						  *  filter */
#define XCRE_COEF04_VPHASE0_OFFSET	0x01D0	/**< Coefficient 04 of
						  *  vertical phase 0
						  *  filter */
#define XCRE_COEF05_VPHASE0_OFFSET	0x01D4	/**< Coefficient 05 of
						  *  vertical phase 0
						  *  filter */
#define XCRE_COEF06_VPHASE0_OFFSET	0x01D8	/**< Coefficient 06 of
						  *  vertical phase 0
						  *  filter */
#define XCRE_COEF07_VPHASE0_OFFSET	0x01DC	/**< Coefficient 07 of
						  *  vertical phase 0
						  *  filter */
/* Coefficient Registers for Vertical Filter Phase 1 */
#define XCRE_COEF00_VPHASE1_OFFSET	0x01E0	/**< Coefficient 00 of
						  *  vertical phase 1
						  *  filter */
#define XCRE_COEF01_VPHASE1_OFFSET	0x01E4	/**< Coefficient 01 of
						  *  vertical phase 1
						  *  filter */
#define XCRE_COEF02_VPHASE1_OFFSET	0x01E8	/**< Coefficient 02 of
						  *  vertical phase 1
						  *  filter */
#define XCRE_COEF03_VPHASE1_OFFSET	0x01EC	/**< Coefficient 03 of
						  *  vertical phase 1
						  *  filter */
#define XCRE_COEF04_VPHASE1_OFFSET	0x01F0  /**< Coefficient 04 of
						  *  vertical phase 1
						  *  filter */
#define XCRE_COEF05_VPHASE1_OFFSET	0x01F4	/**< Coefficient 05 of
						  *  vertical phase 1
						  *  filter */
#define XCRE_COEF06_VPHASE1_OFFSET	0x01F8	/**< Coefficient 06 of
						  *  vertical phase 1
						  *  filter */
#define XCRE_COEF07_VPHASE1_OFFSET	0x01FC	/**< Coefficient 07 of
						  *  vertical phase 1
						  *  filter */
/*@}*/

/** @name Control register bit masks
 * @{
 */
#define XCRE_CTL_SW_EN_MASK	0x00000001	/**< Enable mask */
#define XCRE_CTL_RUE_MASK	0x00000002	/**< Register update mask */
#define XCRE_CTL_BPE_MASK	0x00000010	/**< Bypass mask */
#define XCRE_CTL_TPE_MASK	0x00000020	/**< Test pattern mask */
#define XCRE_CTL_AUTORESET_MASK	0x40000000	/**< Software reset -
						  *  Auto-synchronize to SOF
						  *  mask */
#define XCRE_CTL_RESET_MASK	0x80000000	/**< Software reset -
						  *  instantaneous mask */
/*@}*/

/** @name Interrupt register bit masks. It is applicable for
 * Status and IRQ_ENABLE Registers
 * @{
 */
#define XCRE_IXR_PROCS_STARTED_MASK 0x00000001	/**< Proc started mask */
#define XCRE_IXR_EOF_MASK	0x00000002	/**< End-Of-Frame mask */
#define XCRE_IXR_SE_MASK	0x00010000	/**< Slave Error mask */
#define XCRE_IXR_ALLINTR_MASK	0x00010003U	/**< OR of all mask */
		/* ((XCRE_IXR_PROCS_MASK) | (XCRE_IXR_EOF_MASK) | \
		 * (XCRE_IXR_SE_MASK))	*/

/*@}*/

/** @name Error register bit masks
 * @{
 */
#define XCRE_ERR_EOL_EARLY_MASK	0x00000001	/**< Error: End of Line
						  *  Early mask */
#define XCRE_ERR_EOL_LATE_MASK	0x00000002	/**< Error: End of Line
						  *  Late mask */
#define XCRE_ERR_SOF_EARLY_MASK	0x00000004	/**< Error: Start of
						  *  Frame Early mask */
#define XCRE_ERR_SOF_LATE_MASK	0x00000008	/**< Error: Start of
						  *  Frame Late mask */
/*@}*/

/** @name Version register bit masks and shifts
 * @{
 */
#define XCRE_VER_REV_NUM_MASK	0x000000FF	/**< Revision Number mask */
#define XCRE_VER_PID_MASK	0x00000F00	/**< Patch ID mask */
#define XCRE_VER_REV_MASK	0x0000F000	/**< Version Revision mask */
#define XCRE_VER_MINOR_MASK	0x00FF0000	/**< Version Minor mask */
#define XCRE_VER_MAJOR_MASK	0xFF000000	/**< Version Major mask */

#define XCRE_VER_INTERNAL_SHIFT	0x00000008	/**< Patch ID shift */
#define XCRE_VER_REV_SHIFT	0x0000000C	/**< Version Revision shift */
#define XCRE_VER_MINOR_SHIFT	0x00000010	/**< Version Minor shift */
#define XCRE_VER_MAJOR_SHIFT	0x00000018	/**< Version Major shift */

/*@}*/

/** @name Active size register bit masks and shifts
 * @{
 */
#define XCRE_ACTSIZE_NUM_PIXEL_MASK 0x00001FFF	/**< Number of Active pixels
						  *  per scan line (horizontal)
						  *  mask */
#define XCRE_ACTSIZE_NUM_LINE_MASK 0x1FFF0000	/**< Number of Active lines per
						  *  frame (Vertical) mask */
#define XCRE_ACTSIZE_NUM_LINE_SHIFT 16		/**< Shift for number of
						  *  lines */
/*@}*/

/** @name Encoding register bit masks and shifts
 * @{
 */
#define XCRE_ENCODING_FIELD_MASK  0x00000080	/**< Field parity mask */
#define XCRE_ENCODING_CHROMA_MASK 0x00000100	/**< Chroma parity mask */
#define XCRE_ENCODING_FIELD_SHIFT	7	/**< Field parity shift */
#define XCRE_ENCODING_CHROMA_SHIFT	8	/**< Chroma parity shift */
/*@}*/

/** @name Coefficient bit mask and shift
 * @{
 */
#define XCRE_COEFF_FRAC_MASK	0x00003FFF 	/**< Mask of Fractional part */
#define XCRE_COEF_DECI_MASK	0x00004000	/**< Mask of Decimal part */
#define XCRE_COEF_SIGN_MASK	0x00008000	/**< Mask for Coefficient sign
						  *  bit */
#define XCRE_COEFF_MASK		0x0000FFFF	/**< Coefficient mask */
#define XCRE_COEFF_SHIFT 	14		/**< Shift for decimal value */
#define XCRE_COEFF_SIGN_SHIFT	16		/**< Coefficient shift */

/*@}*/

/** @name General purpose macros
 * @{
 */
#define XCRE_SIGN_MUL		-1		/**< Macro for sign
						  *  multiplication */
#define XCRE_SIGNBIT_MASK	0x10000000	/** Mask for sign bit of 32
						  *  bit number */
#define XCRE_MAX_VALUE		0xFFFFFFFF	/**< 32 bit maximum value */

/*@}*/
/** @name backward compatibility macros
 *
 * To support backward compatibility following macro definition are
 * re-defined.
 * @{
 */
#define CRESAMPLE_CONTROL		XCRE_CONTROL_OFFSET
#define CRESAMPLE_STATUS		XCRE_STATUS_OFFSET
#define CRESAMPLE_ERROR			XCRE_ERROR_OFFSET
#define CRESAMPLE_IRQ_ENABLE		XCRE_IRQ_EN_OFFSET
#define CRESAMPLE_VERSION		XCRE_VERSION_OFFSET
#define CRESAMPLE_SYSDEBUG0		XCRE_SYSDEBUG0_OFFSET
#define CRESAMPLE_SYSDEBUG1		XCRE_SYSDEBUG1_OFFSET
#define CRESAMPLE_SYSDEBUG2		XCRE_SYSDEBUG2_OFFSET

#define CRESAMPLE_ACTIVE_SIZE		XCRE_ACTIVE_SIZE_OFFSET
#define CRESAMPLE_ENCODING		XCRE_ENCODING_OFFSET

#define CRESAMPLE_COEF00_HPHASE0	XCRE_COEF00_HPHASE0_OFFSET
#define CRESAMPLE_COEF01_HPHASE0	XCRE_COEF01_HPHASE0_OFFSET
#define CRESAMPLE_COEF02_HPHASE0	XCRE_COEF02_HPHASE0_OFFSET
#define CRESAMPLE_COEF03_HPHASE0	XCRE_COEF03_HPHASE0_OFFSET
#define CRESAMPLE_COEF04_HPHASE0	XCRE_COEF04_HPHASE0_OFFSET
#define CRESAMPLE_COEF05_HPHASE0	XCRE_COEF05_HPHASE0_OFFSET
#define CRESAMPLE_COEF06_HPHASE0	XCRE_COEF06_HPHASE0_OFFSET
#define CRESAMPLE_COEF07_HPHASE0	XCRE_COEF07_HPHASE0_OFFSET
#define CRESAMPLE_COEF08_HPHASE0	XCRE_COEF08_HPHASE0_OFFSET
#define CRESAMPLE_COEF09_HPHASE0	XCRE_COEF09_HPHASE0_OFFSET
#define CRESAMPLE_COEF10_HPHASE0	XCRE_COEF10_HPHASE0_OFFSET
#define CRESAMPLE_COEF11_HPHASE0	XCRE_COEF11_HPHASE0_OFFSET
#define CRESAMPLE_COEF12_HPHASE0	XCRE_COEF12_HPHASE0_OFFSET
#define CRESAMPLE_COEF13_HPHASE0	XCRE_COEF13_HPHASE0_OFFSET
#define CRESAMPLE_COEF14_HPHASE0	XCRE_COEF14_HPHASE0_OFFSET
#define CRESAMPLE_COEF15_HPHASE0	XCRE_COEF15_HPHASE0_OFFSET
#define CRESAMPLE_COEF16_HPHASE0	XCRE_COEF16_HPHASE0_OFFSET
#define CRESAMPLE_COEF17_HPHASE0	XCRE_COEF17_HPHASE0_OFFSET
#define CRESAMPLE_COEF18_HPHASE0	XCRE_COEF18_HPHASE0_OFFSET
#define CRESAMPLE_COEF19_HPHASE0	XCRE_COEF19_HPHASE0_OFFSET
#define CRESAMPLE_COEF20_HPHASE0	XCRE_COEF20_HPHASE0_OFFSET
#define CRESAMPLE_COEF21_HPHASE0	XCRE_COEF21_HPHASE0_OFFSET
#define CRESAMPLE_COEF22_HPHASE0	XCRE_COEF22_HPHASE0_OFFSET
#define CRESAMPLE_COEF23_HPHASE0	XCRE_COEF23_HPHASE0_OFFSET

#define CRESAMPLE_COEF00_HPHASE1	XCRE_COEF00_HPHASE1_OFFSET
#define CRESAMPLE_COEF01_HPHASE1	XCRE_COEF01_HPHASE1_OFFSET
#define CRESAMPLE_COEF02_HPHASE1	XCRE_COEF02_HPHASE1_OFFSET
#define CRESAMPLE_COEF03_HPHASE1	XCRE_COEF03_HPHASE1_OFFSET
#define CRESAMPLE_COEF04_HPHASE1	XCRE_COEF04_HPHASE1_OFFSET
#define CRESAMPLE_COEF05_HPHASE1	XCRE_COEF05_HPHASE1_OFFSET
#define CRESAMPLE_COEF06_HPHASE1	XCRE_COEF06_HPHASE1_OFFSET
#define CRESAMPLE_COEF07_HPHASE1	XCRE_COEF07_HPHASE1_OFFSET
#define CRESAMPLE_COEF08_HPHASE1	XCRE_COEF08_HPHASE1_OFFSET
#define CRESAMPLE_COEF09_HPHASE1	XCRE_COEF09_HPHASE1_OFFSET
#define CRESAMPLE_COEF10_HPHASE1	XCRE_COEF10_HPHASE1_OFFSET
#define CRESAMPLE_COEF11_HPHASE1	XCRE_COEF11_HPHASE1_OFFSET
#define CRESAMPLE_COEF12_HPHASE1	XCRE_COEF12_HPHASE1_OFFSET
#define CRESAMPLE_COEF13_HPHASE1	XCRE_COEF13_HPHASE1_OFFSET
#define CRESAMPLE_COEF14_HPHASE1	XCRE_COEF14_HPHASE1_OFFSET
#define CRESAMPLE_COEF15_HPHASE1	XCRE_COEF15_HPHASE1_OFFSET
#define CRESAMPLE_COEF16_HPHASE1	XCRE_COEF16_HPHASE1_OFFSET
#define CRESAMPLE_COEF17_HPHASE1	XCRE_COEF17_HPHASE1_OFFSET
#define CRESAMPLE_COEF18_HPHASE1	XCRE_COEF18_HPHASE1_OFFSET
#define CRESAMPLE_COEF19_HPHASE1	XCRE_COEF19_HPHASE1_OFFSET
#define CRESAMPLE_COEF20_HPHASE1	XCRE_COEF20_HPHASE1_OFFSET
#define CRESAMPLE_COEF21_HPHASE1	XCRE_COEF21_HPHASE1_OFFSET
#define CRESAMPLE_COEF22_HPHASE1	XCRE_COEF22_HPHASE1_OFFSET
#define CRESAMPLE_COEF23_HPHASE1	XCRE_COEF23_HPHASE1_OFFSET

#define CRESAMPLE_COEF00_VPHASE0	XCRE_COEF00_VPHASE0_OFFSET
#define CRESAMPLE_COEF01_VPHASE0	XCRE_COEF01_VPHASE0_OFFSET
#define CRESAMPLE_COEF02_VPHASE0	XCRE_COEF02_VPHASE0_OFFSET
#define CRESAMPLE_COEF03_VPHASE0	XCRE_COEF03_VPHASE0_OFFSET
#define CRESAMPLE_COEF04_VPHASE0	XCRE_COEF04_VPHASE0_OFFSET
#define CRESAMPLE_COEF05_VPHASE0	XCRE_COEF05_VPHASE0_OFFSET
#define CRESAMPLE_COEF06_VPHASE0	XCRE_COEF06_VPHASE0_OFFSET
#define CRESAMPLE_COEF07_VPHASE0	XCRE_COEF07_VPHASE0_OFFSET

#define CRESAMPLE_COEF00_VPHASE1	XCRE_COEF00_VPHASE1_OFFSET
#define CRESAMPLE_COEF01_VPHASE1	XCRE_COEF01_VPHASE1_OFFSET
#define CRESAMPLE_COEF02_VPHASE1	XCRE_COEF02_VPHASE1_OFFSET
#define CRESAMPLE_COEF03_VPHASE1	XCRE_COEF03_VPHASE1_OFFSET
#define CRESAMPLE_COEF04_VPHASE1	XCRE_COEF04_VPHASE1_OFFSET
#define CRESAMPLE_COEF05_VPHASE1	XCRE_COEF05_VPHASE1_OFFSET
#define CRESAMPLE_COEF06_VPHASE1	XCRE_COEF06_VPHASE1_OFFSET
#define CRESAMPLE_COEF07_VPHASE1	XCRE_COEF07_VPHASE1_OFFSET

#define CRESAMPLE_CTL_EN_MASK		XCRE_CTL_SW_EN_MASK
#define CRESAMPLE_CTL_RU_MASK		XCRE_CTL_RUE_MASK
#define CRESAMPLE_CTL_AUTORESET		XCRE_CTL_AUTORESET_MASK
#define CRESAMPLE_CTL_RESET		XCRE_CTL_RESET_MASK

#define CRESAMPLE_In32			XCresample_In32
#define CRESAMPLE_Out32			XCresample_Out32

#define CRESAMPLE_ReadReg		XCresample_ReadReg
#define CRESAMPLE_WriteReg		XCresample_WriteReg
/*@}*/

/** @name Interrupt registers
 * @{
 */
#define XCRE_ISR_OFFSET		XCRE_STATUS_OFFSET	/**< Interrupt status
							  *  register */
#define XCRE_IER_OFFSET		XCRE_IRQ_EN_OFFSET	/**< Interrupt enable
							 .*..register
							  *  corresponds to
							  *  status bits */
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

#define XCresample_In32		Xil_In32	/**< Input operations */
#define XCresample_Out32	Xil_Out32	/**< Output operations */

/*****************************************************************************/
/**
*
* This macro reads the given register.
*
* @param	BaseAddress is the Xilinx base address of the Chroma
* 		Resampler core
* @param	RegOffset is the register offset of the register (defined at
* 		top of this file)
*
* @return	The 32-bit value of the register
*
* @note		C-style signature:
* 		u32 XCresample_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XCresample_ReadReg(BaseAddress, RegOffset) \
	XCresample_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This macro writes into the given register.
*
* @param	BaseAddress is the Xilinx base address of the Chroma
*		Resampler core
* @param	RegOffset is the register offset of the register
* 		(defined at top of this file)
* @param	Data is the 32-bit value to write to the register
*
* @return	None.
*
* @note		C-style signature:
*		void XCresample_WriteReg(u32 BaseAddress, u32 RegOffset,
*								u32 Data)
*
******************************************************************************/
#define XCresample_WriteReg(BaseAddress, RegOffset, Data) \
	XCresample_Out32((BaseAddress) + (u32)(RegOffset), (u32)(Data))

/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}

#endif
#endif /* End of protection macro */
/** @} */
