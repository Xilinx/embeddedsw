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
* @file xv_mix_l2.c
*
* Mix Layer-2 Driver. The functions in this file provides an abstraction
* from the register peek/poke methodology by implementing most common use-case
* provided by the sub-core. See xv_mix_l2.h for a detailed description of the
* layer-2 driver
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco   10/29/15   Initial Release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "xv_mix_l2.h"
#include "xdebug.h"

/************************** Constant Definitions *****************************/
#define XVMIX_MASK_ENABLE_ALL_LAYERS    (0x01FF)
#define XVMIX_MASK_DISABLE_ALL_LAYERS   (0)
#define XVMIX_REG_OFFSET                (8)

/* Pixel definition in 8 bit resolution in YUV color space*/
static const u8 bkgndColorYUV[XVMIX_BKGND_LAST][3] =
{
  {  0, 128, 128}, //Black
  {255, 128, 128}, //White
  { 76,  85, 255}, //Red
  {149,  43,  21}, //Green
  { 29, 255, 107}  //Blue
};

/* Pixel map in RGB color space*/
static const u8 bkgndColorRGB[XVMIX_BKGND_LAST][3] =
{
  {0, 0, 0}, //Black
  {1, 1, 1}, //White
  {1, 0, 0}, //Red
  {0, 1, 0}, //Green
  {0, 0, 1}  //Blue
};

/************************** Function Prototypes ******************************/
static void SetPowerOnDefaultState(XV_Mix_l2 *InstancePtr);

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
int XVMix_Initialize(XV_Mix_l2 *InstancePtr, u16 DeviceId)
{
  int Status;
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Setup the instance */
  memset(InstancePtr, 0, sizeof(XV_Mix_l2));
  Status = XV_mix_Initialize(&InstancePtr->Mix, DeviceId);

  if(Status == XST_SUCCESS) {
	SetPowerOnDefaultState(InstancePtr);
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function initializes the mixer core instance to default state
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return None
*
******************************************************************************/
static void SetPowerOnDefaultState(XV_Mix_l2 *InstancePtr)
{
  u32 index;
  XVidC_VideoStream VidStrm;
  XVidC_VideoTiming const *ResTiming;
  XVidC_VideoWindow Win;

  /* Setup Default Output Stream configuration */
  /* Disable All layers */
  XVMix_LayerDisable(InstancePtr, XVMIX_LAYER_ALL);

  /* Set Default Stream In and Out */
  VidStrm.VmId          = XVIDC_VM_1920x1080_60_P;
  VidStrm.ColorFormatId = XVIDC_CSF_RGB;
  VidStrm.FrameRate     = XVIDC_FR_60HZ;
  VidStrm.IsInterlaced  = FALSE;
  VidStrm.ColorDepth    = InstancePtr->Mix.Config.MaxDataWidth;
  VidStrm.PixPerClk     = InstancePtr->Mix.Config.PixPerClk;

  ResTiming = XVidC_GetTimingInfo(VidStrm.VmId);

  VidStrm.Timing = *ResTiming;

  XVMix_SetVidStreamIn(InstancePtr,  &VidStrm);
  XVMix_SetVidStreamOut(InstancePtr, &VidStrm);

  /* Set Stream properties */
  XVMix_SetLayerColorFormat(InstancePtr, XVMIX_LAYER_STREAM, XVIDC_CSF_RGB);
  XVMix_SetResolution(InstancePtr,
                      VidStrm.Timing.HActive,
					  VidStrm.Timing.VActive);

  /* Set Layer Color Format and window coordinates*/
  Win.StartX = 200;
  Win.StartY = 200;
  Win.Width  = 256;
  Win.Height = 256;

  for(index=XVMIX_LAYER_1; index<InstancePtr->Mix.Config.NumLayers; ++index) {
	  Win.StartX += index*50; //offset each layer by 50 pixels in horiz. dir
	  Win.StartY += index*50; //offset each layer by 50 pixels in vert. dir
	  XVMix_SetLayerColorFormat(InstancePtr, index, XVIDC_CSF_RGB);
      XVMix_SetLayerWindow(InstancePtr, index, &Win);
      XVMix_SetLayerAlpha(InstancePtr, index, XVMIX_ALPHA_MAX);
      XVMix_SetLayerScaleFactor(InstancePtr, index, XVMIX_SCALE_FACTOR_1X);
  }

  /* Set Logo Window */
  if(InstancePtr->Mix.Config.LogoEn) {
	  Win.StartX = VidStrm.Timing.HActive - 100;
	  Win.StartY = 64;
	  Win.Width  = 64;
	  Win.Height = 64;
	  XVMix_SetLayerWindow(InstancePtr, XVMIX_LAYER_LOGO, &Win);
  }

  /* Set Background Color */
  XVMix_SetBackgndColor(InstancePtr,
		                XVMIX_BKGND_BLUE,
		                InstancePtr->Mix.Config.MaxDataWidth);

  /* Enable only Stream Layer */
  XVMix_LayerEnable(InstancePtr, XVMIX_LAYER_STREAM);
}


/*****************************************************************************/
/**
* This function starts the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
******************************************************************************/
void XVMix_Start(XV_Mix_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_mix_EnableAutoRestart(&InstancePtr->Mix);
  XV_mix_Start(&InstancePtr->Mix);
}

/*****************************************************************************/
/**
* This function stops the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
******************************************************************************/
void XVMix_Stop(XV_Mix_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_mix_DisableAutoRestart(&InstancePtr->Mix);
}

/*****************************************************************************/
/**
* This function configures the mixer input stream
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  StrmIn is the pointer to input stream configuration
*
* @return none
*
******************************************************************************/
void XVMix_SetVidStreamIn(XV_Mix_l2 *InstancePtr,
                          const XVidC_VideoStream *StrmIn)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(StrmIn != NULL);

  InstancePtr->StrmIn = *StrmIn;
}

/*****************************************************************************/
/**
* This function configures the mixer output stream
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  StrmOut is the pointer to input stream configuration
*
* @return none
*
******************************************************************************/
void XVMix_SetVidStreamOut(XV_Mix_l2 *InstancePtr,
                           const XVidC_VideoStream *StrmOut)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(StrmOut != NULL);

  InstancePtr->StrmOut = *StrmOut;
}

