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

#include "xv_hdmitx1.h"
#include "xparameters.h"
#include "string.h"

/************************** Constant Definitions *****************************/

/*****************************************************************************/
/**
* This table contains the attributes for SCDC fields
* Each entry consists of:
* 1) Register Offset
* 2) Bits Mask
* 3) Bits Shift
*/
const XV_HdmiTx1_ScdcField ScdcField[XV_HDMITX1_SCDCFIELD_SIZE] = {
	{0x02, 0xFF, 0},	/* XV_HDMITX1_SCDCFIELD_SOURCE_VER */
	{0x30, 0xFF, 0},	/* XV_HDMITX1_SCDCFIELD_SNK_CFG0 */
	{0x31, 0xFF, 0},	/* XV_HDMITX1_SCDCFIELD_SNK_CFG1 */
	{0x10, 0x01, 3},	/* XV_HDMITX1_SCDCFIELD_SNK_STU */
	{0x10, 0xFF, 1},	/* XV_HDMITX1_SCDCFIELD_CED_UPDATE */
	{0x10, 0xFF, 4},	/* XV_HDMITX1_SCDCFIELD_FRL_START */
	{0x10, 0xFF, 5},	/* XV_HDMITX1_SCDCFIELD_FLT_UPDATE */
	{0x30, 0x01, 1}		/* XV_HDMITX1_SCDCFIELD_FLT_NO_RETRAIN */
};

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static void StubCallback(void *Callback);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes the HDMI TX core. This function must be called
* prior to using the HDMI TX core. Initialization of the HDMI TX includes
* setting up the instance data and ensuring the hardware is in a quiescent
* state.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param    CfgPtr points to the configuration structure associated with
*       the HDMI TX core.
* @param    EffectiveAddr is the base address of the device. If address
*       translation is being used, then this parameter must reflect the
*       virtual base address. Otherwise, the physical address should be
*       used.
*
* @return
*       - XST_SUCCESS if XV_HdmiTx1_CfgInitialize was successful.
*       - XST_FAILURE if HDMI TX PIO ID mismatched.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_CfgInitialize(XV_HdmiTx1 *InstancePtr, XV_HdmiTx1_Config *CfgPtr,
	UINTPTR EffectiveAddr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0x0);

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XV_HdmiTx1));
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
		sizeof(XV_HdmiTx1_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Set all handlers to stub values, let user configure this data later */
	InstancePtr->ConnectCallback = (XV_HdmiTx1_Callback)((void *)StubCallback);

	InstancePtr->ToggleCallback = (XV_HdmiTx1_Callback)((void *)StubCallback);

	InstancePtr->VsCallback = (XV_HdmiTx1_Callback)((void *)StubCallback);

	InstancePtr->StreamDownCallback =(XV_HdmiTx1_Callback)((void *)StubCallback);

	InstancePtr->StreamUpCallback = (XV_HdmiTx1_Callback)((void *)StubCallback);

	InstancePtr->FrlConfigCallback = (XV_HdmiTx1_Callback)((void *)StubCallback);

	InstancePtr->FrlFfeCallback = (XV_HdmiTx1_Callback)((void *)StubCallback);

	InstancePtr->FrlStartCallback = (XV_HdmiTx1_Callback)((void *)StubCallback);

	InstancePtr->FrlStopCallback = (XV_HdmiTx1_Callback)((void *)StubCallback);

	InstancePtr->TmdsConfigCallback = (XV_HdmiTx1_Callback)((void *)StubCallback);

	InstancePtr->DynHdrMtwCallback = (XV_HdmiTx1_Callback)((void *)StubCallback);

	/* Maximum FRL Rate Supported */
	InstancePtr->Stream.Frl.MaxFrlRate =  InstancePtr->Config.MaxFrlRate;

	/* Clear HDMI variables */
	XV_HdmiTx1_Clear(InstancePtr);

	/* core always runs at 4 ppc */
	InstancePtr->Stream.CorePixPerClk = XVIDC_PPC_4;

	/* Disable scrambler override function */
	InstancePtr->Stream.OverrideScrambler = (FALSE);

	/* Set stream status - The stream is down*/
	InstancePtr->Stream.State = XV_HDMITX1_STATE_STREAM_DOWN;

	/* Set FRL State*/
	InstancePtr->Stream.Frl.TrainingState = XV_HDMITX1_FRLSTATE_LTS_L;
	InstancePtr->Stream.IsHdmi = FALSE;
	InstancePtr->Stream.IsFrl = FALSE;

	/* Clear connected flag*/
	InstancePtr->Stream.IsConnected = (FALSE);

	/* Set Default ACR to use internal Audio Sample Frequency to 32kHz */
	InstancePtr->CTS_N_Source = XV_HDMITX1_INTERNAL_CTS_N;
	InstancePtr->Stream.Audio.SampleFrequency = XHDMIC_SAMPLING_FREQ_32K;
	InstancePtr->Stream.Audio.SFreq = XHDMIC_SAMPLING_FREQUENCY_32K;

	/* Reset all peripherals */
	XV_HdmiTx1_PioDisable(InstancePtr);
	XV_HdmiTx1_DdcDisable(InstancePtr);
	XV_HdmiTx1_AudioDisable(InstancePtr);
	XV_HdmiTx1_AuxDisable(InstancePtr);
	XV_HdmiTx1_FrlIntrDisable(InstancePtr);
	XV_HdmiTx1_FrlReset(InstancePtr, TRUE);
	XV_HdmiTx1_FrlExtVidCkeSource(InstancePtr, FALSE);

	XV_HdmiTx1_PioIntrClear(InstancePtr);
	XV_HdmiTx1_DdcIntrClear(InstancePtr);

	/* Read PIO peripheral Identification register */
	RegValue = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
	(XV_HDMITX1_PIO_ID_OFFSET));

	RegValue = ((RegValue) >> (XV_HDMITX1_SHIFT_16)) &
	(XV_HDMITX1_MASK_16);
	if (RegValue != (XV_HDMITX1_PIO_ID)) {
		return (XST_FAILURE);
	}

	/* PIO: Set event rising edge masks */
	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMITX1_PIO_IN_EVT_RE_OFFSET),
			    (XV_HDMITX1_PIO_IN_BRDG_UNDERFLOW_MASK) |
			    (XV_HDMITX1_PIO_IN_BRDG_OVERFLOW_MASK) |
			    (XV_HDMITX1_PIO_IN_BRDG_LOCKED_MASK) |
			    (XV_HDMITX1_PIO_IN_HPD_TOGGLE_MASK) |
			    (XV_HDMITX1_PIO_IN_HPD_MASK) |
			    (XV_HDMITX1_PIO_IN_VS_MASK) |
			    (XV_HDMITX1_PIO_IN_LNK_RDY_MASK));

	/* PIO: Set event falling edge masks */
	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMITX1_PIO_IN_EVT_FE_OFFSET),
			    (XV_HDMITX1_PIO_IN_BRDG_LOCKED_MASK) |
			    (XV_HDMITX1_PIO_IN_HPD_MASK) |
			    (XV_HDMITX1_PIO_IN_LNK_RDY_MASK));

	/* Set the Timegrid for HPD Pulse for Connect and Toggle Event */
	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
		            XV_HDMITX1_HPD_TIMEGRID_OFFSET,
		            XV_HdmiTx1_GetTime1Ms(InstancePtr));

	/* Toggle HPD Pulse (50ms - 99ms)*/
	RegValue = ((99 << XV_HDMITX1_SHIFT_16) | /* 99 ms on Bit 31:16 */
	            (50)); /* 50 ms on Bit 15:0 */

	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			    XV_HDMITX1_TOGGLE_CONF_OFFSET,
			    RegValue);

	/* HPD/Connect Trigger (100ms + 0ms)*/
	RegValue = ((10 << XV_HDMITX1_SHIFT_16) | /* 10 ms on Bit 31:16 */
				(100)); /* 100 ms on Bit 15:0 */

	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			   XV_HDMITX1_CONNECT_CONF_OFFSET,
			   RegValue);

	/* Set HDMI mode */
	XV_HdmiTx1_SetHdmiTmdsMode(InstancePtr);

	/* Enable the AUX peripheral */
	/* The aux peripheral is enabled at stream up */
	/*XV_HdmiTx1_AuxEnable(InstancePtr);*/

	/* Enable audio */
	/* The audio peripheral is enabled at stream up */
	/*XV_HdmiTx1_AudioEnable(InstancePtr);*/

	/* Enable FRL peripheral */
	XV_HdmiTx1_FrlReset(InstancePtr, FALSE);

	/* Release Audio Reset */
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_CLR_OFFSET),
			    (XV_HDMITX1_AUD_CTRL_AUDRST_MASK));
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_SET_OFFSET),
			    (XV_HDMITX1_AUD_CTRL_AUDRST_MASK));

	/* Enable FRL Interrupt */
	XV_HdmiTx1_FrlIntrEnable(InstancePtr);
	xil_printf("TX: FRL Base: 0x%X\r\n",
		   (InstancePtr)->Config.BaseAddress + XV_HDMITX1_FRL_BASE);

	InstancePtr->Stream.Frl.Lanes = 3;
	InstancePtr->Stream.Frl.RateLock = FALSE;

	XV_HdmiTx1_FrlExecute(InstancePtr);

	/* Reset the hardware and set the flag to indicate the driver is ready */
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function sets the AXI4-Lite Clock Frequency
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param    ClkFreq specifies the value that needs to be set.
*
* @return
*
*
* @note     This is required after a reset or init.
*
******************************************************************************/
void XV_HdmiTx1_SetAxiClkFreq(XV_HdmiTx1 *InstancePtr, u32 ClkFreq)
{
	InstancePtr->Config.AxiLiteClkFreq = ClkFreq;
	InstancePtr->CpuClkFreq = ClkFreq;

	/* Initialize DDC */
	XV_HdmiTx1_DdcInit(InstancePtr, InstancePtr->CpuClkFreq);
}

