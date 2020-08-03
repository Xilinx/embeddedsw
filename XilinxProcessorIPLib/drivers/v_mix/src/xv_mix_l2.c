/******************************************************************************
* Copyright (C) 1986 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_mix_l2.c
* @addtogroup v_mix_v6_1
* @{
*
* Mixer Layer-2 Driver. The functions in this file provides an abstraction
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
*             02/10/16   Updated to support IP changes
*             02/25/16   Replace GetColorFromat function with a macro
*             03/08/16   Replace GetColorFromat macro with function and added
*                        master layer video format
*             03/09/16   Removed stream layer error check from SetWindow API
*             03/18/16   Window coordinates can start from 0,0
* 2.00  rco   07/21/16   Used UINTPTR instead of u32 for Baseaddress
*             08/03/16   Add Logo Pixel Alpha support
*             08/16/16   Add check for stream and logo layer minimum width and
*                        height
*2.10   rco   11/15/16   Add a check for logo layer enable before loading logo
*                        pixel alpha. IP confguration can have logo feature
*                        disabled but logo pixel alpha enabled
*             01/26/16   Bug fix - Read power on default video stream
*                        properties from IP configuration
*             02/09/17   Fix c++ compilation warnings
* 3.00  vyc   10/04/17   Add second buffer pointer for semi-planar formats
* 4.00  vyc   04/04/18   Add 8th overlayer
*                        Move logo layer enable fromb bit 8 to bit 15
* 5.00  pv    11/10/18   Added flushing feature support in driver.
*                        flush bit should be set and held (until reset) by
*                        software to flush pending transactions.IP is expecting
*                        a hard reset, when flushing is done.(There is a flush
*                        status bit and is asserted when the flush is done).
* 6.00  pg    01/10/20   Add Colorimetry feature.
*                        Program Mixer CSC registers to do color conversion
*                        from YUV to RGB and RGB to YUV.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include <sleep.h>
#include "xv_mix_l2.h"

/************************** Constant Definitions *****************************/
#define XVMIX_MASK_ENABLE_ALL_LAYERS    (0xFFFFFFFF)
#define XVMIX_MASK_DISABLE_ALL_LAYERS   (0)
#define XVMIX_REG_OFFSET                (0x100)
#define XVMIX_MIN_STRM_WIDTH            (64u)
#define XVMIX_MIN_STRM_HEIGHT           (64u)
#define XVMIX_MIN_LOGO_WIDTH            (32u)
#define XVMIX_MIN_LOGO_HEIGHT           (32u)
#define XV_WAIT_FOR_FLUSH_DONE		    (25)
#define XV_WAIT_FOR_FLUSH_DONE_TIMEOUT	(2000)

/* Pixel values in 8 bit resolution in YUV color space*/
static const u8 bkgndColorYUV[XVMIX_BKGND_LAST][3] =
{
  {  0, 128, 128}, //Black
  {255, 128, 128}, //White
  { 76,  85, 255}, //Red
  {149,  43,  21}, //Green
  { 29, 255, 107}  //Blue
};

/* Pixel values in RGB color space*/
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
static int IsWindowValid(XVidC_VideoStream *Strm,
                         XVidC_VideoWindow *Win,
                         XVMix_Scalefactor ScaleFactor);

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
  XV_mix *MixPtr;

  MixPtr = &InstancePtr->Mix;

  memset(&VidStrm, 0, sizeof(XVidC_VideoStream));

  /* Disable All layers */
  XVMix_LayerDisable(InstancePtr, XVMIX_LAYER_ALL);

  /* Set Default Stream In and Out */
  VidStrm.VmId           = XVIDC_VM_CUSTOM;
  VidStrm.ColorFormatId  = (XVidC_ColorFormat)MixPtr->Config.ColorFormat;
  VidStrm.FrameRate      = XVIDC_FR_60HZ;
  VidStrm.IsInterlaced   = FALSE;
  VidStrm.ColorDepth     = (XVidC_ColorDepth)MixPtr->Config.MaxDataWidth;
  VidStrm.PixPerClk      = (XVidC_PixelsPerClock)MixPtr->Config.PixPerClk;
  VidStrm.Timing.HActive = MixPtr->Config.MaxWidth;
  VidStrm.Timing.VActive = MixPtr->Config.MaxHeight;

  XVMix_SetVidStream(InstancePtr, &VidStrm);

  /* Set Stream properties */
//  XVMix_SetLayerColorFormat(InstancePtr, XVMIX_LAYER_MASTER, XVIDC_CSF_RGB);

  /* Set default background color */
  XVMix_SetBackgndColor(InstancePtr,
                        XVMIX_BKGND_BLUE,
                        (XVidC_ColorDepth)MixPtr->Config.MaxDataWidth);

  /* Check each layers scaling feature configuration
   *  - Disabled: Update Layer config MaxWidth with IP MaxWidth
   *  - Enabled:  NOPS (Layer Maxwidth is the linebuffer width specified in IP
   *                    config)
   */
  for(index=XVMIX_LAYER_1; index<XVMix_GetNumLayers(InstancePtr); ++index) {
      if(!XVMix_IsScalingEnabled(InstancePtr, index)) {
        MixPtr->Config.LayerMaxWidth[index-1] = MixPtr->Config.MaxWidth;
      }
    }

  /* Enable only Stream Layer */
  XVMix_LayerEnable(InstancePtr, XVMIX_LAYER_MASTER);

  /* Setup polling mode */
  XVMix_InterruptDisable(InstancePtr);
}

/*****************************************************************************/
/**
* This function enables interrupts in the core
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
******************************************************************************/
void XVMix_InterruptEnable(XV_Mix_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  /* Enable Interrupts */
  XV_mix_InterruptEnable(&InstancePtr->Mix, XVMIX_IRQ_DONE_MASK);
  XV_mix_InterruptGlobalEnable(&InstancePtr->Mix);

  /* Clear autostart bit */
  XV_mix_DisableAutoRestart(&InstancePtr->Mix);
}

/*****************************************************************************/
/**
* This function disables interrupts in the core
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
******************************************************************************/
void XVMix_InterruptDisable(XV_Mix_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  /* Disable Interrupts */
  XV_mix_InterruptDisable(&InstancePtr->Mix, XVMIX_IRQ_DONE_MASK);
  XV_mix_InterruptGlobalDisable(&InstancePtr->Mix);

  /* Set autostart bit */
  XV_mix_EnableAutoRestart(&InstancePtr->Mix);
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
  u32 Data = 0;
  u32 cnt = 0;

  Xil_AssertVoid(InstancePtr != NULL);

  /* Clear autostart bit */
  XV_mix_DisableAutoRestart(&InstancePtr->Mix);

  /* Flush the core bit */
  XV_mix_SetFlushbit(&InstancePtr->Mix);

  do {
    Data = XV_mix_Get_FlushDone(&InstancePtr->Mix);
    usleep(XV_WAIT_FOR_FLUSH_DONE_TIMEOUT);
    cnt++;
  } while ((Data == 0) && (cnt < XV_WAIT_FOR_FLUSH_DONE));

  if (Data == 0)
        return;
}

