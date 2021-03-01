/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitx1.c
*
* This is the main file for Xilinx HDMI TX core. Please see xv_hdmitx1.h for
* more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00  EB     22/05/18 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmitx1_frl.h"
#include "xv_hdmitx1.h"
#include "string.h"

/************************** Constant Definitions *****************************/
#define Timer2MS			2
#define Timer5MS			5
#define Timer100MS			100
#define Timer200MS			200
#define Timer250MS			250

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static void XV_HdmiTx1_ClearFrlLtp(XV_HdmiTx1 *InstancePtr);
static int XV_HdmiTx1_ExecFrlState_LtsL(XV_HdmiTx1 *InstancePtr);
static int XV_HdmiTx1_ExecFrlState_Lts1(XV_HdmiTx1 *InstancePtr);
static int XV_HdmiTx1_ExecFrlState_Lts2(XV_HdmiTx1 *InstancePtr);
static int XV_HdmiTx1_ExecFrlState_Lts2_RateWr(XV_HdmiTx1 *InstancePtr);
static int XV_HdmiTx1_ExecFrlState_Lts3(XV_HdmiTx1 *InstancePtr);
static int XV_HdmiTx1_ExecFrlState_Lts4(XV_HdmiTx1 *InstancePtr);
static int XV_HdmiTx1_ExecFrlState_LtsP_Arm(XV_HdmiTx1 *InstancePtr);
static int XV_HdmiTx1_ExecFrlState_LtsP(XV_HdmiTx1 *InstancePtr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This function starts the TMDS mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return	Status on if TMDS can be started or not.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_StartTmdsMode(XV_HdmiTx1 *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

#ifdef DEBUG_TX_FRL_VERBOSITY
	xil_printf(ANSI_COLOR_CYAN "TX: Start TMDS Mode\r\n" ANSI_COLOR_RESET);
#endif

/*    InstancePtr->Stream.VidMode = XHDMIC_VIDMODE_HDMI_FRL;*/
	InstancePtr->Stream.Frl.TrainingState = XV_HDMITX1_FRLSTATE_LTS_L;

	/* Clear timer event flag*/
	InstancePtr->Stream.Frl.TimerEvent = FALSE;

	/* Set short timer to fire state machine */
	XV_HdmiTx1_SetFrl10MicroSecondsTimer(InstancePtr);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function starts the Fixed Rate Link Training.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @param    FrlRate specifies the FRL rate to be attempted
* 			- 0 = FRL Not Supported
* 			- 1 = 3 Lanes 3Gbps
* 			- 2 = 4 Lanes 3Gbps
*			- 3 = 4 Lanes 6Gbsp
*			- 4 = 4 Lanes 8Gbps
*			- 5 = 4 Lanes 10Gbps
*			- 6 = 4 Lanes 12Gbps
*
* @return	Status on if FrlTraining can be started or not.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_StartFrlTraining(XV_HdmiTx1 *InstancePtr,
		XHdmiC_MaxFrlRate FrlRate)
{
	int Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);

	if (FrlRate > InstancePtr->Stream.Frl.MaxFrlRate) {
		InstancePtr->Stream.Frl.FrlRate =
				InstancePtr->Stream.Frl.MaxFrlRate;
	} else {
		InstancePtr->Stream.Frl.FrlRate = FrlRate;
	}

	InstancePtr->Stream.Frl.LineRate =
			FrlRateTable[InstancePtr->Stream.Frl.FrlRate].LineRate;
	InstancePtr->Stream.Frl.Lanes =
			FrlRateTable[InstancePtr->Stream.Frl.FrlRate].Lanes;
	InstancePtr->Stream.IsFrl = TRUE;
	InstancePtr->Stream.IsHdmi = TRUE;
	InstancePtr->Stream.Frl.TrainingState = XV_HDMITX1_FRLSTATE_LTS_1;

#ifdef DEBUG_TX_FRL_VERBOSITY
	xil_printf(ANSI_COLOR_CYAN "TX: Start FRL training (%d lanes "
		   "@ %d Gbps, FRL_Rate: %d)\r\n" ANSI_COLOR_RESET,
		   InstancePtr->Stream.Frl.Lanes,
		   InstancePtr->Stream.Frl.LineRate, InstancePtr->Stream.Frl.FrlRate);
#endif

	/* Clear timer event flag*/
	InstancePtr->Stream.Frl.TimerEvent = FALSE;

	/* Execute XV_HDMITX1_FRLSTATE_LTS_1 */
	Status = XV_HdmiTx1_ExecFrlState(InstancePtr);

	return Status;
}

/*****************************************************************************/
/**
*
* This function sets maximum FRL Rate supported by the system.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @param    MaxFrlRate specifies maximum rates supported
* 			- 0 = FRL Not Supported
* 			- 1 = 3 Lanes 3Gbps
* 			- 2 = 4 Lanes 3Gbps
*			- 3 = 4 Lanes 6Gbsp
*			- 4 = 4 Lanes 8Gbps
*			- 5 = 4 Lanes 10Gbps
*			- 6 = 4 Lanes 12Gbps
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_SetFrlMaxFrlRate(XV_HdmiTx1 *InstancePtr,
		XHdmiC_MaxFrlRate MaxFrlRate)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if(MaxFrlRate <= InstancePtr->Config.MaxFrlRate)
		InstancePtr->Stream.Frl.MaxFrlRate = MaxFrlRate;
	else {
		InstancePtr->Stream.Frl.MaxFrlRate =
				InstancePtr->Config.MaxFrlRate;
		xil_printf(ANSI_COLOR_CYAN "TX: User Configured FRL Rate is not supported by IP.\r\n");
		xil_printf("Reset to FRL Rate : %d\r\n" ANSI_COLOR_RESET,
				InstancePtr->Config.MaxFrlRate);
	}
}

/*****************************************************************************/
/**
*
* This function stops link training patterns from being transmitted.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static void XV_HdmiTx1_ClearFrlLtp(XV_HdmiTx1 *InstancePtr)
{
	XV_HdmiTx1_SetFrlLtp(InstancePtr, 0, XV_HDMITX1_LTP_NO_LTP);
	XV_HdmiTx1_SetFrlLtp(InstancePtr, 1, XV_HDMITX1_LTP_NO_LTP);
	XV_HdmiTx1_SetFrlLtp(InstancePtr, 2, XV_HDMITX1_LTP_NO_LTP);
	XV_HdmiTx1_SetFrlLtp(InstancePtr, 3, XV_HDMITX1_LTP_NO_LTP);
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State Legacy.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiTx1_ExecFrlState_LtsL(XV_HdmiTx1 *InstancePtr)
{
	int Status = XST_FAILURE;
	u8 DdcBuf;

#ifdef DEBUG_TX_FRL_VERBOSITY
	xil_printf(ANSI_COLOR_RED "XV_HdmiTx1_ExecFrlState_LtsL\r\n"
			ANSI_COLOR_RESET);
#endif
	/* Stops timer */
	XV_HdmiTx1_SetFrlTimer(InstancePtr, 0);

	XV_HdmiTx1_FrlReset(InstancePtr, TRUE);
	XV_HdmiTx1_FrlReset(InstancePtr, FALSE);

	/* Switch to TMDS mode */
	XV_HdmiTx1_FrlModeEn(InstancePtr, FALSE);
	Status = XV_HdmiTx1_FrlRate(InstancePtr,
			XHDMIC_MAXFRLRATE_NOT_SUPPORTED);
/*	XV_HdmiTx1_FrlExecute(InstancePtr);*/

	InstancePtr->Stream.IsFrl = FALSE;

	if (Status != XST_SUCCESS) {
		return Status;
	}

	if (InstancePtr->Stream.ScdcSupport == TRUE) {
		Status = XV_HdmiTx1_DdcReadReg(InstancePtr,
					       XV_HDMITX1_DDC_ADDRESS,
					       1,
					       XV_HDMITX1_DDC_UPDATE_FLGS_REG,
					       (u8*)&DdcBuf);

		if (Status != XST_SUCCESS) {
			return Status;
		}

		if (DdcBuf & XV_HDMITX1_DDC_UPDATE_FLGS_FLT_UPDATE_MASK) {
			/* Clears FLT_update */
			Status = XV_HdmiTx1_DdcWriteField(InstancePtr,
						XV_HDMITX1_SCDCFIELD_FLT_UPDATE,
						1);
		}
	}

	XV_HdmiTx1_FrlStreamStop(InstancePtr);

	if (InstancePtr->TmdsConfigCallback) {
		InstancePtr->TmdsConfigCallback(InstancePtr->TmdsConfigRef);
	}

	XV_HdmiTx1_FrlExecute(InstancePtr);

	return Status;
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State 1.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiTx1_ExecFrlState_Lts1(XV_HdmiTx1 *InstancePtr)
{
	int Status = XST_FAILURE;
	u8 DdcBuf;

#ifdef DEBUG_TX_FRL_VERBOSITY
	xil_printf(ANSI_COLOR_CYAN "TX: FRL LTS:1\r\n" ANSI_COLOR_RESET);
#endif
	/* Read Sink version */
	Status = XV_HdmiTx1_DdcReadReg(InstancePtr,
				       XV_HDMITX1_DDC_ADDRESS,
				       1,
				       XV_HDMITX1_DDC_SINK_VER_REG,
				       (u8*)&DdcBuf);

	/* Make sure sink version > 0 */
	if (Status == (XST_SUCCESS) && DdcBuf != 0) {
		Status = XV_HdmiTx1_DdcWriteField(InstancePtr,
						  XV_HDMITX1_SCDCFIELD_SOURCE_VER,
						  1);
		if (Status == (XST_SUCCESS)) {
			InstancePtr->Stream.Frl.TrainingState =
					XV_HDMITX1_FRLSTATE_LTS_2;
			InstancePtr->Stream.Frl.TimerCnt = 0;
			Status = XST_SUCCESS;
		}
	} else {
    	xil_printf("Status: %d, DdcBuf: %d \r\n",Status,DdcBuf);
    	xil_printf("SCDC Wrong Version on Connected Sink!\r\n");
    	InstancePtr->Stream.Frl.TrainingState = XV_HDMITX1_FRLSTATE_LTS_L;
		Status = XST_FAILURE;
	}


	XV_HdmiTx1_SetFrl10MicroSecondsTimer(InstancePtr);

	return Status;
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State 2.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiTx1_ExecFrlState_Lts2(XV_HdmiTx1 *InstancePtr)
{
	int Status = XST_FAILURE;
	u8 DdcBuf = 0;

#ifdef DEBUG_TX_FRL_VERBOSITY
	xil_printf(ANSI_COLOR_CYAN "TX: FRL LTS:2\r\n" ANSI_COLOR_RESET);
#endif
	XV_HdmiTx1_SetFrlTimer(InstancePtr, Timer5MS);

	InstancePtr->Stream.Frl.TimerCnt += Timer5MS;

	/* Note : Additionally, the FLT_NO_UPDATE SCDC Register start
	 * can be read here.
	 */
	Status = XV_HdmiTx1_DdcReadReg(InstancePtr,
				       XV_HDMITX1_DDC_ADDRESS,
				       1,
				       XV_HDMITX1_DDC_STCR_REG,
				       (u8*)&DdcBuf);

	if (Status == XST_SUCCESS) {
		if (DdcBuf & XV_HDMITX1_DDC_STCR_FLT_NO_TIMEOUT_MASK) {
			InstancePtr->Stream.Frl.FltNoTimeout = TRUE;
#ifdef DEBUG_TX_FRL_VERBOSITY
			xil_printf("LTS2:FLTNOTIMEOUT == TRUE\r\n");
#endif
		} else {
			InstancePtr->Stream.Frl.FltNoTimeout = FALSE;
#ifdef DEBUG_TX_FRL_VERBOSITY
			xil_printf("LTS2:FLTNOTIMEOUT == FALSE\r\n");
#endif
		}

		/* Note : The return status is not handled. */
		XV_HdmiTx1_DdcWriteField(InstancePtr,
					 XV_HDMITX1_SCDCFIELD_SNK_STU,
					 1);
	}
	/* Read FLT_NO_UPDATE SCDC Register end */

	if (InstancePtr->Stream.Frl.FltNoTimeout == TRUE ||
	    InstancePtr->Stream.Frl.TimerCnt < Timer100MS) {
		Status = XV_HdmiTx1_DdcReadReg(InstancePtr,
					       XV_HDMITX1_DDC_ADDRESS,
					       1,
					       XV_HDMITX1_DDC_STAT_FLGS_REG,
					       (u8*)&DdcBuf);

		if (Status == XST_SUCCESS &&
		    (DdcBuf & XV_HDMITX1_DDC_STAT_FLGS_FLT_RDY_MASK)) {
#ifdef DEBUG_TX_FRL_VERBOSITY
			xil_printf(ANSI_COLOR_CYAN "TX: FRL LTS:2 "
				   "FLT_ready\r\n" ANSI_COLOR_RESET);
#endif
			/* Disable timer */
			XV_HdmiTx1_SetFrlTimer(InstancePtr, 0);

/*			if (InstancePtr->Stream.Frl.RateLock == FALSE) {*/
			/* Start FRL rate with maximum supported rate */
/*			}*/

			/* Reset LaneFfeAdjReq so application can reset TxFFE */
			InstancePtr->Stream.Frl.LaneFfeAdjReq.Data = 0;

			if (InstancePtr->FrlFfeCallback) {
				InstancePtr->FrlFfeCallback(InstancePtr->FrlFfeRef);
			}

			/* Reset timer counter */
			InstancePtr->Stream.Frl.TimerCnt = 0;

			InstancePtr->Stream.Frl.TrainingState =
					XV_HDMITX1_FRLSTATE_LTS_3_ARM;

			if (InstancePtr->FrlConfigCallback) {
				InstancePtr->FrlConfigCallback(
						InstancePtr->FrlConfigRef);
			}

			/* Starts sending default data to allow sink's GT to
			 * lock early */
			for (u8 ln = 0; ln < 4; ln++) {
				XV_HdmiTx1_SetFrlLtp(InstancePtr,
						ln,
						XV_HDMITX1_LTP_NYQUIST_CLOCK);
			}

			/* Execute FRL register update*/
			XV_HdmiTx1_FrlExecute(InstancePtr);

			/* Check if user callback has been registered*/
			if (InstancePtr->FrlLts2Callback) {
				InstancePtr->FrlLts2Callback(
						InstancePtr->FrlLts2Ref);
			}

			/* xil_printf(ANSI_COLOR_CYAN "TX: FRL LTS:2 endless"
			 *	      " loop\r\n" ANSI_COLOR_RESET);*/
			/* while (1);*/

		}
	} else { /* Timeout, fallback to LTS:L */
		InstancePtr->Stream.Frl.TrainingState = XV_HDMITX1_FRLSTATE_LTS_L;
		/* Set short timer to jump state */
		XV_HdmiTx1_SetFrl10MicroSecondsTimer(InstancePtr);
	}

	return Status;
}

static int XV_HdmiTx1_ExecFrlState_Lts2_RateWr(XV_HdmiTx1 *InstancePtr)
{
	int Status = XST_FAILURE;

	Status = XV_HdmiTx1_FrlTrainingInit(InstancePtr);

	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Execute FRL register update*/
	XV_HdmiTx1_FrlExecute(InstancePtr);

	InstancePtr->Stream.Frl.TrainingState = XV_HDMITX1_FRLSTATE_LTS_3;

	XV_HdmiTx1_SetFrl10MicroSecondsTimer(InstancePtr);

	return Status;
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State 3.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiTx1_ExecFrlState_Lts3(XV_HdmiTx1 *InstancePtr)
{
	u8 DdcBuf[4];
	u8 FfeAdjFlag = FALSE;
	int Status = XST_FAILURE;
	InstancePtr->DBMessage = 0x00;

	/* 200ms Timeout, fallback to LTS:L */
	if (InstancePtr->Stream.Frl.TimerCnt > Timer200MS &&
	    InstancePtr->Stream.Frl.FltNoTimeout == FALSE) {
		/* Check if user callback has been registered*/
		if (InstancePtr->FrlLts3Callback) {
			InstancePtr->DBMessage = 0xFA;
			InstancePtr->FrlLts3Callback(InstancePtr->FrlLts3Ref);
		}

		InstancePtr->Stream.Frl.TimerCnt = 0;
		InstancePtr->Stream.Frl.TrainingState = XV_HDMITX1_FRLSTATE_LTS_L;
		/* Set short timer to jump state */
		XV_HdmiTx1_SetFrl10MicroSecondsTimer(InstancePtr);
#ifdef DEBUG_TX_FRL_VERBOSITY
		xil_printf(ANSI_COLOR_CYAN "TX: FRL LTS:3 "
			   "TIMEOUT\r\n" ANSI_COLOR_RESET);
#endif
		return Status;
	}

	XV_HdmiTx1_SetFrlTimer(InstancePtr, Timer2MS);

	InstancePtr->Stream.Frl.TimerCnt += Timer2MS;

	Status = XV_HdmiTx1_DdcReadReg(InstancePtr,
				       XV_HDMITX1_DDC_ADDRESS,
				       1,
				       XV_HDMITX1_DDC_UPDATE_FLGS_REG,
				       (u8*)&DdcBuf);

	if (Status != XST_SUCCESS ||
	    (DdcBuf[0] & XV_HDMITX1_DDC_UPDATE_FLGS_FLT_UPDATE_MASK) !=
	     XV_HDMITX1_DDC_UPDATE_FLGS_FLT_UPDATE_MASK) {

		return XST_NO_DATA;
	}

	if (DdcBuf[0] & XV_HDMITX1_DDC_UPDATE_FLGS_STUPDATE_MASK) {
		Status = XV_HdmiTx1_DdcReadReg(InstancePtr,
					       XV_HDMITX1_DDC_ADDRESS,
					       1,
					       XV_HDMITX1_DDC_STCR_REG,
					       (u8*)&DdcBuf);

		if (Status == XST_SUCCESS) {
			if (DdcBuf[0] & XV_HDMITX1_DDC_STCR_FLT_NO_TIMEOUT_MASK) {
				InstancePtr->Stream.Frl.FltNoTimeout = TRUE;
#ifdef DEBUG_TX_FRL_VERBOSITY
				xil_printf("FLTNOTIMEOUT == TRUE\r\n");
#endif
			} else {
				InstancePtr->Stream.Frl.FltNoTimeout = FALSE;
#ifdef DEBUG_TX_FRL_VERBOSITY
				xil_printf("FLTNOTIMEOUT == FALSE\r\n");
#endif
			}

			/* Note : The return status is not handled. */
			XV_HdmiTx1_DdcWriteField(InstancePtr,
						 XV_HDMITX1_SCDCFIELD_SNK_STU,
						 1);
		}
	}

	Status = XV_HdmiTx1_DdcReadReg(InstancePtr,
				       XV_HDMITX1_DDC_ADDRESS,
				       2,
				       XV_HDMITX1_DDC_STAT_FLGS_LN01_REG,
				       (u8*)&DdcBuf);

	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Ln3_LTP_req */
	DdcBuf[3] = DdcBuf[1] >> XV_HDMITX1_DDC_STAT_FLGS_LN23_LN3_SHIFT;
	/* Ln2_LTP_req */
	DdcBuf[2] = DdcBuf[1] & XV_HDMITX1_DDC_STAT_FLGS_LN23_LN2_MASK;
	/* Ln1_LTP_req */
	DdcBuf[1] = DdcBuf[0] >> XV_HDMITX1_DDC_STAT_FLGS_LN01_LN1_SHIFT;
	/* Ln0_LTP_req */
	DdcBuf[0] = DdcBuf[0] & XV_HDMITX1_DDC_STAT_FLGS_LN01_LN0_MASK;

	/* 0x0 means passing link training.
	 * If total only 3 lanes are used for FRL,
	 * only the first 3 lanes need to
	 * be compared. The forth lane will be compared
	 * only when 4 lanes are used.
	 */
	if (DdcBuf[0] == 0x0 &&
	    DdcBuf[1] == 0x0 &&
	    DdcBuf[2] == 0x0 &&
	    (InstancePtr->Stream.Frl.Lanes == 3 ||
	     (DdcBuf[3] == 0x0 && InstancePtr->Stream.Frl.Lanes == 4))) {
		InstancePtr->Stream.Frl.TimerCnt = 0;
		InstancePtr->Stream.Frl.TrainingState =
				XV_HDMITX1_FRLSTATE_LTS_P_ARM;
#ifdef DEBUG_TX_FRL_VERBOSITY
xil_printf(ANSI_COLOR_CYAN "TX: LTS:3 0x0 (Pass)\r\n" ANSI_COLOR_RESET);
#endif
		/* Set short timer to jump state */
		XV_HdmiTx1_SetFrl10MicroSecondsTimer(InstancePtr);

		/* Check if user callback has been registered*/
		if (InstancePtr->FrlLts3Callback) {
			InstancePtr->DBMessage = 0xF0;
			InstancePtr->FrlLts3Callback(InstancePtr->FrlLts3Ref);
		}
		return Status;
	}
	/* 0xF means a request to drop FRL rate.
	 * If total only 3 lanes are used for FRL,
	 * only the first 3 lanes need to
	 * be compared. The forth lane will be compared
	 * only when 4 lanes are used.
	 */
	else if (DdcBuf[0] == 0xF &&
	         DdcBuf[1] == 0xF &&
	         DdcBuf[2] == 0xF &&
	         (InstancePtr->Stream.Frl.Lanes == 3 ||
		  (DdcBuf[3] == 0xF && InstancePtr->Stream.Frl.Lanes == 4))) {
#ifdef DEBUG_TX_FRL_VERBOSITY
xil_printf(ANSI_COLOR_CYAN "TX: LTS:3 0xF (Drop Rate)\r\n" ANSI_COLOR_RESET);
#endif
		InstancePtr->Stream.Frl.TimerCnt = 0;
		InstancePtr->Stream.Frl.TrainingState = XV_HDMITX1_FRLSTATE_LTS_4;
		/* Set short timer to jump state */
		XV_HdmiTx1_SetFrl10MicroSecondsTimer(InstancePtr);

		/* Check if user callback has been registered*/
		if (InstancePtr->FrlLts3Callback) {
			InstancePtr->DBMessage = 0xFF;
			InstancePtr->FrlLts3Callback(InstancePtr->FrlLts3Ref);
		}

		return Status;
	} else {
#ifdef DEBUG_TX_FRL_VERBOSITY
xil_printf(ANSI_COLOR_CYAN "TX: LTS:3 LTP: ");
#endif
		for (u8 ln = 0; ln < 4; ln++) {
			/* 0x1 to 0x8 means specific link training pattern is
			 * requested. Each of the lane need to be set to output
			 * the link training pattern as requested.
			 */
#ifdef DEBUG_TX_FRL_VERBOSITY
xil_printf("%X ", DdcBuf[ln]);
#endif
			if (DdcBuf[ln] >= 1 && DdcBuf[ln] <= 8) {
				/* Sink requested for other LTP */
				if (ln == 2 &&
				    InstancePtr->Stream.Frl.DBSendWrongLTP ==
								(TRUE)) {
					XV_HdmiTx1_SetFrlLtp(InstancePtr, 2,
						XV_HDMITX1_LTP_NYQUIST_CLOCK);
				} else {
					/*
					 * When FLT no timeout is cleared/False
					 * and LTP request is 3 then don't
					 * update the new LTP and continue with
					 * previous. Refer Table 6-32 LTP3 row.
					 */
					if (DdcBuf[ln] != 3 ||
					    InstancePtr->Stream.Frl.FltNoTimeout)
						XV_HdmiTx1_SetFrlLtp(InstancePtr,
								     ln,
								     DdcBuf[ln]);
				}
			}
			/* 0xE means FFE of the specific lane needs to be adjusted.
			 */
			else if (DdcBuf[ln] == 0xE) {
				FfeAdjFlag = TRUE;
				/* Sink requested for another FFE for the current lane.
				 * Inform the application what TxFFE level to be set.
				 */
				if (InstancePtr->Stream.Frl.LaneFfeAdjReq.Byte[ln] >=
				    InstancePtr->Stream.Frl.FfeLevels) {
					InstancePtr->Stream.Frl.LaneFfeAdjReq.Byte[ln] = 0;
				} else {
					InstancePtr->Stream.Frl.LaneFfeAdjReq.Byte[ln]++;
				}
			}

			/* Check if user callback has been registered*/
			if (InstancePtr->FrlLts3Callback) {
				InstancePtr->DBMessage = DdcBuf[ln];
				InstancePtr->FrlLts3Callback(InstancePtr->FrlLts3Ref);
			}
		}
#ifdef DEBUG_TX_FRL_VERBOSITY
xil_printf("\r\n" ANSI_COLOR_RESET);
#endif

		/* Execute FRL register update*/
		XV_HdmiTx1_FrlExecute(InstancePtr);

		if (InstancePtr->FrlFfeCallback && FfeAdjFlag == TRUE) {
			InstancePtr->FrlFfeCallback(InstancePtr->FrlFfeRef);
		}
	}

	/* Clears FLT_update */
	/* Note : We need to be careful of this implementation as it doesnot
	 * ensures that this will get sent out within 10 ms after
	 * FLT_update == 1. This might timeout with just 2ms */
	Status = XV_HdmiTx1_DdcWriteField(InstancePtr,
					  XV_HDMITX1_SCDCFIELD_FLT_UPDATE,
					  1);

	return Status;
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State 4.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiTx1_ExecFrlState_Lts4(XV_HdmiTx1 *InstancePtr)
{
	int Status = XST_FAILURE;

	/* Disable TimerCnt */
	XV_HdmiTx1_SetFrlTimer(InstancePtr, 0);

	/* Stops transmitting link training patterns */
	XV_HdmiTx1_ClearFrlLtp(InstancePtr);

	if (InstancePtr->Stream.Frl.RateLock == FALSE) {
		if (InstancePtr->Stream.Frl.FrlRate > 1) {
			InstancePtr->Stream.Frl.FrlRate--;
			InstancePtr->Stream.Frl.LineRate = FrlRateTable[InstancePtr->Stream.Frl.FrlRate].LineRate;
			InstancePtr->Stream.Frl.Lanes = FrlRateTable[InstancePtr->Stream.Frl.FrlRate].Lanes;
			Status = XST_SUCCESS;
		} else {
			Status = XST_FAILURE;
		}
	} else {
		xil_printf("RateLock != FALSE\r\n");
		Status = XST_SUCCESS;
	}

#ifdef DEBUG_TX_FRL_VERBOSITY
	xil_printf(ANSI_COLOR_CYAN "TX: LTS:4 Ln %d Rate "
			"%d\r\n" ANSI_COLOR_RESET,
			InstancePtr->Stream.Frl.Lanes,
			InstancePtr->Stream.Frl.LineRate);
#endif
	if (Status == XST_SUCCESS) {
		/* Check if user callback has been registered*/
		if (InstancePtr->FrlLts4Callback) {
			InstancePtr->FrlLts4Callback(InstancePtr->FrlLts4Ref);
		}

		/* Reset LaneFfeAdjReq so application can reset TxFFE */
		InstancePtr->Stream.Frl.LaneFfeAdjReq.Data = 0;

		if (InstancePtr->FrlFfeCallback) {
			InstancePtr->FrlFfeCallback(InstancePtr->FrlFfeRef);
		}

		/* Clears FLT_update */
		Status = XV_HdmiTx1_DdcWriteField(InstancePtr,
						XV_HDMITX1_SCDCFIELD_FLT_UPDATE,
						1);

		if (Status == XST_SUCCESS) {
			InstancePtr->Stream.Frl.TimerCnt = 0;

			InstancePtr->Stream.Frl.TrainingState =
					XV_HDMITX1_FRLSTATE_LTS_3_ARM;

			/* Check if user callback has been registered*/
			if (InstancePtr->FrlConfigCallback) {
				InstancePtr->FrlConfigCallback(
						InstancePtr->FrlConfigRef);
			}
		}
	} else {
		InstancePtr->Stream.Frl.TimerCnt = 0;
		InstancePtr->Stream.Frl.TrainingState = XV_HDMITX1_FRLSTATE_LTS_L;
		/* Set short timer to jump state */
		XV_HdmiTx1_SetFrl10MicroSecondsTimer(InstancePtr);
	}

	XV_HdmiTx1_FrlExecute(InstancePtr);

	return Status;
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State P Arm.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiTx1_ExecFrlState_LtsP_Arm(XV_HdmiTx1 *InstancePtr)
{
	int Status = XST_FAILURE;

	/* Stops transmitting link training patterns */
	XV_HdmiTx1_ClearFrlLtp(InstancePtr);

	/* Send out GAP characters */
	XV_HdmiTx1_SetFrlActive(InstancePtr, XV_HDMITX1_FRL_ACTIVE_MODE_GAP_ONLY);

	/* Clear FLT_update */
	Status = XV_HdmiTx1_DdcWriteField(InstancePtr,
					  XV_HDMITX1_SCDCFIELD_FLT_UPDATE,
					  1);

	InstancePtr->Stream.Frl.TrainingState = XV_HDMITX1_FRLSTATE_LTS_P;

	return Status;
}

/*****************************************************************************/
/**
*
* This function executes the FRL Training State P.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
static int XV_HdmiTx1_ExecFrlState_LtsP(XV_HdmiTx1 *InstancePtr)
{
	int Status = XST_FAILURE;
	u8 DdcBuf = 0;

	if (InstancePtr->Stream.Frl.TrainingState !=
	    XV_HDMITX1_FRLSTATE_LTS_P_FRL_RDY) {
		XV_HdmiTx1_SetFrlTimer(InstancePtr, Timer2MS);
	} else {
		XV_HdmiTx1_SetFrlTimer(InstancePtr, Timer250MS);
	}

	Status = XV_HdmiTx1_DdcReadReg(InstancePtr,
				       XV_HDMITX1_DDC_ADDRESS,
				       1,
				       XV_HDMITX1_DDC_UPDATE_FLGS_REG,
				       (u8*)&DdcBuf);

	if (Status != XST_SUCCESS) {
#ifdef DEBUG_TX_FRL_VERBOSITY
		xil_printf(ANSI_COLOR_RED "LTS:P XV_HdmiTx1_DdcReadReg "
				"Fail\r\n" ANSI_COLOR_RESET);
#endif
		if (InstancePtr->FrlLtsPCallback) {
			InstancePtr->DBMessage = 2;
			InstancePtr->FrlLtsPCallback(InstancePtr->FrlLtsPRef);
		}
		return Status;
	}

	if (InstancePtr->Stream.Frl.TrainingState ==
	    XV_HDMITX1_FRLSTATE_LTS_P) {
		if (DdcBuf & XV_HDMITX1_DDC_UPDATE_FLGS_FRL_START_MASK) {
			/* Sink has now indicated that video can be sent out now */
			XV_HdmiTx1_SetFrlTimer(InstancePtr, Timer250MS);

			if (InstancePtr->FrlStartCallback) {
				InstancePtr->FrlStartCallback(InstancePtr->FrlStartRef);
			}

			/* Clear FLT_start */
			Status = XV_HdmiTx1_DdcWriteField(InstancePtr,
						XV_HDMITX1_SCDCFIELD_FRL_START,
						1);

			InstancePtr->Stream.Frl.TrainingState =
					XV_HDMITX1_FRLSTATE_LTS_P_FRL_RDY;

			/* Check if user callback has been registered*/
			if (InstancePtr->FrlLtsPCallback) {
				InstancePtr->DBMessage = 0;
				InstancePtr->FrlLtsPCallback(InstancePtr->FrlLtsPRef);
			}
		}
	}

	if (DdcBuf & XV_HDMITX1_DDC_UPDATE_FLGS_FLT_UPDATE_MASK) {
		if (InstancePtr->FrlLtsPCallback) {
			InstancePtr->DBMessage = 1;
			InstancePtr->FrlLtsPCallback(InstancePtr->FrlLtsPRef);
		}

		/* Stops transmitting link training pattern */
		XV_HdmiTx1_ClearFrlLtp(InstancePtr);

		/* Stops transmitting video, audio and control packets */
		XV_HdmiTx1_SetFrlActive(InstancePtr,
					XV_HDMITX1_FRL_ACTIVE_MODE_GAP_ONLY);

		if (InstancePtr->FrlStopCallback) {
			InstancePtr->FrlStopCallback(InstancePtr->FrlStopRef);
		}

		InstancePtr->Stream.Frl.TimerCnt = 0;
		InstancePtr->Stream.Frl.TrainingState = XV_HDMITX1_FRLSTATE_LTS_3;

		/* Set short timer to jump state */
		XV_HdmiTx1_SetFrl10MicroSecondsTimer(InstancePtr);
	} else if (DdcBuf & XV_HDMITX1_DDC_UPDATE_FLGS_CED_UPDATE_MASK) {
#ifdef DEBUG_TX_FRL_VERBOSITY
		xil_printf(ANSI_COLOR_RED "CED Update Flag!\r\n" ANSI_COLOR_RESET);
#endif
		if (InstancePtr->CedUpdateCallback) {
			InstancePtr->CedUpdateCallback(InstancePtr->CedUpdateRef);
		}
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function executes the different of states of FRL.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_ExecFrlState(XV_HdmiTx1 *InstancePtr)
{
	int Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);

	switch (InstancePtr->Stream.Frl.TrainingState) {

		case XV_HDMITX1_FRLSTATE_LTS_L:
			Status = XV_HdmiTx1_ExecFrlState_LtsL(InstancePtr);
			Status = XST_SUCCESS;
			break;

		case XV_HDMITX1_FRLSTATE_LTS_1:
			Status = XV_HdmiTx1_ExecFrlState_Lts1(InstancePtr);
			break;

		case XV_HDMITX1_FRLSTATE_LTS_2:
			Status = XV_HdmiTx1_ExecFrlState_Lts2(InstancePtr);
			break;

		case XV_HDMITX1_FRLSTATE_LTS_3_ARM:
			Status = XV_HdmiTx1_ExecFrlState_Lts2_RateWr(InstancePtr);
			Status = XST_SUCCESS;
			break;

		case XV_HDMITX1_FRLSTATE_LTS_3:
			Status = XV_HdmiTx1_ExecFrlState_Lts3(InstancePtr);
			break;

		case XV_HDMITX1_FRLSTATE_LTS_4:
			Status = XV_HdmiTx1_ExecFrlState_Lts4(InstancePtr);
			break;

		case XV_HDMITX1_FRLSTATE_LTS_P_ARM:
			Status = XV_HdmiTx1_ExecFrlState_LtsP_Arm(InstancePtr);
#ifdef DEBUG_TX_FRL_VERBOSITY
			xil_printf(ANSI_COLOR_CYAN"TX: LTS:"
				   "P_Arm\r\n" ANSI_COLOR_RESET);
#endif
			if (Status == XST_SUCCESS) {
				Status = XV_HdmiTx1_ExecFrlState_LtsP(InstancePtr);
#ifdef DEBUG_TX_FRL_VERBOSITY
				xil_printf(ANSI_COLOR_GREEN "TX: "
					   "LTS:P\r\n" ANSI_COLOR_RESET);
#endif
			}
			break;

		case XV_HDMITX1_FRLSTATE_LTS_P:
			Status = XV_HdmiTx1_ExecFrlState_LtsP(InstancePtr);
			break;

		case XV_HDMITX1_FRLSTATE_LTS_P_FRL_RDY:
			Status = XV_HdmiTx1_ExecFrlState_LtsP(InstancePtr);
			break;

		default:
			xil_printf("TX:S:FRL_INVALID_STATE!\r\n");
			break;
		}

	if (Status == XST_FAILURE) {
/*		xil_printf("TX:S:FAILURE!\r\n");*/
	}

	/* Clear timer event flag */
	InstancePtr->Stream.Frl.TimerEvent = FALSE;

	return Status;
}

/*****************************************************************************/
/**
*
* This function starts FRL video stream. This should be called after the bridge,
* video, audio are all active.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_FrlStreamStart(XV_HdmiTx1 *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#ifdef DEBUG_TX_FRL_VERBOSITY
	xil_printf(ANSI_COLOR_GREEN "TX: FRL stream start\n\r" ANSI_COLOR_RESET);
#endif
	int Status = XST_FAILURE;

	/* Enable the AUX peripheral */
	XV_HdmiTx1_AuxEnable(InstancePtr);

	/* Enable the AUX peripheral interrupt */
	XV_HdmiTx1_AuxIntrEnable(InstancePtr);

	/* FRL transmission includes video, audio and control packets */
	XV_HdmiTx1_SetFrlActive(InstancePtr,
				XV_HDMITX1_FRL_ACTIVE_MODE_FULL_STREAM);

/*	/\* Clears FRL_Start *\/*/
/*	Status = XV_HdmiTx1_DdcWriteField(InstancePtr,*/
/*					  XV_HDMITX1_SCDCFIELD_FRL_START, 1);*/

	/* Set stream status to up */
	InstancePtr->Stream.State = XV_HDMITX1_STATE_STREAM_UP;

	XV_HdmiTx1_DynHdr_DM_Enable(InstancePtr);

	/* Jump to steam up call back */
	if (InstancePtr->StreamUpCallback) {
		InstancePtr->StreamUpCallback(InstancePtr->StreamUpRef);
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function stops FRL video stream. This should be called after the bridge,
* video, audio are all active.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_FrlStreamStop(XV_HdmiTx1 *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#ifdef DEBUG_TX_FRL_VERBOSITY
	xil_printf(ANSI_COLOR_GREEN "TX: FRL stream stop\n\r" ANSI_COLOR_RESET);
#endif
	int Status = XST_FAILURE;

	/* Disable the AUX peripheral */
	XV_HdmiTx1_AuxDisable(InstancePtr);

	/* Disable the AUX peripheral interrupt */
	XV_HdmiTx1_AuxIntrDisable(InstancePtr);

/*	/\* FRL transmission includes video, audio and control packets *\/ */
/*	XV_HdmiTx1_SetFrlActive(InstancePtr,*/
/*				XV_HDMITX1_FRL_ACTIVE_MODE_GAP_ONLY);*/

/*	/\* Clear FLT_start *\/ */
/*	Status = XV_HdmiTx1_DdcWriteField(InstancePtr,*/
/*					  XV_HDMITX1_SCDCFIELD_FRL_START, 1);*/

	/* Set stream status to up */
	InstancePtr->Stream.State = XV_HDMITX1_STATE_STREAM_DOWN;

	XV_HdmiTx1_DynHdr_DM_Disable(InstancePtr);

	/* Jump to steam up call back */
	if (InstancePtr->StreamDownCallback) {
		InstancePtr->StreamDownCallback(InstancePtr->StreamDownRef);
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function sets the link training pattern for the selected lane.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @param    Lane specifies the lane of which the Link Training Pattern will be
* 			set.
*
* @param    Ltp specifies Link Training Pattern
* 			- 0 = No LTP
*			- 1 = LTP1 / ALL 1's pattern
*			- 2 = LTP2 / All 0's pattern
*			- 3 = LTP3 / Nyquist clock pattern
*			- 4 = LTP4 / Source TxDDE compliance test pattern
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
void XV_HdmiTx1_SetFrlLtp(XV_HdmiTx1 *InstancePtr, u8 Lane,
						XV_HdmiTx1_FrlLtpType Ltp)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u32 value = Ltp;
	u32 Data;

	Data = XV_HdmiTx1_ReadReg((InstancePtr)->Config.BaseAddress,
    				  XV_HDMITX1_FRL_CTRL_OFFSET);

	switch (Lane) {
	case 0:
		Data = Data & ~((u32)(XV_HDMITX1_FRL_LTP0_REQ_MASK <<
				      XV_HDMITX1_FRL_LTP0_REQ_SHIFT));
		Data = Data | ((value & XV_HDMITX1_FRL_LTP0_REQ_MASK) <<
				XV_HDMITX1_FRL_LTP0_REQ_SHIFT);
		break;

	case 1:
		Data = Data & ~((u32)(XV_HDMITX1_FRL_LTP0_REQ_MASK <<
				      XV_HDMITX1_FRL_LTP1_REQ_SHIFT));
		Data = Data | ((value & XV_HDMITX1_FRL_LTP0_REQ_MASK) <<
				XV_HDMITX1_FRL_LTP1_REQ_SHIFT);
		break;

	case 2:
		Data = Data & ~((u32)(XV_HDMITX1_FRL_LTP0_REQ_MASK <<
				      XV_HDMITX1_FRL_LTP2_REQ_SHIFT));
		Data = Data | ((value & XV_HDMITX1_FRL_LTP0_REQ_MASK) <<
				XV_HDMITX1_FRL_LTP2_REQ_SHIFT);
		break;

	case 3:
		Data = Data & ~((u32)(XV_HDMITX1_FRL_LTP0_REQ_MASK <<
				      XV_HDMITX1_FRL_LTP3_REQ_SHIFT));
		Data = Data | ((value & XV_HDMITX1_FRL_LTP0_REQ_MASK) <<
				XV_HDMITX1_FRL_LTP3_REQ_SHIFT);
		break;

	default:
		xil_printf("TX:ERROR, Wrong lane is selected!\r\n");
		break;
	}

	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
	    		(XV_HDMITX1_FRL_CTRL_OFFSET), (Data));
}

/*****************************************************************************/
/**
*
* This function sets active FRL mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @param    Mode specifies the active FRL mode.
* 			- 0 = FRL transmission only includes GAP characters
*			- 1 = FRL transmission includes video, audio and control packets
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_SetFrlActive(XV_HdmiTx1 *InstancePtr,
			     XV_HdmiTx1_FrlActiveMode Mode)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Mode == XV_HDMITX1_FRL_ACTIVE_MODE_GAP_ONLY ||
		       Mode == XV_HDMITX1_FRL_ACTIVE_MODE_FULL_STREAM);

	if (Mode) {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    XV_HDMITX1_FRL_CTRL_SET_OFFSET,
				    XV_HDMITX1_FRL_ACT_MASK);
	} else {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    XV_HDMITX1_FRL_CTRL_CLR_OFFSET,
				    XV_HDMITX1_FRL_ACT_MASK);
	}
}

/*****************************************************************************/
/**
*
* This function sets the number of FRL lane in operations.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @param    Lanes specifies the number of FRL lanes in operation.
* 			- 3 = 3 FRL lanes
*			- 4 = 4 FRL lanes
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_SetFrlLanes(XV_HdmiTx1 *InstancePtr, u8 Lanes)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Lanes == 3 || Lanes == 4);

	if (Lanes == 4) {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    XV_HDMITX1_FRL_CTRL_SET_OFFSET,
				    XV_HDMITX1_FRL_CTRL_LN_OP_MASK);
	} else if (Lanes == 3) {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    XV_HDMITX1_FRL_CTRL_CLR_OFFSET,
				    XV_HDMITX1_FRL_CTRL_LN_OP_MASK);
	}
}

/*****************************************************************************/
/**
*
* This function sets the FRL operation mode
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @param    Enable specifies the FRL mode.
* 			- FALSE = TMDS
*			- TRUE  = FRL
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_FrlModeEn(XV_HdmiTx1 *InstancePtr, u8 Enable)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Enable == 0 || Enable == 1);

	if (Enable == (TRUE)) {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    XV_HDMITX1_FRL_CTRL_SET_OFFSET,
				    XV_HDMITX1_FRL_CTRL_OP_MODE_MASK);
	} else {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    XV_HDMITX1_FRL_CTRL_CLR_OFFSET,
				    XV_HDMITX1_FRL_CTRL_OP_MODE_MASK);
	}
}

/*****************************************************************************/
/**
*
* This function resets the FRL peripheral
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
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
void XV_HdmiTx1_FrlReset(XV_HdmiTx1 *InstancePtr, u8 Reset)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Reset == 0 || Reset == 1);

	if (Reset) {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_FRL_CTRL_CLR_OFFSET),
				    (XV_HDMITX1_FRL_CTRL_RSTN_MASK));
	} else {
    	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_FRL_CTRL_SET_OFFSET),
			    (XV_HDMITX1_FRL_CTRL_RSTN_MASK));
	}
}

/*****************************************************************************/
/**
*
* This function sets the TX core's FRL Rate and sends encoded FRL_Rate data and
* FFE Levels to the sink through SCDC.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @param    MaxFrlRate specifies maximum rates supported
* 			- 0 = FRL Not Supported
* 			- 1 = 3 Lanes 3Gbps
* 			- 2 = 4 Lanes 3Gbps
*			- 3 = 4 Lanes 6Gbsp
*			- 4 = 4 Lanes 8Gbps
*			- 5 = 4 Lanes 10Gbps
*			- 6 = 4 Lanes 12Gbps
*
* @return
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_FrlRate(XV_HdmiTx1 *InstancePtr, u8 FrlRate)
{
	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status = XST_SUCCESS;

	InstancePtr->Stream.Frl.FrlRate = FrlRate;
	InstancePtr->Stream.Frl.Lanes = FrlRateTable[FrlRate].Lanes;
	InstancePtr->Stream.Frl.LineRate = FrlRateTable[FrlRate].LineRate;

	XV_HdmiTx1_SetFrlLanes(InstancePtr, FrlRateTable[FrlRate].Lanes);

	if (Status == XST_SUCCESS && InstancePtr->Stream.ScdcSupport == TRUE) {
		Status = XV_HdmiTx1_DdcWriteField(InstancePtr,
						  XV_HDMITX1_SCDCFIELD_SNK_CFG1,
						  FrlRate |
						  (InstancePtr->Stream.Frl.FfeLevels <<
						   XV_HDMITX1_DDC_CFG_1_FFE_LVLS_SHIFT));
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function sets the source of the video clock enable.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @param    Value specifies the number of FRL lanes in operation.
* 			- FALSE = Internally generated video cke
*			- TRUE = External video cke
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_FrlExtVidCkeSource(XV_HdmiTx1 *InstancePtr, u8 Value)
{
	Xil_AssertVoid(InstancePtr != NULL);

	if (Value == TRUE) {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				XV_HDMITX1_FRL_CTRL_SET_OFFSET,
				XV_HDMITX1_FRL_VCKE_EXT_MASK);
	} else {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				XV_HDMITX1_FRL_CTRL_CLR_OFFSET,
				XV_HDMITX1_FRL_VCKE_EXT_MASK);
	}
}

/*****************************************************************************/
/**
*
* This function executes the FRL register updates
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_FrlExecute(XV_HdmiTx1 *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				XV_HDMITX1_FRL_CTRL_SET_OFFSET,
				XV_HDMITX1_FRL_CTRL_EXEC_MASK);
}


/*****************************************************************************/
/**
*
* This function initializes FRL peripheral and sink's SCDC for FRL Training.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_FrlTrainingInit(XV_HdmiTx1 *InstancePtr)
{
	int Status = XST_FAILURE;

	/* Initializes the LTP to No LTP for all the lanes */
	XV_HdmiTx1_ClearFrlLtp(InstancePtr);

	/* Initialize the FRL module to send out GAP characters only for
	 * link training */
	XV_HdmiTx1_SetFrlActive(InstancePtr, XV_HDMITX1_FRL_ACTIVE_MODE_GAP_ONLY);

	/* Initialize the TX core to operate in FRL mode */
	XV_HdmiTx1_FrlModeEn(InstancePtr, TRUE);

	Status = XV_HdmiTx1_FrlRate(InstancePtr,
				    InstancePtr->Stream.Frl.FrlRate);

	if (Status == XST_SUCCESS) {
		/* Declare no support of Read Request and clearing of FLT_no_retrain */
		Status = XV_HdmiTx1_DdcWriteField(InstancePtr,
						  XV_HDMITX1_SCDCFIELD_SNK_CFG0,
						  0);
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function sets the timer of TX Core's FRL peripheral.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
*
* @param	Value specifies the timer's frequency (in milliseconds)
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiTx1_SetFrlTimer(XV_HdmiTx1 *InstancePtr, u32 Milliseconds)
{
	u32 ClockCycles;

	if (Milliseconds > 0) {
		ClockCycles = InstancePtr->Config.AxiLiteClkFreq /
				(1000 / Milliseconds);
	} else {
		ClockCycles = 0;
	}
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			(XV_HDMITX1_FRL_TMR_OFFSET),
			(ClockCycles));
}

/*****************************************************************************/
/**
*
* This function returns the remaining value of the timer of TX Core's FRL
* peripheral.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
*
* @return	Remaining value of the timer in clock cycles.
*
* @note		None.
*
******************************************************************************/
u32 XV_HdmiTx1_GetFrlTimer(XV_HdmiTx1 *InstancePtr)
{
	return XV_HdmiTx1_ReadReg((InstancePtr)->Config.BaseAddress,
			(XV_HDMITX1_FRL_TMR_OFFSET));
}

/*****************************************************************************/
/**
*
* This function sets the timer of TX Core's FRL peripheral in clock cycles.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
*
* @param	ClockCycles specifies the timer's frequency (in clock cycles)
*
* @return	Remaining value of the timer in clock cycles.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiTx1_SetFrlTimerClockCycles(XV_HdmiTx1 *InstancePtr,
		u32 ClockCycles)
{
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			(XV_HDMITX1_FRL_TMR_OFFSET),
			(ClockCycles));
}

/*****************************************************************************/
/**
*
* This function sets the timer of TX Core's FRL peripheral for 10 Microseconds.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiTx1_SetFrl10MicroSecondsTimer(XV_HdmiTx1 *InstancePtr)
{
	u32 ClockCycles;

	ClockCycles = InstancePtr->Config.AxiLiteClkFreq / 100000;

	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_FRL_TMR_OFFSET),
			    (ClockCycles));
}

/*****************************************************************************/
/**
*
* This function sets the core to send out wrong LTP on one of the channel to
* prevent link training from passing.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiTx1_SetFrlWrongLtp(XV_HdmiTx1 *InstancePtr)
{
	InstancePtr->Stream.Frl.DBSendWrongLTP = (TRUE);
}

/*****************************************************************************/
/**
*
* This function clears the debugging flag which would have prevented the core
* from sending out correct LTP.
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiTx1_ClearFrlWrongLtp(XV_HdmiTx1 *InstancePtr)
{
	InstancePtr->Stream.Frl.DBSendWrongLTP = (FALSE);
}
