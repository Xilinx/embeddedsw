/*****************************************************************************
*
* Copyright (C) 2001 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"),to deal
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
*****************************************************************************/

/**
*
* @file tpg.h
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx Test Pattern Generator 
* (TPG) core instance.
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a se   10/01/12 Initial creation
* 2.0a se   02/12/14 Cleaned up comments, updated masks and registers
* 2.0   adk  19/12/13 Updated as per the New Tcl API's
*
******************************************************************************/

#ifndef TPG_DRIVER_H        /* prevent circular inclusions */
#define TPG_DRIVER_H        /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/**
 * Register Offsets
 */
/* General Control Registers */
#define TPG_CONTROL             0x000    /**< Control (R/W) */
#define TPG_STATUS              0x004    /**< Status (R/W) */
#define TPG_ERROR               0x008    /**< Error (R/W) */
#define TPG_IRQ_EN              0x00C    /**< IRQ Enable     */
#define TPG_VERSION             0x010    /**< Version        */
/* Timing Control Registers */
#define TPG_ACTIVE_SIZE         0x020    /**< Active Size (V x H)       */
/* Core Specific Registers */
#define TPG_PATTERN_CONTROL     0x100
#define TPG_MOTION_SPEED        0x104
#define TPG_CROSS_HAIRS         0x108
#define TPG_ZPLATE_HOR_CONTROL  0x10C
#define TPG_ZPLATE_VER_CONTROL  0x110
#define TPG_BOX_SIZE            0x114
#define TPG_BOX_COLOR           0x118
#define TPG_STUCK_PIXEL_THRESH  0x11C
#define TPG_NOISE_GAIN          0x120
#define TPG_BAYER_PHASE         0x124

/**
 * TPG Control Register bit definition
 */
#define TPG_CTL_EN_MASK    0x00000001    /**< TPG Enable */
#define TPG_CTL_RUE_MASK   0x00000002    /**< TPG Register Update */
#define TPG_CTL_CS_MASK    0x00000004    /**< TPG Register Clear Status */

/**
 * TPG Reset Register bit definition     
 */                                      
#define TPG_RST_RESET      0x80000000    /**< Software Reset - Instantaneous */
#define TPG_RST_AUTORESET  0x40000000    /**< Software Reset - Auto-synchronize to SOF */

/**
 * TPG Pattern Control Register bit definition     
 */                                      
#define TPG_PASS_THROUGH   0x00000000
#define TPG_HOR_RAMP       0x00000001
#define TPG_VER_RAMP       0x00000002
#define TPG_TEMP_RAMP      0x00000003
#define TPG_SOLID_RED      0x00000004
#define TPG_SOLID_GREEN    0x00000005
#define TPG_SOLID_BLUE     0x00000006
#define TPG_SOILD_BLACK    0x00000007
#define TPG_SOLID_WHITE    0x00000008
#define TPG_COLOR_BARS     0x00000009
#define TPG_ZONE_PLATE     0x0000000A
#define TPG_TARTAN_BARS    0x0000000B
#define TPG_CROSS_HATCH    0x0000000C
#define TPG_VER_HOR_RAMP   0x0000000E
#define TPG_CHECKER_BOARD  0x0000000F
#define TPG_CROSS_HAIRS    0x00000010
#define TPG_MOVING_BOX     0x00000020
#define TPG_MASK_RED_CR    0x00000040
#define TPG_MASK_GREEN_Y   0x00000080
#define TPG_MASK_BLUE_CB   0x00000100
#define TPG_ENABLE_STUCK   0x00000200
#define TPG_ENABLE_NOISE   0x00000400
#define TPG_ENABLE_MOTION  0x00001000


/***************** Macros (Inline Functions) Definitions *********************/
#define TPG_In32          Xil_In32
#define TPG_Out32         Xil_Out32


