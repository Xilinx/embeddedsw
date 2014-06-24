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
* @file rgb2ycrcb.c
*
* This is main code of Xilinx RGB to YCrCb Color Space Converter (RGB2YCRCB)
* device driver. Please see rgb2ycrcb.h for more details of the driver.
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.00a tb   02/27/12 Updated for RGB2YCRCB v5.00.a 
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "rgb2ycrcb.h"
#include "xenv.h"
#include "stdio.h"


/*****************************************************************************/
// Note: Most of the functions are currently implemented as high-performance 
// macros within rgb2ycrcb.h
/*****************************************************************************/

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
* @param data_width has a valid range of [8,10,12,16]
* @param coef_in is a pointer to a rgb_coef_inputs data structure.
*
* @return   None.
*
* @note
*
******************************************************************************/
void RGB_select_standard(u32 standard_sel, u32 input_range, u32 data_width, struct rgb_coef_inputs *coef_in)
{
  double acoef[4][3] = {{0.299, 0.299,  0.2568}, {0.299, 0.299,  0.2568}, {0.2126, 0.2126, 0.1819}, {0.299,    0.299,    0.299}};
  double bcoef[4][3] = {{0.114, 0.114,  0.0979}, {0.114, 0.114,  0.0979}, {0.0722, 0.0722, 0.0618}, {0.114,    0.114,    0.114}};
  double ccoef[4][3] = {{0.713, 0.7295, 0.5910}, {0.713, 0.7295, 0.5910}, {0.6350, 0.6495, 0.6495}, {0.877283, 0.877283, 0.877283}};
  double dcoef[4][3] = {{0.564, 0.5772, 0.5772}, {0.564, 0.5772, 0.5772}, {0.5389, 0.5512, 0.5512}, {0.492111, 0.492111, 0.492111}};
  u32 yoffset = 1<<(data_width-4);
  u32 coffset = 1<<(data_width-1);
  u32 max[3] = {(240*(1<<(data_width-8))), (235*(1<<(data_width-8))), ((1<<data_width)-1)};
  u32 min[3] = { (16*(1<<(data_width-8))),  (16*(1<<(data_width-8))), 0};

  coef_in->acoef = acoef[standard_sel][input_range];
  coef_in->bcoef = bcoef[standard_sel][input_range];
  coef_in->ccoef = ccoef[standard_sel][input_range];
  coef_in->dcoef = dcoef[standard_sel][input_range];
  coef_in->yoffset  = yoffset;
  coef_in->cboffset = coffset;
  coef_in->croffset = coffset;
  coef_in->ymax = max[input_range];
  coef_in->ymin = min[input_range];
  coef_in->cbmax = max[input_range];
  coef_in->cbmin = min[input_range];
  coef_in->crmax = max[input_range];
  coef_in->crmin = min[input_range];
}


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
u32 RGB_coefficient_translation(struct rgb_coef_inputs *coef_in, struct rgb_coef_outputs *coef_out, u32 data_width)
{
  u32 ret_val = 0;

  if((coef_in->acoef + coef_in->bcoef) > 1.0) {
    printf("WARNING: Acoef (%1f) + Bcoef (%1f) can not be more then 1.0\r\n",coef_in->acoef, coef_in->bcoef);
    ret_val = ret_val | 0x1;
  }
  if(coef_in->yoffset < -(1<<data_width) || coef_in->yoffset > (1<<data_width)-1) {
    printf("WARNING: Y Offset (%u) is outside the data width range [%u, %u]\r\n",
               (unsigned int)coef_in->yoffset, (unsigned int)-(1<<data_width), (unsigned int)(1<<data_width)-1);
    ret_val = ret_val | 0x2;
  }
  if(coef_in->cboffset < -(1<<data_width) || coef_in->cboffset > (1<<data_width)-1) {
    printf("WARNING: Cb Offset (%u) is outside the data width range [%u, %u]\r\n",
               (unsigned int)coef_in->cboffset, (unsigned int)-(1<<data_width), (unsigned int)(1<<data_width)-1);
    ret_val = ret_val | 0x4;
  }
  if(coef_in->croffset < -(1<<data_width) || coef_in->croffset > (1<<data_width)-1) {
    printf("WARNING: Cr Offset (%u) is outside the data width range [%u, %u]\r\n",
               (unsigned int)coef_in->croffset, (unsigned int)-(1<<data_width), (unsigned int)(1<<data_width)-1);
    ret_val = ret_val | 0x8;
  }
  if(coef_in->ymax < 0 || coef_in->ymax > (1<<data_width)-1) {
    printf("WARNING: Y Max (%u) is outside the data width range [0, %u]\r\n",
               (unsigned int)coef_in->ymax, (unsigned int)(1<<data_width)-1);
    ret_val = ret_val | 0x10;
  }
  if(coef_in->ymin < 0 || coef_in->ymin > (1<<data_width)-1) {
    printf("WARNING: Y Min (%u) is outside the data width range [0, %u]\r\n",
               (unsigned int)coef_in->ymin, (unsigned int)(1<<data_width)-1);
    ret_val = ret_val | 0x20;
  }
  if(coef_in->cbmax < 0 || coef_in->cbmax > (1<<data_width)-1) {
    printf("WARNING: Cb Max (%u) is outside the data width range [0, %u]\r\n",
               (unsigned int)coef_in->cbmax, (unsigned int)(1<<data_width)-1);
    ret_val = ret_val | 0x40;
  }
  if(coef_in->cbmin < 0 || coef_in->cbmin > (1<<data_width)-1) {
    printf("WARNING: Cb Min (%u) is outside the data width range [0, %u]\r\n",
               (unsigned int)coef_in->cbmin, (unsigned int)(1<<data_width)-1);
    ret_val = ret_val | 0x80;
  }
  if(coef_in->crmax < 0 || coef_in->crmax > (1<<data_width)-1) {
    printf("WARNING: Cr Max (%u) is outside the data width range [0, %u]\r\n",
               (unsigned int)coef_in->crmax, (unsigned int)(1<<data_width)-1);
    ret_val = ret_val | 0x100;
  }
  if(coef_in->crmin < 0 || coef_in->crmin > (1<<data_width)-1) {
    printf("WARNING: Cr Min (%u) is outside the data width range [0, %u]\r\n",
               (unsigned int)coef_in->crmin, (unsigned int)(1<<data_width)-1);
    ret_val = ret_val | 0x200;
  }

  coef_out->acoef = coef_in->acoef * (1<<16);
  coef_out->bcoef = coef_in->bcoef * (1<<16);
  coef_out->ccoef = coef_in->ccoef * (1<<16);
  coef_out->dcoef = coef_in->dcoef * (1<<16);
  coef_out->yoffset  = coef_in->yoffset;
  coef_out->cboffset = coef_in->cboffset;
  coef_out->croffset = coef_in->croffset;
  coef_out->ymax  = coef_in->ymax;
  coef_out->ymin  = coef_in->ymin;
  coef_out->cbmax = coef_in->cbmax;
  coef_out->cbmin = coef_in->cbmin;
  coef_out->crmax = coef_in->crmax;
  coef_out->crmin = coef_in->crmin;

  return ret_val;
}

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
void RGB_set_coefficients(u32 BaseAddress, struct rgb_coef_outputs *coef_out)
{
    RGB_WriteReg(BaseAddress, RGB_ACOEF, coef_out->acoef);       //ACOEF
    RGB_WriteReg(BaseAddress, RGB_BCOEF, coef_out->bcoef);       //BCOEF
    RGB_WriteReg(BaseAddress, RGB_CCOEF, coef_out->ccoef);       //CCOEF
    RGB_WriteReg(BaseAddress, RGB_DCOEF, coef_out->dcoef);       //DCOEF
    RGB_WriteReg(BaseAddress, RGB_YOFFSET,  coef_out->yoffset);  //YOFFSET
    RGB_WriteReg(BaseAddress, RGB_CBOFFSET, coef_out->cboffset); //CBOFFSET
    RGB_WriteReg(BaseAddress, RGB_CROFFSET, coef_out->croffset); //CROFFSET
    RGB_WriteReg(BaseAddress, RGB_YMAX, coef_out->ymax);         //YMAX
    RGB_WriteReg(BaseAddress, RGB_YMIN, coef_out->ymin);         //YMIN
    RGB_WriteReg(BaseAddress, RGB_CBMAX,coef_out->cbmax);        //CBMAX
    RGB_WriteReg(BaseAddress, RGB_CBMIN, coef_out->cbmin);       //CBMIN
    RGB_WriteReg(BaseAddress, RGB_CRMAX, coef_out->crmax);       //CRMAX
    RGB_WriteReg(BaseAddress, RGB_CRMIN, coef_out->crmin);       //CRMIN

}

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
void RGB_get_coefficients(u32 BaseAddress, struct rgb_coef_outputs *coef_out)
{
  coef_out->acoef = RGB_ReadReg(BaseAddress, RGB_ACOEF); 
  coef_out->bcoef = RGB_ReadReg(BaseAddress, RGB_BCOEF); 
  coef_out->ccoef = RGB_ReadReg(BaseAddress, RGB_CCOEF); 
  coef_out->dcoef = RGB_ReadReg(BaseAddress, RGB_DCOEF);

  coef_out->yoffset  = RGB_ReadReg(BaseAddress, RGB_YOFFSET);
  coef_out->cboffset = RGB_ReadReg(BaseAddress, RGB_CBOFFSET); 
  coef_out->croffset = RGB_ReadReg(BaseAddress, RGB_CROFFSET);

  coef_out->ymax = RGB_ReadReg(BaseAddress, RGB_YMAX); 
  coef_out->ymin = RGB_ReadReg(BaseAddress, RGB_YMIN);

  coef_out->cbmax = RGB_ReadReg(BaseAddress, RGB_CBMAX);
  coef_out->cbmax = RGB_ReadReg(BaseAddress, RGB_CBMIN);

  coef_out->crmax = RGB_ReadReg(BaseAddress, RGB_CRMAX);
  coef_out->crmax = RGB_ReadReg(BaseAddress, RGB_CRMIN);

}
