/******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* @file xv_csc_l2.c
* @addtogroup v_csc_v1_0
* @{
*
* The CSC Layer-2 Driver. The functions in this file provides an abstraction
* from the register peek/poke methodology by implementing most common use-case
* provided by the sub-core. See xv_csc_l2.h for a detailed description of the
* layer-2 driver
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rc   05/01/15   Initial Release

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_csc_l2.h"

/************************** Constant Definitions *****************************/
/* Maximum precision available for csc coefficients */
#define XV_CSC_COEFF_FRACTIONAL_BITS   (12)

/**************************** Type Definitions *******************************/
/**
 * This typedef enumerates the window within which the csc core will have an
 * impact. Coordinates outside the window will be passed as-is to output
 */
typedef enum
{
  UPDT_REG_FULL_FRAME = 0,
  UPD_REG_DEMO_WIN
}XV_CSC_REG_UPDT_WIN;


/**************************** Local Global *******************************/

/************************** Function Prototypes ******************************/
static void cscFwYCbCrtoRGB(s32 K[3][4],
                            XVidC_ColorStd cstdIn,
                            s32 pixPrec,
                            s32 *ClampMin,
                            s32 *ClipMax);

static void cscFwRGBtoYCbCr(s32 K[3][4],
                            XVidC_ColorStd cstdOut,
                            s32 pixPrec,
                            s32 *ClampMin,
                            s32 *ClipMax);
#if 0 //currently not used
static void cscFwGetCoefficients(XV_csc_L2Reg *pCscFwReg,
                                 s32 K[3][4],
                                 s32 *ClampMin,
                                 s32 *ClipMax);
#endif
static void cscFwSetCoefficients(XV_csc_L2Reg *pCscFwReg,
                                 s32 K[3][4],
                                 s32 ClampMin,
                                 s32 ClipMax);
static void cscFwGetActiveCoefficients(XV_csc_L2Reg *pCscFwReg, s32 K[3][4]);
static void cscFwSetActiveCoefficients(XV_csc_L2Reg *pCscFwReg, s32 K[3][4]);
static void cscFwMatrixMult(s32 K1[3][4], s32 K2[3][4], s32 Kout[3][4]);
static void cscFwComputeCoeff(XV_csc_L2Reg *pCscFwReg,
                              s32 K2[3][4]);
static void cscUpdateIPReg(XV_csc *pCsc,
                           XV_csc_L2Reg *pCscFwReg,
                           XV_CSC_REG_UPDT_WIN win);
/*****************************************************************************/
/**
* This function provides the write interface for FW register bank
*
* @param  pCscFwReg is a pointer to fw register map of csc core instance
* @param  offset is register offset
* @param  val is data to write
*
* @return None
*
******************************************************************************/
__inline void cscFw_RegW(XV_csc_L2Reg *pCscFwReg, u32 offset, s32 val)
{
  pCscFwReg->regMap[offset] = val;
}

/*****************************************************************************/
/**
* This function provides the read interface for FW register bank
*
* @param  pCscFwReg is a pointer to fw register map of csc core instance
* @param  offset is register offset
*
* @return Register value at requested offset
*
*
******************************************************************************/
__inline s32 cscFw_RegR(XV_csc_L2Reg *pCscFwReg, u32 offset)
{
  return pCscFwReg->regMap[offset];
}

/*****************************************************************************/
/**
* This function starts the Color space converter core
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_CscStart(XV_csc *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_csc_EnableAutoRestart(InstancePtr);
  XV_csc_Start(InstancePtr);
}

/*****************************************************************************/
/**
* This function stops the Color space converter
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_CscStop(XV_csc *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_csc_DisableAutoRestart(InstancePtr);
}

/*****************************************************************************/
/**
* This function set the frame resolution for the Color space converter
* This also will reset the demo window to full frame
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  width is the input stream width
* @param  height is the input stream height
*
* @return None
*
******************************************************************************/
void XV_CscSetActiveSize(XV_csc *InstancePtr,
                          u32    width,
                          u32    height)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_csc_Set_HwReg_width(InstancePtr,  width);
  XV_csc_Set_HwReg_height(InstancePtr, height);

  //Reset demo window to full frame
  XV_csc_Set_HwReg_ColStart(InstancePtr, 0);
  XV_csc_Set_HwReg_ColEnd(InstancePtr,   (width-1));
  XV_csc_Set_HwReg_RowStart(InstancePtr, 0);
  XV_csc_Set_HwReg_RowEnd(InstancePtr,  (height-1));
}

/*****************************************************************************/
/**
* This function set the demo window for the Color space converter. Any pixel
* outside the demo window will not be impacted and will be passed to output
* as-is
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  ActiveWindow is structure that contains window coordinates and size
*
* @return None
*
******************************************************************************/
void XV_CscSetDemoWindow(XV_csc *InstancePtr, XVidC_VideoWindow *ActiveWindow)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_csc_Set_HwReg_ColStart(InstancePtr, ActiveWindow->StartX);
  XV_csc_Set_HwReg_ColEnd(InstancePtr,   (ActiveWindow->StartX+ActiveWindow->Width-1));
  XV_csc_Set_HwReg_RowStart(InstancePtr, ActiveWindow->StartY);
  XV_csc_Set_HwReg_RowEnd(InstancePtr,   (ActiveWindow->StartY+ActiveWindow->Height-1));
}


/*****************************************************************************/
/**
* This function sets the CSC IP layer 2 fw registers to power on default state
*
* @param  pCscFwReg is a pointer to the layer 2 fw register bank
*
* @return None
*
******************************************************************************/
void XV_CscInitPowerOnDefault(XV_csc_L2Reg *pCscFwReg)
{
  Xil_AssertVoid(pCscFwReg != NULL);

  pCscFwReg->ColorFormatIn     = XVIDC_CSF_RGB;
  pCscFwReg->ColorFormatOut    = XVIDC_CSF_RGB;
  pCscFwReg->StandardIn        = XVIDC_BT_601;
  pCscFwReg->StandardOut       = XVIDC_BT_601;
  pCscFwReg->OutputRange       = XVIDC_CR_0_255;
  pCscFwReg->ColorDepth        = XVIDC_BPC_8;
  pCscFwReg->Brightness        = 120;
  pCscFwReg->Contrast          = 0;
  pCscFwReg->Saturation        = 100;
  pCscFwReg->RedGain           = 120;
  pCscFwReg->GreenGain         = 120;
  pCscFwReg->BlueGain          = 120;
  pCscFwReg->Brightness_active = 120;
  pCscFwReg->Contrast_active   = 0;
  pCscFwReg->Saturation_active = 100;
  pCscFwReg->RedGain_active    = 120;
  pCscFwReg->GreenGain_active  = 120;
  pCscFwReg->BlueGain_active   = 120;
  pCscFwReg->K_active[0][0]    = (1<<XV_CSC_COEFF_FRACTIONAL_BITS);
  pCscFwReg->K_active[0][1]    = 0;
  pCscFwReg->K_active[0][2]    = 0;
  pCscFwReg->K_active[1][0]    = 0;
  pCscFwReg->K_active[1][1]    = (1<<XV_CSC_COEFF_FRACTIONAL_BITS);
  pCscFwReg->K_active[1][2]    = 0;
  pCscFwReg->K_active[2][0]    = 0;
  pCscFwReg->K_active[2][1]    = 0;
  pCscFwReg->K_active[2][2]    = (1<<XV_CSC_COEFF_FRACTIONAL_BITS);
  pCscFwReg->K_active[0][3]    = 0;
  pCscFwReg->K_active[1][3]    = 0;
  pCscFwReg->K_active[2][3]    = 0;
}


