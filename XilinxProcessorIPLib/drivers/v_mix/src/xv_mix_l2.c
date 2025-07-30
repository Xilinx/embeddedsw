/******************************************************************************
* Copyright (C) 1986 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_mix_l2.c
* @addtogroup v_mix Overview
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
* 7.00  pg    07/29/25   Added Tile format Support.
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

#ifndef SDT
/**
 * XVMix_Initialize - Initialize the XV_Mix_l2 instance.
 *
 * This function initializes the XV_Mix_l2 instance specified by InstancePtr
 * using the hardware configuration identified by DeviceId. It first asserts
 * that the provided pointer is not NULL, clears the instance memory, and then
 * calls the lower-level XV_mix_Initialize function. If initialization is
 * successful, it sets the instance to its power-on default state.
 *
 * @param InstancePtr Pointer to the XV_Mix_l2 instance to be initialized.
 * @param DeviceId    Device ID of the hardware configuration to use.
 *
 * @return
 *   - XST_SUCCESS if initialization was successful.
 *   - Error code defined by XV_mix_Initialize otherwise.
 */
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
#else
/**
 * XVMix_Initialize - Initialize the XV_Mix_l2 instance for SDT flow.
 *
 * This function initializes the XV_Mix_l2 instance specified by InstancePtr
 * using the hardware configuration at the given BaseAddress. It first asserts
 * that the provided pointer is not NULL, clears the instance memory, and then
 * calls the lower-level XV_mix_Initialize function. If initialization is
 * successful, it sets the instance to its power-on default state.
 *
 * @param InstancePtr Pointer to the XV_Mix_l2 instance to be initialized.
 * @param BaseAddress Base address of the hardware configuration to use.
 *
 * @return
 *   - XST_SUCCESS if initialization was successful.
 *   - Error code defined by XV_mix_Initialize otherwise.
 */
int XVMix_Initialize(XV_Mix_l2 *InstancePtr, UINTPTR BaseAddress)
{
  int Status;
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Setup the instance */
  memset(InstancePtr, 0, sizeof(XV_Mix_l2));
  Status = XV_mix_Initialize(&InstancePtr->Mix, BaseAddress);

  if(Status == XST_SUCCESS) {
    SetPowerOnDefaultState(InstancePtr);
  }
  return(Status);
}
#endif

/**
 * @brief Sets the XV_Mix_l2 instance to its power-on default state.
 *
 * This function initializes the video mixer instance to a known default state
 * after power-on or reset. The following actions are performed:
 *   - Disables all video layers.
 *   - Sets up a default video stream configuration with custom mode, color format,
 *     frame rate, color depth, and active video dimensions based on the IP configuration.
 *   - Applies the default video stream to the mixer.
 *   - Sets the default background color to blue with the configured color depth.
 *   - For each layer, if scaling is disabled, updates the layer's maximum width to
 *     the IP's maximum width.
 *   - Enables only the master (stream) layer.
 *   - Disables interrupts to set up polling mode.
 *
 * @param InstancePtr Pointer to the XV_Mix_l2 instance to be initialized.
 *
 * @return None
 */
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

/**
 * Enables interrupts for the XV_Mix_l2 instance.
 *
 * This function performs the following actions:
 * 1. Asserts that the provided instance pointer is not NULL.
 * 2. Enables the DONE interrupt for the underlying XV_mix hardware.
 * 3. Globally enables interrupts for the XV_mix hardware.
 * 4. Disables the auto-restart feature of the XV_mix hardware.
 *
 * @param InstancePtr Pointer to the XV_Mix_l2 instance.
 *
 * @return none
 */
void XVMix_InterruptEnable(XV_Mix_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  /* Enable Interrupts */
  XV_mix_InterruptEnable(&InstancePtr->Mix, XVMIX_IRQ_DONE_MASK);
  XV_mix_InterruptGlobalEnable(&InstancePtr->Mix);

  /* Clear autostart bit */
  XV_mix_DisableAutoRestart(&InstancePtr->Mix);
}

