/******************************************************************************
*
* Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xdeint_hw.h
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx Video Deinterlacer device.
*
* For more information about the operation of this device, see the hardware
* specification and documentation in the higher level driver xdeint.h source
* code file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rjh 07/10/11 First release
* 2.00a rjh 18/01/12 Updated for v_deinterlacer 2.00
* </pre>
*
******************************************************************************/

#ifndef XDeint_HW_H      /* prevent circular inclusions */
#define XDeint_HW_H      /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Offsets
 *  @{
 */
#define XDEINT_CONTROL        0x000    /**< Deinterlacer Main Control */
#define XDEINT_MODE           0x004    /**< Deinterlacer internal Modes */
#define XDEINT_IER            0x008    /**< Interrupt Enable Control */
#define XDEINT_ISR            0x00C    /**< Interrupt Enable Status */
#define XDEINT_HEIGHT         0x010    /**< Height */
#define XDEINT_WIDTH          0x014    /**< Width */
#define XDEINT_T1             0x018    /**< T1 Threshold */
#define XDEINT_T2             0x01C    /**< T2 Threshold */
#define XDEINT_XFADE          0x020    /**< Cross Fade Ration */
#define XDEINT_FS_BASE0       0x024    /**< VFBC Field Buffer 0 Base */
#define XDEINT_FS_BASE1       0x028    /**< VFBC Field Buffer 1 Base */
#define XDEINT_FS_BASE2       0x02C    /**< VFBC Field Buffer 2 Base */
#define XDEINT_FS_WORDS       0x030    /**< VFBC Field Buffer Page size in 32bit Words */
#define XDEINT_VER            0x0F0    /**< Hardware Version ID */
#define XDEINT_RESET          0x100    /**< Soft Reset */
/*@}*/

/** @name Interrupt Status/Enable Register bit definition
 *  @{
 */
#define XDEINT_IXR_UPDATE_MASK            0x00000001 /**< Internal Register update done  */
#define XDEINT_IXR_LOCKED_MASK            0x00000002 /**< Deinterlacer is locked to incoming video */
#define XDEINT_IXR_UNLOCKED_MASK          0x00000004 /**< Deinterlacer has lost lock to incoming video */
#define XDEINT_IXR_ERROR_MASK             0x00000008 /**< Deinterlacer internal fifo error */
#define XDEINT_IXR_PULL_ON_MASK           0x00000010 /**< Pulldown activated */
#define XDEINT_IXR_PULL_OFF_MASK          0x00000020 /**< Pulldown cancelled */
#define XDEINT_IXR_FRAME_MASK             0x00000040 /**< Frame Tick  */
#define XDEINT_IXR_FS_CFG_ERROR_MASK      0x00000100 /**< Framestore Write setup error */
#define XDEINT_IXR_FS_WR_ERROR_MASK       0x00000200 /**< Framestore Write fifo overflow */
#define XDEINT_IXR_FS_RD_FIELD_ERROR_MASK 0x00000400 /**< Framestore Read Field underrun */
#define XDEINT_IXR_FS_RD_FRAME_ERROR_MASK 0x00000800 /**< Framestore Read Frame underrun */

#define XDEINT_IXR_ALLINTR_MASK  (XDEINT_IXR_UPDATE_MASK | \
                                  XDEINT_IXR_LOCKED_MASK            | \
                                  XDEINT_IXR_UNLOCKED_MASK          | \
                                  XDEINT_IXR_ERROR_MASK             | \
                                  XDEINT_IXR_PULL_ON_MASK           | \
                                  XDEINT_IXR_PULL_OFF_MASK          | \
                                  XDEINT_IXR_FRAME_MASK             | \
                                  XDEINT_IXR_FS_CFG_ERROR_MASK      | \
                                  XDEINT_IXR_FS_WR_ERROR_MASK       | \
                                  XDEINT_IXR_FS_RD_FIELD_ERROR_MASK | \
                                  XDEINT_IXR_FS_RD_FRAME_ERROR_MASK) /**< Mask for all interrupts */
/*@}*/

/** @name Error Status bit definition
 *  @{
 */
#define XDEINT_STS_ERROR             0x00000008 /**< Deinterlacer internal fifo error */
#define XDEINT_STS_FS_CFG_ERROR      0x00000100 /**< Framestore Write setup error */
#define XDEINT_STS_FS_WR_ERROR       0x00000200 /**< Framestore Write fifo overflow */
#define XDEINT_STS_FS_RD_FIELD_ERROR 0x00000400 /**< Framestore Read Field underrun */
#define XDEINT_STS_FS_RD_FRAME_ERROR 0x00000800 /**< Framestore Read Frame underrun */
/*@}*/

#define XDEINT_RESET_RESET_MASK     0x00000001  /**< Software Reset */

/** @name Deinterlacer COntrol Fields
 *  @{
 */
