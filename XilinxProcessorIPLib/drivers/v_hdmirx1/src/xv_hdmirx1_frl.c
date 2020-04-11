/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirx1_frl.c
*
* This is the main file for Xilinx HDMI RX core for FRL. Please see
* xv_hdmirx1_frl.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00  EB     25/06/18 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmirx1_frl.h"
#include "xv_hdmirx1.h"
/*#include "string.h"*/

/************************** Constant Definitions *****************************/
#define Timer2MS		2
#define Timer5MS		5
#define Timer40MS		40
#define Timer100MS		100
#define Timer200MS		200
#define Timer250MS		250
#define Timer500MS		500

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/*****************************************************************************/
/**
* This table contains the timeout period of LTS3 for different FFE Levels in
* Milliseconds
*/
const u16 FrlTimeoutLts3[4] = {
	180,	/* FFE Level = 0*/
	 90,	/* FFE Level = 1*/
	 60,	/* FFE Level = 2*/
	 45	/* FFE Level = 3*/
};

/*****************************************************************************/
/**
* This table contains the attributes for SCDC fields
* Each entry consists of:
* 1) Register Offset
* 2) Bits Mask
* 3) Bits Shift
*/
const XV_HdmiRx1_FrlScdcField FrlScdcField[XV_HDMIRX1_SCDCFIELD_SIZE] = {
	{0x01, 0xFF, 0},	/* XV_HDMIRX1_SCDCFIELD_SINK_VER */
	{0x02, 0xFF, 0},	/* XV_HDMIRX1_SCDCFIELD_SOURCE_VER */
	{0x10, 0x01, 1},	/* XV_HDMIRX1_SCDCFIELD_CED_UPDATE*/
	{0x10, 0x01, 3},	/* XV_HDMIRX1_SCDCFIELD_SOURCE_TEST_UPDATE */
	{0x10, 0x01, 4},	/* XV_HDMIRX1_SCDCFIELD_FRL_START */
	{0x10, 0x01, 5},	/* XV_HDMIRX1_SCDCFIELD_FLT_UPDATE */
	{0x10, 0x01, 6},	/* XV_HDMIRX1_SCDCFIELD_RSED_UPDATE */
	{0x20, 0x03, 0},	/* XV_HDMIRX1_SCDCFIELD_SCRAMBLER_EN */
	{0x21, 0x01, 0},	/* XV_HDMIRX1_SCDCFIELD_SCRAMBLER_STAT */
	{0x30, 0x01, 1},	/* XV_HDMIRX1_SCDCFIELD_FLT_NO_RETRAIN */
	{0x31, 0x0F, 0},	/* XV_HDMIRX1_SCDCFIELD_FRL_RATE */
	{0x31, 0x0F, 4},	/* XV_HDMIRX1_SCDCFIELD_FFE_LEVELS */
	{0x35, 0x01, 5},	/* XV_HDMIRX1_SCDCFIELD_FLT_NO_TIMEOUT */
	{0x40, 0x0F, 1},	/* XV_HDMIRX1_SCDCFIELD_LNS_LOCK */
	{0x40, 0x01, 6},	/* XV_HDMIRX1_SCDCFIELD_FLT_READY */
	{0x41, 0x0F, 0},	/* XV_HDMIRX1_SCDCFIELD_LN0_LTP_REQ */
	{0x41, 0x0F, 4},	/* XV_HDMIRX1_SCDCFIELD_LN1_LTP_REQ */
	{0x42, 0x0F, 0},	/* XV_HDMIRX1_SCDCFIELD_LN2_LTP_REQ */
	{0x42, 0x0F, 4},	/* XV_HDMIRX1_SCDCFIELD_LN3_LTP_REQ */
	{0x50, 0xFF, 0},	/* XV_HDMIRX1_SCDCFIELD_CH0_ERRCNT_LSB */
	{0x51, 0xFF, 0},	/* XV_HDMIRX1_SCDCFIELD_CH0_ERRCNT_MSB */
	{0x52, 0xFF, 0},	/* XV_HDMIRX1_SCDCFIELD_CH1_ERRCNT_LSB */
	{0x53, 0xFF, 0},	/* XV_HDMIRX1_SCDCFIELD_CH1_ERRCNT_MSB */
	{0x54, 0xFF, 0},	/* XV_HDMIRX1_SCDCFIELD_CH2_ERRCNT_LSB */
	{0x55, 0xFF, 0},	/* XV_HDMIRX1_SCDCFIELD_CH2_ERRCNT_MSB */
	{0x56, 0xFF, 0},	/* XV_HDMIRX1_SCDCFIELD_CED_CHECKSUM */
	{0x57, 0xFF, 0},	/* XV_HDMIRX1_SCDCFIELD_CH3_ERRCNT_LSB */
	{0x58, 0xFF, 0},	/* XV_HDMIRX1_SCDCFIELD_CH3_ERRCNT_MSB */
	{0x59, 0xFF, 0},	/* XV_HDMIRX1_SCDCFIELD_RSCCNT_LSB */
	{0x5A, 0xFF, 0}		/* XV_HDMIRX1_SCDCFIELD_RSCCNT_MSB */
};

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static int XV_HdmiRx1_ExecFrlState_LtsL(XV_HdmiRx1 *InstancePtr);
static int XV_HdmiRx1_ExecFrlState_Lts2(XV_HdmiRx1 *InstancePtr);
static int XV_HdmiRx1_ExecFrlState_Lts3(XV_HdmiRx1 *InstancePtr);
static int XV_HdmiRx1_ExecFrlState_Lts3_RateChange(XV_HdmiRx1 *InstancePtr);
static int XV_HdmiRx1_ExecFrlState_Lts3_LtpDetected(XV_HdmiRx1 *InstancePtr);
static int XV_HdmiRx1_ExecFrlState_Lts3_Timer(XV_HdmiRx1 *InstancePtr);
static int XV_HdmiRx1_ExecFrlState_LtsP(XV_HdmiRx1 *InstancePtr);
static int XV_HdmiRx1_ExecFrlState_LtsP_Timeout(XV_HdmiRx1 *InstancePtr);
static void XV_HdmiRx1_ClearFrlLtp(XV_HdmiRx1 *InstancePtr);
static void XV_HdmiRx1_SetFrlTimer(XV_HdmiRx1 *InstancePtr, u32 Milliseconds);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function enables the FRL mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @param    LtpThreshold specifies the number of times the LTP matching module
*           must match against the incoming link training pattern before a
*           match is indicated
*
* @param    DefaultLtp specify the link training pattern which will be used
* 			for link training purposes
* 			- XV_HDMIRX1_LTP_LFSR0
* 			- XV_HDMIRX1_LTP_LFSR1
* 			- XV_HDMIRX1_LTP_LFSR2
* 			- XV_HDMIRX1_LTP_LFSR3
*
* @return	Status on if FrlTraining can be started or not.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_FrlModeEnable(XV_HdmiRx1 *InstancePtr, u8 LtpThreshold,
			      XV_HdmiRx1_FrlLtp DefaultLtp, u8 FfeSuppFlag)
{
	Xil_AssertVoid(InstancePtr != NULL);
/*	Xil_AssertVoid(DefaultLtp.Byte[0] >= 5 && DefaultLtp.Byte[0] <= 8);*/
/*	Xil_AssertVoid(DefaultLtp.Byte[1] >= 5 && DefaultLtp.Byte[1] <= 8);*/
/*	Xil_AssertVoid(DefaultLtp.Byte[2] >= 5 && DefaultLtp.Byte[2] <= 8);*/
/*	Xil_AssertVoid((DefaultLtp.Byte[3] >= 5 && DefaultLtp.Byte[3] <= 8) ||*/
/*    				DefaultLtp.Byte[3] == 0);*/
	Xil_AssertVoid(FfeSuppFlag == 0 || FfeSuppFlag == 1);

/*	InstancePtr->Stream.Frl.DefaultLtp.Data = DefaultLtp.Data;*/
	InstancePtr->Stream.Frl.DefaultLtp.Byte[0] = DefaultLtp.Byte[0];
	InstancePtr->Stream.Frl.DefaultLtp.Byte[1] = DefaultLtp.Byte[1];
	InstancePtr->Stream.Frl.DefaultLtp.Byte[2] = DefaultLtp.Byte[2];
	InstancePtr->Stream.Frl.DefaultLtp.Byte[3] = DefaultLtp.Byte[3];
	InstancePtr->Stream.Frl.FfeSuppFlag = FfeSuppFlag;

	XV_HdmiRx1_SetFrlLtpThreshold(InstancePtr, LtpThreshold);

	InstancePtr->Stream.Frl.TrainingState = XV_HDMIRX1_FRLSTATE_LTS_2;

	XV_HdmiRx1_ExecFrlState(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State Legacy.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiRx1_ExecFrlState_LtsL(XV_HdmiRx1 *InstancePtr)
{
#ifdef DEBUG_RX_FRL_VERBOSITY
	xil_printf(ANSI_COLOR_MAGENTA "RX: LTS:L\r\n" ANSI_COLOR_RESET);
#endif
	int Status = XST_FAILURE;

	/* Clear HDMI variables */
	XV_HdmiRx1_Clear(InstancePtr);

	XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
				    XV_HDMIRX1_SCDCFIELD_FLT_READY,
				    1);

	XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
				    XV_HDMIRX1_SCDCFIELD_FRL_RATE,
				    0);

	/* Check if user callback has been registered*/
	if (InstancePtr->TmdsConfigCallback) {
		InstancePtr->TmdsConfigCallback(InstancePtr->TmdsConfigRef);
	}

	/* Check if user callback has been registered*/
	if (InstancePtr->FrlLtsLCallback) {
		InstancePtr->FrlLtsLCallback(InstancePtr->FrlLtsLRef);
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State 2.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiRx1_ExecFrlState_Lts2(XV_HdmiRx1 *InstancePtr)
{
	int Status = XST_SUCCESS;
#ifdef DEBUG_DDC_VERBOSITY
	xil_printf(ANSI_COLOR_MAGENTA "RX: LTS:2\r\n" ANSI_COLOR_RESET);
#endif
	XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
				    XV_HDMIRX1_SCDCFIELD_FLT_READY,
				    1);

	/* Check if user callback has been registered*/
	if (InstancePtr->FrlLts2Callback) {
		InstancePtr->FrlLts2Callback(InstancePtr->FrlLts2Ref);
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State 3 (Rate Change).
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiRx1_ExecFrlState_Lts3_RateChange(XV_HdmiRx1 *InstancePtr)
{
	int Status = XST_FAILURE;

	InstancePtr->Stream.Frl.TimerCnt = 0;
	Status = XV_HdmiRx1_RetrieveFrlRateLanes(InstancePtr);

	if (InstancePtr->Stream.Frl.FfeSuppFlag == TRUE) {
		InstancePtr->Stream.Frl.FfeLevels =
				XV_HdmiRx1_FrlDdcReadField(InstancePtr,
						XV_HDMIRX1_SCDCFIELD_FFE_LEVELS);
	} else {
		InstancePtr->Stream.Frl.FfeLevels = 0;
	}

#ifdef DEBUG_RX_FRL_VERBOSITY
	xil_printf(ANSI_COLOR_MAGENTA "RX: LTS:3 Rate Change\r\n"
			ANSI_COLOR_RESET);
#endif

	/* Check if user callback has been registered*/
	if (InstancePtr->FrlLts3Callback) {
		InstancePtr->FrlLts3Callback(InstancePtr->FrlLts3Ref);
	}

	XV_HdmiRx1_FrlFltUpdate(InstancePtr, FALSE);

	if (Status == XST_SUCCESS && InstancePtr->Stream.Frl.LineRate > 0) {
		InstancePtr->Stream.Frl.TrainingState =
				XV_HDMIRX1_FRLSTATE_LTS_3_RATE_CH;

		InstancePtr->Stream.State = XV_HDMIRX1_STATE_FRL_LINK_TRAINING;

		InstancePtr->Stream.IsFrl = TRUE;
		InstancePtr->Stream.IsHdmi = TRUE;

#ifdef DEBUG_RX_FRL_VERBOSITY
		xil_printf(ANSI_COLOR_MAGENTA "*********************\r\n"
				ANSI_COLOR_RESET);
		xil_printf(ANSI_COLOR_MAGENTA "RX: Rate: %d Lanes: %d"
				" Ffe_Lvl: %d\r\n"
				ANSI_COLOR_RESET,
				InstancePtr->Stream.Frl.LineRate,
				InstancePtr->Stream.Frl.Lanes,
				InstancePtr->Stream.Frl.FfeLevels);
		xil_printf(ANSI_COLOR_MAGENTA "*********************\r\n"
				ANSI_COLOR_RESET);
#endif
		XV_HdmiRx1_INT_LRST(InstancePtr, TRUE);
		XV_HdmiRx1_INT_VRST(InstancePtr, TRUE);
		XV_HdmiRx1_EXT_VRST(InstancePtr, TRUE);
		XV_HdmiRx1_EXT_SYSRST(InstancePtr, TRUE);
		/* Disable VTD */
		XV_HdmiRx1_VtdDisable(InstancePtr);

		/* Reset Frl Ltp Detection*/
		XV_HdmiRx1_ResetFrlLtpDetection(InstancePtr);

		/* Initialize LTP */
		XV_HdmiRx1_ClearFrlLtp(InstancePtr);

		XV_HdmiRx1_SetFrlTimer(InstancePtr,
				FrlTimeoutLts3[InstancePtr->Stream.Frl.FfeLevels]);

		/* Initialize the LTP detection with the default LTP */
		InstancePtr->Stream.Frl.Ltp.Byte[0] =
				InstancePtr->Stream.Frl.DefaultLtp.Byte[0];
		InstancePtr->Stream.Frl.Ltp.Byte[1] =
				InstancePtr->Stream.Frl.DefaultLtp.Byte[1];
		InstancePtr->Stream.Frl.Ltp.Byte[2] =
				InstancePtr->Stream.Frl.DefaultLtp.Byte[2];
		InstancePtr->Stream.Frl.Ltp.Byte[3] =
				InstancePtr->Stream.Frl.DefaultLtp.Byte[3];

		InstancePtr->Stream.Frl.LtpMatchedCounts = 0;
		InstancePtr->Stream.Frl.LtpMatchWaitCounts = 0;
		InstancePtr->Stream.Frl.LtpMatchPollCounts = 0;

		/* Check if user callback has been registered*/
		if (InstancePtr->FrlConfigCallback) {
			InstancePtr->FrlConfigCallback(InstancePtr->FrlConfigRef);
		}

		/* Set 4ms on timer 3 for PhyReset callback */
		XV_HdmiRx1_Tmr3Enable(InstancePtr);
		XV_HdmiRx1_TmrStartMs(InstancePtr, 4, 3);

#ifdef DEBUG_RX_FRL_VERBOSITY
		xil_printf(ANSI_COLOR_GREEN "After FrlConfigCallback\r\n"
				ANSI_COLOR_RESET);
#endif
	} else {
#ifdef DEBUG_RX_FRL_VERBOSITY
		xil_printf(ANSI_COLOR_YELLOW "---111 " ANSI_COLOR_RESET);
#endif
		InstancePtr->Stream.Frl.TrainingState =
				XV_HDMIRX1_FRLSTATE_LTS_L;

		XV_HdmiRx1_ExecFrlState(InstancePtr);
		InstancePtr->Stream.State = XV_HDMIRX1_STATE_STREAM_DOWN;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State 3 (LTP detected).
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiRx1_ExecFrlState_Lts3_LtpDetected(XV_HdmiRx1 *InstancePtr)
{
#ifdef DEBUG_RX_FRL_VERBOSITY
xil_printf(ANSI_COLOR_MAGENTA "RX: LTS:3 LTP Detected (%d)\r\n"
	   ANSI_COLOR_RESET, XV_HdmiRx1_GetTmr1Value(InstancePtr));
#endif
	int Status = XST_FAILURE;

	/* Make sure Phy is reset at least once after the patterns have
	 * matched */
	if (InstancePtr->Stream.Frl.LtpMatchedCounts < 1) {
		InstancePtr->Stream.Frl.LtpMatchedCounts++;
		InstancePtr->Stream.Frl.LtpMatchPollCounts = 0;
		InstancePtr->Stream.Frl.LtpMatchWaitCounts = 0;
		if (InstancePtr->PhyResetCallback) {
			InstancePtr->PhyResetCallback(InstancePtr->PhyResetRef);
		}

		InstancePtr->Stream.Frl.TrainingState =
				XV_HDMIRX1_FRLSTATE_LTS_3;
		return Status;
	}

	u8 Data = XV_HdmiRx1_GetPatternsMatchStatus(InstancePtr);

	/* If LTPs are detected on all active lanes */
	if ((InstancePtr->Stream.Frl.Lanes == 3 ? 0x7 : 0xF) == Data) {
		/* Disable Timer 3 which triggers Phy Reset */
		XV_HdmiRx1_TmrStartMs(InstancePtr, 0, 3);
		XV_HdmiRx1_Tmr3Disable(InstancePtr);

		/* 0x0 = Link Training Passing */
		InstancePtr->Stream.Frl.Ltp.Data = 0x00000000;

		InstancePtr->Stream.Frl.TrainingState =
				XV_HDMIRX1_FRLSTATE_LTS_3_RDY;

		/* Check if user callback has been registered*/
		if (InstancePtr->FrlLtsPCallback) {
			InstancePtr->FrlLtsPCallback(InstancePtr->FrlLtsPRef);
		}
#ifdef DEBUG_RX_FRL_VERBOSITY
		xil_printf("LTP_DET:MATCH\r\n");
#endif
	} else {
		InstancePtr->Stream.Frl.TrainingState =
				XV_HDMIRX1_FRLSTATE_LTS_3;
#ifdef DEBUG_RX_FRL_VERBOSITY
		xil_printf("LTP_DET:FALSE:%X\r\n", Data);
#endif
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State 3 (Timer Event).
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiRx1_ExecFrlState_Lts3_Timer(XV_HdmiRx1 *InstancePtr)
{
	int Status = XST_FAILURE;

	u8 Data = XV_HdmiRx1_GetPatternsMatchStatus(InstancePtr);

	InstancePtr->Stream.Frl.FltNoRetrain =
			XV_HdmiRx1_FrlDdcReadField(InstancePtr,
					XV_HDMIRX1_SCDCFIELD_FLT_NO_RETRAIN);

	InstancePtr->Stream.Frl.TimerCnt +=
			FrlTimeoutLts3[InstancePtr->Stream.Frl.FfeLevels];

	if (InstancePtr->Stream.Frl.TrainingState ==
			XV_HDMIRX1_FRLSTATE_LTS_P) {
#ifdef DEBUG_RX_FRL_VERBOSITY
		xil_printf(ANSI_COLOR_RED "RX: LTS:P Lts3_Timer OUT,"
				" FFE_LVL: %d\r\n" ANSI_COLOR_RESET,
				InstancePtr->Stream.Frl.FfeLevels);
#endif
	}

	if ((InstancePtr->Stream.Frl.TrainingState ==
	     XV_HDMIRX1_FRLSTATE_LTS_3) ||
	    (InstancePtr->Stream.Frl.TrainingState ==
	     XV_HDMIRX1_FRLSTATE_LTS_3_TMR) ||
	    (InstancePtr->Stream.Frl.TrainingState ==
	     XV_HDMIRX1_FRLSTATE_LTS_3_RATE_CH) ||
	    (InstancePtr->Stream.Frl.TrainingState ==
	     XV_HDMIRX1_FRLSTATE_LTS_3_RDY)) {
		if (InstancePtr->Stream.Frl.FltNoTimeout == FALSE &&
				InstancePtr->Stream.Frl.FltNoRetrain == FALSE) {
			if (InstancePtr->Stream.Frl.TimerCnt >
			    (FrlTimeoutLts3[InstancePtr->Stream.Frl.FfeLevels] *
			     InstancePtr->Stream.Frl.FfeLevels)) {
				/* If LTPs are not detected on all active
				 * lanes */
				if ((InstancePtr->Stream.Frl.Lanes == 3 ?
				     0x7 : 0xF) !=
				    Data) {
					/* Stop the timer which will initiate
					 * Phy reset */
					XV_HdmiRx1_TmrStartMs(InstancePtr,
							0, 3);

#ifdef DEBUG_RX_FRL_VERBOSITY
					xil_printf("T:0xF\r\n");
#endif
					/* 0xF = Request to change FRL Rate */
					InstancePtr->Stream.Frl.Ltp.Byte[0] = 0xF;
					InstancePtr->Stream.Frl.Ltp.Byte[1] = 0xF;
					InstancePtr->Stream.Frl.Ltp.Byte[2] = 0xF;
					InstancePtr->Stream.Frl.Ltp.Byte[3] = 0xF;

					/* Check if user callback has been
					 * registered */
					if (InstancePtr->FrlLts4Callback) {
						InstancePtr->FrlLts4Callback(
							InstancePtr->FrlLts4Ref);
					}
				}

				Status = XST_SUCCESS;
			} else if (InstancePtr->Stream.Frl.FfeSuppFlag == TRUE) {
				for (u8 ln = 0 ;
				     ln < InstancePtr->Stream.Frl.Lanes ;
				     ln++) {
					/* If any lane is not passing by link training */
					if (((Data >> ln) & 0x1) != 0x01) {
						/* 0xE = Request to change TxFFE */
						InstancePtr->Stream.Frl.Ltp.Byte[ln] = 0xE;
#ifdef DEBUG_RX_FRL_VERBOSITY
						xil_printf(ANSI_COLOR_RED "RX:"
							"%d:0xE\r\n"
							ANSI_COLOR_RESET, ln);
#endif
					}
				}

				XV_HdmiRx1_ResetFrlLtpDetection(InstancePtr);

				XV_HdmiRx1_SetFrlTimer(InstancePtr,
					FrlTimeoutLts3[InstancePtr->Stream.Frl.FfeLevels]);
				Status = XST_SUCCESS;
			}
		}
	}

	if (InstancePtr->Stream.Frl.TrainingState !=
			XV_HDMIRX1_FRLSTATE_LTS_3_RDY) {
		InstancePtr->Stream.Frl.TrainingState =
				XV_HDMIRX1_FRLSTATE_LTS_3;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State 3.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiRx1_ExecFrlState_Lts3(XV_HdmiRx1 *InstancePtr)
{
#ifdef DEBUG_RX_FRL_VERBOSITY
xil_printf(ANSI_COLOR_MAGENTA "RX: LTS:3 (%d)\r\n" ANSI_COLOR_RESET,
	   XV_HdmiRx1_GetTmr1Value(InstancePtr));
#endif
	int Status = XST_FAILURE;

	Status = XV_HdmiRx1_ConfigFrlLtpDetection(InstancePtr);

	if (Status == XST_SUCCESS) {
		if (InstancePtr->Stream.Frl.TrainingState ==
		    XV_HDMIRX1_FRLSTATE_LTS_3_RDY) {
			InstancePtr->Stream.Frl.TrainingState =
					XV_HDMIRX1_FRLSTATE_LTS_P;

#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf("\r\n" ANSI_COLOR_GREEN "RX: "
				   "LTP Pass\r\n" ANSI_COLOR_RESET);
#endif
			/* Disable timer */
			XV_HdmiRx1_SetFrlTimer(InstancePtr, 0);
			Status = XST_SUCCESS;
		} else if (InstancePtr->Stream.Frl.TrainingState ==
		           XV_HDMIRX1_FRLSTATE_LTS_3_TMR) {

		} else if (InstancePtr->Stream.Frl.TrainingState ==
		           XV_HDMIRX1_FRLSTATE_LTS_3) {
		} else {
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf(" -->ELSE\r\n" );
#endif
		}

		Status = XST_SUCCESS;
	} else if (Status == XST_FAILURE) {
#ifdef DEBUG_RX_FRL_VERBOSITY
		xil_printf("\r\n" ANSI_COLOR_RED"RX: LTS_3-->FLT_UPDATE not "
			   "Cleared (%d)\r\n" ANSI_COLOR_RESET,
			   XV_HdmiRx1_GetTmr1Value(InstancePtr));
#endif
		/* Source has not cleared FLT_update so sink should not update
		 * FLT_req and FLT_update as to ensure proper data handshake */
	} else {
		/* Source has cleared FLT_update but no
		 * update from sink is required */
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State P.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiRx1_ExecFrlState_LtsP(XV_HdmiRx1 *InstancePtr)
{
#ifdef DEBUG_RX_FRL_VERBOSITY
	xil_printf(ANSI_COLOR_MAGENTA "RX: LTS:P\r\n" ANSI_COLOR_RESET);
#endif
	int Status = XST_FAILURE;

	if ((InstancePtr->Stream.Frl.TrainingState ==
	     XV_HDMIRX1_FRLSTATE_LTS_P_FRL_RDY) &&
	    InstancePtr->Stream.Frl.FltUpdateAsserted == FALSE) {
#ifdef DEBUG_RX_FRL_VERBOSITY
		xil_printf(ANSI_COLOR_MAGENTA "RX: LTS:"
			   "P_FRL_RDY\r\n" ANSI_COLOR_RESET);
#endif

		XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
					    XV_HDMIRX1_SCDCFIELD_FRL_START,
					    1);
#ifdef DEBUG_RX_FRL_VERBOSITY
		xil_printf(ANSI_COLOR_GREEN "RX: FRL_START\r\n"
				ANSI_COLOR_RESET);
#endif

		if (InstancePtr->FrlStartCallback) {
			InstancePtr->FrlStartCallback(InstancePtr->FrlStartRef);
		}

		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State P when it timed out.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiRx1_ExecFrlState_LtsP_Timeout(XV_HdmiRx1 *InstancePtr)
{
#ifdef DEBUG_RX_FRL_VERBOSITY
	xil_printf(ANSI_COLOR_MAGENTA "RX: LTS:P Timeout\r\n"
			ANSI_COLOR_RESET);
#endif
	/* Check if user callback has been registered*/
	if (InstancePtr->TmdsConfigCallback) {
		InstancePtr->TmdsConfigCallback(InstancePtr->TmdsConfigRef);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function executes the different of states of FRL.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_ExecFrlState(XV_HdmiRx1 *InstancePtr)
{
	int Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);

	switch (InstancePtr->Stream.Frl.TrainingState) {
	case XV_HDMIRX1_FRLSTATE_LTS_L:
		Status = XV_HdmiRx1_ExecFrlState_LtsL(InstancePtr);
		Status = XST_SUCCESS;
#ifdef DEBUG_RX_FRL_VERBOSITY
		xil_printf("---LTSL:\r\n");
#endif
		break;

	case XV_HDMIRX1_FRLSTATE_LTS_2:
		Status = XV_HdmiRx1_ExecFrlState_Lts2(InstancePtr);
		Status = XST_SUCCESS;
		break;

	case XV_HDMIRX1_FRLSTATE_LTS_3_RATE_CH:
		Status = XV_HdmiRx1_ExecFrlState_Lts3_RateChange(InstancePtr);
		/* Note : With some sources such as Realtek, the execution
		 * of LTS3 state can be removed to check if the system still
		 * works.
		 */
		Status = XV_HdmiRx1_ExecFrlState_Lts3(InstancePtr);
		break;

	case XV_HDMIRX1_FRLSTATE_LTS_3_ARM_LNK_RDY:
	case XV_HDMIRX1_FRLSTATE_LTS_3_ARM_VID_RDY:
		Status = XST_SUCCESS;
		break;

	case XV_HDMIRX1_FRLSTATE_LTS_3_LTP_DET:
		Status = XV_HdmiRx1_ExecFrlState_Lts3_LtpDetected(InstancePtr);
		Status = XV_HdmiRx1_ExecFrlState_Lts3(InstancePtr);
		break;

	case XV_HDMIRX1_FRLSTATE_LTS_3_TMR:
		Status = XV_HdmiRx1_ExecFrlState_Lts3_Timer(InstancePtr);
		Status = XV_HdmiRx1_ExecFrlState_Lts3(InstancePtr);
		break;

	case XV_HDMIRX1_FRLSTATE_LTS_3:
	case XV_HDMIRX1_FRLSTATE_LTS_3_RDY:
		Status = XV_HdmiRx1_ExecFrlState_Lts3(InstancePtr);
		break;

	case XV_HDMIRX1_FRLSTATE_LTS_P:
	case XV_HDMIRX1_FRLSTATE_LTS_P_FRL_RDY:
	case XV_HDMIRX1_FRLSTATE_LTS_P_VID_RDY:
		Status = XV_HdmiRx1_ExecFrlState_LtsP(InstancePtr);
		break;

	case XV_HDMIRX1_FRLSTATE_LTS_P_TIMEOUT:
		Status = XV_HdmiRx1_ExecFrlState_LtsP_Timeout(InstancePtr);
		break;

	default:
		xil_printf("RX:S:FRL_INVALID_STATE(%d)!\r\n",
				InstancePtr->Stream.Frl.TrainingState);
		break;
	}

	if (Status == XST_FAILURE) {

	}

    return Status;
}

/*****************************************************************************/
/**
*
* This function clears the set LTP requested in order to restart the link
* training process.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static void XV_HdmiRx1_ClearFrlLtp(XV_HdmiRx1 *InstancePtr)
{
	for (u8 ln = 0; ln < 4; ln++) {
		XV_HdmiRx1_SetFrlLtpDetection(InstancePtr, ln,
					      XV_HDMIRX1_LTP_RATE_CHANGE);
		XV_HdmiRx1_ResetFrlLtpDetection(InstancePtr);
	}
}

/*****************************************************************************/
/**
*
* This function returns the status of the patterns matched lanes
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiRx1_GetPatternsMatchStatus(XV_HdmiRx1 *InstancePtr)
{
	u32 Data = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  XV_HDMIRX1_FRL_STA_OFFSET);

	Data = (Data >> XV_HDMIRX1_FRL_STA_FLT_PM_ALLL_SHIFT) &
		XV_HDMIRX1_FRL_STA_FLT_PM_ALLL_MASK;

	return Data;
}

/*****************************************************************************/
/**
*
* This function polls the pattern matching status and decide if the Phy needs
* to be reset or not.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_PhyResetPoll(XV_HdmiRx1 *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u8 Data = XV_HdmiRx1_GetPatternsMatchStatus(InstancePtr);

	/* Polls every 4ms */
	XV_HdmiRx1_TmrStartMs(InstancePtr, 4, 3);

	/* One or more lanes are patterns matched but the remaining
	 * lanes failed to patterns matched within 4ms or no patterns
	 * have been matched for up to 12ms, proceed to reset Phy */
	if (InstancePtr->Stream.Frl.LtpMatchWaitCounts >= 1 ||
			InstancePtr->Stream.Frl.LtpMatchPollCounts >= 3) {
		InstancePtr->Stream.Frl.LtpMatchPollCounts = 0;
		InstancePtr->Stream.Frl.LtpMatchWaitCounts = 0;

		/* Check if user callback has been registered*/
		if (InstancePtr->PhyResetCallback) {
			InstancePtr->PhyResetCallback(InstancePtr->PhyResetRef);
		}

		return;
	}

	/* Increments the wait counter */
	InstancePtr->Stream.Frl.LtpMatchPollCounts++;

	/* If LTP on some of the lanes are successfully matched */
	if (Data != 0 && Data !=
			(InstancePtr->Stream.Frl.Lanes == 3 ? 0x7 : 0xF)) {
		InstancePtr->Stream.Frl.LtpMatchWaitCounts ++;
	}
}

/*****************************************************************************/
/**
*
* This function sets the link training pattern to be detected for the selected
* lane.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @param    Lane specifies the lane of which the Link Training Pattern will be
* 			detected for.
*
* @param    Ltp specifies Link Training Pattern
*			- 5 = LTP5 / LFSR 0
*			- 6 = LTP6 / LFSR 1
*			- 7 = LTP7 / LFSR 2
*			- 8 = LTP8 / LFSR 3
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_SetFrlLtpDetection(XV_HdmiRx1 *InstancePtr, u8 Lane,
					XV_HdmiRx1_FrlLtpType Ltp)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u32 value = Ltp;

	if (Lane <= 3) {
		XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
				(XV_HDMIRX1_SCDCFIELD_LN0_LTP_REQ + Lane),
				value);
	} else {
		xil_printf("RX:ERROR, Wrong lane is selected!\r\n");
	}
}

/*****************************************************************************/
/**
*
* This function returns the link training pattern to be detected for the
* selected lane.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @param    Lane specifies the lane of which the Link Training Pattern will be
* 			returned.
*
* @return   Link Training Pattern
*			- 5 = LTP5 / LFSR 0
*			- 6 = LTP6 / LFSR 1
*			- 7 = LTP7 / LFSR 2
*			- 8 = LTP8 / LFSR 3
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiRx1_GetFrlLtpDetection(XV_HdmiRx1 *InstancePtr, u8 Lane)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
/*	Xil_AssertVoid(Ltp >= 5 && Ltp <= 8);*/

	u32 Data = 0;

	if (Lane <= 3) {
		Data = XV_HdmiRx1_FrlDdcReadField(InstancePtr,
				(XV_HDMIRX1_SCDCFIELD_LN0_LTP_REQ + Lane));
	} else {
#ifdef DEBUG_RX_FRL_VERBOSITY
		xil_printf("RX:ERROR, Wrong lane is selected!\r\n");
#endif
	}

	return Data;
}

/*****************************************************************************/
/**
*
* This function provides FRL Ratio (Total Pixel)
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   FRL Clock Ratio (Total Pixel)
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiRx1_GetFrlTotalPixRatio(XV_HdmiRx1 *InstancePtr)
{
	u32 FRL_Ratio;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read FRL_Ratio value */
	FRL_Ratio = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				       XV_HDMIRX1_FRL_RATIO_TOT_OFFSET);

	return FRL_Ratio;
}

/*****************************************************************************/
/**
*
* This function provides FRL Ratio (Active Pixel)
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return   FRL Clock Ratio (Active Pixel)
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiRx1_GetFrlActivePixRatio(XV_HdmiRx1 *InstancePtr)
{
	u32 FRL_Ratio;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Read FRL_Ratio value */
	FRL_Ratio = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				       (XV_HDMIRX1_FRL_RATIO_ACT_OFFSET));

	return FRL_Ratio;
}

/*****************************************************************************/
/**
*
* This function reset the link training pattern for the specified lane. This is
* needed whenever the link training pattern is changed or the RxFFE is
* changed.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @param    Lane specifies the lane of which the Link Training Pattern will be
* 			detected for.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_ResetFrlLtpDetection(XV_HdmiRx1 *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    XV_HDMIRX1_FRL_CTRL_SET_OFFSET,
			    XV_HDMIRX1_FRL_CTRL_FLT_CLR_MASK);

	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    XV_HDMIRX1_FRL_CTRL_CLR_OFFSET,
			    XV_HDMIRX1_FRL_CTRL_FLT_CLR_MASK);
}

/*****************************************************************************/
/**
*
* This function enables the LTP detection module.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_FrlLtpDetectionEnable(XV_HdmiRx1 *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    XV_HDMIRX1_FRL_CTRL_CLR_OFFSET,
			    XV_HDMIRX1_FRL_CTRL_FLT_CLR_MASK);
}

/*****************************************************************************/
/**
*
* This function disables the LTP detection module.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_FrlLtpDetectionDisable(XV_HdmiRx1 *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    XV_HDMIRX1_FRL_CTRL_SET_OFFSET,
			    XV_HDMIRX1_FRL_CTRL_FLT_CLR_MASK);
}

/*****************************************************************************/
/**
*
* This function sets the number of times the full link training patterns need
* to be matched before it is considered as a lock.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @param    Threshold specifies the number of times the full link training
* 			patterns need to be matched.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_SetFrlLtpThreshold(XV_HdmiRx1 *InstancePtr, u8 Threshold)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u32 Data;

	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  XV_HDMIRX1_FRL_CTRL_OFFSET);

	Data = Data & ~((u32)(XV_HDMIRX1_FRL_CTRL_FLT_THRES_MASK <<
			XV_HDMIRX1_FRL_CTRL_FLT_THRES_SHIFT));
	Data = Data | ((Threshold & XV_HDMIRX1_FRL_CTRL_FLT_THRES_MASK) <<
			XV_HDMIRX1_FRL_CTRL_FLT_THRES_SHIFT);

	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    XV_HDMIRX1_FRL_CTRL_OFFSET, Data);
}

/*****************************************************************************/
/**
*
* This function configures the link training pattern to be detected
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return	Status
*		- XST_FAILURE
* 		- Source has not cleared FLT_update so sink should not update
* 		  FLT_req and FLT_update as to ensure proper data handshake
* 		- XST_SUCCESS
* 		- Source has cleared FLT_update and sink has updated LTP_req
* 		  and set FLT_update to 1
* 		- XST_NO_DATA
* 		- Source has cleared FLT_update but no update from
* 		  sink is required
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_ConfigFrlLtpDetection(XV_HdmiRx1 *InstancePtr)
{
	int Status = XST_FAILURE;
	u32 Data = 0;
	u32 ConfiguredLtp = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Data = InstancePtr->Stream.Frl.FltUpdateAsserted;

	/* Check if source has read and cleared FLT_update, FALSE = Cleared */
	if (Data == FALSE) {
		for (u8 ln = 0; ln < 4; ln++) {
			ConfiguredLtp = XV_HdmiRx1_GetFrlLtpDetection(InstancePtr, ln);

			/* If the lane was previously configured as 0xE, it needs to be
			 * configured back to the LTP to resume link training. */
			if (ConfiguredLtp == 0xE) {
				InstancePtr->Stream.Frl.Ltp.Byte[ln] =
						InstancePtr->Stream.Frl.DefaultLtp.Byte[ln];
			}

			/* Check if the LTP data requires updating */
			if (ConfiguredLtp != InstancePtr->Stream.Frl.Ltp.Byte[ln]) {
				XV_HdmiRx1_SetFrlLtpDetection(InstancePtr, ln,
						InstancePtr->Stream.Frl.Ltp.Byte[ln]);
				Data = TRUE;
			}
		}

		if (Data == TRUE) {
#ifdef DEBUG_RX_FRL_VERBOSITY
			xil_printf(ANSI_COLOR_MAGENTA
					"RX: LTPREQ: %X %X %X %X\r\n"
					ANSI_COLOR_RESET
					, InstancePtr->Stream.Frl.Ltp.Byte[0],
					InstancePtr->Stream.Frl.Ltp.Byte[1],
					InstancePtr->Stream.Frl.Ltp.Byte[2],
					InstancePtr->Stream.Frl.Ltp.Byte[3]);
			xil_printf(ANSI_COLOR_GREEN "Assert FLT_UPDATE "
				   "(%d)\r\n" ANSI_COLOR_RESET,
				   XV_HdmiRx1_GetTmr1Value(InstancePtr));
#endif
			XV_HdmiRx1_FrlFltUpdate(InstancePtr, TRUE);
			XV_HdmiRx1_ResetFrlLtpDetection(InstancePtr);
			Status = XST_SUCCESS;
		} else {
			/* No updates are made */
			Status = XST_NO_DATA;
		}
	} else {
		/* FLT_update not cleared */
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function updates the software's FRL Rate and FRL Lanes by reading and
* decoding the information from the RX core.
*
* @param	InstancePtr is a pointer to the XHdmi_Rx core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int XV_HdmiRx1_RetrieveFrlRateLanes(XV_HdmiRx1 *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status = XST_FAILURE;
	u32 Data;

	Data = XV_HdmiRx1_FrlDdcReadField(InstancePtr,
					  XV_HDMIRX1_SCDCFIELD_FRL_RATE);

	switch (Data) {
	case 6:
		InstancePtr->Stream.Frl.LineRate = 12;
		InstancePtr->Stream.Frl.Lanes = 4;
		Status = XST_SUCCESS;
		break;

	case 5:
		InstancePtr->Stream.Frl.LineRate = 10;
		InstancePtr->Stream.Frl.Lanes = 4;
		Status = XST_SUCCESS;
		break;

	case 4:
		InstancePtr->Stream.Frl.LineRate = 8;
		InstancePtr->Stream.Frl.Lanes = 4;
		Status = XST_SUCCESS;
		break;

	case 3:
		InstancePtr->Stream.Frl.LineRate = 6;
		InstancePtr->Stream.Frl.Lanes = 4;
		Status = XST_SUCCESS;
		break;

	case 2:
		InstancePtr->Stream.Frl.LineRate = 6;
		InstancePtr->Stream.Frl.Lanes = 3;
		Status = XST_SUCCESS;
		break;

	case 1:
		InstancePtr->Stream.Frl.LineRate = 3;
		InstancePtr->Stream.Frl.Lanes = 3;
		Status = XST_SUCCESS;
		break;

	default:
		InstancePtr->Stream.Frl.LineRate = 0 ;
		InstancePtr->Stream.Frl.Lanes = 0;
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function initiates FRL rate dropping procedure.
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @param    LtpThreshold specifies the number of times the LTP matching module
*           must match against the incoming link training pattern before a
*           match is indicated
*
* @param    DefaultLtp specify the link training pattern which will be used
* 			for link training purposes
* 			- XV_HDMIRX1_LTP_LFSR0
* 			- XV_HDMIRX1_LTP_LFSR1
* 			- XV_HDMIRX1_LTP_LFSR2
* 			- XV_HDMIRX1_LTP_LFSR3
*
* @return	Status on if FrlTraining can be started or not.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_FrlLinkRetrain(XV_HdmiRx1 *InstancePtr, u8 LtpThreshold,
		XV_HdmiRx1_FrlLtp DefaultLtp)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(DefaultLtp.Byte[0] >= 5 && DefaultLtp.Byte[0] <= 8);
	Xil_AssertVoid(DefaultLtp.Byte[1] >= 5 && DefaultLtp.Byte[1] <= 8);
	Xil_AssertVoid(DefaultLtp.Byte[2] >= 5 && DefaultLtp.Byte[2] <= 8);
	Xil_AssertVoid((DefaultLtp.Byte[3] >= 5 && DefaultLtp.Byte[3] <= 8) ||
		       DefaultLtp.Byte[3] == 0);

	/* InstancePtr->Stream.Frl.DefaultLtp.Data = DefaultLtp.Data;*/
	InstancePtr->Stream.Frl.DefaultLtp.Byte[0] = DefaultLtp.Byte[0];
	InstancePtr->Stream.Frl.DefaultLtp.Byte[1] = DefaultLtp.Byte[1];
	InstancePtr->Stream.Frl.DefaultLtp.Byte[2] = DefaultLtp.Byte[2];
	InstancePtr->Stream.Frl.DefaultLtp.Byte[3] = DefaultLtp.Byte[3];

	/* Request for rate drop */
	InstancePtr->Stream.Frl.Ltp.Byte[0] = 0xF;
	InstancePtr->Stream.Frl.Ltp.Byte[1] = 0xF;
	InstancePtr->Stream.Frl.Ltp.Byte[2] = 0xF;
	InstancePtr->Stream.Frl.Ltp.Byte[3] = 0xF;

	XV_HdmiRx1_SetFrlLtpThreshold(InstancePtr, LtpThreshold);

	InstancePtr->Stream.Frl.TrainingState = XV_HDMIRX1_FRLSTATE_LTS_3;

	XV_HdmiRx1_ExecFrlState(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function reads the specified FRL SCDC Field
*
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @param    Field specifies the fields from SCDC channels to be written
*
* @param    Value specifies the values to be written
*
* @return
*       - XST_SUCCESS
*       - XST_FAILURE
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiRx1_FrlDdcReadField(XV_HdmiRx1 *InstancePtr,
			XV_HdmiRx1_FrlScdcFieldType Field)
{
	u32 Data;

	Data = 0xFFFFFFFF;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  XV_HDMIRX1_FRL_SCDC_OFFSET);

	for (u16 i = 0; i < 100; i++) {
		Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
					  XV_HDMIRX1_FRL_SCDC_OFFSET);

		if (Data & XV_HDMIRX1_FRL_SCDC_RDY_MASK) {
			break;
		}
	}

	if ((Data & XV_HDMIRX1_FRL_SCDC_RDY_MASK) == 0) {
		xil_printf(ANSI_COLOR_RED "XV_HdmiRx1_FrlDdcReadField "
			   "F1\r\n" ANSI_COLOR_RESET);
		/* SCDC access is busy*/
		return Data;
	}

	Data = (XV_HDMIRX1_FRL_SCDC_ADDR_MASK & FrlScdcField[Field].Offset) |
			XV_HDMIRX1_FRL_SCDC_RD_MASK;


	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    XV_HDMIRX1_FRL_SCDC_OFFSET, Data);

	for (u16 i = 0; i < 100; i++) {
		Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
					  XV_HDMIRX1_FRL_SCDC_OFFSET);

		if (Data & XV_HDMIRX1_FRL_SCDC_RDY_MASK) {
			Data = Data >> XV_HDMIRX1_FRL_SCDC_DAT_SHIFT;
			return ((Data >> FrlScdcField[Field].Shift) &
					FrlScdcField[Field].Mask);
		}
	}

	xil_printf(ANSI_COLOR_RED "XV_HdmiRx1_FrlDdcReadField F2\r\n"
			ANSI_COLOR_RESET);
	return 0xFFFFFFFF;
}

/*****************************************************************************/
/**
*
* This function writes the specified FRL SCDC Field
*
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @param    Field specifies the fields from SCDC channels to be written
*
* @param    Value specifies the values to be written
*
* @return
*       - XST_SUCCESS
*       - XST_FAILURE
*
* @note     None.
*
******************************************************************************/
int XV_HdmiRx1_FrlDdcWriteField(XV_HdmiRx1 *InstancePtr,
				XV_HdmiRx1_FrlScdcFieldType Field,
				u8 Value)
{
	u32 Data = 0;
	u32 Status = XST_FAILURE;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	 /* Only do read-modify-write if only some bits need to be updated. */
	if (FrlScdcField[Field].Mask != 0xFF) {
		Data = XV_HdmiRx1_FrlDdcReadField(InstancePtr, Field);
	}

	if (Data == 0xFFFFFFFF) {
		return Status;
	}

	/* Check if SCDC access is busy or not */
	Data = XV_HdmiRx1_ReadReg(InstancePtr->Config.BaseAddress,
				  XV_HDMIRX1_FRL_SCDC_OFFSET);

	if ((Data & XV_HDMIRX1_FRL_SCDC_RDY_MASK) == 0) {
		/* SCDC access is busy*/
		return Status;
	}

	if (FrlScdcField[Field].Mask != 0xFF) {
		Data &= ~((FrlScdcField[Field].Mask <<
		           FrlScdcField[Field].Shift) <<
		          XV_HDMIRX1_FRL_SCDC_DAT_SHIFT);
	} else {
		Data &= ~((u32)(XV_HDMIRX1_FRL_SCDC_DAT_MASK <<
		                XV_HDMIRX1_FRL_SCDC_DAT_SHIFT));
	}

	Data &= ~((u32)(XV_HDMIRX1_FRL_SCDC_ADDR_MASK));

	Data |= (((Value & FrlScdcField[Field].Mask) <<
	          FrlScdcField[Field].Shift) <<
	         XV_HDMIRX1_FRL_SCDC_DAT_SHIFT) |
	        (FrlScdcField[Field].Offset & XV_HDMIRX1_FRL_SCDC_ADDR_MASK) |
	        XV_HDMIRX1_FRL_SCDC_WR_MASK;

	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    XV_HDMIRX1_FRL_SCDC_OFFSET, Data);

	return Status;
}

/*****************************************************************************/
/**
*
* This function sets the FRL rate write enable Event
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_SetFrlRateWrEvent_En(XV_HdmiRx1 *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
			    XV_HDMIRX1_FRL_CTRL_SET_OFFSET,
			    XV_HDMIRX1_FRL_CTRL_FRL_RATE_WR_EVT_EN_MASK);
}

/*****************************************************************************/
/**
*
* This function resets the FRL peripheral
*
* @param    InstancePtr is a pointer to the XV_HdmiRx1 core instance.
*
* @param    Reset specifies if the FRL peripheral is under reset or not.
* 			- 0 = Reset released
*			- 1 = Reset asserted
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRx1_FrlReset(XV_HdmiRx1 *InstancePtr, u8 Reset)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Reset == 0 || Reset == 1);

	if (Reset) {
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_FRL_CTRL_CLR_OFFSET),
				    (XV_HDMIRX1_FRL_CTRL_RSTN_MASK));
		/* xil_printf("RESET SET\r\n");*/
		XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
					    XV_HDMIRX1_SCDCFIELD_SINK_VER,
					    1);
		XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
					    XV_HDMIRX1_SCDCFIELD_SCRAMBLER_EN,
					    0);
	} else {
		XV_HdmiRx1_WriteReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMIRX1_FRL_CTRL_SET_OFFSET),
				    (XV_HDMIRX1_FRL_CTRL_RSTN_MASK));
		XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
					    XV_HDMIRX1_SCDCFIELD_FRL_RATE,
					    0);
	}
}