/*****************************************************************************/
/**
*
* This function sets the core into HDMI FRL mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
*
* @note     This is required after a reset or init.
*
******************************************************************************/
void XV_HdmiTx1_SetHdmiFrlMode(XV_HdmiTx1 *InstancePtr)
{

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Set mode bit in core */
	XV_HdmiTx1_SetMode(InstancePtr);

	/* Set flag in structure */
	InstancePtr->Stream.IsFrl = TRUE;
	InstancePtr->Stream.IsHdmi = TRUE;

	/* Enable the AUX peripheral */
	/* The aux peripheral is enabled at stream up */
	XV_HdmiTx1_AuxEnable(InstancePtr);

	/* Enable audio */
	/* The audio peripheral is enabled at stream up */
	XV_HdmiTx1_AudioEnable(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function sets the core into HDMI TMDS mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
*
* @note     This is required after a reset or init.
*
******************************************************************************/
void XV_HdmiTx1_SetHdmiTmdsMode(XV_HdmiTx1 *InstancePtr)
{

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Set mode bit in core */
	XV_HdmiTx1_SetMode(InstancePtr);

	/* Set flag in structure */
	InstancePtr->Stream.IsHdmi = TRUE;
	InstancePtr->Stream.IsFrl = FALSE;

	/* Enable the AUX peripheral */
	/* The aux peripheral is enabled at stream up */
	XV_HdmiTx1_AuxEnable(InstancePtr);

	/* Enable audio */
	/* The audio peripheral is enabled at stream up */
	XV_HdmiTx1_AudioEnable(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function sets the core into DVI mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
*
* @note     This is required after a reset or init.
*
******************************************************************************/
void XV_HdmiTx1_SetDviMode(XV_HdmiTx1 *InstancePtr)
{

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Disable audio peripheral */
	XV_HdmiTx1_AudioDisable(InstancePtr);

	/* Disable aux peripheral */
	XV_HdmiTx1_AuxDisable(InstancePtr);

	/* Clear mode bit in core */
	XV_HdmiTx1_ClearMode(InstancePtr);

	/* Clear flag in structure */
	InstancePtr->Stream.IsHdmi = FALSE;
	InstancePtr->Stream.IsFrl = FALSE;
}

/*****************************************************************************/
/**
*
* This function enables the HDMI TX Auxiliary (AUX) peripheral.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_AuxEnable(XV_HdmiTx1 *InstancePtr)
{
	if ((InstancePtr)->Stream.IsHdmi == TRUE) {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_AUX_CTRL_SET_OFFSET),
				    (XV_HDMITX1_AUX_CTRL_RUN_MASK));
	}
}

/*****************************************************************************/
/**
*
* This macro enables audio in HDMI TX core.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_AudioEnable(XV_HdmiTx1 *InstancePtr)
{

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if  (InstancePtr->Stream.IsFrl == TRUE) {
		/* Start FRL ACR */
		XV_HdmiTx1_FRLACRStart(InstancePtr);
	} else {
		/* Start TMDS ACR */
		XV_HdmiTx1_TMDSACRStart(InstancePtr);
	}

	/* Run the Audio Module */
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
	(XV_HDMITX1_AUD_CTRL_SET_OFFSET), (XV_HDMITX1_AUD_CTRL_RUN_MASK));
}

/*****************************************************************************/
/**
*
* This function clear the HDMI TX variables and sets it to the defaults.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*
*
* @note     This is required after a reset or init.
*
******************************************************************************/
void XV_HdmiTx1_Clear(XV_HdmiTx1 *InstancePtr)
{

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);
}