/*****************************************************************************/
/**
* This function validates if the requested window is within the frame boundary
*
* @param  Strm is a pointer to Mixer master video stream
* @param  Win is the pointer to window coordinates to be validated
* @param  Scale is the scale factor of the window
*
* @return None
*
******************************************************************************/
static int IsWindowValid(XVidC_VideoStream *Strm,
                         XVidC_VideoWindow *Win,
                         XVMix_Scalefactor Scale)
{
  XVidC_VideoWindow NewWin;
  u16 ScaleFactor[XVMIX_SCALE_FACTOR_NUM_SUPPORTED] = {1,2,4};

  NewWin = *Win;

  /* Check if window scale factor is set */
  if(Scale < XVMIX_SCALE_FACTOR_NUM_SUPPORTED) {
    /* update window per scale factor before validating */
      NewWin.Width  *= ScaleFactor[Scale];
      NewWin.Height *= ScaleFactor[Scale];
  }

  if(((NewWin.StartX + NewWin.Width)  <= Strm->Timing.HActive) &&
     ((NewWin.StartY + NewWin.Height) <= Strm->Timing.VActive)) {
      return(TRUE);
  } else {
      return(FALSE);
  }
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
void XVMix_SetVidStream(XV_Mix_l2 *InstancePtr,
                        const XVidC_VideoStream *StrmIn)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(StrmIn != NULL);
  Xil_AssertVoid((StrmIn->Timing.HActive > (XVMIX_MIN_STRM_WIDTH-1)) &&
                 (StrmIn->Timing.HActive <= InstancePtr->Mix.Config.MaxWidth));
  Xil_AssertVoid((StrmIn->Timing.VActive > (XVMIX_MIN_STRM_HEIGHT-1)) &&
                 (StrmIn->Timing.VActive <= InstancePtr->Mix.Config.MaxHeight));

  /* copy stream data */
  InstancePtr->Stream = *StrmIn;

  /* set resolution */
  XV_mix_Set_HwReg_width(&InstancePtr->Mix,  StrmIn->Timing.HActive);
  XV_mix_Set_HwReg_height(&InstancePtr->Mix, StrmIn->Timing.VActive);
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
  u32 NumLayers, CurrenState;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId >= XVMIX_LAYER_MASTER) &&
                    (LayerId < XVMIX_LAYER_LAST));

  MixPtr = &InstancePtr->Mix;
  NumLayers = XVMix_GetNumLayers(InstancePtr);

  //Check if request is to enable all layers or single layer
  if(LayerId == XVMIX_LAYER_ALL) {
    XV_mix_Set_HwReg_layerEnable(MixPtr, XVMIX_MASK_ENABLE_ALL_LAYERS);
    Status = XST_SUCCESS;
  }
  else if((LayerId < NumLayers) ||
          ((LayerId == XVMIX_LAYER_LOGO) &&
           (XVMix_IsLogoEnabled(InstancePtr)))) {

    CurrenState = XV_mix_Get_HwReg_layerEnable(MixPtr);
    CurrenState |= (1<<LayerId);
    XV_mix_Set_HwReg_layerEnable(MixPtr, CurrenState);
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
  u32 NumLayers, CurrenState;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId >= XVMIX_LAYER_MASTER) &&
                    (LayerId < XVMIX_LAYER_LAST));

  MixPtr = &InstancePtr->Mix;
  NumLayers = XVMix_GetNumLayers(InstancePtr);

  //Check if request is to disable all layers or single layer
  if(LayerId == XVMIX_LAYER_ALL) {
    XV_mix_Set_HwReg_layerEnable(MixPtr, XVMIX_MASK_DISABLE_ALL_LAYERS);
    Status = XST_SUCCESS;
  }
  else if((LayerId < NumLayers) ||
          ((LayerId == XVMIX_LAYER_LOGO) &&
           (XVMix_IsLogoEnabled(InstancePtr)))) {
    CurrenState = XV_mix_Get_HwReg_layerEnable(MixPtr);
    CurrenState &= ~(1<<LayerId);
    XV_mix_Set_HwReg_layerEnable(MixPtr, CurrenState);
    Status = XST_SUCCESS;
  }
  return(Status);
}

