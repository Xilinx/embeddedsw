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
* macros) that can be used to access the Xilinx YCrCb to RGB Color Space Converter 
* (RGB2YCRCB) device.
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.00a tb   02/28/12 Updated for YCRCB2RGB V5.00.a
* 5.01a bao  12/28/12 Converted from xio.h to xil_io.h, translating basic types,
* 		      MB cache functions, exceptions and assertions to xil_io
* 		      format
* 6.0   adk  19/12/13 Updated as per the New Tcl API's
*
******************************************************************************/

#ifndef YCRCB2RGB_DRIVER_H        /* prevent circular inclusions */
#define YCRCB2RGB_DRIVER_H        /* by using protection macros */

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
#define YCC_CONTROL        0x000    /**< Control        */
#define YCC_STATUS         0x004    /**< Status         */
#define YCC_ERROR          0x008    /**< Error          */
#define YCC_IRQ_EN         0x00C    /**< IRQ Enable     */
#define YCC_VERSION        0x010    /**< Version        */
#define YCC_SYSDEBUG0      0x014    /**< System Debug 0 */
#define YCC_SYSDEBUG1      0x018    /**< System Debug 1 */
#define YCC_SYSDEBUG2      0x01C    /**< System Debug 2 */
/* Timing Control Registers */
#define YCC_ACTIVE_SIZE    0x020    /**< Active Size (V x H)       */
#define YCC_TIMING_STATUS  0x024    /**< Timing Measurement Status */
/* Core Specific Registers */
#define YCC_RGBMAX         0x100    /**< RGB Clipping */
#define YCC_RGBMIN         0x104    /**< RGB Clamping */
#define YCC_ROFFSET        0x108    /**< R Offset     */
#define YCC_GOFFSET        0x10C    /**< G Offset     */
#define YCC_BOFFSET        0x110    /**< B Offset     */
#define YCC_ACOEF          0x114    /**< Matrix Coversion Coefficient */
#define YCC_BCOEF          0x118    /**< Matrix Coversion Coefficient */
#define YCC_CCOEF          0x11C    /**< Matrix Coversion Coefficient */
#define YCC_DCOEF          0x120    /**< Matrix Coversion Coefficient */

/*
 * CCM Control Register bit definition
 */
#define YCC_CTL_EN_MASK     0x00000001 /**< CCM Enable */
#define YCC_CTL_RUE_MASK    0x00000002 /**< CCM Register Update Enable */

/*
 * CCM Reset Register bit definition
 */
#define YCC_RST_RESET      0x80000000 /**< Software Reset - Instantaneous */
#define YCC_RST_AUTORESET  0x40000000 /**< Software Reset - Auto-synchronize to SOF */


/***************** Macros (Inline Functions) Definitions *********************/

#define YCC_In32          Xil_In32
#define YCC_Out32         Xil_Out32


