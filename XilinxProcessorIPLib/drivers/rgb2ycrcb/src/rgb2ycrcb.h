/******************************************************************************
*
* Copyright (C) 2012 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file rgb2ycrcb.h
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx RGB to YCrCb Color Space Converter 
* (RGB2YCRCB) device.
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.00a tb   02/27/12 Updated for RGB2YCRCB V5.00.a
* 5.01a bao  12/28/12 Converted from xio.h to xil_io.h, translating basic types,
* 		      MB cache functions, exceptions and assertions to xil_io
* 		      format
* 6.0   adk  19/12/13 Updated as per the New Tcl API's
*
******************************************************************************/

#ifndef RGB2YCRCB_DRIVER_H        /* prevent circular inclusions */
#define RGB2YCRCB_DRIVER_H        /* by using protection macros */

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
#define RGB_CONTROL        0x000    /**< Control        */
#define RGB_STATUS         0x004    /**< Status         */
#define RGB_ERROR          0x008    /**< Error          */
#define RGB_IRQ_EN         0x00C    /**< IRQ Enable     */
#define RGB_VERSION        0x010    /**< Version        */
#define RGB_SYSDEBUG0      0x014    /**< System Debug 0 */
#define RGB_SYSDEBUG1      0x018    /**< System Debug 1 */
#define RGB_SYSDEBUG2      0x01C    /**< System Debug 2 */
/* Timing Control Registers */
#define RGB_ACTIVE_SIZE    0x020    /**< Active Size (V x H)       */
#define RGB_TIMING_STATUS  0x024    /**< Timing Measurement Status */
/* Core Specific Registers */
#define RGB_YMAX           0x100    /**< Luma Clipping */
#define RGB_YMIN           0x104    /**< Luma Clamping */
#define RGB_CBMAX          0x108    /**< Cb Clipping   */
#define RGB_CBMIN          0x10C    /**< Cb Clamping   */
#define RGB_CRMAX          0x110    /**< Cr Clipping   */
#define RGB_CRMIN          0x114    /**< Cr Clamping   */
#define RGB_YOFFSET        0x118    /**< Lumma Offset  */
#define RGB_CBOFFSET       0x11C    /**< Cb Offset     */
#define RGB_CROFFSET       0x120    /**< Cr Offset     */
#define RGB_ACOEF          0x124    /**< Matrix Coversion Coefficient */
#define RGB_BCOEF          0x128    /**< Matrix Coversion Coefficient */
#define RGB_CCOEF          0x12C    /**< Matrix Coversion Coefficient */
#define RGB_DCOEF          0x130    /**< Matrix Coversion Coefficient */

/*
 * CCM Control Register bit definition
 */
#define RGB_CTL_EN_MASK     0x00000001 /**< CCM Enable */
#define RGB_CTL_RUE_MASK    0x00000002 /**< CCM Register Update Enable */

/*
 * CCM Reset Register bit definition
 */
#define RGB_RST_RESET      0x80000000 /**< Software Reset - Instantaneous */
#define RGB_RST_AUTORESET  0x40000000 /**< Software Reset - Auto-synchronize to SOF */


/***************** Macros (Inline Functions) Definitions *********************/

#define RGB_In32          Xil_In32
#define RGB_Out32         Xil_Out32


