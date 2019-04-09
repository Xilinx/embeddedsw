/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_menu.c
*
* This file contains the Xilinx implementation to handle i2s 2 channel as used
* I2S Audio in in the HDMI example design.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------
* X.X   ..   DD-MM-YYYY ..
* 1.0   RHe  10-07-2015 Initial version
* </pre>
*
******************************************************************************/

#include "xparameters.h"

#ifdef XPAR_XI2SRX_NUM_INSTANCES

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "xil_assert.h"
#include "xil_types.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "audiogen_drv.h"
#include "xv_hdmirxss.h"
#include "xv_hdmitxss.h"


#include "xi2stx.h"
#include "xi2srx.h"
#include "xgpio.h"

#include "xhdmi_example.h"

u32 data;
u32 appx_fs = 0;
u32 appx_fs_track = 0;
u32 CTS = 0;
u32 N = 0;
u32 tmds = 0;
u8 i2s_invalid = 0;
u8 ratio = 0;
u8 ratio_tmds = 0;
u8 ratio_tmds_track = 0;
u8 rate = 0;

u8 tx_i2s = 0;
u8 rx_i2s = 0;
u8 only_tx = 0;

XI2s_Tx I2s_tx;
XI2s_Rx I2s_rx;

XI2stx_Config *Config;
XI2srx_Config *Config_rx;

XGpio              Gpio_AudClk_resetn;
XGpio_Config       *Gpio_AudClk_resetn_ConfigPtr;

#define I2S_CLK_MULT 768
#define XACR_WriteReg(BaseAddress, RegOffset, Data)   \
    XAudGen_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))

u32 i2s_init();

void aud_clk_reset (void);
void i2s_audio(u32 tmds,
               u8 ratio_tmds,
               u8 tx_up,
               u8 rx_up,
               XV_HdmiTxSs *HdmiTxSsPtr,
               XV_HdmiRxSs *HdmiRxSsPtr,
               XhdmiAudioGen_t *AudioGen);
void start_i2s_rx ();
void hdmi_tx_acr (XV_HdmiTxSs *HdmiTxSsPtr, XhdmiAudioGen_t *AudioGen);