/*****************************************************************************/
/**
*
* This function returns state of the specified layer [enabled or disabled]
*
* @param    InstancePtr is a pointer to the core instance.
* @param    LayerId is the layer index for which information is requested
*
* @return   Enabled(1)/Disabled(0)
*
******************************************************************************/
int XVMix_IsLayerEnabled(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId)
{
  XV_mix *MixPtr;
  u32 State, Mask;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId >= XVMIX_LAYER_MASTER) &&
                    (LayerId < XVMIX_LAYER_LAST));

  MixPtr = &InstancePtr->Mix;
  Mask = (1<<LayerId);
  State = XV_mix_Get_HwReg_layerEnable(MixPtr);
  return ((State & Mask) ? TRUE : FALSE);
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

  cfmt = (XVidC_ColorFormat)XV_mix_Get_HwReg_video_format(&InstancePtr->Mix);

  if(cfmt == XVIDC_CSF_RGB) {
    scale = ((1<<bpc)-1);
    y_r_val = bkgndColorRGB[ColorId][0] * scale;
    u_g_val = bkgndColorRGB[ColorId][1] * scale;
    v_b_val = bkgndColorRGB[ColorId][2] * scale;
  } else {//YUV
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
* @param  Win is the window coordinates in pixels
* @param  StrideInBytes is the stride of the requested window
*           yuv422 Color space requires 2 Bytes/Pixel
*           yuv444 Color space requires 4 Bytes/Pixel
*           Equation to compute stride is as follows
*              Stride = (Window_Width * (YUV422 ? 2 : 4))
*           (Applicable only when layer type is Memory)
*
* @return XST_SUCCESS if command is successful else error code with reason
*
* @note   Applicable only for Layer1-8 and Logo Layer
*
******************************************************************************/
int XVMix_SetLayerWindow(XV_Mix_l2 *InstancePtr,
                         XVMix_LayerId LayerId,
                         XVidC_VideoWindow *Win,
                         u32 StrideInBytes)
{
  XV_mix *MixPtr;
  u32 Align, WinValid;
  XVMix_Scalefactor Scale;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId > XVMIX_LAYER_MASTER) &&
                    (LayerId <= XVMIX_LAYER_LOGO));
  Xil_AssertNonvoid((Win->StartX % InstancePtr->Mix.Config.PixPerClk) == 0);
  Xil_AssertNonvoid((Win->Width  % InstancePtr->Mix.Config.PixPerClk) == 0);

  /* Check window coordinates */
  Scale = (XVMix_Scalefactor)XVMix_GetLayerScaleFactor(InstancePtr, LayerId);
  if(!IsWindowValid(&InstancePtr->Stream, Win, Scale)) {
      return(XVMIX_ERR_LAYER_WINDOW_INVALID);
  }

  MixPtr = &InstancePtr->Mix;

  switch(LayerId) {
    case XVMIX_LAYER_LOGO:
       if(XVMix_IsLogoEnabled(InstancePtr)) {
         u32 WinResInRange;

         WinResInRange = ((Win->Width  > (XVMIX_MIN_LOGO_WIDTH-1))  &&
                          (Win->Height > (XVMIX_MIN_LOGO_HEIGHT-1)) &&
                          (Win->Width  <= MixPtr->Config.MaxLogoWidth) &&
                          (Win->Height <= MixPtr->Config.MaxLogoHeight));
         if(WinResInRange) {
            XV_mix_Set_HwReg_logoStartX(MixPtr, Win->StartX);
            XV_mix_Set_HwReg_logoStartY(MixPtr, Win->StartY);
            XV_mix_Set_HwReg_logoWidth(MixPtr,  Win->Width);
            XV_mix_Set_HwReg_logoHeight(MixPtr, Win->Height);

            InstancePtr->Layer[LayerId].Win = *Win;
            Status = XST_SUCCESS;
         } else {
            Status = XVMIX_ERR_LAYER_WINDOW_INVALID;
         }
      } else {
         Status = XVMIX_ERR_DISABLED_IN_HW;
      }
      break;

    default: //Layer1-Layer8
       if(LayerId < XVMix_GetNumLayers(InstancePtr)) {
         u32 WinResInRange;

         WinResInRange = ((Win->Width  > (XVMIX_MIN_STRM_WIDTH-1))  &&
                          (Win->Height > (XVMIX_MIN_STRM_HEIGHT-1)) &&
                          (Win->Width  < MixPtr->Config.LayerMaxWidth[LayerId-1]) &&
                          (Win->Height <= MixPtr->Config.MaxHeight));
         if(WinResInRange) {
           /* Check layer interface is Stream or Memory */
           if(XVMix_IsLayerInterfaceStream(InstancePtr, LayerId)) {
             /* Stride is not required for stream layer */
             WinValid = TRUE;
           } else {
             /* Check if stride is aligned to aximm width (2*PPC*32-bits) */
             Align = 2 * InstancePtr->Mix.Config.PixPerClk * 4;
             if((StrideInBytes % Align) != 0) {
               WinValid = FALSE;
               Status   = XVMIX_ERR_WIN_STRIDE_MISALIGNED;
             } else {
               WinValid = TRUE;
             }
           }

           if(WinValid) {

             u32 BaseStartXReg, BaseStartYReg;
             u32 BaseWidthReg, BaseHeightReg;
             u32 BaseStrideReg;
             u32 Offset;

             BaseStartXReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_0_DATA;
             BaseStartYReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_0_DATA;
             BaseWidthReg  = XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_0_DATA;
             BaseHeightReg = XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_0_DATA;
             BaseStrideReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_0_DATA;
             Offset = LayerId*XVMIX_REG_OFFSET;

             XV_mix_WriteReg(MixPtr->Config.BaseAddress,
                             (BaseStartXReg+Offset), Win->StartX);
             XV_mix_WriteReg(MixPtr->Config.BaseAddress,
                             (BaseStartYReg+Offset), Win->StartY);
             XV_mix_WriteReg(MixPtr->Config.BaseAddress,
                             (BaseWidthReg+Offset),  Win->Width);
             XV_mix_WriteReg(MixPtr->Config.BaseAddress,
                             (BaseHeightReg+Offset), Win->Height);

             if(!XVMix_IsLayerInterfaceStream(InstancePtr, LayerId)) {
                XV_mix_WriteReg(MixPtr->Config.BaseAddress,
                                (BaseStrideReg+Offset), StrideInBytes);
             }
             InstancePtr->Layer[LayerId].Win = *Win;
             Status = XST_SUCCESS;
           }
         } else { //if(WinResInRange)
             Status = XVMIX_ERR_LAYER_WINDOW_INVALID;
         }
       } else { //if(LayerId <
            Status = XVMIX_ERR_DISABLED_IN_HW;
       }
       break;
  }//switch

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
* @return XST_SUCCESS if command is successful else error code with reason
*
* @note   Applicable only for Layer1-8 and Logo Layer
*
******************************************************************************/
int XVMix_GetLayerWindow(XV_Mix_l2 *InstancePtr,
                         XVMix_LayerId LayerId,
                         XVidC_VideoWindow *Win)
{
  XV_mix *MixPtr;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId > XVMIX_LAYER_MASTER) &&
                    (LayerId <= XVMIX_LAYER_LOGO));
  Xil_AssertNonvoid(Win != NULL);

  MixPtr = &InstancePtr->Mix;

  switch(LayerId) {
    case XVMIX_LAYER_LOGO:
      if(XVMix_IsLogoEnabled(InstancePtr)) {

        Win->StartX = XV_mix_Get_HwReg_logoStartX(MixPtr);
        Win->StartY = XV_mix_Get_HwReg_logoStartY(MixPtr);
        Win->Width  = XV_mix_Get_HwReg_logoWidth(MixPtr);
        Win->Height = XV_mix_Get_HwReg_logoHeight(MixPtr);

        Status = XST_SUCCESS;
      } else {
        Status = XVMIX_ERR_DISABLED_IN_HW;
      }
      break;

    default: //Layer0-Layer8
      if(LayerId < XVMix_GetNumLayers(InstancePtr)) {

        u32 BaseStartXReg, BaseStartYReg;
        u32 BaseWidthReg, BaseHeightReg;
        u32 Offset;

        BaseStartXReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_0_DATA;
        BaseStartYReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_0_DATA;
        BaseWidthReg  = XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_0_DATA;
        BaseHeightReg = XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_0_DATA;
        Offset = LayerId*XVMIX_REG_OFFSET;

        Win->StartX = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
                                     (BaseStartXReg+Offset));
        Win->StartY = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
                                     (BaseStartYReg+Offset));
        Win->Width  = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
                                     (BaseWidthReg+Offset));
        Win->Height = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
                                     (BaseHeightReg+Offset));

        Status = XST_SUCCESS;
      } else {
        Status = XVMIX_ERR_DISABLED_IN_HW;
      }
      break;
  }

  return(Status);
}

