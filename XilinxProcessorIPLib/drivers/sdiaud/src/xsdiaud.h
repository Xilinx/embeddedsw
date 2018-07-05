/*******************************************************************************
 *
 * Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 ******************************************************************************/
/******************************************************************************/
/**
 * @file xsdiaud.h
 * @addtogroup sdiaud_v2_0
 * @{
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date      Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    02/15/18  Initial release.
 * 1.1   kar    04/25/18  Added new line standards.
 *                        Added new API to enable rate control.
 *                        Removed inline function which reads the IP version.
 * 2.0   vve    09/27/18  Add 32 channel support
 *                        Add support for channel status extraction logic both
 *                        on embed and extract side.
 *                        Add APIs to detect group change, sample rate change,
 *                        active channel change
 *
 * </pre>
 *
 ******************************************************************************/

#ifndef XSDIAUD_H
#define XSDIAUD_H
/* Prevent circular inclusions by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsdiaud_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_types.h"
/************************** Constant Definitions *****************************/
#define MAX_AUDIO_CHANNELS 32
#define MAX_AUDIO_GROUPS (MAX_AUDIO_CHANNELS / 4)
/**************************** Type Definitions *******************************/
/** @name Handler Types
 * @{
 */
/**
 * These constants specify different types of handlers and is used to
 * differentiate interrupt requests from the XSdiAud peripheral.
 */
typedef enum {
	XSDIAUD_HANDLER_AUD_GRP_CHNG_DET = 0,
	//!< Audio group change detect handler
	XSDIAUD_HANDLER_ACT_CH_CHNG_DET,
	//!< Active channel change detect handler
	XSDIAUD_HANDLER_SAMPLE_RATE_CHNG_DET,
	//!< Sample rate change detect handler
	XSDIAUD_HANDLER_ASX_CHNG_DET,
	//!< Asynchronous data flag change detect handler
	XSDIAUD_HANDLER_AES_CS_UPDATE_DET,
	//!< Aes cs update detect handler
	XSDIAUD_HANDLER_AES_CS_CHANGE_DET,
	//!< Aes cs change detect handler
	XSDIAUD_HANDLER_CHSTAT_CHNG_DET,
	//!< Channel status change detect handler
	XSDIAUD_HANDLER_VIDEO_PROP_CHNG_DET,
	//!< Video properties change detect handler
	XSDIAUD_HANDLER_FIFO_OVRFLW_DET,
	//!< FIFO Overflow detect handler
	XSDIAUD_HANDLER_PARITY_ERR_DET,
	//!< Parity error detect handler
	XSDIAUD_HANDLER_CHECKSUM_ERR_DET,
	//!< Checksum error detect handler
	XSDIAUD_NUM_HANDLERS //!< Number of handler types
	} XSdiAud_HandlerType;

/** Group numbers
 * @{
 */
/**
 * These constants specify different Group numbers
 */
typedef enum {
	XSDIAUD_GROUP1 = 1,  //!< Group 1
	XSDIAUD_GROUP2,      //!< Group 2
	XSDIAUD_GROUP3,      //!< Group 3
	XSDIAUD_GROUP4       //!< Group 4
	} XSdiAud_GrpNum;

/** Sampling Rates
 * @{
 */
/**
 * These constants specify different audio Sampling Rates
 */
typedef enum {
	XSDIAUD_SMPLRATE_48, //!< 000 - 48 KHz
	XSDIAUD_SMPLRATE_44, //!< 001 - 44.1 KHz
	XSDIAUD_SMPLRATE_32  //!< 010 - 32 KHz
	} XSdiAud_SampRate;

/** Sample Size
 * @{
 */
/**
 * These constants specify different Sample Size
 */
typedef enum {
	XSDIAUD_SAMPSIZE0,  //!< 0 - 20 Bit Audio Sample
	XSDIAUD_SAMPSIZE1   //!< 1 - 24 Bit Audio Sample
	} XSdiAud_SampSize;

/** AES channel pair
 * @{
 */
/**
 * These constants specify different aes channel pairs
 */