/**
 * XVMix_InterruptDisable - Disables interrupts for the XV_Mix_l2 instance.
 *
 * This function disables the DONE interrupt and the global interrupt for the
 * specified XV_Mix_l2 instance. After disabling interrupts, it enables the
 * auto-restart feature to allow the hardware to automatically restart processing
 * without requiring software intervention.
 *
 * @param InstancePtr Pointer to the XV_Mix_l2 instance.
 *
 * @return none
 * @note The InstancePtr must not be NULL.
 */
void XVMix_InterruptDisable(XV_Mix_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  /* Disable Interrupts */
  XV_mix_InterruptDisable(&InstancePtr->Mix, XVMIX_IRQ_DONE_MASK);
  XV_mix_InterruptGlobalDisable(&InstancePtr->Mix);

  /* Set autostart bit */
  XV_mix_EnableAutoRestart(&InstancePtr->Mix);
}

/**
 * @brief
 *   Starts the video mixer hardware.
 *
 * This function asserts that the provided instance pointer is not NULL,
 * and then starts the underlying XV_mix hardware by calling its Start function.
 *
 * @param InstancePtr
 *   Pointer to the XV_Mix_l2 instance to be started.
 *
 * @return none
 */
void XVMix_Start(XV_Mix_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_mix_Start(&InstancePtr->Mix);
}

/**
 * Stops the XV_Mix_l2 core operation.
 *
 * This function disables the autostart feature, initiates a flush operation,
 * and waits for the flush to complete or until a timeout occurs.
 *
 * @param InstancePtr Pointer to the XV_Mix_l2 instance.
 *
 * The function performs the following steps:
 *   1. Asserts that the instance pointer is not NULL.
 *   2. Disables the autostart mode of the core.
 *   3. Sets the flush bit to initiate a flush operation.
 *   4. Waits for the flush operation to complete by polling the flush done status,
 *      with a timeout to prevent indefinite waiting.
 *   5. Returns early if the flush operation does not complete within the timeout.
 *
 * @return none
 */
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

/**
 * Checks if a given video window is valid within the bounds of a video stream,
 * considering a specified scaling factor.
 *
 * This function creates a copy of the provided window, applies the scaling
 * factor to its width and height, and then verifies whether the scaled window
 * fits within the active area of the video stream.
 *
 * @param Strm   Pointer to the video stream structure containing timing info.
 * @param Win    Pointer to the video window structure to validate.
 * @param Scale  Scaling factor to apply to the window dimensions.
 *
 * @return TRUE if the scaled window fits within the stream's active area,
 *         FALSE otherwise.
 */
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

