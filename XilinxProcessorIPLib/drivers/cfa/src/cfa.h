/******************************************************************************
*
* Copyright (C) 2001 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file cfa.h
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx Color Filter Array Interpolation 
* (CFA) core instance.
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.00a se   12/01/11  Updated for CFA v5.0, replaced xio.h with xil_io.h
* 4.00a rc   09/11/11  Updated for CFA v4.0
* 3.00a gz   10/22/10  Updated for CFA V3.0
* 6.0   adk  19/12/13 Updated as per the New Tcl API's
*
******************************************************************************/

#ifndef CFA_DRIVER_H        /* prevent circular inclusions */
#define CFA_DRIVER_H        /* by using protection macros */

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
#define CFA_CONTROL             0x000    /**< Control (R/W) */
#define CFA_STATUS              0x004    /**< Status (R/W) */
#define CFA_ERROR               0x008    /**< Error (R/W) */
#define CFA_IRQ_EN              0x00C    /**< IRQ Enable     */
#define CFA_VERSION             0x010    /**< Version        */
#define CFA_SYSDEBUG0           0x014    /**< System Debug 0 */
#define CFA_SYSDEBUG1           0x018    /**< System Debug 1 */
#define CFA_SYSDEBUG2           0x01C    /**< System Debug 2 */
/* Timing Control Registers */
#define CFA_ACTIVE_SIZE         0x020    /**< Active Size (V x H)       */
/* Core Specific Registers */
#define CFA_BAYER_PHASE         0x100    /**< bayer_phase R/W user register */


/**
 * CFA Control Register bit definition
 */
#define CFA_CTL_EN_MASK    0x00000001    /**< CFA Enable */
#define CFA_CTL_RUE_MASK   0x00000002    /**< CFA Register Update */
#define CFA_CTL_CS_MASK    0x00000004    /**< CFA Register Clear Status */
                                         
/**
 * CFA Reset Register bit definition     
 */                                      
#define CFA_RST_RESET      0x80000000    /**< Software Reset - Instantaneous */
#define CFA_RST_AUTORESET  0x40000000    /**< Software Reset - Auto-synchronize to SOF */

/***************** Macros (Inline Functions) Definitions *********************/
#define CFA_In32          Xil_In32
#define CFA_Out32         Xil_Out32