/*****************************************************************************/
/**
*
* This function starts the HDMI TX core.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     This is required after a reset or initialization.
*
******************************************************************************/
void  XV_HdmiTx1_Start(XV_HdmiTx1 *InstancePtr) {
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Enable the PIO peripheral interrupt */
	XV_HdmiTx1_PioIntrEnable(InstancePtr);

	/* Enable the PIO peripheral */
	XV_HdmiTx1_PioEnable(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function stops the HDMI TX core.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
******************************************************************************/
void  XV_HdmiTx1_Stop(XV_HdmiTx1 *InstancePtr) {
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Disable the PIO peripheral interrupt */
	XV_HdmiTx1_PioIntrDisable(InstancePtr);

	/* Disable the PIO peripheral */
	XV_HdmiTx1_PioDisable(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function provides video identification code of video mode.
*
* @param    VideoMode specifies resolution identifier.
*
* @return   Video identification code defined in the VIC table.
*
* @note     None.
*
******************************************************************************/
u8 XV_HdmiTx1_LookupVic(XVidC_VideoMode VideoMode)
{
	XHdmiC_VicTable const *Entry;
	u8 Index;

	for (Index = 0; Index < sizeof(VicTable)/sizeof(XHdmiC_VicTable);
		Index++) {
	  Entry = &VicTable[Index];
	  if (Entry->VmId == VideoMode)
		return (Entry->Vic);
	}
	return 0;
}

/*****************************************************************************/
/**
*
* This function sets and return the TMDS Clock based on Video Parameter from
* the InstancePtr.
*
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*       - TMDS Clock
*
* @note     None.
*
******************************************************************************/
u64 XV_HdmiTx1_GetTmdsClk(XV_HdmiTx1 *InstancePtr)
{
	u64 TmdsClock;

	/* Calculate reference clock. First calculate the pixel clock */
	TmdsClock = InstancePtr->Stream.Video.Timing.F0PVTotal +
			InstancePtr->Stream.Video.Timing.F1VTotal;

	TmdsClock *= InstancePtr->Stream.Video.Timing.HTotal;

	TmdsClock *= InstancePtr->Stream.Video.FrameRate;

	if (InstancePtr->Stream.Video.Timing.F1VTotal != 0) {
		/* Divide by 2 for interlaced video. */
		TmdsClock >>= 1;
	}

	/* Store the pixel clock in the structure */
	InstancePtr->Stream.PixelClk = TmdsClock;

	/* YUV420 */
	if (InstancePtr->Stream.Video.ColorFormatId == (XVIDC_CSF_YCRCB_420)) {
		/* In YUV420 the tmds clock is divided by two*/
		TmdsClock = TmdsClock >> 1;
	}

	/* RGB, YUV444 and YUV420 */
	if ( InstancePtr->Stream.Video.ColorFormatId != XVIDC_CSF_YCRCB_422 ) {

		switch (InstancePtr->Stream.Video.ColorDepth) {

		/* 10-bits*/
		case XVIDC_BPC_10 :
			TmdsClock = (TmdsClock * 5) >> 2;
			break;

		/* 12-bits*/
		case XVIDC_BPC_12 :
			TmdsClock = (TmdsClock * 3) >> 1;
			break;

		/* 16-bits*/
		case XVIDC_BPC_16 :
			TmdsClock = TmdsClock << 1;
			break;

		/* 8-bits*/
		default:
			TmdsClock = TmdsClock;
			break;
		}
	}

	return TmdsClock;
}

/*****************************************************************************/
/**
*
* This function controls the scrambler. Requires TMDSClock to be up to date in
* order to force enable scrambler when TMDSClock > 340MHz.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*       - XST_SUCCESS if HDMI 2.0
*       - XST_FAILURE if HDMI 1.4
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_Scrambler(XV_HdmiTx1 *InstancePtr)
{
	u8 DdcBuf[2];
	u32 Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check if the sink is HDMI 2.0*/
	/* Check if the TMDS Clock is higher than 340MHz*/
	/* Check scrambler flag*/
	if (InstancePtr->Stream.ScdcSupport &&
	    ((InstancePtr->Stream.TMDSClock > 340000000 &&
	      InstancePtr->Stream.OverrideScrambler != (TRUE)) ||
	     InstancePtr->Stream.IsScrambled)) {
		XV_HdmiTx1_SetScrambler(InstancePtr, (TRUE));
	} else {
		/* Clear*/
		XV_HdmiTx1_SetScrambler(InstancePtr, (FALSE));
	}

	/* Update TMDS configuration*/
	/* Only when it is a HDMI 2.0 sink device*/
	if (InstancePtr->Stream.ScdcSupport) {

		DdcBuf[0] = 0x20;   /* Offset scrambler status*/
		Status = XV_HdmiTx1_DdcWrite(InstancePtr,
					     XV_HDMITX1_DDC_ADDRESS,
					     1,
					     (u8*)&DdcBuf,
					     (FALSE));

		/* Check if write was successful*/
		if (Status == (XST_SUCCESS)) {

			/* Read TMDS configuration*/
			Status = XV_HdmiTx1_DdcRead(InstancePtr,
						    XV_HDMITX1_DDC_ADDRESS,
						    1,
						    (u8*)&DdcBuf,
						    (TRUE));

			/* The result is in ddc_buf[0]*/
			/* Clear scrambling enable bit*/
			DdcBuf[0] &= 0xfe;

			/* Set scrambler bit if scrambler is enabled*/
			if (InstancePtr->Stream.IsScrambled)
				DdcBuf[0] |= 0x01;

			/* Copy buf[0] to buf[1]*/
			DdcBuf[1] = DdcBuf[0];

			/* Offset*/
			DdcBuf[0] = 0x20;   /* Offset scrambler status*/

			/* Write back TMDS configuration*/
			Status = XV_HdmiTx1_DdcWrite(InstancePtr,
						     XV_HDMITX1_DDC_ADDRESS,
						     2,
						     (u8*)&DdcBuf,
						     (TRUE));
		}

		/* Write failed*/
		else {
			return XST_FAILURE;
		}
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function controls the TMDS clock ratio
*
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*       - XST_SUCCESS if HDMI 2.0
*       - XST_FAILURE if HDMI 1.4
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_ClockRatio(XV_HdmiTx1 *InstancePtr)
{
	u8 DdcBuf[2];
	u32 Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Update TMDS configuration*/
	/* Only when it is a HDMI 2.0 sink device*/
	if (InstancePtr->Stream.ScdcSupport) {

		DdcBuf[0] = 0x20;   /* Offset scrambler status*/
		Status = XV_HdmiTx1_DdcWrite(InstancePtr,
					     XV_HDMITX1_DDC_ADDRESS,
					     1,
					     (u8*)&DdcBuf,
					     (FALSE));

		/* Check if write was successful*/
		if (Status == (XST_SUCCESS)) {

			/* Read TMDS configuration*/
			Status = XV_HdmiTx1_DdcRead(InstancePtr,
						    XV_HDMITX1_DDC_ADDRESS,
						    1,
						    (u8*)&DdcBuf,
						    (TRUE));

			/* The result is in ddc_buf[0]*/
			/* Clear TMDS clock ration bit (1)*/
			DdcBuf[0] &= 0xfd;

			/* Set the TMDS clock ratio bit if the bandwidth is
				higher than 3.4 Gbps */
			if (InstancePtr->Stream.TMDSClockRatio) {
				DdcBuf[0] |= 0x02;
			}

			/* Copy buf[0] to buf[1]*/
			DdcBuf[1] = DdcBuf[0];

			/* Offset*/
			DdcBuf[0] = 0x20;   /* Offset scrambler status*/

			/* Write back TMDS configuration*/
			Status = XV_HdmiTx1_DdcWrite(InstancePtr,
						     XV_HDMITX1_DDC_ADDRESS,
						     2,
						     (u8*)&DdcBuf,
						     (TRUE));
		}

		return XST_SUCCESS;
	}

	return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function detects connected sink is a HDMI 2.0/HDMI 1.4 sink device
* and sets appropriate flag in the TX stream.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*       - XST_SUCCESS if HDMI 2.0
*       - XST_FAILURE if HDMI 1.4
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_DetectHdmi20(XV_HdmiTx1 *InstancePtr)
{
	u8 DdcBuf[2];
	u32 Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Write source version. Offset (Source version) */
	DdcBuf[0] = 0x02;

	/* Version 1 */
	DdcBuf[1] = 0x01;
	Status = XV_HdmiTx1_DdcWrite(InstancePtr,
				     XV_HDMITX1_DDC_ADDRESS,
				     2,
				     (u8*)&DdcBuf,
				     (TRUE));

	/* If the write was successful, then the sink is HDMI 2.0 */
	if (Status == (XST_SUCCESS)) {
		InstancePtr->Stream.ScdcSupport = (TRUE);
		Status = (XST_SUCCESS);
	} else {
		/* Else it is a HDMI 1.4 device */
		InstancePtr->Stream.ScdcSupport = (FALSE);
		Status = (XST_FAILURE);
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function shows the sinks SCDC registers.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_ShowSCDC(XV_HdmiTx1 *InstancePtr)
{
	u8 DdcBuf[2];
	u32 Status;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Sink version. Offset Scrambler status */
	DdcBuf[0] = 0x01;
	Status = XV_HdmiTx1_DdcWrite(InstancePtr,
				     XV_HDMITX1_DDC_ADDRESS,
				     1,
				     (u8*)&DdcBuf,
				     (FALSE));

	/* Check if write was successful */
	if (Status == (XST_SUCCESS)) {
		Status = XV_HdmiTx1_DdcRead(InstancePtr,
					    XV_HDMITX1_DDC_ADDRESS,
					    1,
					    (u8*)&DdcBuf,
					    (TRUE));
		xil_printf("HDMI TX: SCDC 0x01 : %0x\r\n", DdcBuf[0]);
	}

	/* TMDS configuration. Offset Scrambler status */
	DdcBuf[0] = 0x20;
	Status = XV_HdmiTx1_DdcWrite(InstancePtr,
				     XV_HDMITX1_DDC_ADDRESS,
				     1,
				     (u8*)&DdcBuf,
				     (FALSE));

	/* Check if write was successful */
	if (Status == (XST_SUCCESS)) {
		Status = XV_HdmiTx1_DdcRead(InstancePtr,
					    XV_HDMITX1_DDC_ADDRESS,
					    1,
					    (u8*)&DdcBuf,
					    (TRUE));
		xil_printf("HDMI TX: SCDC 0x20 : %0x\r\n", DdcBuf[0]);
	}

	/* Scrambler status. Offset Scrambler status */
	DdcBuf[0] = 0x21;
	Status = XV_HdmiTx1_DdcWrite(InstancePtr,
				     XV_HDMITX1_DDC_ADDRESS,
				     1,
				     (u8*)&DdcBuf,
				     (FALSE));

	/* Check if write was successful */
	if (Status == (XST_SUCCESS)) {
		Status = XV_HdmiTx1_DdcRead(InstancePtr,
					    XV_HDMITX1_DDC_ADDRESS,
					    1,
					    (u8*)&DdcBuf,
					    (TRUE));
		xil_printf("HDMI TX: SCDC 0x21 : %0x\r\n", DdcBuf[0]);
	}

	/* Status flags. Offset Scrambler status */
	DdcBuf[0] = 0x40;
	Status = XV_HdmiTx1_DdcWrite(InstancePtr,
				     XV_HDMITX1_DDC_ADDRESS,
				     1,
				     (u8*)&DdcBuf,
				     (FALSE));

	/* Check if write was successful */
	if (Status == (XST_SUCCESS)) {
		Status = XV_HdmiTx1_DdcRead(InstancePtr,
					    XV_HDMITX1_DDC_ADDRESS,
					    1,
					    (u8*)&DdcBuf,
					    (TRUE));
		xil_printf("HDMI TX: SCDC 0x40 : %0x\r\n", DdcBuf[0]);
	}
}

/*****************************************************************************/
/**
*
* This function sets the HDMI TX stream parameters.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param    VideoTiming specifies video timing.
* @param    FrameRate specifies frame rate.
* @param    ColorFormat specifies the type of color format.
*       - 0 = XVIDC_CSF_RGB
*       - 1 = XVIDC_CSF_YCRCB_444
*       - 2 = XVIDC_CSF_YCRCB_422
*       - 3 = XVIDC_CSF_YCRCB_420
* @param    Bpc specifies the color depth/bits per color component.
*       - 6 = XVIDC_BPC_6
*       - 8 = XVIDC_BPC_8
*       - 10 = XVIDC_BPC_10
*       - 12 = XVIDC_BPC_12
*       - 16 = XVIDC_BPC_16
* @param    Ppc specifies the pixel per clock.
*       - 4 = XVIDC_PPC_4
* @param    Info3D 3D info
* @param    FVaFactor - Fast Video Active Factor
* @param    VrrEnabled - VRR is enabled or not
* @param    CnmvrrEnabled - Negative VRR supported flag
* @param    TmdsClock, reference clock calculated based on the input parameters.
*
* @returns	XST_SUCCESS on success else
*		XST_FAILURE
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiTx1_SetStream(XV_HdmiTx1 *InstancePtr,
			 XVidC_VideoTiming VideoTiming,
			 XVidC_FrameRate FrameRate,
			 XVidC_ColorFormat ColorFormat,
			 XVidC_ColorDepth Bpc,
			 XVidC_PixelsPerClock Ppc,
			 XVidC_3DInfo *Info3D,
			 u8 FVaFactor,
			 u8 VrrEnabled,
			 u8 CnmvrrEnabled,
			 u64 *TmdsClock)
{
	u16 Vblank0;
	u16 Vblank1;
	u16 Vfpfva;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((ColorFormat == (XVIDC_CSF_RGB))       ||
		          (ColorFormat == (XVIDC_CSF_YCRCB_444)) ||
		          (ColorFormat == (XVIDC_CSF_YCRCB_422)) ||
		          (ColorFormat == (XVIDC_CSF_YCRCB_420)));
	Xil_AssertNonvoid((Bpc == (XVIDC_BPC_8))  ||
		          (Bpc == (XVIDC_BPC_10)) ||
		          (Bpc == (XVIDC_BPC_12)) ||
		          (Bpc == (XVIDC_BPC_16)));
	Xil_AssertNonvoid((Ppc == (XVIDC_PPC_4)) ||
		          (Ppc == (XVIDC_PPC_8)));
	/* SetVideoStream Start */
	InstancePtr->Stream.Video.Timing	= VideoTiming;
	InstancePtr->Stream.Video.FrameRate	= FrameRate;
	InstancePtr->Stream.Video.ColorFormatId	= ColorFormat;
	InstancePtr->Stream.Video.ColorDepth	= Bpc;
	InstancePtr->Stream.Video.PixPerClk	= Ppc;

	InstancePtr->Stream.Video.IsInterlaced =
			(InstancePtr->Stream.Video.Timing.F1VTotal != 0) ?
					1 : 0;
	/* SetVideoStream End */

	if (Info3D == NULL) {
		/* Set stream to 2D. */
		InstancePtr->Stream.Video.Is3D = FALSE;
		InstancePtr->Stream.Video.Info_3D.Format = XVIDC_3D_UNKNOWN;
		InstancePtr->Stream.Video.Info_3D.Sampling.Method =
				XVIDC_3D_SAMPLING_UNKNOWN;
		InstancePtr->Stream.Video.Info_3D.Sampling.Position =
				XVIDC_3D_SAMPPOS_UNKNOWN;
	} else {
		InstancePtr->Stream.Video.Is3D = TRUE;
		InstancePtr->Stream.Video.Info_3D = *Info3D;

		/* Only 3D format supported is frame packing. */
		if (Info3D->Format != XVIDC_3D_FRAME_PACKING) {
			return XST_FAILURE;
		}

		/* Update the timing based on the 3D format. */

		/* An interlaced format is converted to a progressive frame: */
		/*	3D VActive = (2D VActive * 4) + (2D VBlank field0) +
							(2D Vblank field1 * 2) */
		if (InstancePtr->Stream.Video.IsInterlaced) {
			Vblank0 = InstancePtr->Stream.Video.Timing.F0PVTotal -
					InstancePtr->Stream.Video.Timing.VActive;
			Vblank1 = InstancePtr->Stream.Video.Timing.F1VTotal -
					InstancePtr->Stream.Video.Timing.VActive;
			InstancePtr->Stream.Video.Timing.VActive =
					(InstancePtr->Stream.Video.Timing.VActive * 4) +
					Vblank0 + (Vblank1 * 2);

			/* Set VTotal */
			InstancePtr->Stream.Video.Timing.F0PVTotal *= 2;
			InstancePtr->Stream.Video.Timing.F0PVTotal +=
					InstancePtr->Stream.Video.Timing.F1VTotal
					* 2;

			/* Clear field 1 values. */
			InstancePtr->Stream.Video.Timing.F1VFrontPorch = 0;
			InstancePtr->Stream.Video.Timing.F1VSyncWidth  = 0;
			InstancePtr->Stream.Video.Timing.F1VBackPorch  = 0;
			InstancePtr->Stream.Video.Timing.F1VTotal      = 0;

			/* Set format to progressive */
			InstancePtr->Stream.Video.IsInterlaced = FALSE;
		}
		/* Progressive */
		else {
			/* 3D Vactive = (2D VActive * 2) + (2D VBlank) */
			Vblank0 = InstancePtr->Stream.Video.Timing.F0PVTotal -
					InstancePtr->Stream.Video.Timing.VActive;
			InstancePtr->Stream.Video.Timing.VActive =
					(InstancePtr->Stream.Video.Timing.VActive * 2) +
					Vblank0;

			/* Set VTotal. */
			InstancePtr->Stream.Video.Timing.F0PVTotal =
					InstancePtr->Stream.Video.Timing.F0PVTotal *
					2;
		}
	}

	/** In HDMI the colordepth in YUV422 is always 12 bits,
	* although on the link itself it is being transmitted as 8-bits.
	* Therefore if the colorspace is YUV422, then force the colordepth
	* to 12 bits. */
	if (ColorFormat == XVIDC_CSF_YCRCB_422) {
		InstancePtr->Stream.Video.ColorDepth = XVIDC_BPC_12;
	}

/*	InstancePtr->Stream.Vic = XV_HdmiTx1_LookupVic(
		InstancePtr->Stream.Video.VmId);
*/

	/* Set TX pixel rate*/
	XV_HdmiTx1_SetPixelRate(InstancePtr);

	/* Set TX color space*/
	XV_HdmiTx1_SetColorFormat(InstancePtr);

	/* Set TX color depth*/
	XV_HdmiTx1_SetColorDepth(InstancePtr);

	/* Calculate reference clock. First calculate the pixel clock */
	*TmdsClock = XV_HdmiTx1_GetTmdsClk(InstancePtr);

	/*  update Timing Values for FVA & VRR */


		if (FVaFactor > 1){
			*TmdsClock = *TmdsClock * FVaFactor;
		}
		if (FVaFactor > 1) {
			Vfpfva = (InstancePtr->Stream.Video.Timing.F0PVFrontPorch *
					(FVaFactor)) +
				(VideoTiming.VActive * (FVaFactor - 1));

			if (VrrEnabled && CnmvrrEnabled) {
				InstancePtr->Stream.Video.Timing.F0PVFrontPorch = InstancePtr->Stream.Video.Timing.F0PVFrontPorch *
						FVaFactor;
				InstancePtr->Stream.Video.Timing.F0PVSyncWidth = InstancePtr->Stream.Video.Timing.F0PVSyncWidth *
						FVaFactor;
				InstancePtr->Stream.Video.Timing.F0PVBackPorch = InstancePtr->Stream.Video.Timing.F0PVBackPorch *
						FVaFactor;
				InstancePtr->Stream.Video.Timing.F0PVTotal = InstancePtr->Stream.Video.Timing.VActive +
						InstancePtr->Stream.Video.Timing.F0PVFrontPorch +
						InstancePtr->Stream.Video.Timing.F0PVSyncWidth +
						InstancePtr->Stream.Video.Timing.F0PVBackPorch;
			} else {
				InstancePtr->Stream.Video.Timing.F0PVFrontPorch = Vfpfva;
				InstancePtr->Stream.Video.Timing.F0PVSyncWidth = InstancePtr->Stream.Video.Timing.F0PVSyncWidth *
						FVaFactor;
				InstancePtr->Stream.Video.Timing.F0PVBackPorch = InstancePtr->Stream.Video.Timing.F0PVBackPorch *
								FVaFactor;
				InstancePtr->Stream.Video.Timing.F0PVTotal = InstancePtr->Stream.Video.Timing.VActive +
						InstancePtr->Stream.Video.Timing.F0PVFrontPorch +
						InstancePtr->Stream.Video.Timing.F0PVSyncWidth +
						InstancePtr->Stream.Video.Timing.F0PVBackPorch;
			}
		}
	/* Store TMDS clock for future reference */
	InstancePtr->Stream.TMDSClock = *TmdsClock;

	/* HDMI 2.0 */
	if (InstancePtr->Stream.ScdcSupport && *TmdsClock > 340000000) {
		InstancePtr->Stream.IsScrambled = (TRUE);
		InstancePtr->Stream.TMDSClockRatio  = 1;
	} else {
		/* HDMI 1.4 */
		InstancePtr->Stream.IsScrambled = (FALSE);
		InstancePtr->Stream.TMDSClockRatio  = 0;
	}

	XV_HdmiTx1_Scrambler(InstancePtr);
	XV_HdmiTx1_ClockRatio(InstancePtr);

	if ((InstancePtr->Stream.ScdcSupport == (FALSE)) &&
	    (*TmdsClock > 340000000)) {
		*TmdsClock = 0;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
*  This function asserts or releases the HDMI TX Internal VRST.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param    Reset specifies TRUE/FALSE value to either assert or
*       release HDMI TX Internal VRST.
*
* @return   None.
*
* @note     The reset output of the PIO is inverted. When the system is
*       in reset, the PIO output is cleared and this will reset the
*       HDMI TX. Therefore, clearing the PIO reset output will assert
*       the HDMI Internal video reset.
*       C-style signature:
*       void XV_HdmiTx1_INT_VRST(XV_HdmiTx1 *InstancePtr, u8 Reset)
*
******************************************************************************/
void XV_HdmiTx1_INT_VRST(XV_HdmiTx1 *InstancePtr, u8 Reset)
{

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if (Reset) {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_PIO_OUT_CLR_OFFSET),
				    (XV_HDMITX1_PIO_OUT_INT_VRST_MASK));
	} else {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_PIO_OUT_SET_OFFSET),
				    (XV_HDMITX1_PIO_OUT_INT_VRST_MASK));
	}
}

/*****************************************************************************/
/**
*
*  This function asserts or releases the HDMI TX Internal LRST.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param    Reset specifies TRUE/FALSE value to either assert or
*       release HDMI TX Internal LRST.
*
* @return   None.
*
* @note     The reset output of the PIO is inverted. When the system is
*       in reset, the PIO output is cleared and this will reset the
*       HDMI TX. Therefore, clearing the PIO reset output will assert
*       the HDMI Internal link reset.
*       C-style signature:
*       void XV_HdmiTx1_INT_VRST(XV_HdmiTx1 *InstancePtr, u8 Reset)
*
******************************************************************************/
void XV_HdmiTx1_INT_LRST(XV_HdmiTx1 *InstancePtr, u8 Reset)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if (Reset) {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_PIO_OUT_CLR_OFFSET),
				    (XV_HDMITX1_PIO_OUT_INT_LRST_MASK));
	} else {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_PIO_OUT_SET_OFFSET),
				    (XV_HDMITX1_PIO_OUT_INT_LRST_MASK));
	}
}

