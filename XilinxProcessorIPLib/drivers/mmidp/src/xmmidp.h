/*******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xmmidp.h
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   ck   03/14/25 Initial Release.
 * </pre>
 *
*******************************************************************************/
#ifndef __XMMIDP_H__
#define __XMMIDP_H__

/******************************* Include Files ********************************/
#include "xmmidp_hw.h"
#include <xvidc.h>

#define XMMIDP_MAX_LANES	4
#define XMMIDP_MAX_EDID_READ	128

/****************************** Type Definitions ******************************/

typedef struct {
	u32 BaseAddr;
} XMmiDp_Config;

/**
 * This typedef describes PHY LANES
 */
typedef enum {
	PHY_LANES_1 = 0x0,
	PHY_LANES_2 = 0x1,
	PHY_LANES_4 = 0x2,
} XMmiDp_PhyLanes;

/**
 * This typedef describes PHY RATE
 */
typedef enum {
	PHY_RATE_RBR_162GBPS  = 0x0,
	PHY_RATE_HBR_270GBPS  = 0x1,
	PHY_RATE_HBR2_540GBPS = 0x2,
	PHY_RATE_HBR3_810GBPS = 0x3,
} XMmiDp_PhyRate;

/**
 * This typedef describes PHY TrainingPattern
 */
typedef enum {
	PHY_NO_TRAIN 		= 0x0,
	PHY_TPS1 		= 0x1,
	PHY_TPS2 		= 0x2,
	PHY_TPS3 		= 0x3,
	PHY_TPS4 		= 0x4,
	PHY_SYMBOL_ERR_RATE 	= 0x5,
	PHY_PRBS7 		= 0x6,
	PHY_CUSTOMPAT 		= 0x7,
	PHY_CP2520_PAT_1 	= 0x8,
	PHY_CP2520_PAT_2 	= 0x9,
} XMmiDp_PhyTrainingPattern;

/**
 * This typedef describes PHY VSWING Level
 */
typedef enum {
	PHY_VSWING_LEVEL0 = 0x0,
	PHY_VSWING_LEVEL1 = 0x1,
	PHY_VSWING_LEVEL2 = 0x2,
	PHY_VSWING_LEVEL3 = 0x3,
} XMmiDp_PhyVSwing;

/**
 * This typedef describes PHY PREEMP Level
 */
typedef enum {
	PHY_PREEMP_LEVEL0 = 0x0,
	PHY_PREEMP_LEVEL1 = 0x1,
	PHY_PREEMP_LEVEL2 = 0x2,
	PHY_PREEMP_LEVEL3 = 0x3,
} XMmiDp_PhyPreEmp;

/**
 * This typedef describes PHY XMIT ENABLE
 */
typedef enum {
	PHY_XMIT_EN_LANE0 = 0x1,
	PHY_XMIT_EN_LANE1 = 0x2,
	PHY_XMIT_EN_LANE2 = 0x4,
	PHY_XMIT_EN_LANE3 = 0x8,
} XMmiDp_PhyXmitEn;

/**
 * This typedef describes PHY BUSY
 */
typedef enum {
	PHY_BUSY_LANE0 = 0x1,
	PHY_BUSY_LANE1 = 0x2,
	PHY_BUSY_LANE2 = 0x4,
	PHY_BUSY_LANE3 = 0x8,
} XMmiDp_PhyBusy;

/**
 * This typedef describes PHY POWERDOWN CTRL
 */
typedef enum {
	PHY_POWER_ON 	   = 0x0,
	PHY_INTER_P2_POWER = 0x2,
	PHY_POWER_DOWN 	   = 0x3,
	PHY_POWER_STATE    = 0xC,
} XMmiDp_PhyPwrDown;

/**
 * This typedef describes PHY POWERDOWN CTRL
 */
typedef enum {
	PHY_20BIT = 0x0,
	PHY_40BIT = 0x1,
} XMmiDp_PhyWidth;

/**
 * This typedef enumerates the list of training states used in the state machine
 * during the link training process.
 */
