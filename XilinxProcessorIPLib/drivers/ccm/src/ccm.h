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
* @file ccm.h
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx Color Correction Matrix(CCM) device.
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 2.00a jo   05/1/10  Updated for CCM V2.0
* 3.00a ren  09/11/11 Updated for CCM V3.0
* 4.00a jj   12/18/12 Converted from xio.h to xil_io.h,translating   
*		      basic types,MB cache functions, exceptions 
*		      and assertions to xil_io format 
* 5.0   adk  19/12/13 Updated as per the New Tcl API's
******************************************************************************/

#ifndef CCM_DRIVER_H        /* prevent circular inclusions */
#define CCM_DRIVER_H        /* by using protection macros */

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
#define CCM_CONTROL        0x000    /**< Control        */
#define CCM_STATUS         0x004    /**< Status         */
#define CCM_ERROR          0x008    /**< Error          */
#define CCM_IRQ_EN         0x00C    /**< IRQ Enable     */
#define CCM_VERSION        0x010    /**< Version        */
#define CCM_SYSDEBUG0      0x014    /**< System Debug 0 */
#define CCM_SYSDEBUG1      0x018    /**< System Debug 1 */
#define CCM_SYSDEBUG2      0x01C    /**< System Debug 2 */
/* Timing Control Registers */
#define CCM_ACTIVE_SIZE    0x020    /**< Active Size (V x H)       */
#define CCM_TIMING_STATUS  0x024    /**< Timing Measurement Status */
/* Core Specific Registers */
#define CCM_K11            0x100    /**< K11 Coefficient */
#define CCM_K12            0x104    /**< K12 Coefficient */
#define CCM_K13            0x108    /**< K13 Coefficient */
#define CCM_K21            0x10C    /**< K21 Coefficient */
#define CCM_K22            0x110    /**< K22 Coefficient */
#define CCM_K23            0x114    /**< K23 Coefficient */
#define CCM_K31            0x118    /**< K31 Coefficient */
#define CCM_K32            0x11C    /**< K32 Coefficient */
#define CCM_K33            0x120    /**< K33 Coefficient */
#define CCM_ROFFSET        0x124    /**< Red Offset      */
#define CCM_GOFFSET        0x128    /**< Green Offset    */
#define CCM_BOFFSET        0x12C    /**< Blue Offset     */
#define CCM_CLIP           0x130    /**< Clip (Max)      */
#define CCM_CLAMP          0x134    /**< Clamp (Min)     */

/*
 * CCM Control Register bit definition
 */
#define CCM_CTL_EN_MASK     0x00000001 /**< CCM Enable */
#define CCM_CTL_RUE_MASK    0x00000002 /**< CCM Register Update Enable */

/*
 * CCM Reset Register bit definition
 */
#define CCM_RST_RESET      0x80000000 /**< Software Reset - Instantaneous */
#define CCM_RST_AUTORESET  0x40000000 /**< Software Reset - Auto-synchronize to SOF */


/***************** Macros (Inline Functions) Definitions *********************/

#define CCM_In32          Xil_In32
#define CCM_Out32         Xil_Out32


/*****************************************************************************/
/**
*
* This macro enables a CCM device.
*
* @param BaseAddress is the Xilinx EDK base address of the CCM core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CCM_Enable(u32 BaseAddress);
*
******************************************************************************/
#define CCM_Enable(BaseAddress) \
            CCM_WriteReg(BaseAddress, CCM_CONTROL, \
            	CCM_ReadReg(BaseAddress, CCM_CONTROL) | \
            	CCM_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro disables a CCM device.
*
* @param BaseAddress is the Xilinx EDK base address of the CCM core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CCM_Disable(u32 BaseAddress);
*
******************************************************************************/
#define CCM_Disable(BaseAddress) \
            CCM_WriteReg(BaseAddress, CCM_CONTROL, \
            	CCM_ReadReg(BaseAddress, CCM_CONTROL) & \
            	~CCM_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro tells a CCM device to pick up all the register value changes
* made so far by the software. The registers will be automatically updated
* on the next rising-edge of the VBlank_in signal on the core.
* It is up to the user to manually disable the register update after a sufficient
* amount if time.
*
* This function only works when the CCM core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the CCM core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CCM_RegUpdateEnable(u32 BaseAddress);
*
******************************************************************************/
#define CCM_RegUpdateEnable(BaseAddress) \
            CCM_WriteReg(BaseAddress, CCM_CONTROL, \
                CCM_ReadReg(BaseAddress, CCM_CONTROL) | \
                CCM_CTL_RUE_MASK)

/*****************************************************************************/
/**
*
* This macro tells a CCM device not to update it's configuration registers made
* so far by the software. When disabled, changes to other configuration registers
* are stored, but do not effect the core's behavior. 
*
* This function only works when the CCM core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the CCM core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CCM_RegUpdateDisable(u32 BaseAddress);
*
******************************************************************************/
#define CCM_RegUpdateDisable(BaseAddress) \
            CCM_WriteReg(BaseAddress, CCM_CONTROL, \
                CCM_ReadReg(BaseAddress, CCM_CONTROL) & \
                ~CCM_CTL_RUE_MASK)

/*****************************************************************************/
/**
*
* This macro resets a CCM device. This reset effects the core immediately,
* and may cause image tearing.
*
* This reset resets the CCM's configuration registers, and holds the core's outputs
* in their reset state until CCM_ClearReset() is called.
* 
*
* @param BaseAddress is the Xilinx EDK base address of the CCM core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CCM_Reset(u32 BaseAddress);
*
******************************************************************************/
#define CCM_Reset(BaseAddress) \
            CCM_WriteReg(BaseAddress, CCM_CONTROL, CCM_RST_RESET) \

/*****************************************************************************/
/**
*
* This macro clears the CCM's reset flag (which is set using CCM_Reset(), and
* returns it to normal operation. This ClearReset effects the core immediately,
* and may cause image tearing.
* 
*
* @param BaseAddress is the Xilinx EDK base address of the CCM core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CCM_ClearReset(u32 BaseAddress);
*
******************************************************************************/
#define CCM_ClearReset(BaseAddress) \
            CCM_WriteReg(BaseAddress, CCM_CONTROL, 0) \


/*****************************************************************************/
/**
*
* This macro resets a CCM device, but differs from CCM_Reset() in that it
* automatically synchronizes to the VBlank_in input of the core to prevent tearing.
*
* On the next rising-edge of VBlank_in following a call to CCM_AutoSyncReset(), 
* all of the core's configuration registers and outputs will be reset, then the
* reset flag will be immediately released, allowing the core to immediately resume
* default operation.
*
* @param BaseAddress is the Xilinx EDK base address of the CCM core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CCM_Reset(u32 BaseAddress);
*
******************************************************************************/
#define CCM_AutoSyncReset(BaseAddress) \
            CCM_WriteReg(BaseAddress, CCM_CONTROL, CCM_RST_AUTORESET) \

/*****************************************************************************/
/**
*
* Read the given register.
*
* @param BaseAddress is the Xilinx EDK base address of the CCM core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
*
* @return   The 32-bit value of the register
*
* @note
* C-style signature:
*    u32 CCM_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define CCM_ReadReg(BaseAddress, RegOffset) \
            CCM_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* Write the given register.
*
* @param BaseAddress is the Xilinx EDK base address of the CCM core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
* @param Data is the 32-bit value to write to the register
*
* @return   None.
*
* @note
* C-style signature:
*    void CCM_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define CCM_WriteReg(BaseAddress, RegOffset, Data) \
            CCM_Out32((BaseAddress) + (RegOffset), (Data))

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
