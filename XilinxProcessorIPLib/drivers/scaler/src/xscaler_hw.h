/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
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
/**
*
* @file xscaler_hw.h
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx MVI Video Scaler device.
*
* For more information about the operation of this device, see the hardware
* specification and documentation in the higher level driver xscaler.h source
* code file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a xd   05/14/09 First release
* 2.00a xd   12/14/09 Updated doxygen document tags
* 4.03a mpv  05/28/13 Updated the Driver input, output and aperture size mask
* 7.0   adk  08/22/14 Appended register offset macros with _OFFSET and
*                     Bit definition with _MASK.
*                     Provided backward compatibility for changed macros.
*                     Defined the following macros XSCL_CTL_MEMRD_EN_MASK.
*                     Modified XSCL_CTL_ENABLE to XSCL_CTL_SW_EN_MASK,
*                     XSCL_RESET_RESET_MASK to XSCL_CTL_RESET_MASK,
*                     XSCL_CTL_REGUPDATE to XSCL_CTL_RUE_MASK,
*                     XSCL_STSDONE_DONE and XSCL_STS_COEF_W_RDY_MASK to
*                     XSCL_IXR_COEF_W_RDY_MASK.
*                     Added XSCL_ERR_*_MASK s.
*                     Removed XSCL_GIER_GIE_MASK.
*                     Removed following macros as they were not defined in
*                     latest product guide(v 8.1):
*                     XSCL_STSERR_CODE*_MASK, XSCL_IXR_OUTPUT_FRAME_DONE_MASK,
*                     XSCL_IXR_COEF_FIFO_READY_MASK, XSCL_IXR_INPUT_ERROR_MASK
*                     XSCL_IXR_COEF_WR_ERROR_MASK,
*                     XSCL_IXR_REG_UPDATE_DONE_MASK,
*                     XSCL_IXR_OUTPUT_ERROR_MASK, XSCL_IXR_EVENT_MASK,
*                     XSCL_IXR_ERROR_MASK, XSCL_IXR_ALLINTR_MASK,
*                     XSCL_HSF_INT_MASK, XSCL_VSF_INT_MASK,
*                     XSCL_COEFFVALUE_BASE_SHIFT and XSCL_COEFVALUE_BASE_MASK.
*                     Modified bits of the following macros:
*                     XSCL_HSF_FRAC_MASK and XSCL_VSF_FRAC_MASK.
* </pre>
*
******************************************************************************/