/*****************************************************************************/
/**
* This function enables the specified layer of the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is layer number to be enabled
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   To enable all layers use layer id  XVMIX_LAYER_ALL
*
******************************************************************************/
int XVMix_LayerEnable(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId)
{
  XV_mix *MixPtr;
  u16 CurrenState;
  u32 NumLayers, index;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId >= XVMIX_LAYER_STREAM) &&
		            (LayerId < XVMIX_LAYER_LAST));

  MixPtr = &InstancePtr->Mix;
  NumLayers = MixPtr->Config.NumLayers;

  //Check if request is to enable all layers or single layer
  if(LayerId == XVMIX_LAYER_ALL) {
    XV_mix_Set_HwReg_layerEnable(MixPtr, XVMIX_MASK_ENABLE_ALL_LAYERS);

    for(index=XVMIX_LAYER_STREAM; index<NumLayers; ++index) {
        InstancePtr->Layer[index].IsEnabled = TRUE;
    }

    if(MixPtr->Config.LogoEn) {
        InstancePtr->Layer[XVMIX_LAYER_LOGO].IsEnabled = TRUE;
    }
    Status = XST_SUCCESS;
  }
  else if((LayerId < NumLayers) ||
		  ((LayerId == XVMIX_LAYER_LOGO) &&
		   (MixPtr->Config.LogoEn))) {

    CurrenState = XV_mix_Get_HwReg_layerEnable(MixPtr);
    CurrenState |= (1<<LayerId);
    XV_mix_Set_HwReg_layerEnable(MixPtr, CurrenState);
    InstancePtr->Layer[LayerId].IsEnabled = TRUE;
    Status = XST_SUCCESS;
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function disables the specified layer of the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is layer number to be disabled
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   To disable all layers use layer id  XVMIX_LAYER_ALL
*
******************************************************************************/
int XVMix_LayerDisable(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId)
{
  XV_mix *MixPtr;
  u16 CurrenState;
  u32 NumLayers, index;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId >= XVMIX_LAYER_STREAM) &&
		            (LayerId < XVMIX_LAYER_LAST));

  MixPtr = &InstancePtr->Mix;
  NumLayers = MixPtr->Config.NumLayers;

  //Check if request is to disable all layers or single layer
  if(LayerId == XVMIX_LAYER_ALL) {
    XV_mix_Set_HwReg_layerEnable(MixPtr, XVMIX_MASK_DISABLE_ALL_LAYERS);

    for(index=XVMIX_LAYER_STREAM; index<NumLayers; ++index) {
        InstancePtr->Layer[index].IsEnabled = FALSE;
    }

    if(MixPtr->Config.LogoEn) {
        InstancePtr->Layer[XVMIX_LAYER_LOGO].IsEnabled = FALSE;
    }
    Status = XST_SUCCESS;
  }
  else if((LayerId < NumLayers) ||
		  ((LayerId == XVMIX_LAYER_LOGO) &&
		   (MixPtr->Config.LogoEn))) {
    CurrenState = XV_mix_Get_HwReg_layerEnable(MixPtr);
    CurrenState &= ~(1<<LayerId);
    XV_mix_Set_HwReg_layerEnable(MixPtr, CurrenState);
    InstancePtr->Layer[LayerId].IsEnabled = FALSE;
    Status = XST_SUCCESS;
  }
  return(Status);
}