/*****************************************************************************/
/**
 * This function configures the Color space converter to user specified
 * settings. Before any feature specific calls in layer-2 driver is made
 * csc core should have been configured via this call.
 *
 * @param  InstancePtr is a pointer to the core instance to be worked on.
 * @param  pCscFwReg is pointer to layer 2 register bank
 * @param  cfmtIn is input color space
 * @param  cfmtOut is output color space
 * @param  cstdIn is input color standard
 * @param  cstdOut is output color standard
 * @param  cRangeOut is the selected output range
 *
 * @return None
 *
 *****************************************************************************/
void XV_CscSetColorspace(XV_csc *InstancePtr,
                          XV_csc_L2Reg  *pCscFwReg,
                          XVidC_ColorFormat cfmtIn,
                          XVidC_ColorFormat cfmtOut,
                          XVidC_ColorStd cstdIn,
                          XVidC_ColorStd cstdOut,
                          XVidC_ColorRange  cRangeOut
                         )
{
  s32 K[3][4], K1[3][4], K2[3][4];
  s32 ClampMin = 0;
  s32 ClipMax;
  s32 scale_factor;
  XV_csc *pCsc = InstancePtr;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(pCscFwReg != NULL);

  ClipMax  = ((1<<pCscFwReg->ColorDepth)-1);
  scale_factor = (1<<XV_CSC_COEFF_FRACTIONAL_BITS);

  //initialize to identity matrix
  K[0][0] = scale_factor;
  K[0][1] = 0;
  K[0][2] = 0;
  K[1][0] = 0;
  K[1][1] = scale_factor;
  K[1][2] = 0;
  K[2][0] = 0;
  K[2][1] = 0;
  K[2][2] = scale_factor;
  K[0][3] = 0;
  K[1][3] = 0;
  K[2][3] = 0;

  XV_csc_Set_HwReg_InVideoFormat(pCsc,  cfmtIn);
  XV_csc_Set_HwReg_OutVideoFormat(pCsc, cfmtOut);
  if ((cfmtIn == XVIDC_CSF_RGB) && (cfmtOut == XVIDC_CSF_YCRCB_444) )
  {
    cscFwRGBtoYCbCr(K, cstdOut, pCscFwReg->ColorDepth, &ClampMin, &ClipMax);
  }
  else if ((cfmtIn == XVIDC_CSF_YCRCB_444) && (cfmtOut == XVIDC_CSF_RGB))
  {
    cscFwYCbCrtoRGB(K, cstdIn, pCscFwReg->ColorDepth, &ClampMin, &ClipMax);
  }
  else if ((cfmtIn == XVIDC_CSF_RGB) && (cfmtOut == XVIDC_CSF_RGB) )
  {
    //nop
  }
  else //422->422 or 444->444
  {
    if (cstdIn != cstdOut)
    {
      cscFwYCbCrtoRGB(K1, cstdIn, pCscFwReg->ColorDepth, &ClampMin, &ClipMax);
      cscFwRGBtoYCbCr(K2, cstdOut, pCscFwReg->ColorDepth, &ClampMin, &ClipMax);
      cscFwMatrixMult(K1, K2, K);
    }
  }
  //update fw registers
  pCscFwReg->ColorFormatIn  = cfmtIn;
  pCscFwReg->ColorFormatOut = cfmtOut;
  pCscFwReg->StandardIn     = cstdIn;
  pCscFwReg->StandardOut    = cstdOut;
  pCscFwReg->OutputRange    = cRangeOut;

  cscFw_RegW(pCscFwReg, CSC_FW_REG_K11,K[0][0]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K12,K[0][1]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K13,K[0][2]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K21,K[1][0]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K22,K[1][1]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K23,K[1][2]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K13,K[2][0]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K32,K[2][1]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K33,K[2][2]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_ROffset,K[0][3]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_GOffset,K[1][3]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_BOffset,K[2][3]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_ClampMin,ClampMin);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_ClipMax,ClipMax);

  //compute coeff for Demo window
  cscFwComputeCoeff(pCscFwReg, pCscFwReg->K_active);

  //write IP Registers
  cscUpdateIPReg(pCsc, pCscFwReg, UPDT_REG_FULL_FRAME);
  cscUpdateIPReg(pCsc, pCscFwReg, UPD_REG_DEMO_WIN);
}

