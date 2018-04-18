/******************************************************************************
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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xrfdc_mts.c
* @addtogroup xrfdc_v4_0
* @{
*
* Contains the multi tile sync functions of the XRFdc driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 3.1   jm     01/24/18 Initial release
* 3.2   jm     03/12/18 Fixed DAC latency calculation.
*       jm     03/12/18 Added support for reloading DTC scans.
*       jm     03/12/18 Add option to configure sysref capture after MTS.
* 4.0   sk     04/09/18 Added API to enable/disable the sysref.
*       rk     04/17/18 Adjust calculated latency by sysref period, where doing
*                       so results in closer alignment to the target latency.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xrfdc_mts.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static u32 XRFdc_MTS_Sysref_TRx(XRFdc* InstancePtr, u32 Enable);
static u32 XRFdc_MTS_Sysref_Ctrl(XRFdc* InstancePtr, u32 Type, u32 Tile_Id,
						u32 Is_PLL, u32 Enable_Cap, u32 Enable_Div_Reset);
static u32 XRFdc_MTS_Sysref_Dist(XRFdc* InstancePtr, int Num_DAC);
static u32 XRFdc_MTS_Sysref_Count(XRFdc* InstancePtr, u32 Type, u32 Count_Val);
static u32 XRFdc_MTS_Dtc_Scan (XRFdc* InstancePtr, u32 Type, u32 Tile_Id,
							XRFdc_MTS_DTC_Settings* Settings);
static u32 XRFdc_MTS_Dtc_Code (XRFdc* InstancePtr, u32 Type, u32 BaseAddr,
			u32 SRCtrlAddr, u32 DTCAddr, u16 SRctl, u16 SRclr_m, u32 Code);
static u32 XRFdc_MTS_Dtc_Calc (u32 Type, u32 Tile_Id,
						XRFdc_MTS_DTC_Settings* Settings, u8 *Flags);
static void XRFdc_MTS_Dtc_Flag_Debug(u8 *Flags, u32 Type, u32 Tile_Id,
								u32 Target, u32 Picked);
static void XRFdc_MTS_FIFOCtrl (XRFdc* InstancePtr, u32 Type, u32 FIFO_Mode,
								u32 Tiles_To_Clear);
static u32 XRFdc_MTS_GetMarker(XRFdc* InstancePtr, u32 Type, u32 Tiles,
							XRFdc_MTS_Marker* Markers, int Marker_Delay);
static u32 XRFdc_MTS_Marker_Read(XRFdc* InstancePtr, u32 Type, u32 Tile_Id,
							u32 FIFO_Id, u32 *Count, u32 *Loc, u32 *Done);
static u32 XRFdc_MTS_Latency(XRFdc* InstancePtr, u32 Type,
		XRFdc_MultiConverter_Sync_Config* Config, XRFdc_MTS_Marker* Markers);

/*****************************************************************************/
/**
*
* This API enables the master tile sysref Tx/Rx
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Enable the master tile sysref for Tx/Rx, valid values are 0 and 1.
*
* @return
*		- XRFDC_MTS_OK if successful.
*
* @note		None
*
******************************************************************************/
static u32 XRFdc_MTS_Sysref_TRx(XRFdc* InstancePtr, u32 Enable)
{
	u32 BaseAddr;
	u32 Data;

	BaseAddr = XRFDC_DRP_BASE(XRFDC_DAC_TILE, 0) + XRFDC_HSCOM_ADDR;
	Data = (Enable) ? 0xFFFF : 0;

	XRFdc_MTS_RMW_DRP(InstancePtr, BaseAddr, XRFDC_MTS_SRCAP_T1,
				XRFDC_MTS_SRCAP_EN_TRX_M, Data);

	return XRFDC_MTS_OK;
}

/*****************************************************************************/
/**
*
* This API Control SysRef Capture Settings
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Is_PLL Valid values are 0 and 1.
* @param	Enable_Cap Valid values are 0 and 1.
* @param	Enable_Div_Reset Valid values are 0 and 1.
*
* @return
*		- XRFDC_MTS_OK if successful.
*
* @note		None
*
******************************************************************************/
static u32 XRFdc_MTS_Sysref_Ctrl(XRFdc* InstancePtr, u32 Type, u32 Tile_Id,
					u32 Is_PLL, u32 Enable_Cap, u32 Enable_Div_Reset)
{
	u32 BaseAddr;
	u16 RegData;

	RegData = 0;
	BaseAddr = XRFDC_DRP_BASE(Type, Tile_Id) + XRFDC_HSCOM_ADDR;

	/* Write some bits to ensure sysref is in the right mode */
	XRFdc_MTS_RMW_DRP(InstancePtr, BaseAddr, XRFDC_MTS_SRCAP_T1,
			XRFDC_MTS_SRCAP_INIT_M, 0);

	if (Is_PLL) {
		/* PLL Cap */
		RegData = (Enable_Cap) ? XRFDC_MTS_SRCAP_PLL_M : 0;
		XRFdc_MTS_RMW_DRP(InstancePtr, BaseAddr, XRFDC_MTS_SRCAP_PLL,
				XRFDC_MTS_SRCAP_PLL_M, RegData);
	} else {
		/* Analog Cap disable */
		XRFdc_MTS_RMW_DRP(InstancePtr, BaseAddr, XRFDC_MTS_SRCAP_T1,
				XRFDC_MTS_SRCAP_T1_EN, 0);

		/* Analog Divider */
		RegData  = (Enable_Div_Reset) ? 0 : XRFDC_MTS_SRCAP_T1_RST;
		XRFdc_MTS_RMW_DRP(InstancePtr, BaseAddr, XRFDC_MTS_SRCAP_T1,
				XRFDC_MTS_SRCAP_T1_RST, RegData);

		/* Digital Divider */
		RegData  = (Enable_Div_Reset) ? 0 : XRFDC_MTS_SRCAP_DIG_M;
		XRFdc_MTS_RMW_DRP(InstancePtr, BaseAddr, XRFDC_MTS_SRCAP_DIG,
				XRFDC_MTS_SRCAP_DIG_M, RegData);

		/* Set SysRef Cap Clear */
		XRFdc_MTS_RMW_DRP(InstancePtr, BaseAddr, XRFDC_MTS_SRCAP_T1,
		XRFDC_MTS_SRCLR_T1_M, XRFDC_MTS_SRCLR_T1_M);

		/* Analog Cap enable */
		RegData  = (Enable_Cap) ? XRFDC_MTS_SRCAP_T1_EN : 0;
		XRFdc_MTS_RMW_DRP(InstancePtr, BaseAddr, XRFDC_MTS_SRCAP_T1,
				XRFDC_MTS_SRCAP_T1_EN, RegData);

		/* Unset SysRef Cap Clear */
		XRFdc_MTS_RMW_DRP(InstancePtr, BaseAddr, XRFDC_MTS_SRCAP_T1,
		XRFDC_MTS_SRCLR_T1_M, 0);
	}

	return XRFDC_MTS_OK;
}

