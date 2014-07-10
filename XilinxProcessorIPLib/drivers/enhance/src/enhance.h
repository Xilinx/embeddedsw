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
/**
*
* @file enhance.h
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx Image Enhancement core instance.
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.00a vyc  06/19/13  Updated for ENHANCE V8.0
*                      New edge enhancement algorithm and registers
*                      Noise reduction support added
* 4.00a vyc  04/24/12  Updated for ENHANCE V4.00.a
*                      Converted from xio.h to xil_io.h, translating
*                      basic type, MB cache functions, exceptions and
*                      assertion to xil_io format.
* 3.00a rc   09/11/11  Updated for ENHANCE V3.0
* 2.00a vc   12/14/10  Updated for ENHANCE V2.0
* 6.0   adk  19/12/13 Updated as per the New Tcl API's
*
******************************************************************************/

#ifndef ENHANCE_DRIVER_H        /* prevent circular inclusions */
#define ENHANCE_DRIVER_H        /* by using protection macros */

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
#define ENHANCE_CONTROL          0x0000    /**< Control        */
#define ENHANCE_STATUS           0x0004    /**< Status         */
#define ENHANCE_ERROR            0x0008    /**< Error          */
#define ENHANCE_IRQ_ENABLE       0x000C    /**< IRQ Enable     */
#define ENHANCE_VERSION          0x0010    /**< Version        */
#define ENHANCE_SYSDEBUG0        0x0014    /**< System Debug 0 */
#define ENHANCE_SYSDEBUG1        0x0018    /**< System Debug 1 */
#define ENHANCE_SYSDEBUG2        0x001C    /**< System Debug 2 */
/* Timing Control Registers */
#define ENHANCE_ACTIVE_SIZE      0x0020    /**< Horizontal and Vertical Active Frame Size */
/* Core Specific Registers */
#define ENHANCE_NOISE_THRESHOLD  0x0100    /**< Noise Reduction Control */
#define ENHANCE_ENHANCE_STRENGTH 0x0104    /**< Edge Enhancement Control */
#define ENHANCE_HALO_SUPPRESS    0x0108    /**< Halo Suppression Control */


/*****************************************************************************/
/**
 * Control Register bit definition
 */
#define ENHANCE_CTL_EN_MASK    0x00000001 /**< Enable */
#define ENHANCE_CTL_RU_MASK    0x00000002 /**< Register Update */
#define ENHANCE_CTL_AUTORESET  0x40000000 /**< Software Reset - Auto-synchronize to SOF */
#define ENHANCE_CTL_RESET      0x80000000 /**< Software Reset - Instantaneous */

/***************** Macros (Inline Functions) Definitions *********************/
#define ENHANCE_In32          Xil_In32
#define ENHANCE_Out32         Xil_Out32

/*****************************************************************************/
/**
*
* Read the given register.
*
* @param BaseAddress is the Xilinx base address of the Image Enhancement core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
*
* @return   The 32-bit value of the register
*
* @note
* C-style signature:
*    u32 ENHANCE_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define ENHANCE_ReadReg(BaseAddress, RegOffset) \
            ENHANCE_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* Write the given register.
*
* @param BaseAddress is the Xilinx base address of the Image Enhancement core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
* @param Data is the 32-bit value to write to the register
*
* @return   None.
*
* @note
* C-style signature:
*    void ENHANCE_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define ENHANCE_WriteReg(BaseAddress, RegOffset, Data) \
            ENHANCE_Out32((BaseAddress) + (RegOffset), (Data))

/*****************************************************************************/
/**
*
* This macro enables a Image Enhancement core instance.
*
* @param BaseAddress is the Xilinx base address of the Image Enhancement core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void ENHANCE_Enable(u32 BaseAddress);
*
******************************************************************************/
#define ENHANCE_Enable(BaseAddress) \
            ENHANCE_WriteReg(BaseAddress, ENHANCE_CONTROL, \
            	ENHANCE_ReadReg(BaseAddress, ENHANCE_CONTROL) | \
            	ENHANCE_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro disables a Image Enhancement core instance.
*
* @param BaseAddress is the Xilinx base address of the Image Enhancement core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void ENHANCE_Disable(u32 BaseAddress);
*
******************************************************************************/
#define ENHANCE_Disable(BaseAddress) \
            ENHANCE_WriteReg(BaseAddress, ENHANCE_CONTROL, \
            	ENHANCE_ReadReg(BaseAddress, ENHANCE_CONTROL) & \
            	~ENHANCE_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro commits all the register value changes made so far by the software 
* to the Image Enhancement core instance. The registers will be automatically updated
* on the next rising-edge of the SOF signal on the core.
* It is up to the user to manually disable the register update after a sufficient
* amount of time.
*
* This function only works when the Image Enhancement core is enabled.
*
* @param BaseAddress is the Xilinx base address of the Image Enhancement core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void ENHANCE_RegUpdateEnable(u32 BaseAddress);
*
******************************************************************************/
#define ENHANCE_RegUpdateEnable(BaseAddress) \
            ENHANCE_WriteReg(BaseAddress, ENHANCE_CONTROL, \
                ENHANCE_ReadReg(BaseAddress, ENHANCE_CONTROL) | \
                ENHANCE_CTL_RU_MASK)

/*****************************************************************************/
/**
*
* This macro prevents the Image Enhancement core instance from committing recent changes made 
* so far by the software. When disabled, changes to other configuration registers
* are stored, but do not effect the behavior of the core. 
*
* This function only works when the Image Enhancement core is enabled.
*
* @param BaseAddress is the Xilinx base address of the Image Enhancement core (from xparameters.h)
*
* @return None.
*
* @note 
* C-style signature:
*    void ENHANCE_RegUpdateDisable(u32 BaseAddress);
*
******************************************************************************/
#define ENHANCE_RegUpdateDisable(BaseAddress) \
            ENHANCE_WriteReg(BaseAddress, ENHANCE_CONTROL, \
                ENHANCE_ReadReg(BaseAddress, ENHANCE_CONTROL) & \
                ~ENHANCE_CTL_RU_MASK)

/*****************************************************************************/

/**
*
* This macro resets a Image Enhancement core instance. This reset effects the core immediately,
* and may cause image tearing.
*
* This reset resets the Image Enhancement's configuration registers.
* 
*
* @param BaseAddress is the Xilinx base address of the Image Enhancement core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void ENHANCE_Reset(u32 BaseAddress);
*
******************************************************************************/
#define ENHANCE_Reset(BaseAddress) \
            ENHANCE_WriteReg(BaseAddress, ENHANCE_CONTROL, ENHANCE_CTL_RESET) \

/*****************************************************************************/
/**
*
* This macro resets a Image Enhancement core instance, but differs from ENHANCE_Reset() in that it
* automatically synchronizes to the SOF input of the core to prevent tearing.
*
* On the next SOF following a call to ENHANCE_AutoSyncReset(), 
* all of the core's configuration registers and outputs will be reset, then the
* reset flag will be immediately released, allowing the core to immediately resume
* default operation.
*
* @param BaseAddress is the Xilinx base address of the Image Enhancement core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void ENHANCE_AutoSyncReset(u32 BaseAddress);
*
******************************************************************************/
#define ENHANCE_AutoSyncReset(BaseAddress) \
            ENHANCE_WriteReg(BaseAddress, ENHANCE_CONTROL, ENHANCE_CTL_AUTORESET) \


/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */ 