/*****************************************************************************/
/**
* This function moved the window position of the specified layer to new
* coordinates
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer for which window position is to be set
* @param  StartX is the new X position
* @param  StartY is the new Y position
*
* @return XST_SUCCESS if command is successful else error code with reason
*
* @note   Applicable only for Layer1-8 and Logo Layer
*
******************************************************************************/
int XVMix_MoveLayerWindow(XV_Mix_l2 *InstancePtr,
                          XVMix_LayerId LayerId,
                          u16 StartX,
                          u16 StartY)
{
  XV_mix *MixPtr;
  XVidC_VideoWindow CurrWin;
  XVMix_Scalefactor Scale;
  int Status = XST_FAILURE;
  int WinStatus;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId > XVMIX_LAYER_MASTER) &&
                    (LayerId <= XVMIX_LAYER_LOGO));
  Xil_AssertNonvoid((StartX % InstancePtr->Mix.Config.PixPerClk) == 0);

  /* Get current window */
  WinStatus = XVMix_GetLayerWindow(InstancePtr, LayerId, &CurrWin);
  if(WinStatus != XST_SUCCESS) {
     return(WinStatus);
  }

  /* Update Start Position */
  CurrWin.StartX = StartX;
  CurrWin.StartY = StartY;
  /* Get scale factor */
  Scale = (XVMix_Scalefactor)XVMix_GetLayerScaleFactor(InstancePtr, LayerId);
  /* Validate new start position will not cause the layer window
   * to go out of scope
   */
  if(!IsWindowValid(&InstancePtr->Stream, &CurrWin, Scale)) {
      return(XVMIX_ERR_LAYER_WINDOW_INVALID);
  }

  MixPtr = &InstancePtr->Mix;

  switch(LayerId) {
    case XVMIX_LAYER_LOGO:
      if(XVMix_IsLogoEnabled(InstancePtr)) {

        XV_mix_Set_HwReg_logoStartX(MixPtr, StartX);
        XV_mix_Set_HwReg_logoStartY(MixPtr, StartY);

        InstancePtr->Layer[LayerId].Win.StartX = StartX;
        InstancePtr->Layer[LayerId].Win.StartY = StartY;
        Status = XST_SUCCESS;
      }
      break;

    default: //Layer1-Layer8
      if(LayerId < XVMix_GetNumLayers(InstancePtr)) {
        u32 BaseStartXReg, BaseStartYReg;
        u32 Offset;

        BaseStartXReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_0_DATA;
        BaseStartYReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_0_DATA;
        Offset = LayerId*XVMIX_REG_OFFSET;

        XV_mix_WriteReg(MixPtr->Config.BaseAddress,
                        (BaseStartXReg+Offset), StartX);
        XV_mix_WriteReg(MixPtr->Config.BaseAddress,
                        (BaseStartYReg+Offset), StartY);

        InstancePtr->Layer[LayerId].Win.StartX = StartX;
        InstancePtr->Layer[LayerId].Win.StartY = StartY;
        Status = XST_SUCCESS;
      }
      break;
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function configures the scaling factor of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer to be updated
* @param  Scale is the scale factor
*
* @return XST_SUCCESS if command is successful else error code with reason
*
* @note   Applicable only for Layer1-8 and Logo Layer
*
******************************************************************************/
int XVMix_SetLayerScaleFactor(XV_Mix_l2 *InstancePtr,
                              XVMix_LayerId LayerId,
                              XVMix_Scalefactor Scale)
{
  XV_mix *MixPtr;
  XVidC_VideoWindow CurrWin;
  int Status = XST_FAILURE;
  int WinStatus;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId > XVMIX_LAYER_MASTER) &&
                    (LayerId <= XVMIX_LAYER_LOGO));
  Xil_AssertNonvoid((Scale >= XVMIX_SCALE_FACTOR_1X) &&
                    (Scale <= XVMIX_SCALE_FACTOR_4X));

  /* Validate if scaling will cause the layer window to go out of scope */
  WinStatus = XVMix_GetLayerWindow(InstancePtr, LayerId, &CurrWin);
  if(WinStatus != XST_SUCCESS) {
     return(WinStatus);
  }

  if(!IsWindowValid(&InstancePtr->Stream, &CurrWin, Scale)) {
      return(XVMIX_ERR_LAYER_WINDOW_INVALID);
  }

  MixPtr = &InstancePtr->Mix;

  switch(LayerId) {
    case XVMIX_LAYER_LOGO:
      if(XVMix_IsLogoEnabled(InstancePtr)) {
        XV_mix_Set_HwReg_logoScaleFactor(MixPtr, Scale);
        Status = XST_SUCCESS;
      }
      break;

    default: //Layer0-Layer8
      if((LayerId < XVMix_GetNumLayers(InstancePtr)) &&
         (XVMix_IsScalingEnabled(InstancePtr, LayerId))) {

        u32 BaseReg;

        BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_0_DATA;
        XV_mix_WriteReg(MixPtr->Config.BaseAddress,
                        (BaseReg+(LayerId*XVMIX_REG_OFFSET)), Scale);

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
* @return If layer enabled, current set value, else ~0
*
* @note   Applicable only for Layer1-8 and Logo Layer
*
******************************************************************************/
int XVMix_GetLayerScaleFactor(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId)
{
  XV_mix *MixPtr;
  u32 ReadVal = ~0;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId > XVMIX_LAYER_MASTER) &&
                    (LayerId <= XVMIX_LAYER_LOGO));

  MixPtr = &InstancePtr->Mix;

  switch(LayerId) {
    case XVMIX_LAYER_LOGO:
      if(XVMix_IsLogoEnabled(InstancePtr)) {
        ReadVal = XV_mix_Get_HwReg_logoScaleFactor(MixPtr);
      }
      break;

    default: //Layer0-Layer8
      if((LayerId < XVMix_GetNumLayers(InstancePtr)) &&
         (XVMix_IsScalingEnabled(InstancePtr, LayerId))) {
        u32 BaseReg;

        BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_0_DATA;
        ReadVal = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
                                 (BaseReg+(LayerId*XVMIX_REG_OFFSET)));
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
* @return XST_SUCCESS if command is successful else error code with reason
*
* @note   Applicable only for Layer1-8 and Logo Layer
*
******************************************************************************/
int XVMix_SetLayerAlpha(XV_Mix_l2 *InstancePtr,
                        XVMix_LayerId LayerId,
                        u16 Alpha)
{
  XV_mix *MixPtr;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId > XVMIX_LAYER_MASTER) &&
                    (LayerId <= XVMIX_LAYER_LOGO));
  Xil_AssertNonvoid(Alpha <= XVMIX_ALPHA_MAX);

  MixPtr = &InstancePtr->Mix;

  switch(LayerId) {
    case XVMIX_LAYER_LOGO:
      if(XVMix_IsLogoEnabled(InstancePtr)) {
        XV_mix_Set_HwReg_logoAlpha(MixPtr, Alpha);
        Status = XST_SUCCESS;
      } else {
        Status = XVMIX_ERR_DISABLED_IN_HW;
      }
      break;

    default: //Layer1-Layer8
      if((LayerId < XVMix_GetNumLayers(InstancePtr)) &&
         (XVMix_IsAlphaEnabled(InstancePtr, LayerId))) {
        u32 BaseReg;

        BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_0_DATA;
        XV_mix_WriteReg(MixPtr->Config.BaseAddress,
                        (BaseReg+(LayerId*XVMIX_REG_OFFSET)), Alpha);
        Status = XST_SUCCESS;
      } else {
        Status = XVMIX_ERR_DISABLED_IN_HW;
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
* @return If layer enabled, current set value, else ~0
*
* @note   Applicable only for Layer1-8 and Logo Layer
*
******************************************************************************/
int XVMix_GetLayerAlpha(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId)
{
  XV_mix *MixPtr;
  u32 ReadVal = ~0;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId > XVMIX_LAYER_MASTER) &&
                    (LayerId <= XVMIX_LAYER_LOGO));

  MixPtr = &InstancePtr->Mix;

  switch(LayerId) {
    case XVMIX_LAYER_LOGO:
      if(XVMix_IsLogoEnabled(InstancePtr)) {
        ReadVal = XV_mix_Get_HwReg_logoAlpha(MixPtr);
      }
      break;

    default: //Layer1-Layer8
      if((LayerId < XVMix_GetNumLayers(InstancePtr)) &&
         (XVMix_IsAlphaEnabled(InstancePtr, LayerId))) {
        u32 BaseReg;

        BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_0_DATA;
        ReadVal = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
                                 (BaseReg+(LayerId*XVMIX_REG_OFFSET)));
      }
      break;
  }
  return(ReadVal);
}