typedef enum {
	XMMIDP_TS_CLOCK_RECOVERY,
	XMMIDP_TS_CHANNEL_EQUALIZATION,
	XMMIDP_TS_ADJUST_LINK_RATE,
	XMMIDP_TS_ADJUST_LANE_COUNT,
	XMMIDP_TS_FAILURE,
	XMMIDP_TS_SUCCESS,
} XMmiDp_TrainingState;

/**
 * This typedef describes Pixels per Clk
 */
typedef enum {
	XMMIDP_SINGLE_PIX_MODE   = 0x0,
	XMMIDP_DUAL_PIX_MODE     = 0x1,
	XMMIDP_QUAD_PIX_MODE     = 0x2,
	XMMIDP_RESERVED_PIX_MODE = 0x3,
} XMmiDp_PPC;

/**
 * This typedef gets Video Input mapping
 */
typedef enum {
	XMMIDP_RGB_6BPC 	= 0x0,
	XMMIDP_RGB_8BPC 	= 0x1,
	XMMIDP_RGB_10BPC 	= 0x2,
	XMMIDP_RGB_12BPC 	= 0x3,
	XMMIDP_RGB_16BPC 	= 0x4,
	XMMIDP_YCbCr444_8BPC 	= 0x5,
	XMMIDP_YCbCr444_10BPC 	= 0x6,
	XMMIDP_YCbCr444_12BPC 	= 0x7,
	XMMIDP_YCbCr444_16BPC 	= 0x8,
	XMMIDP_YCbCr422_8BPC 	= 0x9,
	XMMIDP_YCbCr422_10BPC   = 0xA,
	XMMIDP_YCbCr422_12BPC   = 0xB,
	XMMIDP_YCbCr422_16BPC   = 0xC,
	XMMIDP_YCbCr420_8BPC    = 0xD,
	XMMIDP_YCbCr420_10BPC   = 0xE,
	XMMIDP_YCbCr420_12BPC   = 0xF,
	XMMIDP_YCbCr420_16BPC   = 0x10,
	XMMIDP_Y_8BPC 		= 0x11,
	XMMIDP_Y_10BPC 		= 0x12,
	XMMIDP_Y_12BPC 		= 0x13,
	XMMIDP_Y_16BPC 		= 0x14,
	XMMIDP_RAW_8BPC 	= 0x17,
	XMMIDP_RAW_10BPC 	= 0x18,
	XMMIDP_RAW_12BPC 	= 0x19,
	XMMIDP_RAW_16BPC 	= 0x1B,
} XMmiDp_VidMap;

typedef struct {
	u8 LaneCount;
	u8 NumLanes;
	u8 LinkRate;
	u8 LinkBW;
	u8 TrainingPattern;
	u8 *VsLevel;
	u8 *PeLevel;
	u8 XmitEn;
	u8 ScrambleEn;
	u8 SSCEn;
	u8 PwrDown;
} XMmiDp_PhyConfig;

typedef struct {
	u8 Edid[XMMIDP_MAX_EDID_READ];
	u8 EdidNext[XMMIDP_MAX_EDID_READ];
	u8 MaxLaneCount;
	u8 MaxNumLanes;
	u8 EnhancedFrameCap;
	u8 PostLtAdjReqSupported;
	u8 Tps3Supported;
	u8 MaxLinkRate;
	u8 MaxLinkBW;
	u32 TrainingAuxRdInterval;
	u8 LaneStatusAdjReqs[6];
	u8 Tps4Supported;
	u8 NoAuxLinkTraining;
	u8 MaxDownspread;
	u8 DpcdRev;
	u8 ExtendedReceiverCap;
} XMmiDp_RxConfig;

typedef struct {
	u8 LaneCount;
	u8 NumLanes;
	u8 LinkRate;
	u8 LinkBW;
	u8 FastLinkTrainEn;
	u8 VsLevel[XMMIDP_MAX_LANES];
	u8 PeLevel[XMMIDP_MAX_LANES];
	u8 SpreadAmp;
	u8 ChannelCodingSet;
	u8 CrDoneCnt;
	u8 CrDoneOldState;
	u8 ScrambleEn;
	u8 VsLevelUpdated;
	u8 PeLevelUpdated;
} XMmiDp_LinkConfig;

/**
 * This typedef contains the main stream attributes which determine how the
 * video will be displayed.
 */