#define XDEINT_VER_MAJOR_MASK        0xF0000000 /**< Major Version */
#define XDEINT_VER_MAJOR_SHIFT       28         /**< Major Bit Shift */
#define XDEINT_VER_MINOR_MASK        0x0FF00000 /**< Minor Version */
#define XDEINT_VER_MINOR_SHIFT       20         /**< Minor Bit Shift */
#define XDEINT_VER_REV_MASK          0x000F0000 /**< Revision Version */
#define XDEINT_VER_REV_SHIFT         16         /**< Revision Bit Shift */
/*@}*/

/** @name Deinterlacer COntrol Fields
 *  @{
 */
#define XDEINT_CTL_UPDATE_REQ        0x00000001 /**< Queue a register update request */
#define XDEINT_CTL_ENABLE            0x00000002 /**< Enable/Disable deinterlacer algorithms*/
#define XDEINT_CTL_ACCEPT_VIDEO      0x00000004 /**< Accept Video into the deinterlacer */
/*@}*/

/** @name Deinterlacer Mode Fields
 *  @{
 */
#define XDEINT_MODE_ALGORITHM_0      0x00000001 /**< Deinterlacer algorithm */
#define XDEINT_MODE_ALGORITHM_1      0x00000002 /**< Deinterlacer algorithm */
#define XDEINT_MODE_COL              0x00000004 /**< Colour Space */
#define XDEINT_MODE_PACKING_0        0x00000008 /**< XSVI Packing */
#define XDEINT_MODE_PACKING_1        0x00000010 /**< XSVI Packing */
#define XDEINT_MODE_FIELD_ORDER      0x00000020 /**< First field order */
#define XDEINT_MODE_PSF_ENABLE       0x00000040 /**< PSF passthrough enable */
#define XDEINT_MODE_PULL_32_ENABLE   0x00000080 /**< Pulldown 3:2 control enable  */
#define XDEINT_MODE_PULL_22_ENABLE   0x00000100 /**< Pulldown 3:2 control enable  */

#define XDEINT_MODE_COLOUR_RGB       0x00000000 /**< Deinterlacer colour space*/
#define XDEINT_MODE_COLOUR_YUV       0x00000004 /**< Deinterlacer colour space*/

#define XDEINT_MODE_ALGORITHM_RAW    0x00000000 /**< Deinterlacer algorithm option 0*/
#define XDEINT_MODE_ALGORITHM_DIAG   0x00000001 /**< Deinterlacer algorithm option 1*/
#define XDEINT_MODE_ALGORITHM_MOTION 0x00000002 /**< Deinterlacer algorithm option 2*/
#define XDEINT_MODE_ALGORITHM_FULL   0x00000003 /**< Deinterlacer algorithm option 3*/

#define XDEINT_MODE_PACKING_420      0x00000000 /**< XSVI Packing mode 420*/
#define XDEINT_MODE_PACKING_422      0x00000008 /**< XSVI Packing mode 422*/
#define XDEINT_MODE_PACKING_444      0x00000010 /**< XSVI Packing mode 444*/

#define XDEINT_MODE_FIELD_EVEN_FIRST 0x00000020 /**< First field of frame contains even video lines*/
#define XDEINT_MODE_FIELD_ODD_FIRST  0x00000000 /**< First field of frame contains odd video lines*/

// RMW Masking Bits.
#define XDEINT_MODE_ALGORITHM_MASK        0x00000003 /**< Deinterlacer algorithm */
#define XDEINT_MODE_COL_MASK              0x00000004 /**< Colour Space */
#define XDEINT_MODE_PACKING_MASK          0x00000018 /**< XSVI Packing */
#define XDEINT_MODE_FIELD_ORDER_MASK      0x00000020 /**< First field order */
#define XDEINT_MODE_PSF_ENABLE_MASK       0x00000040 /**< PSF passthrough enable */
#define XDEINT_MODE_PULL_ENABLE_MASK      0x00000180 /**< Pulldown control enable  */

/*@}*/



/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/** @name Device register I/O APIs
 *  @{
 */

#define XDeint_In32  Xil_In32
#define XDeint_Out32 Xil_Out32

/*****************************************************************************/
/**
*
* Read the given register.
*
* @param    BaseAddress is the base address of the device
* @param    RegOffset is the register offset to be read
*
* @return   The 32-bit value of the register
*
* @note
* C-style signature:
*    u32 XDeint_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XDeint_ReadReg(BaseAddress, RegOffset)             \
    XDeint_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* Write the given register.
*
* @param    BaseAddress is the base address of the device
* @param    RegOffset is the register offset to be written
* @param    Data is the 32-bit value to write to the register
*
* @return   None.
*
* @note
* C-style signature:
*    void XDeint_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XDeint_WriteReg(BaseAddress, RegOffset, Data)          \
    XDeint_Out32((BaseAddress) + (RegOffset), (Data))

/*@}*/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
