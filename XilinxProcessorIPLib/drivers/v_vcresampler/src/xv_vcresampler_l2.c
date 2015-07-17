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
* @file xv_vcresampler_l2.c
* @addtogroup v_vcresampler_v1_0
* @{
* @details
*
* The Vertical Chroma Resampler Layer-2 Driver.
* The functions in this file provides an abstraction from the register peek/poke
* methodology by implementing most common use-case provided by the sub-core.
* See xv_vvcresampler_l2.h for a detailed description of the layer-2 driver
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco   07/21/15   Initial Release

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_vcresampler_l2.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Local Global *******************************/

/************************** Function Prototypes ******************************/
static void vcrUpdateCoefficients(XV_vcresampler *pVCrsmplr, u32 coeff[2][4]);

/*****************************************************************************/
/**
 * This function starts the Chroma resampler core
 *
 * @param  InstancePtr is a pointer to the core instance to be worked on.
 *
 * @return None
 *
 *****************************************************************************/
void XV_VCrsmplStart(XV_vcresampler *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_vcresampler_EnableAutoRestart(InstancePtr);
  XV_vcresampler_Start(InstancePtr);
}

/*****************************************************************************/
/**
 * This function stops the Chroma resampler core
 *
 * @param  InstancePtr is a pointer to the core instance to be worked on.
 *
 * @return None
 *
 *****************************************************************************/
void XV_VCrsmplStop(XV_vcresampler *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_vcresampler_DisableAutoRestart(InstancePtr);
}

/*****************************************************************************/
/**
* This function configures the Chroma resampler active resolution
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  width is the active frame width
* @param  height is the active frame height
*
* @return None
*
******************************************************************************/
void XV_VCrsmplSetActiveSize(XV_vcresampler *InstancePtr,
                              u32             width,
                              u32             height)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_vcresampler_Set_HwReg_width(InstancePtr,  width);
  XV_vcresampler_Set_HwReg_height(InstancePtr, height);
}

/*****************************************************************************/
/**
* This function configures the Chroma resampler for the required format
* conversion
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  formatIn is the input chroma format
* @param  formatOut is required chroma format
*
* @return None
*
******************************************************************************/
void XV_VCrsmplSetFormat(XV_vcresampler *InstancePtr,
                         XVidC_ColorFormat formatIn,
                         XVidC_ColorFormat formatOut)
{
  u32 fmtIn, fmtOut;
  u32 K[2][4] = {{0}};

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  /* Temp: Resampler IP YUV420 has 2 modes with distinct indexes. The difference is
   * in how the data is packed into the AXIS stream. HDMI IP packing is different
   * than standard AXIS definition. Ideally HDMI should convert and repack it in the
   * standard format, but until update is available the sub-cores that work with
   * YUV420 will have 2 modes to support
   * 		- YUV420_AXIS	= 3
   * 		- YUV420_HDMI	= 4
   *
   * 	Video Common driver has only YUV420_AXIS mode defined at index 3
   * 	VPRD beta release, is tightly coupled with HDMI IP, hence fow now the 420 mode
   * 	is mapped to YUV420_HDMI (4)
   */

  fmtIn  = ((formatIn  == XVIDC_CSF_YCRCB_420) ? (XVIDC_CSF_YCRCB_420+1) : formatIn);
  fmtOut = ((formatOut == XVIDC_CSF_YCRCB_420) ? (XVIDC_CSF_YCRCB_420+1) : formatOut);

  XV_vcresampler_Set_HwReg_input_video_format(InstancePtr,  fmtIn);
  XV_vcresampler_Set_HwReg_output_video_format(InstancePtr, fmtOut);

  if((formatIn == XVIDC_CSF_YCRCB_420) &&
     (formatOut == XVIDC_CSF_YCRCB_422))
  {
    K[0][0] = 0;
    K[0][1] = 4096;
    K[0][2] = 0;
    K[0][3] = 0;
    K[1][0] = 506;
    K[1][1] = 1542;
    K[1][2] = 1542;
    K[1][3] = 506;
  }
  else if((formatIn == XVIDC_CSF_YCRCB_422) &&
        (formatOut == XVIDC_CSF_YCRCB_420))

  {
    K[0][0] = 0;
    K[0][1] = 1024;
    K[0][2] = 2048;
    K[0][3] = 1024;
    K[1][0] = 0;
    K[1][1] = 0;
    K[1][2] = 0;
    K[1][3] = 0;
  }

  vcrUpdateCoefficients(InstancePtr, K);
}

/*****************************************************************************/
/**
* This function updates the core registers with computed coefficients for
* required conversion
*
* @param  pVCrsmplr is a pointer to the core instance to be worked on.
* @param  coeff is the array holding computed coefficients
*
* @return None
*
******************************************************************************/
static void vcrUpdateCoefficients(XV_vcresampler *pVCrsmplr, u32 coeff[2][4])
{
  XV_vcresampler_Set_HwReg_coefs_0_0(pVCrsmplr, coeff[0][0]);
  XV_vcresampler_Set_HwReg_coefs_0_1(pVCrsmplr, coeff[0][1]);
  XV_vcresampler_Set_HwReg_coefs_0_2(pVCrsmplr, coeff[0][2]);
  XV_vcresampler_Set_HwReg_coefs_0_3(pVCrsmplr, coeff[0][3]);
  XV_vcresampler_Set_HwReg_coefs_1_0(pVCrsmplr, coeff[1][0]);
  XV_vcresampler_Set_HwReg_coefs_1_1(pVCrsmplr, coeff[1][1]);
  XV_vcresampler_Set_HwReg_coefs_1_2(pVCrsmplr, coeff[1][2]);
  XV_vcresampler_Set_HwReg_coefs_1_3(pVCrsmplr, coeff[1][3]);
}

/*****************************************************************************/
/**
*
* This function prints Chroma Resampler status on the console
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_VCrsmplDbgReportStatus(XV_vcresampler *InstancePtr)
{
  XV_vcresampler *pVCrsmplr = InstancePtr;
  u32 done, idle, ready, ctrl;
  u32 vidfmtIn, vidfmtOut, height, width;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n\r\n----->V Chroma Resampler IP STATUS<----\r\n");

  done  = XV_vcresampler_IsDone(pVCrsmplr);
  idle  = XV_vcresampler_IsIdle(pVCrsmplr);
  ready = XV_vcresampler_IsReady(pVCrsmplr);
  ctrl  = XV_vcresampler_ReadReg(pVCrsmplr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL);

  vidfmtIn  = XV_vcresampler_Get_HwReg_input_video_format(pVCrsmplr);
  vidfmtOut = XV_vcresampler_Get_HwReg_output_video_format(pVCrsmplr);
  height    = XV_vcresampler_Get_HwReg_height(pVCrsmplr);
  width     = XV_vcresampler_Get_HwReg_width(pVCrsmplr);

  xil_printf("IsDone:  %d\r\n", done);
  xil_printf("IsIdle:  %d\r\n", idle);
  xil_printf("IsReady: %d\r\n", ready);
  xil_printf("Ctrl:    0x%x\r\n\r\n", ctrl);

  xil_printf("Video Format In:  %d\r\n", vidfmtIn);
  xil_printf("Video Format Out: %d\r\n", vidfmtOut);
  xil_printf("Width:            %d\r\n", width);
  xil_printf("Height:           %d\r\n", height);
}
/** @} */
