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
* This is main code of Xilinx YCrCb to RGB Color Space Converter (YCRCB2RGB)
* device driver. Please see ycrcb2rgb.h for more details of the driver.
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.00a tb   02/28/12 Updated for YCRCB2RGB v5.00.a 
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "ycrcb2rgb.h"
#include "xenv.h"
#include "stdio.h"

double ycc_pow2(u32 a) {  return( (a>0) ? (1 << a) : 1.0/(1 << (-a)) ); }


/*****************************************************************************/
// Note: Most of the functions are currently implemented as high-performance 
// macros within ycrcb2rgb.h
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
* @param coef_in is a pointer to a ycc_coef_inputs data structure.
*
* @return   None.
*
* @note
*
******************************************************************************/
void YCC_select_standard(u32 standard_sel, u32 input_range, u32 data_width, struct ycc_coef_inputs *coef_in)
{
  double acoef[4][3] = {0.299, 0.299,  0.2568, 0.299, 0.299,  0.2568, 0.2126, 0.2126, 0.1819, 0.299,    0.299,    0.299};
  double bcoef[4][3] = {0.114, 0.114,  0.0979, 0.114, 0.114,  0.0979, 0.0722, 0.0722, 0.0618, 0.114,    0.114,    0.114};
  double ccoef[4][3] = {0.713, 0.7295, 0.5910, 0.713, 0.7295, 0.5910, 0.6350, 0.6495, 0.6495, 0.877283, 0.877283, 0.877283};
  double dcoef[4][3] = {0.564, 0.5772, 0.5772, 0.564, 0.5772, 0.5772, 0.5389, 0.5512, 0.5512, 0.492111, 0.492111, 0.492111};
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
  coef_in->rgbmax = max[input_range];
  coef_in->rgbmin = min[input_range];
}