/*****************************************************************************/
/**
* This function sets coefficients for YCbCr to RGB conversion
*
* @param  YCC2RGB is the array to hold coefficients
* @param  cstdIn is input color standard
* @param  pixPrec is the color depth
* @param  ClampMin is min value to clamp computed by the function
* @param  ClipMax  is max value to saturate computed by the function
*
* @return Implicitly returns Clamp Min/Max values
*
******************************************************************************/
static void cscFwYCbCrtoRGB(s32 YCC2RGB[3][4],
                            XVidC_ColorStd cstdIn,
                            s32 pixPrec,
                            s32 *ClampMin,
                            s32 *ClipMax)
{
  s32 scale_factor = (1<<XV_CSC_COEFF_FRACTIONAL_BITS);
  s32 bpcScale = (1<<(pixPrec-8));

  switch(cstdIn)
  {
    case XVIDC_BT_601:
        YCC2RGB[0][0] = (s32) ( 1.1644*(float)scale_factor);  //K11
        YCC2RGB[0][1] = (s32)  0;                             //K12
        YCC2RGB[0][2] = (s32) ( 1.5906*(float)scale_factor);  //K13
        YCC2RGB[1][0] = (s32) ( 1.1644*(float)scale_factor);  //K21
        YCC2RGB[1][1] = (s32) (-0.3918*(float)scale_factor);  //K22
        YCC2RGB[1][2] = (s32) (-0.8130*(float)scale_factor);  //K23
        YCC2RGB[2][0] = (s32)  (1.1644*(float)scale_factor);  //K31
        YCC2RGB[2][1] = (s32) ( 2.0172*(float)scale_factor);  //K32
        YCC2RGB[2][2] = (s32)  0;                             //K33
        YCC2RGB[0][3] = (s32) -223*bpcScale;                  //R Offset
        YCC2RGB[1][3] = (s32)  136*bpcScale;                  //G Offset
        YCC2RGB[2][3] = (s32) -277*bpcScale;                  //B Offset
        break;

    case XVIDC_BT_709:
        YCC2RGB[0][0] = (s32) ( 1.1644*(float)scale_factor);  //K11
        YCC2RGB[0][1] = (s32)  0;                             //K12
        YCC2RGB[0][2] = (s32) ( 1.7927*(float)scale_factor);  //K13
        YCC2RGB[1][0] = (s32) ( 1.1644*(float)scale_factor);  //K21
        YCC2RGB[1][1] = (s32) (-0.2132*(float)scale_factor);  //K22
        YCC2RGB[1][2] = (s32) (-0.5329*(float)scale_factor);  //K23
        YCC2RGB[2][0] = (s32) ( 1.1644*(float)scale_factor);  //K31
        YCC2RGB[2][1] = (s32) ( 2.1124*(float)scale_factor);  //K32
        YCC2RGB[2][2] = (s32)  0;                             //K33
        YCC2RGB[0][3] = (s32) -248*bpcScale;                  //R Offset
        YCC2RGB[1][3] = (s32)  77*bpcScale;                   //G Offset
        YCC2RGB[2][3] = (s32) -289*bpcScale;                  //B Offset
        break;

    case XVIDC_BT_2020:
        YCC2RGB[0][0] = (s32) ( 1.1644*(float)scale_factor);  //K11
        YCC2RGB[0][1] = (s32)  0;                             //K12
        YCC2RGB[0][2] = (s32) ( 1.6787*(float)scale_factor);  //K13
        YCC2RGB[1][0] = (s32) ( 1.1644*(float)scale_factor);  //K21
        YCC2RGB[1][1] = (s32) (-0.1873*(float)scale_factor);  //K22
        YCC2RGB[1][2] = (s32) (-0.6504*(float)scale_factor);  //K23
        YCC2RGB[2][0] = (s32) ( 1.1644*(float)scale_factor);  //K31
        YCC2RGB[2][1] = (s32) ( 2.1418*(float)scale_factor);  //K32
        YCC2RGB[2][2] = (s32)  0;                             //K33
        YCC2RGB[0][3] = (s32) -234*bpcScale;                  //R Offset
        YCC2RGB[1][3] = (s32)  89*bpcScale;                   //G Offset
        YCC2RGB[2][3] = (s32) -293*bpcScale;                  //B Offset
        break;

    default: //use 601 numbers
        YCC2RGB[0][0] = (s32) ( 1.1644*(float)scale_factor);  //K11
        YCC2RGB[0][1] = (s32)  0;                             //K12
        YCC2RGB[0][2] = (s32) ( 1.5906*(float)scale_factor);  //K13
        YCC2RGB[1][0] = (s32) ( 1.1644*(float)scale_factor);  //K21
        YCC2RGB[1][1] = (s32) (-0.3918*(float)scale_factor);  //K22
        YCC2RGB[1][2] = (s32) (-0.8130*(float)scale_factor);  //K23
        YCC2RGB[2][0] = (s32)  (1.1644*(float)scale_factor);  //K31
        YCC2RGB[2][1] = (s32) ( 2.0172*(float)scale_factor);  //K32
        YCC2RGB[2][2] = (s32)  0;                             //K33
        YCC2RGB[0][3] = (s32) -223*bpcScale;                  //R Offset
        YCC2RGB[1][3] = (s32)  136*bpcScale;                  //G Offset
        YCC2RGB[2][3] = (s32) -277*bpcScale;                  //B Offset
        break;
  }

  *ClampMin = 0;
  *ClipMax  = ((1<<pixPrec)-1);
}

/*****************************************************************************/
/**
* This function sets coefficients for RGB to YCbCr conversion
*
* @param  RGB2YCC is the array to hold coefficients
* @param  cstdOut is output color standard
* @param  pixPrec is the color depth
* @param  ClampMin is min value to clamp computed by the function
* @param  ClipMax  is max value to saturate computed by the function
*
* @return Implicitly returns Clamp Min/Max values
*
******************************************************************************/
static void cscFwRGBtoYCbCr(s32 RGB2YCC[3][4],
                            XVidC_ColorStd cstdOut,
                            s32 pixPrec,
                            s32 *ClampMin,
                            s32 *ClipMax)
{
  s32 scale_factor = (1<<XV_CSC_COEFF_FRACTIONAL_BITS);
  s32 bpcScale = (1<<(pixPrec-8));

  switch(cstdOut)
  {
    case XVIDC_BT_601:
        RGB2YCC[0][0] = (s32) ( 0.2568*(float)scale_factor);  //K11
        RGB2YCC[0][1] = (s32) ( 0.5041*(float)scale_factor);  //K12
        RGB2YCC[0][2] = (s32) ( 0.0979*(float)scale_factor);  //K13
        RGB2YCC[1][0] = (s32) (-0.1482*(float)scale_factor);  //K21
        RGB2YCC[1][1] = (s32) (-0.2910*(float)scale_factor);  //K22
        RGB2YCC[1][2] = (s32) ( 0.4393*(float)scale_factor);  //K23
        RGB2YCC[2][0] = (s32) ( 0.4393*(float)scale_factor);  //K31
        RGB2YCC[2][1] = (s32) (-0.3678*(float)scale_factor);  //K32
        RGB2YCC[2][2] = (s32) (-0.0714*(float)scale_factor);  //K33
        RGB2YCC[0][3] =  16*bpcScale;                   //R Offset
        RGB2YCC[1][3] =  128*bpcScale;                  //G Offset
        RGB2YCC[2][3] =  128*bpcScale;                  //B Offset
        break;

    case XVIDC_BT_709:
        RGB2YCC[0][0] = (s32) ( 0.1826*(float)scale_factor);  //K11
        RGB2YCC[0][1] = (s32) ( 0.6142*(float)scale_factor);  //K12
        RGB2YCC[0][2] = (s32) ( 0.0620*(float)scale_factor);  //K13
        RGB2YCC[1][0] = (s32) (-0.1006*(float)scale_factor);  //K21
        RGB2YCC[1][1] = (s32) (-0.3386*(float)scale_factor);  //K22
        RGB2YCC[1][2] = (s32) ( 0.4392*(float)scale_factor);  //K23
        RGB2YCC[2][0] = (s32) ( 0.4392*(float)scale_factor);  //K31
        RGB2YCC[2][1] = (s32) (-0.3989*(float)scale_factor);  //K32
        RGB2YCC[2][2] = (s32) (-0.0403*(float)scale_factor);  //K33
        RGB2YCC[0][3] =  16*bpcScale;                   //R Offset
        RGB2YCC[1][3] =  128*bpcScale;                  //G Offset
        RGB2YCC[2][3] =  128*bpcScale;                  //B Offset
        break;

    case XVIDC_BT_2020:
        RGB2YCC[0][0] = (s32) ( 0.2256*(float)scale_factor);  //K11
        RGB2YCC[0][1] = (s32) ( 0.5823*(float)scale_factor);  //K12
        RGB2YCC[0][2] = (s32) ( 0.0509*(float)scale_factor);  //K13
        RGB2YCC[1][0] = (s32) (-0.1227*(float)scale_factor);  //K21
        RGB2YCC[1][1] = (s32) (-0.3166*(float)scale_factor);  //K22
        RGB2YCC[1][2] = (s32) ( 0.4392*(float)scale_factor);  //K23
        RGB2YCC[2][0] = (s32) ( 0.4392*(float)scale_factor);  //K31
        RGB2YCC[2][1] = (s32) (-0.4039*(float)scale_factor);  //K32
        RGB2YCC[2][2] = (s32) (-0.0353*(float)scale_factor);  //K33
        RGB2YCC[0][3] =  16*bpcScale;                   //R Offset
        RGB2YCC[1][3] =  128*bpcScale;                  //G Offset
        RGB2YCC[2][3] =  128*bpcScale;                  //B Offset
        break;

    default:
        RGB2YCC[0][0] = (s32) ( 0.2568*(float)scale_factor);  //K11
        RGB2YCC[0][1] = (s32) ( 0.5041*(float)scale_factor);  //K12
        RGB2YCC[0][2] = (s32) ( 0.0979*(float)scale_factor);  //K13
        RGB2YCC[1][0] = (s32) (-0.1482*(float)scale_factor);  //K21
        RGB2YCC[1][1] = (s32) (-0.2910*(float)scale_factor);  //K22
        RGB2YCC[1][2] = (s32) ( 0.4393*(float)scale_factor);  //K23
        RGB2YCC[2][0] = (s32) ( 0.4393*(float)scale_factor);  //K31
        RGB2YCC[2][1] = (s32) (-0.3678*(float)scale_factor);  //K32
        RGB2YCC[2][2] = (s32) (-0.0714*(float)scale_factor);  //K33
        RGB2YCC[0][3] =  16*bpcScale;                   //R Offset
        RGB2YCC[1][3] =  128*bpcScale;                  //G Offset
        RGB2YCC[2][3] =  128*bpcScale;                  //B Offset
        break;
  }

  *ClampMin = 0;
  *ClipMax  = ((1<<pixPrec)-1);
}

