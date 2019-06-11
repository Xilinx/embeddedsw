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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xv_letterbox_l2.c
* @addtogroup v_letterbox_v2_1
* @{
* @details
*
* The Letterbox Layer-2 Driver.
* The functions in this file provides an abstraction from the register peek/poke
* methodology by implementing most common use-case provided by the sub-core.
* See xv_letterbox_l2.h for a detailed description of the layer-2 driver
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco   07/21/15   Initial Release
* 2.00  rco   11/05/15   Integrate layer-1 with layer-2
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "xv_letterbox_l2.h"

/************************** Constant Definitions *****************************/
/* Pixel definition in 8 bit resolution in YUV color space*/
const u8 bkgndColorYUV[XLBOX_BKGND_LAST][3] =
{
  {  0, 128, 128}, //Black
  {255, 128, 128}, //White
  { 76,  85, 255}, //Red
  {149,  43,  21}, //Green
  { 29, 255, 107}  //Blue
};

/* Pixel map in RGB color space*/
const u8 bkgndColorRGB[XLBOX_BKGND_LAST][3] =
{
  {0, 0, 0}, //Black
  {1, 1, 1}, //White
  {1, 0, 0}, //Red
  {0, 1, 0}, //Green
  {0, 0, 1}  //Blue
};

/**************************** Type Definitions *******************************/

/**************************** Local Global *******************************/

/************************** Function Prototypes ******************************/

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
int XV_LBoxInitialize(XV_Lbox_l2 *InstancePtr, u16 DeviceId)
{
  int Status;
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Setup the instance */
  memset(InstancePtr, 0, sizeof(XV_Lbox_l2));
  Status = XV_letterbox_Initialize(&InstancePtr->Lbox, DeviceId);

  return(Status);
}

/*****************************************************************************/
/**
* This function starts the letter box core
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_LBoxStart(XV_Lbox_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_letterbox_EnableAutoRestart(&InstancePtr->Lbox);
  XV_letterbox_Start(&InstancePtr->Lbox);
}

/*****************************************************************************/
/**
* This function stops the letter box core
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_LBoxStop(XV_Lbox_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_letterbox_DisableAutoRestart(&InstancePtr->Lbox);
}

/*****************************************************************************/
/**
* This function configures the letterbox active window. All pixels within the
* window are passed to the output as-is. Any pixel outside the defined window
* will be clamped to the programmed background color
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  ActiveWindow is structure that contains window coordinates and size
* @param  FrameWidth is the input stream width
* @param  FrameHeight is the input stream height
*
* @return None
*
******************************************************************************/
void XV_LBoxSetActiveWin(XV_Lbox_l2 *InstancePtr,
                         XVidC_VideoWindow *ActiveWindow,
                         u32 FrameWidth,
                         u32 FrameHeight)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_letterbox_Set_HwReg_col_start(&InstancePtr->Lbox, ActiveWindow->StartX);
  XV_letterbox_Set_HwReg_col_end(&InstancePtr->Lbox,
                ActiveWindow->StartX+ActiveWindow->Width);
  XV_letterbox_Set_HwReg_row_start(&InstancePtr->Lbox, ActiveWindow->StartY);
  XV_letterbox_Set_HwReg_row_end(&InstancePtr->Lbox,
                ActiveWindow->StartY+ActiveWindow->Height);

  XV_letterbox_Set_HwReg_height(&InstancePtr->Lbox, FrameHeight);
  XV_letterbox_Set_HwReg_width(&InstancePtr->Lbox,  FrameWidth);
}

