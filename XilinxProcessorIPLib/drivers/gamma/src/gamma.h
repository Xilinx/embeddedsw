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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file gamma.h
* @addtogroup gamma_v6_0
* @{
* @details
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx Gamma Correction (GAMMA) instance.
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.01a jude 09/07/12  Updated for GAMMA V6.01.a-Added Masks for register bits
*                      - Converted from xio.h to xil_io.h, translating basic
*                      types, MB cache functions, exceptions and assertions to
*                      xil_io format
* 5.00a tb   02/27/12  Updated for GAMMA V5.00.a
* 4.00a rc   09/11/11  Updated for GAMMA V4.0
* 3.00a jo   07/20/10  Updated for GAMMA V3.0
* 3.00a gs   07/27/10  Updated for gamma registers and functionality
* 6.0   adk  19/12/13 Updated as per the New Tcl API's
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
*
******************************************************************************/

#ifndef GAMMA_DRIVER_H        /* prevent circular inclusions */
#define GAMMA_DRIVER_H        /* by using protection macros */

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
#define GAMMA_CONTROL        0x000    /**< Control        */
#define GAMMA_STATUS         0x004    /**< Status         */
#define GAMMA_ERROR          0x008    /**< Error          */
#define GAMMA_IRQ_EN         0x00C    /**< IRQ Enable     */
#define GAMMA_VERSION        0x010    /**< Version        */
#define GAMMA_SYSDEBUG0      0x014    /**< System Debug 0 */
#define GAMMA_SYSDEBUG1      0x018    /**< System Debug 1 */
#define GAMMA_SYSDEBUG2      0x01C    /**< System Debug 2 */
/* Timing Control Registers */
#define GAMMA_ACTIVE_SIZE    0x020    /**< Active Size (V x H)       */
#define GAMMA_TIMING_STATUS  0x024    /**< Timing Measurement Status */
/* Core Specific Registers */
#define GAMMA_TABLE_UPDATE   0x100    /**< Swap to inactive LUT      */
#define GAMMA_ADDR_DATA      0x104    /**< Address and Data register */

/*
 * Gamma Control Register bit definition
 */
#define GAMMA_CTL_EN_MASK        0x00000001 /**< Gamma Enable */
#define GAMMA_CTL_RUE_MASK       0x00000002 /**< Gamma Register Update Enable */
#define GAMMA_CTL_BPE_MASK       0x00000010 /**< Gamma ByPass Enable */
#define GAMMA_CTL_TPE_MASK       0x00000020 /**< Gamma Test Pattern Enable */

/*
 * Gamma Reset Register bit definition
 */
#define GAMMA_RST_RESET      0x80000000 /**< Software Reset - Instantaneous */
#define GAMMA_RST_AUTORESET  0x40000000 /**< Software Reset - Auto-synchronize to SOF */


/***************** Macros (Inline Functions) Definitions *********************/

#define GAMMA_In32          Xil_In32
#define GAMMA_Out32         Xil_Out32


/*****************************************************************************/
/**
*
* This macro enables a  instance.
*
* @param BaseAddress is the Xilinx EDK base address of the Gamma core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void GAMMA_Enable(u32 BaseAddress);
*
******************************************************************************/
#define GAMMA_Enable(BaseAddress) \
            GAMMA_WriteReg(BaseAddress, GAMMA_CONTROL,  \
            GAMMA_ReadReg(BaseAddress, GAMMA_CONTROL) | \
            GAMMA_CTL_EN_MASK)

/*****************************************************************************/

/**
*
* This macro disables a Gamma instance.
*
* @param BaseAddress is the Xilinx EDK base address of the Gamma core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void GAMMA_Disable(u32 BaseAddress);
*
******************************************************************************/
#define GAMMA_Disable(BaseAddress) \
            GAMMA_WriteReg(BaseAddress, GAMMA_CONTROL, \
            GAMMA_ReadReg(BaseAddress, GAMMA_CONTROL) & \
            ~GAMMA_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro tells a Gamma instance to pick up all the register value changes
* made so far by the software. The registers will be automatically updated
* on the next rising-edge of the VBlank_in signal on the core.
* It is up to the user to manually disable the register update after a sufficient
* amount if time.
*
* This function only works when the Gamma core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the Gamma core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void GAMMA_RegUpdateEnable(u32 BaseAddress);
*
******************************************************************************/
#define GAMMA_RegUpdateEnable(BaseAddress) \
            GAMMA_WriteReg(BaseAddress, GAMMA_CONTROL, \
                GAMMA_ReadReg(BaseAddress, GAMMA_CONTROL) | \
                GAMMA_CTL_RUE_MASK)

/*****************************************************************************/
/**
*
* This macro tells a Gamma instance not to update it's configuration registers made
* so far by the software. When disabled, changes to other configuration registers
* are stored, but do not effect the core's behavior. 
*
* This function only works when the Gamma core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the Gamma core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void GAMMA_RegUpdateDisable(u32 BaseAddress);
*
******************************************************************************/
#define GAMMA_RegUpdateDisable(BaseAddress) \
            GAMMA_WriteReg(BaseAddress, GAMMA_CONTROL, \
                GAMMA_ReadReg(BaseAddress, GAMMA_CONTROL) & \
                ~GAMMA_CTL_RUE_MASK)

/*****************************************************************************/
/**
*
* This macro enables Bypass mode.
*
* @param BaseAddress is the Xilinx EDK base address of the Gamma core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void GAMMA_BypassEnable(u32 BaseAddress);
*
******************************************************************************/
#define GAMMA_BypassEnable(BaseAddress) \
            GAMMA_WriteReg(BaseAddress, GAMMA_CONTROL,  \
            GAMMA_ReadReg(BaseAddress, GAMMA_CONTROL) | \
            GAMMA_CTL_BPE_MASK)

/*****************************************************************************/
/**
*
* This macro disables Bypass mode
*
* @param BaseAddress is the Xilinx EDK base address of the Gamma core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void GAMMA_BypassDisable(u32 BaseAddress);
*
******************************************************************************/
#define GAMMA_BypassDisable(BaseAddress) \
            GAMMA_WriteReg(BaseAddress, GAMMA_CONTROL, \
            GAMMA_ReadReg(BaseAddress, GAMMA_CONTROL) & \
            ~GAMMA_CTL_BPE_MASK)
/*****************************************************************************/
/**
*
* This macro enables Test Pattern Input.
*
* @param BaseAddress is the Xilinx EDK base address of the Gamma core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void GAMMA_TestPatternEnable(u32 BaseAddress);
*
******************************************************************************/
#define GAMMA_TestPatternEnable(BaseAddress) \
            GAMMA_WriteReg(BaseAddress, GAMMA_CONTROL,  \
            GAMMA_ReadReg(BaseAddress, GAMMA_CONTROL) | \
            GAMMA_CTL_TPE_MASK)

/*****************************************************************************/
/**
*
* This macro disables Test Pattern Input
*
* @param BaseAddress is the Xilinx EDK base address of the Gamma core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void GAMMA_TestPatternDisable(u32 BaseAddress);
*
******************************************************************************/
#define GAMMA_TestPatternDisable(BaseAddress) \
            GAMMA_WriteReg(BaseAddress, GAMMA_CONTROL, \
            GAMMA_ReadReg(BaseAddress, GAMMA_CONTROL) & \
            ~GAMMA_CTL_TPE_MASK)

/*****************************************************************************/
/**
*
* This macro resets a Gamma instance. This reset effects the core immediately,
* and may cause image tearing.
*
* This reset resets the Gamma's configuration registers, and holds the core's outputs
* in their reset state until GAMMA_ClearReset() is called.
* 
*
* @param BaseAddress is the Xilinx EDK base address of the Gamma core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void GAMMA_Reset(u32 BaseAddress);
*
******************************************************************************/
#define GAMMA_Reset(BaseAddress) \
            GAMMA_WriteReg(BaseAddress, GAMMA_CONTROL, GAMMA_RST_RESET) \

/*****************************************************************************/
/**
*
* This macro clears the Gamma's reset flag (which is set using GAMMA_Reset(), and
* returns it to normal operation. This ClearReset effects the core immediately,
* and may cause image tearing.
* 
*
* @param BaseAddress is the Xilinx EDK base address of the Gamma core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void GAMMA_ClearReset(u32 BaseAddress);
*
******************************************************************************/
#define GAMMA_ClearReset(BaseAddress) \
            GAMMA_WriteReg(BaseAddress, GAMMA_CONTROL, 0) \


/*****************************************************************************/
/**
*
* This macro resets a Gamma instance, but differs from GAMMA_Reset() in that it
* automatically synchronizes to the VBlank_in input of the core to prevent tearing.
*
* On the next rising-edge of VBlank_in following a call to GAMMA_AutoSyncReset(), 
* all of the core's configuration registers and outputs will be reset, then the
* reset flag will be immediately released, allowing the core to immediately resume
* default operation.
*
* @param BaseAddress is the Xilinx EDK base address of the Gamma core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void GAMMA_Reset(u32 BaseAddress);
*
******************************************************************************/
#define GAMMA_AutoSyncReset(BaseAddress) \
            GAMMA_WriteReg(BaseAddress, GAMMA_CONTROL, GAMMA_RST_AUTORESET) \

/*****************************************************************************/
/**
*
* Read the given register.
*
* @param BaseAddress is the Xilinx EDK base address of the Gamma core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
*
* @return   The 32-bit value of the register
*
* @note
* C-style signature:
*    u32 GAMMA_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define GAMMA_ReadReg(BaseAddress, RegOffset) \
            GAMMA_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* Write the given register.
*
* @param BaseAddress is the Xilinx EDK base address of the Gamma core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
* @param Data is the 32-bit value to write to the register
*
* @return   None.
*
* @note
* C-style signature:
*    void GAMMA_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define GAMMA_WriteReg(BaseAddress, RegOffset, Data) \
            GAMMA_Out32((BaseAddress) + (RegOffset), (Data))


/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