/*****************************************************************************/
/**
*
*  This function asserts or releases the HDMI TX External VRST.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param    Reset specifies TRUE/FALSE value to either assert or
*       release HDMI TX External VRST.
*
* @return   None.
*
* @note     The reset output of the PIO is inverted. When the system is
*       in reset, the PIO output is cleared and this will reset the
*       HDMI TX. Therefore, clearing the PIO reset output will assert
*       the HDMI external video reset.
*       C-style signature:
*       void XV_HdmiTx1_EXT_VRST(XV_HdmiTx1 *InstancePtr, u8 Reset)
*
******************************************************************************/
void XV_HdmiTx1_EXT_VRST(XV_HdmiTx1 *InstancePtr, u8 Reset)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if (Reset) {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_PIO_OUT_CLR_OFFSET),
				    (XV_HDMITX1_PIO_OUT_EXT_VRST_MASK));
	} else {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_PIO_OUT_SET_OFFSET),
				    (XV_HDMITX1_PIO_OUT_EXT_VRST_MASK));
	}
}

/*****************************************************************************/
/**
*
*  This function asserts or releases the HDMI TX External SYSRST.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param    Reset specifies TRUE/FALSE value to either assert or
*       release HDMI TX External SYSRST.
*
* @return   None.
*
* @note     The reset output of the PIO is inverted. When the system is
*       in reset, the PIO output is cleared and this will reset the
*       HDMI TX. Therefore, clearing the PIO reset output will assert
*       the HDMI External system reset.
*       C-style signature:
*       void XV_HdmiTx1_EXT_SYSRST(XV_HdmiTx1 *InstancePtr, u8 Reset)
*
******************************************************************************/
void XV_HdmiTx1_EXT_SYSRST(XV_HdmiTx1 *InstancePtr, u8 Reset)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if (Reset) {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_PIO_OUT_CLR_OFFSET),
				    (XV_HDMITX1_PIO_OUT_EXT_SYSRST_MASK));
	} else {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_PIO_OUT_SET_OFFSET),
				    (XV_HDMITX1_PIO_OUT_EXT_SYSRST_MASK));
	}
}

/*****************************************************************************/
/**
*
*  This function sets the HDMI TX AUX GCP register AVMUTE bit.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_SetGcpAvmuteBit(XV_HdmiTx1 *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_PIO_OUT_SET_OFFSET),
			    (XV_HDMITX1_PIO_OUT_GCP_AVMUTE_MASK));
}

/*****************************************************************************/
/**
*
*  This function clears the HDMI TX AUX GCP register AVMUTE bit.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_ClearGcpAvmuteBit(XV_HdmiTx1 *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_PIO_OUT_CLR_OFFSET),
			    (XV_HDMITX1_PIO_OUT_GCP_AVMUTE_MASK));

}

/*****************************************************************************/
/**
*
*  This function sets the HDMI TX AUX GCP register CLEAR_AVMUTE bit.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_SetGcpClearAvmuteBit(XV_HdmiTx1 *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_PIO_OUT_SET_OFFSET),
			    (XV_HDMITX1_PIO_OUT_GCP_CLEARAVMUTE_MASK));
}

/*****************************************************************************/
/**
*
*  This function clears the HDMI TX AUX GCP register CLEAR_AVMUTE bit.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_ClearGcpClearAvmuteBit(XV_HdmiTx1 *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_PIO_OUT_CLR_OFFSET),
			    (XV_HDMITX1_PIO_OUT_GCP_CLEARAVMUTE_MASK));

}

/*****************************************************************************/
/**
*
* This function sets the pixel rate at output.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_SetPixelRate(XV_HdmiTx1 *InstancePtr)
{
	u32 RegValue;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Mask PIO Out Mask register */
	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMITX1_PIO_OUT_MSK_OFFSET),
			    (XV_HDMITX1_PIO_OUT_PIXEL_RATE_MASK));

	/* Check for pixel width */
	switch (InstancePtr->Stream.CorePixPerClk) {
	case (XVIDC_PPC_4):
		RegValue = 2;
		break;

	default:
		RegValue = 0;
		break;
	}

	/* Write pixel rate into PIO Out register */
	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMITX1_PIO_OUT_OFFSET),
			    (RegValue << (XV_HDMITX1_PIO_OUT_PIXEL_RATE_SHIFT)));
}

/*****************************************************************************/
/**
*
* This function sets the sample rate at output.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param    SampleRate specifies the value that needs to be set.
*       - 2 samples per clock
*       - 3 samples per clock.
*       - 5 samples per clock.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_SetSampleRate(XV_HdmiTx1 *InstancePtr, u8 SampleRate)
{
	u32 RegValue;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(SampleRate < 0xFF);

	/* Store sample rate in structure*/
	InstancePtr->Stream.SampleRate = SampleRate;

	/* Mask PIO Out Mask register*/
	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMITX1_PIO_OUT_MSK_OFFSET),
			    (XV_HDMITX1_PIO_OUT_SAMPLE_RATE_MASK));

	/* Check for sample rate*/
	switch (SampleRate) {
	case 2:
		RegValue = 2;
		break;

	case 3:
		RegValue = 1;
		break;

	case 5:
		RegValue = 3;
		break;

	default:
		RegValue = 0;
		break;
	}

	/* Write sample rate into PIO Out register*/
	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMITX1_PIO_OUT_OFFSET),
			    (RegValue << (XV_HDMITX1_PIO_OUT_SAMPLE_RATE_SHIFT)));
}

/*****************************************************************************/
/**
*
* This function sets the color format
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_SetColorFormat(XV_HdmiTx1 *InstancePtr)
{
	u32 RegValue;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Mask PIO Out Mask register */
	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMITX1_PIO_OUT_MSK_OFFSET),
			    (XV_HDMITX1_PIO_OUT_COLOR_SPACE_MASK));

	/* Check for color format */
	switch (InstancePtr->Stream.Video.ColorFormatId) {
	case (XVIDC_CSF_YCRCB_444):
		RegValue = 1;
		break;

	case (XVIDC_CSF_YCRCB_422):
		RegValue = 2;
		break;

	case (XVIDC_CSF_YCRCB_420):
		RegValue = 3;
		break;

	default:
		RegValue = 0;
		break;
	}

	/* Write color space into PIO Out register */
	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMITX1_PIO_OUT_OFFSET),
			    (RegValue << (XV_HDMITX1_PIO_OUT_COLOR_SPACE_SHIFT)));
}

/*****************************************************************************/
/**
*
* This function sets the color depth
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_SetColorDepth(XV_HdmiTx1 *InstancePtr)
{
	u32 RegValue;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Mask PIO Out Mask register */
	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMITX1_PIO_OUT_MSK_OFFSET),
			    (XV_HDMITX1_PIO_OUT_COLOR_DEPTH_MASK));

	/* Color depth*/
	switch (InstancePtr->Stream.Video.ColorDepth) {
	/* 10 bits*/
	case (XVIDC_BPC_10):
		RegValue = 1;
		break;

	/* 12 bits*/
	case (XVIDC_BPC_12):
		RegValue = 2;
		break;

	/* 16 bits*/
	case (XVIDC_BPC_16):
		RegValue = 3;
		break;

	/* 8 bits*/
	default:
		RegValue = 0;
		break;
	}

	/* Write color depth into PIO Out register */
	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMITX1_PIO_OUT_OFFSET),
			    (RegValue << (XV_HDMITX1_PIO_OUT_COLOR_DEPTH_SHIFT)));
}