/*****************************************************************************/
/**
* This function sets the output video resolution of the mixer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Width is the active width of output stream
* @param  Height is the active height of output stream
*
* @return none
*
* @note   none
*
******************************************************************************/
void XVMix_SetResolution(XV_Mix_l2 *InstancePtr, u32 Width, u32 Height)
{
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((Width > 0) &&
		         (Width <= InstancePtr->Mix.Config.MaxWidth));
  Xil_AssertVoid((Height > 0) &&
		         (Height <= InstancePtr->Mix.Config.MaxHeight));

  XV_mix_Set_HwReg_width(&InstancePtr->Mix,  Width);
  XV_mix_Set_HwReg_height(&InstancePtr->Mix, Height);
}

/*****************************************************************************/
/**
* This function sets the background color to be displayed when stream layer is
* disabled
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  ColorId is requested background color
* @param  bpc is color depth
*
* @return none
*
* @note   none
*
******************************************************************************/
void XVMix_SetBackgndColor(XV_Mix_l2 *InstancePtr,
		                   XVMix_BackgroundId ColorId,
                           XVidC_ColorDepth  bpc)
{
  u16 v_b_val,y_r_val,u_g_val;
  u16 scale;
  XVidC_ColorFormat cfmt;

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((ColorId >= XVMIX_BKGND_BLACK) &&
		         (ColorId < XVMIX_BKGND_LAST));
  Xil_AssertVoid((bpc >= XVIDC_BPC_8) &&
                 (bpc <= InstancePtr->Mix.Config.MaxDataWidth))

  cfmt = XV_mix_Get_HwReg_video_format(&InstancePtr->Mix);

  if(cfmt == XVIDC_CSF_RGB) {
    scale = ((1<<bpc)-1);
    y_r_val = bkgndColorRGB[ColorId][0] * scale;
    u_g_val = bkgndColorRGB[ColorId][1] * scale;
    v_b_val = bkgndColorRGB[ColorId][2] * scale;
  }
  else {//YUV
    scale =  (1<<(bpc-XVIDC_BPC_8));
    y_r_val = bkgndColorYUV[ColorId][0] * scale;
    u_g_val = bkgndColorYUV[ColorId][1] * scale;
    v_b_val = bkgndColorYUV[ColorId][2] * scale;
  }

  /* Set Background Color */
  XV_mix_Set_HwReg_background_Y_R(&InstancePtr->Mix, y_r_val);
  XV_mix_Set_HwReg_background_U_G(&InstancePtr->Mix, u_g_val);
  XV_mix_Set_HwReg_background_V_B(&InstancePtr->Mix, v_b_val);
}

/*****************************************************************************/
/**
* This function configures the window coordinates of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer for which window coordinates are to be set
* @param  Win is the updated window coordinates
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   none
*
******************************************************************************/
int XVMix_SetLayerWindow(XV_Mix_l2 *InstancePtr,
		                 XVMix_LayerId LayerId,
		                 XVidC_VideoWindow *Win)
{
  XV_mix *MixPtr;
  u32 Status = XST_FAILURE;

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((LayerId >= XVMIX_LAYER_STREAM) &&
		         (LayerId <= XVMIX_LAYER_LOGO));
  Xil_AssertVoid((Win->Width !=0 ) && (Win->Height!=0));

  MixPtr = &InstancePtr->Mix;

  //TODO: Add error check on window coordinates

  switch(LayerId) {
    case XVMIX_LAYER_LOGO:
	if(MixPtr->Config.LogoEn) {
		if(Win->Width <= MixPtr->Config.MaxLogoWidth) {
			XV_mix_Set_HwReg_logoStartX(MixPtr, Win->StartX);
			XV_mix_Set_HwReg_logoStartY(MixPtr, Win->StartY);
			XV_mix_Set_HwReg_logoWidth(MixPtr,  Win->Width);
			XV_mix_Set_HwReg_logoHeight(MixPtr, Win->Height);

			InstancePtr->Layer[LayerId].Win = *Win;
			Status = XST_SUCCESS;
		}
	}
	break;

    default: //Layer1-Layer7
	if(LayerId < MixPtr->Config.NumLayers) {

		if (((LayerId == XVMIX_LAYER_STREAM)         &&
			 (Win->Width <= MixPtr->Config.MaxWidth) &&
			 (Win->Height <= MixPtr->Config.MaxHeight)) ||
			((LayerId > XVMIX_LAYER_STREAM) &&
			 (Win->Width < MixPtr->Config.LayerMaxWidth[LayerId-1]))) {

			u32 BaseStartXReg, BaseStartYReg;
			u32 BaseWidthReg, BaseHeightReg;

			BaseStartXReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_0_DATA;
			BaseStartYReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_0_DATA;
			BaseWidthReg  = XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_0_DATA;
			BaseHeightReg = XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_0_DATA;

			XV_mix_WriteReg(MixPtr->Config.BaseAddress,
				(BaseStartXReg+LayerId*XVMIX_REG_OFFSET), Win->StartX);
			XV_mix_WriteReg(MixPtr->Config.BaseAddress,
				(BaseStartYReg+LayerId*XVMIX_REG_OFFSET), Win->StartY);
			XV_mix_WriteReg(MixPtr->Config.BaseAddress,
				(BaseWidthReg+LayerId*XVMIX_REG_OFFSET),  Win->Width);
			XV_mix_WriteReg(MixPtr->Config.BaseAddress,
				(BaseHeightReg+LayerId*XVMIX_REG_OFFSET), Win->Height);

			InstancePtr->Layer[LayerId].Win = *Win;
			Status = XST_SUCCESS;
		}
	}
	break;
  }

  return(Status);
}

