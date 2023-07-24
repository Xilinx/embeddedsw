/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_csc_l2.c
* @addtogroup v_csc Overview
* @{
* @details
*
* The CSC Layer-2 Driver. The functions in this file provides an abstraction
* from the register peek/poke methodology by implementing most common use-case
* provided by the sub-core. See xv_csc_l2.h for a detailed description of the
* layer-2 driver
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date    Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco   07/21/15 Initial Release
* 2.00  rco   11/05/15 Integrate layer-1 with layer-2
*       dmc   12/17/15 IsDemoWindowEnabled prevents access to absent HW regs
*                      Corrected typo in XV_CscSetColorspace setting K31 FW reg
*                      Updated the XV_CscDbgReportStatus routine
* 2.1   rco   02/09/17 Fix c++ warnings
* 2.2   vyc   10/04/17 Added support for 4:2:0
* 2.3   viv   06/19/18 Added support for color range
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
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
                            s32 *ClipMax,
                            XVidC_ColorRange cRangeOut);

static void cscFwRGBtoYCbCr(s32 K[3][4],
                            XVidC_ColorStd cstdOut,
                            s32 pixPrec,
                            s32 *ClampMin,
                            s32 *ClipMax,
                            XVidC_ColorRange cRangeOut);
#if 0 //currently not used
static void cscFwGetCoefficients(XV_Csc_l2 *CscPtr,
                                 s32 K[3][4],
                                 s32 *ClampMin,
                                 s32 *ClipMax);
#endif
static void cscFwSetCoefficients(XV_Csc_l2 *CscPtr,
                                 s32 K[3][4],
                                 s32 ClampMin,
                                 s32 ClipMax);
static void cscFwGetActiveCoefficients(XV_Csc_l2 *CscPtr, s32 K[3][4]);
static void cscFwSetActiveCoefficients(XV_Csc_l2 *CscPtr, s32 K[3][4]);
static void cscFwMatrixMult(s32 K1[3][4], s32 K2[3][4], s32 Kout[3][4]);
static void cscFwComputeCoeff(XV_Csc_l2 *CscPtr,
                              s32 K2[3][4]);
static void cscUpdateIPReg(XV_Csc_l2 *CscPtr,
                           XV_CSC_REG_UPDT_WIN win);
/*****************************************************************************/
/**
* This function provides the write interface for FW register bank
*
* @param  CscPtr is a pointer to layer 2 of csc core instance
* @param  offset is register offset
* @param  val is data to write
*
* @return None
*
******************************************************************************/
static __inline void cscFw_RegW(XV_Csc_l2 *CscPtr, u32 offset, s32 val)
{
  CscPtr->regMap[offset] = val;
}

/*****************************************************************************/
/**
* This function provides the read interface for FW register bank
*
* @param  CscPtr is a pointer to layer 2 of csc core instance
* @param  offset is register offset
*
* @return Register value at requested offset
*
*
******************************************************************************/
static __inline s32 cscFw_RegR(XV_Csc_l2 *CscPtr, u32 offset)
{
  return CscPtr->regMap[offset];
}