/*****************************************************************************/
/**
*
* This function prepares TX DDC peripheral to use.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param    Frequency specifies the value that needs to be set.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_DdcInit(XV_HdmiTx1 *InstancePtr, u32 Frequency)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Frequency > 0x0);

	RegValue = (Frequency / 100000) / 2;
	RegValue = ((RegValue) << (XV_HDMITX1_DDC_CTRL_CLK_DIV_SHIFT)) &
		   ((XV_HDMITX1_DDC_CTRL_CLK_DIV_MASK) <<
		    (XV_HDMITX1_DDC_CTRL_CLK_DIV_SHIFT));

	/* Update DDC Control register */
	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			    (XV_HDMITX1_DDC_CTRL_OFFSET), RegValue);
}

/*****************************************************************************/
/**
*
* This function gets the acknowledge flag
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_DdcGetAck(XV_HdmiTx1 *InstancePtr)
{
	u32 Status;

	/* Read status register*/
	Status = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				    (XV_HDMITX1_DDC_STA_OFFSET));
	return (Status & XV_HDMITX1_DDC_STA_ACK_MASK);
}

/*****************************************************************************/
/**
*
* This function waits for the done flag to be set
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_DdcWaitForDone(XV_HdmiTx1 *InstancePtr)
{
	u32 Data;
	u32 Status;
	u32 Exit;

	Exit = (FALSE);

	/* Default status, assume failure*/
	Status = XST_FAILURE;

	do {
		/* Read control register*/
		Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
					  (XV_HDMITX1_DDC_CTRL_OFFSET));

		if (Data & (XV_HDMITX1_DDC_CTRL_RUN_MASK)) {

			/* Read status register*/
			Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
						  (XV_HDMITX1_DDC_STA_OFFSET));

			/* Done*/
			if (Data & (XV_HDMITX1_DDC_STA_DONE_MASK)) {
				/* Clear done flag*/
				XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
						    (XV_HDMITX1_DDC_STA_OFFSET),
						    (XV_HDMITX1_DDC_STA_DONE_MASK));
				Exit = (TRUE);
				Status = XST_SUCCESS;
			}

			/* Time out*/
			else if (Data & (XV_HDMITX1_DDC_STA_TIMEOUT_MASK)) {
				/* Clear time out flag*/
				XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
						    (XV_HDMITX1_DDC_STA_OFFSET),
						    (XV_HDMITX1_DDC_STA_TIMEOUT_MASK));
				Exit = (TRUE);
				Status = XST_FAILURE;
			}
		}
		else {
			Status = (XST_FAILURE);
			Exit = (TRUE);
		}

	} while (!Exit);

	return Status;
}

/*****************************************************************************/
/**
*
* This function writes data into the command fifo.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_DdcWriteCommand(XV_HdmiTx1 *InstancePtr, u32 Cmd)
{
	u32 Status;
	u32 Exit;

	Exit = (FALSE);

	do {
		/* Read control register*/
		Status = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
					    (XV_HDMITX1_DDC_CTRL_OFFSET));

		if (Status & (XV_HDMITX1_DDC_CTRL_RUN_MASK)) {
			/* Read status register*/
			Status = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
						    (XV_HDMITX1_DDC_STA_OFFSET));

			/* Mask command fifo full flag*/
			Status &= XV_HDMITX1_DDC_STA_CMD_FULL;

			/* Check if the command fifo isn't full*/
			if (!Status) {
				XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
						    (XV_HDMITX1_DDC_CMD_OFFSET),
						    (Cmd));
				Exit = (TRUE);
			}
		}
		else {
			Status = (XST_FAILURE);
			Exit = (TRUE);
		}
	} while (!Exit);
}

/*****************************************************************************/
/**
*
* This function reads data from the data fifo.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @note     None.
*
******************************************************************************/
u8 XV_HdmiTx1_DdcReadData(XV_HdmiTx1 *InstancePtr)
{
	u32 Status;
	u32 Exit;
	u32 Data;

	Exit = (FALSE);

	do {
		/* Read control register*/
		Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
					  (XV_HDMITX1_DDC_CTRL_OFFSET));

		if (Data & (XV_HDMITX1_DDC_CTRL_RUN_MASK)) {
			/* Read status register*/
			Status = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
						    (XV_HDMITX1_DDC_STA_OFFSET));

			/* Mask data fifo empty flag*/
			Status &= XV_HDMITX1_DDC_STA_DAT_EMPTY;

			/* Check if the data fifo has data*/
			if (!Status) {
				Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
							  (XV_HDMITX1_DDC_DAT_OFFSET));
				Exit = (TRUE);
			}
		}
		else {
			Exit = (TRUE);
			Data = 0;
		}
	} while (!Exit);

	return (Data);
}

/*****************************************************************************/
/**
*
* This function writes data from DDC peripheral from given slave address.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param    Slave specifies the slave address from where data needs to be
*       read.
* @param    Length specifies number of bytes to be read.
* @param    Buffer specifies a pointer to u8 variable that will be
*       filled with data.
* @param    Stop specifies the stop flag which is either TRUE/FALSE.
*
* @return
*       - XST_SUCCESS if an acknowledgement received and timeout.
*       - XST_FAILURE if no acknowledgement received.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_DdcWrite(XV_HdmiTx1 *InstancePtr, u8 Slave,
	u16 Length, u8 *Buffer, u8 Stop)
{
	u32 Status;
	u32 Data;
	u32 Index;
	u32 TempTimer;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Slave > 0x0);
	Xil_AssertNonvoid(Length > 0x0);
	Xil_AssertNonvoid(Buffer != NULL);
	Xil_AssertNonvoid((Stop == (TRUE)) || (Stop == (FALSE)));

	/* Disable PIO Interrupt to prevent another DDC transactions from happening
	 * in the middle of an ongoing DDC transaction
	 */
	XV_HdmiTx1_PioIntrDisable(InstancePtr);

	/* Disable FRL timer to prevent another DDC transactions from happening
	 * in the middle of an ongoing DDC transaction
	 */
	TempTimer = XV_HdmiTx1_GetFrlTimer(InstancePtr);
	if (TempTimer != 0) {
		XV_HdmiTx1_SetFrlTimerClockCycles(InstancePtr, 0);
	}

	/* Status default, assume failure*/
	Status = XST_FAILURE;

#ifdef DEBUG_DDC_VERBOSITY
	xil_printf("-WR:");
#endif

	/* Enable DDC peripheral*/
	XV_HdmiTx1_DdcEnable(InstancePtr);

	/* Disable interrupt in DDC peripheral*/
	/* Polling is used*/
	XV_HdmiTx1_DdcIntrDisable(InstancePtr);

	/* Write start token*/
	XV_HdmiTx1_DdcWriteCommand(InstancePtr, (XV_HDMITX1_DDC_CMD_STR_TOKEN));

	/* First check if the slave can be addressed*/
	/* Write write token*/
	XV_HdmiTx1_DdcWriteCommand(InstancePtr, (XV_HDMITX1_DDC_CMD_WR_TOKEN));

	/* Write length (high)*/
	XV_HdmiTx1_DdcWriteCommand(InstancePtr, 0);

	/* Write length (low)*/
	XV_HdmiTx1_DdcWriteCommand(InstancePtr, 1);

	/* Slave address*/
	Data = Slave << 1;

	/* Set write bit (low)*/
	Data &= 0xFE;

	/* Write slave address*/
	XV_HdmiTx1_DdcWriteCommand(InstancePtr, (Data));

	/* Wait for done flag*/
	if (XV_HdmiTx1_DdcWaitForDone(InstancePtr) == XST_SUCCESS) {

		/* Check acknowledge*/
		if (XV_HdmiTx1_DdcGetAck(InstancePtr)) {

			/* Now write the data*/
			/* Write write token*/
			XV_HdmiTx1_DdcWriteCommand(InstancePtr,
						   (XV_HDMITX1_DDC_CMD_WR_TOKEN));

			/* Write length (high)*/
			Data = ((Length >> 8) & 0xFF);
			XV_HdmiTx1_DdcWriteCommand(InstancePtr, Data);

			/* Write length (low)*/
			Data = (Length & 0xFF);
			XV_HdmiTx1_DdcWriteCommand(InstancePtr, Data);

			/* Write Data*/
			for (Index = 0; Index < Length; Index++) {
#ifdef DEBUG_DDC_VERBOSITY
		    	xil_printf("[%d]=0x%X, ", Index, *(Buffer));
#endif
				XV_HdmiTx1_DdcWriteCommand(InstancePtr, *Buffer++);
			}

			/* Wait for done flag*/
			if (XV_HdmiTx1_DdcWaitForDone(InstancePtr) == XST_SUCCESS) {

				/* Check acknowledge*/
				/* ACK*/
				if (XV_HdmiTx1_DdcGetAck(InstancePtr)) {

					/* Stop condition*/
					if (Stop) {
						/* Write stop token*/
						XV_HdmiTx1_DdcWriteCommand(InstancePtr,
							(XV_HDMITX1_DDC_CMD_STP_TOKEN));

						/* Wait for done flag*/
						XV_HdmiTx1_DdcWaitForDone(InstancePtr);

					}

					/* Update status flag*/
					Status = XST_SUCCESS;
				}
			}
		}
	}

	/* Disable DDC peripheral*/
	XV_HdmiTx1_DdcDisable(InstancePtr);

	/* Enable the interrupts which were disabled earlier*/
	XV_HdmiTx1_PioIntrEnable(InstancePtr);
	if (TempTimer != 0) {
		XV_HdmiTx1_SetFrlTimerClockCycles(InstancePtr, TempTimer);
	}

#ifdef DEBUG_DDC_VERBOSITY
	xil_printf("\r\n");
#endif

	return Status;
}

/*****************************************************************************/
/**
*
* This function reads data from DDC peripheral from given slave address.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param    Slave specifies the slave address from where data needs to be
*       read.
* @param    Length specifies number of bytes to be read.
* @param    Buffer specifies a pointer to u8 variable that will be
*       filled with data.
* @param    Stop specifies the stop flag which is either TRUE/FALSE.
*
* @return
*       - XST_SUCCESS if an acknowledgement received and timeout.
*       - XST_FAILURE if no acknowledgement received.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_DdcRead(XV_HdmiTx1 *InstancePtr, u8 Slave, u16 Length,
	u8 *Buffer, u8 Stop)
{
	u32 Status;
	u32 Data;
	u32 Index;
	u32 TempTimer;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Slave > 0x0);
	Xil_AssertNonvoid(Length > 0x0);
	Xil_AssertNonvoid(Buffer != NULL);
	Xil_AssertNonvoid((Stop == (TRUE)) || (Stop == (FALSE)));

	/* Disable PIO Interrupt to prevent another DDC transactions from happening
	 * in the middle of an ongoing DDC transaction
	 */
	XV_HdmiTx1_PioIntrDisable(InstancePtr);

	/* Disable FRL timer to prevent another DDC transactions from happening
	 * in the middle of an ongoing DDC transaction
	 */
	TempTimer = XV_HdmiTx1_GetFrlTimer(InstancePtr);
	if (TempTimer != 0) {
		XV_HdmiTx1_SetFrlTimerClockCycles(InstancePtr, 0);
	}

	/* Status default, assume failure*/
	Status = XST_FAILURE;

#ifdef DEBUG_DDC_VERBOSITY
	xil_printf("-RD:");
