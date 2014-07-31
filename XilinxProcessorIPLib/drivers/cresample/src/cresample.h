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

/**
*
* @file cresample.h
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx Chroma Resampler core instance.
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 2.00a vy   04/24/12  Updated for version 2.00.a
*                      Converted from xio.h to xil_io.h, translating
*                      basic type, MB cache functions, exceptions and
*                      assertion to xil_io format.
* 1.00a vy   10/22/10  Initial version
* 3.0   adk  19/12/13 Updated as per the New Tcl API's
*
******************************************************************************/

#ifndef CRESAMPLE_DRIVER_H        /* prevent circular inclusions */
#define CRESAMPLE_DRIVER_H        /* by using protection macros */

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
#define CRESAMPLE_CONTROL        0x0000    /**< Control        */
#define CRESAMPLE_STATUS         0x0004    /**< Status         */
#define CRESAMPLE_ERROR          0x0008    /**< Error          */
#define CRESAMPLE_IRQ_ENABLE     0x000C    /**< IRQ Enable     */
#define CRESAMPLE_VERSION        0x0010    /**< Version        */
#define CRESAMPLE_SYSDEBUG0      0x0014    /**< System Debug 0 */
#define CRESAMPLE_SYSDEBUG1      0x0018    /**< System Debug 1 */
#define CRESAMPLE_SYSDEBUG2      0x001C    /**< System Debug 2 */
/* Timing Control Registers */
#define CRESAMPLE_ACTIVE_SIZE    0x0020    /**< Horizontal and Vertical Active Frame Size */
#define CRESAMPLE_ENCODING       0x0028    /**< Frame Encoding */
/* Core Specific Registers */
#define CRESAMPLE_COEF00_HPHASE0      0x0100   /**< Coefficient 00 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF01_HPHASE0      0x0104   /**< Coefficient 01 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF02_HPHASE0      0x0108   /**< Coefficient 02 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF03_HPHASE0      0x010C   /**< Coefficient 03 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF04_HPHASE0      0x0110   /**< Coefficient 04 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF05_HPHASE0      0x0114   /**< Coefficient 05 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF06_HPHASE0      0x0118   /**< Coefficient 06 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF07_HPHASE0      0x011C   /**< Coefficient 07 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF08_HPHASE0      0x0120   /**< Coefficient 08 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF09_HPHASE0      0x0124   /**< Coefficient 09 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF10_HPHASE0      0x0128   /**< Coefficient 10 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF11_HPHASE0      0x012C   /**< Coefficient 11 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF12_HPHASE0      0x0130   /**< Coefficient 12 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF13_HPHASE0      0x0134   /**< Coefficient 13 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF14_HPHASE0      0x0138   /**< Coefficient 14 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF15_HPHASE0      0x013C   /**< Coefficient 15 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF16_HPHASE0      0x0140   /**< Coefficient 16 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF17_HPHASE0      0x0144   /**< Coefficient 17 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF18_HPHASE0      0x0148   /**< Coefficient 18 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF19_HPHASE0      0x014C   /**< Coefficient 19 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF20_HPHASE0      0x0150   /**< Coefficient 20 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF21_HPHASE0      0x0154   /**< Coefficient 21 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF22_HPHASE0      0x0158   /**< Coefficient 22 of Horizontal Phase 0 Filter */
#define CRESAMPLE_COEF23_HPHASE0      0x015C   /**< Coefficient 23 of Horizontal Phase 0 Filter */

#define CRESAMPLE_COEF00_HPHASE1      0x0160   /**< Coefficient 00 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF01_HPHASE1      0x0164   /**< Coefficient 01 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF02_HPHASE1      0x0168   /**< Coefficient 02 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF03_HPHASE1      0x016C   /**< Coefficient 03 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF04_HPHASE1      0x0170   /**< Coefficient 04 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF05_HPHASE1      0x0174   /**< Coefficient 05 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF06_HPHASE1      0x0178   /**< Coefficient 06 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF07_HPHASE1      0x017C   /**< Coefficient 07 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF08_HPHASE1      0x0180   /**< Coefficient 08 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF09_HPHASE1      0x0184   /**< Coefficient 09 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF10_HPHASE1      0x0188   /**< Coefficient 10 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF11_HPHASE1      0x018C   /**< Coefficient 11 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF12_HPHASE1      0x0190   /**< Coefficient 12 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF13_HPHASE1      0x0194   /**< Coefficient 13 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF14_HPHASE1      0x0198   /**< Coefficient 14 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF15_HPHASE1      0x019C   /**< Coefficient 15 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF16_HPHASE1      0x01A0   /**< Coefficient 16 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF17_HPHASE1      0x01A4   /**< Coefficient 17 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF18_HPHASE1      0x01A8   /**< Coefficient 18 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF19_HPHASE1      0x01AC   /**< Coefficient 19 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF20_HPHASE1      0x01B0   /**< Coefficient 20 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF21_HPHASE1      0x01B4   /**< Coefficient 21 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF22_HPHASE1      0x01B8   /**< Coefficient 22 of Horizontal Phase 1 Filter */
#define CRESAMPLE_COEF23_HPHASE1      0x01BC   /**< Coefficient 23 of Horizontal Phase 1 Filter */