/*****************************************************************************/
/**
*
* This API Update SysRef Distribution between tiles
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Num_DAC is number of DAC tiles
*
* @return
*		- XRFDC_MTS_OK if successful.
*		- XRFDC_MTS_NOT_SUPPORTED
*
* @note		None
*
******************************************************************************/
static u32 XRFdc_MTS_Sysref_Dist(XRFdc* InstancePtr, int Num_DAC)
{

	if (Num_DAC < 0) {
		/* Auto-detect. Only 2 types Supported - 2GSPS ADCs, 4GSPS ADCs */
		if (XRFdc_IsADC4GSPS(InstancePtr))
			Num_DAC = 2;
		else
			Num_DAC = 4;
	}

	if (Num_DAC == 2) {
		/* 2 DACs, 4ADCs */
		XRFdc_WriteReg16(InstancePtr, XRFDC_DAC_TILE_DRP_ADDR(0),
								XRFDC_MTS_SRDIST, 0xC980);
		XRFdc_WriteReg16(InstancePtr, XRFDC_DAC_TILE_DRP_ADDR(1),
								XRFDC_MTS_SRDIST, 0x0100);
		XRFdc_WriteReg16(InstancePtr, XRFDC_ADC_TILE_DRP_ADDR(3),
								XRFDC_MTS_SRDIST, 0x1700);
	} else if (Num_DAC == 4) {
		/* 4 DACs, 4ADCs */
		XRFdc_WriteReg16(InstancePtr, XRFDC_DAC_TILE_DRP_ADDR(0),
								XRFDC_MTS_SRDIST, 0xCA80);
		XRFdc_WriteReg16(InstancePtr, XRFDC_DAC_TILE_DRP_ADDR(1),
								XRFDC_MTS_SRDIST, 0x2400);
		XRFdc_WriteReg16(InstancePtr, XRFDC_DAC_TILE_DRP_ADDR(2),
								XRFDC_MTS_SRDIST, 0x0980);
		XRFdc_WriteReg16(InstancePtr, XRFDC_DAC_TILE_DRP_ADDR(3),
								XRFDC_MTS_SRDIST, 0x0100);
		XRFdc_WriteReg16(InstancePtr, XRFDC_ADC_TILE_DRP_ADDR(3),
								XRFDC_MTS_SRDIST, 0x0700);
	} else {
		return XRFDC_MTS_NOT_SUPPORTED;
	}

	XRFdc_WriteReg16(InstancePtr, XRFDC_ADC_TILE_DRP_ADDR(0),
								XRFDC_MTS_SRDIST, 0x0280);
	XRFdc_WriteReg16(InstancePtr, XRFDC_ADC_TILE_DRP_ADDR(1),
								XRFDC_MTS_SRDIST, 0x0600);
	XRFdc_WriteReg16(InstancePtr, XRFDC_ADC_TILE_DRP_ADDR(2),
								XRFDC_MTS_SRDIST, 0x8880);

	return XRFDC_MTS_OK;
}

/*****************************************************************************/
/**
*
* This API Wait for a number of sysref's to be captured
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Count_Val to wait for a number of sysref's to be captured.
*
* @return
*		- XRFDC_MTS_OK if successful.
*		- XRFDC_MTS_TIMEOUT if timeout occurs.
*
* @note		None
*
******************************************************************************/
static u32 XRFdc_MTS_Sysref_Count(XRFdc* InstancePtr, u32 Type, u32 Count_Val)
{
	u32 RegData;
	u32 Timeout;
	u32 Shift;

	RegData = (Type == XRFDC_DAC_TILE) ? 0x2 : 0x1;
	Shift   = (Type == XRFDC_DAC_TILE) ? 8 : 0;

	/* Start counter */
	XRFdc_WriteReg(InstancePtr, 0, XRFDC_MTS_SRCOUNT_CTRL, RegData);

	/* Check counter with timeout in case sysref is not active */
	Timeout = 0;
	while (Timeout < XRFDC_MTS_SRCOUNT_TIMEOUT) {
		RegData = XRFdc_ReadReg(InstancePtr, 0, XRFDC_MTS_SRCOUNT_VAL);
		RegData = ((RegData >> Shift) & XRFDC_MTS_SRCOUNT_M);
		if (RegData >= Count_Val) {
			break;
		}
		Timeout++;
	}

	if (Timeout >= XRFDC_MTS_SRCOUNT_TIMEOUT) {
		metal_log(METAL_LOG_ERROR,
            "PL SysRef Timeout - PL SysRef not active: %d\n in %s\n", Timeout,
            __func__);
		return XRFDC_MTS_TIMEOUT;
	}

	return XRFDC_MTS_OK;
}

