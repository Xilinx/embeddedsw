/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtpg_hw.h
* @addtogroup tpg_v3_3
* @{
*
* This header file contains the hardware register offsets and register bit
* definitions for the Xilinx Test Pattern Generator (TPG) core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------------
* 3.0   adk    02/19/14 First release.
*                       Added the register offsets and bit masks for the
*                       registers and added backward compatibility for macros.
* </pre>
*
******************************************************************************/
#ifndef XTPG_HW_H_
#define XTPG_HW_H_	/**< Prevent circular inclusions
			  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name TPG registers offsets
 * @{
 */
#define XTPG_CONTROL_OFFSET		0x000	/**< Control Offset */
#define XTPG_STATUS_OFFSET		0x004	/**< Status Offset */
#define XTPG_ERROR_OFFSET		0x008	/**< Error Offset */
#define XTPG_IRQ_EN_OFFSET		0x00C	/**< IRQ Enable Offset */
#define XTPG_VERSION_OFFSET		0x010	/**< Version Offset */
#define XTPG_ACTIVE_SIZE_OFFSET		0x020	/**< Active Size (V x H)
						  *  Offset */
#define XTPG_PATTERN_CONTROL_OFFSET	0x100	/**< Pattern Control Offset */
#define XTPG_MOTION_SPEED_OFFSET	0x104	/**< Motion Speed Offset */
#define XTPG_CROSS_HAIRS_OFFSET		0x108	/**< Cross Hair Offset */
#define XTPG_ZPLATE_HOR_CONTROL_OFFSET	0x10C	/**< ZPlate Horizontal Control
						  *  Offset */
#define XTPG_ZPLATE_VER_CONTROL_OFFSET	0x110	/**< ZPlate Vertical Control
						  *  Offset */
#define XTPG_BOX_SIZE_OFFSET		0x114	/**< Box Size Offset */
#define XTPG_BOX_COLOR_OFFSET		0x118	/**< Box Color Offset */
#define XTPG_STUCK_PIXEL_THRESH_OFFSET	0x11C	/**< Stuck Pixel Threshold
						  *  Offset */
#define XTPG_NOISE_GAIN_OFFSET		0x120	/**< Noise Gain Offset */
#define XTPG_BAYER_PHASE_OFFSET		0x124	/**< Bayer Phase Offset */
/*@}*/


/** @name Control register bit masks
 * @{
 */
#define XTPG_CTL_SW_EN_MASK	0x00000001	/**< S/W Enable Mask */
#define XTPG_CTL_RUE_MASK	0x00000002	/**< Register Update Enable
						  *  Mask */
#define XTPG_CTL_AUTORESET_MASK	0x40000000	/**< Software Reset -
						  *  Auto-synchronize to SOF
						  *  Mask */
#define XTPG_CTL_RESET_MASK	0x80000000	/**< Software Reset -
						  *  Instantaneous Mask */
/*@}*/

/** @name Interrupt register bit masks. It is applicable for
 *	  Status and IRQ_ENABLE Registers
 * @{
 */
#define XTPG_IXR_PROCS_STARTED_MASK	0x00000001/**< Process started Mask */
#define XTPG_IXR_EOF_MASK	0x00000002	/**< End-Of-Frame Mask */
#define XTPG_IXR_SE_MASK	0x00010000	/**< Slave Error Mask */
#define XTPG_IXR_ALLINTR_MASK	0x00010003	/**< Addition of all Masks */
/*@}*/

/** @name Error register bit mask definitions
 * @{
 */
#define XTPG_ERR_EOL_EARLY_MASK	0x00000001	/**< End of Line Early Mask */
#define XTPG_ERR_EOL_LATE_MASK	0x00000002	/**< End of Line Late Mask */
#define XTPG_ERR_SOF_EARLY_MASK	0x00000004	/**< Start of Frame Early
						  *  Mask */
#define XTPG_ERR_SOF_LATE_MASK	0x00000008	/**< Start of Frame Late
						   * Mask */
/*@}*/

/** @name Version register bit masks and shifts
 * @{
 */
#define XTPG_VER_REV_NUM_MASK	0x000000FF	/**< Version Revision Number
						  *  Mask */
#define XTPG_VER_PID_MASK	0x00000F00	/**< Version Patch ID Mask */
#define XTPG_VER_REV_MASK	0x0000F000 	/**< Version Revision Mask */
#define XTPG_VER_MINOR_MASK	0x00FF0000 	/**< Version Minor Mask */
#define XTPG_VER_MAJOR_MASK	0xFF000000 	/**< Version Major Mask */
#define	XTPG_VER_INTERNAL_SHIFT	8		/**< Version Internal Shift */
#define XTPG_VER_REV_SHIFT	12		/**< Version Revision Shift */
#define XTPG_VER_MINOR_SHIFT	16		/**< Version Minor Shift */
#define XTPG_VER_MAJOR_SHIFT	24		/**< Version Major Shift */

/*@}*/

/** @name Active Size register bit masks and shifts
 *  @{
 */
#define XTPG_ACTSIZE_NUM_PIXEL_MASK	0x00001FFF	/**< Number of Active
							  *  pixels per
							  *  scan line
							  *  (Horizontal)
							  *  Mask */
#define XTPG_ACTSIZE_NUM_LINE_MASK	0x1FFF0000	/**< Number of Active
							  *  lines per Frame
							  *  (Vertical)
							  *  Mask */
#define XTPG_ACTSIZE_NUM_LINE_SHIFT	16		/**< Shift for number
							  *  of lines */
/*@}*/

/** @name Pattern Control register bit masks
 * @{
 */
#define XTPG_PTRN_CTL_SET_BG_MASK	0x0000000F	/**< Set background
							  *  pattern
							  *  mask */
#define XTPG_PTRN_CTL_EN_CROSSHAIR_MASK	0x00000010	/**< Enable Cross Hair
							  *  Mask */
#define XTPG_PTRN_CTL_EN_BOX_MASK	0x00000020	/**< Moving box enable
							  *  Mask */
#define XTPG_PTRN_CTL_MASK_COLR_COMP_MASK	0x000001C0	/**< Mask out a
								  *  particular
								  *  color
								  *  component
								  *  mask */

#define XTPG_PTRN_CTL_EN_STUCK_MASK	0x00000200	/**< Enable Stuck
							  *  Mask */
#define XTPG_PTRN_CTL_EN_NOISE_MASK	0x00000400	/**< Enable Noise
							  *  Mask */
#define XTPG_PTRN_CTL_EN_MOTION_MASK	0x00001000	/**< Enable Motion
							  *  Mask */


#define XTPG_PTRN_CTL_MASK_COLR_COMP_SHIFT	5	/**< Shift for a
							  *  particular
							  *  color component
							  *  shift */
/*@}*/

/** @name Pattern values of Pattern Control register
 * @{
 */
 #define XTPG_PTRN_CTL_PASS_THROUGH	0x00000000	/**< Value for Pass
							  *  Through */
 #define XTPG_PTRN_CTL_HOR_RAMP		0x00000001	/**< Value for
							  *  Horizontal
							  *  Ramp */
 #define XTPG_PTRN_CTL_VER_RAMP		0x00000002	/**< Value for
							  *  Vertical Ramp */
 #define XTPG_PTRN_CTL_TEMP_RAMP	0x00000003	/**< Value for Temporal
							  *  Ramp */
 #define XTPG_PTRN_CTL_SOLID_RED	0x00000004	/**< Value for Solid
							  *  Red Output */
 #define XTPG_PTRN_CTL_SOLID_GREEN	0x00000005	/**< Value for Solid
							  *  Green Output */
 #define XTPG_PTRN_CTL_SOLID_BLUE	0x00000006	/**< Value for Solid
							  *  Blue Output */
 #define XTPG_PTRN_CTL_SOLID_BLACK	0x00000007	/**< Value for Solid
							  *  Black Output */
 #define XTPG_PTRN_CTL_SOLID_WHITE	0x00000008	/**< Value for Solid
							  *  White Output */
 #define XTPG_PTRN_CTL_COLOR_BARS	0x00000009	/**< Value for Color
 							  *  Bars Output */
 #define XTPG_PTRN_CTL_ZONE_PLATE	0x0000000A	/**< Value for
							  *  Zone Plate */
 #define XTPG_PTRN_CTL_TARTAN_BARS	0x0000000B	/**< Value for Tartan
							  *  Bars */
 #define XTPG_PTRN_CTL_CROSS_HATCH	0x0000000C	/**< Value for Cross
							  *  Hatch */
 #define XTPG_PTRN_CTL_VER_HOR_RAMP	0x0000000E	/**< Value for Combined
							  *  Vertical and
							  *  Horizontal ramp */
 #define XTPG_PTRN_CTL_CHECKER_BOARD	0x0000000F	/**< Value for Black
							  *  and White Checker
							  *  Board */


 #define XTPG_PTRN_CTL_MASK_RED_CR	0x00000040	/**< Value for Masking
							  *  Red Component */
 #define XTPG_PTRN_CTL_MASK_GREEN_Y	0x00000080	/**< Value for Masking
							  *  Green Component */
 #define XTPG_PTRN_CTL_MASK_BLUE_CB	0x00000100	/**< Value for Masking
							  *  Blue Component */
/*@}*/

/** @name Motion Speed register bit mask
 * @{
 */
#define XTPG_MOTION_SPEED_MASK	0x000000FF	/**< Motion Speed Mask */
/*@}*/

/** @name Cross Hair register bit masks and shift
 * @{
 */
#define XTPG_CROSSHAIR_HPOS_MASK	0x00001FFF	/**< CrossHair
							  *  Horizontal
							  *  Position Mask */
#define XTPG_CROSSHAIR_VPOS_MASK	0x1FFF0000	/**< CrossHair Vertical
							  *  Position Mask */
#define XTPG_CROSSHAIR_SHIFT		16		/**< Cross Hair
							  *  Shift */
/*@}*/

/** @name ZPlate Horizontal Control register bit masks and shift
 * @{
 */
#define XTPG_ZPLATEHOR_START_MASK	0x0000FFFF	/**< ZPlate Horizontal
							  *  Start Mask */
#define XTPG_ZPLATEHOR_SPEED_MASK	0xFFFF0000	/**< ZPlate Horizontal
							  *  Speed Mask */
#define XTPG_ZPLATEHOR_SPEED_SHIFT	16		/**< ZPlate Horizontal
							  *  Speed Shift */
/*@}*/

/** @name ZPlate Vertical Control register bit masks
 * @{
 */
#define XTPG_ZPLATEVER_START_MASK	0x0000FFFF	/**< ZPlate Vertical
							  *  Start Mask */
#define XTPG_ZPLATEVER_SPEED_MASK	0xFFFF0000	/**< ZPlate Vertical
							  *  Speed Mask */
#define XTPG_ZPLATEVER_SPEED_SHIFT	16		/**< ZPlate Vertical
							  *  Speed */
/*@}*/

/** @name Box Size register bit mask
 * @{
 */
#define XTPG_BOX_SIZE_MASK	0x00001FFF	/**< Box Size Mask */
/*@}*/

/** @name TPG Box Color register bit masks
 * @{
 */
#define XTPG_BOXCOL_BLUE_MASK	0x000000FF	/**< Blue Color Mask */
#define XTPG_BOXCOL_GREEN_MASK	0x0000FF00	/**< Green Color Mask */
#define XTPG_BOXCOL_RED_MASK	0x00FF0000	/**< Red Color Mask */
#define XTPG_BOXCOL_GREEN_SHIFT	8		/**< Green color shift */
#define XTPG_BOXCOL_RED_SHIFT	16		/**< Red color shift */
/*@}*/

/** @name Stuck Pixel Threshold register bit mask
 * @{
 */
#define XTPG_STUCKPIX_THRESH_MASK	0x0000FFFF	/**< Stuck Pixel
							  *  Threshold Mask */
/*@}*/

/** @name Noise Gain register bit mask
 * @{
 */
#define XTPG_NOISE_GAIN_MASK	0x000000FF	/**< Nose Gain Mask */
/*@}*/

/** @name Bayer Phase register bit mask
 * @{
 */
#define XTPG_BAYER_PHASE_MASK	0x00000007	/**< Bayer Phase Mask */
/*@}*/

/** @name General purpose masks
 * @{
 */
#define XTPG_8_BIT_MASK		0x000000FF	/**< 8-bit Mask */
/*@}*/

/**@name Backward compatibility macros
 * @{
 */
#define TPG_CONTROL		XTPG_CONTROL_OFFSET
#define TPG_STATUS		XTPG_STATUS_OFFSET
#define TPG_ERROR		XTPG_ERROR_OFFSET
#define TPG_IRQ_EN		XTPG_IRQ_EN_OFFSET
#define TPG_VERSION		XTPG_VERSION_OFFSET
#define TPG_ACTIVE_SIZE		XTPG_ACTIVE_SIZE_OFFSET
#define TPG_PATTERN_CONTROL	XTPG_PATTERN_CONTROL_OFFSET
#define TPG_MOTION_SPEED	XTPG_MOTION_SPEED_OFFSET
#define TPG_CROSS_HAIRS		XTPG_CROSS_HAIRS_OFFSET
#define TPG_ZPLATE_HOR_CONTROL	XTPG_ZPLATE_HOR_CONTROL_OFFSET
#define TPG_ZPLATE_VER_CONTROL	XTPG_ZPLATE_VER_CONTROL_OFFSET
#define TPG_BOX_SIZE		XTPG_BOX_SIZE_OFFSET_OFFSET
#define TPG_BOX_COLOR		XTPG_BOX_COLOR_OFFSET
#define TPG_STUCK_PIXEL_THRESH	XTPG_STUCK_PIXEL_THRESH_OFFSET
#define TPG_NOISE_GAIN		XTPG_NOISE_GAIN_OFFSET
#define TPG_CTL_EN_MASK		XTPG_CTL_SW_EN_MASK
#define TPG_CTL_RUE_MASK	XTPG_CTL_RUE_MASK
#define TPG_RST_RESET		XTPG_CTL_RESET_MASK
#define TPG_RST_AUTORESET	XTPG_CTL_AUTORESET_MASK
#define TPG_BAYER_PHASE		XTPG_BAYER_PHASE_OFFSET
#define TPG_PASS_THROUGH	XTPG_PASS_THROUGH
#define TPG_HOR_RAMP		XTPG_HOR_RAMP
#define TPG_VER_RAMP		XTPG_VER_RAMP
#define TPG_TEMP_RAMP		XTPG_TEMP_RAMP
#define TPG_SOLID_RED		XTPG_SOLID_RED
#define TPG_SOLID_GREEN		XTPG_SOLID_GREEN
#define TPG_SOLID_BLUE		XTPG_SOLID_BLUE
#define TPG_SOILD_BLACK		XTPG_SOLID_BLACK
#define TPG_SOLID_WHITE		XTPG_SOLID_WHITE
#define TPG_COLOR_BARS		XTPG_COLOR_BARS
#define TPG_ZONE_PLATE		XTPG_ZONE_PLATE
#define TPG_TARTAN_BARS		XTPG_TARTAN_BARS
#define TPG_CROSS_HATCH		XTPG_CROSS_HATCH
#define TPG_VER_HOR_RAMP	XTPG_VER_HOR_RAMP
#define TPG_CHECKER_BOARD	XTPG_CHECKER_BOARD
#define TPG_MOVING_BOX		XTPG_PTRN_CTL_EN_BOX_MASK
#define TPG_MASK_RED_CR		XTPG_MASK_RED_CR
#define TPG_MASK_GREEN_Y	XTPG_MASK_GREEN_Y
#define TPG_MASK_BLUE_CB	XTPG_MASK_BLUE_CB
#define TPG_ENABLE_STUCK	XTPG_PTRN_CTL_EN_STUCK_MASK
#define TPG_ENABLE_NOISE	XTPG_PTRN_CTL_EN_NOISE_MASK
#define TPG_ENABLE_MOTION	XTPG_PTRN_CTL_EN_MOTION_MASK
#define TPG_In32		XTpg_In32
#define TPG_Out32		XTpg_Out32
#define XTPG_ReadReg		XTpg_ReadReg
#define XTPG_WriteReg		XTpg_WriteReg

/*@}*/

/** @name Interrupt Enable and Status Registers Offsets
 * @{
 */
/**
* Interrupt status register generates a interrupt if the corresponding bits of
* interrupt enable register bits are set.
*/
#define XTPG_ISR_OFFSET	XTPG_STATUS_OFFSET	/**< Interrupt Status Offset */
#define XTPG_IER_OFFSET	XTPG_IRQ_EN_OFFSET	/**< Interrupt Enable Offset */
 /*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

#define	XTpg_In32	Xil_In32	/**< Input Operation */
#define	XTpg_Out32	Xil_Out32	/**< Output Operation */

/*****************************************************************************/
/**
*
* This function macro reads the given register.
*
* @param	BaseAddress is the base address of the TPG core.
* @param	RegOffset is the register offset of the register (defined at
*		top of this file).
*
* @return	32-bit value of the register.
*
* @note		C-style signature:
* 		u32 XTpg_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XTpg_ReadReg(BaseAddress, RegOffset) \
	XTpg_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This function macro writes the given register.
*
* @param	BaseAddress is the base address of the TPG core.
* @param	RegOffset is the register offset of the register (defined at
*		top of this file).
* @param	Data is the 32-bit value to write into the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XTpg_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XTpg_WriteReg(BaseAddress, RegOffset, Data) \
	XTpg_Out32((BaseAddress) + (u32)(RegOffset), (Data))

/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}

#endif

#endif /* End of protection macro */
/** @} */