#ifndef XSCALER_HW_H      /* prevent circular inclusions */
#define XSCALER_HW_H      /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Device Register Offsets
 *  @{
 */
#define XSCL_CTL_OFFSET		0x000	/**< Control Offset */
#define XSCL_STATUS_OFFSET	0x004	/**< Status Offset */
#define XSCL_ERROR_OFFSET	0x008	/**< Error Status Offset */
#define XSCL_IRQ_EN_OFFSET	0x00C	/**< For detecting operation
					  *  status Offset */
#define XSCL_VER_OFFSET		0x010	/**< Version Register Offset */
#define XSCL_HSF_OFFSET		0x100	/**< Horizontal Shrink Factor
					  *  Offset */
#define XSCL_VSF_OFFSET		0x104	/**< Vertical Shrink Factor
					  *  Offset */
#define XSCL_SRCSIZE_OFFSET	0x108	/**< Source-video resolution
					  *  Offset */
#define XSCL_APTHORI_OFFSET	0x10C	/**< First and last subject
					  *  pixels in input line
					  *  Offset */
#define XSCL_APTVERT_OFFSET	0x110	/**< First and last subject
					  *  lines in input image
					  *  Offset */
#define XSCL_OUTSIZE_OFFSET	0x114	/**< Output image size: width
					  *  and height Offset */
#define XSCL_NUMPHASE_OFFSET	0x118	/**< The numbers of phases in
					  *   current filter Offset */
#define XSCL_COEFFSETS_OFFSET	0x11C	/**< Active horizontal and
					  *  vertical coefficient sets
					  *  to use Offset */
#define XSCL_FRCTLUMALEFT_OFFSET	0x120	/**< Fractional value used to
						  *  initialize horizontal
						  *  accumulator at rectangle
						  *  left edge for luma
						  *  Offset */
#define XSCL_FRCTCHROMALEFT_OFFSET	0x124	/**< Fractional value used to
						  *  initialize horizontal
						  *  accumulator at rectangle
						  *  left edge for chroma
						  *  Offset */
#define XSCL_FRCTLUMATOP_OFFSET		0x128	/**< Fractional value used to
						  *  initialize horizontal
						  *  accumulator at rectangle
						  *  top edge for luma
						  *  Offset */
#define XSCL_FRCTCHROMATOP_OFFSET	0x12C	/**< Fractional value used to
						  *  initialize horizontal
						  *  accumulator at rectangle
						  *  top edge for chroma
						  *  Offset */
#define XSCL_COEFFSETADDR_OFFSET	0x130	/**< Address of Coefficient
						  *  set to write Offset */
#define XSCL_COEFFVALUE_OFFSET		0x134	/**< Coefficient values to
						  *  write Offset */
#define XSCL_COEFF_SET_BANK_OFFSET	0x138	/**< Coefficient set/bank
						  *  read address Offset */
#define XSCL_COEFF_MEM_OFFSET		0x13C	/**< Coefficient mem read
						  *  address Offset */
/*@}*/

/** @name Control Register bit definition
 *  @{
 */
#define XSCL_CTL_SW_EN_MASK	0x00000001	/**< Enable the Scaler on the
 						  *  next video frame Mask */
#define XSCL_CTL_RUE_MASK	0x00000002	/**< Register Update Enable
						  *  Mask */
#define XSCL_CTL_MEMRD_EN_MASK	0x00000008	/**< Coefficient Memory Read
						  *  Enable Mask */
#define XSCL_CTL_RESET_MASK	0x80000000	/**< Software reset
						  *  bit Mask */
/*@}*/

/** @name Status Register bit definition
 *  @{
 */
#define XSCL_IXR_COEF_W_RDY_MASK	0x00000001	/**< If 1, Coefficient
							  *  values can
							  *  be written into the
							  *  core Mask */
/*@}*/

/** @name Error Status Register bit definition (to be defined)
 *  @{
 */
#define XSCL_ERR_EOL_MASK	0x00000001	/**< End of line Mask */
#define XSCL_ERR_SOF_MASK	0x00000004	/**< Error in starting a frame
						  *  Mask */
#define XSCL_ERR_COEFF_WR_MASK	0x00000010	/**< Error while Writing a
						  *  Coefficient into core */
/*@}*/


/** @name Horizontal Shrink Factor Register bit definition
 *  @{
 */
#define XSCL_HSF_FRAC_MASK	0x00FFFFFF	/**< Horizontal Shrink Factor
						  *  fractional Mask */
/*@}*/

/** @name Vertical Shrink Factor Register bit definition
 *  @{
 */
 #define XSCL_VSF_FRAC_MASK	0x00FFFFFF	/**< Vertical Shrink Factor
						  *  fractional Mask */
/*@}*/

/** @name Aperture Horizontal Register bit definition
 *   @{
 */
#define XSCL_APTHORI_LASTPXL_MASK	0x1FFF0000 /**< Location of last pixel
						     *  in line */
#define XSCL_APTHORI_LASTPXL_SHIFT	16	/**< Shift for location of
						  *  last pixel */
#define XSCL_APTHORI_FIRSTPXL_MASK	0x00001FFF /**< Location of first pixel
						     *  in line */
/*@}*/

/** @name Aperture Vertical Register bit definition
 *  @{
 */
#define XSCL_APTVERT_LASTLINE_MASK	0x1FFF0000 /**< Location of last line
						     *  in active video */
#define XSCL_APTVERT_LASTLINE_SHIFT	16	/**< Shift for location of
						  * last line */
#define XSCL_APTVERT_FIRSTLINE_MASK	0x00001FFF /**< Location of first line
						     *  in active video */
/*@}*/

/** @name Output Size Register bit definition
 *  @{
 */
#define XSCL_OUTSIZE_NUMLINE_MASK	0x1FFF0000 /**< The number of lines in
						     *  output rectangle */
#define XSCL_OUTSIZE_NUMLINE_SHIFT	16	/**< Shift for the number of
						  *  lines */
#define XSCL_OUTSIZE_NUMPXL_MASK	0x00001FFF /**< The number of pixels in
						     *  output rectangle */
/*@}*/

/** @name Source Size Register bit definition
 *  @{
 */
#define XSCL_SRCSIZE_NUMLINE_MASK	0x1FFF0000 /**< The number of lines in
						     *  source image */
#define XSCL_SRCSIZE_NUMLINE_SHIFT	16	/**< Shift for the number of
						  *  lines */
#define XSCL_SRCSIZE_NUMPXL_MASK	0x00001FFF /**< The number of pixels in
						     *  source image */
/*@}*/


/** @name Number of Phases Register bit definition
 *  @{
 */
#define XSCL_NUMPHASE_VERT_MASK		0x00007F00 /**< The number of vertical
						     *  phases */
#define XSCL_NUMPHASE_VERT_SHIFT	8	/**< Shift for the number of
						  *  vertical phases */
#define XSCL_NUMPHASE_HORI_MASK		0x0000007F /**< The number of
						     *  horizontal phases */
/*@}*/

/** @name Active Coefficient Set Register bit definition
 *  @{
 */
#define XSCL_COEFFSETS_VERT_MASK	0x000000F0	/**< Active vertical
							  *  coefficient set */
#define XSCL_COEFFSETS_VERT_SHIFT	4		/**< Active vertical
							  *  coefficient set
							  *  shift */
#define XSCL_COEFFSETS_HORI_MASK	0x0000000F	/**< Active horizontal
							  *  coefficient set */
/*@}*/

/** @name Luma left edge horizontal accumulator fractional value register
 *  @{
 */
#define XSCL_FRCTLUMALEFT_VALUE_MASK	0x001FFFFF /**< Fractional value to
						     *  initialize horizontal
						.....*..accumulator for luma */
/*@}*/

/** @name Chroma left edge horizontal accumulator fractional value register
 *  @{
 */
#define XSCL_FRCTCHROMALEFT_VALUE_MASK	0x001FFFFF/**< Fractional value to
						    *  initialize horizontal
						    *  accumulator for
						    *  chroma */
/*@}*/

/** @name Luma top edge vertical accumulator fractional value register
 *  @{
 */
#define XSCL_FRCTLUMATOP_VALUE_MASK	0x001FFFFF /**< Fractional value to
						     *  initialize vertical
						     *  accumulator for luma */
/*@}*/

/** @name Chroma top edge vertical accumulator fractional value register
 *  @{
 */
#define XSCL_FRCTCHROMATOP_VALUE_MASK	0x001FFFFF /**< Fractional value to
						     *  initialize vertical
						     *  accumulator for
						     *  chroma */
/*@}*/

/** @name Coefficient band address register bit definition
 *  @{
 */
#define XSCL_COEFFSETADDR_ADDR_MASK	0x0000000F /**< Address of the
						     *  Coefficient bank to
						     *  write next */
/*@}*/

/** @name Coefficient Value Register bit definition
 *  @{
 */
#define XSCL_COEFFVALUE_NPLUS1_MASK	0xFFFF0000 /**< Second value in the
						     *  pair */
#define XSCL_COEFFVALUE_N_MASK		0x0000FFFF /**< First value in the
						     *  pair */
/*@}*/

/** @name Coefficient Set Bank Read bit definition
 *  @{
 */
#define XSCL_COEFF_SELECT_BANK_MASK	0x00000003	/**< Select require
							  *  bank Mask */
#define XSCL_COEFF_SELECT_SET_MASK	0x00000F00	/**< Select require
							  *  Set Mask */
/*@}*/

/** @name Chroma Format Type Definition
 *  @{
 */
#define XSCL_CHROMA_FORMAT_420		1	/**< YUV4:2:0 */
#define XSCL_CHROMA_FORMAT_422		2	/**< YUV4:2:2 */
#define XSCL_CHROMA_FORMAT_444		3	/**< YUV4:4:4 */
/*@}*/

/** @name Backward compatibility macros
 *  @{
 */
#define XSCL_CTL		XSCL_CTL_OFFSET
#define XSCL_STATUS		XSCL_STATUS_OFFSET
#define XSCL_ERROR		XSCL_ERROR_OFFSET
#define IRQ_ENABLE 		XSCL_IRQ_EN_OFFSET
#define XSCL_VER		XSCL_VER_OFFSET
#define XSCL_HSF		XSCL_HSF_OFFSET
#define XSCL_VSF		XSCL_VSF_OFFSET
#define XSCL_SRCSIZE		XSCL_SRCSIZE_OFFSET
#define XSCL_APTHORI		XSCL_APTHORI_OFFSET
#define XSCL_APTVERT		XSCL_APTVERT_OFFSET
#define XSCL_OUTSIZE		XSCL_OUTSIZE_OFFSET
#define XSCL_NUMPHASE		XSCL_NUMPHASE_OFFSET
#define XSCL_COEFFSETS		XSCL_COEFFSETS_OFFSET
#define XSCL_FRCTLUMALEFT	XSCL_FRCTLUMALEFT_OFFSET
#define XSCL_FRCTCHROMALEFT 	XSCL_FRCTCHROMALEFT_OFFSET
#define XSCL_FRCTLUMATOP	XSCL_FRCTLUMATOP_OFFSET
#define XSCL_FRCTCHROMATOP	XSCL_FRCTCHROMATOP_OFFSET
#define XSCL_COEFFSETADDR	XSCL_COEFFSETADDR_OFFSET
#define XSCL_COEFFVALUE		XSCL_COEFFVALUE_OFFSET
#define XSCL_COEFF_SET_BANK	XSCL_COEFF_SET_BANK_OFFSET
#define XSCL_COEFF_MEM		XSCL_COEFF_MEM_OFFSET

#define XSCL_CTL_REGUPDATE	XSCL_CTL_RUE_MASK
#define XSCL_CTL_ENABLE		XSCL_CTL_SW_EN_MASK
#define XSCL_RESET_RESET_MASK	XSCL_CTL_RESET_MASK

#define XSCL_STSDONE_DONE	XSCL_IXR_COEF_W_RDY_MASK
#define XSCL_STS_COEF_W_RDY_MASK XSCL_IXR_COEF_W_RDY_MASK


/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Scaler Register Access Macro Definition
 *  @{
 */
#define XScaler_In32  Xil_In32
#define XScaler_Out32 Xil_Out32

/*****************************************************************************/
/**
*
* Read the given register.
*
* @param	BaseAddress is the base address of the device
* @param	RegOffset is the register offset to be read
*
* @return	The 32-bit value of the register
*
* @note		C-style signature:
*		u32 XScaler_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XScaler_ReadReg(BaseAddress, RegOffset) \
	XScaler_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* Write the given register.
*
* @param	BaseAddress is the base address of the device
* @param	RegOffset is the register offset to be written
* @param	Data is the 32-bit value to write to the register
*
* @return	None.
*
* @note		C-style signature:
*		void XScaler_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XScaler_WriteReg(BaseAddress, RegOffset, Data) \
	XScaler_Out32((BaseAddress) + (RegOffset), (Data))

/*@}*/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