/*****************************************************************************/
/**
*
* This function Initialize the I2S Receiver and Transmitter
*
* @param
*
* @return Initialization status
*
*
******************************************************************************/
u32 i2s_init() {

	u32 Status;

	Config = XI2s_Tx_LookupConfig(XPAR_XI2STX_0_DEVICE_ID);
	if (Config == NULL) {
		 return XST_FAILURE;
	}

	Status = XI2s_Tx_CfgInitialize(&I2s_tx, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		 return XST_FAILURE;
	}


	Config_rx = XI2s_Rx_LookupConfig(XPAR_XI2SRX_0_DEVICE_ID);
	if (Config == NULL) {
		  return XST_FAILURE;
	}

	Status = XI2s_Rx_CfgInitialize(&I2s_rx,
				Config_rx,
				Config_rx->BaseAddress);
	if (Status != XST_SUCCESS) {
		  return XST_FAILURE;
	}

	Gpio_AudClk_resetn_ConfigPtr =
		XGpio_LookupConfig(XPAR_AUDIO_I2S_SS_0_AXI_GPIO_DEVICE_ID);

	if(Gpio_AudClk_resetn_ConfigPtr == NULL) {
		Gpio_AudClk_resetn.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XGpio_CfgInitialize(&Gpio_AudClk_resetn,
				Gpio_AudClk_resetn_ConfigPtr,
				Gpio_AudClk_resetn_ConfigPtr->BaseAddress);
	if(Status != XST_SUCCESS) {
		xil_printf("ERR:: GPIO for TPG Reset ");
		xil_printf("Initialization failed %d\r\n", Status);
		return(XST_FAILURE);
	}

	return Status;
}

/*****************************************************************************/
/**
*
* The I2S RX and TX, can work at different rates based on the clocks provided
* On ZCU102, they work at same rate as they use the same Audio clock
*
* @param
*
* @return
*
*
******************************************************************************/
void start_i2s_rx () {
	XI2s_Rx_SetSclkOutDiv (&I2s_rx, appx_fs*I2S_CLK_MULT, appx_fs);
	XI2s_Rx_Enable(&I2s_rx, 1);
}

/*****************************************************************************/
/**
*
* Start the I2S Transmitter operation
*
* @param
*
* @return
*
*
******************************************************************************/
void start_i2s_tx () {
	XI2s_Tx_SetSclkOutDiv (&I2s_tx, appx_fs*I2S_CLK_MULT, appx_fs);
	XI2s_Tx_Enable(&I2s_tx, 1);
}

/*****************************************************************************/
/**
*
* Following process resets the PSR that is connected to Audio Clock
*
* @param
*
* @return
*
*
******************************************************************************/
void aud_clk_reset (void) {
	/* Will keep the PSR in reset state */
	XGpio_WriteReg (Gpio_AudClk_resetn_ConfigPtr->BaseAddress, 0x0, 0x0);
	/* Will keep the PSR in reset state */
	XGpio_WriteReg (Gpio_AudClk_resetn_ConfigPtr->BaseAddress, 0x0, 0x0);
	/* Will keep the PSR out of reset state */
	XGpio_WriteReg (Gpio_AudClk_resetn_ConfigPtr->BaseAddress, 0x0, 0x1);
}

/*****************************************************************************/
/**
*
* Following process sets up the HDMI ACR CTRL (TX side)
* and configured the HDMI TX for 2 Channel PCM Audio
* HDMI TX ACR always programmed in generate mode
*
* @param
*
* @return
*
*
******************************************************************************/
void hdmi_tx_acr (XV_HdmiTxSs *HdmiTxSsPtr , XhdmiAudioGen_t *AudioGen) {

	XV_HdmiTxSs_AudioMute(HdmiTxSsPtr, TRUE);
	/* Put the HDMI_ACR_CTRL in gen mode */
	XhdmiACRCtrl_Sel(AudioGen, ACR_SEL_GEN);
	XhdmiACRCtrl_Enab(AudioGen, 0);
	data = XHdmi_ACR_GetNVal(XV_HdmiTxSs_GetTmdsClockFreqHz(HdmiTxSsPtr),
				rate);
	XhdmiACRCtrl_SetNVal(AudioGen, data);
	XhdmiACRCtrl_TMDSClkRatio(AudioGen,
				HdmiTxSsPtr->HdmiTxPtr->Stream.TMDSClockRatio);
	XhdmiACRCtrl_Enab(AudioGen, 1);
	XV_HdmiTxSs_SetAudioFormat(HdmiTxSsPtr, XV_HDMITX_AUDFMT_LPCM);
	XV_HdmiTxSs_SetAudioChannels(HdmiTxSsPtr, 2);
	XV_HdmiTxSs_AudioMute(HdmiTxSsPtr, FALSE);
}

/*****************************************************************************/
/**
*
* The following process configures the I2S and continuously tracks the incoming
* Audio sampling rate on RX side
* This exdes supports 32K, 44.1K and 44K sampling rates
* A change in Sampling rate triggers re programming of Si5328 and I2S IP
* In absence of HDMI RX, the I2S RX is always configured for 48Khz
*
* @param
*
* @return
*
*
******************************************************************************/
void i2s_audio (u32 tmds, u8 ratio_tmds, u8 tx_up, u8 rx_up,
			XV_HdmiTxSs *HdmiTxSsPtr,
			XV_HdmiRxSs *HdmiRxSsPtr,
			XhdmiAudioGen_t *AudioGen) {

	if (tx_up) {
		if (!rx_up && !only_tx) {
			/* Programming only for TX i.e. non PassThrough Mode */
			appx_fs = 48000;
			/* disable RX ACR */
			XACR_WriteReg
			    (XPAR_AUDIO_I2S_SS_0_AUDIO_CLOCK_RECOVERY_BASEADDR,
			     0x8,
			     0x0);
			I2cClk_Ps(0, 48000*I2S_CLK_MULT);
			usleep(15000);
			aud_clk_reset();
			hdmi_tx_acr (HdmiTxSsPtr, AudioGen);
			start_i2s_rx();
			print("Programming the I2S RX (48Khz)...\r\n");
			only_tx = 1;
			tx_i2s = 0;
		}
	} else {
		only_tx = 0;
	}

	if (rx_up) {
		only_tx = 0;
		/* Start I2S TX after HDMI RX
		 * there is no good way to find out availability of N, CTS
		 * hence the following
		 */
		CTS = XV_HdmiRx_GetAcrCts(HdmiRxSsPtr->HdmiRxPtr);
		N = XV_HdmiRx_GetAcrN(HdmiRxSsPtr->HdmiRxPtr);
		if (CTS != 0 && N != 0) {
			i2s_invalid = 0;
			if (ratio_tmds == 1) {
				ratio = 4;
			} else {
				ratio = 1;
			}
			appx_fs = (tmds / CTS) * (ratio);
			appx_fs = (appx_fs * N)/128;
			appx_fs = appx_fs / 1000;
			/* list may be extended for other sampling rates */
			if (appx_fs >= 31 && appx_fs <= 33) {
				appx_fs = 32000;
				rate = 0;
			} else if (appx_fs >= 43 && appx_fs <= 45) {
				appx_fs = 44100;
				rate = 1;
			} else if (appx_fs >= 47 && appx_fs <= 49) {
				appx_fs = 48000;
				rate = 2;
			} else {
				/* I2S Invalid */
				i2s_invalid = 1;
			}
			if ((appx_fs_track != appx_fs) ||
				(ratio_tmds != ratio_tmds_track)) {
				tx_i2s = 0;
			}
			if (!i2s_invalid && !tx_i2s) {
				XACR_WriteReg
			(XPAR_AUDIO_I2S_SS_0_AUDIO_CLOCK_RECOVERY_BASEADDR,
					0x20, 0x0);
				/* Divider */
				XACR_WriteReg
			(XPAR_AUDIO_I2S_SS_0_AUDIO_CLOCK_RECOVERY_BASEADDR,
					0x70, 0x10);
				XACR_WriteReg
			(XPAR_AUDIO_I2S_SS_0_AUDIO_CLOCK_RECOVERY_BASEADDR,
					0x8, 0x1);
				/* appx_fs*512 is the required
				 * Audio clock for DACs/ADCs/I2S
				 */
				I2cClk_Ps(appx_fs*2/ratio,
					appx_fs*I2S_CLK_MULT);
				usleep(15000);
				aud_clk_reset();
				start_i2s_tx();
				tx_i2s = 1;
				/* ensures I2S RX is always
				 * started as a slave
				 */
				rx_i2s = 0;
				appx_fs_track = appx_fs;
				ratio_tmds_track = ratio_tmds;
				xil_printf ("Received Audio Sampling Fs is"
				" : %d Hz\r\n", appx_fs);
			}
		}
		if (tx_up && !rx_i2s && tx_i2s) {
			/* Start I2S RX after all things
			 * with HDMI RX Audio are done.
			 * TX is slave to RX
			 */
			hdmi_tx_acr (HdmiTxSsPtr, AudioGen);
			start_i2s_rx();
			rx_i2s = 1;
		}
	}
}
#endif