/*****************************************************************************/
/**
*
* This API print the DTC scan results
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Target is for internal usage.
* @param	Picked is for internal usage.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void XRFdc_MTS_Dtc_Flag_Debug(u8 *Flags, u32 Type, u32 Tile_Id,
								u32 Target, u32 Picked)
{
	u32 i;
	char buf[XRFDC_MTS_NUM_DTC+1];

	for (i = 0; i < XRFDC_MTS_NUM_DTC; i++) {
		if (i == Picked) {
			buf[i] = '*';
		} else if (i == Target) {
			buf[i] = '#';
		} else {
			buf[i] = '0' + Flags[i];
		}
	}
	buf[XRFDC_MTS_NUM_DTC] = '\0';
	metal_log(METAL_LOG_INFO, "%s%d: %s\n",
			(Type == XRFDC_DAC_TILE) ? "DAC" : "ADC", Tile_Id, buf);

}

/*****************************************************************************/
/**
*
* This API Calculate the best DTC code to use
*
*
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Settings dtc settings structure.
* @param	Flags is for internal usage.
*
* @return
* 		- XRFDC_MTS_OK if successful.
*		- XRFDC_MTS_NOT_SUPPORTED if MTS is not supported.
*
* @note		None
*
******************************************************************************/
static u32 XRFdc_MTS_Dtc_Calc (u32 Type, u32 Tile_Id,
							XRFdc_MTS_DTC_Settings* Settings, u8 *Flags)
{
	u32 i;
	int Last;
	int Current_Gap;
	u32 Num_Found;
	int Min_Gap;
	int Max_Gap;
	int Diff;
	int Min_Diff;
	int Min_Range;
	int Val;
	int Target;
	int Max_Overlap;
	int Overlap_Cnt;
	int Min_Gap_Allowed;
	int Codes[XRFDC_MTS_MAX_CODE];
	int Status;

	Min_Gap_Allowed = (Settings->IsPLL) ? XRFDC_MTS_MIN_GAP_PLL :
							XRFDC_MTS_MIN_GAP_T1;
	Status = XRFDC_MTS_OK;

	/* Scan the flags and find candidate DTC codes */
	Num_Found = 0;
	Max_Gap = 0;
	Min_Gap = XRFDC_MTS_NUM_DTC;
	Max_Overlap = 0;
	Overlap_Cnt = 0;
	Last = -1;
	Flags[XRFDC_MTS_NUM_DTC] = 1;
	for (i = 0; i <= XRFDC_MTS_NUM_DTC; i++) {
		Current_Gap = i-Last;
		if (Flags[i] != 0) {
			if (Current_Gap > Min_Gap_Allowed) {
				Codes[Num_Found] = Last + Current_Gap / 2;
				Num_Found++;
				/* Record max/min gaps */
				Current_Gap--;
				if(Current_Gap > Max_Gap)
					Max_Gap = Current_Gap;
				if(Current_Gap < Min_Gap)
					Min_Gap = Current_Gap;
			}
			Last = i;
		}
		/* check for the longest run of overlapping codes */
		if (Flags[i] == 3) {
			Overlap_Cnt++;
			if (Overlap_Cnt > Max_Overlap)
				Max_Overlap = Overlap_Cnt;
		} else {
			Overlap_Cnt=0;
		}
	}

	/* Record some stats */
	Settings->Num_Windows[Tile_Id] = Num_Found;
	Settings->Max_Gap[Tile_Id]     = Max_Gap;
	Settings->Min_Gap[Tile_Id]     = Min_Gap;
	Settings->Max_Overlap[Tile_Id] = Max_Overlap;

	/* Calculate the best code */
	if (Settings->Scan_Mode == XRFDC_MTS_SCAN_INIT) {
		/* Initial scan */
		if (Tile_Id == Settings->RefTile) {
			/* RefTile: Get the code closest to the target */
			Target   = XRFDC_MTS_REF_TARGET;
			Settings->Target[Tile_Id] = XRFDC_MTS_REF_TARGET;
			Min_Diff = XRFDC_MTS_NUM_DTC;
			/* scan all codes to find the closest */
			for (i = 0; i < Num_Found; i++) {
				Diff = XRFDC_MTS_ABS(Target - Codes[i]);
				if (Diff < Min_Diff ) {
					Min_Diff = Diff;
					Settings->DTC_Code[Tile_Id] = Codes[i];
				}
				metal_log(METAL_LOG_DEBUG,
					"Target %d, DTC Code %d, Diff %d, Min %d\n", Target,
					Codes[i], Diff, Min_Diff);
			}
			/* set the reference code as the target for the other tiles */
			for (i = 0; i < 4; i++) {
				if (i != Tile_Id)
					Settings->Target[i] = Settings->DTC_Code[Tile_Id];
			}
			metal_log(METAL_LOG_DEBUG,
					"RefTile (%d): DTC Code Target %d, Picked %d\n", Tile_Id,
					Target, Settings->DTC_Code[Tile_Id]);

		} else {
			/*
			 *  Other Tiles: Get the code that minimises the total range of codes
			 *  compute the range of the existing dtc codes
			 */
			Max_Gap = 0;
			Min_Gap = XRFDC_MTS_NUM_DTC;
			for (i = 0; i < 4; i++) {
				Val = Settings->DTC_Code[i];
				if ((Val != -1) && (Val > Max_Gap))
					Max_Gap = Val;
				if ((Val != -1) && (Val < Min_Gap))
					Min_Gap = Val;
			}
			metal_log(METAL_LOG_DEBUG,
					"Tile (%d): Max/Min %d/%d, Range %d\n", Tile_Id, Max_Gap,
					Min_Gap, Max_Gap-Min_Gap);
			Min_Range = XRFDC_MTS_NUM_DTC;
			for (i = 0; i < Num_Found; i++) {
				Val = Codes[i];
				Diff = Max_Gap - Min_Gap;
				if (Val < Min_Gap)
					Diff = Max_Gap - Val;
				if (Val > Max_Gap)
					Diff = Val - Min_Gap;
				if (Diff <= Min_Range) {
					Min_Range = Diff;
					Settings->DTC_Code[Tile_Id] = Val;
				}
				metal_log(METAL_LOG_DEBUG,
					"Tile (%d): Code %d, New-Range: %d, Min-Range: %d\n",
					Tile_Id, Val, Diff, Min_Range);
			}
			metal_log(METAL_LOG_DEBUG,
					"Tile (%d): Code %d, Range Prev %d, New %d\n", Tile_Id,
					Settings->DTC_Code[Tile_Id], Max_Gap-Min_Gap, Min_Range);
		}
	} else {
		/* Reload the results of an initial scan to seed a new scan */
		if (Tile_Id == Settings->RefTile ) {
			/* RefTile: Get code closest to the target */
			Target = Settings->Target[Tile_Id];
		} else {
			Target = Settings->DTC_Code[Settings->RefTile] +
				Settings->Target[Tile_Id] - Settings->Target[Settings->RefTile];
		}
		Min_Diff = XRFDC_MTS_NUM_DTC;
		/* scan all codes to find the closest */
		for (i = 0; i < Num_Found; i++) {
			Diff = XRFDC_MTS_ABS(Target - Codes[i]);
			if (Diff < Min_Diff ) {
				Min_Diff = Diff;
				Settings->DTC_Code[Tile_Id] = Codes[i];
			}
			metal_log(METAL_LOG_DEBUG,
				"Reload Target %d, DTC Code %d, Diff %d, Min %d\n", Target,
				Codes[i], Diff, Min_Diff);
		}
	}

	/* Print some debug info */
	XRFdc_MTS_Dtc_Flag_Debug(Flags, Type, Tile_Id, Settings->Target[Tile_Id],
								Settings->DTC_Code[Tile_Id]);

	return Status;
}