/*****************************************************************************/
/**
*
* Translate input coefficients into coefficients that can be programmed into the 
* YCrCb2RGB core.
*
* @param coef_in is a pointer to a rgb_coef_inputs data structure.
* @param coef_out is a pointer to a rgb_coef_output data structure.
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
u32 YCC_coefficient_translation(struct ycc_coef_inputs *coef_in, struct ycc_coef_outputs *coef_out, u32 data_width, u32 mwidth)
{
  u32 ret_val = 0;
  u32 scale_coeffs;
  double scale_ic2m;
  u32 coef_range;
  double acoef, bcoef, ccoef, dcoef;
  double acr, bcr, ccr, dcr;
  u32 ac, bc, cc, dc;
  u32 yoffset, cboffset, croffset;
  u32 rgbmax, rgbmin;
  u32 iwidth, owidth, cwidth;
  double rounding_const;

  iwidth = data_width;
  owidth = data_width;
  coef_range = 2;
  cwidth = 17;

  acoef = coef_in->acoef;
  bcoef = coef_in->bcoef;
  ccoef = coef_in->ccoef;
  dcoef = coef_in->dcoef;
  yoffset = coef_in->yoffset;
  cboffset = coef_in->cboffset;
  croffset = coef_in->croffset;
  rgbmax = coef_in->rgbmax;
  rgbmin = coef_in->rgbmin;

  if((acoef + bcoef) > 1.0) {
    printf("WARNING: Acoef (%lf) + Bcoef (%lf) can not be more then 1.0\r\n",acoef, bcoef);
    ret_val = ret_val | 0x1;
  }
  if(yoffset < -(1<<iwidth) || yoffset > (1<<iwidth)-1) {
    printf("WARNING: Y Offset (%u) is outside the data width range [%u, %u]\r\n",
               (unsigned int)yoffset, (unsigned int)-(1<<iwidth), (unsigned int)(1<<iwidth)-1);
    ret_val = ret_val | 0x2;
  }
  if(cboffset < -(1<<iwidth) || cboffset > (1<<iwidth)-1) {
    printf("WARNING: Cb Offset (%u) is outside the data width range [%u, %u]\r\n",
               (unsigned int)cboffset, (unsigned int)-(1<<iwidth), (unsigned int)(1<<iwidth)-1);
    ret_val = ret_val | 0x4;
  }
  if(croffset < -(1<<iwidth) || croffset > (1<<iwidth)-1) {
    printf("WARNING: Cr Offset (%u) is outside the data width range [%u, %u]\r\n",
               (unsigned int)croffset, (unsigned int)-(1<<iwidth), (unsigned int)(1<<iwidth)-1);
    ret_val = ret_val | 0x8;
  }
  if(rgbmax < 0 || rgbmax > (1<<iwidth)-1) {
    printf("WARNING: RGB Max (%u) is outside the data width range [0, %u]\r\n",
               (unsigned int)rgbmax, (unsigned int)(1<<iwidth)-1);
    ret_val = ret_val | 0x10;
  }
  if(rgbmin < 0 || rgbmin > (1<<iwidth)-1) {
    printf("WARNING: RGB Min (%u) is outside the data width range [0, %u]\r\n",
               (unsigned int)rgbmin, (unsigned int)(1<<iwidth)-1);
    ret_val = ret_val | 0x20;
  }

  acr = 1/ccoef;
  bcr = -(acoef/ccoef/(1-acoef-bcoef));
  ccr = -(bcoef/dcoef/(1-acoef-bcoef));
  dcr = 1/dcoef;

  scale_coeffs = ycc_max(1, (1 << (cwidth-coef_range-1)));  
  rounding_const = ycc_pow2(mwidth - owidth - coef_range -2); //  +0.5 turns truncation to biased rounding 
  scale_ic2m = ycc_pow2(1+mwidth - iwidth - cwidth );

  // Quantize coefficients:
  ac = (u32) (acr*scale_coeffs); // round, wrap
  bc = (u32) (bcr*scale_coeffs); 
  cc = (u32) (ccr*scale_coeffs); 
  dc = (u32) (dcr*scale_coeffs); 

  coef_out->acoef = ac;
  coef_out->bcoef = bc; 
  coef_out->ccoef = cc; 
  coef_out->dcoef = dc; 
    
  // Assuming signals are OWIDTH+1 bits wide after the offset compensating rounders / adders:   
  coef_out->roffset = (u32) ((rounding_const - (ac*croffset                   + yoffset*scale_coeffs))*scale_ic2m); 
  coef_out->goffset = (u32) ((rounding_const - (((bc*croffset)+(cc*cboffset)) + yoffset*scale_coeffs))*scale_ic2m); 
  coef_out->boffset = (u32) ((rounding_const - (dc*cboffset                   + yoffset*scale_coeffs))*scale_ic2m);

  coef_out->rgbmax  = rgbmax;
  coef_out->rgbmin  = rgbmin;

  return ret_val;
}

/*****************************************************************************/
/**
*
* Program the YCrCb2RGB coefficient/offset registers.
*
* @param BaseAddress is the Xilinx EDK base address of the YCrCb2RGB core (from xparameters.h)
* @param coef_out is a pointer to a rgb_coef_output data structure.
*
* @return   None.
*
* @note
*
******************************************************************************/
void YCC_set_coefficients(u32 BaseAddress, struct ycc_coef_outputs *coef_out)
{
    YCC_WriteReg(BaseAddress, YCC_ACOEF,   coef_out->acoef);     //ACOEF
    YCC_WriteReg(BaseAddress, YCC_BCOEF,   coef_out->bcoef);     //BCOEF
    YCC_WriteReg(BaseAddress, YCC_CCOEF,   coef_out->ccoef);     //CCOEF
    YCC_WriteReg(BaseAddress, YCC_DCOEF,   coef_out->dcoef);     //DCOEF
    YCC_WriteReg(BaseAddress, YCC_ROFFSET, coef_out->roffset);   //ROFFSET
    YCC_WriteReg(BaseAddress, YCC_GOFFSET, coef_out->goffset);   //GOFFSET
    YCC_WriteReg(BaseAddress, YCC_BOFFSET, coef_out->boffset);   //BOFFSET
    YCC_WriteReg(BaseAddress, YCC_RGBMAX,  coef_out->rgbmax);    //RGBMAX
    YCC_WriteReg(BaseAddress, YCC_RGBMIN,  coef_out->rgbmin);    //RGBMIN
}

/*****************************************************************************/
/**
*
* Read the YCrCb2RGB coefficient/offset registers.
*
* @param BaseAddress is the Xilinx EDK base address of the YCrCb2RGB core (from xparameters.h)
* @param coef_out is a pointer to a rgb_coef_output data structure.
*
* @return   None.
*
* @note
*
******************************************************************************/
void YCC_get_coefficients(u32 BaseAddress, struct ycc_coef_outputs *coef_out)
{
  coef_out->acoef   = YCC_ReadReg(BaseAddress, YCC_ACOEF); 
  coef_out->bcoef   = YCC_ReadReg(BaseAddress, YCC_BCOEF); 
  coef_out->ccoef   = YCC_ReadReg(BaseAddress, YCC_CCOEF); 
  coef_out->dcoef   = YCC_ReadReg(BaseAddress, YCC_DCOEF);
  coef_out->roffset = YCC_ReadReg(BaseAddress, YCC_ROFFSET); 
  coef_out->goffset = YCC_ReadReg(BaseAddress, YCC_GOFFSET); 
  coef_out->boffset = YCC_ReadReg(BaseAddress, YCC_BOFFSET);
  coef_out->rgbmax  = YCC_ReadReg(BaseAddress, YCC_RGBMAX); 
  coef_out->rgbmin  = YCC_ReadReg(BaseAddress, YCC_RGBMIN);
}