/**
 * Sets the video stream configuration for the mixer instance.
 *
 * This function copies the provided video stream structure into the mixer instance,
 * and programs the hardware registers for the active video width and height.
 *
 * @param InstancePtr Pointer to the XV_Mix_l2 instance.
 * @param StrmIn      Pointer to the input XVidC_VideoStream structure.
 *
 * @return None.
 *
 * @note The function asserts that the input pointers are valid and that the
 *       resolution is within the supported range.
 */
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

         WinResInRange = ((Win->Width > (XVMIX_MIN_STRM_WIDTH-1)) &&
                          (Win->Height > (XVMIX_MIN_STRM_HEIGHT-1)) &&
                          (Win->Width <= MixPtr->Config.LayerMaxWidth[LayerId-1]) &&
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

/**
* This function reads the color format of the specified layer
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer to be updated
* @param  Cfmt is pointer to color format return variable
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   none
*
**/
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
* @note   Applicable only for Layer1-16
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
* @note   Applicable only for Layer1-16
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
* for the U plane for 3-planar formats
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  LayerId is the layer to be updated
* @param  Addr is the absolute address of second buffer in memory
*
* @return XST_SUCCESS or XST_FAILURE
*
* @note   Applicable only for Layer1-16
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
* for the UV plane for semi-planar formats and
* for the U plane for 3-planar formats
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

/**
 * Set the chroma buffer address for the specified layer.
 *
 * This function sets the address of the second chroma buffer for a given layer in the
 * XV_Mix_l2 instance. It checks for valid input parameters, ensures the address is properly
 * aligned according to the AXI memory interface requirements, and writes the address to the
 * appropriate hardware register. The function updates the internal state of the instance
 * with the new buffer address.
 *
 * @param InstancePtr Pointer to the XV_Mix_l2 instance.
 * @param LayerId     ID of the layer for which the chroma buffer address is to be set.
 *                    Must be greater than XVMIX_LAYER_MASTER and less than XVMIX_LAYER_LOGO.
 * @param Addr        Physical address of the chroma buffer. Must be non-zero and properly aligned.
 *
 * @return
 *   - XST_SUCCESS if the address was set successfully.
 *   - XVMIX_ERR_MEM_ADDR_MISALIGNED if the address is not properly aligned.
 *   - XST_FAILURE for other failures (e.g., invalid parameters).
 *
 * @note
 *   The address must be aligned to (2 * PixPerClk * 4) bytes.
 *   The function does not perform any memory allocation or buffer management.
 */
int XVMix_SetLayerChromaBuffer2Addr(XV_Mix_l2 *InstancePtr,
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
        BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF3_V_DATA;

        XV_mix_WriteReg(MixPtr->Config.BaseAddress,
                        (BaseReg+((LayerId-1)*XVMIX_REG_OFFSET)), Addr);

        InstancePtr->Layer[LayerId].ChromaBufAddr = Addr;
        Status = XST_SUCCESS;
      }
  }
  return(Status);
}

/**
 * Retrieves the address of the second chroma buffer
 *                                   for a specified layer in the video mixer instance.
 *
 * @param InstancePtr: Pointer to the XV_Mix_l2 instance.
 * @param LayerId:      Identifier of the layer for which the chroma buffer address is requested.
 *                      Must be greater than XVMIX_LAYER_MASTER and less than XVMIX_LAYER_LOGO.
 *
 * @return The address (UINTPTR) of the second chroma buffer for the specified layer.
 *         Returns 0 if the LayerId is invalid or out of range.
 *
 * @note   The function asserts that InstancePtr is not NULL and that LayerId is within
 *         the valid range. The function only returns a valid address if LayerId is less
 *         than the number of layers configured in the instance.
 */
UINTPTR XVMix_GetLayerChromaBuffer2Addr(XV_Mix_l2 *InstancePtr,
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
        BaseReg = XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF3_V_DATA;

        ReadVal = XV_mix_ReadReg(MixPtr->Config.BaseAddress,
                                 (BaseReg+((LayerId-1)*XVMIX_REG_OFFSET)));
  }
  return(ReadVal);
}

/**
 * Set the logo color key values for the XV_Mix_l2 instance.
 *
 * This function configures the minimum and maximum RGB values for the logo color key,
 * which are used to enable color keying (transparency) for the logo overlay in the video mixer.
 * The function only sets the color key values if both the logo and logo color key features
 * are enabled in the instance.
 *
 * @param InstancePtr   Pointer to the XV_Mix_l2 instance.
 * @param ColorKeyData  Structure containing the minimum and maximum RGB values for the color key.
 *
 * @return
 *   - XST_SUCCESS if the color key values were set successfully.
 *   - XST_FAILURE if the logo or logo color key feature is not enabled.
 */
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

/**
 * Retrieves the current logo color key range from the hardware.
 *
 * @param InstancePtr   Pointer to the XV_Mix_l2 instance.
 * @param ColorKeyData  Pointer to an XVMix_LogoColorKey structure where the color key
 *                 minimum and maximum RGB values will be stored.
 *
 * This function checks if the logo and logo color key features are enabled.
 * If both are enabled, it reads the minimum and maximum RGB values for the logo
 * color key from the hardware registers and stores them in the provided
 * ColorKeyData structure.
 *
 * @return  XST_SUCCESS if the color key values were successfully retrieved,
 *          XST_FAILURE otherwise.
 */
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

