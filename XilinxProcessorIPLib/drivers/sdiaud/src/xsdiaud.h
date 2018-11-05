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
/******************************************************************************/
/**
 * @file xsdiaud.h
 * @addtogroup sdiaud_v1_1
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
	XSDIAUD_HANDLER_CNTRL_PKT_CHNG_DET,
	//!< Control packet change detect handler
	XSDIAUD_HANDLER_CHSTAT_CHNG_DET,
	//!< Channel status change detect handler
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
	XSDIAUD_SAMPRATE0,  //!< 000 - 48 KHz
	XSDIAUD_SAMPRATE1,  //!< 001 - 44.1 KHz
	XSDIAUD_SAMPRATE2   //!< 010 - 32 KHz
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

/** Line standard
 * @{
 */
/**
 * These constants specify different line standards
 */
typedef enum {
	XSDIAUD_SMPTE_260M_1035i_30Hz = 0,
	XSDIAUD_SMPTE_295M_1080i_25Hz = 1,
	XSDIAUD_SMPTE_274M_1080i_or_1080sF_30Hz_or_29p97Hz = 2,
	XSDIAUD_SMPTE_274M_1080i_or_1080sF_25Hz = 3,
	XSDIAUD_SMPTE_274M_1080p_30Hz = 4,
	XSDIAUD_SMPTE_274M_1080p_25Hz = 5,
	XSDIAUD_SMPTE_274M_1080p_24Hz_or_23p98Hz = 6,
	XSDIAUD_SMPTE_296M_720p_60Hz_or_59p94Hz = 7,
	XSDIAUD_SMPTE_274M_1080sF_24Hz_or_23p98Hz = 8,
	XSDIAUD_SMPTE_296M_720p_50Hz = 9,
	XSDIAUD_SMPTE_296M_720p_30Hz_or_29p97Hz = 10,
	XSDIAUD_SMPTE_296M_720p_25Hz = 11,
	XSDIAUD_SMPTE_296M_720p_24Hz_or_23p98Hz = 12,
	XSDIAUD_SMPTE_274M_1080p_60Hz_or_59p94Hz = 13,
	XSDIAUD_SMPTE_274M_1080p_50Hz = 14,
	XSDIAUD_NTSC = 16,
	XSDIAUD_PAL = 17,
	XSDIAUD_2160p_23p98Hz = 18,
	XSDIAUD_2160p_24Hz = 19,
	XSDIAUD_2160p_25Hz = 20,
	XSDIAUD_2160p_29p97Hz = 21,
        XSDIAUD_2160p_30Hz = 22,
	XSDIAUD_2160p_47p95Hz_or_48Hz = 23,
	XSDIAUD_2160p_50Hz = 24,
	XSDIAUD_2160p_59p94Hz_or_60Hz = 25,
	XSDIAUD_2048X1080I_29p97Hz_or_30Hz = 26,
	XSDIAUD_2048X1080I_25Hz = 27,
	XSDIAUD_SMPTE_2048_2_1080p_29p97Hz_or_30Hz = 28,
	XSDIAUD_SMPTE_2048_2_1080p_23p98Hz_or_24Hz_or_25Hz = 29,
	XSDIAUD_SMPTE_2048_2_1080p_47p95Hz_or_48Hz_50Hz_59p94Hz_60Hz = 30
} XSdiAud_LineStnd;

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
	XSDIAUD_16_CHANNELS //!< 16 channels
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
	XSDIAUD_CHANNEL16 //!< channel 16
	} XSdiAud_ChNum;

/** Groups that are present (i.e. all the combinations)
 * @{
 */
/**
 * These constants specify different Group combinations
 */