typedef struct {
	XVidC_VideoTimingMode Vtm;
	u8 BitsPerColor;
	u8 PixelClockHz;
	u32 HStart;
	u32 VStart;
	u32 Misc0;
	u32 MVid;
	u32 Misc1;
	u32 NVid;
	u32 HBlankInterval;
} XMmiDp_MainStreamAttributes;

/**
 * typedef contains VideoConfig values
 */
typedef struct {
	u8 DeInPolarity;
	u8 HSyncInPolarity;
	u8 VSyncInPolarity;
	u16 HActive;
	u16 HBlank;
	u8 I_P;
	u8 RVBlankInOsc;
	u16 VBlank;
	u16 VActive;
	u16 HSyncWidth;
	u16 VSyncWidth;
	u8 InitThresholdHi;
	u8 En3DFrameFieldSeq;
	u8 AvgBytesPerTuFrac;
	u8 InitThreshold;
	u8 AvgBytesPerTu;
} XMmiDp_VideoConfig;

typedef struct {
	u8 LinkUpdPps;
	u8 StreamType;
	u8 EncryptionEnable;
	u8 DscEn;
	XMmiDp_PPC PixModeSel;
	XMmiDp_VidMap VideoMapping;
	u8 VidMapIpiEn;
	u8 VidStreamEn;
	u8 BcbStuffData;
	u8 GyDataStuffEn;
	u8 RcrDataStuffEn;
	u8 BcbDataStuffEn;
	u8 GyStuffData;
	u8 RcrStuffData;
} XMmiDp_VSampleCtrl;

typedef void (*XMmiDp_HpdIrqHandler)(void *InstancePtr);
typedef void (*XMmiDp_HpdHotPlugHandler)(void *InstancePtr);

/**
 * The XMmiDp driver instance data. The user is required to allocate a variable
 * of this type for every XMmiDp device in the system. A pointer to a variable of
 * this type is then passed to the driver API functions.
 */
typedef struct {

	u32 AuxDelayUs;
	XMmiDp_Config Config;
	XMmiDp_PhyConfig PhyConfig;
	XMmiDp_RxConfig RxConfig;
	XMmiDp_LinkConfig LinkConfig;
	XMmiDp_VideoConfig VideoConfig[XMMIDP_MAX_LANES];
	XMmiDp_VSampleCtrl VSampleCtrl[XMMIDP_MAX_LANES];
	XMmiDp_MainStreamAttributes MsaConfig[XMMIDP_MAX_LANES];
	XMmiDp_HpdIrqHandler HpdIrqHandler;
	void *HpdIrqCallbackRef;
	XMmiDp_HpdHotPlugHandler HpdHotPlugHandler;
	void *HpdHotPlugCallbackRef;

} XMmiDp;

/**************************** Function Prototypes *****************************/
/* CfgInitialize */
void XMmiDp_CfgInitialize(XMmiDp *InstancePtr, u32 BaseAddr);
void XMmiDp_Initialize(XMmiDp *InstancePtr);

/* AUX Transactions */
u32 XMmiDp_AuxWrite(XMmiDp *InstancePtr, u32 DpcdAddr, u32 Bytes, void *Data);
u32 XMmiDp_AuxRead(XMmiDp *InstancePtr, u32 DpcdAddr, u32 Bytes, void *Data);
u32 XMmiDp_I2cWrite(XMmiDp *InstancePtr, u32 I2cAddr, u32 Bytes, void *Data);
u32 XMmiDp_I2cRead(XMmiDp *InstancePtr, u32 I2cAddr, u16 Offset, u32 Bytes, void *Data);

void XMmiDp_RegReadModifyWrite(XMmiDp *InstancePtr, u32 RegOffset, u32
			       Mask, u32 Shift, u32 Val);
void XMmiDp_DpcdReadModifyWrite(XMmiDp *InstancePtr, u32 RegOffset, u32
				Mask, u32 Shift, u32 Val);
/* Phy Control Config */