typedef enum {
	XSDIAUD_AES_CHANNEL_PAIR_1 = 0,
	XSDIAUD_AES_CHANNEL_PAIR_2,
	XSDIAUD_AES_CHANNEL_PAIR_3,
	XSDIAUD_AES_CHANNEL_PAIR_4,
	XSDIAUD_AES_CHANNEL_PAIR_5,
	XSDIAUD_AES_CHANNEL_PAIR_6,
	XSDIAUD_AES_CHANNEL_PAIR_7,
	XSDIAUD_AES_CHANNEL_PAIR_8,
	XSDIAUD_AES_CHANNEL_PAIR_9,
	XSDIAUD_AES_CHANNEL_PAIR_10,
	XSDIAUD_AES_CHANNEL_PAIR_11,
	XSDIAUD_AES_CHANNEL_PAIR_12,
	XSDIAUD_AES_CHANNEL_PAIR_13,
	XSDIAUD_AES_CHANNEL_PAIR_14,
	XSDIAUD_AES_CHANNEL_PAIR_15,
	XSDIAUD_AES_CHANNEL_PAIR_16
} XSdiAud_AesChPair;

/** Asynchronous data flag
 * @{
 */
/**
 * These constants specify different asynchronous data flag
 */
typedef enum {
	XSDIAUD_SYNCHRONOUS_AUDIO = 0,
	XSDIAUD_ASYNCHRONOUS_AUDIO
} XSdiAud_Asx;

/*@}**/

/** Channel number in any Group
 * @{
 */
/**
 * These constants specify different channel numbers in Group 1 or 2 or 3 or 4
 */
typedef enum {
	XSDIAUD_GROUPX_CHANNEL1 = 1, //!< Channel 1 of Group 1/2/3/4
	XSDIAUD_GROUPX_CHANNEL2, //!< Channel 2 of Group 1/2/3/4
	XSDIAUD_GROUPX_CHANNEL3, //!< Channel 3 of Group 1/2/3/4
	XSDIAUD_GROUPX_CHANNEL4 //!< Channel 4 of Group 1/2/3/4
	} XSdiAud_GrpXChNum;

/** Video Rate
 * @{
 */
/**
 * These constants specify different video rates
 */
typedef enum {
	XSDIAUD_VID_RATE_23_98_HZ = 2,
	XSDIAUD_VID_RATE_24_HZ,
	XSDIAUD_VID_RATE_47_95_HZ2,
	XSDIAUD_VID_RATE_25_HZ,
	XSDIAUD_VID_RATE_29_97_HZ,
	XSDIAUD_VID_RATE_30_HZ,
	XSDIAUD_VID_RATE_48_HZ,
	XSDIAUD_VID_RATE_50_HZ,
	XSDIAUD_VID_RATE_59_94_HZ,
	XSDIAUD_VID_RATE_60_HZ,
} XSdiAud_VidRate;

/** Video Scan
 * @{
 */
/**
 * These constants specify different video scans
 */
typedef enum {
	XSDIAUD_VID_SCAN_INTERLACED = 0,
	XSDIAUD_VID_SCAN_PROGRESSIVE,
} XSdiAud_VidScan;

/** Video Family
 * @{
 */
/**
 * These constants specify different video families
 */
typedef enum {
	XSDIAUD_VID_FAMILY_1920_1080 = 0,
	XSDIAUD_VID_FAMILY_1280_720 = 1,
	XSDIAUD_VID_FAMILY_2048_1080 = 2,
	XSDIAUD_VID_FAMILY_720_486 = 8,
	XSDIAUD_VID_FAMILY_720_576 = 9
} XSdiAud_VidFam;

/** Number of Channels
 * @{
 */
/**
 * These constants specify number of channels
 */
typedef enum {
	XSDIAUD_1_CHANNELS = 1, //!< 1 channel
	XSDIAUD_2_CHANNELS, //!< 2 channels
	XSDIAUD_3_CHANNELS, //!< 3 channels
	XSDIAUD_4_CHANNELS,//!<  4 channels
	XSDIAUD_5_CHANNELS,//!<  5 channels
	XSDIAUD_6_CHANNELS,//!<  6 channels
	XSDIAUD_7_CHANNELS,//!<  7 channels
	XSDIAUD_8_CHANNELS,//!<  8 channels
	XSDIAUD_9_CHANNELS,//!<  9 channels
	XSDIAUD_10_CHANNELS,//!< 10 channels
	XSDIAUD_11_CHANNELS,//!< 11 channels
	XSDIAUD_12_CHANNELS,//!< 12 channels
	XSDIAUD_13_CHANNELS,//!< 13 channels
	XSDIAUD_14_CHANNELS,//!< 14 channels
	XSDIAUD_15_CHANNELS,//!< 15 channels
	XSDIAUD_16_CHANNELS, //!< 16 channels
	XSDIAUD_32_CHANNELS = 32//!< 32 channels
	} XSdiAud_NumOfCh;

