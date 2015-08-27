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
* @file xv_vscaler_l2.c
* @addtogroup v_vscaler_v1_0
* @{
* @details
*
* The Vertical Scaler Layer-2 Driver.
* The functions in this file provides an abstraction from the register peek/poke
* methodology by implementing most common use-case provided by the sub-core.
* See xv_vscaler_l2.h for a detailed description of the layer-2 driver
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
#include "xv_vscaler_l2.h"

/************************** Constant Definitions *****************************/
#define STEP_PRECISION         (65536)  // 2^16

/* Mask definitions for Low and high 16 bits in a 32 bit number */
#define XVSC_MASK_LOW_16BITS       (0x0000FFFF)
#define XVSC_MASK_HIGH_16BITS      (0xFFFF0000)

/**************************** Type Definitions *******************************/

/**************************** Local Global *******************************/
const short XV_vscaler_fixedcoeff_taps6[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_6];
const short XV_vscaler_fixedcoeff_taps8[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_8];
const short XV_vscaler_fixedcoeff_taps10[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_10];
const short XV_vscaler_fixedcoeff_taps12[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_12];

/************************** Function Prototypes ******************************/
static void XV_VScalerSelectCoeff(XV_vscaler *InstancePtr,
		                          XV_vscaler_l2 *VscL2DataPtr,
		                          u32 HeightIn,
		                          u32 HeightOut);

static void XV_VScalerSetCoeff(XV_vscaler *pVsc,
                               XV_vscaler_l2 *VscL2DataPtr);

/*****************************************************************************/
/**
* This function starts the vertical scaler core
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_VScalerStart(XV_vscaler *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_vscaler_EnableAutoRestart(InstancePtr);
  XV_vscaler_Start(InstancePtr);
}

/*****************************************************************************/
/**
* This function stops the vertical scaler core
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_VScalerStop(XV_vscaler *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_vscaler_DisableAutoRestart(InstancePtr);
}

/*****************************************************************************/
/**
* This function loads default filter coefficients in the scaler coefficient
* storage based on the selected TAP configuration
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  VscL2DataPtr is a pointer to the core instance layer 2 data.
* @param  WidthIn is the input stream height
* @param  Widthout is the output stream height

* @return None
*
******************************************************************************/
static void XV_VScalerSelectCoeff(XV_vscaler *InstancePtr,
		                          XV_vscaler_l2 *VscL2DataPtr,
		                          u32 HeightIn,
		                          u32 HeightOut)
{
  const short *coeff;
  u16 numTaps, numPhases;
  u16 ScalingRatio;
  u16 IsScaleDown;

  /*
   * validates input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(VscL2DataPtr != NULL);

  numPhases = (1<<InstancePtr->Config.PhaseShift);

  IsScaleDown = (HeightOut < HeightIn);
  /* Scale Down Mode will use dynamic filter selection logic
   * Scale Up Mode (including 1:1) will always use 6 tap filter
   */
  if(IsScaleDown)
  {
    ScalingRatio = ((HeightIn * 10)/HeightOut);

    switch(InstancePtr->Config.NumTaps)
    {
      case XV_VSCALER_TAPS_6:
	       coeff = &XV_vscaler_fixedcoeff_taps6[0][0];
           numTaps = XV_VSCALER_TAPS_6;
		   break;

      case XV_VSCALER_TAPS_8:
	   if(ScalingRatio > 15)
	   {
		 coeff = &XV_vscaler_fixedcoeff_taps8[0][0];
	         numTaps = XV_VSCALER_TAPS_8;
	   }
	   else // <= 1.5
	   {
	         coeff = &XV_vscaler_fixedcoeff_taps6[0][0];
	         numTaps = XV_VSCALER_TAPS_6;
	   }
		   break;

      case XV_VSCALER_TAPS_10:
	       if(ScalingRatio > 25) // >2.5
	       {
		     coeff = &XV_vscaler_fixedcoeff_taps10[0][0];
	         numTaps = XV_VSCALER_TAPS_10;
	       }
	       else if(ScalingRatio > 15) // 1.6 < ratio <= 2.5
	       {
	         coeff = &XV_vscaler_fixedcoeff_taps8[0][0];
	         numTaps = XV_VSCALER_TAPS_8;
	       }
	       else // <= 1.5
	       {
	         coeff = &XV_vscaler_fixedcoeff_taps6[0][0];
	         numTaps = XV_VSCALER_TAPS_6;
	       }
           break;

    case XV_VSCALER_TAPS_12:
	       if(ScalingRatio > 35) // > 3.5
	       {
		     coeff = &XV_vscaler_fixedcoeff_taps12[0][0];
	         numTaps = XV_VSCALER_TAPS_12;
	       }
	       else if(ScalingRatio > 25) //2.6 < Ratio <= 3.5
	       {
	         coeff = &XV_vscaler_fixedcoeff_taps10[0][0];
	         numTaps = XV_VSCALER_TAPS_10;
	       }
	       else if(ScalingRatio > 15) //1.6 < Ratio <= 2.5
	       {
	         coeff = &XV_vscaler_fixedcoeff_taps8[0][0];
	         numTaps = XV_VSCALER_TAPS_8;
	       }
	       else // <= 1.5
	       {
	         coeff = &XV_vscaler_fixedcoeff_taps6[0][0];
	         numTaps = XV_VSCALER_TAPS_6;
	       }
		   break;

	  default:
		  xil_printf("ERR: H-Scaler %d Taps Not Supported",numTaps);
		  return;
	}
  }
  else //Scale Up
  {
    coeff = &XV_vscaler_fixedcoeff_taps6[0][0];
    numTaps = XV_VSCALER_TAPS_6;
  }

  XV_VScalerLoadExtCoeff(InstancePtr,
		                 VscL2DataPtr,
		                 numPhases,
		                 numTaps,
		                 coeff);

  /* Disable use of external coefficients */
  VscL2DataPtr->UseExtCoeff = FALSE;
}

/*****************************************************************************/
/**
* This function loads user defined filter coefficients in the scaler coefficient
* storage
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  VscL2DataPtr is a pointer to the core instance layer 2 data.
* @param  num_phases is the number of phases in coefficient table
* @param  num_taps is the number of taps in coefficient table
* @param  Coeff is a pointer to user defined filter coefficients table
*
* @return None
*
******************************************************************************/
void XV_VScalerLoadExtCoeff(XV_vscaler *InstancePtr,
                            XV_vscaler_l2 *VscL2DataPtr,
                            u16 num_phases,
                            u16 num_taps,
                            const short *Coeff)
{
  int i,j, pad, offset;

  /*
   * validate input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(VscL2DataPtr != NULL);
  Xil_AssertVoid(num_taps <= InstancePtr->Config.NumTaps);
  Xil_AssertVoid(num_phases == (1<<InstancePtr->Config.PhaseShift));
  Xil_AssertVoid(Coeff != NULL);

  switch(num_taps)
  {
    case XV_VSCALER_TAPS_6:
    case XV_VSCALER_TAPS_8:
    case XV_VSCALER_TAPS_10:
    case XV_VSCALER_TAPS_12:
	break;

    default:
	xil_printf("\r\nERR: V Scaler %d TAPS not supported. (Select from 8/10/12/16)\r\n");
	return;
  }

  //determine if coefficient needs padding (effective vs. max taps)
  pad = XV_VSCALER_MAX_V_TAPS - num_taps;
  offset = ((pad) ? (pad>>1) : 0);

  //Load User defined coefficients into scaler coefficient table
  for (i = 0; i < num_phases; i++)
  {
    for (j=0; j<num_taps; ++j)
    {
      VscL2DataPtr->coeff[i][j+offset] = Coeff[i*num_taps+j];
    }
  }

  if(pad) //effective taps < max_taps
  {
    for (i = 0; i < num_phases; i++)
    {
      //pad left
      for (j = 0; j < offset; j++)
      {
        VscL2DataPtr->coeff[i][j] = 0;
      }
        //pad right
      for (j = (num_taps+offset); j < XV_VSCALER_MAX_V_TAPS; j++)
      {
        VscL2DataPtr->coeff[i][j] = 0;
      }
    }
  }

  /* Enable use of external coefficients */
  VscL2DataPtr->UseExtCoeff = TRUE;
}

/*****************************************************************************/
/**
* This function programs the computed filter coefficients and phase data into
* core registers
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  VCoeff is the array that holds computed coefficients
*
* @return None
*
* @Note  This version of driver does not make use of computed coefficients.
*        Pre-computed coefficients are stored in a local table which are used
*        to overwrite any computed coefficients before being programmed into
*        the core registers. Control flow still computes the coefficients to
*        maintain the sw latency for driver version which would eventually use
*        computed coefficients
******************************************************************************/
static void XV_VScalerSetCoeff(XV_vscaler *pVsc,
                               XV_vscaler_l2 *VscL2DataPtr)
{
  int num_phases = 1<<pVsc->Config.PhaseShift;
  int num_taps   = pVsc->Config.NumTaps/2;
  int val,i,j,offset,rdIndx;
  u32 baseAddr;

  offset = (XV_VSCALER_MAX_V_TAPS - pVsc->Config.NumTaps)/2;
  baseAddr = XV_vscaler_Get_HwReg_vfltCoeff_BaseAddress(pVsc);
  for (i=0; i < num_phases; i++)
  {
    for(j=0; j < num_taps; j++)
    {
       rdIndx = j*2+offset;
       val = (VscL2DataPtr->coeff[i][rdIndx+1] << 16) | (VscL2DataPtr->coeff[i][rdIndx] & XVSC_MASK_LOW_16BITS);
       Xil_Out32(baseAddr+((i*num_taps+j)*4), val);
    }
  }
}

/*****************************************************************************/
/**
* This function configures the scaler core registers with the specified
* configuration parameters of the axi stream
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  WidthIn is the input stream width
* @param  HeightIn is the input stream height
* @param  HeightOut is the output stream height
*
* @return None
*
******************************************************************************/
void XV_VScalerSetup(XV_vscaler  *InstancePtr,
                     XV_vscaler_l2 *VscL2DataPtr,
                     u32         WidthIn,
                     u32         HeightIn,
                     u32         HeightOut)
{
  u32 LineRate;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(VscL2DataPtr != NULL);
  Xil_AssertVoid((WidthIn>0) && (WidthIn<=InstancePtr->Config.MaxWidth));
  Xil_AssertVoid((HeightIn>0) && (HeightIn<=InstancePtr->Config.MaxHeight));
  Xil_AssertVoid((HeightOut>0) && (HeightOut<=InstancePtr->Config.MaxHeight));
  Xil_AssertVoid((InstancePtr->Config.PixPerClk >= XVIDC_PPC_1) &&
                 (InstancePtr->Config.PixPerClk <= XVIDC_PPC_4));

  if(InstancePtr->Config.ScalerType == XV_VSCALER_POLYPHASE)
  {
    if(!VscL2DataPtr->UseExtCoeff) //No user defined coefficients
    {
      /* Determine coefficient table to use */
      XV_VScalerSelectCoeff(InstancePtr,
		                VscL2DataPtr,
		                HeightIn,
		                HeightOut);
    }

    /* Program coefficients into the IP register bank */
    XV_VScalerSetCoeff(InstancePtr, VscL2DataPtr);
  }

  LineRate = (HeightIn * STEP_PRECISION)/HeightOut;

  XV_vscaler_Set_HwReg_HeightIn(InstancePtr,   HeightIn);
  XV_vscaler_Set_HwReg_Width(InstancePtr,      WidthIn);
  XV_vscaler_Set_HwReg_HeightOut(InstancePtr,  HeightOut);
  XV_vscaler_Set_HwReg_LineRate(InstancePtr,   LineRate);
}

/*****************************************************************************/
/**
*
* This function prints V Scaler status on the console
*
* @param	InstancePtr is the instance pointer to the core instance.
*
* @return	None
*
******************************************************************************/
void XV_VScalerDbgReportStatus(XV_vscaler *InstancePtr)
{
  XV_vscaler *pVsc = InstancePtr;
  u32 done, idle, ready, ctrl;
  u32 widthin, heightin, heightout, linerate;
  u32 baseAddr, taps, phases;
  int val,i,j;
  const char *ScalerTypeStr[] = {"Bilinear", "Bicubic", "Polyphase"};

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n\r\n----->V SCALER IP STATUS<----\r\n");
  done  = XV_vscaler_IsDone(pVsc);
  idle  = XV_vscaler_IsIdle(pVsc);
  ready = XV_vscaler_IsReady(pVsc);
  ctrl =  XV_vscaler_ReadReg(pVsc->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_AP_CTRL);


  heightin  = XV_vscaler_Get_HwReg_HeightIn(pVsc);
  widthin   = XV_vscaler_Get_HwReg_Width(pVsc);
  heightout = XV_vscaler_Get_HwReg_HeightOut(pVsc);
  linerate  = XV_vscaler_Get_HwReg_LineRate(pVsc);

  taps   = pVsc->Config.NumTaps/2;
  phases = (1<<pVsc->Config.PhaseShift);

  xil_printf("IsDone:  %d\r\n", done);
  xil_printf("IsIdle:  %d\r\n", idle);
  xil_printf("IsReady: %d\r\n", ready);
  xil_printf("Ctrl:    0x%x\r\n\r\n", ctrl);

  if(pVsc->Config.ScalerType <= XV_VSCALER_POLYPHASE)
  {
    xil_printf("Scaler Type:     %s\r\n",ScalerTypeStr[pVsc->Config.ScalerType]);
  }
  else
  {
    xil_printf("Scaler Type:     Unknown\r\n");
  }
  xil_printf("Input Width:     %d\r\n",widthin);
  xil_printf("Input Height:    %d\r\n",heightin);
  xil_printf("Output Height:   %d\r\n",heightout);
  xil_printf("Line Rate:       %d\r\n",linerate);
  xil_printf("Num Phases:      %d\r\n",phases);

  if(pVsc->Config.ScalerType == XV_VSCALER_POLYPHASE)
  {
    short lsb, msb;

    xil_printf("Num Taps:        %d\r\n",taps*2);
    xil_printf("\r\nCoefficients:");

    baseAddr = XV_vscaler_Get_HwReg_vfltCoeff_BaseAddress(pVsc);
    for(i = 0; i < phases; i++)
    {
      xil_printf("\r\nPhase %2d: ",i);
      for(j=0; j< taps; j++)
      {
        val = Xil_In32(baseAddr+((i*taps+j)*4));

        //coefficients are 12-bits
        lsb = (short)(val & XVSC_MASK_LOW_16BITS);
        msb = (short)((val & XVSC_MASK_HIGH_16BITS)>>16);

        xil_printf("%5d %5d ", lsb, msb);
      }
    }
  }
}
/** @} */