/*****************************************************************************/
/**
*
* This API Set a DTC code and wait for it to be updated. Return early/late
* flags, if set
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	BaseAddr is for internal usage.
* @param	SRCtrlAddr is for internal usage.
* @param	DTCAddr is for internal usage.
* @param	SRctl is for internal usage.
* @param	SRclr_m is for internal usage.
* @param	Code is for internal usage.
*
* @return
* 		- XRFDC_MTS_OK if successful.
*		- XRFDC_MTS_TIMEOUT if timeout occurs.
*
* @note		None
*
******************************************************************************/
static u32 XRFdc_MTS_Dtc_Code (XRFdc* InstancePtr, u32 Type, u32 BaseAddr,
				u32 SRCtrlAddr, u32 DTCAddr, u16 SRctl, u16 SRclr_m, u32 Code)
{
	u32 Status;

	/* set the DTC code */
	XRFdc_WriteReg16(InstancePtr, BaseAddr, DTCAddr, Code);

	/* set sysref cap clear */
	XRFdc_WriteReg16(InstancePtr, BaseAddr, SRCtrlAddr, SRctl | SRclr_m);

	/* unset sysref cap clear */
	XRFdc_WriteReg16(InstancePtr, BaseAddr, SRCtrlAddr, SRctl);

	Status = XRFdc_MTS_Sysref_Count(InstancePtr, Type, XRFDC_MTS_DTC_COUNT);

	return Status;
}

/*****************************************************************************/
/**
*
* This API Scan the DTC codes and determine the optimal capture code for
* both PLL and T1 cases
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Settings dtc settings structure.
*
* @return
* 		- XRFDC_MTS_OK if successful.
*		- XRFDC_MTS_TIMEOUT if timeout occurs.
*
* @note		None
*
******************************************************************************/
static u32 XRFdc_MTS_Dtc_Scan (XRFdc* InstancePtr, u32 Type, u32 Tile_Id,
							XRFdc_MTS_DTC_Settings* Settings)
{
	u32 Status;
    u32 BaseAddr;
    u32 SRCtrlAddr;
    u32 DTCAddr;
    u8 Flags[XRFDC_MTS_NUM_DTC+1];
    u16 SRctl;
    u16 SRclr_m;
    u16 Flag_s;
    u32 i;

    BaseAddr = XRFDC_DRP_BASE(Type, Tile_Id) + XRFDC_HSCOM_ADDR;
    Status = XRFDC_MTS_OK;

	/*  Enable SysRef Capture and Disable Divide Reset */
    XRFdc_MTS_Sysref_Ctrl(InstancePtr, Type, Tile_Id, Settings->IsPLL, 1, 0);

    SRCtrlAddr = (Settings->IsPLL) ? XRFDC_MTS_SRCAP_PLL : XRFDC_MTS_SRCAP_T1;
    DTCAddr = (Settings->IsPLL) ? XRFDC_MTS_SRDTC_PLL : XRFDC_MTS_SRDTC_T1;
    SRclr_m = (Settings->IsPLL) ? XRFDC_MTS_SRCLR_PLL_M : XRFDC_MTS_SRCLR_T1_M;
    Flag_s = (Settings->IsPLL) ? XRFDC_MTS_SRFLAG_PLL : XRFDC_MTS_SRFLAG_T1;

    SRctl = XRFdc_ReadReg16(InstancePtr, BaseAddr, SRCtrlAddr) & ~SRclr_m;

    for (i = 0; i < XRFDC_MTS_NUM_DTC; i++)
	Flags[i] = 0;
    for (i = 0; (i < XRFDC_MTS_NUM_DTC) && (Status == XRFDC_MTS_OK); i++) {
	Status  |= XRFdc_MTS_Dtc_Code(InstancePtr, Type, BaseAddr,
					SRCtrlAddr, DTCAddr, SRctl, SRclr_m, i);
	Flags[i] = (XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_MTS_SRFLAG) >>
					Flag_s) & 0x3;
    }

	/* Calculate the best DTC code */
    XRFdc_MTS_Dtc_Calc(Type, Tile_Id, Settings, Flags);

	/* Program the calculated code */
    if ( Settings->DTC_Code[Tile_Id] == - 1 ) {
       metal_log(METAL_LOG_ERROR,
           "Unable to capture analog SysRef safely on %s tile %d\n"
           , (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id);
       Status |= XRFDC_MTS_DTC_INVALID;
    } else {
       XRFdc_MTS_Dtc_Code(InstancePtr, Type, BaseAddr, SRCtrlAddr, DTCAddr,
                          SRctl, SRclr_m, Settings->DTC_Code[Tile_Id]);
    }

    if (Settings->IsPLL) {
		/* PLL - Disable SysRef Capture */
	XRFdc_MTS_Sysref_Ctrl(InstancePtr, Type, Tile_Id, 1, 0, 0);
    } else {
	/* T1 - Reset Dividers */
	XRFdc_MTS_Sysref_Ctrl(InstancePtr, Type, Tile_Id, 0, 1, 1);
	Status |= XRFdc_MTS_Sysref_Count(InstancePtr, Type,
						XRFDC_MTS_DTC_COUNT);
	XRFdc_MTS_Sysref_Ctrl(InstancePtr, Type, Tile_Id, 0, 1, 0);
    }

    return Status;
}