#define CRESAMPLE_COEF00_VPHASE0      0x01C0   /**< Coefficient 00 of Vertical Phase 0 Filter */
#define CRESAMPLE_COEF01_VPHASE0      0x01C4   /**< Coefficient 01 of Vertical Phase 0 Filter */
#define CRESAMPLE_COEF02_VPHASE0      0x01C8   /**< Coefficient 02 of Vertical Phase 0 Filter */
#define CRESAMPLE_COEF03_VPHASE0      0x01CC   /**< Coefficient 03 of Vertical Phase 0 Filter */
#define CRESAMPLE_COEF04_VPHASE0      0x01D0   /**< Coefficient 04 of Vertical Phase 0 Filter */
#define CRESAMPLE_COEF05_VPHASE0      0x01D4   /**< Coefficient 05 of Vertical Phase 0 Filter */
#define CRESAMPLE_COEF06_VPHASE0      0x01D8   /**< Coefficient 06 of Vertical Phase 0 Filter */
#define CRESAMPLE_COEF07_VPHASE0      0x01DC   /**< Coefficient 07 of Vertical Phase 0 Filter */

#define CRESAMPLE_COEF00_VPHASE1      0x01E0   /**< Coefficient 00 of Vertical Phase 1 Filter */
#define CRESAMPLE_COEF01_VPHASE1      0x01E4   /**< Coefficient 01 of Vertical Phase 1 Filter */
#define CRESAMPLE_COEF02_VPHASE1      0x01E8   /**< Coefficient 02 of Vertical Phase 1 Filter */
#define CRESAMPLE_COEF03_VPHASE1      0x01EC   /**< Coefficient 03 of Vertical Phase 1 Filter */
#define CRESAMPLE_COEF04_VPHASE1      0x01F0   /**< Coefficient 04 of Vertical Phase 1 Filter */
#define CRESAMPLE_COEF05_VPHASE1      0x01F4   /**< Coefficient 05 of Vertical Phase 1 Filter */
#define CRESAMPLE_COEF06_VPHASE1      0x01F8   /**< Coefficient 06 of Vertical Phase 1 Filter */
#define CRESAMPLE_COEF07_VPHASE1      0x01FC   /**< Coefficient 07 of Vertical Phase 1 Filter */

/*****************************************************************************/
/**
 * Control Register bit definition
 */
#define CRESAMPLE_CTL_EN_MASK    0x00000001 /**< Enable */
#define CRESAMPLE_CTL_RU_MASK    0x00000002 /**< Register Update */
#define CRESAMPLE_CTL_AUTORESET  0x40000000 /**< Software Reset - Auto-synchronize to SOF */
#define CRESAMPLE_CTL_RESET      0x80000000 /**< Software Reset - Instantaneous */

/***************** Macros (Inline Functions) Definitions *********************/
#define CRESAMPLE_In32          Xil_In32
#define CRESAMPLE_Out32         Xil_Out32