/** Channel Number
 * @{
 */
/**
 * These constants specify Channel number
 */
typedef enum {
	XSDIAUD_CHANNEL1 = 1, //!< channel 1
	XSDIAUD_CHANNEL2, //!< channel 2
	XSDIAUD_CHANNEL3, //!< channel 3
	XSDIAUD_CHANNEL4,//!<  channel 4
	XSDIAUD_CHANNEL5,//!<  channel 5
	XSDIAUD_CHANNEL6,//!<  channel 6
	XSDIAUD_CHANNEL7,//!<  channel 7
	XSDIAUD_CHANNEL8,//!<  channel 8
	XSDIAUD_CHANNEL9,//!<  channel 9
	XSDIAUD_CHANNEL10,//!< channel 10
	XSDIAUD_CHANNEL11,//!< channel 11
	XSDIAUD_CHANNEL12,//!< channel 12
	XSDIAUD_CHANNEL13,//!< channel 13
	XSDIAUD_CHANNEL14,//!< channel 14
	XSDIAUD_CHANNEL15,//!< channel 15
	XSDIAUD_CHANNEL16,//!< channel 16
	XSDIAUD_CHANNEL17,//!< channel 17
	XSDIAUD_CHANNEL18,//!< channel 18
	XSDIAUD_CHANNEL19,//!< channel 19
	XSDIAUD_CHANNEL20,//!< channel 20
	XSDIAUD_CHANNEL21,//!< channel 21
	XSDIAUD_CHANNEL22,//!< channel 22
	XSDIAUD_CHANNEL23,//!< channel 23
	XSDIAUD_CHANNEL24,//!< channel 24
	XSDIAUD_CHANNEL25,//!< channel 25
	XSDIAUD_CHANNEL26,//!< channel 26
	XSDIAUD_CHANNEL27,//!< channel 27
	XSDIAUD_CHANNEL28,//!< channel 28
	XSDIAUD_CHANNEL29,//!< channel 29
	XSDIAUD_CHANNEL30,//!< channel 30
	XSDIAUD_CHANNEL31,//!< channel 31
	XSDIAUD_CHANNEL32 //!< channel 32
	} XSdiAud_ChNum;

typedef void (*XSdiAud_Callback)(void *CallbackRef);
/*@}*/
/**
 * @brief This typedef contains configuration information for the XSdiAud.
 */

typedef struct {
	u32 DeviceId;	//!< DeviceId is the unique ID of XSdiaud
	UINTPTR BaseAddress;
	//!< BaseAddress of the XSdiaud
	u8 IsEmbed; //!< Is Audio Embed or Extract
	u8 LineRate;  //!< UHD SDI standard
	u8 MaxNumChannels;
	//!< Indicates the max number of channels supported by the core
	} XSdiAud_Config;
/**
 * @brief The XSdiAud driver instance data.
 *
 * An instance must be allocated for each XSdiAud core in use.
 */