int GetLaneCount(XMmiDp *InstancePtr, u8 NumLanes);
int GetNumLanes(XMmiDp *InstancePtr, u8 LaneCount);
void XMmiDp_SetPhyLaneCount(XMmiDp *InstancePtr, XMmiDp_PhyLanes);
int GetLinkRate(XMmiDp *InstancePtr, u8 LinkBw);
int GetLinkBW(XMmiDp *InstancePtr, u8 LinkRate);
void XMmiDp_SetPhyLinkRate(XMmiDp *InstancePtr, XMmiDp_PhyRate LinkRate);
void XMmiDp_SetPhyTrainingPattern(XMmiDp *InstancePtr,
				  XMmiDp_PhyTrainingPattern TrainingPattern);
void XMmiDp_SetPhyVoltageSwing(XMmiDp *InstancePtr, u8 *VsLevel);
void XMmiDp_SetPhyPreEmphasis(XMmiDp *InstancePtr, u8 *PeLevel);
void XMmiDp_SetPhyXmitEnable(XMmiDp *InstancePtr);
void XMmiDp_SetPhyXmitDisable(XMmiDp *InstancePtr);
void XMmiDp_PhyScrambleEnable(XMmiDp *InstancePtr);
void XMmiDp_PhyScrambleDisable(XMmiDp *InstancePtr);
void XMmiDp_PhySoftReset(XMmiDp *InstancePtr);
void XMmiDp_PhySSCEnable(XMmiDp *InstancePtr);
void XMmiDp_PhySSCDisable(XMmiDp *InstancePtr);
u32 XMmiDp_PhyWaitReady(XMmiDp *InstancePtr);
void XMmiDp_SetPhyPowerdown(XMmiDp *InstancePtr, XMmiDp_PhyPwrDown Control);
void XMmiDp_SetPhyWidth(XMmiDp *InstancePtr, XMmiDp_PhyWidth Width);
void XMmiDp_SetAux_250Us_Cnt_Limit(XMmiDp *InstancePtr, u16 Limit);
void XMmiDp_SetAux_2000Us_Cnt_Limit(XMmiDp *InstancePtr, u16 Limit);
void XMmiDp_SetAux_100000Us_Cnt_Limit(XMmiDp *InstancePtr, u16 Limit);
void XMmiDp_SetPmConfig1(XMmiDp *InstancePtr, u32 Val);
void XMmiDp_SetPmConfig2(XMmiDp *InstancePtr, u32 Val);

/* HPD */
u32 XMmiDp_IsConnected(XMmiDp *InstancePtr);
void XMmiDp_SetHpdIrqHandler(XMmiDp *InstancePtr, XMmiDp_HpdIrqHandler CallbackFun,
			     void *CallbackRef);
void XMmiDp_SetHpdHotPlugHandler(XMmiDp *InstancePtr, XMmiDp_HpdHotPlugHandler CallbackFun,
				 void *CallbackRef);

void XMmiDp_HpdInterruptHandler(XMmiDp *InstancePtr);

/* EDID */
u32 XMmiDp_GetEdidBlock(XMmiDp *InstancePtr, u8 *Data, u8 BlockNum);

/* Interrupt Enable */
void XMmiDp_GeneralInterruptEnable(XMmiDp *InstancePtr, u32 Mask);
void XMmiDp_HpdInterruptEnable(XMmiDp *InstancePtr, u32 Mask);

/* DPCD */
void XMmiDp_SetDpcdLaneCount(XMmiDp *InstancePtr);
void XMmiDp_SetDpcdLinkRate(XMmiDp *InstancePtr);
void XMmiDp_SetDpcdLinkQualPattern(XMmiDp *InstancePtr,
				   XMmiDp_PhyTrainingPattern Pattern);
void XMmiDp_SetDpcdTrainingPattern(XMmiDp *InstancePtr,
				   XMmiDp_PhyTrainingPattern Pattern);
void XMmiDp_SetDpcdVoltageSwing(XMmiDp *InstancePtr, XMmiDp_PhyVSwing *VsLevel);
void XMmiDp_SetDpcdPreEmphasis(XMmiDp *InstancePtr, XMmiDp_PhyPreEmp *PeLevel);
u32 XMmiDp_GetRxMaxLaneCount(XMmiDp *InstancePtr);
u32 XMmiDp_GetRxMaxLinkRate(XMmiDp *InstancePtr);
u32 XMmiDp_SetSinkDpcdLinkCfgField(XMmiDp *InstancePtr);
void XMmiDp_GetDpcdTrainingAuxRdInterval(XMmiDp *InstancePtr);
u32 XMmiDp_GetDpcdLaneStatusAdjReqs(XMmiDp *InstancePtr);