/*****************************************************************************/
/**
*
* This API Control the FIFO enable for the group. If Tiles_to_clear has bits
* set, the FIFOs of those tiles will have their FIFO flags cleared.
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	FIFO_Mode is fifo mode.
* @param	Tiles_To_Clear bits set, FIFO flags will be cleared for those tiles.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void XRFdc_MTS_FIFOCtrl (XRFdc* InstancePtr, u32 Type, u32 FIFO_Mode,
							u32 Tiles_To_Clear)
{
	u32 RegAddr;
	u32 BaseAddr;
	int i;
	int j;

	/* Clear the FIFO Flags */
	RegAddr = (Type == XRFDC_ADC_TILE) ? XRFDC_ADC_FABRIC_ISR_OFFSET :
						XRFDC_DAC_FABRIC_ISR_OFFSET;
	for (i = 0; i < 4; i++) {
		if ((1 << i) & Tiles_To_Clear) {
			for (j = 0; j < 4; j++) {
				BaseAddr = XRFDC_DRP_BASE(Type, i) +
								XRFDC_BLOCK_ADDR_OFFSET(j);
				XRFdc_WriteReg16(InstancePtr, BaseAddr,	RegAddr,
								XRFDC_IXR_FIFOUSRDAT_MASK);
			}
		}
	}

	/* Enable the FIFOs */
	RegAddr = (Type == XRFDC_ADC_TILE) ? XRFDC_MTS_FIFO_CTRL_ADC :
						XRFDC_MTS_FIFO_CTRL_DAC;
	XRFdc_WriteReg(InstancePtr, 0, RegAddr, FIFO_Mode);
}

/*****************************************************************************/
/**
*
* This API Read-back the marker data for an ADC or DAC
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	FIFO_Id is FIFO number.
* @param	Count is for internal usage.
* @param	Loc is for internal usage.
* @param	Done is for internal usage.
*
* @return
* 		- XRFDC_MTS_OK if successful.
*
* @note		None
*
******************************************************************************/
static u32 XRFdc_MTS_Marker_Read(XRFdc* InstancePtr, u32 Type, u32 Tile_Id,
						u32 FIFO_Id, u32 *Count, u32 *Loc, u32 *Done)
{
	u32 BaseAddr;
	u32 RegData = 0x0;

	if (Type == XRFDC_ADC_TILE) {
		BaseAddr = XRFDC_DRP_BASE(Type, Tile_Id) - 0x2000;
		RegData  = XRFdc_ReadReg(InstancePtr, BaseAddr,
						XRFDC_MTS_ADC_MARKER_CNT+(FIFO_Id << 2));
		*Count = XRFDC_MTS_FIELD(RegData, XRFDC_MTS_AMARK_CNT_M, 0);
		*Loc   = XRFDC_MTS_FIELD(RegData, XRFDC_MTS_AMARK_LOC_M,
								XRFDC_MTS_AMARK_LOC_S);
		*Done  = XRFDC_MTS_FIELD(RegData, XRFDC_MTS_AMARK_DONE_M,
								XRFDC_MTS_AMARK_DONE_S);
	} else {
		BaseAddr = XRFDC_DRP_BASE(Type, Tile_Id) +
							XRFDC_BLOCK_ADDR_OFFSET(FIFO_Id);
		*Count = XRFdc_ReadReg(InstancePtr, BaseAddr,
								XRFDC_MTS_DAC_MARKER_CNT);
		*Loc   = XRFdc_ReadReg(InstancePtr, BaseAddr,
								XRFDC_MTS_DAC_MARKER_LOC);
		*Done  = 1;
	}
	metal_log(METAL_LOG_DEBUG,
		"Marker Read Tile %d, FIFO %d - %08X = %04X: count=%d, loc=%d,"
		"done=%d\n", Tile_Id, FIFO_Id, BaseAddr, RegData, *Count, *Loc, *Done);

	return XRFDC_MTS_OK;
}