#if 0 //currently not used
/*****************************************************************************/
/**
 * This function reads demo window coefficient set from the register bank
*
* @param  pCscFwReg is the pointer to layer 2 fw register bank
* @param  K is the array to hold coefficients
*
* @return
* 	- ClampMin is min value to clamp
*   - ClipMax  is max value to saturate
*
******************************************************************************/
static void cscFwGetCoefficients(XV_csc_L2Reg *pCscFwReg,
                                 s32 K[3][4],
                                 s32 *ClampMin,
                                 s32 *ClipMax)
{
  K[0][0]  = cscFw_RegR(pCscFwReg, CSC_FW_REG_K11_2);
  K[0][1]  = cscFw_RegR(pCscFwReg, CSC_FW_REG_K12_2);
  K[0][2]  = cscFw_RegR(pCscFwReg, CSC_FW_REG_K13_2);
  K[1][0]  = cscFw_RegR(pCscFwReg, CSC_FW_REG_K21_2);
  K[1][1]  = cscFw_RegR(pCscFwReg, CSC_FW_REG_K22_2);
  K[1][2]  = cscFw_RegR(pCscFwReg, CSC_FW_REG_K23_2);
  K[2][0]  = cscFw_RegR(pCscFwReg, CSC_FW_REG_K31_2);
  K[2][1]  = cscFw_RegR(pCscFwReg, CSC_FW_REG_K32_2);
  K[2][2]  = cscFw_RegR(pCscFwReg, CSC_FW_REG_K33_2);
  K[0][3]  = cscFw_RegR(pCscFwReg, CSC_FW_REG_ROffset_2);
  K[1][3]  = cscFw_RegR(pCscFwReg, CSC_FW_REG_GOffset_2);
  K[2][3]  = cscFw_RegR(pCscFwReg, CSC_FW_REG_BOffset_2);
  *ClampMin = cscFw_RegR(pCscFwReg, CSC_FW_REG_ClampMin_2);
  *ClipMax  = cscFw_RegR(pCscFwReg, CSC_FW_REG_ClipMax_2);
}
#endif

/*****************************************************************************/
/**
* This function writes demo window coefficients to layer 2 fw register bank
*
* @param  pCscFwReg is a pointer to layer 2 fw register bank
* @param  K is an array to hold coefficients
* @param  ClampMin is min value to clamp
* @param  ClipMax is max value to saturate
*
* @return None
*
******************************************************************************/
static void cscFwSetCoefficients(XV_csc_L2Reg *pCscFwReg,
                                 s32 K[3][4],
                                 s32 ClampMin,
                                 s32 ClipMax)
{
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K11_2,K[0][0]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K12_2,K[0][1]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K13_2,K[0][2]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K21_2,K[1][0]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K22_2,K[1][1]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K23_2,K[1][2]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K31_2,K[2][0]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K32_2,K[2][1]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_K33_2,K[2][2]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_ROffset_2,K[0][3]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_GOffset_2,K[1][3]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_BOffset_2,K[2][3]);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_ClampMin_2,ClampMin);
  cscFw_RegW(pCscFwReg, CSC_FW_REG_ClipMax_2,ClipMax);
}

/*****************************************************************************/
/**
* This function gets the current RGB coefficients for demo window
*
* @param  pCscFwReg is the pointer to layer 2 fw register bank
* @param  K is the array to hold coefficients
*
* @return None
*
******************************************************************************/
static void cscFwGetActiveCoefficients(XV_csc_L2Reg *pCscFwReg, s32 K[3][4])
{
  u8 x,y;

  for(x=0; x<3; ++x)
  {
    for(y=0; y<4; ++y)
    {
      K[x][y] = pCscFwReg->K_active[x][y];
    }
  }
}

/*****************************************************************************/
/**
* This function sets the current RGB coefficients for demo window
*
* @param  pCscFwReg is the pointer to layer 2 fw register bank
* @param  K is the array to hold coefficients
*
* @return None
*
******************************************************************************/
static void cscFwSetActiveCoefficients(XV_csc_L2Reg *pCscFwReg, s32 K[3][4])
{
  u8 x,y;

  for(x=0; x<3; ++x)
  {
    for(y=0; y<4; ++y)
    {
        pCscFwReg->K_active[x][y] = K[x][y];
    }
  }
}