/*****************************************************************************/
/**
*
* This function sets the timer of RX Core.
*
* @param	InstancePtr is a pointer to the XHdmi_Rx core instance.
*
* @param	Milliseconds specifies the timer's frequency (in milliseconds)
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiRx1_SetFrlTimer(XV_HdmiRx1 *InstancePtr, u32 Milliseconds)
{
	/* FRL uses Timer1 */
	XV_HdmiRx1_TmrStartMs(InstancePtr, Milliseconds, 1);
}

/*****************************************************************************/
/**
*
* This function sets the timer of RX Core's FRL peripheral for 10 Microseconds.
*
* @param	InstancePtr is a pointer to the XHdmi_Rx core instance.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiRx1_SetFrl10MicroSecondsTimer(XV_HdmiRx1 *InstancePtr)
{
	u32 ClockCycles;

	ClockCycles = InstancePtr->Config.AxiLiteClkFreq / 100000;

	XV_HdmiRx1_Tmr1Start(InstancePtr, ClockCycles);
}

void XV_HdmiRx1_FrlFltUpdate(XV_HdmiRx1 *InstancePtr, u8 Flag)
{
	Xil_AssertVoid(Flag == TRUE || Flag == FALSE);

	XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
				    XV_HDMIRX1_SCDCFIELD_FLT_UPDATE,
				    Flag);

	InstancePtr->Stream.Frl.FltUpdateAsserted = Flag;
}

void XV_HdmiRx1_SetFrlFltNoTimeout(XV_HdmiRx1 *InstancePtr)
{
	if (InstancePtr->Stream.Frl.FltNoTimeout == TRUE) {
		XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
				XV_HDMIRX1_SCDCFIELD_FLT_NO_TIMEOUT,
				1);
		xil_printf(ANSI_COLOR_MAGENTA "FLT_NO_TIMEOUT, "
			   "with SCDC" ANSI_COLOR_RESET);
	} else {
		InstancePtr->Stream.Frl.FltNoTimeout = (TRUE);
		xil_printf(ANSI_COLOR_MAGENTA "FLT_NO_TIMEOUT, "
			   "without SCDC" ANSI_COLOR_RESET);
	}
}

void XV_HdmiRx1_ClearFrlFltNoTimeout(XV_HdmiRx1 *InstancePtr)
{
	InstancePtr->Stream.Frl.FltNoTimeout = (FALSE);
	XV_HdmiRx1_FrlDdcWriteField(InstancePtr,
				    XV_HDMIRX1_SCDCFIELD_FLT_NO_TIMEOUT,
				    0);
}

void XV_HdmiRx1_RestartFrlLt(XV_HdmiRx1 *InstancePtr)
{
	xil_printf("RestartFrlLt_0: S: %X\r\n",
		   InstancePtr->Stream.Frl.TrainingState);
	if (InstancePtr->Stream.Frl.TrainingState >=
	    XV_HDMIRX1_FRLSTATE_LTS_P_FRL_RDY) {

		/* Initialize the LTP detection with the default LTP */
		InstancePtr->Stream.Frl.Ltp.Byte[0] =
				InstancePtr->Stream.Frl.DefaultLtp.Byte[0];
		InstancePtr->Stream.Frl.Ltp.Byte[1] =
				InstancePtr->Stream.Frl.DefaultLtp.Byte[1];
		InstancePtr->Stream.Frl.Ltp.Byte[2] =
				InstancePtr->Stream.Frl.DefaultLtp.Byte[2];
		InstancePtr->Stream.Frl.Ltp.Byte[3] =
				InstancePtr->Stream.Frl.DefaultLtp.Byte[3];

		InstancePtr->Stream.Frl.TrainingState =
				XV_HDMIRX1_FRLSTATE_LTS_3;

		XV_HdmiRx1_ExecFrlState(InstancePtr);

		xil_printf("RestartFrlLt_1\r\n");
	}
}