/* Link Training */
void XMmiDp_FastLinkTrainEnable(XMmiDp *InstancePtr);
void XMmiDp_FastLinkTrainDisable(XMmiDp *InstancePtr);
u32 XMmiDp_StartLinkXmit(XMmiDp *InstancePtr);
u32 XMmiDp_SetSinkDpcdLinkCfgField(XMmiDp *InstancePtr);
void XDpPSu14_EnableCctlEnhanceFraming(XMmiDp *InstancePtr);
void XDpPSu14_DisableCctlEnhanceFraming(XMmiDp *InstancePtr);
u32 XMmiDp_GetTrainingDelay(XMmiDp *InstancePtr);
u32 XMmiDp_CheckClockRecovery(XMmiDp *InstancePtr, u8 LaneCount);
u32 XMmiDp_AdjVswingPreemp(XMmiDp *InstancePtr);
u32 XMmiDp_SetTrainingPattern(XMmiDp *InstancePtr, XMmiDp_PhyTrainingPattern Pattern);
void XMmiDp_SetVswingPreemp(XMmiDp *InstancePtr, u8 *AuxData);
u32 XMmiDp_SetLinkRate(XMmiDp *InstancePtr, XMmiDp_PhyRate LinkRate);
void XMmiDp_SetLaneCount(XMmiDp *InstancePtr, XMmiDp_PhyLanes LaneCount);
XMmiDp_TrainingState XMmiDp_TrainingStateClockRecovery(XMmiDp *InstancePtr);
XMmiDp_TrainingState XMmiDp_TrainingStateAdjustLinkRate(XMmiDp *InstancePtr);
XMmiDp_TrainingState XMmiDp_TrainingStateAdjustLaneCount(XMmiDp *InstancePtr);
void XMmiDp_GetDpcdMaxDownspread(XMmiDp *InstancePtr);
void XMmiDp_GetDpcdRev(XMmiDp *InstancePtr);
u32 XMmiDp_GetRxCapabilities(XMmiDp *InstancePtr);
u32 XMmiDp_CheckChannelEqualization(XMmiDp *InstancePtr, u8 LaneCount);
XMmiDp_TrainingState XMmiDp_TrainingStateChannelEqualization(XMmiDp *InstancePtr);
u32 XMmiDp_CheckLinkStatus(XMmiDp *InstancePtr, u8 LaneCount);
u32 XMmiDp_RunTraining(XMmiDp *InstancePtr);

void XMmiDp_WaitUs(XMmiDp *InstancePtr, u32 MicroSeconds);

/* Stream Policy Maker */
void XMmiDp_DisableVideoStream(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_EnableVideoStream(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_ClearMsaValues(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_ClearVideoConfigValues(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_SetVInputPolarityCtrl(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_SetVideoConfig1(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_SetVideoConfig2(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_SetVideoConfig3(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_SetVideoConfig4(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_SetVideoConfig5(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_SetVSampleCtrl(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_SetVideoMsa1(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_SetVideoMsa2(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_SetVideoMsa3(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_SetHBlankInterval(XMmiDp *InstancePtr, u8 Stream);
void XMmiDp_ConfigureVideoController(XMmiDp *InstancePtr,
				     u8 Stream);
void XMmiDp_ClearVideoController(XMmiDp *InstancePtr);
void XMmiDp_SetMsaBpc(XMmiDp *InstancePtr, u8 Stream,
		      u8 BitsPerColor);
void XMmiDp_SetPixModeSel(XMmiDp *InstancePtr, u8 Stream,
			  XMmiDp_PPC PixModeSel);
void XMmiDp_SetVideoMapping(XMmiDp *InstancePtr,
			    u8 Stream, XMmiDp_VidMap VidMap);
void XMmiDp_SetVidStreamEnable(XMmiDp *InstancePtr, u8 Stream,
			       u8 StreamEnable);

#endif /* __XMMIDP_H__ */