/*****************************************************************************/
/**
*
* This macro enables a RGB2YCrCb device.
*
* @param BaseAddress is the Xilinx EDK base address of the RGB2YCrCb core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void RGB_Enable(u32 BaseAddress);
*
******************************************************************************/
#define RGB_Enable(BaseAddress) \
            RGB_WriteReg(BaseAddress, RGB_CONTROL, \
            	RGB_ReadReg(BaseAddress, RGB_CONTROL) | \
            	RGB_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro disables a RGB2YCrCb device.
*
* @param BaseAddress is the Xilinx EDK base address of the RGB2YCrCb core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void RGB_Disable(u32 BaseAddress);
*
******************************************************************************/
#define RGB_Disable(BaseAddress) \
            RGB_WriteReg(BaseAddress, RGB_CONTROL, \
            	RGB_ReadReg(BaseAddress, RGB_CONTROL) & \
            	~RGB_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro tells a RGB2YCrCb device to pick up all the register value changes
* made so far by the software. The registers will be automatically updated
* on the next SOF signal on the core.
* It is up to the user to manually disable the register update after a sufficient
* amount if time.
*
* This function only works when the RGB2YCrCb core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the RGB2YCrCb core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void RGB_RegUpdateEnable(u32 BaseAddress);
*
******************************************************************************/
#define RGB_RegUpdateEnable(BaseAddress) \
            RGB_WriteReg(BaseAddress, RGB_CONTROL, \
                RGB_ReadReg(BaseAddress, RGB_CONTROL) | \
                RGB_CTL_RUE_MASK)

/*****************************************************************************/
/**
*
* This macro tells a RGB2YCrCb device not to update it's configuration registers made
* so far by the software. When disabled, changes to other configuration registers
* are stored, but do not effect the core's behavior. 
*
* This function only works when the RGB2YCrCb core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the RGB2YCrCb core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void RGB_RegUpdateDisable(u32 BaseAddress);
*
******************************************************************************/
#define RGB_RegUpdateDisable(BaseAddress) \
            RGB_WriteReg(BaseAddress, RGB_CONTROL, \
                RGB_ReadReg(BaseAddress, RGB_CONTROL) & \
                ~RGB_CTL_RUE_MASK)

/*****************************************************************************/
/**
*
* This macro resets a RGB2YCrCb device. This reset effects the core immediately,
* and may cause image tearing.
*
* This reset resets the RGB2YCrCb's configuration registers, and holds the core's outputs
* in their reset state until RGB_ClearReset() is called.
* 
*
* @param BaseAddress is the Xilinx EDK base address of the RGB2YCrCb core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void RGB_Reset(u32 BaseAddress);
*
******************************************************************************/
#define RGB_Reset(BaseAddress) \
            RGB_WriteReg(BaseAddress, RGB_CONTROL, RGB_RST_RESET) \

/*****************************************************************************/
/**
*
* This macro clears the RGB2YCrCb's reset flag (which is set using RGB_Reset(), and
* returns it to normal operation. This ClearReset effects the core immediately,
* and may cause image tearing.
* 
*
* @param BaseAddress is the Xilinx EDK base address of the RGB2YCrCb core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void RGB_ClearReset(u32 BaseAddress);
*
******************************************************************************/
#define RGB_ClearReset(BaseAddress) \
            RGB_WriteReg(BaseAddress, RGB_CONTROL, 0) \


/*****************************************************************************/
/**
*
* This macro resets a RGB2YCrCb device, but differs from RGB_Reset() in that it
* automatically synchronizes to the VBlank_in input of the core to prevent tearing.
*
* On the next rising-edge of VBlank_in following a call to RGB_AutoSyncReset(), 
* all of the core's configuration registers and outputs will be reset, then the
* reset flag will be immediately released, allowing the core to immediately resume
* default operation.
*
* @param BaseAddress is the Xilinx EDK base address of the RGB2YCrCb core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void RGB_Reset(u32 BaseAddress);
*
******************************************************************************/
#define RGB_AutoSyncReset(BaseAddress) \
            RGB_WriteReg(BaseAddress, RGB_CONTROL, RGB_RST_AUTORESET) \

/*****************************************************************************/
/**
*
* Read the given register.
*
* @param BaseAddress is the Xilinx EDK base address of the RGB2YCrCb core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
*
* @return   The 32-bit value of the register
*
* @note
* C-style signature:
*    u32 RGB_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define RGB_ReadReg(BaseAddress, RegOffset) \
            RGB_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* Write the given register.
*
* @param BaseAddress is the Xilinx EDK base address of the RGB2YCrCb core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
* @param Data is the 32-bit value to write to the register
*
* @return   None.
*
* @note
* C-style signature:
*    void RGB_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define RGB_WriteReg(BaseAddress, RegOffset, Data) \
            RGB_Out32((BaseAddress) + (RegOffset), (Data))

/************************** Function Prototypes ******************************/

struct rgb_coef_inputs
{
  /* Pre-translated coefficient/offset data */
  double  acoef;          //@- [ 0.0 - 1.0 ]       0.0 < ACOEFF + BCOEFF < 1.0
  double  bcoef;          //@- [ 0.0 - 1.0 ]       0.0 < ACOEFF + BCOEFF < 1.0
  double  ccoef;          //@- [ 0.0 - 0.9 ]
  double  dcoef;          //@- [ 0.0 - 0.9 ]
  u32     yoffset;        //@- Offset for the Luminance Channel
  u32     cboffset;       //@- Offset for the Chrominance Channels
  u32     croffset;       //@- Offset for the Chrominance Channels
  u32     ymax;           //@- Y Clipping
  u32     ymin;           //@- Y Clamping
  u32     cbmax;          //@- Cb Clipping
  u32     cbmin;          //@- Cb Clamping
  u32     crmax;          //@- Cr Clipping
  u32     crmin;          //@- Cr Clamping
};

struct rgb_coef_outputs
{
  /* Translated coefficient/offset data */
  u32     acoef;        //@- Translated ACoef
  u32     bcoef;        //@- Translated BCoef
  u32     ccoef;        //@- Translated CCoef
  u32     dcoef;        //@- Translated DCoef
  u32     yoffset;      //@- Translated Offset for the Luminance Channel
  u32     cboffset;     //@- Translated Offset for the Chrominance Channels
  u32     croffset;     //@- Translated Offset for the Chrominance Channels
  u32     ymax;         //@- Translated Y Clipping
  u32     ymin;         //@- Translated Y Clamping
  u32     cbmax;        //@- Translated Cb Clipping
  u32     cbmin;        //@- Translated Cb Clamping
  u32     crmax;        //@- Translated Cr Clipping
  u32     crmin;        //@- Translated Cr Clamping
};

/*****************************************************************************/
/**
*
* Select input coefficients for 4 supported Standards and 3 Input Ranges.
*
* @param standard_sel is the standards selection: 0 = SD_ITU_601 
*                                                 1 = HD_ITU_709__1125_NTSC
*                                                 2 = HD_ITU_709__1250_PAL
*                                                 3 = YUV
* @param input_range is the limit on the range of the data: 0 = 16_to_240_for_TV, 
*                                                           1 = 16_to_235_for_Studio_Equipment, 
*                                                           3 = 0_to_255_for_Computer_Graphics
* @param data_width has a valid range of [8, 10,12,16]
* @param coef_in is a pointer to a rgb_coef_inputs data structure.
*
* @return   None.
*
* @note
*
******************************************************************************/
void RGB_select_standard(u32 standard_sel, u32 input_range, u32 data_width, struct rgb_coef_inputs *coef_in);


/*****************************************************************************/
/**
*
* Translate input coefficients into coefficients that can be programmed into the 
* RGB2YCrCb core.
*
* @param coef_in is a pointer to a rgb_coef_inputs data structure.
* @param coef_out is a pointer to a rgb_coef_output data structure.
*
* @return   The 32-bit value: bit(0)= Acoef + Bcoef > 1.0
*                             bit(1)= Y Offset outside data width range  [-2^data_width, (2^data_width)-1]
*                             bit(2)= Cb Offset outside data width range [-2^data_width, (2^data_width)-1]
*                             bit(3)= Cr Offset outside data width range [-2^data_width, (2^data_width)-1]
*                             bit(4)= Y Max outside data width range  [0, (2^data_width)-1]
*                             bit(5)= Y Min outside data width range  [0, (2^data_width)-1]
*                             bit(6)= Cb Max outside data width range [0, (2^data_width)-1]
*                             bit(7)= Cb Min outside data width range [0, (2^data_width)-1]
*                             bit(8)= Cr Max outside data width range [0, (2^data_width)-1]
*                             bit(9)= Cr Min outside data width range [0, (2^data_width)-1]
*
* @note
*
******************************************************************************/
u32 RGB_coefficient_translation(struct rgb_coef_inputs *coef_in, struct rgb_coef_outputs *coef_out, u32 data_width);


/*****************************************************************************/
/**
*
* Program the RGB2YCrCb coefficient/offset registers.
*
* @param BaseAddress is the Xilinx EDK base address of the RGB2YCrCb core (from xparameters.h)
* @param coef_out is a pointer to a rgb_coef_output data structure.
*
* @return   None.
*
* @note
*
******************************************************************************/
void RGB_set_coefficients(u32 BaseAddress, struct rgb_coef_outputs *coef_out);


/*****************************************************************************/
/**
*
* Read the RGB2YCrCb coefficient/offset registers.
*
* @param BaseAddress is the Xilinx EDK base address of the RGB2YCrCb core (from xparameters.h)
* @param coef_out is a pointer to a rgb_coef_output data structure.
*
* @return   None.
*
* @note
*
******************************************************************************/
void RGB_get_coefficients(u32 BaseAddress, struct rgb_coef_outputs *coef_out);


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