#endif

	/* Enable DDC peripheral*/
	XV_HdmiTx1_DdcEnable(InstancePtr);

	/* Disable interrupt in DDC peripheral*/
	/* Polling is used*/
	XV_HdmiTx1_DdcIntrDisable(InstancePtr);

	/* Write start token*/
	XV_HdmiTx1_DdcWriteCommand(InstancePtr, (XV_HDMITX1_DDC_CMD_STR_TOKEN));

	/* First check if the slave can be addressed*/
	/* Write write token*/
	XV_HdmiTx1_DdcWriteCommand(InstancePtr, (XV_HDMITX1_DDC_CMD_WR_TOKEN));

	/* Write length (high)*/
	XV_HdmiTx1_DdcWriteCommand(InstancePtr, 0);

	/* Write length (low)*/
	XV_HdmiTx1_DdcWriteCommand(InstancePtr, 1);

	/* Slave address*/
	Data = Slave << 1;

	/* Set read bit (high)*/
	Data |= 0x01;

	/* Write slave address*/
	XV_HdmiTx1_DdcWriteCommand(InstancePtr, (Data));

	/* Wait for done flag*/
	if (XV_HdmiTx1_DdcWaitForDone(InstancePtr) == XST_SUCCESS) {

		/* Check acknowledge*/
		if (XV_HdmiTx1_DdcGetAck(InstancePtr)) {

			/* Write read token*/
			XV_HdmiTx1_DdcWriteCommand(InstancePtr,
						   (XV_HDMITX1_DDC_CMD_RD_TOKEN));

			/* Write read length (high)*/
			Data = (Length >> 8) & 0xFF;
			XV_HdmiTx1_DdcWriteCommand(InstancePtr, (Data));

			/* Write read length (low)*/
			Data = Length & 0xFF;
			XV_HdmiTx1_DdcWriteCommand(InstancePtr, (Data));

			/* Read Data*/
			for (Index = 0; Index < Length; Index++) {
#ifdef DEBUG_DDC_VERBOSITY
				*Buffer = XV_HdmiTx1_DdcReadData(InstancePtr);
				xil_printf("[%d]=0x%X, ", Index, *Buffer++);
#else
				*Buffer++ = XV_HdmiTx1_DdcReadData(InstancePtr);
#endif
			}

			/* Wait for done flag*/
			if (XV_HdmiTx1_DdcWaitForDone(InstancePtr) == XST_SUCCESS) {

				/* Stop condition*/
				if (Stop) {
					/* Write stop token*/
					XV_HdmiTx1_DdcWriteCommand(InstancePtr,
						(XV_HDMITX1_DDC_CMD_STP_TOKEN));

					/* Wait for done flag*/
					XV_HdmiTx1_DdcWaitForDone(InstancePtr);

				}

				/* Update status*/
				Status = XST_SUCCESS;
			}
		}
	}

	/* Disable DDC peripheral*/
	XV_HdmiTx1_DdcDisable(InstancePtr);

	/* Enable the interrupts which were disabled earlier*/
	XV_HdmiTx1_PioIntrEnable(InstancePtr);
	if (TempTimer != 0) {
		XV_HdmiTx1_SetFrlTimerClockCycles(InstancePtr, TempTimer);
	}

#ifdef DEBUG_DDC_VERBOSITY
	xil_printf("\r\n");
#endif

	return Status;
}

/*****************************************************************************/
/**
*
* This function reads specified register from DDC peripheral from given slave
* address.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
* @param    Slave specifies the slave address from where data needs to be
*       	read.
* @param    Length specifies number of bytes to be read.
* @param    RegAddr specifies the register address from where data needs to be
*       	read.
* @param    Buffer specifies a pointer to u8 variable that will be
*       	filled with data.
* @param    Stop specifies the stop flag which is either TRUE/FALSE.
*
* @return
*       - XST_SUCCESS if an acknowledgement received and timeout.
*       - XST_FAILURE if no acknowledgement received.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_DdcReadReg(XV_HdmiTx1 *InstancePtr, u8 Slave, u16 Length,
	u8 RegAddr, u8 *Buffer)
{
	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	int Status = XST_FAILURE;

	/* Set the write register to be read */
	Status = XV_HdmiTx1_DdcWrite(InstancePtr,
				     Slave,
				     1,
				     (u8*)&RegAddr,
				     (FALSE));

	/* Check if write was successful */
	if (Status == (XST_SUCCESS)) {
		/* Read the register */
		Status = XV_HdmiTx1_DdcRead(InstancePtr,
					    Slave,
					    Length,
					    Buffer,
					    (TRUE));
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function writes the specified SCDC Field
*
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
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
int XV_HdmiTx1_DdcWriteField(XV_HdmiTx1 *InstancePtr,
			     XV_HdmiTx1_ScdcFieldType Field,
			     u8 Value)
{
	u8 DdcBuf[2];
	u8 Offset = ScdcField[Field].Offset;
	u32 Status = XST_FAILURE;

	DdcBuf[0] = 0;
	DdcBuf[1] = 0;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Only do read-modify-write if only some bits need to be updated. */
	if (ScdcField[Field].Mask != 0xFF) {
		/* DDC Write to the register in order to read from it */
		Status = XV_HdmiTx1_DdcWrite(InstancePtr,
					     XV_HDMITX1_DDC_ADDRESS,
					     1,
					     (u8*)&Offset,
					     (FALSE));
	} else {
		Status = XST_SUCCESS;
	}

	/* Check if write was successful*/
	if (Status == (XST_SUCCESS)) {
		/* Read register*/
		if (ScdcField[Field].Mask != 0xFF) {
			Status = XV_HdmiTx1_DdcRead(InstancePtr,
						    XV_HDMITX1_DDC_ADDRESS,
						    1,
						    (u8*)&DdcBuf,
						    (TRUE));
		}

		if (Status == (XST_SUCCESS)) {
			if (ScdcField[Field].Mask != 0xFF) {
				DdcBuf[0] &= ~(ScdcField[Field].Mask <<
					       ScdcField[Field].Shift);
			}
			DdcBuf[0] |= ((Value & ScdcField[Field].Mask) <<
				      ScdcField[Field].Shift);
		}

		/* Copy buf[0] to buf[1]*/
		DdcBuf[1] = DdcBuf[0];

		/* Offset*/
		DdcBuf[0] = Offset;

		/* Write back to DDC register*/
		Status = XV_HdmiTx1_DdcWrite(InstancePtr,
					     XV_HDMITX1_DDC_ADDRESS,
					     2,
					     (u8*)&DdcBuf,
					     (TRUE));
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function transmits the infoframes generated by the processor.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*       - XST_SUCCESS if infoframes transmitted successfully.
*       - XST_FAILURE if AUX FIFO is full.
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiTx1_AuxSend(XV_HdmiTx1 *InstancePtr)
{
	u32 Index;
	u32 Status;
	u32 RegValue;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Default*/
	Status = (XST_FAILURE);

	/* Read the AUX status register */
	RegValue = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				      (XV_HDMITX1_AUX_STA_OFFSET));

	/* First check if the AUX packet is ready*/
	if (RegValue & (XV_HDMITX1_AUX_STA_PKT_RDY_MASK)) {

	/* Check if the fifo is full*/
		if (RegValue & (XV_HDMITX1_AUX_STA_FIFO_FUL_MASK)) {
			RegValue = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
						      (XV_HDMITX1_AUX_STA_OFFSET));

			xdbg_printf((XDBG_DEBUG_GENERAL), "HDMI TX AUX"
					" FIFO full\r\n");
		}
		else {
			/* Update AUX with header data */
			XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
					    (XV_HDMITX1_AUX_DAT_OFFSET),
					    InstancePtr->Aux.Header.Data);

			/* Update AUX with actual data */
			for (Index = 0x0; Index < 8; Index++) {
				XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
						    (XV_HDMITX1_AUX_DAT_OFFSET),
						    InstancePtr->Aux.Data.Data[Index]);
			}

			Status = (XST_SUCCESS);
		}
	}
	return Status;
}

/******************************************************************************/
/**
*
* This function prints stream and timing information on STDIO/Uart console.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_Info(XV_HdmiTx1 *InstancePtr)
{

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Print stream information */
	XVidC_ReportStreamInfo(&InstancePtr->Stream.Video);

	/* Print timing information */
	XVidC_ReportTiming(&InstancePtr->Stream.Video.Timing,
		           InstancePtr->Stream.Video.IsInterlaced);
}

/******************************************************************************/
/**
*
* This function prints debug information on STDIO/UART console.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_DebugInfo(XV_HdmiTx1 *InstancePtr)
{
	u32 Data;
	u32 Data1;
	u32 FrlCtrl;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Version */
	Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMITX1_VER_ID_OFFSET));

	xil_printf("CORE_VER_PUB, INT: 0x%X, 0x%X\r\n",
			XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
					(XV_HDMITX1_VER_VERSION_OFFSET)),
			Data & 0xFFFF);
	xil_printf("ADD_CORE_DBG: 0x%X\r\n", (Data >> 16) & 0xFFFF0000);


	FrlCtrl = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_HDMITX1_FRL_CTRL_OFFSET));
	xil_printf("FRL_MODE, LANES: %d, %d\r\n",
			(FrlCtrl & XV_HDMITX1_FRL_CTRL_OP_MODE_MASK) &&
			XV_HDMITX1_FRL_CTRL_OP_MODE_MASK,
			(FrlCtrl & XV_HDMITX1_FRL_CTRL_LN_OP_MASK) &&
			XV_HDMITX1_FRL_CTRL_LN_OP_MASK);

	Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_HDMITX1_PIO_OUT_OFFSET));
	xil_printf("HDMI/DVI MODE: %d\r\n",
			(Data & XV_HDMITX1_PIO_OUT_MODE_MASK) &&
			XV_HDMITX1_PIO_OUT_MODE_MASK);

	Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_HDMITX1_FRL_STA_OFFSET));
	xil_printf("FRL_LCKE_OOS: %d\r\n",
			(Data & XV_HDMITX1_FRL_STA_LNK_CLK_OOS_MASK) &&
			XV_HDMITX1_FRL_STA_LNK_CLK_OOS_MASK);

	Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_HDMITX1_FRL_LNK_CLK_OFFSET));
	xil_printf("FRL_LCKE_FREQ(Khz): %d\r\n", Data);

	xil_printf("FRL_EXT_VCKE: %d\r\n",
			(FrlCtrl & XV_HDMITX1_FRL_VCKE_EXT_MASK) &&
			XV_HDMITX1_FRL_VCKE_EXT_MASK);

	Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_HDMITX1_FRL_VID_CLK_OFFSET));
	xil_printf("FRL_VCKE_FREQ(Khz): %d\r\n", Data);

	Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_HDMITX1_VCKE_SYS_CNT_OFFSET));
	xil_printf("FRL_VCKE_FREQ(Measured Khz): %d\r\n", (5000*400000)/Data);

	Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_HDMITX1_PIO_IN_OFFSET));
	xil_printf("TX Bridge Locked: %d\r\n",
			(Data & XV_HDMITX1_PIO_IN_BRDG_LOCKED_MASK) &&
			XV_HDMITX1_PIO_IN_BRDG_LOCKED_MASK);

	Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
				(XV_HDMITX1_DBG_STS_OFFSET));
	xil_printf("NTV_ANLZ_TIM_CHG (add_core_dbg=1): %d\r\n",
			(Data & XV_HDMITX1_DBG_STS_NTV_ANLZ_VID_TIM_CHG_MASK) &&
			XV_HDMITX1_DBG_STS_NTV_ANLZ_VID_TIM_CHG_MASK);
	xil_printf("TRIB_ANLZ_TIM_CHG (add_core_dbg=2): %d\r\n",
			(Data & XV_HDMITX1_DBG_STS_TRIB_ANLZ_VID_TIM_CHG_MASK) &&
			XV_HDMITX1_DBG_STS_TRIB_ANLZ_VID_TIM_CHG_MASK);


	/* FRL Tribyte Analyzer timing changed counter Status */
	xil_printf("FRL_ANLZ Video Timing (add_core_dbg=3)\r\n"
			"  TIM_CHG: %d\r\n",
			(Data & XV_HDMITX1_DBG_STS_FRL_ANLZ_VID_TIM_CHG_MASK) &&
			XV_HDMITX1_DBG_STS_FRL_ANLZ_VID_TIM_CHG_MASK);

	Data = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMITX1_ANLZ_HBP_HS_OFFSET));
	Data1 = XV_HdmiTx1_ReadReg(InstancePtr->Config.BaseAddress,
			(XV_HDMITX1_ANLZ_LN_ACT_OFFSET));
	xil_printf("  HS, HBP, ACT, LN: %d, %d, %d, %d\r\n",
			(Data & XV_HDMITX1_ANLZ_HBP_HS_HS_SZ_MASK),
			(Data >> XV_HDMITX1_ANLZ_HBP_HS_HPB_SZ_SHIFT) &
			XV_HDMITX1_ANLZ_HBP_HS_HPB_SZ_MASK,
			(Data1 & XV_HDMITX1_ANLZ_LN_ACT_ACT_SZ_MASK),
			(Data1 >> XV_HDMITX1_ANLZ_LN_ACT_LN_SZ_SHIFT) &
			XV_HDMITX1_ANLZ_LN_ACT_LN_SZ_MASK);
}