/*****************************************************************************/
/**
*
* This macro enables a YCrCb2RGB device.
*
* @param BaseAddress is the Xilinx EDK base address of the YCrCb2RGB core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void YCC_Enable(u32 BaseAddress);
*
******************************************************************************/
#define YCC_Enable(BaseAddress) \
            YCC_WriteReg(BaseAddress, YCC_CONTROL, \
            	YCC_ReadReg(BaseAddress, YCC_CONTROL) | \
            	YCC_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro disables a YCrCb2RGB device.
*
* @param BaseAddress is the Xilinx EDK base address of the YCrCb2RGB core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void YCC_Disable(u32 BaseAddress);
*
******************************************************************************/
#define YCC_Disable(BaseAddress) \
            YCC_WriteReg(BaseAddress, YCC_CONTROL, \
            	YCC_ReadReg(BaseAddress, YCC_CONTROL) & \
            	~YCC_CTL_EN_MASK)

/*****************************************************************************/
/**
*
* This macro tells a YCrCb2RGB device to pick up all the register value changes
* made so far by the software. The registers will be automatically updated
* on the next SOF signal on the core.
* It is up to the user to manually disable the register update after a sufficient
* amount if time.
*
* This function only works when the YCrCb2RGB core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the YCrCb2RGB core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void YCC_RegUpdateEnable(u32 BaseAddress);
*
******************************************************************************/
#define YCC_RegUpdateEnable(BaseAddress) \
            YCC_WriteReg(BaseAddress, YCC_CONTROL, \
                YCC_ReadReg(BaseAddress, YCC_CONTROL) | \
                YCC_CTL_RUE_MASK)

/*****************************************************************************/
/**
*
* This macro tells a YCrCb2RGB device not to update it's configuration registers made
* so far by the software. When disabled, changes to other configuration registers
* are stored, but do not effect the core's behavior. 
*
* This function only works when the YCrCb2RGB core is enabled.
*
* @param BaseAddress is the Xilinx EDK base address of the YCrCb2RGB core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void YCC_RegUpdateDisable(u32 BaseAddress);
*
******************************************************************************/
#define YCC_RegUpdateDisable(BaseAddress) \
            YCC_WriteReg(BaseAddress, YCC_CONTROL, \
                YCC_ReadReg(BaseAddress, YCC_CONTROL) & \
                ~YCC_CTL_RUE_MASK)

/*****************************************************************************/
/**
*
* This macro resets a YCrCb2RGB device. This reset effects the core immediately,
* and may cause image tearing.
*
* This reset resets the YCrCb2RGB's configuration registers, and holds the core's outputs
* in their reset state until YCC_ClearReset() is called.
* 
*
* @param BaseAddress is the Xilinx EDK base address of the YCrCb2RGB core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void YCC_Reset(u32 BaseAddress);
*
******************************************************************************/
#define YCC_Reset(BaseAddress) \
            YCC_WriteReg(BaseAddress, YCC_CONTROL, YCC_RST_RESET) \

/*****************************************************************************/
/**
*
* This macro clears the YCrCb2RGB's reset flag (which is set using YCC_Reset(), and
* returns it to normal operation. This ClearReset effects the core immediately,
* and may cause image tearing.
* 
*
* @param BaseAddress is the Xilinx EDK base address of the YCrCb2RGB core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void YCC_ClearReset(u32 BaseAddress);
*
******************************************************************************/
#define YCC_ClearReset(BaseAddress) \
            YCC_WriteReg(BaseAddress, YCC_CONTROL, 0) \


/*****************************************************************************/
/**
*
* This macro resets a YCrCb2RGB device, but differs from YCC_Reset() in that it
* automatically synchronizes to the VBlank_in input of the core to prevent tearing.
*
* On the next rising-edge of VBlank_in following a call to YCC_AutoSyncReset(), 
* all of the core's configuration registers and outputs will be reset, then the
* reset flag will be immediately released, allowing the core to immediately resume
* default operation.
*
* @param BaseAddress is the Xilinx EDK base address of the YCrCb2RGB core (from xparameters.h)
*
* @return None.
*
* @note
* C-style signature:
*    void YCC_Reset(u32 BaseAddress);
*
******************************************************************************/
#define YCC_AutoSyncReset(BaseAddress) \
            YCC_WriteReg(BaseAddress, YCC_CONTROL, YCC_RST_AUTORESET) \

/*****************************************************************************/
/**
*
* Read the given register.
*
* @param BaseAddress is the Xilinx EDK base address of the YCrCb2RGB core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
*
* @return   The 32-bit value of the register
*
* @note
* C-style signature:
*    u32 YCC_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define YCC_ReadReg(BaseAddress, RegOffset) \
            YCC_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* Write the given register.
*
* @param BaseAddress is the Xilinx EDK base address of the YCrCb2RGB core (from xparameters.h)
* @param RegOffset is the register offset of the register (defined at top of this file)
* @param Data is the 32-bit value to write to the register
*
* @return   None.
*
* @note
* C-style signature:
*    void YCC_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define YCC_WriteReg(BaseAddress, RegOffset, Data) \
            YCC_Out32((BaseAddress) + (RegOffset), (Data))

/************************** Function Prototypes ******************************/

struct ycc_coef_inputs
{
  /* Pre-translated coefficient/offset data */
  double  acoef;          //@- [ 0.0 - 1.0 ]       0.0 < ACOEFF + BCOEFF < 1.0
  double  bcoef;          //@- [ 0.0 - 1.0 ]       0.0 < ACOEFF + BCOEFF < 1.0
  double  ccoef;          //@- [ 0.0 - 0.9 ]
  double  dcoef;          //@- [ 0.0 - 0.9 ]
  u32     yoffset;        //@- Offset for the Luminance Channel
  u32     cboffset;       //@- Offset for the Chrominance Channels
  u32     croffset;       //@- Offset for the Chrominance Channels
  u32     rgbmax;         //@- RGB Clipping
  u32     rgbmin;         //@- RGB Clamping
};

struct ycc_coef_outputs
{
  /* Translated coefficient/offset data */
  u32     acoef;        //@- Translated ACoef
  u32     bcoef;        //@- Translated BCoef
  u32     ccoef;        //@- Translated CCoef
  u32     dcoef;        //@- Translated DCoef
  u32     roffset;      //@- Translated Offset for the R Channel
  u32     goffset;      //@- Translated Offset for the G Channel
  u32     boffset;      //@- Translated Offset for the B Channel
  u32     rgbmax;       //@- Translated RGB Clipping
  u32     rgbmin;       //@- Translated RGB Clamping
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
* @param coef_in is a pointer to a ycc_coef_inputs data structure.
*
* @return   None.
*
* @note
*
******************************************************************************/
void YCC_select_standard(u32 standard_sel, u32 input_range, u32 data_width, struct ycc_coef_inputs *coef_in);


/*****************************************************************************/
/**
*
* Translate input coefficients into coefficients that can be programmed into the 
* YCrCb2RGB core.
*
* @param coef_in is a pointer to a ycc_coef_inputs data structure.
* @param coef_out is a pointer to a ycc_coef_output data structure.
* @param data_width is the bit width of the data
* @param mwidth is multiplicatio width value calculated by the core.
*
* @return   The 32-bit value: bit(0)= 1=data width outside range [8, 10, 12, 16]
*                             bit(1)= Acoef + Bcoef > 1.0
*                             bit(2)= Y Offset outside data width range [-2^data_width, (2^data_width)-1]
*                             bit(3)= Cb Offset outside data width range [-2^data_width, (2^data_width)-1]
*                             bit(4)= Cr Offset outside data width range [-2^data_width, (2^data_width)-1]
*                             bit(5)= RGB Max outside data width range [0, (2^data_width)-1]
*                             bit(6)= RGB Min outside data width range [0, (2^data_width)-1]
*
* @note
*
******************************************************************************/
u32 YCC_coefficient_translation(struct ycc_coef_inputs *coef_in, struct ycc_coef_outputs *coef_out, u32 data_width, u32 mwidth);


/*****************************************************************************/
/**
*
* Program the YCrCb2RGB coefficient/offset registers.
*
* @param BaseAddress is the Xilinx EDK base address of the YCrCb2RGB core (from xparameters.h)
* @param coef_out is a pointer to a ycc_coef_output data structure.
*
* @return   None.
*
* @note
*
******************************************************************************/
void YCC_set_coefficients(u32 BaseAddress, struct ycc_coef_outputs *coef_out);


/*****************************************************************************/
/**
*
* Read the YCrCb2RGB coefficient/offset registers.
*
* @param BaseAddress is the Xilinx EDK base address of the YCrCb2RGB core (from xparameters.h)
* @param coef_out is a pointer to a ycc_coef_output data structure.
*
* @return   None.
*
* @note
*
******************************************************************************/
void YCC_get_coefficients(u32 BaseAddress, struct ycc_coef_outputs *coef_out);


#define ycc_max(a,b) (((a)>(b)) ? (a) : (b))


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