#if 0 //Reserved for future use - In IP this is build time option
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
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId >= XVMIX_LAYER_MASTER) &&
                    (LayerId < XVMIX_LAYER_LOGO));
  Xil_AssertNonvoid((Cfmt >= XVIDC_CSF_RGB) &&
                    (Cfmt < XVIDC_CSF_YCRCB_420));

  MixPtr = &InstancePtr->Mix;

  if(LayerId < XVMix_GetNumLayers(InstancePtr)) {

      if (LayerId == XVMIX_LAYER_MASTER) {
          XV_mix_Set_HwReg_video_format(MixPtr, Cfmt);
      } else { //Layer 1-8
          u32 BaseReg;

          BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_0_DATA;
          XV_mix_WriteReg(MixPtr->Config.BaseAddress,
                          (BaseReg+(LayerId*XVMIX_REG_OFFSET)), Cfmt);
      }
      InstancePtr->Layer[LayerId].ColorFormat = Cfmt;
      Status = XST_SUCCESS;
  }
  return(Status);
}
#endif

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
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId >= XVMIX_LAYER_MASTER) &&
                    (LayerId < XVMIX_LAYER_LOGO));
  Xil_AssertNonvoid(Cfmt != NULL);

  MixPtr = &InstancePtr->Mix;

  if(LayerId < XVMix_GetNumLayers(InstancePtr)) {

#if 0 //register interface not used - IP build time option
      if (LayerId == XVMIX_LAYER_MASTER) {
          *Cfmt = XV_mix_Get_HwReg_video_format(MixPtr);
      } else { //Layer 1-8
          u32 BaseReg;

          BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_0_DATA;
          *Cfmt = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
                                 (BaseReg+(LayerId*XVMIX_REG_OFFSET)));
      }
#endif
      if (LayerId == XVMIX_LAYER_MASTER) {
          *Cfmt = (XVidC_ColorFormat)MixPtr->Config.ColorFormat;
      } else { //Layer 1-16
          *Cfmt = (XVidC_ColorFormat)(MixPtr->Config.LayerColorFmt[LayerId-1]);
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
* @param  Addr is the absolute address of buffer in memory
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   Applicable only for Layer1-8
*
******************************************************************************/
int XVMix_SetLayerBufferAddr(XV_Mix_l2 *InstancePtr,
                             XVMix_LayerId LayerId,
                             UINTPTR Addr)
{
  XV_mix *MixPtr;
  UINTPTR BaseReg, Align;
  u32 WinValid = FALSE;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId > XVMIX_LAYER_MASTER) &&
                    (LayerId < XVMIX_LAYER_LOGO));
  Xil_AssertNonvoid(Addr != 0);

  MixPtr = &InstancePtr->Mix;

  if(LayerId < XVMix_GetNumLayers(InstancePtr)) {
      /* Check if addr is aligned to aximm width (2*PPC*32-bits (4Bytes)) */
      Align = 2 * InstancePtr->Mix.Config.PixPerClk * 4;
      if((Addr % Align) != 0) {
         WinValid = FALSE;
         Status   = XVMIX_ERR_MEM_ADDR_MISALIGNED;
      } else {
         WinValid = TRUE;
      }

      if(WinValid) {
        BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF1_V_DATA;

        XV_mix_WriteReg(MixPtr->Config.BaseAddress,
                        (BaseReg+((LayerId-1)*XVMIX_REG_OFFSET)), Addr);

        InstancePtr->Layer[LayerId].BufAddr = Addr;
        Status = XST_SUCCESS;
      }
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function reads the buffer address of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer to be updated
*
* @return Address of buffer in memory
*
* @note   Applicable only for Layer1-8
*
******************************************************************************/
UINTPTR XVMix_GetLayerBufferAddr(XV_Mix_l2 *InstancePtr, XVMix_LayerId LayerId)
{
  XV_mix *MixPtr;
  u32 BaseReg;
  UINTPTR ReadVal = 0;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId > XVMIX_LAYER_MASTER) &&
                    (LayerId < XVMIX_LAYER_LOGO));

  MixPtr = &InstancePtr->Mix;

  if(LayerId < XVMix_GetNumLayers(InstancePtr)) {
        BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF1_V_DATA;

        ReadVal = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
                                 (BaseReg+((LayerId-1)*XVMIX_REG_OFFSET)));
  }
  return(ReadVal);
}