/*****************************************************************************/
/**
* This function prints out HDMI TX register
*
* @param	InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiTx1_RegisterDebug(XV_HdmiTx1 *InstancePtr)
{
	u32 RegOffset;

	xil_printf("-------------------------------------\r\n");
	xil_printf("       HDMI TX Register Dump \r\n");
	xil_printf("-------------------------------------\r\n");
	for (RegOffset = 0;
		RegOffset <= XV_HDMITX1_FRL_FEC_ERR_INJ_OFFSET; ) {
		xil_printf("0x%04x      0x%08x\r\n",RegOffset,
		XV_HdmiTx1_ReadReg(
		InstancePtr->Config.BaseAddress, RegOffset));
		RegOffset += 4;
		/* Ignore the DDC Space Register */
		if (RegOffset == 0x94) {
			RegOffset = 0xC0;
		}
	}
}

/*****************************************************************************/
/**
*
* This function provides status of the stream
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*       - TRUE = Scrambled.
*       - FALSE = Not scrambled.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_IsStreamScrambled(XV_HdmiTx1 *InstancePtr)
{

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (InstancePtr->Stream.IsScrambled);
}

/*****************************************************************************/
/**
*
* This function provides the stream connected status
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*       - TRUE = Stream is connected.
*       - FALSE = Stream is connected.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_IsStreamConnected(XV_HdmiTx1 *InstancePtr)
{

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (InstancePtr->Stream.IsConnected);
}

/*****************************************************************************/
/**
*
* This function Starts the internal ACR module for FRL
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   Active audio format of HDMI Tx
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_FRLACRStart(XV_HdmiTx1 *InstancePtr)
{
	XHdmiC_FRLCharRate FRLCharRate;
	u32 N_Val;
	u32 RegValue;

	Xil_AssertVoid(InstancePtr != NULL);

	switch (InstancePtr->Stream.Frl.LineRate) {
	case 3:
		FRLCharRate = R_166_667;
		break;
	case 6:
		FRLCharRate = R_333_333;
		break;
	case 8:
		FRLCharRate = R_444_444;
		break;
	case 10:
		FRLCharRate = R_555_556;
		break;
	case 12:
		FRLCharRate = R_666_667;
		break;
	default:
		FRLCharRate = R_166_667;
		break;
	}

	/* Audio Reset Pulse */
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_CLR_OFFSET),
			    (XV_HDMITX1_AUD_CTRL_AUDRST_MASK));

	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_SET_OFFSET),
			    (XV_HDMITX1_AUD_CTRL_AUDRST_MASK));


	N_Val = XHdmiC_FRL_GetNVal(FRLCharRate,
				   InstancePtr->Stream.Audio.SampleFrequency);

	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_ACR_N_OFFSET), N_Val);

	/* 512 = 128*4*Fs */
	RegValue = (0x4 << XV_HDMITX1_AUD_CTRL_AUDCLK_RATIO_SHIFT);

	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_SET_OFFSET), RegValue);

	if (InstancePtr->CTS_N_Source == XV_HDMITX1_INTERNAL_CTS_N) {
		/* Audio ACR Select: Internal CTS */
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_AUD_CTRL_SET_OFFSET),
				    (XV_HDMITX1_AUD_CTRL_ACR_SEL_MASK));
	} else {
		/* Audio ACR Select: External CTS */
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_AUD_CTRL_CLR_OFFSET),
				    (XV_HDMITX1_AUD_CTRL_ACR_SEL_MASK));
	}

	/* Audio ACR Enable */
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_SET_OFFSET),
			    (XV_HDMITX1_AUD_CTRL_ACR_EN_MASK));

}

/*****************************************************************************/
/**
*
* This function Starts the internal ACR module for FRL
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   Active audio format of HDMI Tx
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTx1_TMDSACRStart(XV_HdmiTx1 *InstancePtr)
{
	u64 TMDSCharRate;
	u32 N_Val;
	u32 RegValue;

	Xil_AssertVoid(InstancePtr != NULL);

	TMDSCharRate = InstancePtr->Stream.TMDSClock;


	/* Audio Reset Pulse */
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_CLR_OFFSET),
			    (XV_HDMITX1_AUD_CTRL_AUDRST_MASK));

	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_SET_OFFSET),
			    (XV_HDMITX1_AUD_CTRL_AUDRST_MASK));


	N_Val = XHdmiC_TMDS_GetNVal (TMDSCharRate,
			InstancePtr->Stream.Audio.SFreq);

	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_ACR_N_OFFSET), N_Val);

	/* 512 = 128*4*Fs */
	RegValue = (0x4 << XV_HDMITX1_AUD_CTRL_AUDCLK_RATIO_SHIFT);

	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_SET_OFFSET),
			    RegValue);

	RegValue = 0x0;

	/* Setting the Data Clock and Link Clock Ratio, and for PPC 4 it
	 * always 0x4
	 */
	RegValue = (0x4 << XV_HDMITX1_AUD_CTRL_DATACLK_LNKCLK_RATIO_SHIFT);

	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_SET_OFFSET),
			    RegValue);

	if (InstancePtr->CTS_N_Source == XV_HDMITX1_INTERNAL_CTS_N) {
		/* Audio ACR Select: Internal CTS */
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_AUD_CTRL_SET_OFFSET),
				    (XV_HDMITX1_AUD_CTRL_ACR_SEL_MASK));
	} else {
		/* Audio ACR Select: External CTS */
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_AUD_CTRL_CLR_OFFSET),
				    (XV_HDMITX1_AUD_CTRL_ACR_SEL_MASK));
	}

	/* Audio ACR Enable */
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_SET_OFFSET),
			    (XV_HDMITX1_AUD_CTRL_ACR_EN_MASK));

}

/*****************************************************************************/
/**
*
* This function sets the active audio channels
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*       - XST_SUCCESS if active channels were set.
*       - XST_FAILURE if no active channles were set.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_SetAudioChannels(XV_HdmiTx1 *InstancePtr, u8 Value)
{
	u32 Data;
	u32 Status = XST_SUCCESS;
	u8 AudioStatus;
	u32 AudioCtrl;
	u8 Flag3d = FALSE;

	AudioStatus = XV_HdmiTx1_ReadReg((InstancePtr)->Config.BaseAddress,
					 XV_HDMITX1_AUD_CTRL_OFFSET) &
		      XV_HDMITX1_AUD_CTRL_RUN_MASK;

	AudioCtrl = XV_HdmiTx1_ReadReg((InstancePtr)->Config.BaseAddress,
			XV_HDMITX1_AUD_CTRL_OFFSET);

	AudioStatus = AudioCtrl & XV_HDMITX1_AUD_CTRL_RUN_MASK;

	/* Stop peripheral*/
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_CLR_OFFSET),
			    (XV_HDMITX1_AUD_CTRL_RUN_MASK));

	AudioCtrl &= ~((u32)XV_HDMITX1_AUD_CTRL_RUN_MASK);

	switch (Value) {
	case 32:
	    Data = 6;
	    Flag3d = TRUE;
	    break;

	case 24:
	    Data = 4;
	    Flag3d = TRUE;
	    break;

	case 12:
	    Data = 0;
	    Flag3d = TRUE;
	    break;

	/* 8 Channels*/
	case 8:
	    Data = 3;
	    break;

	/* 6 Channels*/
	case 6:
	    Data = 2;
	    break;

	/* 4 Channels*/
	case 4:
	    Data = 1;
	    break;

	/* 2 Channels*/
	case 2:
	    Data = 0;
	    break;

	default :
	    Status = (XST_FAILURE);
	    break;
	}

	if (Status == (XST_SUCCESS)) {
		if (Flag3d) {
			Data &= XV_HDMITX1_3DAUD_CTRL_CH_MASK;
			Data <<= XV_HDMITX1_3DAUD_CTRL_CH_SHIFT;
			AudioCtrl &= ~((u32)(XV_HDMITX1_3DAUD_CTRL_CH_MASK <<
					XV_HDMITX1_3DAUD_CTRL_CH_SHIFT));
		} else {
			Data &= XV_HDMITX1_AUD_CTRL_CH_MASK;
			Data <<= XV_HDMITX1_AUD_CTRL_CH_SHIFT;
			AudioCtrl &= ~((u32)(XV_HDMITX1_AUD_CTRL_CH_MASK <<
					XV_HDMITX1_AUD_CTRL_CH_SHIFT));
		}

		AudioCtrl |= Data;

		/* Set active channels*/
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				(XV_HDMITX1_AUD_CTRL_OFFSET), AudioCtrl);

		/* Store active channel in structure*/
		(InstancePtr)->Stream.Audio.Channels = Value;

		/* Start peripheral*/
		if (AudioStatus) {
			XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
					    (XV_HDMITX1_AUD_CTRL_SET_OFFSET),
					    (XV_HDMITX1_AUD_CTRL_RUN_MASK));

			XV_HdmiTx1_AudioEnable(InstancePtr);
	    }
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function gets the Generated ACR CTS Value
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return   Generated ACR CTS Value
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiTxSs1_GetAudioCtsVal(XV_HdmiTx1 *InstancePtr)
{
	u32 CtsVal;

	CtsVal = XV_HdmiTx1_ReadReg((InstancePtr)->Config.BaseAddress,
				    XV_HDMITX1_AUD_ACR_CTS_OFFSET) &
				    XV_HDMITX1_AUD_CTRL_ACR_CTS_MASK;

	return CtsVal;
}