/*****************************************************************************/
/**
* This function read the window coordinates of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer for which window coordinates are to be set
* @param  Win is the pointer to return window coordinates
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   none
*
******************************************************************************/
int XVMix_GetLayerWindow(XV_Mix_l2 *InstancePtr,
		                 XVMix_LayerId LayerId,
		                 XVidC_VideoWindow *Win)
{
  XV_mix *MixPtr;
  u32 Status = XST_FAILURE;

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((LayerId >= XVMIX_LAYER_STREAM) &&
		         (LayerId <= XVMIX_LAYER_LOGO));
  Xil_AssertVoid(Win != NULL);

  MixPtr = &InstancePtr->Mix;

  switch(LayerId) {
    case XVMIX_LAYER_LOGO:
	if((MixPtr->Config.LogoEn) &&
			(XVMix_IsLayerEnabled(InstancePtr, LayerId))) {

		Win->StartX = XV_mix_Get_HwReg_logoStartX(MixPtr);
		Win->StartY = XV_mix_Get_HwReg_logoStartY(MixPtr);
		Win->Width	= XV_mix_Get_HwReg_logoWidth(MixPtr);
		Win->Height = XV_mix_Get_HwReg_logoHeight(MixPtr);

			Status = XST_SUCCESS;
	}
	break;

    default: //Layer0-Layer7
	if((LayerId < MixPtr->Config.NumLayers) &&
			(XVMix_IsLayerEnabled(InstancePtr, LayerId))) {

		u32 BaseStartXReg, BaseStartYReg;
		u32 BaseWidthReg, BaseHeightReg;

		BaseStartXReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_0_DATA;
		BaseStartYReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_0_DATA;
		BaseWidthReg  = XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_0_DATA;
		BaseHeightReg = XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_0_DATA;

		Win->StartX = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
						 (BaseStartXReg+LayerId*XVMIX_REG_OFFSET));
		Win->StartY = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
						 (BaseStartYReg+LayerId*XVMIX_REG_OFFSET));
		Win->Width  = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
						(BaseWidthReg+LayerId*XVMIX_REG_OFFSET));
		Win->Height = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
						(BaseHeightReg+LayerId*XVMIX_REG_OFFSET));

		Status = XST_SUCCESS;
	}
	break;
  }

  return(Status);
}

/*****************************************************************************/
/**
* This function configures the window position of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer for which window position is to be set
* @param  StartX is the new X position
* @param  StartY is the new Y position
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   none
*
******************************************************************************/
int XVMix_SetLayerPosition(XV_Mix_l2 *InstancePtr,
		                   XVMix_LayerId LayerId,
		                   u16 StartX,
		                   u16 StartY)
{
  XV_mix *MixPtr;
  u32 Status = XST_FAILURE;

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((LayerId >= XVMIX_LAYER_STREAM) &&
		         (LayerId <= XVMIX_LAYER_LOGO));

  MixPtr = &InstancePtr->Mix;

  //TODO: Add error check on window start coordinates

  switch(LayerId) {
    case XVMIX_LAYER_LOGO:
	if(MixPtr->Config.LogoEn) {

		XV_mix_Set_HwReg_logoStartX(MixPtr, StartX);
		XV_mix_Set_HwReg_logoStartY(MixPtr, StartY);

		InstancePtr->Layer[LayerId].Win.StartX = StartX;
		InstancePtr->Layer[LayerId].Win.StartY = StartY;
		Status = XST_SUCCESS;
	}
	break;

    default: //Layer0-Layer7
	if(LayerId < MixPtr->Config.NumLayers) {
		u32 BaseStartXReg, BaseStartYReg;

		BaseStartXReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_0_DATA;
		BaseStartYReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_0_DATA;

		XV_mix_WriteReg(MixPtr->Config.BaseAddress,
			(BaseStartXReg+LayerId*XVMIX_REG_OFFSET), StartX);
		XV_mix_WriteReg(MixPtr->Config.BaseAddress,
			(BaseStartYReg+LayerId*XVMIX_REG_OFFSET), StartY);

		InstancePtr->Layer[LayerId].Win.StartX = StartX;
		InstancePtr->Layer[LayerId].Win.StartY = StartY;
		Status = XST_SUCCESS;
	}
	break;
  }
}