/*****************************************************************************/
/**
*
* This API Run the marker counter and read the results
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tiles is tiles to get marker
* @param	Markers mts marker structure.
* @param	Marker_Delay is marker delay.
*
* @return
* 		- XRFDC_MTS_OK if successful.
* 		- XRFDC_MTS_TIMEOUT if timeout occurs.
* 		- XRFDC_MTS_MARKER_RUN
* 		- XRFDC_MTS_MARKER_MISM
* 		-
*
* @note		None
*
******************************************************************************/
static u32 XRFdc_MTS_GetMarker(XRFdc* InstancePtr, u32 Type, u32 Tiles,
						XRFdc_MTS_Marker* Markers, int Marker_Delay)
{
	u32 Done;
	u32 Count;
	u32 Loc;
	int i;
	int j;
	u32 Status;

	Status = XRFDC_MTS_OK;
	if (Type == XRFDC_ADC_TILE) {
		/* Reset marker counter */
		XRFdc_WriteReg(InstancePtr, 0, XRFDC_MTS_ADC_MARKER, 1);
		XRFdc_WriteReg(InstancePtr, 0, XRFDC_MTS_ADC_MARKER, 0);
	} else {
		/*
		 * SysRef Capture should be still active from the DTC Scan
		 * but set it anyway to be sure
		 */
		for (i = 0; i < 4; i++) {
			if ((1 << i) & Tiles)
				XRFdc_MTS_Sysref_Ctrl(InstancePtr, XRFDC_DAC_TILE,
						i, 0, 1, 0);
		}

		/* Set marker delay */
		XRFdc_WriteReg(InstancePtr, 0, XRFDC_MTS_DAC_MARKER_CTRL,
							Marker_Delay);
	}

	/* Allow the marker counter to run */
	Status |= XRFdc_MTS_Sysref_Count(InstancePtr, Type,
							XRFDC_MTS_MARKER_COUNT);

	/* Read master FIFO (FIFO0 in each Tile) */
	for (i = 0; i < 4; i++) {
		if ((1 << i) & Tiles) {
			if (Type == XRFDC_DAC_TILE) {
				/* Disable SysRef Capture before reading it */
				XRFdc_MTS_Sysref_Ctrl(InstancePtr, XRFDC_DAC_TILE,
									i, 0, 0, 0);
				Status |= XRFdc_MTS_Sysref_Count(InstancePtr, Type,
								XRFDC_MTS_MARKER_COUNT);
			}

			XRFdc_MTS_Marker_Read(InstancePtr, Type, i, 0, &Count,
								&Loc, &Done);
			Markers->Count[i] = Count;
			Markers->Loc[i]   = Loc;
			metal_log(METAL_LOG_INFO,
				"%s%d: Marker: - %d, %d\n", (Type==XRFDC_DAC_TILE)?
				"DAC":"ADC", i, Markers->Count[i], Markers->Loc[i]);

			if (!Done) {
               metal_log(METAL_LOG_ERROR,
               "Analog SysRef timeout, SysRef not detected on %s tile %d\n"
               , (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", i);
               Status |= XRFDC_MTS_MARKER_RUN;
            }

			/*
			 * Check all enabled FIFOs agree with the master FIFO.
			 * This is optional.
			 */
			for (j = 0; j < 4; j++) {
				if (XRFdc_IsFifoEnabled(InstancePtr, Type, i, j)) {
					XRFdc_MTS_Marker_Read(InstancePtr, Type, i, j,
								&Count, &Loc, &Done);
					if ((Markers->Count[i] != Count) ||
								(Markers->Loc[i] != Loc)) {
						metal_log(METAL_LOG_DEBUG,
							"Tile %d, FIFO %d Marker != Expected: %d, %d  vs"
							"%d, %d\n", i, j, Markers->Count[i],
							Markers->Loc[i], Count, Loc);
                        metal_log(METAL_LOG_ERROR,
                             "SysRef capture mismatch on %s tile %d,"
                             " PL SysRef may not have been"
                             " captured synchronously\n"
                             , (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", j);
						Status |= XRFDC_MTS_MARKER_MISM;

					}
				}
			}
		}
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This API Calculate the absoulte/relative latencies
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Config is mts config structure.
* @param	Markers is mts marker structure.
*
* @return
* 		- XRFDC_MTS_OK if successful.
* 		- XRFDC_MTS_DELAY_OVER
* 		- XRFDC_MTS_TARGET_LOW
* 		-
*
* @note     Latency calculation will use Sysref frequency counters
*           logic which will work with IP version 2.0.1 and above.
*
******************************************************************************/
static u32 XRFdc_MTS_Latency(XRFdc* InstancePtr, u32 Type,
		XRFdc_MultiConverter_Sync_Config* Config, XRFdc_MTS_Marker* Markers)
{
	u32 Status;
	int Count_w;
	int Loc_w;
	int i;
	int Fifo;
	int Latency;
	int Offset;
	int Max_Latency;
	int Target;
	int Delta;
	int i_part;
	int f_part;
	u32 Read_Words;
	u32 Factor;
	u32 BaseAddr;
	u32 RegAddr;
	u32 RegData;
	u32 Write_Words;
	u32 SysRefFreqCntrDone;
	int SysRefT1Period;
	int Target_Latency = -1;
	int LatencyDiff;
	int LatencyOffset;
	int LatencyOffsetDiff;

	Status = XRFDC_MTS_OK;
	if (Type == XRFDC_ADC_TILE) {
		XRFdc_GetDecimationFactor(InstancePtr, Config->RefTile, 0, &Factor);
	} else {
		XRFdc_GetInterpolationFactor(InstancePtr, Config->RefTile, 0, &Factor);
		XRFdc_GetFabWrVldWords(InstancePtr, Type, Config->RefTile, 0, &Write_Words);
	}
    XRFdc_GetFabRdVldWords(InstancePtr, Type, Config->RefTile, 0, &Read_Words);
    Count_w = Read_Words * Factor;
    Loc_w   = Factor;

    metal_log(METAL_LOG_DEBUG,
			"Count_w %d, loc_w %d\n", Count_w, Loc_w);

    /* Find the individual latencies */
    Max_Latency=0;

    /* Determine relative SysRef frequency */
    RegData = XRFdc_ReadReg(InstancePtr, 0, XRFDC_MTS_SRFREQ_VAL);
    if (Type == XRFDC_ADC_TILE) {
		/* ADC SysRef frequency information contained in lower 16 bits */
		RegData = RegData & 0XFFFF;
    } else {
		/* DAC SysRef frequency information contained in upper 16 bits */
		RegData = (RegData >> 16) & 0XFFFF;
    }

    /* 
     * Ensure SysRef frequency counter has completed.
     * Sysref frequency counters logic will work with IP version
     * 2.0.1 and above.
     */
    SysRefFreqCntrDone = RegData & 0x1;
    if (SysRefFreqCntrDone == 0U) {
		metal_log(METAL_LOG_ERROR, "Error : %s SysRef frequency counter not yet done\n",
			(Type == XRFDC_ADC_TILE) ? "ADC" : "DAC");
		Status |= XRFDC_MTS_SYSREF_FREQ_NDONE;
		/* Set SysRef period in terms of T1's will not be used */
		SysRefT1Period = 0;
    } else {
		SysRefT1Period = (RegData >> 1) * Count_w;
		if (Type == XRFDC_DAC_TILE) {
			/*
			 * DAC marker counter is on the tile clock domain so need
			 * to update SysRef period accordingly
			 */
			SysRefT1Period = (SysRefT1Period * Write_Words) / Read_Words;
		}
		metal_log(METAL_LOG_INFO, "SysRef period in terms of %s T1s = %d\n",
			(Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", SysRefT1Period);
    }

    /* Work out the latencies */
    for (i = 0; i < 4; i++) {
		if ((1 << i) & Config->Tiles ) {
			Latency = (Markers->Count[i] * Count_w) + (Markers->Loc[i] * Loc_w);
			/* Set marker counter target on first tile */
			if (Target_Latency < 0) {
				Target_Latency = Config->Target_Latency;
				if (Target_Latency < 0)
					Target_Latency = Latency;
				metal_log(METAL_LOG_INFO, "%s target latency = %d\n",
					(Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Target_Latency);
			}

			/*
			 * Adjust reported counter values if offsetting by a SysRef
			 * period reduces distance between current and target latencies
			 */
			LatencyDiff = Target_Latency - Latency;
			LatencyOffset = (LatencyDiff > 0) ? (Latency + SysRefT1Period) :
					(Latency - SysRefT1Period);
			LatencyOffsetDiff = Target_Latency - LatencyOffset;
			if (abs(LatencyDiff) > abs(LatencyOffsetDiff)) {
				Latency = LatencyOffset;
				metal_log(METAL_LOG_INFO, "%s%d latency offset by a SysRef period to %d\n",
					(Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", i, Latency);
			}
			Config->Latency[i] = Latency;
			if (Latency > Max_Latency)
				Max_Latency = Latency;
			metal_log(METAL_LOG_DEBUG, "Tile %d, latency %d, max %d\n",
					i, Latency, Max_Latency);
		}
	}

	/*
	 * Adjust the latencies to meet the target. Choose max, if it
	 * is not supplied by the user.
	 */
	Target = (Config->Target_Latency < 0) ? Max_Latency :
							Config->Target_Latency;

	if (Target < Max_Latency) {
		/* Cannot correct for -ve latencies, so default to aligning */
		Target = Max_Latency;
		metal_log(METAL_LOG_ERROR, "Error : %s alignment target latency of %d < minimum possible %d\n",
				(Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Target, Max_Latency);
		Status |= XRFDC_MTS_TARGET_LOW;
	}

	for (i = 0; i < 4; i++) {
		if ((1 << i) & Config->Tiles) {
			Delta = Target - Config->Latency[i];
			if (Delta < 0)
				Delta=0;
			i_part = Delta / Factor;
			f_part = Delta % Factor;
			Offset = i_part;
			if (f_part > (int)(Factor / 2))
				Offset++;
			metal_log(METAL_LOG_DEBUG,
				"Target %d, Tile %d, delta %d, i/f_part %d/%d, offset %d\n",
				Target, i, Delta, i_part, f_part, Offset*Factor);

			/* check for excessive delay correction values */
			if (Offset > (int)XRFDC_MTS_DELAY_MAX) {
				Offset  = (int)XRFDC_MTS_DELAY_MAX;
				metal_log(METAL_LOG_ERROR,
                          "Alignment correction delay %d"
                          " required exceeds maximum for %s Tile %d\n",
                          Offset, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC",
                          XRFDC_MTS_DELAY_MAX, i);
				Status |= XRFDC_MTS_DELAY_OVER;
			}

			/* Adjust the latency, write the same value to each FIFO */
			BaseAddr = XRFDC_DRP_BASE(Type, i) - 0x2000;
			for (Fifo = 0; Fifo < 4; Fifo++) {
				RegAddr  = XRFDC_MTS_DELAY_CTRL + (Fifo << 2);
				RegData  = XRFdc_ReadReg(InstancePtr, BaseAddr, RegAddr);
				RegData  = XRFDC_MTS_RMW(RegData, XRFDC_MTS_DELAY_VAL_M,
										Offset);
				XRFdc_WriteReg(InstancePtr, BaseAddr, RegAddr, RegData);
			}

			/* Report the total latency for this tile */
			Config->Latency[i] = Config->Latency[i] + (Offset * Factor);
			Config->Offset[i]  = Offset;

			/* Set the Final SysRef Capture Enable state */
			XRFdc_MTS_Sysref_Ctrl(InstancePtr, Type, i, 0, Config->SysRef_Enable, 0);
		}
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This API is used to enable/disable the sysref.
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	DACSyncConfig is pointer to DAC Multi-Tile Sync config structure.
* @param	ADCSyncConfig is pointer to ADC Multi-Tile Sync config structure.
* @param	SysRefEnable valid values are 0(disable) and 1(enable).
*
* @return
* 		- XRFDC_MTS_OK if successful.
*
* @note		None
*
******************************************************************************/
u32 XRFdc_MTS_Sysref_Config(XRFdc* InstancePtr,
			XRFdc_MultiConverter_Sync_Config* DACSyncConfig,
			XRFdc_MultiConverter_Sync_Config* ADCSyncConfig, u32 SysRefEnable)
{
	u32 Status;
	int Tile;

	/* Enable/disable SysRef Capture on all DACs participating in MTS */
	for (Tile = 0; Tile < 4; Tile++) {
		if((1 << Tile) & DACSyncConfig->Tiles)
			XRFdc_MTS_Sysref_Ctrl(InstancePtr, XRFDC_DAC_TILE, Tile, 0,
								SysRefEnable, 0);
	}

	/* Enable/Disable SysRef Capture on all ADCs participating in MTS */
	for (Tile = 0; Tile < 4; Tile++) {
		if((1 << Tile) & ADCSyncConfig->Tiles)
			XRFdc_MTS_Sysref_Ctrl(InstancePtr, XRFDC_ADC_TILE, Tile, 0,
								SysRefEnable, 0);
	}

	/* Enable/Disable SysRef TRX */
	Status = XRFdc_MTS_Sysref_TRx(InstancePtr, SysRefEnable);

	return Status;
}

/*****************************************************************************/
/**
*
* This API Initializes the multi-tile sync config structures.
* Optionally allows target codes to be provided for the Pll/T1
* analog sysref capture
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Config Multi-tile sync config structure.
* @param	PLL_Codes PLL analog sysref capture.
* @param	T1_Codes T1 analog sysref capture.
*
* @return	None
*
* @note		None
*
******************************************************************************/
void XRFdc_MultiConverter_Init (XRFdc_MultiConverter_Sync_Config* Config,
					int *PLL_Codes, int *T1_Codes)
{
	u32 i;

	Config->RefTile = 0;
	Config->DTC_Set_PLL.Scan_Mode = (PLL_Codes == 0) ? XRFDC_MTS_SCAN_INIT :
										XRFDC_MTS_SCAN_RELOAD;
	Config->DTC_Set_T1.Scan_Mode = (T1_Codes == 0) ? XRFDC_MTS_SCAN_INIT :
										XRFDC_MTS_SCAN_RELOAD;
	Config->DTC_Set_PLL.IsPLL = 1;
	Config->DTC_Set_T1 .IsPLL = 0;
	Config->Target_Latency = -1;
	Config->Marker_Delay = 15;
	Config->SysRef_Enable = 1; /* By default enable Sysref capture after MTS */

	/* Initialize variables per tile */
	for (i = 0; i < 4; i++) {
		if (PLL_Codes != 0)
			Config->DTC_Set_PLL.Target[i] = PLL_Codes[i];
		else
			Config->DTC_Set_PLL.Target[i] = 0;
		if (T1_Codes  != 0)
			Config->DTC_Set_T1.Target[i] = T1_Codes[i];
		else
			Config->DTC_Set_T1.Target[i] = 0;

		Config->DTC_Set_PLL.DTC_Code[i] = -1;
		Config->DTC_Set_T1.DTC_Code[i] = -1;
	}

}

/*****************************************************************************/
/**
*
* This is the top level API which will be used for Multi-tile
* Synchronization.
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Config Multi-tile sync config structure.
*
* @return
* 		- XRFDC_MTS_OK if successful.
*		- XRFDC_MTS_TIMEOUT if timeout occurs.
* 		- XRFDC_MTS_MARKER_RUN
* 		- XRFDC_MTS_MARKER_MISM
* 		- XRFDC_MTS_NOT_SUPPORTED if MTS is not supported.
*
* @note		None
*
******************************************************************************/
u32 XRFdc_MultiConverter_Sync (XRFdc* InstancePtr, u32 Type,
						XRFdc_MultiConverter_Sync_Config* Config)
{
	u32 Status;
	u32 i;
	u32 RegData;
	XRFdc_IPStatus IPStatus;
	XRFdc_MTS_Marker Markers;
	u32 BaseAddr;
	u32 TileState;

	Status = XRFDC_MTS_OK;

	XRFdc_GetIPStatus(InstancePtr, &IPStatus);
	for (i = 0; i < 4; i++) {
		if (Config->Tiles & (1 << i)) {
			TileState = (Type == XRFDC_DAC_TILE) ?
							 IPStatus.DACTileStatus[i].TileState :
							 IPStatus.ADCTileStatus[i].TileState ;
			if(TileState != 0xF) {
				metal_log(METAL_LOG_ERROR,
				    "%s tile %d in Multi-Tile group not started\n",
                    (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", i);
				Status |= XRFDC_MTS_IP_NOT_READY;
			}
            BaseAddr = XRFDC_DRP_BASE(Type, i) - XRFDC_TILE_DRP_OFFSET;
            RegData  = XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_MTS_DLY_ALIGNER);
			if (RegData == 0U) {
				metal_log(METAL_LOG_ERROR,"%s tile %d is not enabled for MTS, check IP configuration\n",
					(Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", i);
				Status |= XRFDC_MTS_NOT_ENABLED;
			}
		}
	}
	if(Status != XRFDC_MTS_OK) return Status;

	/* Disable the FIFOs */
	XRFdc_MTS_FIFOCtrl(InstancePtr, Type, XRFDC_MTS_FIFO_DISABLE, 0);

	/* Enable SysRef Rx */
	Status |= XRFdc_MTS_Sysref_TRx(InstancePtr, 1);

	/* Update distribution */
	Status |= XRFdc_MTS_Sysref_Dist(InstancePtr, -1);

	/* Scan DTCs for each tile */
	for (i = 0; i < 4; i++) {
		if (Config->Tiles & (1 << i)) {
			/* Run DTC Scan for T1/PLL */
			BaseAddr = XRFDC_DRP_BASE(Type, i) + XRFDC_HSCOM_ADDR;
			RegData  = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_MTS_CLKSTAT);
			if (RegData & XRFDC_MTS_PLLEN_M) {
				/* DTC Scan PLL */
				if(i==0) {
					metal_log(METAL_LOG_INFO, "\nDTC Scan PLL\n", 0);
				}
				Config->DTC_Set_PLL.RefTile = Config->RefTile;
				Status |= XRFdc_MTS_Dtc_Scan(InstancePtr, Type, i,
									&Config->DTC_Set_PLL);
			}
		}
	}

	/* Scan DTCs for each tile T1 */
	metal_log(METAL_LOG_INFO, "\nDTC Scan T1\n", 0);
	for (i = 0; i < 4; i++) {
		if (Config->Tiles & (1 << i)) {
			Config->DTC_Set_T1 .RefTile = Config->RefTile;
			Status |= XRFdc_MTS_Dtc_Scan(InstancePtr, Type, i,
									&Config->DTC_Set_T1);
		}
	}

	/* Enable FIFOs */
	XRFdc_MTS_FIFOCtrl(InstancePtr, Type, XRFDC_MTS_FIFO_ENABLE,
									Config->Tiles);

	/* Measure latency */
	Status |= XRFdc_MTS_GetMarker(InstancePtr, Type, Config->Tiles, &Markers,
									Config->Marker_Delay);

	/* Calculate latency difference and adjust for it */
	Status |= XRFdc_MTS_Latency(InstancePtr, Type, Config, &Markers);

	return Status;
}