/*****************************************************************************/
/**
*
* This function gets the programmed ACR N Value
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return   Programmed ACR N Value
*
* @note     None.
*
******************************************************************************/
u32 XV_HdmiTxSs1_GetAudioNVal(XV_HdmiTx1 *InstancePtr)
{
	u32 NVal;

	NVal = XV_HdmiTx1_ReadReg((InstancePtr)->Config.BaseAddress,
				  XV_HDMITX1_AUD_ACR_N_OFFSET) &
	       XV_HDMITX1_AUD_CTRL_ACR_N_MASK;

	return NVal;
}


/*****************************************************************************/
/**
*
* This function sets the active audio format
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return
*       - XST_SUCCESS if active channels were set.
*       - XST_FAILURE if no active channles were set.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTx1_SetAudioFormat(XV_HdmiTx1 *InstancePtr,
			      XV_HdmiTx1_AudioFormatType Value)
{
	u32 Status;
	u32 AudioCtrl;

	if (Value > XV_HDMITX1_AUDFMT_3D)
		return XST_FAILURE;

	AudioCtrl = XV_HdmiTx1_ReadReg((InstancePtr)->Config.BaseAddress,
				XV_HDMITX1_AUD_CTRL_OFFSET);

	/* Stop peripheral*/
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_CLR_OFFSET),
			    (XV_HDMITX1_AUD_CTRL_RUN_MASK));

	/* default LPCM */
	XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_CLR_OFFSET),
			    (XV_HDMITX1_AUD_CTRL_AUDFMT_MASK |
			     XV_HDMITX1_AUD_CTRL_3DAUDFMT_MASK));

	switch (Value) {
	case XV_HDMITX1_AUDFMT_3D:
		/* 3D audio*/
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_SET_OFFSET),
			    XV_HDMITX1_AUD_CTRL_3DAUDFMT_EN);
		break;

	case XV_HDMITX1_AUDFMT_HBR:
		/* HBR audio*/
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
				    (XV_HDMITX1_AUD_CTRL_SET_OFFSET),
				    (XV_HDMITX1_AUD_CTRL_AUDFMT_MASK));
		break;

	default:
		break;
	}

	/* Start peripheral*/
	if (AudioCtrl & XV_HDMITX1_AUD_CTRL_RUN_MASK) {
		XV_HdmiTx1_WriteReg((InstancePtr)->Config.BaseAddress,
			    (XV_HDMITX1_AUD_CTRL_SET_OFFSET),
			    (XV_HDMITX1_AUD_CTRL_RUN_MASK));
	}

	XV_HdmiTx1_AudioEnable(InstancePtr);

	Status = (XST_SUCCESS);

	return Status;
}

/*****************************************************************************/
/**
*
* This function gets the active audio format
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return   Active audio format of HDMI Tx
*
* @note     None.
*
******************************************************************************/
XV_HdmiTx1_AudioFormatType XV_HdmiTx1_GetAudioFormat(XV_HdmiTx1 *InstancePtr)
{
	XV_HdmiTx1_AudioFormatType RegValue;

	Xil_AssertNonvoid(InstancePtr != NULL);

	RegValue = XV_HdmiTx1_ReadReg((InstancePtr)->Config.BaseAddress,
				      (XV_HDMITX1_AUD_CTRL_OFFSET));
	/* 3D */
	if (RegValue & XV_HDMITX1_AUD_CTRL_3DAUDFMT_MASK)
		return XV_HDMITX1_AUDFMT_3D;

	RegValue = (RegValue & (XV_HDMITX1_AUD_CTRL_AUDFMT_MASK)) >>
	           XV_HDMITX1_AUD_CTRL_AUDFMT_SHIFT;

	return RegValue;
}

/*****************************************************************************/
/**
*
* This function reads the CED and RSED registers from the sink and returns the
* pointer to the data structure which stores the CED related readings (from
* SCDC register 0x50 to 0x5A).
*
* @param	InstancePtr is a pointer to the XHdmi_Tx core instance.
*
* @return	None.
*
* @note		Reading the SCDC registers will clear the values at the sink
*
******************************************************************************/
u8 *XV_HdmiTx1_GetScdcEdRegisters(XV_HdmiTx1 *InstancePtr)
{
	u8 ReadCounts = 0;

	if (InstancePtr->Stream.IsFrl == TRUE) {
		ReadCounts = 11;
	} else {
		ReadCounts = 7;
	}

	XV_HdmiTx1_DdcReadReg(InstancePtr,
			      XV_HDMITX1_DDC_ADDRESS,
			      ReadCounts,
			      XV_HDMITX1_DDC_CED_REG,
			      (u8*)&(InstancePtr->Stream.ScdcEd));

	return &(InstancePtr->Stream.ScdcEd[0]);
}

/*****************************************************************************/
/**
*
* This function is a stub for the asynchronous callback. The stub is here in
* case the upper layer forgot to set the handlers. On initialization, all
* handlers are set to this callback. It is considered an error for this
* handler to be invoked.
*
* @param    CallbackRef is a callback reference passed in by the upper
*       layer when setting the callback functions, and passed back to
*       the upper layer when the callback is invoked.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void StubCallback(void *Callback)
{
	Xil_AssertVoid(Callback != NULL);
	Xil_AssertVoidAlways();
}

XV_HdmiC_VideoTimingExtMeta *XV_HdmiTx1_GetVidTimingExtMeta(
		XV_HdmiTx1 *InstancePtr)
{
	return &(InstancePtr->VrrIF.VidTimingExtMeta);
}

void XV_HdmiTx1_GenerateVideoTimingExtMetaIF(XV_HdmiTx1 *InstancePtr,
			XV_HdmiC_VideoTimingExtMeta *ExtMeta)
{
	u32 Data = 0;
	XV_HdmiC_VideoTimingExtMeta *Tx1ExtMeta;

	Xil_AssertVoid(InstancePtr != NULL);

	Tx1ExtMeta = XV_HdmiTx1_GetVidTimingExtMeta(InstancePtr);
	/* Only for info/debugging, no functional usage */
	memcpy(Tx1ExtMeta, ExtMeta, sizeof(XV_HdmiC_VideoTimingExtMeta));

	Data |= ExtMeta->VRREnabled;
	Data |= ExtMeta->MConstEnabled << XV_HDMITX1_AUX_VTEM_M_CONST_SHIFT;
	Data |= ExtMeta->FVAFactorMinus1 <<
		XV_HDMITX1_AUX_VTEM_FVA_FACT_M1_SHIFT;
	Data |= ExtMeta->BaseVFront << XV_HDMITX1_AUX_VTEM_BASE_VFRONT_SHIFT;
	Data |= ExtMeta->BaseRefreshRate <<
		XV_HDMITX1_AUX_VTEM_BASE_REFRESH_RATE_SHIFT;
	Data |= ExtMeta->RBEnabled << XV_HDMITX1_AUX_VTEM_RB_SHIFT;

	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			XV_HDMITX1_AUX_VTEM_OFFSET, Data);
}

XV_HdmiC_SrcProdDescIF *XV_HdmiTx1_GetSrcProdDescIF(
		XV_HdmiTx1 *InstancePtr)
{
	return &(InstancePtr->VrrIF.SrcProdDescIF);
}

void XV_HdmiTx1_GenerateSrcProdDescInfoframe(XV_HdmiTx1 *InstancePtr,
			XV_HdmiC_SrcProdDescIF *SpdIfPtr)
{
	XV_HdmiC_SrcProdDescIF *Tx1SpdIfPtr;
	XV_HdmiC_FreeSync *FreeSyncPtr;
	XV_HdmiC_FreeSyncPro *FreeSyncProPtr;
	u32 FsyncData = 0, FsyncProData = 0;

	Xil_AssertVoid(InstancePtr != NULL);

	Tx1SpdIfPtr = XV_HdmiTx1_GetSrcProdDescIF(InstancePtr);
	/* Only for info/debugging, no functional usage */
	memcpy(Tx1SpdIfPtr, SpdIfPtr, sizeof(XV_HdmiC_SrcProdDescIF));

	FreeSyncPtr = &SpdIfPtr->FreeSync;
	FreeSyncProPtr = &SpdIfPtr->FreeSyncPro;

	FsyncData |= FreeSyncPtr->Version;
	FsyncData |= FreeSyncPtr->FreeSyncSupported <<
		XV_HDMITX1_AUX_FSYNC_SUPPORT_SHIFT;
	FsyncData |= FreeSyncPtr->FreeSyncEnabled <<
		XV_HDMITX1_AUX_FSYNC_ENABLED_SHIFT;
	FsyncData |= FreeSyncPtr->FreeSyncActive <<
		XV_HDMITX1_AUX_FSYNC_ACTIVE_SHIFT;
	FsyncData |= FreeSyncPtr->FreeSyncMinRefreshRate <<
		XV_HDMITX1_AUX_FSYNC_MIN_REF_RATE_SHIFT;
	FsyncData |= FreeSyncPtr->FreeSyncMaxRefreshRate <<
		XV_HDMITX1_AUX_FSYNC_MAX_REF_RATE_SHIFT;

	XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			XV_HDMITX1_AUX_FSYNC_OFFSET, FsyncData);

	if (FreeSyncPtr->Version == 2) {
		FsyncProData |= FreeSyncProPtr->NativeColorSpaceActive <<
			XV_HDMITX1_AUX_FSYNC_PRO_NTV_CS_ACT_SHIFT;
		FsyncProData |= FreeSyncProPtr->BrightnessControlActive <<
			XV_HDMITX1_AUX_FSYNC_PRO_BRIGHT_CTRL_ACT_SHIFT;
		FsyncProData |= FreeSyncProPtr->LocalDimControlActive <<
			XV_HDMITX1_AUX_FSYNC_PRO_LDIMM_CTRL_ACT_SHIFT;
		FsyncProData |= FreeSyncProPtr->sRGBEOTFActive;
		FsyncProData |= FreeSyncProPtr->BT709EOTFActive <<
			XV_HDMITX1_AUX_FSYNC_PRO_BT709_EOTF_SHIFT;
		FsyncProData |= FreeSyncProPtr->Gamma22EOTFActive <<
			XV_HDMITX1_AUX_FSYNC_PRO_GAMMA_2_2_EOTF_SHIFT;
		FsyncProData |= FreeSyncProPtr->Gamma26EOTFActive <<
			XV_HDMITX1_AUX_FSYNC_PRO_GAMMA_2_6_EOTF_SHIFT;
		FsyncProData |= FreeSyncProPtr->PQEOTFActive <<
			XV_HDMITX1_AUX_FSYNC_PRO_PQ_EOTF_SHIFT;
		FsyncProData |= FreeSyncProPtr->BrightnessControl <<
			XV_HDMITX1_AUX_FSYNC_PRO_BRIGHT_CTRL_SHIFT;

		XV_HdmiTx1_WriteReg(InstancePtr->Config.BaseAddress,
			XV_HDMITX1_AUX_FSYNC_PRO_OF, FsyncProData);
	}
}
