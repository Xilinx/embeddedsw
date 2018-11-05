/*******************************************************************************
 *
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file idt_8t49n24x.h
* 
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  RHe    16/06/20 Initial release. A ported version from the
*                       8T49N28X-FrequencyProgrammingGuide-register-calculations.py script
* 2.00  MG     16/08/16 Major update
* 2.10  MG     16/09/05 Added LOS variable
* </pre>
*
******************************************************************************/

#ifndef IDT_8T49N24X_H
#define IDT_8T49N24X_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions *****************************/
#define IDT_8T49N24X_REVID 0x0    /**< Device Revision */
#define IDT_8T49N24X_DEVID 0x0607 /**< Device ID Code */

#define IDT_8T49N24X_XTAL_FREQ 40000000  //!< The frequency of the crystal in Hz

#define IDT_8T49N24X_FVCO_MAX 4000000000 //!< Maximum VCO Operating Frequency in Hz
#define IDT_8T49N24X_FVCO_MIN 3000000000 //!< Minimum VCO Operating Frequency in Hz

#define IDT_8T49N24X_FOUT_MAX 400000000  //!< Maximum Output Frequency in Hz
#define IDT_8T49N24X_FOUT_MIN      8000  //!< Minimum Output Frequency in Hz

#define IDT_8T49N24X_FIN_MAX 875000000  //!< Maximum Input Frequency in Hz
#define IDT_8T49N24X_FIN_MIN      8000  //!< Minimum Input Frequency in Hz

//#define IDT_8T49N24X_FPD_MAX 8000000  //!< Maximum Phase Detector Frequency in Hz
#define IDT_8T49N24X_FPD_MAX 128000  //!< Maximum Phase Detector Frequency in Hz
#define IDT_8T49N24X_FPD_MIN   8000  //!< Minimum Phase Detector Frequency in Hz

#define IDT_8T49N24X_P_MAX pow(2,22)  //!< Maximum P divider value
#define IDT_8T49N24X_M_MAX pow(2,24)  //!< Maximum M multiplier value

/* 
 * This configuration was created with the IDT timing commander.
 * IT configures the clock device in synthesizer mode.
 * It produces a 148.5 MHz free running clock on outputs Q2 and Q3. 
 * */ 
static const u8 IDT_8T49N24x_Config_Syn[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xEF, 0x00,
	0x03, 0x00, 0x31, 0x00, 0x04, 0x89, 0x00, 0x00, 
	
	0x01, 0x00, 0x63, 0xC6, 0x07, 0x00, 0x00, 0x77,
	0x6D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 

	0xFF, 0xFF, 0xFF, 0x01, 0x3F, 0x00, 0x28, 0x00,
	0x1A, 0xCC, 0xCD, 0x00, 0x01, 0x00, 0x00, 0xD0, 

	0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00,
	0x00, 0x0C, 0x00, 0x00, 0x00, 0x44, 0x44, 0x00, 

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B,
	0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x89, 0x0A, 0x2B, 0x20, 0x00, 0x00, 0x00, 0x03, 

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 

	0x00, 0x00, 0x00, 0x00
};

/*
 * This configuration was created with the IDT timing commander.
 * It configures the clock device is Jitter Attenuator mode.
 * It produces a 148.5 MHz clock on outputs
 * Q2 and Q3 from an incoming 148.5 MHz clock. 
 * */
static const u8 IDT_8T49N24x_Config_JA[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xEF, 0x00,
	0x03, 0x00, 0x20, 0x00, 0x04, 0x89, 0x00, 0x00, 

	0x01, 0x00, 0x63, 0xC6, 0x07, 0x00, 0x00, 0x77,
	0x6D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 

	0xFF, 0xFF, 0xFF, 0x01, 0x3F, 0x00, 0x28, 0x00,
	0x1A, 0xCC, 0xCD, 0x00, 0x01, 0x00, 0x00, 0xD0, 

	0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00,
	0x00, 0x0C, 0x00, 0x00, 0x00, 0x44, 0x44, 0x00, 

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B,
	0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x89, 0x02, 0x2B, 0x20, 0x00, 0x00, 0x00, 0x03, 

	0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 

	0x00, 0x00, 0x00, 0x00
};

/**************************** Type Definitions *******************************/

typedef struct {
	/* Integer Output Divider */
	u8  NS1_Qx;
	u16 NS2_Qx;
	
	/* Fractional Output Divider */
	u32 N_Qx;
	u32 NFRAC_Qx;
	
	/* Upper Loop Feedback Divider */
	u16 DSM_INT;
	u32 DSM_FRAC;
	
	/* Lower Loop Dividers */
	u32 M1_x;
	u32 PRE_x;

	/* Input monitor clocks */
	u32 LOS_x;
	
} IDT_8T49N24x_Settings;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int IDT_8T49N24x_Init(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int IDT_8T49N24x_SetClock(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			int FIn, int FOut, u8 FreeRun);
void IDT_8T49N24x_RegisterDump(u32 I2CBaseAddress, u8 I2CSlaveAddress);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* IDT_8T49N24X_H */