/*****************************************************************************/
/**
* This function sets the buffer address of the specified layer
* for the UV plane for semi-planar formats
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer to be updated
* @param  Addr is the absolute address of second buffer in memory
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   Applicable only for Layer1-8
*
******************************************************************************/
int XVMix_SetLayerChromaBufferAddr(XV_Mix_l2 *InstancePtr,
                                   XVMix_LayerId LayerId,
                                   UINTPTR Addr)
{
  XV_mix *MixPtr;
  UINTPTR BaseReg, Align;
  u32 WinValid = FALSE;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId > XVMIX_LAYER_MASTER) &&
                    (LayerId < XVMIX_LAYER_LOGO));
  Xil_AssertNonvoid(Addr != 0);

  MixPtr = &InstancePtr->Mix;

  if(LayerId < XVMix_GetNumLayers(InstancePtr)) {
      /* Check if addr is aligned to aximm width (2*PPC*32-bits (4Bytes)) */
      Align = 2 * InstancePtr->Mix.Config.PixPerClk * 4;
      if((Addr % Align) != 0) {
         WinValid = FALSE;
         Status   = XVMIX_ERR_MEM_ADDR_MISALIGNED;
      } else {
         WinValid = TRUE;
      }

      if(WinValid) {
        BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF2_V_DATA;

        XV_mix_WriteReg(MixPtr->Config.BaseAddress,
                        (BaseReg+((LayerId-1)*XVMIX_REG_OFFSET)), Addr);

        InstancePtr->Layer[LayerId].ChromaBufAddr = Addr;
        Status = XST_SUCCESS;
      }
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function reads the buffer address of the specified layer
* for the UV plane for semi-planar formats
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer to be updated
*
* @return Address of second buffer in memory
*
* @note   Applicable only for Layer1-8
*
******************************************************************************/
UINTPTR XVMix_GetLayerChromaBufferAddr(XV_Mix_l2 *InstancePtr,
                                       XVMix_LayerId LayerId)
{
  XV_mix *MixPtr;
  u32 BaseReg;
  UINTPTR ReadVal = 0;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((LayerId > XVMIX_LAYER_MASTER) &&
                    (LayerId < XVMIX_LAYER_LOGO));

  MixPtr = &InstancePtr->Mix;

  if(LayerId < XVMix_GetNumLayers(InstancePtr)) {
        BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF2_V_DATA;

        ReadVal = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
                                 (BaseReg+((LayerId-1)*XVMIX_REG_OFFSET)));
  }
  return(ReadVal);
}