/**
*
* Read the given register.
*
* @param BaseAddress is the Xilinx EDK base address of the CFA core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
*
* @return   The 32-bit value of the register
*
* @note
* C-style signature:
*    u32 CFA_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define CFA_ReadReg(BaseAddress, RegOffset) \
            CFA_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* Write the given register.
*
* @param BaseAddress is the Xilinx EDK base address of the CFA core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
* @param Data is the 32-bit value to write to the register
*
* @return   None.
*
* @note
* C-style signature:
*    void CFA_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define CFA_WriteReg(BaseAddress, RegOffset, Data) \
            CFA_Out32((BaseAddress) + (RegOffset), (Data))

/*****************************************************************************/
/**
*
* This macro enables a CFA core instance.
*
* @param BaseAddress is the Xilinx EDK base address of the CFA core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CFA_Enable(u32 BaseAddress);
*
******************************************************************************/
#define CFA_Enable(BaseAddress) \
            CFA_WriteReg(BaseAddress, CFA_CONTROL, \
            	CFA_ReadReg(BaseAddress, CFA_CONTROL) | \
            	CFA_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro disables a CFA core instance.
*
* @param BaseAddress is the Xilinx EDK base address of the CFA core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CFA_Disable(u32 BaseAddress);
*
******************************************************************************/
#define CFA_Disable(BaseAddress) \
            CFA_WriteReg(BaseAddress, CFA_CONTROL, \
            	CFA_ReadReg(BaseAddress, CFA_CONTROL) & \
            	~CFA_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro commits all the register value changes made so far by the software 
* to the CFA core instance. The registers will be automatically updated
* on the next rising-edge of the VBlank_in signal on the core.
* It is up to the user to manually disable the register update after a sufficient
* amount if time.
*
* This function only works when the CFA core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the CFA core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CFA_RegUpdateEnable(u32 BaseAddress);
*
******************************************************************************/
#define CFA_RegUpdateEnable(BaseAddress) \
            CFA_WriteReg(BaseAddress, CFA_CONTROL, \
                CFA_ReadReg(BaseAddress, CFA_CONTROL) | \
                CFA_CTL_RUE_MASK)

/*****************************************************************************/
/**
*
* This macro prevents the CFA core instance from committing recent changes made 
* so far by the software. When disabled, changes to other configuration registers
* are stored, but do not effect the behavior of the core. 
*
* This function only works when the CFA core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the CFA core (from xparameters.h)
*
* @return None.
*
* @note 
* C-style signature:
*    void CFA_RegUpdateDisable(u32 BaseAddress);
*
******************************************************************************/
#define CFA_RegUpdateDisable(BaseAddress) \
            CFA_WriteReg(BaseAddress, CFA_CONTROL, \
                CFA_ReadReg(BaseAddress, CFA_CONTROL) & \
                ~CFA_CTL_RUE_MASK)

/*****************************************************************************/

/**
*
* This macro clears the status register of the CFA instance, by first asserting then
* deasserting the CLEAR_STATUS flag of CFA_CONTROL. 
* This function only works when the CFA core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the CFA core (from xparameters.h)
*
* @return None.
*
* @note 
* C-style signature:
*    void CFA_ClearStatus(u32 BaseAddress);
*
******************************************************************************/
#define CFA_ClearStatus(BaseAddress) \
   CFA_WriteReg(BaseAddress, CFA_CONTROL, CFA_ReadReg(BaseAddress, CFA_CONTROL) |  CFA_CTL_CS_MASK); \
   CFA_WriteReg(BaseAddress, CFA_CONTROL, CFA_ReadReg(BaseAddress, CFA_CONTROL) & ~CFA_CTL_CS_MASK) 

/*****************************************************************************/


/**
*
* This macro resets a CFA core instance. This reset effects the core immediately,
* and may cause image tearing.
*
* This reset resets the CFA's configuration registers, and holds the core's outputs
* in their reset state until CFA_ClearReset() is called.
* 
*
* @param BaseAddress is the Xilinx EDK base address of the CFA core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CFA_Reset(u32 BaseAddress);
*
******************************************************************************/
#define CFA_Reset(BaseAddress) \
            CFA_WriteReg(BaseAddress, CFA_CONTROL, CFA_RST_RESET) \

/*****************************************************************************/
/**
*
* This macro clears the CFA's reset flag (which is set using CFA_Reset(), and
* returns it to normal operation. This ClearReset effects the core immediately,
* and may cause image tearing.
* 
*
* @param BaseAddress is the Xilinx EDK base address of the CFA core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CFA_ClearReset(u32 BaseAddress);
*
******************************************************************************/
#define CFA_ClearReset(BaseAddress) \
            CFA_WriteReg(BaseAddress, CFA_CONTROL, 0) \

/*****************************************************************************/
/**
*
* This macro resets a CFA instance, but differs from CFA_Reset() in that it
* automatically synchronizes to the SOF of the core to prevent tearing.
*
* On the next rising-edge of SOF following a call to CFA_AutoSyncReset(), 
* all of the core's configuration registers and outputs will be reset, then the
* reset flag will be immediately released, allowing the core to immediately resume
* default operation.
*
* @param BaseAddress is the Xilinx EDK base address of the CFA core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CFA_FSyncReset(u32 BaseAddress);
*
******************************************************************************/
#define CFA_FSyncReset(BaseAddress) \
            CFA_WriteReg(BaseAddress, CFA_CONTROL, CFA_RST_AUTORESET) \

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */ 