/*****************************************************************************/
/**
* This function set brightness to specified value. It also translates user
* setting of 0-100 to hw register range
*
* @param  InstancePtr is pointer to csc core instance
* @param  pCscFwReg is a pointer to layer 2 fw register bank
* @param  val is new brightness value
*
* @return None
*
******************************************************************************/
void XV_CscSetBrightness(XV_csc *InstancePtr, XV_csc_L2Reg *pCscFwReg, s32 val)
{
  XV_csc *pCsc = InstancePtr;
  s32 K1[3][4], K2[3][4];
  float brightness_f;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(pCscFwReg != NULL);

  pCscFwReg->Brightness = (val*2+20);
  brightness_f = (float)(pCscFwReg->Brightness)/(float)(pCscFwReg->Brightness_active);

  //get active coefficient set in RGB
  cscFwGetActiveCoefficients(pCscFwReg, K1);

  K2[0][0] = (s32) ((float) K1[0][0] * brightness_f);
  K2[0][1] = (s32) ((float) K1[0][1] * brightness_f);
  K2[0][2] = (s32) ((float) K1[0][2] * brightness_f);
  K2[1][0] = (s32) ((float) K1[1][0] * brightness_f);
  K2[1][1] = (s32) ((float) K1[1][1] * brightness_f);
  K2[1][2] = (s32) ((float) K1[1][2] * brightness_f);
  K2[2][0] = (s32) ((float) K1[2][0] * brightness_f);
  K2[2][1] = (s32) ((float) K1[2][1] * brightness_f);
  K2[2][2] = (s32) ((float) K1[2][2] * brightness_f);
  K2[0][3] = K1[0][3];
  K2[1][3] = K1[1][3];
  K2[2][3] = K1[2][3];

  //write new active coefficient set in RGB
  cscFwSetActiveCoefficients(pCscFwReg, K2);
  //write new active brightness value
  pCscFwReg->Brightness_active = pCscFwReg->Brightness;

  cscFwComputeCoeff(pCscFwReg, K2);
  cscUpdateIPReg(pCsc, pCscFwReg, UPD_REG_DEMO_WIN);
}

/*****************************************************************************/
/**
* This function set contrast to specified value. It also translates user
* setting of 0-100 to hw register range
*
* @param  InstancePtr is pointer to csc core instance
* @param  pCscFwReg is a pointer to layer 2 fw register bank
* @param  val is new contrast value
*
* @return None
*
******************************************************************************/
void XV_CscSetContrast(XV_csc *InstancePtr, XV_csc_L2Reg *pCscFwReg, s32 val)
{
  XV_csc *pCsc = InstancePtr;
  s32 contrast;
  s32 K1[3][4], K2[3][4];
  s32 scale;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(pCscFwReg != NULL);

  pCscFwReg->Contrast = val*4 - 200;
  contrast = (pCscFwReg->Contrast) - (pCscFwReg->Contrast_active);

  scale = (1<<(pCscFwReg->ColorDepth-8));
  //get active coefficient set in RGB
  cscFwGetActiveCoefficients(pCscFwReg, K1);

  K2[0][0] = K1[0][0];
  K2[0][1] = K1[0][1];
  K2[0][2] = K1[0][2];
  K2[1][0] = K1[1][0];
  K2[1][1] = K1[1][1];
  K2[1][2] = K1[1][2];
  K2[2][0] = K1[2][0];
  K2[2][1] = K1[2][1];
  K2[2][2] = K1[2][2];
  K2[0][3] = K1[0][3] + contrast*scale;
  K2[1][3] = K1[1][3] + contrast*scale;
  K2[2][3] = K1[2][3] + contrast*scale;

  //write new active coefficient set in RGB
  cscFwSetActiveCoefficients(pCscFwReg, K2);
  //write new active contrast value
  pCscFwReg->Contrast_active = pCscFwReg->Contrast;

  cscFwComputeCoeff(pCscFwReg, K2);
  cscUpdateIPReg(pCsc, pCscFwReg, UPD_REG_DEMO_WIN);
}

/*****************************************************************************/
/**
* This function set saturation to specified value. It also translates user
* setting of 0-100 to hw register range
*
* @param  InstancePtr is pointer to csc core instance
* @param  pCscFwReg is a pointer to layer 2 fw register bank
* @param  val is new saturation value
*
* @return None
*
******************************************************************************/
void XV_CscSetSaturation(XV_csc *InstancePtr, XV_csc_L2Reg *pCscFwReg, s32 val)
{
  XV_csc *pCsc = InstancePtr;
  s32 x, y;
  s32 K1[3][4], K2[3][4], K3[3][4], K4[3][4], M1[3][4], M2[3][4], Kout[3][4];
  s32 ClampMin = 0;
  s32 ClipMax, scale_factor;

  float saturation_f;
  float rwgt, gwgt, bwgt;
  float a,b,c,d,e,f,g,h,i;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(pCscFwReg != NULL);

  ClipMax  = ((1<<pCscFwReg->ColorDepth)-1);
  scale_factor = (1<<XV_CSC_COEFF_FRACTIONAL_BITS);

  pCscFwReg->Saturation = ((val == 0) ? 1 : val*2);
  saturation_f = (float)(pCscFwReg->Saturation)/(float)(pCscFwReg->Saturation_active);

  //get active coefficient set in RGB
  cscFwGetActiveCoefficients(pCscFwReg, K1);

  rwgt = 0.3086f;
  gwgt = 0.6094f;
  bwgt = 0.0820f;
  //rwgt = 0.299f;
  //gwgt = 0.587f;
  //bwgt = 0.114f;

  a = ((1.0f - saturation_f) * rwgt + saturation_f);
  b = ((1.0f - saturation_f) * rwgt);
  c = ((1.0f - saturation_f) * rwgt);
  d = ((1.0f - saturation_f) * gwgt);
  e = ((1.0f - saturation_f) * gwgt + saturation_f);
  f = ((1.0f - saturation_f) * gwgt);
  g = ((1.0f - saturation_f) * bwgt);
  h = ((1.0f - saturation_f) * bwgt);
  i = ((1.0f - saturation_f) * bwgt + saturation_f);

  K2[0][0]=(s32)(a*scale_factor);
  K2[0][1]=(s32)(d*scale_factor);
  K2[0][2]=(s32)(g*scale_factor);
  K2[1][0]=(s32)(b*scale_factor);
  K2[1][1]=(s32)(e*scale_factor);
  K2[1][2]=(s32)(h*scale_factor);
  K2[2][0]=(s32)(c*scale_factor);
  K2[2][1]=(s32)(f*scale_factor);
  K2[2][2]=(s32)(i*scale_factor);
  K2[0][3] = 0;
  K2[1][3] = 0;
  K2[2][3] = 0;

  cscFwMatrixMult(K1, K2, K3);

  //write new active coefficient set in RGB
  cscFwSetActiveCoefficients(pCscFwReg, K3);
  //write new active saturation value
  pCscFwReg->Saturation_active = pCscFwReg->Saturation;

  if ((pCscFwReg->ColorFormatIn == XVIDC_CSF_RGB) &&
      (pCscFwReg->ColorFormatOut == XVIDC_CSF_RGB) )
  {
    for (x=0; x<3; x++)  for (y=0; y<4; y++)
      Kout[x][y] = K3[x][y];
  }

  if ((pCscFwReg->ColorFormatIn == XVIDC_CSF_RGB) &&
      (pCscFwReg->ColorFormatOut == XVIDC_CSF_YCRCB_444) )
  {
    cscFwRGBtoYCbCr(M2, pCscFwReg->StandardOut, pCscFwReg->ColorDepth, &ClampMin, &ClipMax);
    cscFwMatrixMult(K3, M2, Kout);
  }

  if ((pCscFwReg->ColorFormatIn == XVIDC_CSF_YCRCB_444) &&
      (pCscFwReg->ColorFormatOut == XVIDC_CSF_RGB) )
  {
    cscFwYCbCrtoRGB(M1, pCscFwReg->StandardIn, pCscFwReg->ColorDepth, &ClampMin, &ClipMax);
    cscFwMatrixMult(M1, K3, Kout);
  }

  if ((pCscFwReg->ColorFormatIn == XVIDC_CSF_YCRCB_444) &&
      (pCscFwReg->ColorFormatOut == XVIDC_CSF_YCRCB_444) )
  {
    cscFwYCbCrtoRGB(M1, pCscFwReg->StandardIn, pCscFwReg->ColorDepth, &ClampMin, &ClipMax);
    cscFwMatrixMult(M1, K3, K4);
    cscFwRGBtoYCbCr(M2, pCscFwReg->StandardOut, pCscFwReg->ColorDepth, &ClampMin, &ClipMax);
    cscFwMatrixMult(K4, M2, Kout);
  }

  cscFwSetCoefficients(pCscFwReg, Kout, ClampMin, ClipMax);
  cscUpdateIPReg(pCsc, pCscFwReg, UPD_REG_DEMO_WIN);
}