/*****************************************************************************/
/**
* This function configures the background color to be painted outside active
* window
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  ColorId is the background color requested
* @param  cfmt is the color format of the input stream
* @param  bpc is the color depth (bits per channel)
*
* @return None
*
******************************************************************************/
void XV_LboxSetBackgroundColor(XV_Lbox_l2    *InstancePtr,
                                XLboxColorId ColorId,
                                XVidC_ColorFormat cfmt,
                                XVidC_ColorDepth  bpc)
{
  u16 Cr_b_val,y_r_val,Cb_g_val;
  u16 scale;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((bpc >= XVIDC_BPC_8) &&
                 (bpc <= InstancePtr->Lbox.Config.MaxDataWidth))

  if(cfmt == XVIDC_CSF_RGB)
  {
    scale = ((1<<bpc)-1);
    y_r_val  = bkgndColorRGB[ColorId][0] * scale;
    Cb_g_val = bkgndColorRGB[ColorId][1] * scale;
    Cr_b_val = bkgndColorRGB[ColorId][2] * scale;
  }
  else //YUV
  {
    scale =  (1<<(bpc-XVIDC_BPC_8));
    y_r_val  = bkgndColorYUV[ColorId][0] * scale;
    Cb_g_val = bkgndColorYUV[ColorId][1] * scale;
    Cr_b_val = bkgndColorYUV[ColorId][2] * scale;
  }

  /* Set video format */
  XV_letterbox_Set_HwReg_video_format(&InstancePtr->Lbox, cfmt);

  /* Set Background color (outside window) */
  XV_letterbox_Set_HwReg_Y_R_value(&InstancePtr->Lbox,  y_r_val);
  XV_letterbox_Set_HwReg_Cb_G_value(&InstancePtr->Lbox, Cb_g_val);
  XV_letterbox_Set_HwReg_Cr_B_value(&InstancePtr->Lbox, Cr_b_val);
}


/*****************************************************************************/
/**
*
* This function prints LBox IP status on the console
*
* @param	InstancePtr is the pointer to the core instance.
*
* @return	None
*
******************************************************************************/
void XV_LBoxDbgReportStatus(XV_Lbox_l2 *InstancePtr)
{
  XV_letterbox *pLbox = &InstancePtr->Lbox;
  u32 done, idle, ready, ctrl;
  u32 colstart, colend, rowstart, rowend;
  u32 yr,cbg,crb, cfmt, width, height;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n\r\n----->LBOX IP STATUS<----\r\n");

  done  = XV_letterbox_IsDone(pLbox);
  idle  = XV_letterbox_IsIdle(pLbox);
  ready = XV_letterbox_IsReady(pLbox);
  ctrl  = XV_letterbox_ReadReg(pLbox->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_AP_CTRL);

  colstart = XV_letterbox_Get_HwReg_col_start(pLbox);
  colend   = XV_letterbox_Get_HwReg_col_end(pLbox);
  rowstart = XV_letterbox_Get_HwReg_row_start(pLbox);
  rowend   = XV_letterbox_Get_HwReg_row_end(pLbox);
  yr       = XV_letterbox_Get_HwReg_Y_R_value(pLbox);
  cbg      = XV_letterbox_Get_HwReg_Cb_G_value(pLbox);
  crb      = XV_letterbox_Get_HwReg_Cr_B_value(pLbox);
  height   = XV_letterbox_Get_HwReg_height(pLbox);
  width    = XV_letterbox_Get_HwReg_width(pLbox);
  cfmt     = XV_letterbox_Get_HwReg_video_format(pLbox);


  xil_printf("IsDone:  %d\r\n", done);
  xil_printf("IsIdle:  %d\r\n", idle);
  xil_printf("IsReady: %d\r\n", ready);
  xil_printf("Ctrl:    0x%x\r\n\r\n", ctrl);

  xil_printf(" Window Start X : %d\r\n",colstart);
  xil_printf(" Window End   X : %d\r\n",colend);
  xil_printf(" Window Start Y : %d\r\n",rowstart);
  xil_printf(" Window End   Y : %d\r\n",rowend);
  xil_printf(" Frame Width    : %d\r\n",width);
  xil_printf(" Frame Height   : %d\r\n",height);
  xil_printf(" Color Format   : %d\r\n",cfmt);
  xil_printf(" Bkgnd Color Y/R: %d\r\n",yr);
  xil_printf(" Bkgnd Color U/G: %d\r\n",cbg);
  xil_printf(" Bkgnd Color V/B: %d\r\n",crb);
}
/** @} */
