/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_vcresampler_l2.c
* @addtogroup v_vcresampler_v3_1
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
* 2.00  rco   11/05/15   Integrate layer-1 with layer-2
* 2.10  rco   07/20/16   Add passthrough mode support
*       rco   02/09/17   Fix c++ compilation warnings
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "xv_vcresampler_l2.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Local Global *******************************/
extern const short XV_vcrsmplrcoeff_taps4[XV_VCRSMPLR_NUM_CONVERSIONS][XV_VCRSMPLR_MAX_PHASES][XV_VCRSMPLR_TAPS_4];
extern const short XV_vcrsmplrcoeff_taps6[XV_VCRSMPLR_NUM_CONVERSIONS][XV_VCRSMPLR_MAX_PHASES][XV_VCRSMPLR_TAPS_6];
extern const short XV_vcrsmplrcoeff_taps8[XV_VCRSMPLR_NUM_CONVERSIONS][XV_VCRSMPLR_MAX_PHASES][XV_VCRSMPLR_TAPS_8];
extern const short XV_vcrsmplrcoeff_taps10[XV_VCRSMPLR_NUM_CONVERSIONS][XV_VCRSMPLR_MAX_PHASES][XV_VCRSMPLR_TAPS_10];


/************************** Function Prototypes ******************************/
static void XV_vcresampler_SetCoefficients(XV_Vcresampler_l2 *pVCrsmplr,
		                                   XV_VCRESAMPLER_CONVERSION convType);

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
int XV_VcrsmplInitialize(XV_Vcresampler_l2 *InstancePtr, u16 DeviceId)
{
  int Status;
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Setup the instance */
  memset(InstancePtr, 0, sizeof(XV_Vcresampler_l2));
  Status = XV_vcresampler_Initialize(&InstancePtr->Vcr, DeviceId);

  return(Status);
}

/*****************************************************************************/
/**
 * This function starts the Chroma resampler core
 *
 * @param  InstancePtr is a pointer to the core instance to be worked on.
 *
 * @return None
 *
 *****************************************************************************/
void XV_VCrsmplStart(XV_Vcresampler_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_vcresampler_EnableAutoRestart(&InstancePtr->Vcr);
  XV_vcresampler_Start(&InstancePtr->Vcr);
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
void XV_VCrsmplStop(XV_Vcresampler_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_vcresampler_DisableAutoRestart(&InstancePtr->Vcr);
}

/*****************************************************************************/
/**
* This function loads default filter coefficients in the chroma resampler
* coefficient storage based on the selected TAP configuration
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_VCrsmplLoadDefaultCoeff(XV_Vcresampler_l2 *InstancePtr)
{
  u16 numTaps;
  const short *coeff;

  /*
   * validates input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  numTaps = InstancePtr->Vcr.Config.NumTaps;

  switch(numTaps)
  {
    case XV_VCRSMPLR_TAPS_4:
	     coeff = &XV_vcrsmplrcoeff_taps4[0][0][0];
		 break;

    case XV_VCRSMPLR_TAPS_6:
	     coeff = &XV_vcrsmplrcoeff_taps6[0][0][0];
		 break;

    case XV_VCRSMPLR_TAPS_8:
	     coeff = &XV_vcrsmplrcoeff_taps8[0][0][0];
         break;

    case XV_VCRSMPLR_TAPS_10:
         coeff = &XV_vcrsmplrcoeff_taps10[0][0][0];
		 break;

	  default:
         return;
  }

  /* Use external filter load API */
  XV_VCrsmplrLoadExtCoeff(InstancePtr, numTaps, coeff);

  /* Disable use of external coefficients */
  InstancePtr->UseExtCoeff = FALSE;
}