/*****************************************************************************/
/**
* This function configures the scaling factor of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer to be updated
* @param  Scale is the scale factor
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   none
*
******************************************************************************/
int XVMix_SetLayerScaleFactor(XV_Mix_l2 *InstancePtr,
		                      XVMix_LayerId LayerId,
		                      XVMix_Scalefactor Scale)
{
  XV_mix *MixPtr;
  u32 Status = XST_FAILURE;

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((LayerId >= XVMIX_LAYER_STREAM) &&
	             (LayerId <= XVMIX_LAYER_LOGO));
  Xil_AssertVoid((Scale >= XVMIX_SCALE_FACTOR_1X) &&
	             (Scale <= XVMIX_SCALE_FACTOR_4X));

  MixPtr = &InstancePtr->Mix;

  switch(LayerId) {
    case XVMIX_LAYER_LOGO:
	if(MixPtr->Config.LogoEn) {
		XV_mix_Set_HwReg_logoScaleFactor(MixPtr, Scale);
		Status = XST_SUCCESS;
	}
	break;

    default: //Layer0-Layer7
	if((LayerId < MixPtr->Config.NumLayers) &&
	   (XVMix_IsScalingEnabled(InstancePtr, LayerId))) {

		u32 BaseReg;

		BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_0_DATA;
		XV_mix_WriteReg(MixPtr->Config.BaseAddress,
			(BaseReg+LayerId*XVMIX_REG_OFFSET), Scale);

		Status = XST_SUCCESS;
	}
        break;
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function returns the scaling factor of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer for which information is requested
*
* @return If layer enabled, current set value, else 0
*
* @note   none
*
******************************************************************************/
int XVMix_GetLayerScaleFactor(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId)
{
  XV_mix *MixPtr;
  u32 Status = XST_FAILURE;
  u32 ReadVal = 0;

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((LayerId >= XVMIX_LAYER_STREAM) &&
	             (LayerId <= XVMIX_LAYER_LOGO));

  MixPtr = &InstancePtr->Mix;

  switch(LayerId) {
    case XVMIX_LAYER_LOGO:
	if((MixPtr->Config.LogoEn) &&
	   (XVMix_IsLayerEnabled(InstancePtr, LayerId)) &&
	   (XVMix_IsScalingEnabled(InstancePtr, LayerId))) {
		ReadVal = XV_mix_Get_HwReg_logoScaleFactor(MixPtr);
	}
	break;

    default: //Layer0-Layer7
	if((LayerId < MixPtr->Config.NumLayers) &&
	   (XVMix_IsLayerEnabled(InstancePtr, LayerId)) &&
	   (XVMix_IsScalingEnabled(InstancePtr, LayerId))) {
		u32 BaseReg;

		BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_0_DATA;
		ReadVal = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
					(BaseReg+LayerId*XVMIX_REG_OFFSET));
	}
	break;
  }
  return(ReadVal);
}

/*****************************************************************************/
/**
* This function configures the Alpha level of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer to be updated
* @param  Alpha is the new value
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   none
*
******************************************************************************/
int XVMix_SetLayerAlpha(XV_Mix_l2 *InstancePtr,
		                XVMix_LayerId LayerId,
		                u8 Alpha)
{
  XV_mix *MixPtr;
  u32 Status = XST_FAILURE;

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((LayerId >= XVMIX_LAYER_STREAM) &&
	             (LayerId <= XVMIX_LAYER_LOGO));
  Xil_AssertVoid((Alpha >= XVMIX_ALPHA_MIN) &&
		         (Alpha <= XVMIX_ALPHA_MAX));

  MixPtr = &InstancePtr->Mix;

  switch(LayerId) {
    case XVMIX_LAYER_LOGO:
	if(MixPtr->Config.LogoEn) {
		XV_mix_Set_HwReg_logoAlpha(MixPtr, Alpha);
		Status = XST_SUCCESS;
	}
	break;

    default: //Layer0-Layer7
	if((LayerId < MixPtr->Config.NumLayers) &&
	   (XVMix_IsAlphaEnabled(InstancePtr, LayerId))) {
		u32 BaseReg;

		BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_0_DATA;
		XV_mix_WriteReg(MixPtr->Config.BaseAddress,
			(BaseReg+LayerId*XVMIX_REG_OFFSET), Alpha);
		Status = XST_SUCCESS;
	}
        break;
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function returns the alpha of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer for which information is requested
*
* @return If layer enabled, current set value, else 0
*
* @note   none
*
******************************************************************************/
int XVMix_GetLayerAlpha(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId)
{
  XV_mix *MixPtr;
  u32 Status = XST_FAILURE;
  u32 ReadVal = 0;

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((LayerId >= XVMIX_LAYER_STREAM) &&
	             (LayerId <= XVMIX_LAYER_LOGO));

  MixPtr = &InstancePtr->Mix;

  switch(LayerId) {
    case XVMIX_LAYER_LOGO:
	if((MixPtr->Config.LogoEn) &&
	   (XVMix_IsLayerEnabled(InstancePtr, LayerId)) &&
	   (XVMix_IsAlphaEnabled(InstancePtr, LayerId))) {
		ReadVal = XV_mix_Get_HwReg_logoAlpha(MixPtr);
	}
	break;

    default: //Layer0-Layer7
	if((LayerId < MixPtr->Config.NumLayers) &&
	   (XVMix_IsLayerEnabled(InstancePtr, LayerId)) &&
	   (XVMix_IsAlphaEnabled(InstancePtr, LayerId))) {
		u32 BaseReg;

		BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_0_DATA;
		ReadVal = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
					(BaseReg+LayerId*XVMIX_REG_OFFSET));
	}
	break;
  }
  return(ReadVal);
}

/*****************************************************************************/
/**
* This function configures the color format of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer to be updated
* @param  cfmt is the color format to be set for the specified layer
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   none
*
******************************************************************************/
int XVMix_SetLayerColorFormat(XV_Mix_l2 *InstancePtr,
		                      XVMix_LayerId LayerId,
		                      XVidC_ColorFormat Cfmt)
{
  XV_mix *MixPtr;
  u32 Status = XST_FAILURE;

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((LayerId >= XVMIX_LAYER_STREAM) &&
	             (LayerId < XVMIX_LAYER_LOGO));
  Xil_AssertVoid((Cfmt >= XVIDC_CSF_RGB) &&
		         (Cfmt < XVIDC_CSF_YCRCB_420));

  MixPtr = &InstancePtr->Mix;

  if(LayerId < MixPtr->Config.NumLayers) {

	  if (LayerId == XVMIX_LAYER_STREAM) {
		  XV_mix_Set_HwReg_video_format(MixPtr, Cfmt);
	  } else { //Layer 1-7
		  u32 BaseReg;

		  BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_1_DATA;
		  XV_mix_WriteReg(MixPtr->Config.BaseAddress,
				  (BaseReg+((LayerId-1)*XVMIX_REG_OFFSET)), Cfmt);
	  }
	  InstancePtr->Layer[LayerId].ColorFormat = Cfmt;
	  Status = XST_SUCCESS;
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function reads the color format of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer to be updated
* @param  cfmt is pointer to color format return variable
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   none
*
******************************************************************************/
int XVMix_GetLayerColorFormat(XV_Mix_l2 *InstancePtr,
		                      XVMix_LayerId LayerId,
		                      XVidC_ColorFormat *Cfmt)
{
  XV_mix *MixPtr;
  u32 Status = XST_FAILURE;

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((LayerId >= XVMIX_LAYER_STREAM) &&
	             (LayerId < XVMIX_LAYER_LOGO));
  Xil_AssertVoid(Cfmt != NULL);

  MixPtr = &InstancePtr->Mix;

  if((LayerId < MixPtr->Config.NumLayers) &&
	 (XVMix_IsLayerEnabled(InstancePtr, LayerId))) {

	  if (LayerId == XVMIX_LAYER_STREAM) {
		  *Cfmt = XV_mix_Get_HwReg_video_format(MixPtr);
	  } else { //Layer 1-7
		  u32 BaseReg;

		  BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_1_DATA;
		  *Cfmt = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
						  (BaseReg+((LayerId-1)*XVMIX_REG_OFFSET)));
	  }
	  Status = XST_SUCCESS;
  }

  return(Status);
}

/*****************************************************************************/
/**
* This function sets the buffer address of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer to be updated
* @param  Addr is the absolute addrees of buffer in memory
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   none
*
******************************************************************************/
int XVMix_SetLayerBufferAddr(XV_Mix_l2 *InstancePtr,
                             XVMix_LayerId LayerId,
		                     u32 Addr)
{
  XV_mix *MixPtr;
  u32 BaseReg;
  u32 Status = XST_FAILURE;

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((LayerId > XVMIX_LAYER_STREAM) &&
	             (LayerId < XVMIX_LAYER_LOGO));
  Xil_AssertVoid(Addr != 0);

  MixPtr = &InstancePtr->Mix;

  if(LayerId < MixPtr->Config.NumLayers) {
		BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYER1_V_DATA;

		XV_mix_WriteReg(MixPtr->Config.BaseAddress,
			(BaseReg+((LayerId-1)*XVMIX_REG_OFFSET)), Addr);

		InstancePtr->Layer[LayerId].BufAddr = Addr;
		Status = XST_SUCCESS;
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function reads the buffer address of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer to be updated
* @param
*
* @return Address of buffer in memory
*
* @note   none
*
******************************************************************************/
u32 XVMix_GetLayerBufferAddr(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId)
{
  XV_mix *MixPtr;
  u32 BaseReg;
  u32 ReadVal = 0;

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((LayerId > XVMIX_LAYER_STREAM) &&
	             (LayerId < XVMIX_LAYER_LOGO));

  MixPtr = &InstancePtr->Mix;

  if(LayerId < MixPtr->Config.NumLayers) {
		BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYER1_V_DATA;

		ReadVal = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
					(BaseReg+((LayerId-1)*XVMIX_REG_OFFSET)));
  }
  return(ReadVal);
}

/*****************************************************************************/
/**
* This function loads the logo data into core BRAM
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Win is logo window (logo width must be multiple of 4 bytes)
* @param  Height is logo height
* @param  RBuffer is the pointer to Red buffer
* @param  GBuffer is the pointer to Green buffer
* @param  BBuffer is the pointer to Blue buffer
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   none
*
******************************************************************************/
int XVMix_LoadLogo(XV_Mix_l2 *InstancePtr,
		           XVidC_VideoWindow *Win,
		           u8 *RBuffer,
		           u8 *GBuffer,
		           u8 *BBuffer)
{
  XV_mix *MixPtr;
  u32 BaseReg;
  int x,y;
  u32 Rword, Gword, Bword;
  u32 Width, Height;
  u32 RBaseAddr, GBaseAddr, BBaseAddr;
  u32 Status = XST_FAILURE;

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(RBuffer != NULL);
  Xil_AssertVoid(GBuffer != NULL);
  Xil_AssertVoid(BBuffer != NULL);
  Xil_AssertVoid(Win->Width%4 == 0); //must be multiple of 4 bytes


  MixPtr = &InstancePtr->Mix;
  Width  = Win->Width;
  Height = Win->Height;
  if(MixPtr->Config.LogoEn) {

	  RBaseAddr = XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE;
	  GBaseAddr = XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE;
	  BBaseAddr = XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE;

	  for (y=0; y<Height; y++) {
		  for (x=0; x<Width; x+=4) {
			Rword = (u32)RBuffer[y*Width+x] |
					(((u32)RBuffer[y*Width+x+1])<<8) |
					(((u32)RBuffer[y*Width+x+2])<<16) |
					(((u32)RBuffer[y*Width+x+3])<<24);

			Gword = (u32)GBuffer[y*Width+x] |
					(((u32)GBuffer[y*Width+x+1])<<8) |
					(((u32)GBuffer[y*Width+x+2])<<16) |
					(((u32)GBuffer[y*Width+x+3])<<24);

			Bword = (u32)BBuffer[y*Width+x] |
					(((u32)BBuffer[y*Width+x+1])<<8) |
					(((u32)BBuffer[y*Width+x+2])<<16) |
					(((u32)BBuffer[y*Width+x+3])<<24);

			XV_mix_WriteReg(MixPtr->Config.BaseAddress, (RBaseAddr+(y*Width+x)), Rword);
			XV_mix_WriteReg(MixPtr->Config.BaseAddress, (GBaseAddr+(y*Width+x)), Gword);
			XV_mix_WriteReg(MixPtr->Config.BaseAddress, (BBaseAddr+(y*Width+x)), Bword);
		  }
	  }
	  InstancePtr->Layer[XVMIX_LAYER_LOGO].RBuffer = RBuffer;
	  InstancePtr->Layer[XVMIX_LAYER_LOGO].GBuffer = GBuffer;
	  InstancePtr->Layer[XVMIX_LAYER_LOGO].BBuffer = BBuffer;

	  XVMix_SetLayerWindow(InstancePtr, XVMIX_LAYER_LOGO, Win);

	  Status = XST_SUCCESS;
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function reports the mixer status
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
* @note   none
*
******************************************************************************/
void XVMix_DbgReportStatus(XV_Mix_l2 *InstancePtr)
{
  XV_mix *MixPtr;
  u32 done, idle, ready, ctrl;
  u32 index, IsEnabled;
  const char *Status[2] = {"Disabled", "Enabled"};

  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n\r\n----->MIXER STATUS<----\r\n");
  MixPtr = &InstancePtr->Mix;

  done  = XV_mix_IsDone(MixPtr);
  idle  = XV_mix_IsIdle(MixPtr);
  ready = XV_mix_IsReady(MixPtr);
  ctrl  = XV_mix_ReadReg(MixPtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL);

  xil_printf("IsDone:  %d\r\n", done);
  xil_printf("IsIdle:  %d\r\n", idle);
  xil_printf("IsReady: %d\r\n", ready);
  xil_printf("Ctrl:    0x%x\r\n\r\n", ctrl);

  xil_printf("Number of Layers: %d\r\n",MixPtr->Config.NumLayers);
  xil_printf("Layers Enable Reg: 0x%x\r\n\r\n",XV_mix_Get_HwReg_layerEnable(MixPtr));

  IsEnabled = XVMix_IsLayerEnabled(InstancePtr, XVMIX_LAYER_STREAM);
  xil_printf("Layer Stream: %s\r\n", Status[IsEnabled]);
  for(index = XVMIX_LAYER_1; index<XVMIX_LAYER_LOGO; ++index) {
	  xil_printf("Layer %d     : %s\r\n" ,index,
			  Status[(XVMix_IsLayerEnabled(InstancePtr, index))]);
  }
  IsEnabled = XVMix_IsLayerEnabled(InstancePtr, XVMIX_LAYER_LOGO);
  xil_printf("Layer Logo  : %s\r\n\r\n", Status[IsEnabled]);

  xil_printf("Background Color Y/R: %d\r\n", XV_mix_Get_HwReg_background_Y_R(MixPtr));
  xil_printf("Background Color U/G: %d\r\n", XV_mix_Get_HwReg_background_U_G(MixPtr));
  xil_printf("Background Color V/B: %d\r\n\r\n", XV_mix_Get_HwReg_background_V_B(MixPtr));
}

/*****************************************************************************/
/**
* This function reports the mixer status of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer to be updated
*
* @return none
*
* @note   none
*
******************************************************************************/
void XVMix_DbgLayerInfo(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId)
{
  XV_mix *MixPtr;
  u32 index, IsEnabled;
  u32 ReadVal;
  XVidC_VideoWindow Win;
  XVidC_ColorFormat ColFormat;
  const char *Status[2] = {"Disabled", "Enabled"};

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((LayerId >= XVMIX_LAYER_STREAM) &&
	             (LayerId <= XVMIX_LAYER_LOGO));

  if (LayerId == XVMIX_LAYER_LOGO) {
	  xil_printf("\r\n\r\n----->Layer LOGO Status<----\r\n");
  } else {
	  xil_printf("\r\n\r\n----->Layer %d Status<----\r\n", LayerId);
  }

  MixPtr = &InstancePtr->Mix;
  IsEnabled = XVMix_IsLayerEnabled(InstancePtr, LayerId);
  xil_printf("Layer State: %s\r\n", Status[IsEnabled]);

  if(IsEnabled) {
	  if(LayerId == XVMIX_LAYER_STREAM) {
		  XVMix_GetLayerColorFormat(InstancePtr, LayerId, &ColFormat);
		  xil_printf("Layer ColorFormat: %s\r\n\r\n",
				  XVidC_GetColorFormatStr(ColFormat));
		  xil_printf("->Stream Input\r\n");
		  XVidC_ReportStreamInfo(&InstancePtr->StrmIn);
		  xil_printf("\r\n->Stream Output\r\n");
		  XVidC_ReportStreamInfo(&InstancePtr->StrmOut);
	  } else {
		  /* Report Memory layer Status */
		  IsEnabled = XVMix_IsAlphaEnabled(InstancePtr, LayerId);
		  if (IsEnabled) {
			  ReadVal = XVMix_GetLayerAlpha(InstancePtr, LayerId);
			  xil_printf("Layer Alpha:       %d\r\n", ReadVal);
		  } else {
			  xil_printf("Layer Alpha: %s\r\n", Status[IsEnabled]);
		  }

		  IsEnabled = XVMix_IsScalingEnabled(InstancePtr, LayerId);
		  if (IsEnabled) {
			  ReadVal = XVMix_GetLayerScaleFactor(InstancePtr, LayerId);
			  xil_printf("Layer Scalefactor: %d\r\n", ReadVal);
		  } else {
			  xil_printf("Layer Scalefactor: %s\r\n", Status[IsEnabled]);
		  }

		  if (LayerId <XVMIX_LAYER_LOGO) {
			  XVMix_GetLayerColorFormat(InstancePtr, LayerId, &ColFormat);
			  xil_printf("Layer ColorFormat: %s\r\n",
					  XVidC_GetColorFormatStr(ColFormat));
		  }
		  xil_printf("Layer Window: \r\n");
		  XVMix_GetLayerWindow(InstancePtr, LayerId, &Win);
		  xil_printf("   Start X    = %d\r\n", Win.StartX);
		  xil_printf("   Start Y    = %d\r\n", Win.StartY);
		  xil_printf("   Win Width  = %d\r\n", Win.Width);
		  xil_printf("   Win Height = %d\r\n", Win.Height);

		  if(LayerId != XVMIX_LAYER_LOGO) {
			  xil_printf("Layer Buffer Addr: 0x%x\r\n",
				  XVMix_GetLayerBufferAddr(InstancePtr, LayerId));
		  }
	  }
  }
}