/*****************************************************************************/
/**
* This function sets the logo layer color key data
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  ColorKeyData is the structure that holds the min/max values
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   none
*
******************************************************************************/
int XVMix_SetLogoColorKey(XV_Mix_l2 *InstancePtr,
                          XVMix_LogoColorKey ColorKeyData)
{
  XV_mix *MixPtr;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);

  if(XVMix_IsLogoEnabled(InstancePtr) &&
     XVMix_IsLogoColorKeyEnabled(InstancePtr)) {

      MixPtr = &InstancePtr->Mix;

      XV_mix_Set_HwReg_logoClrKeyMin_R(MixPtr, ColorKeyData.RGB_Min[0]);
      XV_mix_Set_HwReg_logoClrKeyMin_G(MixPtr, ColorKeyData.RGB_Min[1]);
      XV_mix_Set_HwReg_logoClrKeyMin_B(MixPtr, ColorKeyData.RGB_Min[2]);
      XV_mix_Set_HwReg_logoClrKeyMax_R(MixPtr, ColorKeyData.RGB_Max[0]);
      XV_mix_Set_HwReg_logoClrKeyMax_G(MixPtr, ColorKeyData.RGB_Max[1]);
      XV_mix_Set_HwReg_logoClrKeyMax_B(MixPtr, ColorKeyData.RGB_Max[2]);

      Status = XST_SUCCESS;
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function reads the logo layer color key data
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  ColorKeyData is the structure that holds return min/max values
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   none
*
******************************************************************************/
int XVMix_GetLogoColorKey(XV_Mix_l2 *InstancePtr,
                          XVMix_LogoColorKey *ColorKeyData)
{
  XV_mix *MixPtr;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);

  if(XVMix_IsLogoEnabled(InstancePtr) &&
     XVMix_IsLogoColorKeyEnabled(InstancePtr)) {

      MixPtr = &InstancePtr->Mix;

      ColorKeyData->RGB_Min[0] = XV_mix_Get_HwReg_logoClrKeyMin_R(MixPtr);
      ColorKeyData->RGB_Min[1] = XV_mix_Get_HwReg_logoClrKeyMin_G(MixPtr);
      ColorKeyData->RGB_Min[2] = XV_mix_Get_HwReg_logoClrKeyMin_B(MixPtr);
      ColorKeyData->RGB_Max[0] = XV_mix_Get_HwReg_logoClrKeyMax_R(MixPtr);
      ColorKeyData->RGB_Max[1] = XV_mix_Get_HwReg_logoClrKeyMax_G(MixPtr);
      ColorKeyData->RGB_Max[2] = XV_mix_Get_HwReg_logoClrKeyMax_B(MixPtr);

      Status = XST_SUCCESS;
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function loads the logo data into core BRAM
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Win is logo window (logo width must be multiple of 4 bytes)
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
  u32 x,y;
  u32 Rword, Gword, Bword;
  u32 Width, Height;
  u32 RBaseAddr, GBaseAddr, BBaseAddr;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(RBuffer != NULL);
  Xil_AssertNonvoid(GBuffer != NULL);
  Xil_AssertNonvoid(BBuffer != NULL);
  Xil_AssertNonvoid((Win->Width  > (XVMIX_MIN_LOGO_WIDTH-1)) &&
                    (Win->Width  <= InstancePtr->Mix.Config.MaxLogoWidth));
  Xil_AssertNonvoid((Win->Height > (XVMIX_MIN_LOGO_HEIGHT-1)) &&
                    (Win->Height <= InstancePtr->Mix.Config.MaxLogoHeight));
  Xil_AssertNonvoid((Win->StartX % InstancePtr->Mix.Config.PixPerClk) == 0);
  Xil_AssertNonvoid((Win->Width  % InstancePtr->Mix.Config.PixPerClk) == 0);

  if(XVMix_IsLogoEnabled(InstancePtr)) {
      MixPtr = &InstancePtr->Mix;
      Width  = Win->Width;
      Height = Win->Height;

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

      Status = XVMix_SetLayerWindow(InstancePtr, XVMIX_LAYER_LOGO, Win, 0);
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function loads the logo pixel alpha data into core BRAM
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Win is logo window (logo width must be multiple of 4 bytes)
* @param  ABuffer is the pointer to Pixel Alpha buffer
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   none
*
******************************************************************************/
int XVMix_LoadLogoPixelAlpha(XV_Mix_l2 *InstancePtr,
                             XVidC_VideoWindow *Win,
                             u8 *ABuffer)
{
  XV_mix *MixPtr;
  u32 x,y;
  u32 Aword, ABaseAddr;
  u32 Width, Height;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(ABuffer != NULL);
  Xil_AssertNonvoid((Win->Width  > (XVMIX_MIN_LOGO_WIDTH-1)) &&
                    (Win->Width  <= InstancePtr->Mix.Config.MaxLogoWidth));
  Xil_AssertNonvoid((Win->Height > (XVMIX_MIN_LOGO_HEIGHT-1)) &&
                    (Win->Height <= InstancePtr->Mix.Config.MaxLogoHeight));
  Xil_AssertNonvoid((Win->StartX % InstancePtr->Mix.Config.PixPerClk) == 0);
  Xil_AssertNonvoid((Win->Width  % InstancePtr->Mix.Config.PixPerClk) == 0);

  if(XVMix_IsLogoEnabled(InstancePtr) &&
     XVMix_IsLogoPixAlphaEnabled(InstancePtr)) {
      MixPtr = &InstancePtr->Mix;
      Width  = Win->Width;
      Height = Win->Height;

      ABaseAddr = XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE;

      for (y=0; y<Height; y++) {
          for (x=0; x<Width; x+=4) {
            Aword = (u32)ABuffer[y*Width+x] |
                    (((u32)ABuffer[y*Width+x+1])<<8) |
                    (((u32)ABuffer[y*Width+x+2])<<16) |
                    (((u32)ABuffer[y*Width+x+3])<<24);

            XV_mix_WriteReg(MixPtr->Config.BaseAddress, (ABaseAddr+(y*Width+x)), Aword);
          }
      }
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
  u32 index, IsEnabled, ctrl;
  const char *Status[2] = {"Disabled", "Enabled"};

  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n\r\n----->MIXER STATUS<----\r\n");
  MixPtr = &InstancePtr->Mix;

  ctrl  = XV_mix_ReadReg(MixPtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL);

  xil_printf("Pixels Per Clock: %d\r\n", InstancePtr->Mix.Config.PixPerClk);
  xil_printf("Color Depth:      %d\r\n", InstancePtr->Mix.Config.MaxDataWidth);
  xil_printf("Number of Layers: %d\r\n",XVMix_GetNumLayers(InstancePtr));
  xil_printf("Control Reg:      0x%x\r\n", ctrl);
  xil_printf("Layer Enable Reg: 0x%x\r\n\r\n",XV_mix_Get_HwReg_layerEnable(MixPtr));

  IsEnabled = XVMix_IsLayerEnabled(InstancePtr, XVMIX_LAYER_MASTER);
  xil_printf("Layer Master: %s\r\n", Status[IsEnabled]);
  for(index = XVMIX_LAYER_1; index<XVMIX_LAYER_LOGO; ++index) {
      xil_printf("Layer %d     : %s\r\n" ,index,
              Status[(XVMix_IsLayerEnabled(InstancePtr, (XVMix_LayerId)index))]);
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
  u32 IsEnabled;
  u32 ReadVal;
  XVidC_VideoWindow Win;
  XVidC_ColorFormat ColFormat;
  XVMix_LayerType LayerType;
  const char *Status[2] = {"Disabled", "Enabled"};
  const char *ScaleFactor[3] = {"1x", "2x", "4x"};
  const char *IntfType[2] = {"Memory", "Stream"};

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((LayerId >= XVMIX_LAYER_MASTER) &&
                 (LayerId <= XVMIX_LAYER_LOGO));

  MixPtr = &InstancePtr->Mix;
  IsEnabled = XVMix_IsLayerEnabled(InstancePtr, LayerId);

  switch(LayerId) {
    case XVMIX_LAYER_MASTER:
        xil_printf("\r\n\r\n----->Master Layer Status<----\r\n");
        xil_printf("State: %s\r\n", Status[IsEnabled]);
        if(IsEnabled) {
          u32 width, height;

          XVMix_GetLayerColorFormat(InstancePtr, LayerId, &ColFormat);
          width  = XV_mix_Get_HwReg_width(&InstancePtr->Mix);
          height = XV_mix_Get_HwReg_height(&InstancePtr->Mix);
          xil_printf("Color Format: %s\r\n\r\n",
                     XVidC_GetColorFormatStr(ColFormat));
          xil_printf("Resolution: %d x %d\r\n", width, height);
          xil_printf("Stream Info->\r\n");
          XVidC_ReportStreamInfo(&InstancePtr->Stream);
        }
        break;

    case XVMIX_LAYER_LOGO:
        xil_printf("\r\n\r\n----->Layer LOGO Status<----\r\n");
        xil_printf("State: %s\r\n", Status[IsEnabled]);
        if(IsEnabled) {

          ReadVal = XVMix_GetLayerAlpha(InstancePtr, LayerId);
          xil_printf("Alpha: %d\r\n", ReadVal);
          ReadVal = XVMix_GetLayerScaleFactor(InstancePtr, LayerId);
          xil_printf("Scale: %s\r\n\r\n", ScaleFactor[ReadVal]);
          xil_printf("Window Data: \r\n");
          XVMix_GetLayerWindow(InstancePtr, LayerId, &Win);
          xil_printf("   Start X    = %d\r\n", Win.StartX);
          xil_printf("   Start Y    = %d\r\n", Win.StartY);
          xil_printf("   Win Width  = %d\r\n", Win.Width);
          xil_printf("   Win Height = %d\r\n", Win.Height);

          IsEnabled = XVMix_IsLogoColorKeyEnabled(InstancePtr);
          if(IsEnabled) {
            XVMix_LogoColorKey Data;

            XVMix_GetLogoColorKey(InstancePtr, &Data);
            xil_printf("\r\nColor Key Data: \r\n");
            xil_printf("     Min    Max\r\n");
            xil_printf("    -----  -----\r\n");
            xil_printf("  R: %3d    %3d\r\n", Data.RGB_Min[0],Data.RGB_Max[0]);
            xil_printf("  G: %3d    %3d\r\n", Data.RGB_Min[1],Data.RGB_Max[1]);
            xil_printf("  B: %3d    %3d\r\n", Data.RGB_Min[2],Data.RGB_Max[2]);
          } else {
            xil_printf("Color Key: %s\r\n", Status[IsEnabled]);
          }
          IsEnabled = XVMix_IsLogoPixAlphaEnabled(InstancePtr);
          xil_printf("\r\nPixel Alpha: %s\r\n", Status[IsEnabled]);
        }
        break;

    default: //Layer1-8
        LayerType = (XVMix_LayerType)XVMix_GetLayerInterfaceType(InstancePtr, LayerId);
        xil_printf("\r\n\r\n----->Layer %d Status<----\r\n", LayerId);
        xil_printf("State: %s\r\n", Status[IsEnabled]);
        xil_printf("Type : %s\r\n", IntfType[LayerType]);
        if(IsEnabled) {
          u32 Stride, Reg, Offset;

          xil_printf("Addr : 0x%x\r\n",
                       XVMix_GetLayerBufferAddr(InstancePtr, LayerId));
          xil_printf("Chroma Addr : 0x%x\r\n",
                       XVMix_GetLayerChromaBufferAddr(InstancePtr, LayerId));

          IsEnabled = XVMix_IsAlphaEnabled(InstancePtr, LayerId);
          if (IsEnabled) {
              ReadVal = XVMix_GetLayerAlpha(InstancePtr, LayerId);
              xil_printf("Alpha: %d\r\n", ReadVal);
          } else {
              xil_printf("Alpha: %s\r\n", Status[IsEnabled]);
          }

          IsEnabled = XVMix_IsScalingEnabled(InstancePtr, LayerId);
          if (IsEnabled) {
              ReadVal = XVMix_GetLayerScaleFactor(InstancePtr, LayerId);
              xil_printf("Scale: %s\r\n", ScaleFactor[ReadVal]);
          } else {
              xil_printf("Scale: %s\r\n", Status[IsEnabled]);
          }

          XVMix_GetLayerColorFormat(InstancePtr, LayerId, &ColFormat);
          xil_printf("Color Format: %s\r\n\r\n",
                          XVidC_GetColorFormatStr(ColFormat));

          xil_printf("Window Data: \r\n");
          Reg = XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_0_DATA;
          Offset = LayerId*XVMIX_REG_OFFSET;
          Stride = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
                                  (Reg+Offset));

          XVMix_GetLayerWindow(InstancePtr, LayerId, &Win);
          xil_printf("   Start X    = %d\r\n", Win.StartX);
          xil_printf("   Start Y    = %d\r\n", Win.StartY);
          xil_printf("   Win Width  = %d\r\n", Win.Width);
          xil_printf("   Win Height = %d\r\n", Win.Height);
          xil_printf("   Win Stride = %d\r\n", Stride);
        } //Layer State
        break;
  }
}

/*****************************************************************************/
/**
* This function sets up the Mixer coefficient values for YUV to RGB
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  ColorStd is used to calculate coefficients
* @param  colorRange is used to calculate coefficients
* @param  colorDepth is width of colour component
*
* @return none
*
* @note   Applicable for all 16 Layers
*
******************************************************************************/
static void XVMix_SetCoeffForYuvToRgb(XV_Mix_l2 *InstancePtr,
		XVidC_ColorStd ColorStd,
		XVidC_ColorRange colorRange,
		u8 colorDepth)
{
	XV_mix *MixPtr;
	u32 scale_factor = 1 << XVMIX_CSC_COEFF_FRACTIONAL_BITS;
	u32 bpcScale = 1 << (colorDepth-8);
	u32 i;

	MixPtr = &InstancePtr->Mix;

	for (i = 0; i < XVMIX_CSC_MATRIX_SIZE; i++)
		XV_mix_WriteReg(MixPtr->Config.BaseAddress,
				XV_MIX_CTRL_ADDR_HWREG_K11_DATA + i * 8,
				xv_mix_yuv2rgb_coeffs[ColorStd][colorRange][i] \
				* scale_factor / XVMIX_CSC_COEFF_DIVISOR);

	for (i = XVMIX_CSC_MATRIX_SIZE; i < XVMIX_CSC_COEFF_SIZE; i++)
		XV_mix_WriteReg(MixPtr->Config.BaseAddress,
				XV_MIX_CTRL_ADDR_HWREG_K11_DATA + i * 8,
				xv_mix_yuv2rgb_coeffs[ColorStd][colorRange][i] * bpcScale);

}

/*****************************************************************************/
/**
 * This function sets up the Mixer coefficient values for RGB to YUV
 *
 * @param  InstancePtr is a pointer to core instance to be worked upon
 * @param  ColorStd is used to calculate coefficients
 * @param  colorRange is used to calculate coefficients
 * @param  colorDepth is width of colour component
 *
 * @return none
 *
 * @note   Applicable for all 16 Layers
 *
 ******************************************************************************/
static void XVMix_SetCoeffForRgbToYuv(XV_Mix_l2 *InstancePtr,
		XVidC_ColorStd ColorStd,
		XVidC_ColorRange colorRange,
		u8 colorDepth)
{
	XV_mix *MixPtr;
	u32 scale_factor = 1 << XVMIX_CSC_COEFF_FRACTIONAL_BITS;
	u32 bpcScale = 1 << (colorDepth-8);
	u32 i;

	MixPtr = &InstancePtr->Mix;

	for (i = 0; i < XVMIX_CSC_MATRIX_SIZE; i++)
		XV_mix_WriteReg(MixPtr->Config.BaseAddress,
				XV_MIX_CTRL_ADDR_HWREG_K11_2_DATA + i * 8,
				xv_mix_rgb2yuv_coeffs[ColorStd][colorRange][i] \
				* scale_factor / XVMIX_CSC_COEFF_DIVISOR);

	for (i = XVMIX_CSC_MATRIX_SIZE; i < XVMIX_CSC_COEFF_SIZE; i++)
		XV_mix_WriteReg(MixPtr->Config.BaseAddress,
				XV_MIX_CTRL_ADDR_HWREG_K11_2_DATA + i * 8,
				xv_mix_rgb2yuv_coeffs[ColorStd][colorRange][i] * bpcScale);

}

/*****************************************************************************/
/*
 * This function calls Mixer coefficient programming functions
 *
 * @param  InstancePtr is a pointer to core instance to be worked upon
 * @param  colorStandard is used to calculate coefficients
 * @param  colorRange is used to calculate coefficients
 * @param  colorDepth is width of colour component
 *
 * @return XST_SUCCESS if command is successful else error code with reason
 *
 * @note   none
 *
 ******************************************************************************/
u32 XVMix_SetCscCoeffs(XV_Mix_l2 *InstancePtr,
		XVidC_ColorStd colorStandard,
		XVidC_ColorRange colorRange,
		u8 colorDepth)
{
	if ((colorStandard >= XVIDC_BT_NUM_SUPPORTED) ||
			(colorRange >= XVIDC_CR_NUM_SUPPORTED))
	{
		xil_printf("\n\rERROR: UnKnown colorStandard: %d\t colorRange: %d\n\r",
				colorStandard, colorRange);
		return XST_FAILURE;
	}

	if (XVMix_IsCscCoeffsRegsEnabled(InstancePtr)) {
		XVMix_SetCoeffForYuvToRgb(InstancePtr, colorStandard,
				colorRange, colorDepth);

		XVMix_SetCoeffForRgbToYuv(InstancePtr, colorStandard,
				colorRange, colorDepth);
	} else {
		xil_printf("\n\r ***ALERT: CSC Coefficient Registers Parameter "\
				"is Disabled in IP*** \n\r");
		return XST_NO_FEATURE;
	}

	return XST_SUCCESS;

}
/** @} */