typedef struct {
	u32 IsReady;
	//!< Core and the driver instance are initialized
	u32 IsStarted;
	//!< Core and the driver instance has started
	XSdiAud_Config Config; //!< Hardware Configuration
	/* Call backs */
	XSdiAud_Callback GrpChangeDetHandler;
	//!< Start of group change detected handler
	void *GrpChangeDetHandlerRef;
	//!< Callback reference for group change detected handler
	XSdiAud_Callback ActiveChannelChangeDetHandler;
	//!< Start of active channel change detected handler
	void *ActiveChannelChangeDetHandlerRef;
	//!< Callback reference for active channel change detected handler
	XSdiAud_Callback SampleRateChangeDetHandler;
	//!< Start of sample rate change detected handler
	void *SampleRateChangeDetHandlerRef;
	//!< Callback reference for sample rate change detected handler
	XSdiAud_Callback AsxChangeDetHandler;
	//!< Start of asynchronous data flag value change detected handler
	void *AsxChangeDetHandlerRef;
	//!< Callback reference for asynchronous data flag change handler
	XSdiAud_Callback StatChangeDetHandler;
	//!< Start of status change detected handler
	void *StatChangeDetHandlerRef;
	//!< Callback reference for status change detected handler
	XSdiAud_Callback AesCsUpdateDetHandler;
	//!< Start of AES channel status value update detected handler
	void *AesCsUpdateDetHandlerRef;
	//!< Callback reference for AES channel status value updated handler
	XSdiAud_Callback AesCsChangeDetHandler;
	//!< Start of AES channel status value change detected handler
	void *AesCsChangeDetHandlerRef;
	//!< Callback reference for AES channel status value change handler
	XSdiAud_Callback VidPropChangeDetHandler;
	//!< Start of video properties change detected handler
	void *VidPropChangeDetHandlerRef;
	//!< Callback reference for video properties change detected handler
	XSdiAud_Callback FifoOvrflwDetHandler;
	//!< Start of fifo overflow detected handler
	void *FifoOvrflwDetHandlerRef;
	//!< Callback reference for fifo overflow detected handler
	XSdiAud_Callback ParityErrDetHandler;
	//!< Start of parity error detected handler
	void *ParityErrDetHandlerRef;
	//!< Callback reference for Parity Error detected Handler
	XSdiAud_Callback ChecksumErrDetHandler;
	//!< Start of checksum error detected handler
	void *ChecksumErrDetHandlerRef;
	//!< Callback reference for checksum error detected handler
	XSdiAud_GrpNum StrtGrpNum;
	//!< start group number that is configured
	XSdiAud_NumOfCh NumOfCh;
	//!< Total Number of channels that are configured
	} XSdiAud;

/**
 * @brief This structure contains the video properties.
 *
 */

typedef struct {
	XSdiAud_VidFam XSdiAud_TFamily;
	//!< Transport video family, it is enum XSdiAud_VidFam
	XSdiAud_VidRate XSdiAud_TRate;
	//!< Transport video rate, it is enum XSdiAud_VidRate
	XSdiAud_VidScan XSdiAud_TScan;
	//!< Transport video scan, it is enum XSdiAud_VidScan
} XSdiAud_Emb_Vid_Props;

typedef struct {
	u8 NumGroups;
	u8 GrpActive[MAX_AUDIO_GROUPS];
} XSdiAud_ActGrpSt;

typedef struct {
	u8 NumChannels;
	u8 GrpActCh[MAX_AUDIO_GROUPS];
} XSdiAud_ActChSt;

typedef struct {
	u8 SRChPair[MAX_AUDIO_CHANNELS / 2];
} XSdiAud_SRSt;

typedef struct {
	u8 AsxPair[MAX_AUDIO_CHANNELS / 2];
} XSdiAud_AsxSt;

/* Interrupt related functions */
/*****************************************************************************
 **
 * This function clears the specified interrupt of the XSdiAud.
 *
 * @param InstancePtr is a pointer to the XSdiAud core instance.
 * @param Mask is a bit mask of the interrupts to be cleared.
 * @see xsdiaud_hw.h file for the available interrupt masks.
 *
 * @return None.
 *
 ******************************************************************************/
static inline void XSdiAud_IntrClr(XSdiAud *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_INT_STS_REG_OFFSET, Mask);
}

/*****************************************************************************
 **
 * This function enables the specified interrupt of the XSdiAud.
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  Mask is a bit mask of the interrupts to be enabled.
 *
 * @return None.
 *
 * @see xsdiaud_hw.h file for the available interrupt masks.
 *
 ******************************************************************************/
static inline void XSdiAud_IntrEnable(XSdiAud *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	u32 RegValue = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_INT_EN_REG_OFFSET);
	RegValue |= Mask;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_INT_EN_REG_OFFSET,
			RegValue);
}

/*****************************************************************************
 **
 * This function disables the specified interrupt of the XSdiAud.
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  Mask is a bit mask of the interrupts to be disabled.
 *
 * @return None.
 *
 * @see xsdiaud_hw.h file for the available interrupt masks.
 *
 ******************************************************************************/
