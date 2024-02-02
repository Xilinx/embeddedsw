/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/
/**
*
* @file idt_8t49n24x.h
* @addtogroup IDT_8T49N24x
* @{
*
* <pre>
* Copyright (c) 2016 Xilinx, Inc.
* Copyright (c) 2016 Adeas B.V. All rights reserved.
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  RHe    16/06/20 Initial release. A ported version from the
*                       8T49N28X-FrequencyProgrammingGuide-
*                       	register-calculations.py script
* 2.00  MG     16/08/15 Major update
* 2.10  MG     16/09/05 Added LOS variable
* 2.20  GM     18/02/08 Converted math.h functions (e.g. ceil) to
* 							standard functions
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
#define IDT_8T49N24X_REVID 0x0    		 //!< Device Revision
#define IDT_8T49N24X_DEVID 0x0607 		 //!< Device ID Code

#define IDT_8T49N24X_XTAL_FREQ 40000000  //!< The freq of the crystal in Hz

#define IDT_8T49N24X_FVCO_MAX 4000000000 //!< Max VCO Operating Freq in Hz
#define IDT_8T49N24X_FVCO_MIN 3000000000 //!< Min VCO Operating Freq in Hz

#define IDT_8T49N24X_FOUT_MAX 400000000  //!< Max Output Freq in Hz
#define IDT_8T49N24X_FOUT_MIN      8000  //!< Min Output Freq in Hz

#define IDT_8T49N24X_FIN_MAX 875000000   //!< Max Input Freq in Hz
#define IDT_8T49N24X_FIN_MIN      8000   //!< Min Input Freq in Hz

//#define IDT_8T49N24X_FPD_MAX 8000000   //!< Max Phase Detector Freq in Hz
#define IDT_8T49N24X_FPD_MAX 128000      //!< Max Phase Detector Freq in Hz
#define IDT_8T49N24X_FPD_MIN   8000      //!< Min Phase Detector Freq in Hz

#define IDT_8T49N24X_P_MAX 4194304  /* pow(2,22) */  //!< Max P div value
#define IDT_8T49N24X_M_MAX 16777216 /* pow(2,24) */  //!< Max M mult value

/*
	This configuration was created with the IDT timing commander.
	IT configures the clock device in synthesizer mode.
	It produces a 148.5 MHz free running clock on outputs Q2 and Q3.
*/
static const u8 IDT_8T49N24x_Config_Syn[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xEF, 0x00, 0x03, 0x00, 0x31, 0x00,
	0x04, 0x89, 0x00, 0x00, 0x01, 0x00, 0x63, 0xC6, 0x07, 0x00, 0x00, 0x77,
	0x6D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x01,
	0x3F, 0x00, 0x28, 0x00, 0x1A, 0xCC, 0xCD, 0x00, 0x01, 0x00, 0x00, 0xD0,
	0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x0C, 0x00, 0x00,
	0x00, 0x44, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B,
	0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x89, 0x0A, 0x2B, 0x20,
	0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*
	This configuration was created with the IDT timing commander.
	It configures the clock device is Jitter Attenuator mode.
	It produces a 148.5 MHz clock on outputs Q2 and Q3 from an incoming
	148.5 MHz clock.
*/
static const u8 IDT_8T49N24x_Config_JA[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xEF, 0x00, 0x03, 0x00, 0x20, 0x00,
	0x04, 0x89, 0x00, 0x00, 0x01, 0x00, 0x63, 0xC6, 0x07, 0x00, 0x00, 0x77,
	0x6D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x01,
	0x3F, 0x00, 0x28, 0x00, 0x1A, 0xCC, 0xCD, 0x00, 0x01, 0x00, 0x00, 0xD0,
	0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x0C, 0x00, 0x00,
	0x00, 0x44, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B,
	0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x89, 0x02, 0x2B, 0x20,
	0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/**************************** Type Definitions *******************************/

typedef struct {
	// Integer Output Divider
	u8  NS1_Qx;
	u16 NS2_Qx;

	// Fractional Output Divider
	u32 N_Qx;
	u32 NFRAC_Qx;

	// Upper Loop Feedback Divider
	u16 DSM_INT;
	u32 DSM_FRAC;

	// Lower Loop Dividers
	u32 M1_x;
	u32 PRE_x;

	// Input monitor clocks
	u32 LOS_x;

} IDT_8T49N24x_Settings;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int IDT_8T49N24x_Init(u32 I2CBaseAddress, u8 I2CSlaveAddress);
int IDT_8T49N24x_SetClock(u32 I2CBaseAddress, u8 I2CSlaveAddress, int FIn,
							int FOut, u8 FreeRun);
void IDT_8T49N24x_RegisterDump(u32 I2CBaseAddress, u8 I2CSlaveAddress);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* IDT_8T49N24X_H */
/** @} */