typedef enum {
	XSDIAUD_GROUP_0 = 0,  //!< All Groups are absent
	XSDIAUD_GROUP_1,      //!< Group 1 is present
	XSDIAUD_GROUP_2, //!< Group 2 is present
	XSDIAUD_GROUP_1_2, //!< Group 1 and 2 are present
	XSDIAUD_GROUP_3,//!< Group 3 is present
	XSDIAUD_GROUP_1_3,//!< Group 1 and 3 are present
	XSDIAUD_GROUP_2_3,//!< Group 2 and 3 are present
	XSDIAUD_GROUP_1_2_3,//!< Group 1, 2 and 3 are present
	XSDIAUD_GROUP_4,//!< Group 4 is present
	XSDIAUD_GROUP_1_4,//!< Group 1 and 4 are present
	XSDIAUD_GROUP_2_4,//!< Group 2 and 4 are present
	XSDIAUD_GROUP_1_2_4,//!< Group 1, 2 and 4 are present
	XSDIAUD_GROUP_3_4,//!< Group 3 and 4 are present
	XSDIAUD_GROUP_1_3_4,//!< Group 1, 3 and 4 are present
	XSDIAUD_GROUP_2_3_4,//!< Group 2, 3 and 4 are present
	XSDIAUD_GROUP_ALL,//!< All Groups are present
	XSDIAUD_NUM_CHANNELS //!<Number of Group combinations
	} XSdiAud_GrpsPrsnt;

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
	XSdiAud_Callback CntrlPktDetHandler;
	//!< Start of control packet detected handler
	void *CntrlPktDetHandlerRef;
	//!< Callback reference for control packet detected handler
	XSdiAud_Callback StatChangeDetHandler;
	//!< Start of status change detected handler
	void *StatChangeDetHandlerRef;
	//!< Callback reference for status change detected handler
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
void XSdiAud_ResetCoreEn(XSdiAud *InstancePtr, u8 RstCoreEnable);

/* Function to soft reset the XSdiaud registers*/
void XSdiAud_ResetReg(XSdiAud *InstancePtr);

void XSdiAud_IntrHandler(void *InstancePtr);

int XSdiAud_SetHandler(XSdiAud *InstancePtr, XSdiAud_HandlerType HandlerType,
		XSdiAud_Callback FuncPtr, void *CallbackRef);

u32 XSdiAud_GetIntStat(XSdiAud *InstancePtr);

/*Audio Embed Function to set the sampling rate */
void XSdiAud_Emb_SetSmpRate(XSdiAud *InstancePtr, XSdiAud_SampRate XSdiAud_SRate);

/* Audio Embed Function to set the sample size in SD Mode */
void XSdiAud_Emb_SetSmpSize(XSdiAud *InstancePtr, XSdiAud_SampSize XSdiAud_SSize);

/* Video Embed Function to set the Line standard */
void XSdiAud_Emb_SetLineStd(XSdiAud *InstancePtr, XSdiAud_LineStnd XSdiAud_LS);

/* Video Embed Function to set enable external line */
void XSdiAud_Emb_EnExtrnLine(XSdiAud *InstancePtr, u8 XSdiAud_En);

/* Audio Extract Function to set the Clock Phase in HD Mode */
void XSdiAud_Ext_SetClkPhase(XSdiAud *InstancePtr, u8 XSdiAud_SetClkP);

/* Channel status related function */
void XSdiAud_Ext_GetChStat(XSdiAud *InstancePtr, u8 *ChStatBuf);

/* Function to know the detected groups*/
XSdiAud_GrpsPrsnt XSdiAud_DetAudGrp(XSdiAud *InstancePtr);

/* Function to set channel */
void XSdiAud_SetCh(XSdiAud *InstancePtr, XSdiAud_GrpNum XSdiStrtGrpNum,
		XSdiAud_NumOfCh XSdiANumOfCh);

/* Function to mute a specific channel from a group */
void XSdiAud_Ext_Mute(XSdiAud *InstancePtr, XSdiAud_GrpNum XSdiAGrpNum,
		XSdiAud_GrpXChNum XSdiAChNum);

/* Function reads the control packet status register's active channel field */
u32 XSdiAud_Ext_GetActCh(XSdiAud *InstancePtr);

/* Function to control the rate at which audio samples are inserted */
void XSdiAud_Emb_RateCntrlEn(XSdiAud *InstancePtr, u8 XSdiAud_RCE);

/************************** Variable Declarations ****************************/
#ifdef __cplusplus
}
#endif

#endif /* XSDIAUD_H */
/** @} */