/*****************************************************************************/
/**
* This function set red gain to specified value. It also translates user
* setting of 0-100 to hw register range
*
* @param  InstancePtr is pointer to csc core instance
* @param  pCscFwReg is a pointer to layer 2 fw register bank
* @param  val is new gain value
*
* @return None
*
******************************************************************************/
void XV_CscSetRedGain(XV_csc *InstancePtr, XV_csc_L2Reg *pCscFwReg, s32 val)
{
  XV_csc *pCsc = InstancePtr;
  s32 K1[3][4], K2[3][4];
  float red_f;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(pCscFwReg != NULL);

  pCscFwReg->RedGain = (val*2+20);
  red_f = (float)(pCscFwReg->RedGain)/(float)(pCscFwReg->RedGain_active);

  //get active coefficient set in RGB
  cscFwGetActiveCoefficients(pCscFwReg, K1);

  K2[0][0] = (s32) ((float) K1[0][0] * red_f);
  K2[0][1] = (s32) ((float) K1[0][1] * red_f);
  K2[0][2] = (s32) ((float) K1[0][2] * red_f);
  K2[1][0] = K1[1][0];
  K2[1][1] = K1[1][1];
  K2[1][2] = K1[1][2];
  K2[2][0] = K1[2][0];
  K2[2][1] = K1[2][1];
  K2[2][2] = K1[2][2];
  K2[0][3] = K1[0][3];
  K2[1][3] = K1[1][3];
  K2[2][3] = K1[2][3];

  //write new active coefficient set in RGB
  cscFwSetActiveCoefficients(pCscFwReg, K2);
  //write new active red value
  pCscFwReg->RedGain_active = pCscFwReg->RedGain;

  cscFwComputeCoeff(pCscFwReg, K2);
  cscUpdateIPReg(pCsc, pCscFwReg, UPD_REG_DEMO_WIN);
}

/*****************************************************************************/
/**
* This function set green gain to specified value. It also translates user
* setting of 0-100 to hw register range
*
* @param  InstancePtr is pointer to csc core instance
* @param  pCscFwReg is a pointer to layer 2 fw register bank
* @param  val is new gain value
*
* @return None
*
******************************************************************************/
void XV_CscSetGreenGain(XV_csc *InstancePtr, XV_csc_L2Reg *pCscFwReg, s32 val)
{
  XV_csc *pCsc = InstancePtr;
  s32 K1[3][4], K2[3][4];
  float green_f;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(pCscFwReg != NULL);

  pCscFwReg->GreenGain = (val*2+20);
  green_f = (float)(pCscFwReg->GreenGain)/(float)(pCscFwReg->GreenGain_active);

  //get active coefficient set in RGB
  cscFwGetActiveCoefficients(pCscFwReg, K1);

  K2[0][0] = K1[0][0];
  K2[0][1] = K1[0][1];
  K2[0][2] = K1[0][2];
  K2[1][0] = (s32) ((float) K1[1][0] * green_f);
  K2[1][1] = (s32) ((float) K1[1][1] * green_f);
  K2[1][2] = (s32) ((float) K1[1][2] * green_f);
  K2[2][0] = K1[2][0];
  K2[2][1] = K1[2][1];
  K2[2][2] = K1[2][2];
  K2[0][3] = K1[0][3];
  K2[1][3] = K1[1][3];
  K2[2][3] = K1[2][3];

  //write new active coefficient set in RGB
  cscFwSetActiveCoefficients(pCscFwReg, K2);
  //write new active green value
  pCscFwReg->GreenGain_active = pCscFwReg->GreenGain;

  cscFwComputeCoeff(pCscFwReg, K2);
  cscUpdateIPReg(pCsc, pCscFwReg, UPD_REG_DEMO_WIN);
}

/*****************************************************************************/
/**
* This function set blue gain to specified value. It also translates user
* setting of 0-100 to hw register range
*
* @param  InstancePtr is pointer to csc core instance
* @param  pCscFwReg is a pointer to layer 2 fw register bank
* @param  val is new gain value
*
* @return None
*
******************************************************************************/
void XV_CscSetBlueGain(XV_csc *InstancePtr, XV_csc_L2Reg *pCscFwReg, s32 val)
{
  XV_csc *pCsc = InstancePtr;
  s32 K1[3][4], K2[3][4];
  float blue_f;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(pCscFwReg != NULL);

  pCscFwReg->BlueGain = (val*2+20);
  blue_f = (float)(pCscFwReg->BlueGain)/(float)(pCscFwReg->BlueGain_active);

  //get active coefficient set in RGB
  cscFwGetActiveCoefficients(pCscFwReg, K1);

  K2[0][0] = K1[0][0];
  K2[0][1] = K1[0][1];
  K2[0][2] = K1[0][2];
  K2[1][0] = K1[1][0];
  K2[1][1] = K1[1][1];
  K2[1][2] = K1[1][2];
  K2[2][0] = (s32) ((float) K1[2][0] * blue_f);
  K2[2][1] = (s32) ((float) K1[2][1] * blue_f);
  K2[2][2] = (s32) ((float) K1[2][2] * blue_f);
  K2[0][3] = K1[0][3];
  K2[1][3] = K1[1][3];
  K2[2][3] = K1[2][3];

  //write new active coefficient set in RGB
  cscFwSetActiveCoefficients(pCscFwReg, K2);
  //write new active blue value
  pCscFwReg->BlueGain_active = pCscFwReg->BlueGain;

  cscFwComputeCoeff(pCscFwReg, K2);
  cscUpdateIPReg(pCsc, pCscFwReg, UPD_REG_DEMO_WIN);
}