/**
 * Loads a logo image into the mixer hardware.
 *
 * This function takes separate R, G, and B buffers representing a logo image,
 * and writes the pixel data into the hardware registers of the video mixer
 * for display as a logo overlay. The function supports logos with dimensions
 * and alignment constraints as specified by the hardware configuration.
 *
 * @param InstancePtr Pointer to the XV_Mix_l2 instance.
 * @param Win         Pointer to an XVidC_VideoWindow structure specifying
 *                    the position and size of the logo window.
 * @param RBuffer     Pointer to the buffer containing the red channel data.
 * @param GBuffer     Pointer to the buffer containing the green channel data.
 * @param BBuffer     Pointer to the buffer containing the blue channel data.
 *
 * @return
 *   - XST_SUCCESS if the logo was loaded successfully.
 *   - XST_FAILURE if the logo could not be loaded (e.g., invalid parameters or logo not enabled).
 *
 * Preconditions:
 *   - The logo feature must be enabled in the mixer hardware.
 *   - The window dimensions and alignment must meet hardware requirements.
 *   - The input buffers must be valid and sized appropriately for the window.
 *
 * Side Effects:
 *   - Writes logo pixel data to hardware registers.
 *   - Updates the logo layer buffer pointers in the instance.
 */
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

/**
 * XVMix_LoadLogoPixelAlpha - Loads per-pixel alpha values for a logo window into the hardware.
 *
 * @param InstancePtr  Pointer to the XV_Mix_l2 instance.
 * @param Win          Pointer to the XVidC_VideoWindow structure specifying the logo window position and size.
 * @param ABuffer      Pointer to the buffer containing alpha values for each pixel in the logo window.
 *
 * This function loads the per-pixel alpha values from the provided buffer into the hardware registers
 * for the specified logo window. The function checks that the logo and per-pixel alpha features are enabled,
 * and that the window parameters are within valid ranges. The alpha values are packed into 32-bit words
 * (4 pixels per word) and written to the hardware.
 *
 * @return  XST_SUCCESS if the operation is successful, XST_FAILURE otherwise.
 */
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

/**
 * XVMix_DbgReportStatus - Prints the current status and configuration of the mixer instance.
 *
 * @param InstancePtr Pointer to the XV_Mix_l2 instance to report status for.
 *
 * This function outputs detailed information about the mixer hardware and its configuration
 * to the standard output using xil_printf. The reported information includes:
 *   - Pixels per clock
 *   - Color depth
 *   - Number of layers
 *   - Control register value
 *   - Layer enable register value
 *   - Enable/disable status of each layer (Master, Layer 1-N, Logo)
 *   - Background color values (Y/R, U/G, V/B)
 *
 * The function asserts that the provided instance pointer is not NULL.
 * It is intended for debugging and diagnostic purposes.
 *
 * @return None.
 */
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

  IsEnabled = XVMix_IsTileFormatEnabled(InstancePtr);
  xil_printf("Tile Format : %s\r\n\r\n", Status[IsEnabled]);
}

/**
 * Prints debug information for a specific layer in the video mixer.
 *
 * @param InstancePtr Pointer to the XV_Mix_l2 instance.
 * @param LayerId     Identifier of the layer to report (master, logo, or regular layer).
 *
 * This function prints detailed status and configuration information for the specified
 * layer in the video mixer. The information includes:
 *   - Whether the layer is enabled or disabled.
 *   - For the master layer: color format, resolution, and stream info.
 *   - For the logo layer: alpha, scale factor, window position/size, color key data,
 *     and pixel alpha status.
 *   - For regular layers: interface type (memory/stream), buffer addresses, alpha and
 *     scaling status/values, color format, window position/size, and stride.
 *
 * The function uses xil_printf for output and is intended for debugging purposes.
 * Assertions are used to validate input parameters.
 *
 * @return none
 *
 * @note   none
 */
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