/*****************************************************************************/
/**
*
* Read the given register.
*
* @param BaseAddress is the Xilinx EDK base address of the Chroma Resampler core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
*
* @return   The 32-bit value of the register
*
* @note
* C-style signature:
*    u32 CRESAMPLE_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define CRESAMPLE_ReadReg(BaseAddress, RegOffset) \
            CRESAMPLE_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* Write the given register.
*
* @param BaseAddress is the Xilinx EDK base address of the Chroma Resampler core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
* @param Data is the 32-bit value to write to the register
*
* @return   None.
*
* @note
* C-style signature:
*    void CRESAMPLE_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define CRESAMPLE_WriteReg(BaseAddress, RegOffset, Data) \
            CRESAMPLE_Out32((BaseAddress) + (RegOffset), (Data))

/*****************************************************************************/
/**
*
* This macro enables a Chroma Resampler core instance.
*
* @param BaseAddress is the Xilinx EDK base address of the Chroma Resampler core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CRESAMPLE_Enable(u32 BaseAddress);
*
******************************************************************************/
#define CRESAMPLE_Enable(BaseAddress) \
            CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_CONTROL, \
            	CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_CONTROL) | \
            	CRESAMPLE_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro disables a Chroma Resampler core instance.
*
* @param BaseAddress is the Xilinx EDK base address of the Chroma Resampler core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CRESAMPLE_Disable(u32 BaseAddress);
*
******************************************************************************/
#define CRESAMPLE_Disable(BaseAddress) \
            CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_CONTROL, \
            	CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_CONTROL) & \
            	~CRESAMPLE_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro commits all the register value changes made so far by the software 
* to the Chroma Resampler core instance. The registers will be automatically updated
* on the next rising-edge of the SOF signal on the core.
* It is up to the user to manually disable the register update after a sufficient
* amount of time.
*
* This function only works when the Chroma Resampler core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the Chroma Resampler core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CRESAMPLE_RegUpdateEnable(u32 BaseAddress);
*
******************************************************************************/
#define CRESAMPLE_RegUpdateEnable(BaseAddress) \
            CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_CONTROL, \
                CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_CONTROL) | \
                CRESAMPLE_CTL_RU_MASK)

/*****************************************************************************/
/**
*
* This macro prevents the Chroma Resampler core instance from committing recent changes made 
* so far by the software. When disabled, changes to other configuration registers
* are stored, but do not effect the behavior of the core. 
*
* This function only works when the Chroma Resampler core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the Chroma Resampler core (from xparameters.h)
*
* @return None.
*
* @note 
* C-style signature:
*    void CRESAMPLE_RegUpdateDisable(u32 BaseAddress);
*
******************************************************************************/
#define CRESAMPLE_RegUpdateDisable(BaseAddress) \
            CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_CONTROL, \
                CRESAMPLE_ReadReg(BaseAddress, CRESAMPLE_CONTROL) & \
                ~CRESAMPLE_CTL_RU_MASK)

/*****************************************************************************/

/**
*
* This macro resets a Chroma Resampler core instance. This reset effects the core immediately,
* and may cause image tearing.
*
* This reset resets the Chroma Resampler's configuration registers, and holds the core's outputs
* in their reset state until CRESAMPLE_ClearReset() is called.
* 
*
* @param BaseAddress is the Xilinx EDK base address of the Chroma Resampler core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CRESAMPLE_Reset(u32 BaseAddress);
*
******************************************************************************/
#define CRESAMPLE_Reset(BaseAddress) \
            CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_CONTROL, CRESAMPLE_CTL_RESET) \

/*****************************************************************************/
/**
*
* This macro clears the Chroma Resampler's reset flag (which is set using CRESAMPLE_Reset(), and
* returns it to normal operation. This ClearReset effects the core immediately,
* and may cause image tearing.
* 
*
* @param BaseAddress is the Xilinx EDK base address of the Chroma Resampler core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CRESAMPLE_ClearReset(u32 BaseAddress);
*
******************************************************************************/
#define CRESAMPLE_ClearReset(BaseAddress) \
            CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_CONTROL, 0) \

/*****************************************************************************/
/**
*
* This macro resets a Chroma Resampler core instance, but differs from CRESAMPLE_Reset() in that it
* automatically synchronizes to the SOF input of the core to prevent tearing.
*
* On the next SOF following a call to CRESAMPLE_AutoSyncReset(), 
* all of the core's configuration registers and outputs will be reset, then the
* reset flag will be immediately released, allowing the core to immediately resume
* default operation.
*
* @param BaseAddress is the Xilinx EDK base address of the Chroma Resampler core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void CRESAMPLE_AutoSyncReset(u32 BaseAddress);
*
******************************************************************************/
#define CRESAMPLE_AutoSyncReset(BaseAddress) \
            CRESAMPLE_WriteReg(BaseAddress, CRESAMPLE_CONTROL, CRESAMPLE_CTL_AUTORESET) \


/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */ 