/*****************************************************************************/
/**
* Compute the coefficients for the required color space and write to layer 2
* fw register bank
*
* @param  pCscFwReg is a pointer to layer 2 fw register bank
* @param  K2 is the active coefficients
*
* @return None
*
******************************************************************************/
static void cscFwComputeCoeff(XV_csc_L2Reg *pCscFwReg,
                              s32 K2[3][4])
{
  u32 x,y;
  s32 K3[3][4], M1[3][4], M2[3][4], Kout[3][4];;
  s32 ClampMin = 0;
  s32 ClipMax  = ((1<<pCscFwReg->ColorDepth)-1);

  if((pCscFwReg->ColorFormatIn == XVIDC_CSF_RGB) &&
     (pCscFwReg->ColorFormatOut == XVIDC_CSF_RGB) )
  {
    for (x=0; x<3; x++)  for (y=0; y<4; y++)
      Kout[x][y] = K2[x][y];
  }

  if ((pCscFwReg->ColorFormatIn == XVIDC_CSF_RGB) &&
      (pCscFwReg->ColorFormatOut == XVIDC_CSF_YCRCB_444) )
  {
    cscFwRGBtoYCbCr(M2, pCscFwReg->StandardOut, pCscFwReg->ColorDepth, &ClampMin, &ClipMax);
    cscFwMatrixMult(K2, M2, Kout);
  }

  if ((pCscFwReg->ColorFormatIn == XVIDC_CSF_YCRCB_444) &&
      (pCscFwReg->ColorFormatOut == XVIDC_CSF_RGB) )
  {
    cscFwYCbCrtoRGB(M1, pCscFwReg->StandardIn, pCscFwReg->ColorDepth, &ClampMin, &ClipMax);
    cscFwMatrixMult(M1, K2, Kout);
  }

  if (((pCscFwReg->ColorFormatIn == XVIDC_CSF_YCRCB_444) &&
       (pCscFwReg->ColorFormatOut == XVIDC_CSF_YCRCB_444)) ||
	  ((pCscFwReg->ColorFormatIn == XVIDC_CSF_YCRCB_422) &&
	   (pCscFwReg->ColorFormatOut == XVIDC_CSF_YCRCB_422)))
  {
    cscFwYCbCrtoRGB(M1, pCscFwReg->StandardIn, pCscFwReg->ColorDepth, &ClampMin, &ClipMax);
    cscFwMatrixMult(M1, K2, K3);
    cscFwRGBtoYCbCr(M2, pCscFwReg->StandardOut, pCscFwReg->ColorDepth, &ClampMin, &ClipMax);
    cscFwMatrixMult(K3, M2, Kout);
  }
  cscFwSetCoefficients(pCscFwReg, Kout, ClampMin, ClipMax);
}

/*****************************************************************************/
/**
* This function multiplies Matrices. (Utility function)
*
* @param  K1 input matrix
* @param  K2 input matrix
* @param  Kout is the output matrix (K1 * K2)
*
* @return Matrix multiplication via Kout
*
******************************************************************************/
static void cscFwMatrixMult(s32 K1[3][4], s32 K2[3][4], s32 Kout[3][4])
{

  s32 A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X;
  s32 scale_factor = (1<<XV_CSC_COEFF_FRACTIONAL_BITS);

  A = K1[0][0]; B = K1[0][1]; C = K1[0][2];   J = K1[0][3];
  D = K1[1][0]; E = K1[1][1]; F = K1[1][2];   K = K1[1][3];
  G = K1[2][0]; H = K1[2][1]; I = K1[2][2];   L = K1[2][3];

  M = K2[0][0]; N = K2[0][1]; O = K2[0][2];  V = K2[0][3];
  P = K2[1][0]; Q = K2[1][1]; R = K2[1][2];  W = K2[1][3];
  S = K2[2][0]; T = K2[2][1]; U = K2[2][2];  X = K2[2][3];

  Kout[0][0] =  (M*A + N*D + O*G)/scale_factor;
  Kout[0][1] =  (M*B + N*E + O*H)/scale_factor;
  Kout[0][2] =  (M*C + N*F + O*I)/scale_factor;
  Kout[1][0] =  (P*A + Q*D + R*G)/scale_factor;
  Kout[1][1] =  (P*B + Q*E + R*H)/scale_factor;
  Kout[1][2] =  (P*C + Q*F + R*I)/scale_factor;
  Kout[2][0] =  (S*A + T*D + U*G)/scale_factor;
  Kout[2][1] =  (S*B + T*E + U*H)/scale_factor;
  Kout[2][2] =  (S*C + T*F + U*I)/scale_factor;
  Kout[0][3] = ((M*J + N*K + O*L)/scale_factor) + V;
  Kout[1][3] = ((P*J + Q*K + R*L)/scale_factor) + W;
  Kout[2][3] = ((S*J + T*K + U*L)/scale_factor) + X;
}