/**
 * @brief Sets the coefficients for YUV to RGB color space conversion in the XV_Mix hardware.
 *
 * This function configures the color space conversion (CSC) matrix coefficients
 * for converting YUV to RGB based on the specified color standard, color range,
 * and color depth. The coefficients are written to the hardware registers of the
 * XV_Mix instance.
 *
 * @param InstancePtr Pointer to the XV_Mix_l2 instance.
 * @param ColorStd    The color standard to use (e.g., BT.601, BT.709).
 * @param colorRange  The color range (e.g., full range, limited range).
 * @param colorDepth  The color depth in bits per component (e.g., 8, 10, 12).
 *
 * @return none
 *
 * @note   Applicable for all 16 Layers
 *
 ***/
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
				(int) ((float)xv_mix_yuv2rgb_coeffs[ColorStd][colorRange][i] \
				/ (float)XVMIX_CSC_COEFF_DIVISOR * (float)scale_factor));

	for (i = XVMIX_CSC_MATRIX_SIZE; i < XVMIX_CSC_COEFF_SIZE; i++)
		XV_mix_WriteReg(MixPtr->Config.BaseAddress,
				XV_MIX_CTRL_ADDR_HWREG_K11_DATA + i * 8,
				xv_mix_yuv2rgb_coeffs[ColorStd][colorRange][i] * bpcScale);

}

/**
 * @brief Sets the RGB to YUV color space conversion coefficients for the mixer instance.
 *
 * This function configures the color space conversion (CSC) matrix coefficients
 * in the hardware registers of the mixer to perform RGB to YUV conversion.
 * The coefficients are selected based on the specified color standard, color range,
 * and color depth. The function writes the scaled coefficients to the appropriate
 * hardware registers.
 *
 * @param InstancePtr Pointer to the XV_Mix_l2 instance.
 * @param ColorStd    The color standard to use (e.g., BT.601, BT.709).
 * @param colorRange  The color range (e.g., full range, limited range).
 * @param colorDepth  The color depth (bits per component).
 */
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
				(int) ((float)xv_mix_rgb2yuv_coeffs[ColorStd][colorRange][i] \
				/ (float)XVMIX_CSC_COEFF_DIVISOR * (float)scale_factor));

	for (i = XVMIX_CSC_MATRIX_SIZE; i < XVMIX_CSC_COEFF_SIZE; i++)
		XV_mix_WriteReg(MixPtr->Config.BaseAddress,
				XV_MIX_CTRL_ADDR_HWREG_K11_2_DATA + i * 8,
				xv_mix_rgb2yuv_coeffs[ColorStd][colorRange][i] * bpcScale);

}

/**
 * Sets the Color Space Conversion (CSC) coefficients for the XV_Mix_l2 instance.
 *
 * This function configures the CSC coefficients based on the specified color standard,
 * color range, and color depth. It first checks if the provided color standard and color range
 * are supported. If the CSC coefficient registers are enabled in the hardware, it sets the
 * coefficients for both YUV-to-RGB and RGB-to-YUV conversions. If the registers are not enabled,
 * it returns an error indicating that the feature is not available.
 *
 * @param	InstancePtr	Pointer to the XV_Mix_l2 instance.
 * @param	colorStandard	Specifies the color standard (e.g., BT.601, BT.709).
 * @param	colorRange	Specifies the color range (e.g., full, limited).
 * @param	colorDepth	Specifies the color depth (in bits).
 *
 * @return
 *		- XST_SUCCESS if the coefficients are set successfully.
 *		- XST_FAILURE if an unknown color standard or color range is provided.
 *		- XST_NO_FEATURE if the CSC coefficient registers are not enabled.
 */
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