/*****************************************************************************/
/**
* This function initializes the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  DeviceId is instance id of the core
*
* @return XST_SUCCESS if device is found and initialized
*         XST_DEVICE_NOT_FOUND if device is not found
*
******************************************************************************/
int XV_CscInitialize(XV_Csc_l2 *InstancePtr, u16 DeviceId)
{
  int Status;
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Setup the instance */
  memset(InstancePtr, 0, sizeof(XV_Csc_l2));
  Status = XV_csc_Initialize(&InstancePtr->Csc, DeviceId);

  if(Status == XST_SUCCESS) {
	XV_CscSetPowerOnDefaultState(InstancePtr);
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function sets the CSC IP layer 2 fw registers to power on default state
*
* @param  CscPtr is a pointer to the layer 2
*
* @return None
*
******************************************************************************/
void XV_CscSetPowerOnDefaultState(XV_Csc_l2 *CscPtr)
{
  Xil_AssertVoid(CscPtr != NULL);

  CscPtr->ColorFormatIn     = XVIDC_CSF_RGB;
  CscPtr->ColorFormatOut    = XVIDC_CSF_RGB;
  CscPtr->StandardIn        = XVIDC_BT_601;
  CscPtr->StandardOut       = XVIDC_BT_601;
  CscPtr->ColorDepth        = XVIDC_BPC_8;
  CscPtr->Brightness        = 120;
  CscPtr->Contrast          = 0;
  CscPtr->Saturation        = 100;
  CscPtr->RedGain           = 120;
  CscPtr->GreenGain         = 120;
  CscPtr->BlueGain          = 120;
  CscPtr->Brightness_active = 120;
  CscPtr->Contrast_active   = 0;
  CscPtr->Saturation_active = 100;
  CscPtr->RedGain_active    = 120;
  CscPtr->GreenGain_active  = 120;
  CscPtr->BlueGain_active   = 120;
  CscPtr->K_active[0][0]    = (1<<XV_CSC_COEFF_FRACTIONAL_BITS);
  CscPtr->K_active[0][1]    = 0;
  CscPtr->K_active[0][2]    = 0;
  CscPtr->K_active[1][0]    = 0;
  CscPtr->K_active[1][1]    = (1<<XV_CSC_COEFF_FRACTIONAL_BITS);
  CscPtr->K_active[1][2]    = 0;
  CscPtr->K_active[2][0]    = 0;
  CscPtr->K_active[2][1]    = 0;
  CscPtr->K_active[2][2]    = (1<<XV_CSC_COEFF_FRACTIONAL_BITS);
  CscPtr->K_active[0][3]    = 0;
  CscPtr->K_active[1][3]    = 0;
  CscPtr->K_active[2][3]    = 0;
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
void XV_CscStart(XV_Csc_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_csc_EnableAutoRestart(&InstancePtr->Csc);
  XV_csc_Start(&InstancePtr->Csc);
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
void XV_CscStop(XV_Csc_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_csc_DisableAutoRestart(&InstancePtr->Csc);
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
void XV_CscSetActiveSize(XV_Csc_l2 *InstancePtr,
                          u32    width,
                          u32    height)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_csc_Set_HwReg_width(&InstancePtr->Csc,  width);
  XV_csc_Set_HwReg_height(&InstancePtr->Csc, height);

  if (XV_CscIsDemoWindowEnabled(InstancePtr)) {
    //Reset demo window to full frame
    XV_csc_Set_HwReg_ColStart(&InstancePtr->Csc, 0);
    XV_csc_Set_HwReg_ColEnd(&InstancePtr->Csc, (width-1));
    XV_csc_Set_HwReg_RowStart(&InstancePtr->Csc, 0);
    XV_csc_Set_HwReg_RowEnd(&InstancePtr->Csc, (height-1));
  }
}

/*****************************************************************************/
/**
* This function sets the demo window for the Color space converter. Any pixel
* outside the demo window will not be impacted and will be passed to output
* as-is
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  DemoWindow is structure that contains window coordinates and size
*
* @return XST_SUCCESS if this core instance has the demo window enabled
*         XST_FAILURE if this instance does not have the demo window enabled
*
******************************************************************************/
int XV_CscSetDemoWindow(XV_Csc_l2 *InstancePtr, XVidC_VideoWindow *DemoWindow)
{
  Xil_AssertNonvoid(InstancePtr != NULL);

  if (XV_CscIsDemoWindowEnabled(InstancePtr)) {
    XV_csc_Set_HwReg_ColStart(&InstancePtr->Csc, DemoWindow->StartX);
    XV_csc_Set_HwReg_ColEnd(&InstancePtr->Csc,
      (DemoWindow->StartX+DemoWindow->Width-1));
    XV_csc_Set_HwReg_RowStart(&InstancePtr->Csc, DemoWindow->StartY);
    XV_csc_Set_HwReg_RowEnd(&InstancePtr->Csc,
      (DemoWindow->StartY+DemoWindow->Height-1));
	return XST_SUCCESS;
  } else {
	return XST_FAILURE;
  }
}

/*****************************************************************************/
/**
 * This function configures the Color space converter to user specified
 * settings. Before any feature specific calls in layer-2 driver is made
 * the csc core should have been configured via this call.
 *
 * @param  InstancePtr is a pointer to the core instance to be worked on.
 * @param  cfmtIn is input color space
 * @param  cfmtOut is output color space
 * @param  cstdIn is input color standard
 * @param  cstdOut is output color standard
 * @param  cRangeOut is the selected output range
 *
 * @return XST_SUCCESS if the requested color format is allowed
 *         XST_FAILURE if YUV422/420 color format is requested but not allowed
 *
 *****************************************************************************/
int XV_CscSetColorspace(XV_Csc_l2 *InstancePtr,
                         XVidC_ColorFormat cfmtIn,
                         XVidC_ColorFormat cfmtOut,
                         XVidC_ColorStd cstdIn,
                         XVidC_ColorStd cstdOut,
                         XVidC_ColorRange cRangeOut
                        )
{
  s32 K[3][4], K1[3][4], K2[3][4];
  s32 ClampMin = 0;
  s32 ClipMax;
  s32 scale_factor;
  XV_csc *pCsc = &InstancePtr->Csc;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertNonvoid(InstancePtr != NULL);

  // if 422 is requested but not allowed, return failure
  if (((cfmtIn == XVIDC_CSF_YCRCB_422)||(cfmtOut == XVIDC_CSF_YCRCB_422)) &&
      !XV_CscIs422Enabled(InstancePtr)) {
    return XST_FAILURE;
  }

  // if 420 is requested but not allowed, return failure
  if (((cfmtIn == XVIDC_CSF_YCRCB_420)||(cfmtOut == XVIDC_CSF_YCRCB_420)) &&
      !XV_CscIs420Enabled(InstancePtr)) {
    return XST_FAILURE;
  }

  ClipMax  = ((1<<InstancePtr->ColorDepth)-1);
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
  //RGB in and 444/422/420 out
  if ((cfmtIn == XVIDC_CSF_RGB) && (cfmtOut != XVIDC_CSF_RGB) )
  {
    cscFwRGBtoYCbCr(K, cstdOut, InstancePtr->ColorDepth, &ClampMin, &ClipMax, cRangeOut);
  }
  //444/422/420 in and RGB out
  else if ((cfmtIn != XVIDC_CSF_RGB) && (cfmtOut == XVIDC_CSF_RGB))
  {
    cscFwYCbCrtoRGB(K, cstdIn, InstancePtr->ColorDepth, &ClampMin, &ClipMax, cRangeOut);
  }
  //RGB in and RGB out
  else if ((cfmtIn == XVIDC_CSF_RGB) && (cfmtOut == XVIDC_CSF_RGB) )
  {
    //nop
  }
  //444/422/420 in and 444/422/420 out
  else
  {
    //color standard change from input to output
    if (cstdIn != cstdOut)
    {
      cscFwYCbCrtoRGB(K1, cstdIn,  InstancePtr->ColorDepth, &ClampMin, &ClipMax, cRangeOut);
      cscFwRGBtoYCbCr(K2, cstdOut, InstancePtr->ColorDepth, &ClampMin, &ClipMax, cRangeOut);
      cscFwMatrixMult(K1, K2, K);
    }
  }
  //update fw registers
  InstancePtr->ColorFormatIn  = cfmtIn;
  InstancePtr->ColorFormatOut = cfmtOut;
  InstancePtr->StandardIn     = cstdIn;
  InstancePtr->StandardOut    = cstdOut;
  InstancePtr->OutputRange    = cRangeOut;

  cscFw_RegW(InstancePtr, CSC_FW_REG_K11,K[0][0]);
  cscFw_RegW(InstancePtr, CSC_FW_REG_K12,K[0][1]);
  cscFw_RegW(InstancePtr, CSC_FW_REG_K13,K[0][2]);
  cscFw_RegW(InstancePtr, CSC_FW_REG_K21,K[1][0]);
  cscFw_RegW(InstancePtr, CSC_FW_REG_K22,K[1][1]);
  cscFw_RegW(InstancePtr, CSC_FW_REG_K23,K[1][2]);
  cscFw_RegW(InstancePtr, CSC_FW_REG_K31,K[2][0]);
  cscFw_RegW(InstancePtr, CSC_FW_REG_K32,K[2][1]);
  cscFw_RegW(InstancePtr, CSC_FW_REG_K33,K[2][2]);
  cscFw_RegW(InstancePtr, CSC_FW_REG_ROffset,K[0][3]);
  cscFw_RegW(InstancePtr, CSC_FW_REG_GOffset,K[1][3]);
  cscFw_RegW(InstancePtr, CSC_FW_REG_BOffset,K[2][3]);
  cscFw_RegW(InstancePtr, CSC_FW_REG_ClampMin,ClampMin);
  cscFw_RegW(InstancePtr, CSC_FW_REG_ClipMax,ClipMax);

  //compute coeff for Demo window
  cscFwComputeCoeff(InstancePtr, InstancePtr->K_active);

  //write IP Registers
  cscUpdateIPReg(InstancePtr, UPDT_REG_FULL_FRAME);
  cscUpdateIPReg(InstancePtr, UPD_REG_DEMO_WIN);

  return XST_SUCCESS;
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
                            s32 *ClipMax,
                            XVidC_ColorRange cRangeOut)
{
  s32 scale_factor = (1<<XV_CSC_COEFF_FRACTIONAL_BITS);
  s32 bpcScale = (1<<(pixPrec-8));

  switch(cstdIn)
  {
    case XVIDC_BT_601:
        switch(cRangeOut)
        {
          case XVIDC_CR_0_255:
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
              YCC2RGB[2][3] = (s32) -277*bpcScale;
              break;

          case XVIDC_CR_16_235:
              YCC2RGB[0][0] = (s32) ( 1.0000*(float)scale_factor);  //K11
              YCC2RGB[0][1] = (s32)  0;                             //K12
              YCC2RGB[0][2] = (s32) ( 1.3669*(float)scale_factor);  //K13
              YCC2RGB[1][0] = (s32) ( 1.0000*(float)scale_factor);  //K21
              YCC2RGB[1][1] = (s32) (-0.3367*(float)scale_factor);  //K22
              YCC2RGB[1][2] = (s32) (-0.6986*(float)scale_factor);  //K23
              YCC2RGB[2][0] = (s32)  (1.0000*(float)scale_factor);  //K31
              YCC2RGB[2][1] = (s32) ( 1.7335*(float)scale_factor);  //K32
              YCC2RGB[2][2] = (s32)  0;                             //K33
              YCC2RGB[0][3] = (s32) -175*bpcScale;                  //R Offset
              YCC2RGB[1][3] = (s32)  132*bpcScale;                  //G Offset
              YCC2RGB[2][3] = (s32) -222*bpcScale;
       	      break;

          case XVIDC_CR_16_240:
              YCC2RGB[0][0] = (s32) ( 1.0479*(float)scale_factor);  //K11
              YCC2RGB[0][1] = (s32)  0;                             //K12
              YCC2RGB[0][2] = (s32) ( 1.3979*(float)scale_factor);  //K13
              YCC2RGB[1][0] = (s32) ( 1.0479*(float)scale_factor);  //K21
              YCC2RGB[1][1] = (s32) (-0.3443*(float)scale_factor);  //K22
              YCC2RGB[1][2] = (s32) (-0.7145*(float)scale_factor);  //K23
              YCC2RGB[2][0] = (s32)  (1.0479*(float)scale_factor);  //K31
              YCC2RGB[2][1] = (s32) ( 1.7729*(float)scale_factor);  //K32
              YCC2RGB[2][2] = (s32)  0;                             //K33
              YCC2RGB[0][3] = (s32) -179*bpcScale;                  //R Offset
              YCC2RGB[1][3] = (s32)  136*bpcScale;                  //G Offset
              YCC2RGB[2][3] = (s32) -227*bpcScale;
	      break;

          default:
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
        break;

    case XVIDC_BT_709:
        switch(cRangeOut)
        {
          case XVIDC_CR_0_255:
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

          case XVIDC_CR_16_235:
              YCC2RGB[0][0] = (s32) ( 1.0000*(float)scale_factor);  //K11
              YCC2RGB[0][1] = (s32)  0;                             //K12
              YCC2RGB[0][2] = (s32) ( 1.5406*(float)scale_factor);  //K13
              YCC2RGB[1][0] = (s32) ( 1.0000*(float)scale_factor);  //K21
              YCC2RGB[1][1] = (s32) (-0.1832*(float)scale_factor);  //K22
              YCC2RGB[1][2] = (s32) (-0.4579*(float)scale_factor);  //K23
              YCC2RGB[2][0] = (s32) ( 1.0000*(float)scale_factor);  //K31
              YCC2RGB[2][1] = (s32) ( 1.8153*(float)scale_factor);  //K32
              YCC2RGB[2][2] = (s32)  0;                             //K33
              YCC2RGB[0][3] = (s32) -197*bpcScale;                  //R Offset
              YCC2RGB[1][3] = (s32)  82*bpcScale;                   //G Offset
              YCC2RGB[2][3] = (s32) -232*bpcScale;                  //B Offset
              break;

          case XVIDC_CR_16_240:
              YCC2RGB[0][0] = (s32) ( 1.0233*(float)scale_factor);  //K11
              YCC2RGB[0][1] = (s32)  0;                             //K12
              YCC2RGB[0][2] = (s32) ( 1.5756*(float)scale_factor);  //K13
              YCC2RGB[1][0] = (s32) ( 1.0233*(float)scale_factor);  //K21
              YCC2RGB[1][1] = (s32) (-0.1873*(float)scale_factor);  //K22
              YCC2RGB[1][2] = (s32) (-0.4683*(float)scale_factor);  //K23
              YCC2RGB[2][0] = (s32) ( 1.0233*(float)scale_factor);  //K31
              YCC2RGB[2][1] = (s32) ( 1.8566*(float)scale_factor);  //K32
              YCC2RGB[2][2] = (s32)  0;                             //K33
              YCC2RGB[0][3] = (s32) -202*bpcScale;                  //R Offset
              YCC2RGB[1][3] = (s32)  84*bpcScale;                   //G Offset
              YCC2RGB[2][3] = (s32) -238*bpcScale;                  //B Offset
              break;

          default:
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
        }
        break;

    case XVIDC_BT_2020:
        switch(cRangeOut)
 	{
          case XVIDC_CR_0_255:
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

          case XVIDC_CR_16_235:
              YCC2RGB[0][0] = (s32) ( 1.0000*(float)scale_factor);  //K11
              YCC2RGB[0][1] = (s32)  0;                             //K12
              YCC2RGB[0][2] = (s32) ( 1.4426*(float)scale_factor);  //K13
              YCC2RGB[1][0] = (s32) ( 1.0000*(float)scale_factor);  //K21
              YCC2RGB[1][1] = (s32) (-0.1609*(float)scale_factor);  //K22
              YCC2RGB[1][2] = (s32) (-0.5589*(float)scale_factor);  //K23
              YCC2RGB[2][0] = (s32) ( 1.0000*(float)scale_factor);  //K31
              YCC2RGB[2][1] = (s32) ( 1.8406*(float)scale_factor);  //K32
              YCC2RGB[2][2] = (s32)  0;                             //K33
              YCC2RGB[0][3] = (s32) -185*bpcScale;                  //R Offset
              YCC2RGB[1][3] = (s32)  92*bpcScale;                   //G Offset
              YCC2RGB[2][3] = (s32) -236*bpcScale;                  //B Offset
              break;

          case XVIDC_CR_16_240:
              YCC2RGB[0][0] = (s32) ( 1.0233*(float)scale_factor);  //K11
              YCC2RGB[0][1] = (s32)  0;                             //K12
              YCC2RGB[0][2] = (s32) ( 1.4754*(float)scale_factor);  //K13
              YCC2RGB[1][0] = (s32) ( 1.0233*(float)scale_factor);  //K21
              YCC2RGB[1][1] = (s32) (-0.1646*(float)scale_factor);  //K22
              YCC2RGB[1][2] = (s32) (-0.5716*(float)scale_factor);  //K23
              YCC2RGB[2][0] = (s32) ( 1.0233*(float)scale_factor);  //K31
              YCC2RGB[2][1] = (s32) ( 1.8824*(float)scale_factor);  //K32
              YCC2RGB[2][2] = (s32)  0;                             //K33
              YCC2RGB[0][3] = (s32) -189*bpcScale;                  //R Offset
              YCC2RGB[1][3] = (s32)  94*bpcScale;                   //G Offset
              YCC2RGB[2][3] = (s32) -241*bpcScale;                  //B Offset
              break;

          default:
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
        }
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
                            s32 *ClipMax,
                            XVidC_ColorRange cRangeOut)
{
  s32 scale_factor = (1<<XV_CSC_COEFF_FRACTIONAL_BITS);
  s32 bpcScale = (1<<(pixPrec-8));

  switch(cstdOut)
  {
    case XVIDC_BT_601:
        switch(cRangeOut)
 	{
          case XVIDC_CR_0_255:
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

          case XVIDC_CR_16_235:
              RGB2YCC[0][0] = (s32) ( 0.299*(float)scale_factor);  //K11
              RGB2YCC[0][1] = (s32) ( 0.587*(float)scale_factor);  //K12
              RGB2YCC[0][2] = (s32) ( 0.144*(float)scale_factor);  //K13
              RGB2YCC[1][0] = (s32) (-0.172*(float)scale_factor);  //K21
              RGB2YCC[1][1] = (s32) (-0.339*(float)scale_factor);  //K22
              RGB2YCC[1][2] = (s32) ( 0.511*(float)scale_factor);  //K23
              RGB2YCC[2][0] = (s32) ( 0.511*(float)scale_factor);  //K31
              RGB2YCC[2][1] = (s32) (-0.428*(float)scale_factor);  //K32
              RGB2YCC[2][2] = (s32) (-0.083*(float)scale_factor);  //K33
              RGB2YCC[0][3] =  0*bpcScale;                   //R Offset
              RGB2YCC[1][3] =  128*bpcScale;                  //G Offset
              RGB2YCC[2][3] =  128*bpcScale;                  //B Offset
              break;

          case XVIDC_CR_16_240:
              RGB2YCC[0][0] = (s32) ( 0.2921*(float)scale_factor);  //K11
              RGB2YCC[0][1] = (s32) ( 0.5735*(float)scale_factor);  //K12
              RGB2YCC[0][2] = (s32) ( 0.1113*(float)scale_factor);  //K13
              RGB2YCC[1][0] = (s32) (-0.1686*(float)scale_factor);  //K21
              RGB2YCC[1][1] = (s32) (-0.3310*(float)scale_factor);  //K22
              RGB2YCC[1][2] = (s32) ( 0.4393*(float)scale_factor);  //K23
              RGB2YCC[2][0] = (s32) ( 0.4393*(float)scale_factor);  //K31
              RGB2YCC[2][1] = (s32) (-0.4184*(float)scale_factor);  //K32
              RGB2YCC[2][2] = (s32) (-0.0812*(float)scale_factor);  //K33
              RGB2YCC[0][3] =  0*bpcScale;                   //R Offset
              RGB2YCC[1][3] =  128*bpcScale;                  //G Offset
              RGB2YCC[2][3] =  128*bpcScale;                  //B Offset
              break;

          default:
              RGB2YCC[0][0] = (s32) ( 0.2568*(float)scale_factor);  //K11
              RGB2YCC[0][1] = (s32) ( 0.5041*(float)scale_factor);  //K12
              RGB2YCC[0][2] = (s32) ( 0.0979*(float)scale_factor);  //K13
              RGB2YCC[1][0] = (s32) (-0.1482*(float)scale_factor);  //K21
              RGB2YCC[1][1] = (s32) (-0.2910*(float)scale_factor);  //K22
              RGB2YCC[1][2] = (s32) ( 0.4999*(float)scale_factor);  //K23
              RGB2YCC[2][0] = (s32) ( 0.4999*(float)scale_factor);  //K31
              RGB2YCC[2][1] = (s32) (-0.3678*(float)scale_factor);  //K32
              RGB2YCC[2][2] = (s32) (-0.0714*(float)scale_factor);  //K33
              RGB2YCC[0][3] =  16*bpcScale;                   //R Offset
              RGB2YCC[1][3] =  128*bpcScale;                  //G Offset
              RGB2YCC[2][3] =  128*bpcScale;                  //B Offset
              break;
        }
        break;

    case XVIDC_BT_709:
        switch(cRangeOut)
        {
          case XVIDC_CR_0_255:
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

          case XVIDC_CR_16_235:
              RGB2YCC[0][0] = (s32) ( 0.212*(float)scale_factor);  //K11
              RGB2YCC[0][1] = (s32) ( 0.715*(float)scale_factor);  //K12
              RGB2YCC[0][2] = (s32) ( 0.072*(float)scale_factor);  //K13
              RGB2YCC[1][0] = (s32) (-0.117*(float)scale_factor);  //K21
              RGB2YCC[1][1] = (s32) (-0.394*(float)scale_factor);  //K22
              RGB2YCC[1][2] = (s32) ( 0.511*(float)scale_factor);  //K23
              RGB2YCC[2][0] = (s32) ( 0.51*(float)scale_factor);  //K31
              RGB2YCC[2][1] = (s32) (-0.464*(float)scale_factor);  //K32
              RGB2YCC[2][2] = (s32) (-0.047*(float)scale_factor);  //K33
              RGB2YCC[0][3] =  0*bpcScale;                   //R Offset
              RGB2YCC[1][3] =  128*bpcScale;                  //G Offset
              RGB2YCC[2][3] =  128*bpcScale;                  //B Offset
              break;

          case XVIDC_CR_16_240:
              RGB2YCC[0][0] = (s32) ( 0.2077*(float)scale_factor);  //K11
              RGB2YCC[0][1] = (s32) ( 0.6988*(float)scale_factor);  //K12
              RGB2YCC[0][2] = (s32) ( 0.0705*(float)scale_factor);  //K13
              RGB2YCC[1][0] = (s32) (-0.1144*(float)scale_factor);  //K21
              RGB2YCC[1][1] = (s32) (-0.3582*(float)scale_factor);  //K22
              RGB2YCC[1][2] = (s32) ( 0.4997*(float)scale_factor);  //K23
              RGB2YCC[2][0] = (s32) ( 0.4997*(float)scale_factor);  //K31
              RGB2YCC[2][1] = (s32) (-0.4538*(float)scale_factor);  //K32
              RGB2YCC[2][2] = (s32) (-0.0458*(float)scale_factor);  //K33
              RGB2YCC[0][3] =  0*bpcScale;                   //R Offset
              RGB2YCC[1][3] =  128*bpcScale;                  //G Offset
              RGB2YCC[2][3] =  128*bpcScale;                  //B Offset
              break;

          default:
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
        }
        break;

    case XVIDC_BT_2020:
        switch(cRangeOut)
        {
          case XVIDC_CR_0_255:
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

          case XVIDC_CR_16_235:
              RGB2YCC[0][0] = (s32) ( 0.2625*(float)scale_factor);  //K11
              RGB2YCC[0][1] = (s32) ( 0.6775*(float)scale_factor);  //K12
              RGB2YCC[0][2] = (s32) ( 0.0592*(float)scale_factor);  //K13
              RGB2YCC[1][0] = (s32) (-0.1427*(float)scale_factor);  //K21
              RGB2YCC[1][1] = (s32) (-0.3684*(float)scale_factor);  //K22
              RGB2YCC[1][2] = (s32) ( 0.5110*(float)scale_factor);  //K23
              RGB2YCC[2][0] = (s32) ( 0.5110*(float)scale_factor);  //K31
              RGB2YCC[2][1] = (s32) (-0.4699*(float)scale_factor);  //K32
              RGB2YCC[2][2] = (s32) (-0.0410*(float)scale_factor);  //K33
              RGB2YCC[0][3] =  0*bpcScale;                   //R Offset
              RGB2YCC[1][3] =  128*bpcScale;                  //G Offset
              RGB2YCC[2][3] =  128*bpcScale;                  //B Offset
              break;

          case XVIDC_CR_16_240:
              RGB2YCC[0][0] = (s32) ( 0.2566*(float)scale_factor);  //K11
              RGB2YCC[0][1] = (s32) ( 0.6625*(float)scale_factor);  //K12
              RGB2YCC[0][2] = (s32) ( 0.0579*(float)scale_factor);  //K13
              RGB2YCC[1][0] = (s32) (-0.1396*(float)scale_factor);  //K21
              RGB2YCC[1][1] = (s32) (-0.3602*(float)scale_factor);  //K22
              RGB2YCC[1][2] = (s32) ( 0.4997*(float)scale_factor);  //K23
              RGB2YCC[2][0] = (s32) ( 0.4997*(float)scale_factor);  //K31
              RGB2YCC[2][1] = (s32) (-0.4595*(float)scale_factor);  //K32
              RGB2YCC[2][2] = (s32) (-0.0401*(float)scale_factor);  //K33
              RGB2YCC[0][3] =  0*bpcScale;                   //R Offset
              RGB2YCC[1][3] =  128*bpcScale;                  //G Offset
              RGB2YCC[2][3] =  128*bpcScale;                  //B Offset
              break;

          default:
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
        }
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
* @param  CscPtr is the pointer to layer 2 fw register bank
* @param  K is the array to hold coefficients
*
* @return
* 	- ClampMin is min value to clamp
*   - ClipMax  is max value to saturate
*
******************************************************************************/
static void cscFwGetCoefficients(XV_Csc_l2 *CscPtr,
                                 s32 K[3][4],
                                 s32 *ClampMin,
                                 s32 *ClipMax)
{
  K[0][0]  = cscFw_RegR(CscPtr, CSC_FW_REG_K11_2);
  K[0][1]  = cscFw_RegR(CscPtr, CSC_FW_REG_K12_2);
  K[0][2]  = cscFw_RegR(CscPtr, CSC_FW_REG_K13_2);
  K[1][0]  = cscFw_RegR(CscPtr, CSC_FW_REG_K21_2);
  K[1][1]  = cscFw_RegR(CscPtr, CSC_FW_REG_K22_2);
  K[1][2]  = cscFw_RegR(CscPtr, CSC_FW_REG_K23_2);
  K[2][0]  = cscFw_RegR(CscPtr, CSC_FW_REG_K31_2);
  K[2][1]  = cscFw_RegR(CscPtr, CSC_FW_REG_K32_2);
  K[2][2]  = cscFw_RegR(CscPtr, CSC_FW_REG_K33_2);
  K[0][3]  = cscFw_RegR(CscPtr, CSC_FW_REG_ROffset_2);
  K[1][3]  = cscFw_RegR(CscPtr, CSC_FW_REG_GOffset_2);
  K[2][3]  = cscFw_RegR(CscPtr, CSC_FW_REG_BOffset_2);
  *ClampMin = cscFw_RegR(CscPtr, CSC_FW_REG_ClampMin_2);
  *ClipMax  = cscFw_RegR(CscPtr, CSC_FW_REG_ClipMax_2);
}
#endif

/*****************************************************************************/
/**
* This function writes demo window coefficients to layer 2 fw register bank
*
* @param  CscPtr is a pointer to layer 2 fw register bank
* @param  K is an array to hold coefficients
* @param  ClampMin is min value to clamp
* @param  ClipMax is max value to saturate
*
* @return None
*
******************************************************************************/
static void cscFwSetCoefficients(XV_Csc_l2 *CscPtr,
                                 s32 K[3][4],
                                 s32 ClampMin,
                                 s32 ClipMax)
{
  cscFw_RegW(CscPtr, CSC_FW_REG_K11_2,K[0][0]);
  cscFw_RegW(CscPtr, CSC_FW_REG_K12_2,K[0][1]);
  cscFw_RegW(CscPtr, CSC_FW_REG_K13_2,K[0][2]);
  cscFw_RegW(CscPtr, CSC_FW_REG_K21_2,K[1][0]);
  cscFw_RegW(CscPtr, CSC_FW_REG_K22_2,K[1][1]);
  cscFw_RegW(CscPtr, CSC_FW_REG_K23_2,K[1][2]);
  cscFw_RegW(CscPtr, CSC_FW_REG_K31_2,K[2][0]);
  cscFw_RegW(CscPtr, CSC_FW_REG_K32_2,K[2][1]);
  cscFw_RegW(CscPtr, CSC_FW_REG_K33_2,K[2][2]);
  cscFw_RegW(CscPtr, CSC_FW_REG_ROffset_2,K[0][3]);
  cscFw_RegW(CscPtr, CSC_FW_REG_GOffset_2,K[1][3]);
  cscFw_RegW(CscPtr, CSC_FW_REG_BOffset_2,K[2][3]);
  cscFw_RegW(CscPtr, CSC_FW_REG_ClampMin_2,ClampMin);
  cscFw_RegW(CscPtr, CSC_FW_REG_ClipMax_2,ClipMax);
}

/*****************************************************************************/
/**
* This function gets the current RGB coefficients for demo window
*
* @param  CscPtr is the pointer to layer 2 fw register bank
* @param  K is the array to hold coefficients
*
* @return None
*
******************************************************************************/
static void cscFwGetActiveCoefficients(XV_Csc_l2 *CscPtr, s32 K[3][4])
{
  u8 x,y;

  for(x=0; x<3; ++x)
  {
    for(y=0; y<4; ++y)
    {
      K[x][y] = CscPtr->K_active[x][y];
    }
  }
}

/*****************************************************************************/
/**
* This function sets the current RGB coefficients for demo window
*
* @param  CscPtr is the pointer to layer 2 fw register bank
* @param  K is the array to hold coefficients
*
* @return None
*
******************************************************************************/
static void cscFwSetActiveCoefficients(XV_Csc_l2 *CscPtr, s32 K[3][4])
{
  u8 x,y;

  for(x=0; x<3; ++x)
  {
    for(y=0; y<4; ++y)
    {
        CscPtr->K_active[x][y] = K[x][y];
    }
  }
}

/*****************************************************************************/
/**
* This function set brightness to specified value. It also translates user
* setting of 0-100 to hw register range
*
* @param  InstancePtr is pointer to csc core instance
* @param  val is new brightness value
*
* @return None
*
******************************************************************************/
void XV_CscSetBrightness(XV_Csc_l2 *InstancePtr, s32 val)
{
  s32 K1[3][4], K2[3][4];
  float brightness_f;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  InstancePtr->Brightness = (val*2+20);
  brightness_f = (float)(InstancePtr->Brightness)/(float)(InstancePtr->Brightness_active);

  //get active coefficient set in RGB
  cscFwGetActiveCoefficients(InstancePtr, K1);

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
  cscFwSetActiveCoefficients(InstancePtr, K2);
  //write new active brightness value
  InstancePtr->Brightness_active = InstancePtr->Brightness;

  cscFwComputeCoeff(InstancePtr, K2);
  cscUpdateIPReg(InstancePtr, UPD_REG_DEMO_WIN);
}

/*****************************************************************************/
/**
* This function set contrast to specified value. It also translates user
* setting of 0-100 to hw register range
*
* @param  InstancePtr is pointer to csc core instance
* @param  val is new contrast value
*
* @return None
*
******************************************************************************/
void XV_CscSetContrast(XV_Csc_l2 *InstancePtr, s32 val)
{
  s32 contrast;
  s32 K1[3][4], K2[3][4];
  s32 scale;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  InstancePtr->Contrast = val*4 - 200;
  contrast = (InstancePtr->Contrast) - (InstancePtr->Contrast_active);

  scale = (1<<(InstancePtr->ColorDepth-8));
  //get active coefficient set in RGB
  cscFwGetActiveCoefficients(InstancePtr, K1);

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
  cscFwSetActiveCoefficients(InstancePtr, K2);
  //write new active contrast value
  InstancePtr->Contrast_active = InstancePtr->Contrast;

  cscFwComputeCoeff(InstancePtr, K2);
  cscUpdateIPReg(InstancePtr, UPD_REG_DEMO_WIN);
}

/*****************************************************************************/
/**
* This function set saturation to specified value. It also translates user
* setting of 0-100 to hw register range
*
* @param  InstancePtr is pointer to csc core instance
* @param  val is new saturation value
*
* @return None
*
******************************************************************************/
void XV_CscSetSaturation(XV_Csc_l2 *InstancePtr, s32 val)
{
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

  ClipMax  = ((1<<InstancePtr->ColorDepth)-1);
  scale_factor = (1<<XV_CSC_COEFF_FRACTIONAL_BITS);

  InstancePtr->Saturation = ((val == 0) ? 1 : val*2);
  saturation_f = (float)(InstancePtr->Saturation)/(float)(InstancePtr->Saturation_active);

  //get active coefficient set in RGB
  cscFwGetActiveCoefficients(InstancePtr, K1);

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
  cscFwSetActiveCoefficients(InstancePtr, K3);
  //write new active saturation value
  InstancePtr->Saturation_active = InstancePtr->Saturation;

  //RGB in and RGB out
  if ((InstancePtr->ColorFormatIn == XVIDC_CSF_RGB) &&
      (InstancePtr->ColorFormatOut == XVIDC_CSF_RGB) )
  {
    for (x=0; x<3; x++)  for (y=0; y<4; y++)
      Kout[x][y] = K3[x][y];
  }

  //RGB in and 444/422/420 out
  if ((InstancePtr->ColorFormatIn == XVIDC_CSF_RGB) &&
      (InstancePtr->ColorFormatOut != XVIDC_CSF_RGB) )
  {
    cscFwRGBtoYCbCr(M2, InstancePtr->StandardOut, InstancePtr->ColorDepth, &ClampMin, &ClipMax, InstancePtr->OutputRange);
    cscFwMatrixMult(K3, M2, Kout);
  }

  //444/422/420 in and RGB out
  if ((InstancePtr->ColorFormatIn != XVIDC_CSF_RGB) &&
      (InstancePtr->ColorFormatOut == XVIDC_CSF_RGB) )
  {
    cscFwYCbCrtoRGB(M1, InstancePtr->StandardIn, InstancePtr->ColorDepth, &ClampMin, &ClipMax, InstancePtr->OutputRange);
    cscFwMatrixMult(M1, K3, Kout);
  }

  //444/422/420 in and 444/422/420 out
  if ((InstancePtr->ColorFormatIn != XVIDC_CSF_RGB) &&
      (InstancePtr->ColorFormatOut != XVIDC_CSF_RGB) )
  {
    cscFwYCbCrtoRGB(M1, InstancePtr->StandardIn, InstancePtr->ColorDepth, &ClampMin, &ClipMax, InstancePtr->OutputRange);
    cscFwMatrixMult(M1, K3, K4);
    cscFwRGBtoYCbCr(M2, InstancePtr->StandardOut, InstancePtr->ColorDepth, &ClampMin, &ClipMax, InstancePtr->OutputRange);
    cscFwMatrixMult(K4, M2, Kout);
  }

  cscFwSetCoefficients(InstancePtr, Kout, ClampMin, ClipMax);
  cscUpdateIPReg(InstancePtr, UPD_REG_DEMO_WIN);
}

/*****************************************************************************/
/**
* This function set red gain to specified value. It also translates user
* setting of 0-100 to hw register range
*
* @param  InstancePtr is pointer to csc core instance
* @param  val is new gain value
*
* @return None
*
******************************************************************************/
void XV_CscSetRedGain(XV_Csc_l2 *InstancePtr, s32 val)
{
  s32 K1[3][4], K2[3][4];
  float red_f;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  InstancePtr->RedGain = (val*2+20);
  red_f = (float)(InstancePtr->RedGain)/(float)(InstancePtr->RedGain_active);

  //get active coefficient set in RGB
  cscFwGetActiveCoefficients(InstancePtr, K1);

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
  cscFwSetActiveCoefficients(InstancePtr, K2);
  //write new active red value
  InstancePtr->RedGain_active = InstancePtr->RedGain;

  cscFwComputeCoeff(InstancePtr, K2);
  cscUpdateIPReg(InstancePtr, UPD_REG_DEMO_WIN);
}

/*****************************************************************************/
/**
* This function set green gain to specified value. It also translates user
* setting of 0-100 to hw register range
*
* @param  InstancePtr is pointer to csc core instance
* @param  val is new gain value
*
* @return None
*
******************************************************************************/
void XV_CscSetGreenGain(XV_Csc_l2 *InstancePtr, s32 val)
{
  s32 K1[3][4], K2[3][4];
  float green_f;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  InstancePtr->GreenGain = (val*2+20);
  green_f = (float)(InstancePtr->GreenGain)/(float)(InstancePtr->GreenGain_active);

  //get active coefficient set in RGB
  cscFwGetActiveCoefficients(InstancePtr, K1);

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
  cscFwSetActiveCoefficients(InstancePtr, K2);
  //write new active green value
  InstancePtr->GreenGain_active = InstancePtr->GreenGain;

  cscFwComputeCoeff(InstancePtr, K2);
  cscUpdateIPReg(InstancePtr, UPD_REG_DEMO_WIN);
}

/*****************************************************************************/
/**
* This function set blue gain to specified value. It also translates user
* setting of 0-100 to hw register range
*
* @param  InstancePtr is pointer to csc core instance
* @param  val is new gain value
*
* @return None
*
******************************************************************************/
void XV_CscSetBlueGain(XV_Csc_l2 *InstancePtr, s32 val)
{
  s32 K1[3][4], K2[3][4];
  float blue_f;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  InstancePtr->BlueGain = (val*2+20);
  blue_f = (float)(InstancePtr->BlueGain)/(float)(InstancePtr->BlueGain_active);

  //get active coefficient set in RGB
  cscFwGetActiveCoefficients(InstancePtr, K1);

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
  cscFwSetActiveCoefficients(InstancePtr, K2);
  //write new active blue value
  InstancePtr->BlueGain_active = InstancePtr->BlueGain;

  cscFwComputeCoeff(InstancePtr, K2);
  cscUpdateIPReg(InstancePtr, UPD_REG_DEMO_WIN);
}


/*****************************************************************************/
/**
* Compute the coefficients for the required color space and write to layer 2
* fw register bank
*
* @param  CscPtr is a pointer to layer 2 fw register bank
* @param  K2 is the active coefficients
*
* @return None
*
******************************************************************************/
static void cscFwComputeCoeff(XV_Csc_l2 *CscPtr,
                              s32 K2[3][4])
{
  u32 x,y;
  s32 K3[3][4], M1[3][4], M2[3][4], Kout[3][4];;
  s32 ClampMin = 0;
  s32 ClipMax  = ((1<<CscPtr->ColorDepth)-1);

  //RGB in and RGB out
  if((CscPtr->ColorFormatIn == XVIDC_CSF_RGB) &&
     (CscPtr->ColorFormatOut == XVIDC_CSF_RGB) )
  {
    for (x=0; x<3; x++)  for (y=0; y<4; y++)
      Kout[x][y] = K2[x][y];
  }
  //RGB in and 444/422/420 out
  else if ((CscPtr->ColorFormatIn == XVIDC_CSF_RGB) &&
          (CscPtr->ColorFormatOut != XVIDC_CSF_RGB) )
  {
    cscFwRGBtoYCbCr(M2, CscPtr->StandardOut, CscPtr->ColorDepth, &ClampMin, &ClipMax, CscPtr->OutputRange);
    cscFwMatrixMult(K2, M2, Kout);
  }
  //444/422/420 in and RGB out
  else if ((CscPtr->ColorFormatIn != XVIDC_CSF_RGB) &&
          (CscPtr->ColorFormatOut == XVIDC_CSF_RGB) )
  {
    cscFwYCbCrtoRGB(M1, CscPtr->StandardIn, CscPtr->ColorDepth, &ClampMin, &ClipMax, CscPtr->OutputRange);
    cscFwMatrixMult(M1, K2, Kout);
  }
  //444/422/420 in and 444/422/420 out
  else
  {
    cscFwYCbCrtoRGB(M1, CscPtr->StandardIn, CscPtr->ColorDepth, &ClampMin, &ClipMax, CscPtr->OutputRange);
    cscFwMatrixMult(M1, K2, K3);
    cscFwRGBtoYCbCr(M2, CscPtr->StandardOut, CscPtr->ColorDepth, &ClampMin, &ClipMax, CscPtr->OutputRange);
    cscFwMatrixMult(K3, M2, Kout);
  }
  cscFwSetCoefficients(CscPtr, Kout, ClampMin, ClipMax);
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
* @param  CscPtr is a pointer to layer 2 fw register bank
* @param  win is the window mode: Full Frame or Demo window
*
* @return None
*
******************************************************************************/
static void cscUpdateIPReg(XV_Csc_l2 *CscPtr,
                           XV_CSC_REG_UPDT_WIN win)
{
  u8 x,y;
  s32 K[3][4];
  u32 clampMin, clipMax;
  XV_csc *pCsc = &CscPtr->Csc;

  switch(win)
  {
    case UPDT_REG_FULL_FRAME:
        for(x=0; x<3; ++x)
        {
          for(y=0; y<3; ++y)
          {
            K[x][y] = cscFw_RegR(CscPtr, (x*3+y)+CSC_FW_REG_K11);
          }
        }
        K[0][3] = cscFw_RegR(CscPtr, CSC_FW_REG_ROffset);
        K[1][3] = cscFw_RegR(CscPtr, CSC_FW_REG_GOffset);
        K[2][3] = cscFw_RegR(CscPtr, CSC_FW_REG_BOffset);
        clampMin = cscFw_RegR(CscPtr, CSC_FW_REG_ClampMin);
        clipMax  = cscFw_RegR(CscPtr, CSC_FW_REG_ClipMax);

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
            K[x][y] = cscFw_RegR(CscPtr, (x*3+y)+CSC_FW_REG_K11_2);
          }
        }
        K[0][3] = cscFw_RegR(CscPtr, CSC_FW_REG_ROffset_2);
        K[1][3] = cscFw_RegR(CscPtr, CSC_FW_REG_GOffset_2);
        K[2][3] = cscFw_RegR(CscPtr, CSC_FW_REG_BOffset_2);
        clampMin = cscFw_RegR(CscPtr, CSC_FW_REG_ClampMin_2);
        clipMax  = cscFw_RegR(CscPtr, CSC_FW_REG_ClipMax_2);
        if (XV_CscIsDemoWindowEnabled(CscPtr)) {
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
        } else {
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
        }
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
void XV_CscDbgReportStatus(XV_Csc_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_csc *pCsc = &InstancePtr->Csc;
  u32 done, idle, ready, ctrl;
  u32 colstart, colend, rowstart, rowend;
  s16 coeff[3][3];
  u32 offset_r, offset_g, offset_b;
  u32 minclamp,maxclamp;
  u32 height, width, i, j;
  u16 allow422, allow420, allowWindow;
  u32 CfmtIn,CfmtOut;

  done  = XV_csc_IsDone(pCsc);
  idle  = XV_csc_IsIdle(pCsc);
  ready = XV_csc_IsReady(pCsc);
  ctrl  = XV_csc_ReadReg(pCsc->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL);

  height   = XV_csc_Get_HwReg_height(pCsc);
  width    = XV_csc_Get_HwReg_width(pCsc);
  allow422 = XV_CscIs422Enabled(InstancePtr);
  allow420 = XV_CscIs420Enabled(InstancePtr);
  allowWindow = XV_CscIsDemoWindowEnabled(InstancePtr);
  CfmtIn   = XV_csc_Get_HwReg_InVideoFormat(pCsc);
  CfmtOut  = XV_csc_Get_HwReg_OutVideoFormat(pCsc);

  colstart = allowWindow? XV_csc_Get_HwReg_ColStart(pCsc) : 0;
  colend   = allowWindow? XV_csc_Get_HwReg_ColEnd(pCsc)   : width-1;
  rowstart = allowWindow? XV_csc_Get_HwReg_RowStart(pCsc) : 0;
  rowend   = allowWindow? XV_csc_Get_HwReg_RowEnd(pCsc)   : height-1;

  xil_printf("\r\n\r\n----->CSC IP STATUS<----\r\n");
  xil_printf("IsDone:           %d\r\n", done);
  xil_printf("IsIdle:           %d\r\n", idle);
  xil_printf("IsReady:          %d\r\n", ready);
  xil_printf("Ctrl:             0x%x\r\n\r\n", ctrl);

  xil_printf("4:2:2 processing: %s\r\n", allow422?"Enabled":"Disabled");
  xil_printf("4:2:0 processing: %s\r\n", allow420?"Enabled":"Disabled");
  xil_printf("Color Format In:  %s\r\n",
		  XVidC_GetColorFormatStr((XVidC_ColorFormat)CfmtIn));
  xil_printf("Color Format Out: %s\r\n",
		  XVidC_GetColorFormatStr((XVidC_ColorFormat)CfmtOut));
  xil_printf("Active Width:     %d\r\n",width);
  xil_printf("Active Height:    %d\r\n\r\n",height);

  xil_printf("Demo Window:      %s\r\n", allowWindow?"Enabled":"Disabled");
  xil_printf("Column Start:     %d\r\n",colstart);
  xil_printf("Column End:       %d\r\n",colend);
  xil_printf("Row Start:        %d\r\n",rowstart);
  xil_printf("Row End:          %d\r\n",rowend);

  xil_printf("\r\nGlobal Window:\r\n");
  offset_r = XV_csc_Get_HwReg_ROffset_V(pCsc);
  offset_g = XV_csc_Get_HwReg_GOffset_V(pCsc);
  offset_b = XV_csc_Get_HwReg_BOffset_V(pCsc);
  minclamp = XV_csc_Get_HwReg_ClampMin_V(pCsc);
  maxclamp = XV_csc_Get_HwReg_ClipMax_V(pCsc);
  coeff[0][0] = (s16)XV_csc_Get_HwReg_K11(pCsc);
  coeff[0][1] = (s16)XV_csc_Get_HwReg_K12(pCsc);
  coeff[0][2] = (s16)XV_csc_Get_HwReg_K13(pCsc);
  coeff[1][0] = (s16)XV_csc_Get_HwReg_K21(pCsc);
  coeff[1][1] = (s16)XV_csc_Get_HwReg_K22(pCsc);
  coeff[1][2] = (s16)XV_csc_Get_HwReg_K23(pCsc);
  coeff[2][0] = (s16)XV_csc_Get_HwReg_K31(pCsc);
  coeff[2][1] = (s16)XV_csc_Get_HwReg_K32(pCsc);
  coeff[2][2] = (s16)XV_csc_Get_HwReg_K33(pCsc);

  xil_printf("R Offset:         %d\r\n",offset_r);
  xil_printf("G Offset:         %d\r\n",offset_g);
  xil_printf("B Offset:         %d\r\n",offset_b);
  xil_printf("Min Clamp:        %d\r\n",minclamp);
  xil_printf("Max Clamp:        %d\r\n",maxclamp);

  xil_printf("\r\nCoefficients:");
  for(i=0; i<3; ++i) {
    xil_printf("\r\n r%d: ",i);
    for(j=0; j<3; ++j) {
      xil_printf("%5d ",coeff[i][j]);
    }
  }

  if (allowWindow) {
    xil_printf("\r\nDemo Window:\r\n");
    offset_r = XV_csc_Get_HwReg_ROffset_2_V(pCsc);
    offset_g = XV_csc_Get_HwReg_GOffset_2_V(pCsc);
    offset_b = XV_csc_Get_HwReg_BOffset_2_V(pCsc);
    minclamp = XV_csc_Get_HwReg_ClampMin_2_V(pCsc);
    maxclamp = XV_csc_Get_HwReg_ClipMax_2_V(pCsc);
    coeff[0][0] = (s16)XV_csc_Get_HwReg_K11_2(pCsc);
    coeff[0][1] = (s16)XV_csc_Get_HwReg_K12_2(pCsc);
    coeff[0][2] = (s16)XV_csc_Get_HwReg_K13_2(pCsc);
    coeff[1][0] = (s16)XV_csc_Get_HwReg_K21_2(pCsc);
    coeff[1][1] = (s16)XV_csc_Get_HwReg_K22_2(pCsc);
    coeff[1][2] = (s16)XV_csc_Get_HwReg_K23_2(pCsc);
    coeff[2][0] = (s16)XV_csc_Get_HwReg_K31_2(pCsc);
    coeff[2][1] = (s16)XV_csc_Get_HwReg_K32_2(pCsc);
    coeff[2][2] = (s16)XV_csc_Get_HwReg_K33_2(pCsc);
    xil_printf("R Offset:          %d\r\n",offset_r);
    xil_printf("G Offset:          %d\r\n",offset_g);
    xil_printf("B Offset:          %d\r\n",offset_b);
    xil_printf("Min Clamp:         %d\r\n",minclamp);
    xil_printf("Max Clamp:         %d\r\n",maxclamp);

    xil_printf("\r\nCoefficients:");
    for(i=0; i<3; ++i) {
      xil_printf("\r\n r%d: ",i);
      for(j=0; j<3; ++j) {
        xil_printf("%5d ",coeff[i][j]);
      }
    }
  } else {
    xil_printf("\r\nDemo Window is the Global Window.\r\n");
  }
}
/** @} */