/*****************************************************************************/
/**
* Write computed coefficients to IP HW registers
*
* @param  pCsc is pointer to csc core instance
* @param  pCscFwReg is a pointer to layer 2 fw register bank
* @param  win is the window mode: Full Frame or Demo window
*
* @return None
*
******************************************************************************/
static void cscUpdateIPReg(XV_csc *pCsc,
                           XV_csc_L2Reg *pCscFwReg,
                           XV_CSC_REG_UPDT_WIN win)
{
  u8 x,y;
  s32 K[3][4];
  u32 clampMin, clipMax;

  switch(win)
  {
    case UPDT_REG_FULL_FRAME:
        for(x=0; x<3; ++x)
        {
          for(y=0; y<3; ++y)
          {
            K[x][y] = cscFw_RegR(pCscFwReg, (x*3+y)+CSC_FW_REG_K11);
          }
        }
        K[0][3] = cscFw_RegR(pCscFwReg, CSC_FW_REG_ROffset);
        K[1][3] = cscFw_RegR(pCscFwReg, CSC_FW_REG_GOffset);
        K[2][3] = cscFw_RegR(pCscFwReg, CSC_FW_REG_BOffset);
        clampMin = cscFw_RegR(pCscFwReg, CSC_FW_REG_ClampMin);
        clipMax  = cscFw_RegR(pCscFwReg, CSC_FW_REG_ClipMax);

        XV_csc_Set_HwReg_K11(pCsc, K[0][0]);
        XV_csc_Set_HwReg_K12(pCsc, K[0][1]);
        XV_csc_Set_HwReg_K13(pCsc, K[0][2]);
        XV_csc_Set_HwReg_K21(pCsc, K[1][0]);
        XV_csc_Set_HwReg_K22(pCsc, K[1][1]);
        XV_csc_Set_HwReg_K23(pCsc, K[1][2]);
        XV_csc_Set_HwReg_K31(pCsc, K[2][0]);
        XV_csc_Set_HwReg_K32(pCsc, K[2][1]);
        XV_csc_Set_HwReg_K33(pCsc, K[2][2]);
        XV_csc_Set_HwReg_ROffset_V(pCsc,  K[0][3]);
        XV_csc_Set_HwReg_GOffset_V(pCsc,  K[1][3]);
        XV_csc_Set_HwReg_BOffset_V(pCsc,  K[2][3]);
        XV_csc_Set_HwReg_ClampMin_V(pCsc, clampMin);
        XV_csc_Set_HwReg_ClipMax_V(pCsc,  clipMax);
        break;

    case UPD_REG_DEMO_WIN:
        for(x=0; x<3; ++x)
        {
          for(y=0; y<3; ++y)
          {
            K[x][y] = cscFw_RegR(pCscFwReg, (x*3+y)+CSC_FW_REG_K11_2);
          }
        }
        K[0][3] = cscFw_RegR(pCscFwReg, CSC_FW_REG_ROffset_2);
        K[1][3] = cscFw_RegR(pCscFwReg, CSC_FW_REG_GOffset_2);
        K[2][3] = cscFw_RegR(pCscFwReg, CSC_FW_REG_BOffset_2);
        clampMin = cscFw_RegR(pCscFwReg, CSC_FW_REG_ClampMin_2);
        clipMax  = cscFw_RegR(pCscFwReg, CSC_FW_REG_ClipMax_2);

        XV_csc_Set_HwReg_K11_2(pCsc, K[0][0]);
        XV_csc_Set_HwReg_K12_2(pCsc, K[0][1]);
        XV_csc_Set_HwReg_K13_2(pCsc, K[0][2]);
        XV_csc_Set_HwReg_K21_2(pCsc, K[1][0]);
        XV_csc_Set_HwReg_K22_2(pCsc, K[1][1]);
        XV_csc_Set_HwReg_K23_2(pCsc, K[1][2]);
        XV_csc_Set_HwReg_K31_2(pCsc, K[2][0]);
        XV_csc_Set_HwReg_K32_2(pCsc, K[2][1]);
        XV_csc_Set_HwReg_K33_2(pCsc, K[2][2]);
        XV_csc_Set_HwReg_ROffset_2_V(pCsc,  K[0][3]);
        XV_csc_Set_HwReg_GOffset_2_V(pCsc,  K[1][3]);
        XV_csc_Set_HwReg_BOffset_2_V(pCsc,  K[2][3]);
        XV_csc_Set_HwReg_ClampMin_2_V(pCsc, clampMin);
        XV_csc_Set_HwReg_ClipMax_2_V(pCsc,  clipMax);
        break;

    default:
        break;
  }
}

/*****************************************************************************/
/**
* This function prints CSC IP status on console
*
* @param  InstancePtr is the instance pointer to the CSC IP instance to be worked on
*
* @return None
*
******************************************************************************/
void XV_CscDbgReportStatus(XV_csc *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_csc *pCsc = InstancePtr;
  u32 done, idle, ready, ctrl;
  u32 colstart, colend, rowstart, rowend;
  u32 coeff[3][3];
  u32 offset_r, offset_g, offset_b;
  u32 minclamp,maxclamp;
  u32 height, width, i, j;

  xil_printf("\r\n\r\n----->CSC IP STATUS<----\r\n");

  done  = XV_csc_IsDone(pCsc);
  idle  = XV_csc_IsIdle(pCsc);
  ready = XV_csc_IsReady(pCsc);
  ctrl  = XV_csc_ReadReg(pCsc->Ctrl_BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL);

  colstart = XV_csc_Get_HwReg_ColStart(pCsc);
  colend   = XV_csc_Get_HwReg_ColEnd(pCsc);
  rowstart = XV_csc_Get_HwReg_RowStart(pCsc);
  rowend   = XV_csc_Get_HwReg_RowEnd(pCsc);
  offset_r = XV_csc_Get_HwReg_ROffset_V(pCsc);
  offset_g = XV_csc_Get_HwReg_GOffset_V(pCsc);
  offset_b = XV_csc_Get_HwReg_BOffset_V(pCsc);
  minclamp = XV_csc_Get_HwReg_ClampMin_V(pCsc);
  maxclamp = XV_csc_Get_HwReg_ClipMax_V(pCsc);
  height   = XV_csc_Get_HwReg_height(pCsc);
  width    = XV_csc_Get_HwReg_width(pCsc);

  coeff[0][0] = XV_csc_Get_HwReg_K11_2(pCsc);
  coeff[0][1] = XV_csc_Get_HwReg_K12_2(pCsc);
  coeff[0][2] = XV_csc_Get_HwReg_K13_2(pCsc);
  coeff[1][0] = XV_csc_Get_HwReg_K21_2(pCsc);
  coeff[1][1] = XV_csc_Get_HwReg_K22_2(pCsc);
  coeff[1][2] = XV_csc_Get_HwReg_K23_2(pCsc);
  coeff[2][0] = XV_csc_Get_HwReg_K31_2(pCsc);
  coeff[2][1] = XV_csc_Get_HwReg_K32_2(pCsc);
  coeff[2][2] = XV_csc_Get_HwReg_K33_2(pCsc);

  xil_printf("IsDone:  %d\r\n", done);
  xil_printf("IsIdle:  %d\r\n", idle);
  xil_printf("IsReady: %d\r\n", ready);
  xil_printf("Ctrl:    0x%x\r\n\r\n", ctrl);

  xil_printf("Color Format In:   %s\r\n",
               XVidC_GetColorFormatStr(XV_csc_Get_HwReg_InVideoFormat(pCsc)));
  xil_printf("Color Format Out:  %s\r\n",
               XVidC_GetColorFormatStr(XV_csc_Get_HwReg_OutVideoFormat(pCsc)));
  xil_printf("Column Start:      %d\r\n",colstart);
  xil_printf("Column End:        %d\r\n",colend);
  xil_printf("Row Start:         %d\r\n",rowstart);
  xil_printf("Row End:           %d\r\n",rowend);
  xil_printf("R Offset:          %d\r\n",offset_r);
  xil_printf("G Offset:          %d\r\n",offset_g);
  xil_printf("B Offset:          %d\r\n",offset_b);
  xil_printf("Min Clamp:         %d\r\n",minclamp);
  xil_printf("Max Clamp:         %d\r\n",maxclamp);
  xil_printf("Active Width:      %d\r\n",width);
  xil_printf("Active Height:     %d\r\n",height);

  xil_printf("\r\nCoefficients:",height);
  for(i=0; i<3; ++i)
  {
    xil_printf("\r\n r%d: ",i);
    for(j=0; j<3; ++j)
    {
      xil_printf("%4d ",coeff[i][j]);
    }
  }
}
/** @} */