/**
*
* Read the given register.
*
* @param BaseAddress is the Xilinx EDK base address of the TPG core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
*
* @return   The 32-bit value of the register
*
* @note
* C-style signature:
*    u32 TPG_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define TPG_ReadReg(BaseAddress, RegOffset) \
            TPG_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* Write the given register.
*
* @param BaseAddress is the Xilinx EDK base address of the TPG core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
* @param Data is the 32-bit value to write to the register
*
* @return   None.
*
* @note
* C-style signature:
*    void TPG_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define TPG_WriteReg(BaseAddress, RegOffset, Data) \
            TPG_Out32((BaseAddress) + (RegOffset), (Data))

/*****************************************************************************/
/**
*
* This macro enables a TPG core instance.
*
* @param BaseAddress is the Xilinx EDK base address of the TPG core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void TPG_Enable(u32 BaseAddress);
*
******************************************************************************/
#define TPG_Enable(BaseAddress) \
            TPG_WriteReg(BaseAddress, TPG_CONTROL, \
            	TPG_ReadReg(BaseAddress, TPG_CONTROL) | \
            	TPG_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro disables a TPG core instance.
*
* @param BaseAddress is the Xilinx EDK base address of the TPG core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void TPG_Disable(u32 BaseAddress);
*
******************************************************************************/
#define TPG_Disable(BaseAddress) \
            TPG_WriteReg(BaseAddress, TPG_CONTROL, \
            	TPG_ReadReg(BaseAddress, TPG_CONTROL) & \
            	~TPG_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro commits all the register value changes made so far by the software 
* to the TPG core instance. The registers will be automatically updated
* on the next rising-edge of the VBlank_in signal on the core.
* It is up to the user to manually disable the register update after a sufficient
* amount if time.
*
* This function only works when the TPG core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the TPG core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void TPG_RegUpdateEnable(u32 BaseAddress);
*
******************************************************************************/
#define TPG_RegUpdateEnable(BaseAddress) \
            TPG_WriteReg(BaseAddress, TPG_CONTROL, \
                TPG_ReadReg(BaseAddress, TPG_CONTROL) | \
                TPG_CTL_RUE_MASK)

/*****************************************************************************/
/**
*
* This macro prevents the TPG core instance from committing recent changes made 
* so far by the software. When disabled, changes to other configuration registers
* are stored, but do not effect the behavior of the core. 
*
* This function only works when the TPG core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the TPG core (from xparameters.h)
*
* @return None.
*
* @note 
* C-style signature:
*    void TPG_RegUpdateDisable(u32 BaseAddress);
*
******************************************************************************/
#define TPG_RegUpdateDisable(BaseAddress) \
            TPG_WriteReg(BaseAddress, TPG_CONTROL, \
                TPG_ReadReg(BaseAddress, TPG_CONTROL) & \
                ~TPG_CTL_RUE_MASK)

/*****************************************************************************/

/**
*
* This macro clears the status register of the TPG instance, by first asserting then
* deasserting the CLEAR_STATUS flag of TPG_CONTROL. 
* This function only works when the TPG core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the TPG core (from xparameters.h)
*
* @return None.
*
* @note 
* C-style signature:
*    void TPG_ClearStatus(u32 BaseAddress);
*
******************************************************************************/
#define TPG_ClearStatus(BaseAddress) \
   TPG_WriteReg(BaseAddress, TPG_CONTROL, TPG_ReadReg(BaseAddress, TPG_CONTROL) |  TPG_CTL_CS_MASK); \
   TPG_WriteReg(BaseAddress, TPG_CONTROL, TPG_ReadReg(BaseAddress, TPG_CONTROL) & ~TPG_CTL_CS_MASK) 

/*****************************************************************************/


/**
*
* This macro resets a TPG core instance. This reset effects the core immediately,
* and may cause image tearing.
*
* This reset resets the TPG's configuration registers, and holds the core's outputs
* in their reset state until TPG_ClearReset() is called.
* 
*
* @param BaseAddress is the Xilinx EDK base address of the TPG core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void TPG_Reset(u32 BaseAddress);
*
******************************************************************************/
#define TPG_Reset(BaseAddress) \
            TPG_WriteReg(BaseAddress, TPG_CONTROL, TPG_RST_RESET) \

/*****************************************************************************/
/**
*
* This macro clears the TPG's reset flag (which is set using TPG_Reset(), and
* returns it to normal operation. This ClearReset effects the core immediately,
* and may cause image tearing.
* 
*
* @param BaseAddress is the Xilinx EDK base address of the TPG core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void TPG_ClearReset(u32 BaseAddress);
*
******************************************************************************/
#define TPG_ClearReset(BaseAddress) \
            TPG_WriteReg(BaseAddress, TPG_CONTROL, 0) \

/*****************************************************************************/
/**
*
* This macro resets a TPG instance, but differs from TPG_Reset() in that it
* automatically synchronizes to the SOF of the core to prevent tearing.
*
* On the next rising-edge of SOF following a call to TPG_AutoSyncReset(), 
* all of the core's configuration registers and outputs will be reset, then the
* reset flag will be immediately released, allowing the core to immediately resume
* default operation.
*
* @param BaseAddress is the Xilinx EDK base address of the TPG core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void TPG_FSyncReset(u32 BaseAddress);
*
******************************************************************************/
#define TPG_FSyncReset(BaseAddress) \
            TPG_WriteReg(BaseAddress, TPG_CONTROL, TPG_RST_AUTORESET) \

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */ 