static inline void XSdiAud_IntrDisable(XSdiAud *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	u32 RegValue = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_INT_EN_REG_OFFSET);
	RegValue &= ~Mask;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_INT_EN_REG_OFFSET,
			RegValue);
}

/************************* Function Prototypes ******************************/

/* Initialization function in xsdiAud_sinit.c */
XSdiAud_Config *XSdiAud_LookupConfig(u16 DeviceId);

int XSdiAud_Initialize(XSdiAud *InstancePtr, u16 DeviceId);

/* Initialization and control functions in xsdiaud.c */
int XSdiAud_CfgInitialize(XSdiAud *InstancePtr,
		XSdiAud_Config *CfgPtr, UINTPTR EffectiveAddr);

void XSdiAud_Enable(XSdiAud *InstancePtr, u8 Enable);

int XSdiAud_SelfTest(XSdiAud *InstancePtr);

/* Function to soft reset the XSdiaud core*/
void XSdiAud_CoreReset(XSdiAud *InstancePtr, u8 RstCoreEnable);

/* Function to soft reset the XSdiaud registers*/
void XSdiAud_ConfigReset(XSdiAud *InstancePtr);

void XSdiAud_IntrHandler(void *InstancePtr);

int XSdiAud_SetHandler(XSdiAud *InstancePtr, XSdiAud_HandlerType HandlerType,
		XSdiAud_Callback FuncPtr, void *CallbackRef);

u32 XSdiAud_GetCoreVersion(XSdiAud *InstancePtr);

u32 XSdiAud_GetIntStat(XSdiAud *InstancePtr);

/*Audio Embed Function to set the sampling rate */
void XSdiAud_Emb_SetSmpRate(XSdiAud *InstancePtr, XSdiAud_SampRate XSdiAud_SRate);

/* Audio Embed Function to set the sample size in SD Mode */
void XSdiAud_Emb_SetSmpSize(XSdiAud *InstancePtr, XSdiAud_SampSize XSdiAud_SSize);

/* Audio Embed Function to set the asynchronous data flag */
void XSdiAud_Emb_SetAsx(XSdiAud *InstancePtr, XSdiAud_Asx XSdiAud_SRate);

/* Audio Embed Function to set the aes channel pair */
void XSdiAud_SetAesChPair(XSdiAud *InstancePtr, XSdiAud_AesChPair XSdiAud_ACP);

/* Video Embed Function to set the video properties */
void XSdiAud_Emb_SetVidProps(XSdiAud *InstancePtr, XSdiAud_Emb_Vid_Props
	*XSdiAud_VP);

/* Video Embed Function to set enable external line */
void XSdiAud_Emb_EnExtrnLine(XSdiAud *InstancePtr, u8 XSdiAud_En);

/* Audio Extract Function to disable the Clock Phase in HD Mode */
void XSdiAud_Ext_DisableClkPhase(XSdiAud *InstancePtr, u8 XSdiAud_SetClkP);

/* Channel status related function */
void XSdiAud_GetChStat(XSdiAud *InstancePtr, u8 *ChStatBuf);

/* Function to know the detected groups*/
void XSdiAud_GetActGrpStatus(XSdiAud *InstancePtr, XSdiAud_ActGrpSt *GrpSt);

/* Function to set specific channels */
void XSdiAud_SetCh(XSdiAud *InstancePtr, u32 XSdiAudSetChMask);

/* Function to mute specific channels */
void XSdiAud_MuteCh(XSdiAud *InstancePtr, u32 XSdiAudMuteChMask);

/* Function reads the FIFO overflow status register field */
u8 XSdiAud_Ext_GetFIFOOvFlwStatus(XSdiAud *InstancePtr);

/* Function reads the active channle status register field */
void XSdiAud_Ext_GetAcChStatus(XSdiAud *InstancePtr, XSdiAud_ActChSt *ActChSt);

/* Function reads the sample rate status register field */
void XSdiAud_Ext_GetSRStatus(XSdiAud *InstancePtr, XSdiAud_SRSt *SRSt);

/* Function reads the async channel pair status register field */
void XSdiAud_Ext_GetAsxStatus(XSdiAud *InstancePtr, XSdiAud_AsxSt *AsxSt);

/************************** Variable Declarations ****************************/
#ifdef __cplusplus
}
#endif

#endif /* XSDIAUD_H */
/** @} */
