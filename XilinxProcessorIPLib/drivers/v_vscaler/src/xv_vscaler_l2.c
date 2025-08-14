/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_vscaler_l2.c
* @addtogroup v_vscaler Overview
* @{
* @brief
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
* 2.00  rco   11/05/15   Integrate layer-1 with layer-2
* 3.0   mpe   04/28/16   Added optional color format conversion handling
*       rco   02/09/17   Fix c++ compilation warnings
*	jsr   09/07/18 Fix for 64-bit driver support
* 3.1   vsa   04/07/20   Improve quality with new coefficients
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "xv_vscaler_l2.h"

/************************** Constant Definitions *****************************/
#define STEP_PRECISION         (65536)  // 2^16

/* Mask definitions for Low and high 16 bits in a 32 bit number */
#define XVSC_MASK_LOW_16BITS       (0x0000FFFF)
#define XVSC_MASK_HIGH_16BITS      (0xFFFF0000)

/**************************** Type Definitions *******************************/

/**************************** Local Global *******************************/
extern const short XV_vscaler_Lanczos2_taps6[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_6];
extern const short XV_vscaler_fixedcoeff_taps6_ScalingRatio1p2[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_6];
extern const short XV_vscaler_fixedcoeff_taps6_ScalingRatio2[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_6];
extern const short XV_vscaler_fixedcoeff_taps6_ScalingRatio3[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_6];
extern const short XV_vscaler_fixedcoeff_taps6_ScalingRatio4[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_6];
extern const short XV_vscaler_fixedcoeff_taps8_ScalingRatio2[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_8];
extern const short XV_vscaler_fixedcoeff_taps8_ScalingRatio3[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_8];
extern const short XV_vscaler_fixedcoeff_taps8_ScalingRatio4[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_8];
extern const short XV_vscaler_fixedcoeff_taps10_ScalingRatio3[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_10];
extern const short XV_vscaler_fixedcoeff_taps10_ScalingRatio4[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_10];
extern const short XV_vscaler_fixedcoeff_taps12_ScalingRatio4[XV_VSCALER_MAX_V_PHASES][XV_VSCALER_TAPS_12];

/************************** Function Prototypes ******************************/
static void XV_VScalerSelectCoeff(XV_Vscaler_l2 *InstancePtr,
		                          u32 HeightIn,
		                          u32 HeightOut);

static void XV_VScalerSetCoeff(XV_Vscaler_l2 *VscPtr);

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
int XV_VScalerInitialize(XV_Vscaler_l2 *InstancePtr, u16 DeviceId)
{
  int Status;
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Setup the instance */
  memset(InstancePtr, 0, sizeof(XV_Vscaler_l2));
  Status = XV_vscaler_Initialize(&InstancePtr->Vsc, DeviceId);

  return(Status);
}

/*****************************************************************************/
/**
* This function starts the vertical scaler core
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_VScalerStart(XV_Vscaler_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_vscaler_EnableAutoRestart(&InstancePtr->Vsc);
  XV_vscaler_Start(&InstancePtr->Vsc);
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
void XV_VScalerStop(XV_Vscaler_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_vscaler_DisableAutoRestart(&InstancePtr->Vsc);
}

/*****************************************************************************/
/**
* This function loads default filter coefficients in the scaler coefficient
* storage based on the selected TAP configuration
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  WidthIn is the input stream height
* @param  Widthout is the output stream height

* @return None
*
******************************************************************************/
static void XV_VScalerSelectCoeff(XV_Vscaler_l2 *InstancePtr,
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

  numPhases = (1<<InstancePtr->Vsc.Config.PhaseShift);

  IsScaleDown = (HeightOut < HeightIn);
  /* Scale Down Mode will use dynamic filter selection logic
   * Scale Up Mode (including 1:1) will always use 6 tap filter
   */
  if(IsScaleDown)
  {
    ScalingRatio = ((HeightIn * 10)/HeightOut);

    switch(InstancePtr->Vsc.Config.NumTaps)
    {
	case XV_VSCALER_TAPS_6:
		if (ScalingRatio > 35) {// > 3.5
			coeff = &XV_vscaler_fixedcoeff_taps6_ScalingRatio4[0][0];
			numTaps = XV_VSCALER_TAPS_6;
		} else if (ScalingRatio > 25) {//2.6 < Ratio <= 3.5
			coeff = &XV_vscaler_fixedcoeff_taps6_ScalingRatio3[0][0];
			numTaps = XV_VSCALER_TAPS_6;
		} else if (ScalingRatio > 15) {//1.6 < Ratio <= 2.5
			coeff = &XV_vscaler_fixedcoeff_taps6_ScalingRatio2[0][0];
			numTaps = XV_VSCALER_TAPS_6;
		} else {// <= 1.5
			coeff = &XV_vscaler_fixedcoeff_taps6_ScalingRatio1p2[0][0];
			numTaps = XV_VSCALER_TAPS_6;
		}
	break;

	case XV_VSCALER_TAPS_8:
		if (ScalingRatio > 35) {// > 3.5
			coeff = &XV_vscaler_fixedcoeff_taps8_ScalingRatio4[0][0];
			numTaps = XV_VSCALER_TAPS_8;
		} else if (ScalingRatio > 25) {//2.6 < Ratio <= 3.5
			coeff = &XV_vscaler_fixedcoeff_taps8_ScalingRatio3[0][0];
			numTaps = XV_VSCALER_TAPS_8;
		} else if (ScalingRatio > 15) {//1.6 < Ratio <= 2.5
			coeff = &XV_vscaler_fixedcoeff_taps8_ScalingRatio2[0][0];
			numTaps = XV_VSCALER_TAPS_8;
		} else {// <= 1.5
			coeff = &XV_vscaler_fixedcoeff_taps6_ScalingRatio1p2[0][0];
			numTaps = XV_VSCALER_TAPS_6;
		}
	break;

	case XV_VSCALER_TAPS_10:
		if (ScalingRatio > 35) {// > 3.5
			coeff = &XV_vscaler_fixedcoeff_taps10_ScalingRatio4[0][0];
			numTaps = XV_VSCALER_TAPS_10;
		} else if (ScalingRatio > 25) {//2.6 < Ratio <= 3.5
			coeff = &XV_vscaler_fixedcoeff_taps10_ScalingRatio3[0][0];
			numTaps = XV_VSCALER_TAPS_10;
		} else if (ScalingRatio > 15) {//1.6 < Ratio <= 2.5
			coeff = &XV_vscaler_fixedcoeff_taps8_ScalingRatio2[0][0];
			numTaps = XV_VSCALER_TAPS_8;
		} else {// <= 1.5
			coeff = &XV_vscaler_fixedcoeff_taps6_ScalingRatio1p2[0][0];
			numTaps = XV_VSCALER_TAPS_6;
		}
	break;

	case XV_VSCALER_TAPS_12:
		if (ScalingRatio > 35) {// > 3.5
			coeff = &XV_vscaler_fixedcoeff_taps12_ScalingRatio4[0][0];
			numTaps = XV_VSCALER_TAPS_12;
		} else if (ScalingRatio > 25) {//2.6 < Ratio <= 3.5
			coeff = &XV_vscaler_fixedcoeff_taps10_ScalingRatio3[0][0];
			numTaps = XV_VSCALER_TAPS_10;
		} else if (ScalingRatio > 15) {//1.6 < Ratio <= 2.5
			coeff = &XV_vscaler_fixedcoeff_taps8_ScalingRatio2[0][0];
			numTaps = XV_VSCALER_TAPS_8;
		} else {// <= 1.5
			coeff = &XV_vscaler_fixedcoeff_taps6_ScalingRatio1p2[0][0];
			numTaps = XV_VSCALER_TAPS_6;
		}
	break;

	default:
		return;
	}
  }
  else //Scale Up
  {
	coeff = &XV_vscaler_Lanczos2_taps6[0][0];
	numTaps = XV_VSCALER_TAPS_6;
  }

  XV_VScalerLoadExtCoeff(InstancePtr,
		                 numPhases,
		                 numTaps,
		                 coeff);

  /* Disable use of external coefficients */
  InstancePtr->UseExtCoeff = FALSE;
}

/*****************************************************************************/
/**
* This function loads user defined filter coefficients in the scaler coefficient
* storage
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  num_phases is the number of phases in coefficient table
* @param  num_taps is the number of taps in coefficient table
* @param  Coeff is a pointer to user defined filter coefficients table
*
* @return None
*
******************************************************************************/
void XV_VScalerLoadExtCoeff(XV_Vscaler_l2 *InstancePtr,
                            u16 num_phases,
                            u16 num_taps,
                            const short *Coeff)
{
  int i,j, pad, offset;

  /*
   * validate input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(num_taps <= InstancePtr->Vsc.Config.NumTaps);
  Xil_AssertVoid(num_phases == (1<<InstancePtr->Vsc.Config.PhaseShift));
  Xil_AssertVoid(Coeff != NULL);

  switch(num_taps)
  {
    case XV_VSCALER_TAPS_6:
    case XV_VSCALER_TAPS_8:
    case XV_VSCALER_TAPS_10:
    case XV_VSCALER_TAPS_12:
         break;

    default:
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
      InstancePtr->coeff[i][j+offset] = Coeff[i*num_taps+j];
    }
  }

  if(pad) //effective taps < max_taps
  {
    for (i = 0; i < num_phases; i++)
    {
      //pad left
      for (j = 0; j < offset; j++)
      {
        InstancePtr->coeff[i][j] = 0;
      }
        //pad right
      for (j = (num_taps+offset); j < XV_VSCALER_MAX_V_TAPS; j++)
      {
        InstancePtr->coeff[i][j] = 0;
      }
    }
  }

  /* Enable use of external coefficients */
  InstancePtr->UseExtCoeff = TRUE;
}

/*****************************************************************************/
/**
 * This Function Sets the vertical scaler filter coefficients in hardware registers.
 *
 * This function programs the hardware vertical scaler with the appropriate
 * filter coefficients stored in the XV_Vscaler_l2 instance. It calculates
 * the number of phases and taps, determines the correct offset for the
 * coefficients, and writes the packed coefficients to the hardware registers.
 *
 * @param VscPtr Pointer to the XV_Vscaler_l2 instance containing configuration
 *               and coefficient data.
 *
 * @return None
 * @Note  This version of driver does not make use of computed coefficients.
 *        Pre-computed coefficients are stored in a local table which are used
 *        to overwrite any computed coefficients before being programmed into
 *        the core registers. Control flow still computes the coefficients to
 *        maintain the sw latency for driver version which would eventually use
 *        computed coefficients
 */
static void XV_VScalerSetCoeff(XV_Vscaler_l2 *VscPtr)
{
  int num_phases = 1<<VscPtr->Vsc.Config.PhaseShift;
  int num_taps   = VscPtr->Vsc.Config.NumTaps/2;
  int val,i,j,offset,rdIndx;
  UINTPTR baseAddr;

  offset = (XV_VSCALER_MAX_V_TAPS - VscPtr->Vsc.Config.NumTaps)/2;
  baseAddr = XV_vscaler_Get_HwReg_vfltCoeff_BaseAddress(&VscPtr->Vsc);
  for (i=0; i < num_phases; i++)
  {
    for(j=0; j < num_taps; j++)
    {
       rdIndx = j*2+offset;
       val = (VscPtr->coeff[i][rdIndx+1] << 16) | (VscPtr->coeff[i][rdIndx] & XVSC_MASK_LOW_16BITS);
       Xil_Out32(baseAddr+((i*num_taps+j)*4), val);
    }
  }
}

/*****************************************************************************/
/**
* This function configures the scaler core registers with the specified
* configuration parameters of the AXI stream.
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  WidthIn is the input stream width.
* @param  HeightIn is the input stream height.
* @param  HeightOut is the output stream height.
* @param  ColorFormat is the color format of the stream.
*
* @return XST_SUCCESS if successful, XST_FAILURE otherwise.
*
******************************************************************************/
int XV_VScalerSetup(XV_Vscaler_l2  *InstancePtr,
          u32            WidthIn,
          u32            HeightIn,
          u32            HeightOut,
          u32            ColorFormat)
{
  u32 LineRate;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((WidthIn>0) && (WidthIn<=InstancePtr->Vsc.Config.MaxWidth));
  Xil_AssertNonvoid((HeightIn>0) && (HeightIn<=InstancePtr->Vsc.Config.MaxHeight));
  Xil_AssertNonvoid((HeightOut>0) && (HeightOut<=InstancePtr->Vsc.Config.MaxHeight));
  Xil_AssertNonvoid((InstancePtr->Vsc.Config.PixPerClk >= XVIDC_PPC_1) &&
		  (InstancePtr->Vsc.Config.PixPerClk <= XVIDC_PPC_8));

  if(ColorFormat==XVIDC_CSF_YCRCB_420 && !XV_VscalerIs420Enabled(InstancePtr)) {
    return(XST_FAILURE);
  }

  if(InstancePtr->Vsc.Config.ScalerType == XV_VSCALER_POLYPHASE)
  {
    if(!InstancePtr->UseExtCoeff) //No user defined coefficients
    {
      /* Determine coefficient table to use */
      XV_VScalerSelectCoeff(InstancePtr,  HeightIn, HeightOut);
    }

    /* Program coefficients into the IP register bank */
    XV_VScalerSetCoeff(InstancePtr);
  }

  LineRate = (HeightIn * STEP_PRECISION)/HeightOut;

  XV_vscaler_Set_HwReg_HeightIn(&InstancePtr->Vsc,   HeightIn);
  XV_vscaler_Set_HwReg_Width(&InstancePtr->Vsc,      WidthIn);
  XV_vscaler_Set_HwReg_HeightOut(&InstancePtr->Vsc,  HeightOut);
  XV_vscaler_Set_HwReg_LineRate(&InstancePtr->Vsc,   LineRate);
  XV_vscaler_Set_HwReg_ColorMode(&InstancePtr->Vsc,  ColorFormat);

  return XST_SUCCESS;
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
void XV_VScalerDbgReportStatus(XV_Vscaler_l2 *InstancePtr)
{
  XV_vscaler *VscPtr = &InstancePtr->Vsc;
  u32 done, idle, ready, ctrl;
  u32 widthin, heightin, heightout, linerate, cformat;
  u32 taps, phases, i, j;
  UINTPTR baseAddr;
  u16 allow420;
  int val;
  const char *ScalerTypeStr[] = {"Bilinear", "Bicubic", "Polyphase"};

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n\r\n----->V SCALER IP STATUS<----\r\n");
  done  = XV_vscaler_IsDone(VscPtr);
  idle  = XV_vscaler_IsIdle(VscPtr);
  ready = XV_vscaler_IsReady(VscPtr);
  ctrl =  XV_vscaler_ReadReg(VscPtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_AP_CTRL);

  heightin  = XV_vscaler_Get_HwReg_HeightIn(VscPtr);
  widthin   = XV_vscaler_Get_HwReg_Width(VscPtr);
  heightout = XV_vscaler_Get_HwReg_HeightOut(VscPtr);
  linerate  = XV_vscaler_Get_HwReg_LineRate(VscPtr);
  cformat   = XV_vscaler_Get_HwReg_ColorMode(VscPtr);
  allow420  = XV_VscalerIs420Enabled(InstancePtr);

  taps   = VscPtr->Config.NumTaps/2;
  phases = (1<<VscPtr->Config.PhaseShift);

  xil_printf("IsDone:  %d\r\n", done);
  xil_printf("IsIdle:  %d\r\n", idle);
  xil_printf("IsReady: %d\r\n", ready);
  xil_printf("Ctrl:    0x%x\r\n\r\n", ctrl);

  if(VscPtr->Config.ScalerType <= XV_VSCALER_POLYPHASE)
  {
    xil_printf("Scaler Type:     %s\r\n",ScalerTypeStr[VscPtr->Config.ScalerType]);
  }
  else
  {
    xil_printf("Scaler Type:     Unknown\r\n");
  }
  xil_printf("Input Width:      %d\r\n",widthin);
  xil_printf("Input Height:     %d\r\n",heightin);
  xil_printf("Output Height:    %d\r\n",heightout);
  xil_printf("4:2:0 processing: %s\r\n", allow420?"Enabled":"Disabled");
  xil_printf("Color Format:     %s\r\n",
		  XVidC_GetColorFormatStr((XVidC_ColorFormat)cformat));
  xil_printf("Line Rate:        %d\r\n",linerate);
  xil_printf("Num Phases:       %d\r\n",phases);

  if(VscPtr->Config.ScalerType == XV_VSCALER_POLYPHASE)
  {
    short lsb, msb;

    xil_printf("Num Taps:        %d\r\n",taps*2);
    xil_printf("\r\nCoefficients:");

    baseAddr = XV_vscaler_Get_HwReg_vfltCoeff_BaseAddress(VscPtr);
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