/*****************************************************************************/
/**
* This function loads user defined filter coefficients in the horiz. chroma
* resampler coefficient storage
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  num_taps is the number of taps
* @param  Coeff is a pointer to user defined filter coefficients table
*
* @return None
*
******************************************************************************/
void XV_VCrsmplrLoadExtCoeff(XV_Vcresampler_l2 *InstancePtr,
                             u16 num_taps,
                             const short *Coeff)
{
  int pad, offset;
  int index, phase, tap, conversion;

  /*
   * validate input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(num_taps <= InstancePtr->Vcr.Config.NumTaps);
  Xil_AssertVoid(Coeff != NULL);

  switch(num_taps)
  {
    case XV_VCRSMPLR_TAPS_4:
    case XV_VCRSMPLR_TAPS_6:
    case XV_VCRSMPLR_TAPS_8:
    case XV_VCRSMPLR_TAPS_10:
         break;

    default:
	     return;
  }

  //determine if coefficient needs padding (effective vs. max taps)
  pad = XV_VCRSMPLR_MAX_TAPS - InstancePtr->Vcr.Config.NumTaps;
  offset = ((pad) ? (pad>>1) : 0);

  index = 0;
  //Load User defined coefficients into coefficient storage
  for (conversion = 0; conversion < XV_VCRSMPLR_NUM_CONVERSIONS; ++conversion)
  {
    for (phase = 0; phase < XV_VCRSMPLR_MAX_PHASES; ++phase)
    {
      for (tap = 0; tap < num_taps; ++tap)
      {
	    index = (conversion*XV_VCRSMPLR_MAX_PHASES*num_taps) + (phase*num_taps) + tap;
        InstancePtr->coeff[conversion][phase][tap+offset] = Coeff[index];
      }
    }
  }

  if(pad) //effective taps < max_taps
  {
    for (conversion = 0; conversion < XV_VCRSMPLR_NUM_CONVERSIONS; ++conversion)
    {
      for(phase = 0; phase < XV_VCRSMPLR_MAX_PHASES; ++phase)
      {
        //pad left
        for (tap = 0; tap < offset; ++tap)
        {
          InstancePtr->coeff[conversion][phase][tap] = 0;
        }
        //pad right
        for (tap = (num_taps+offset); tap < XV_VCRSMPLR_MAX_TAPS; ++tap)
        {
          InstancePtr->coeff[conversion][phase][tap] = 0;
        }
      }
    }
  }

  /* Enable use of external coefficients */
  InstancePtr->UseExtCoeff = TRUE;
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
void XV_VCrsmplSetActiveSize(XV_Vcresampler_l2 *InstancePtr,
                              u32 width,
                              u32 height)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_vcresampler_Set_HwReg_width(&InstancePtr->Vcr,  width);
  XV_vcresampler_Set_HwReg_height(&InstancePtr->Vcr, height);
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
void XV_VCrsmplSetFormat(XV_Vcresampler_l2 *InstancePtr,
                         XVidC_ColorFormat formatIn,
                         XVidC_ColorFormat formatOut)
{
  XV_VCRESAMPLER_CONVERSION convType = XV_VCRSMPLR_NUM_CONVERSIONS;

  /*
   * validate the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  XV_vcresampler_Set_HwReg_input_video_format(&InstancePtr->Vcr,  formatIn);
  XV_vcresampler_Set_HwReg_output_video_format(&InstancePtr->Vcr, formatOut);

  if(InstancePtr->Vcr.Config.ResamplingType == XV_VCRSMPLR_TYPE_FIR)
  {
    if((formatIn  == XVIDC_CSF_YCRCB_420) &&
       (formatOut == XVIDC_CSF_YCRCB_422))
    {
	  convType = XV_VCRSMPLR_420_TO_422;
    }
    else if((formatIn  == XVIDC_CSF_YCRCB_422) &&
            (formatOut == XVIDC_CSF_YCRCB_420))
    {
	  convType = XV_VCRSMPLR_422_TO_420;
    }

    //Update coefficients only if conversion type is set
    if(convType != XV_VCRSMPLR_NUM_CONVERSIONS) {
      XV_vcresampler_SetCoefficients(InstancePtr, convType);
    }
  }
}

/*****************************************************************************/
/**
* This function updates the core registers with computed coefficients for
* required conversion
*
* @param  pVCrsmplr is a pointer to the core instance to be worked on.
* @param  convType is the format conversion requested
*
* @return None
*
******************************************************************************/
static void XV_vcresampler_SetCoefficients(XV_Vcresampler_l2 *pVCrsmplr,
		                                   XV_VCRESAMPLER_CONVERSION convType)
{
  u16 pad, offset;
  u32 baseaddr;
  u16 tap, phase,regcount;

  //determine if coefficients are padded
  pad = XV_VCRSMPLR_MAX_TAPS - pVCrsmplr->Vcr.Config.NumTaps;
  offset = ((pad) ? (pad>>1) : 0);

  regcount = 0; //number of registers being written
  baseaddr = XV_VCRESAMPLER_CTRL_ADDR_HWREG_COEFS_0_0_DATA;
  baseaddr += pVCrsmplr->Vcr.Config.BaseAddress;

  for(phase = 0; phase < XV_VCRSMPLR_MAX_PHASES; ++phase)
  {
    for(tap = 0; tap < pVCrsmplr->Vcr.Config.NumTaps; ++tap)
    {
	  Xil_Out32((baseaddr+(regcount*8)), pVCrsmplr->coeff[convType][phase][offset+tap]);
	  ++regcount;
    }
  }
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
void XV_VCrsmplDbgReportStatus(XV_Vcresampler_l2 *InstancePtr)
{
  XV_vcresampler *pVCrsmplr = &InstancePtr->Vcr;
  u32 done, idle, ready, ctrl;
  u32 vidfmtIn, vidfmtOut, height, width;
  u32 baseAddr;
  const char *RsmplrTypeStr[] = {"Nearest Neighbor", "Fixed Coeff", "FIR"};

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

  if(pVCrsmplr->Config.ResamplingType <= XV_VCRSMPLR_TYPE_FIR)
  {
    xil_printf("Resampling Type:  %s\r\n", RsmplrTypeStr[pVCrsmplr->Config.ResamplingType]);
  }
  else
  {
	xil_printf("Resampling Type:  Unknown\r\n");
  }
  xil_printf("Video Format In:  %s\r\n",
		  XVidC_GetColorFormatStr((XVidC_ColorFormat)vidfmtIn));
  xil_printf("Video Format Out: %s\r\n",
		  XVidC_GetColorFormatStr((XVidC_ColorFormat)vidfmtOut));
  xil_printf("Width:            %d\r\n", width);
  xil_printf("Height:           %d\r\n", height);

  if(pVCrsmplr->Config.ResamplingType == XV_VCRSMPLR_TYPE_FIR)
  {
    u32 numTaps, tap, phase, regcount;
    u32 coeff;

    numTaps = pVCrsmplr->Config.NumTaps;
    xil_printf("Num Taps:         %d\r\n", numTaps);
    xil_printf("\r\nCoefficients:");

    regcount = 0;
    baseAddr = XV_VCRESAMPLER_CTRL_ADDR_HWREG_COEFS_0_0_DATA;
    baseAddr += pVCrsmplr->Config.BaseAddress;
    for(phase=0; phase < XV_VCRSMPLR_MAX_PHASES; ++phase)
    {
      xil_printf("\r\nPhase %2d: ", phase);
      for(tap=0; tap < numTaps; ++tap)
      {
	    coeff = Xil_In32((baseAddr+(regcount*8)));
	    xil_printf("%4d ",coeff);
	    ++regcount;
      }
    }
  }
}
/** @} */
