/******************************************************************************
*
* Copyright (C) 2014 - 2017 Xilinx, Inc.  All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xhdmi_example.c
*
* This file demonstrates how to use Xilinx HDMI TX Subsystem, HDMI RX Subsystem
* and Video PHY Controller drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         25/11/15 Initial release.
* 1.10         05/02/16 Updated function RxAuxCallback.
* 2.00  MG     02/03/15 Added upgraded with HDCP driver and overlay
* 2.10  MH     06/23/16 Added HDCP repeater support.
* 2.11  YH     04/08/16 Added two level validation routines
*                       Basic_validation will only check the received VmId
*                       PRBS_validation will check both video & audio contents
* 2.12  GM     07/10/16 Added onboard SI5324 Initialization API to enable
*                       125Mhz as NI-DRU reference clock
* 2.13  YH     03/01/16 Fixed a system hang issue by clearing TxBusy flag when
*                            a non-supportedvideo resolution is set
*                            during enable colorbar API
* 2.14  GM     23/01/17 Replace the Extraction Value of VPhy line rate with,
*                            XVphy_GetLineRateHz Rate API return value.
* 2.15  ms     04/10/17 Modified filename tag to include the file in doxygen
*                            examples.
* 2.16  mmo    05/05/17 Replace pre-processed interrupt vector ID with the
*                            pre-processed canonical interrupt vector ID for
*                            microblaze processor
* 2.17  YH     12/06/17 Removed unused PRBS validation related codes
*                       Added VPHY error processing APIs and typedef
*                       Placed Si5324 on reset on bonded mode in StartTxAfterRx
*                       Changed printf usage to xil_printf
*                       Changed "\n\r" in xil_printf calls to "\r\n"
* 2.18  mmo    21/07/17 Remove the i2c_dp159 API Call and
*                            XVphy_Clkout1OBufTdsEnable API Call from the
*                            TxStreamCallback API to avoid the race condition,
*                            and replace to be call at the global while loop.
*       MH     26/07/17 Set TMDS SCDC register after TX HPD toggle event
*       GM     18/08/17 Added SI Initialization after the SI Reset in
*                            StartTxAfterRx API
*       YH     18/08/17 Add HDCP Ready checking before set down streams
*       GM     28/08/17 Replace XVphy_HdmiInitialize API Call during
*                            Initialization with XVphy_Hdmi_CfgInitialize API
*                            Call
*       mmo    04/10/17 Updated function TxStreamUpCallback to include
*                            XhdmiACRCtrl_TMDSClkRatio API Call
*       EB     06/11/17 Updated function RxAudCallback to allow pass-through
*                            of audio format setting
* 3.00  mmo    29/12/17 Added EDID Parsing Capability
*       EB     16/01/18 Added InfoFrame capability
*       YH     16/01/18 Added video_bridge overflow interrupt
*                       Added video_bridge unlock interrupt
*       GM     16/01/18 Updated EnableColorBar to skip TX reconfiguration
*                            when the requested TX video resolution
*                            is not supported
*       EB     23/01/18 Modified RxStreamUpCallback so that scrambling flag
*                            is always enabled for HDMI 2.0 resolutions and
*                            always disabled for HDMI 1.4 resolutions during
*                            pass-through mode
*       EB     26/01/18 Updated function UpdateFrameRate to use the API
*                            XVidC_GetVideoModeIdExtensive
*       MMO    08/02/18 Updated the EnableColorBar, UpdateFrameRate,
*                            UpdateColorDepth, UpdateColorFormat API for
*                            clean flow.
*       GM              Added support for ZCU104
*       SM     28/02/18 Added code to call API for setting App version to
*                            support backward compatibility related issues.
* 3.01  EB     09/04/18 Updated XV_ConfigTpg and EnableColorBar APIs
*              18/04/18 Updated RxBrdgOverflowCallback to remove printing
* 3.02  mmo    23/04/18 Added checking Sink Capability whether it's a DVI sink
*                             or HDMI sink based on EDID-HDMI VSDB.
*                       Fixed system flow to avoid RX Buffer Overflow during
*                              transition.
*                       Code Clean-Up on comments and 80 Characted per line.
*                       Improve audio configuration during Pass-through mode.
*                       Disable HDMI RX Video Stream when EnableColorBar API
*                              is called.
*                       Added TX Bridge Overflow and TX Bridge Underflow
* 3.03  YB     08/14/18 Clubbing Repeater specific code under the
*                       'ENABLE_HDCP_REPEATER' macro.
*       EB     09/21/18 Added new API ToggleHdmiRxHpd and SetHdmiRxHpd
*                       Updated CloneTxEdid API
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include "xhdmi_menu.h"
#include "xhdmi_hdcp_keys_table.h"
#include "xhdmi_example.h"

/***************** Macros (Inline Functions) Definitions *********************/
/* These macro values need to changed whenever there is a change in version */
#define APP_MAJ_VERSION 5
#define APP_MIN_VERSION 2

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int I2cMux(void);
int I2cClk(u32 InFreq, u32 OutFreq);

#if defined (__arm__) && (!defined(ARMR5))
int OnBoardSi5324Init(void);
#endif

void Info(void);

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
void EnableColorBar(XVphy *VphyPtr, XV_HdmiTxSs *HdmiTxSsPtr,
					XVidC_VideoMode VideoMode,
					XVidC_ColorFormat ColorFormat,
					XVidC_ColorDepth Bpc);
void UpdateFrameRate(XVphy *VphyPtr, XV_HdmiTxSs *HdmiTxSsPtr,
					 XVidC_FrameRate FrameRate);
void UpdateColorFormat(XVphy *VphyPtr, XV_HdmiTxSs *HdmiTxSsPtr,
					   XVidC_ColorFormat ColorFormat);
void UpdateColorDepth(XVphy *VphyPtr, XV_HdmiTxSs *HdmiTxSsPtr,
					  XVidC_ColorDepth ColorDepth);
void CloneTxEdid(void);
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#ifdef XPAR_XV_TPG_NUM_INSTANCES
void XV_ConfigTpg(XV_tpg *InstancePtr);
void ResetTpg(void);
#endif
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
void TxConnectCallback(void *CallbackRef);
void TxToggleCallback(void *CallbackRef);
void TxVsCallback(void *CallbackRef);
void TxBrdgUnlockedCallback(void *CallbackRef);
void TxStreamUpCallback(void *CallbackRef);
void TxStreamDownCallback(void *CallbackRef);
void VphyHdmiTxInitCallback(void *CallbackRef);
void VphyHdmiTxReadyCallback(void *CallbackRef);
void TxInfoFrameReset(void);
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
void RxConnectCallback(void *CallbackRef);
void RxBrdgOverflowCallback(void *CallbackRef);
void RxStreamUpCallback(void *CallbackRef);
void RxStreamDownCallback(void *CallbackRef);
void VphyHdmiRxInitCallback(void *CallbackRef);
void VphyHdmiRxReadyCallback(void *CallbackRef);
#endif
void VphyErrorCallback(void *CallbackRef);
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
void VphyPllLayoutErrorCallback(void *CallbackRef);
#endif
void VphyProcessError(void);

/* Needed for ZCU106 RevB */
#if defined (ARMR5) || (__aarch64__)
void Disable_TMDS181_HPD_passthrough();
#define TMDS181_ADDR    0x5c
#endif

/************************* Variable Definitions *****************************/
/* VPHY structure */
XVphy              Vphy;
u8                 VphyErrorFlag;
u8                 VphyPllLayoutErrorFlag;

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/* HDMI TX SS structure */
XV_HdmiTxSs        HdmiTxSs;
XV_HdmiTxSs_Config *XV_HdmiTxSs_ConfigPtr;

#ifdef XPAR_XV_TPG_NUM_INSTANCES
/* Test Pattern Generator Structure */
XV_tpg             Tpg;
XV_tpg_Config      *Tpg_ConfigPtr;
/* Video Pattern */
XTpg_PatternId     Pattern;
#endif

#ifdef USE_HDMI_AUDGEN
/* Audio Generator structure */
XhdmiAudioGen_t    AudioGen;
#endif

XHdmiC_Aux         AuxFifo[AUXFIFOSIZE];
u8                 AuxFifoStartIndex;
u8                 AuxFifoEndIndex;
u8                 AuxFifoCount;
u8				   AuxFifoOvrFlowCnt;

/* Flag indicates whether the TX Cable is connected or not */
u8                 TxCableConnect = (FALSE);

/* TX busy flag. This flag is set while the TX is initialized*/
u8                 TxBusy = (TRUE);
/* TX restart colorbar. This flag is set when the TX cable
 * has been reconnected and the TX colorbar was showing.
 */
u8                 TxRestartColorbar = (FALSE);
/* TX Stream Up Status Flag, Avoiding Race condition */
u8                 IsStreamUp = (FALSE);
u64                TxLineRate = 0;
/* Sink Ready: Become true when the EDID parsing is completed
 * upon cable connect */
u8                 SinkReady = (FALSE);

/* Variable for pass-through operation */
u8                 AuxFifoStartFlag = (FALSE);
#endif

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/* HDMI RX SS structure */
XV_HdmiRxSs        HdmiRxSs;
XV_HdmiRxSs_Config *XV_HdmiRxSs_ConfigPtr;
#endif

#ifdef XPAR_XGPIO_NUM_INSTANCES
XGpio              Gpio_Tpg_resetn;
XGpio_Config       *Gpio_Tpg_resetn_ConfigPtr;
#endif

#ifdef XPAR_XI2SRX_NUM_INSTANCES

u8 tx_is_up = 0;
u8 rx_is_up = 0;
void i2s_init (void);
void aud_clk_reset(void);
void i2s_audio(u32 tmds, u8 ratio_tmds, u8 tx_up, u8 rx_up,
		       XV_HdmiTxSs *HdmiTxSsPtr, XV_HdmiRxSs *HdmiRxSsPtr,
			   XhdmiAudioGen_t *AudioGen);
#define TX_ACR_ADDR XPAR_AUDIO_SS_0_HDMI_ACR_CTRL_BASEADDR
#endif

#ifdef VIDEO_FRAME_CRC_EN
Video_CRC_Config   VidFrameCRC;
#endif

#ifdef USE_HDCP
XHdcp_Repeater     HdcpRepeater;
#endif

/* Interrupt Controller Data Structure */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
static XScuGic     Intc;
#else
static XIntc       Intc;
#endif

/* HDMI Application Menu: Data Structure */
XHdmi_Menu         HdmiMenu;

/**< Demo mode IsPassThrough
 * (TRUE)  = Pass-through mode
 * (FALSE) = Color Bar mode
 *  */
u8                 IsPassThrough = (FALSE);
u8                 StartTxAfterRxFlag = (FALSE);

/* General HDMI Application Variable Declaration (Scratch Pad) */

/************************** Function Definitions *****************************/

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
/*****************************************************************************/
/**
*
* This function checks the system PLL, whether it's bonded or non-bonded
* @return None.
*
* @note   None.
*
******************************************************************************/
u8 PLLBondedCheck(void)
{
	/* The colorbar can only be displayed when the GT is not bonded. */
	if (XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)) {
		xil_printf("\r\nThe GT TX and RX are bonded and clocked by "
				   "the RX clock.\r\n");
		xil_printf("Please select independent PLL layout to enable "
				   "TX only mode.\r\n");
	}
	return (XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1));
}
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function clones the EDID of the connected sink device to the HDMI RX
* @return None.
*
* @note   None.
*
******************************************************************************/
void CloneTxEdid(void)
{
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
    u8 Buffer[256];
    u32 Status;

    /* Read TX edid */
    Status = XV_HdmiTxSs_ReadEdid(&HdmiTxSs, (u8*)&Buffer);

    /* Check if read was successful */
    if (Status == (XST_SUCCESS)) {
        /* Load new EDID */
        XV_HdmiRxSs_LoadEdid(&HdmiRxSs, (u8*)&Buffer, sizeof(Buffer));

        /* Toggle HPD after loading new HPD */
        ToggleHdmiRxHpd(&Vphy, &HdmiRxSs);

        xil_printf("\r\n");
        xil_printf("Successfully cloned EDID and toggled HPD.\r\n");
    }
#else
    xil_printf("\r\nEdid Cloning no possible with HDMI RX SS.\r\n");
#endif
}
/*****************************************************************************/
/**
*
* This function generates video pattern.
*
* @param  IsPassThrough specifies either pass-through or colorbar mode.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_ConfigTpg(XV_tpg *InstancePtr)
{
	XV_tpg                *pTpg = InstancePtr;
	XVidC_VideoStream     *HdmiTxSsVidStreamPtr;
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	XHdmiC_AVI_InfoFrame  *AVIInfoFramePtr;
#endif

	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(&HdmiTxSs);

	u32 width, height;
	XVidC_VideoMode VideoMode;
	VideoMode = HdmiTxSsVidStreamPtr->VmId;

	/* If Color bar, the 480i/576i HActive need to be divided by 2 */
	/* 1440x480i/1440x576i --> 720x480i/720x576i */
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	if(!IsPassThrough) {
#endif
		/* NTSC/PAL Support */
		if ((VideoMode == XVIDC_VM_1440x480_60_I) ||
				(VideoMode == XVIDC_VM_1440x576_50_I) ) {

			width  = HdmiTxSsVidStreamPtr->Timing.HActive/2;
			height = HdmiTxSsVidStreamPtr->Timing.VActive;
		} else {
			/* If not NTSC/PAL, the HActive,
			 * and VActive remain as it is
			 */
			width  = HdmiTxSsVidStreamPtr->Timing.HActive;
			height = HdmiTxSsVidStreamPtr->Timing.VActive;
		}
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	} else {
		/* Get the Current HDMI RX AVI Info Frame
		 * As this design is pass-through, is save to read the incoming
		 * stream Info frame
		 */
		AVIInfoFramePtr = XV_HdmiRxSs_GetAviInfoframe(&HdmiRxSs);

		/* If the incoming (HDMI RX) info frame (pixel repetition) = 2
		 * The 480i/576i HActive need to be divided by 2
		 */
		if (AVIInfoFramePtr->PixelRepetition ==
					XHDMIC_PIXEL_REPETITION_FACTOR_2){
			width  = HdmiTxSsVidStreamPtr->Timing.HActive/2;
			height = HdmiTxSsVidStreamPtr->Timing.VActive;
		} else {
			/* If Pixel Repetition != 2, the HActive, and VActive
			 * remain as it is */
			width  = HdmiTxSsVidStreamPtr->Timing.HActive;
			height = HdmiTxSsVidStreamPtr->Timing.VActive;
		}
	}
#endif

	/* Work around:
	 * Can't set TPG to pass-through mode if the width or height = 0
	 */
	if (!((width == 0 || height == 0) && IsPassThrough)) {
		/* Stop TPG */
		XV_tpg_DisableAutoRestart(pTpg);

		XV_tpg_Set_height(pTpg, height);
		XV_tpg_Set_width(pTpg,  width);
		XV_tpg_Set_colorFormat(pTpg,
				HdmiTxSsVidStreamPtr->ColorFormatId);
		XV_tpg_Set_bckgndId(pTpg, Pattern);
		XV_tpg_Set_ovrlayId(pTpg, 0);

		XV_tpg_Set_enableInput(pTpg, IsPassThrough);

		if (IsPassThrough) {
			XV_tpg_Set_passthruStartX(pTpg,0);
			XV_tpg_Set_passthruStartY(pTpg,0);
			XV_tpg_Set_passthruEndX(pTpg,width);
			XV_tpg_Set_passthruEndY(pTpg,height);
		}

		/* Start TPG */
		XV_tpg_EnableAutoRestart(pTpg);
		XV_tpg_Start(pTpg);
	}
}

void ResetTpg(void)
{
	XGpio_SetDataDirection(&Gpio_Tpg_resetn, 1, 0);
	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, 0);
	usleep(1000);

	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, 1);
	usleep(1000);
}

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function resets the AuxFifo.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ResetAuxFifo(void)
{
	AuxFifoStartFlag   = (FALSE);
	AuxFifoStartIndex  = 0;
	AuxFifoEndIndex    = 0;
	AuxFifoCount	   = 0;
	AuxFifoOvrFlowCnt  = 0;
}
#endif

/*****************************************************************************/
/**
*
* This function checks the TX Busy flag, and returns TRUE with prompt. and
* FALSE
* @return TRUE/FALSE.
*
* @note   None.
*
******************************************************************************/
u8 CheckTxBusy (void)
{
	if (TxBusy) {
		xil_printf("Either TX still on transition to a new video"
		            " format\r\nor the TX cable is not connected\r\n");
	}
	return (TxBusy);
}

/* Send Vendor Specific InfoFrame */
void SendVSInfoframe(XV_HdmiTxSs *HdmiTxSsPtr)
{
	XHdmiC_VSIF *VSIFPtr;
	VSIFPtr = XV_HdmiTxSs_GetVSIF(HdmiTxSsPtr);

	XHdmiC_Aux Aux;

	(void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));
	(void)memset((void *)&Aux, 0, sizeof(XHdmiC_Aux));

	VSIFPtr->Version = 0x1;
	VSIFPtr->IEEE_ID = 0xC03;

	if (XVidC_IsStream3D(&(HdmiTxSsPtr->HdmiTxPtr->Stream.Video))) {
		VSIFPtr->Format = XHDMIC_VSIF_VF_3D;
		VSIFPtr->Info_3D.Stream =
				HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Info_3D;
		VSIFPtr->Info_3D.MetaData.IsPresent = FALSE;
	} else if (HdmiTxSsPtr->HdmiTxPtr->Stream.Video.VmId ==
					   XVIDC_VM_3840x2160_24_P ||
			   HdmiTxSsPtr->HdmiTxPtr->Stream.Video.VmId ==
					   XVIDC_VM_3840x2160_25_P ||
			   HdmiTxSsPtr->HdmiTxPtr->Stream.Video.VmId ==
					   XVIDC_VM_3840x2160_30_P ||
			   HdmiTxSsPtr->HdmiTxPtr->Stream.Video.VmId ==
					   XVIDC_VM_4096x2160_24_P) {
		VSIFPtr->Format = XHDMIC_VSIF_VF_EXTRES;

		/* Set HDMI VIC */
		switch(HdmiTxSsPtr->HdmiTxPtr->Stream.Video.VmId) {
			case XVIDC_VM_4096x2160_24_P :
				VSIFPtr->HDMI_VIC = 4;
				break;
			case XVIDC_VM_3840x2160_24_P :
				VSIFPtr->HDMI_VIC = 3;
				break;
			case XVIDC_VM_3840x2160_25_P :
				VSIFPtr->HDMI_VIC = 2;
				break;
			case XVIDC_VM_3840x2160_30_P :
				VSIFPtr->HDMI_VIC = 1;
				break;
			default :
				break;
		}
	} else {
		VSIFPtr->Format = XHDMIC_VSIF_VF_NOINFO;
	}

	Aux = XV_HdmiC_VSIF_GeneratePacket(VSIFPtr);

	XV_HdmiTxSs_SendGenericAuxInfoframe(HdmiTxSsPtr, &Aux);
}

/* Send out all the InfoFrames in the AuxFifo during PassThrough mode
 * or send out AVI, Audio, Vendor Specific InfoFrames.
 */
void SendInfoframe(XV_HdmiTxSs *HdmiTxSsPtr)
{
	u32 Status;
	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
	XHdmiC_AudioInfoFrame *AudioInfoFramePtr;
	XHdmiC_VSIF *VSIFPtr;

	AviInfoFramePtr = XV_HdmiTxSs_GetAviInfoframe(HdmiTxSsPtr);
	AudioInfoFramePtr = XV_HdmiTxSs_GetAudioInfoframe(HdmiTxSsPtr);
	VSIFPtr = XV_HdmiTxSs_GetVSIF(HdmiTxSsPtr);
	Status = (XST_FAILURE);

	if (!IsPassThrough) {
		/* Generate Aux from the current TX InfoFrame */
		AuxFifo[0] = XV_HdmiC_AVIIF_GeneratePacket(AviInfoFramePtr);
		XV_HdmiTxSs_SendGenericAuxInfoframe(HdmiTxSsPtr,
						    &(AuxFifo[0]));

		/* GCP does not need to be sent out because GCP packets on
		 * the TX side is handled by the HDMI TX core fully.
		 */

		AuxFifo[0] =
			XV_HdmiC_AudioIF_GeneratePacket(AudioInfoFramePtr);
		XV_HdmiTxSs_SendGenericAuxInfoframe(HdmiTxSsPtr,
						    &(AuxFifo[0]));
		SendVSInfoframe(HdmiTxSsPtr);
	} else {
		if(AuxFifoCount > AUXFIFOSIZE) {
			AuxFifoStartIndex = AuxFifoEndIndex;
		}

		/* If PassThrough, update TX's InfoFrame Data Structure
		 * from AuxFiFO
		 */
		while (AuxFifoStartIndex != AuxFifoEndIndex) {
			if(AuxFifo[AuxFifoStartIndex].Header.Byte[0] ==
							AUX_VSIF_TYPE) {
				/* Reset Vendor Specific InfoFrame */
				(void)memset((void *)VSIFPtr,
					     0,
					     sizeof(XHdmiC_VSIF));

				XV_HdmiC_VSIF_ParsePacket
						(&AuxFifo[AuxFifoStartIndex],
						 VSIFPtr);
			} else if(AuxFifo[AuxFifoStartIndex].Header.Byte[0] ==
					AUX_AVI_INFOFRAME_TYPE) {
				/* Reset Avi InfoFrame */
				(void)memset((void *)AviInfoFramePtr, 0,
						sizeof(XHdmiC_AVI_InfoFrame));

				XV_HdmiC_ParseAVIInfoFrame
						(&AuxFifo[AuxFifoStartIndex],
						 AviInfoFramePtr);

				/* Modify the TX's InfoFrame here before
				 * sending out
				 * E.g:
				 *     AviInfoFramePtr->VIC = 107;
				 */

				/* Generate Aux from the modified TX's
				 * InfoFrame before sending out
				 * E.g:
				 * 	AuxFifo[AuxFifoStartIndex] =
				 * 		XV_HdmiC_AVIIF_GeneratePacket
				 *			     (AviInfoFramePtr);
				 */
			} else if(AuxFifo[AuxFifoStartIndex].Header.Byte[0] ==
					AUX_AUDIO_INFOFRAME_TYPE) {
				/* Reset Audio InfoFrame */
				(void)memset((void *)AudioInfoFramePtr, 0,
						sizeof(XHdmiC_AudioInfoFrame));

				XV_HdmiC_ParseAudioInfoFrame
						(&AuxFifo[AuxFifoStartIndex],
						 AudioInfoFramePtr);

				/* Modify the TX's InfoFrame here
				 * before sending out
				 * E.g:
				 * 	AudioInfoFramePtr->ChannelCount =
				 * 		XHDMIC_AUDIO_CHANNEL_COUNT_3;
				 */

				/* Generate Aux from the modified TX's
				 * InfoFrame beforesending out
				 * E.g :
				 * 	AuxFifo[AuxFifoStartIndex] =
				 * 		XV_HdmiC_AudioIF_GeneratePacket
				 *			   (AudioInfoFramePtr);
				 */
			}

			Status = XV_HdmiTxSs_SendGenericAuxInfoframe
					(HdmiTxSsPtr,
					 &(AuxFifo[AuxFifoStartIndex]));

			/* If TX Core's hardware Aux FIFO is full,
			 * from the while loop, retry during the
			 * next main while iteration.
			 */
			if (Status != (XST_SUCCESS)) {
				xil_printf(ANSI_COLOR_RED
				           "HW Aux Full"
					   ANSI_COLOR_RESET
				           "\r\n");
			}

			if(AuxFifoStartIndex < (AUXFIFOSIZE - 1)) {
				AuxFifoStartIndex++;
			} else {
				AuxFifoStartIndex = 0;
			}
		}

		AuxFifoCount = 0;
	}
}

void TxInfoFrameReset(void)
{
	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
	XHdmiC_AudioInfoFrame *AudioInfoFramePtr;

	AviInfoFramePtr = XV_HdmiTxSs_GetAviInfoframe(&HdmiTxSs);
	AudioInfoFramePtr = XV_HdmiTxSs_GetAudioInfoframe(&HdmiTxSs);

	/* Reset Avi InfoFrame */
	(void)memset((void *)AviInfoFramePtr,
	             0,
		     sizeof(XHdmiC_AVI_InfoFrame));

	/* Reset Audio InfoFrame */
	(void)memset((void *)AudioInfoFramePtr,
	             0,
		     sizeof(XHdmiC_AudioInfoFrame));

	AviInfoFramePtr->Version = 2;
	AviInfoFramePtr->ColorSpace = XHDMIC_COLORSPACE_RGB;
	AviInfoFramePtr->VIC = 16;
	AviInfoFramePtr->PicAspectRatio = XHDMIC_PIC_ASPECT_RATIO_16_9;
	/* AudioInfoFramePtr->ChannelCount = XHDMIC_AUDIO_CHANNEL_COUNT_3; */
}
#endif
#if (!defined XPS_BOARD_ZCU104)
/*****************************************************************************/
/**
*
* This function setup SI5324 clock generator over IIC.
*
* @param  None.
*
* @return The number of bytes sent.
*
* @note   None.
*
******************************************************************************/
int I2cMux(void)
{
	u8 Buffer;
	int Status;

	/* Select SI5324 clock generator */
#if defined (ARMR5) || (__aarch64__)
	Buffer = 0x10;
	Status = XIicPs_MasterSendPolled(&Ps_Iic1, (u8 *)&Buffer, 1, 0);
#else
	Buffer = 0x80;
	Status = XIic_Send((XPAR_IIC_0_BASEADDR), (I2C_MUX_ADDR),
					   (u8 *)&Buffer, 1, (XIIC_STOP));
#endif

	return Status;
}
#endif

/*****************************************************************************/
/**
*
* This function setup SI5324 clock generator either in free or locked mode.
*
* @param  Index specifies an index for selecting mode frequency.
* @param  Mode specifies either free or locked mode.
*
* @return
*   - Zero if error in programming external clock.
*   - One if programmed external clock.
*
* @note   None.
*
******************************************************************************/
int I2cClk(u32 InFreq, u32 OutFreq)
{
	int Status;

#if (!defined XPS_BOARD_ZCU104)
	/* Free running mode */
	if (InFreq == 0) {

		Status = Si5324_SetClock((XPAR_IIC_0_BASEADDR),
					 (I2C_CLK_ADDR),
			                 (SI5324_CLKSRC_XTAL),
					 (SI5324_XTAL_FREQ),
					 OutFreq);
		if (Status != (SI5324_SUCCESS)) {
			xil_printf("Error programming SI5324\r\n");
			return 0;
		}
	}

	/* Locked mode */
	else {
		Status = Si5324_SetClock((XPAR_IIC_0_BASEADDR),
					 (I2C_CLK_ADDR),
					 (SI5324_CLKSRC_CLK1),
					 InFreq,
					 OutFreq);

		if (Status != (SI5324_SUCCESS)) {
			xil_printf("Error programming SI5324\r\n");
			return 0;
		}
	}
#else
	/* Reset I2C controller before issuing new transaction.
	 * This is required torecover the IIC controller in case a previous
	 * transaction is pending.
	 */
	XIic_WriteReg(XPAR_IIC_0_BASEADDR, XIIC_RESETR_OFFSET,
				  XIIC_RESET_MASK);

	/* Free running mode */
	if (InFreq == 0) {
		Status = IDT_8T49N24x_SetClock((XPAR_IIC_0_BASEADDR),
					       (I2C_CLK_ADDR),
					       (IDT_8T49N24X_XTAL_FREQ),
					       OutFreq,
					       TRUE);

		if (Status != (XST_SUCCESS)) {
			print("Error programming IDT_8T49N241\r\n");
			return 0;
		}
	}

	/* Locked mode */
	else {
		Status = IDT_8T49N24x_SetClock((XPAR_IIC_0_BASEADDR),
					       (I2C_CLK_ADDR),
					       InFreq,
					       OutFreq,
					       FALSE);

		if (Status != (XST_SUCCESS)) {
			print("Error programming SI5324\n\r");
			return 0;
		}
	}
#endif
	return 1;
}

#if (defined (ARMR5) || (__aarch64__)) && (!defined XPS_BOARD_ZCU104)

int I2cMux_Ps(void)
{
	u8 Buffer;
	int Status;

	/* Select SI5324 clock generator */
	Buffer = 0x10;
	Status = XIicPs_MasterSendPolled(&Ps_Iic1,
	                                 (u8 *)&Buffer,
					 1,
					 I2C_MUX_ADDR);

	return Status;
}

int I2cClk_Ps(u32 InFreq, u32 OutFreq)
{
	int Status;

	/* Free running mode */
	if (InFreq == 0) {

		Status = Si5324_SetClock_Ps(&Ps_Iic1,
						(SI5328_I2C_ADDR),
						(SI5324_CLKSRC_XTAL),
						(SI5324_XTAL_FREQ),
						OutFreq);

		if (Status != (SI5324_SUCCESS)) {
			print("Error programming SI5324\r\n");
			return 0;
		}
	}

	/* Locked mode */
	else {
		Status = Si5324_SetClock_Ps(&Ps_Iic1,
						(SI5328_I2C_ADDR),
						(SI5324_CLKSRC_CLK1),
						InFreq,
						OutFreq);

		if (Status != (SI5324_SUCCESS)) {
			print("Error programming SI5324\r\n");
			return 0;
		}
	}

	return 1;
}

void Disable_TMDS181_HPD_passthrough()
{
	u8 Buffer[2];

	/* Disable TMDS181 HPD pass through */
	Buffer[1] = 0xF1;
	Buffer[0] = 0x0A;
	XIic_Send((XPAR_IIC_0_BASEADDR), (TMDS181_ADDR),
			  (u8 *)&Buffer, 2, (XIIC_STOP));

}
#endif

#if defined (__arm__) && (!defined(ARMR5))
/*****************************************************************************/
/**
*
* This function initializes the ZC706 on-board SI5324 clock generator over IIC.
* CLKOUT1 is set to 125 MHz
*
* @param  None.
*
* @return The number of bytes sent.
*
* @note   None.
*
******************************************************************************/
int OnBoardSi5324Init(void)
{
	u8 Buffer;
	int Status;

	/* Select SI5324 clock generator */
	Buffer = 0x10;
	XIic_Send((XPAR_IIC_1_BASEADDR), (I2C_MUX_ADDR),
			  (u8 *)&Buffer, 1, (XIIC_STOP));

	/* Initialize Si5324 */
	Si5324_Init(XPAR_IIC_1_BASEADDR, I2C_CLK_ADDR);

	/* Program Output Frequency Si5324 */
	Status = Si5324_SetClock((XPAR_IIC_1_BASEADDR),
	                         (I2C_CLK_ADDR),
				 (SI5324_CLKSRC_XTAL),
				 (SI5324_XTAL_FREQ), (u32)125000000);

	if (Status != (SI5324_SUCCESS)) {
		xil_printf("Error programming On-Board SI5324\r\n");
		return 0;
	}

	return Status;
}
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function reports the stream mode
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ReportStreamMode(XV_HdmiTxSs *HdmiTxSsPtr, u8 IsPassThrough)
{
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(HdmiTxSsPtr);

	if (IsPassThrough) {
		xil_printf("--------\r\nPass-Through :\r\n");
	} else {
		xil_printf("--------\r\nColorbar :\r\n");
	}

	XVidC_ReportStreamInfo(HdmiTxSsVidStreamPtr);
	xil_printf("--------\r\n");
}
#endif

/*****************************************************************************/
/**
*
* This function outputs the video timing , Audio, Link Status, HDMI RX state of
* HDMI RX core. In addition, it also prints information about HDMI TX, and
* HDMI GT cores.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void Info(void)
{
	u32 Data;
	xil_printf("\r\n-----\r\n");
	xil_printf("Info\r\n");
	xil_printf("-----\r\n\r\n");

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	XV_HdmiTxSs_ReportInfo(&HdmiTxSs);
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	XV_HdmiRxSs_ReportInfo(&HdmiRxSs);
#endif
#ifdef VIDEO_FRAME_CRC_EN
	XVidFrameCrc_Report();
#endif

	/* GT */
	xil_printf("------------\r\n");
	xil_printf("HDMI PHY\r\n");
	xil_printf("------------\r\n");
	Data = XVphy_GetVersion(&Vphy);
	xil_printf("  VPhy version : %02d.%02d (%04x)\r\n",
			   ((Data >> 24) & 0xFF),
			   ((Data >> 16) & 0xFF),
			   (Data & 0xFFFF));
	xil_printf("\r\n");
	xil_printf("GT status\r\n");
	xil_printf("---------\r\n");
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	xil_printf("TX reference clock frequency: %0d Hz\r\n",
			   XVphy_ClkDetGetRefClkFreqHz(&Vphy, XVPHY_DIR_TX));
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	xil_printf("RX reference clock frequency: %0d Hz\r\n",
			   XVphy_ClkDetGetRefClkFreqHz(&Vphy, XVPHY_DIR_RX));
	if(Vphy.Config.DruIsPresent == (TRUE)) {
		xil_printf("DRU reference clock frequency: %0d Hz\r\n",
				   XVphy_DruGetRefClkFreqHz(&Vphy));
	}
#endif
	XVphy_HdmiDebugInfo(&Vphy, 0, XVPHY_CHANNEL_ID_CH1);

#if(defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && \
		defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES))
	xil_printf("------------\r\n");
	xil_printf("Debugging\r\n");
	xil_printf("------------\r\n");
#endif
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && \
		defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
	xil_printf("AuxFifo Overflow Count: %d\r\n", AuxFifoOvrFlowCnt);
#endif
}

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called when a TX connect event has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxConnectCallback(void *CallbackRef) {
	XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;
	if(HdmiTxSsPtr->IsStreamConnected == (FALSE)) {
		/* TX Cable is disconnected */
		TxCableConnect = (FALSE);

		if (IsPassThrough) {
			/* If the system in the Pass-through
			 * reset the AUX FIFO
			 */
			ResetAuxFifo();
			/* Clearing the Restarting the TX after RX up flag */
			StartTxAfterRxFlag = (FALSE);
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			/* Mute/Stop the HDMI RX SS Output
			 * Enable only when the downstream is ready
			 * (TPG & HDMI TX)
			 */
			XV_HdmiRxSs_VRST(&HdmiRxSs,TRUE);
#endif
		}

		/* Cable is disconnected, don't restart colorbar */
		TxRestartColorbar = (FALSE);
		/* Cable is disconnected, don't allow any TX operation */
		TxBusy = (TRUE);

		XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_TX, (FALSE));

#ifdef USE_HDCP
		/* Call HDCP disconnect callback */
		XHdcp_StreamDisconnectCallback(&HdcpRepeater);
#endif
	} else {
		/* Set TX Cable Connect Flag to (TRUE) as the cable is
		 * connected
		 */
		TxCableConnect = (TRUE);

		/* Set Flag when the cable is connected
		 * this call back take in to account two scneario
		 * cable connect and cable disconnect
		 * Stable RX stream is available
		 */
		if (IsPassThrough) {
			/* Restart Stream */
			StartTxAfterRxFlag = (TRUE);
		}
		else { /* No stable RX Stream available, Only TX Cable Connect,
				* Hence TX-Only mode or Color bar
				*/
#if(LOOPBACK_MODE_EN != 1)
			TxRestartColorbar = (TRUE);
			TxBusy = (FALSE);
#endif
		}

		XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_TX, (TRUE));

		/* Initialize EDID App during cable connect */
		EDIDConnectInit(&EdidHdmi20_t);
		/* Read the EDID and the SCDC */
		EdidScdcCheck(HdmiTxSsPtr, &EdidHdmi20_t);

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
		/* When the GT TX and RX are coupled, then start the TXPLL */
		if (XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)) {
			/* Start TX MMCM */
			XVphy_MmcmStart(&Vphy, 0, XVPHY_DIR_TX);
			usleep(10000);
			/* GT TX reset. */
			XVphy_ResetGtTxRx(&Vphy, 0, XVPHY_CHANNEL_ID_CHA,
							  XVPHY_DIR_TX, FALSE);
		}
#endif

#ifdef USE_HDCP
		/* Call HDCP connect callback */
		XHdcp_StreamConnectCallback(&HdcpRepeater);
#endif
	}
}

/*****************************************************************************/
/**
*
* This function is called when a TX toggle event has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxToggleCallback(void *CallbackRef) {
	XV_HdmiTxSs_StreamStart(&HdmiTxSs);

#ifdef USE_HDCP
	/* Call HDCP connect callback */
	XHdcp_Authenticate(&HdcpRepeater);
#endif
}

/*****************************************************************************/
/**
*
* This function is called when the GT TX reference input clock has changed.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VphyHdmiTxInitCallback(void *CallbackRef) {
	XV_HdmiTxSs_RefClockChangeInit(&HdmiTxSs);
}

/*****************************************************************************/
/**
*
* This function is called when the GT TX has been initialized
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VphyHdmiTxReadyCallback(void *CallbackRef) {
}
#endif

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is used to toggle RX's HPD Line
*
* @param  VphyPtr is a pointer to the VPHY instance.
* @param  HdmiRxSsPtr is a pointer to the HDMI RX Subsystem instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void ToggleHdmiRxHpd(XVphy *VphyPtr, XV_HdmiRxSs *HdmiRxSsPtr) {
	SetHdmiRxHpd(VphyPtr, HdmiRxSsPtr, FALSE);
	/* Wait 500 ms */
	usleep(500000);
	SetHdmiRxHpd(VphyPtr, HdmiRxSsPtr, TRUE);
}

/*****************************************************************************/
/**
*
* This function sets the HPD on the HDMI RXSS.
*
* @param  VphyPtr is a pointer to the VPHY instance.
* @param  HdmiRxSsPtr is a pointer to the HDMI RX Subsystem instance.
* @param  Value is a flag used to set the HPD.
*   - TRUE drives HPD high
*   - FALSE drives HPD low
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void SetHdmiRxHpd(XVphy *VphyPtr, XV_HdmiRxSs *HdmiRxSsPtr, u8 Hpd) {
	if (Hpd == TRUE) {
		XV_HdmiRxSs_SetHpd(HdmiRxSsPtr, Hpd);
		XVphy_IBufDsEnable(VphyPtr, 0, XVPHY_DIR_RX, (TRUE));
	} else {
		XVphy_MmcmPowerDown(VphyPtr, 0, XVPHY_DIR_RX, FALSE);
		XVphy_Clkout1OBufTdsEnable(VphyPtr, XVPHY_DIR_RX, (FALSE));
		XVphy_IBufDsEnable(VphyPtr, 0, XVPHY_DIR_RX, (FALSE));
		XV_HdmiRxSs_SetHpd(HdmiRxSsPtr, Hpd);
	}
}

/*****************************************************************************/
/**
*
* This function is called when a RX connect event has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxConnectCallback(void *CallbackRef) {
	XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

	/* RX cable is disconnected */
	if(HdmiRxSsPtr->IsStreamConnected == (FALSE)) {
		/* Clear GT RX TMDS clock ratio */
		Vphy.HdmiRxTmdsClockRatio = 0;

#if(LOOPBACK_MODE_EN != 1)
		/* Check for Pass-through:
		 * Doesnt require to restart colorbar
		 * if the system is in colorbar mode
		 */
		if (IsPassThrough) {
			/* Clear pass-through flag
			 * as the system is in TX-Only
			 * mode
			 */
			IsPassThrough = (FALSE);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			/* Start colorbar with same
			 * video parameters
			 */
			if (TxCableConnect) {
				TxRestartColorbar = (TRUE);
				TxBusy = (FALSE);
			} else {
				TxRestartColorbar = (FALSE);
				TxBusy = (TRUE);
			}
#endif
		}
#endif

		XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (FALSE));

#ifdef USE_HDCP
		/* Call HDCP disconnect callback */
		XHdcp_StreamDisconnectCallback(&HdcpRepeater);
#endif
	} else {
		XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (TRUE));

#ifdef USE_HDCP
		/* Call HDCP connect callback */
		XHdcp_StreamConnectCallback(&HdcpRepeater);
#endif
	}

}

/*****************************************************************************/
/**
*
* This function is called when the GT RX reference input clock has changed.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VphyHdmiRxInitCallback(void *CallbackRef) {
	XVphy *VphyPtr = (XVphy *)CallbackRef;

	XV_HdmiRxSs_RefClockChangeInit(&HdmiRxSs);
	VphyPtr->HdmiRxTmdsClockRatio = HdmiRxSs.TMDSClockRatio;
}

/*****************************************************************************/
/**
*
* This function is called when the GT RX has been initialized.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VphyHdmiRxReadyCallback(void *CallbackRef) {
	XVphy *VphyPtr = (XVphy *)CallbackRef;
	XVphy_PllType RxPllType;

	/* Reset the menu to main */
	XHdmi_MenuReset(&HdmiMenu);

	RxPllType = XVphy_GetPllType(VphyPtr,
	                             0,
				     XVPHY_DIR_RX,
				     XVPHY_CHANNEL_ID_CH1);

	if (!(RxPllType == XVPHY_PLL_TYPE_CPLL)) {
		XV_HdmiRxSs_SetStream(&HdmiRxSs,
		        VphyPtr->HdmiRxRefClkHz,
			(XVphy_GetLineRateHz(&Vphy,
				             0,
					     XVPHY_CHANNEL_ID_CMN0)/1000000));

	} else {
		XV_HdmiRxSs_SetStream(&HdmiRxSs, VphyPtr->HdmiRxRefClkHz,
			 (XVphy_GetLineRateHz(&Vphy,
			                      0,
					      XVPHY_CHANNEL_ID_CH1)/1000000));
	}
}
#endif

/*****************************************************************************/
/**
*
* This function is called whenever an error condition in VPHY occurs.
* This will fill the FIFO of VPHY error events which will be processed outside
* the ISR.
*
* @param  CallbackRef is the VPHY instance pointer
* @param  ErrIrqType is the VPHY error type
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VphyErrorCallback(void *CallbackRef) {
	VphyErrorFlag = TRUE;
}

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
/*****************************************************************************/
/**
*
* This function is called whenever a GTXE2 PLL layout error condition in VPHY
* occurs. This function can be used automatically switch the PLL layout.
* This will set the VPHY PLL Layout error flag to TRUE.
*
* @param  CallbackRef is the VPHY instance pointer
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VphyPllLayoutErrorCallback(void *CallbackRef) {
	VphyPllLayoutErrorFlag = TRUE;
}
#endif

/*****************************************************************************/
/**
*
* This function is called in the application to process the pending
* VPHY errors
*
* @param  None.
*
* @return None.
*
* @note   This function can be expanded to perform necessary actions depending
*		on the error type. For example, XVPHY_ERR_PLL_LAYOUT can be
*		used to automatically switch in and out of bonded mode for
*               GTXE2 devices
*
******************************************************************************/
void VphyProcessError(void) {
	if (VphyErrorFlag == TRUE) {
		xil_printf(ANSI_COLOR_RED "VPHY Error: See log for details"
				   ANSI_COLOR_RESET "\r\n");
	}
	/* Clear Flag */
	VphyErrorFlag = FALSE;

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
	if (VphyPllLayoutErrorFlag == TRUE) {
		xil_printf(ANSI_COLOR_RED "VPHY Error: Try changing to "
				 "another PLL Layout" ANSI_COLOR_RESET "\r\n");
	}
	/* Clear Flag */
	VphyPllLayoutErrorFlag = FALSE;
#endif
}

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES

/*****************************************************************************/
/**
*
* This function is called when a TX vsync has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxVsCallback(void *CallbackRef) {
	/* When the TX stream is confirmed to have started, start accepting
	 * Aux from RxAuxCallback
	 */
	if (IsPassThrough) {
		AuxFifoStartFlag = (TRUE);
	}

	/* Check whether the sink is DVI/HDMI Supported
	 * If the sink is DVI, don't send Info-frame
	 */
	if (EdidHdmi20_t.EdidCtrlParam.IsHdmi == XVIDC_ISHDMI) {
		SendInfoframe(&HdmiTxSs);
	}
}

/*****************************************************************************/
/**
*
* This function is called when a bridge unlocked has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxBrdgUnlockedCallback(void *CallbackRef) {
	/* When video out bridge lost lock, reset TPG */
	/* ResetTpg();                                */
	/* Config and Run the TPG                     */
	/* XV_ConfigTpg(&Tpg);                        */
}
#endif

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called when a RX aux irq has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxAuxCallback(void *CallbackRef) {
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	XHdmiC_Aux *AuxPtr;
	AuxPtr = XV_HdmiRxSs_GetAuxiliary(&HdmiRxSs);

	/* In pass-through mode copy some aux packets into local buffer.
	 * GCP does not need to be sent out because GCP packets on the TX side
	 * is handled by the HDMI TX core fully. Starts storing Aux only
	 * when TX stream has started to prevent AuxFifo Overflow.
	 */
	if (IsPassThrough && AuxPtr->Header.Byte[0] !=
			AUX_GENERAL_CONTROL_PACKET_TYPE &&
						AuxFifoStartFlag == TRUE) {
		memcpy(&(AuxFifo[AuxFifoEndIndex]),
		       AuxPtr,
		       sizeof(XHdmiC_Aux));

		if(AuxFifoEndIndex < (AUXFIFOSIZE - 1)) {
			AuxFifoEndIndex++;
		} else {
			AuxFifoEndIndex = 0;
		}

		if(AuxFifoCount >= AUXFIFOSIZE) {
			AuxFifoOvrFlowCnt++;
		}

		AuxFifoCount++;
	}
#endif
}

/*****************************************************************************/
/**
*
* This function is called when a RX audio irq has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxAudCallback(void *CallbackRef) {
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#ifndef XPAR_XI2SRX_NUM_INSTANCES
	XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;
#endif

#ifdef XPAR_XI2SRX_NUM_INSTANCES

	/* With I2S, it is always a PCM data
	 * The TX audio is handled separately after the I2S IP are setup
     */

#else
	if (IsPassThrough) {
		/* Set TX Audio params:
		 * Audio Channels:
		 */
		XV_HdmiTxSs_SetAudioChannels(&HdmiTxSs,
				XV_HdmiRxSs_GetAudioChannels(HdmiRxSsPtr));

		/* HBR audio */
		if (XV_HdmiRxSs_GetAudioFormat(HdmiRxSsPtr) ==
						XV_HDMIRX_AUDFMT_HBR) {
			XV_HdmiTxSs_SetAudioFormat(&HdmiTxSs,
						   XV_HDMITX_AUDFMT_HBR);
		}
		/* L-PCM */
		else {
			XV_HdmiTxSs_SetAudioFormat(&HdmiTxSs,
						   XV_HDMITX_AUDFMT_LPCM);
		}

	}
#endif
#endif
}

/*****************************************************************************/
/**
*
* This function is called when a RX link status irq has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxLnkStaCallback(void *CallbackRef) {
	XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

	if (IsPassThrough) {
		/* Reset RX when the link error has reached its maximum */
		if ((HdmiRxSsPtr->IsLinkStatusErrMax) &&
				(Vphy.Quads[0].Plls[0].RxState ==
						XVPHY_GT_STATE_READY)) {

			/* Pulse RX PLL reset */
			XVphy_ClkDetFreqReset(&Vphy, 0, XVPHY_DIR_RX);
		}
	}
}

/*****************************************************************************/
/**
*
* This function is called when the RX DDC irq has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxDdcCallback(void *CallbackRef) {
}

/*****************************************************************************/
/**
*
* This function is called when the RX HDCP irq has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxHdcpCallback(void *CallbackRef) {
}

/*****************************************************************************/
/**
*
* This function is called when the RX stream is down.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxStreamDownCallback(void *CallbackRef) {

#if(LOOPBACK_MODE_EN != 1)
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	ResetAuxFifo();
	/* Check for Pass-through
	* Doesnt require to restart colorbar
	* if the system is in colorbar mode
	*/
	if (IsPassThrough) {
		/* RX Stream Down happen due to 2 possible scenarios
		* 1 - When resolution change
		* 2 - When the source stops sending stream even the RX cable
		*     is connected
		* In this example, The color bar should happen only for 2
		* scenarios, when the TX Cable is still connected
		* 1 - When pressing "c" by forcing the color bar
		* 2 - When RX cable is disconnected
		* Hence, when the stream change, the example design
		* wouldn't enter to color bar mode unless both scenario's32
		* above
		*/
		TxRestartColorbar = (FALSE);
		TxBusy = (TRUE);
	}
#endif
#endif

#ifdef USE_HDCP
	/* Call HDCP stream-down callback */
	XHdcp_StreamDownCallback(&HdcpRepeater);
#endif

#ifdef XPAR_XI2SRX_NUM_INSTANCES
	rx_is_up = 0;
#endif


}

/*****************************************************************************/
/**
*
* This function is called when the RX stream init
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxStreamInitCallback(void *CallbackRef) {
	XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;
	XVidC_VideoStream *HdmiRxSsVidStreamPtr;
	u32 Status;

	/* Calculate RX MMCM parameters
	 * In the application the YUV422 colordepth is 12 bits
	 * However the HDMI transports YUV422 in 8 bits.
	 * Therefore force the colordepth to 8 bits when the
	 * colorspace is YUV422
	 */

	HdmiRxSsVidStreamPtr = XV_HdmiRxSs_GetVideoStream(HdmiRxSsPtr);

	if (HdmiRxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_YCRCB_422) {
		Status =
			XVphy_HdmiCfgCalcMmcmParam(&Vphy,
					       0,
					       XVPHY_CHANNEL_ID_CH1,
					       XVPHY_DIR_RX,
					       HdmiRxSsVidStreamPtr->PixPerClk,
					       XVIDC_BPC_8);
	}

	/* Other colorspaces */
	else {
		Status = XVphy_HdmiCfgCalcMmcmParam(&Vphy,
					     0,
					     XVPHY_CHANNEL_ID_CH1,
					     XVPHY_DIR_RX,
					     HdmiRxSsVidStreamPtr->PixPerClk,
					     HdmiRxSsVidStreamPtr->ColorDepth);
	}

	if (Status == XST_FAILURE) {
		return;
	}

	/* Enable and configure RX MMCM */
	XVphy_MmcmStart(&Vphy, 0, XVPHY_DIR_RX);

	usleep(10000);
}

/*****************************************************************************/
/**
*
* This function is called when the RX stream is up.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxStreamUpCallback(void *CallbackRef) {
	XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;
	xil_printf("RX stream is up\r\n");
#if(LOOPBACK_MODE_EN != 1 && XPAR_XV_HDMITXSS_NUM_INSTANCES > 0)
	u32 Status;
	u64 LineRate;
	XVphy_PllType RxPllType;
	XVidC_VideoStream *HdmiRxSsVidStreamPtr;
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;

	HdmiRxSsVidStreamPtr = XV_HdmiRxSs_GetVideoStream(HdmiRxSsPtr);
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(&HdmiTxSs);

	/* Copy video parameters */
	*HdmiTxSsVidStreamPtr = *HdmiRxSsVidStreamPtr;
	XV_HdmiTxSs_SetVideoIDCode(&HdmiTxSs,
				XV_HdmiRxSs_GetVideoIDCode(HdmiRxSsPtr));
	XV_HdmiTxSs_SetVideoStreamType(&HdmiTxSs,
				XV_HdmiRxSs_GetVideoStreamType(HdmiRxSsPtr));

	RxPllType = XVphy_GetPllType(&Vphy,
				     0,
				     XVPHY_DIR_RX,
				     XVPHY_CHANNEL_ID_CH1);

	if (Vphy.Config.XcvrType != XVPHY_GT_TYPE_GTPE2) {
		if (!(RxPllType == XVPHY_PLL_TYPE_CPLL)) {
			LineRate = Vphy.Quads[0].Plls[XVPHY_CHANNEL_ID_CMN0 -
				XVPHY_CHANNEL_ID_CH1].LineRateHz;
		} else {
			LineRate = Vphy.Quads[0].Plls[0].LineRateHz;
		}
	} else { /* GTP */
		if (RxPllType == XVPHY_PLL_TYPE_PLL0) {
			LineRate = Vphy.Quads[0].Plls[XVPHY_CHANNEL_ID_CMN0 -
				XVPHY_CHANNEL_ID_CH1].LineRateHz;
		} else {
			LineRate = Vphy.Quads[0].Plls[XVPHY_CHANNEL_ID_CMN1 -
				XVPHY_CHANNEL_ID_CH1].LineRateHz;
		}
	}


	/* Check GT line rate
	 * For 4k60p the reference clock must be multiplied by four
	 */
	if ((LineRate / 1000000) > 3400) {
		/* TX reference clock */
		Vphy.HdmiTxRefClkHz = Vphy.HdmiRxRefClkHz * 4;
		XV_HdmiTxSs_SetTmdsClockRatio(&HdmiTxSs, 1);

		XV_HdmiTxSs_SetVideoStreamScramblingFlag(&HdmiTxSs, (TRUE));
	}

	/* Other resolutions */
	else {
		Vphy.HdmiTxRefClkHz = Vphy.HdmiRxRefClkHz;
		XV_HdmiTxSs_SetTmdsClockRatio(&HdmiTxSs, 0);

		XV_HdmiTxSs_SetVideoStreamScramblingFlag(&HdmiTxSs, (FALSE));
	}

	/* Set GT TX parameters */
	Status = XVphy_SetHdmiTxParam(&Vphy,
	                              0,
				      XVPHY_CHANNEL_ID_CHA,
				      HdmiTxSsVidStreamPtr->PixPerClk,
				      HdmiTxSsVidStreamPtr->ColorDepth,
				      HdmiTxSsVidStreamPtr->ColorFormatId);

	if (Status == XST_FAILURE) {
		return;
	}

	/* Disable the Colorbar */
	TxRestartColorbar = (FALSE);
	if (TxCableConnect) {
		/* Restart the TX if the TX Cable Connected */
		StartTxAfterRxFlag = (TRUE);

		/* Mute/Stop the HDMI RX SS Output
		 * Enable only when the downstream is ready
		 * (TPG & HDMI TX)
		 */
		XV_HdmiRxSs_VRST(HdmiRxSsPtr,TRUE);
	}

	/* Enable pass-through */
	IsPassThrough = (TRUE);

#else
	XVidC_ReportStreamInfo(&HdmiRxSsPtr->HdmiRxPtr->Stream.Video);

#endif

#ifdef USE_HDCP
	/* Call HDCP stream-up callback */
	XHdcp_StreamUpCallback(&HdcpRepeater);
#endif

#ifdef VIDEO_FRAME_CRC_EN
	/* Reset Video Frame CRC */
	XVidFrameCrc_Reset();
#endif

#ifdef XPAR_XI2SRX_NUM_INSTANCES
	rx_is_up = 1;
#endif
}

/*****************************************************************************/
/**
*
* This function is called when a RX Bridge Overflow event has occurred.
* RX Video Bridge Debug Utility
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxBrdgOverflowCallback(void *CallbackRef) {

	/* xil_printf(ANSI_COLOR_YELLOW "RX Video Bridge Overflow"
			ANSI_COLOR_RESET "\r\n"); */
}
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called when the TX stream is up.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxStreamUpCallback(void *CallbackRef) {

#if defined(XPAR_XV_HDMITXSS_NUM_INSTANCES)
#ifndef XPAR_XI2SRX_NUM_INSTANCES
	XHdmiC_AudioInfoFrame *AudioInfoFramePtr;
#endif
	XHdmiC_AVI_InfoFrame  *AVIInfoFramePtr;
#endif
	IsStreamUp = TRUE;

	XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;
	XVphy_PllType TxPllType;
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	XVidC_VideoStream *HdmiRxSsVidStreamPtr;

	HdmiRxSsVidStreamPtr = XV_HdmiRxSs_GetVideoStream(&HdmiRxSs);

	/* In passthrough copy the RX stream parameters to the TX stream */
	if (IsPassThrough) {
		XV_HdmiTxSs_SetVideoStream(HdmiTxSsPtr, *HdmiRxSsVidStreamPtr);
		/* If the Start TX after RX Flag is (TRUE), hence,
		 * no need to set thethe TX, let the StartTxAfterRX API
		 * executed which will trigger TX Stream up eventually
		 */
		if (StartTxAfterRxFlag
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
			&& !(XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1))
#endif
#endif
		) {
			return;
		}
	}
#endif

	/* Reset the TPG */
	ResetTpg();

	/* Obtain the stream information:
	 * Notes: XV_HdmiTxSs_GetVideoStream are with updated value, either
	 * colorbar or pass-through
	 */
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(HdmiTxSsPtr);

	/* Check whether the sink is DVI/HDMI Supported */
	if (EdidHdmi20_t.EdidCtrlParam.IsHdmi == XVIDC_ISDVI) {
		if (HdmiTxSsVidStreamPtr->ColorDepth != XVIDC_BPC_8 ||
			HdmiTxSsVidStreamPtr->ColorFormatId != XVIDC_CSF_RGB) {
			xil_printf(ANSI_COLOR_YELLOW "Un-able to set TX "
						"stream, sink is DVI\r\n"
						ANSI_COLOR_RESET "\r\n");
			/* Clear TX busy flag */
			TxBusy = (FALSE);
			/* Don't set TX, if the Sink is DVI, but the source
			 * properties are:
			 *      - Color Depth more than 8 BPC
			 *      - Color Space not RGB
			 */
			return;
		} else {
			xil_printf(ANSI_COLOR_YELLOW "Set TX stream to DVI,"
				" sink is DVI\r\n" ANSI_COLOR_RESET "\r\n");
			XV_HdmiTxSs_AudioMute(HdmiTxSsPtr, TRUE);
			XV_HdmiTxSS_SetDviMode(HdmiTxSsPtr);
		}
	} else {
		XV_HdmiTxSS_SetHdmiMode(HdmiTxSsPtr);
		XV_HdmiTxSs_AudioMute(HdmiTxSsPtr, FALSE);
	}

	xil_printf("TX stream is up\r\n");

	/* Check for the 480i/576i during color bar mode
	 * When it's (TRUE), set the Info Frame Pixel Repetition to x2
	 */
	if (!IsPassThrough) {
		AVIInfoFramePtr = XV_HdmiTxSs_GetAviInfoframe(HdmiTxSsPtr);

		if ( (HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_1440x480_60_I) ||
		     (HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_1440x576_50_I) ) {
			AVIInfoFramePtr->PixelRepetition =
					XHDMIC_PIXEL_REPETITION_FACTOR_2;
		} else {
			AVIInfoFramePtr->PixelRepetition =
					XHDMIC_PIXEL_REPETITION_FACTOR_1;
		}
	}
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	else {
		/* Parse RX AVI Infoframe to TX AVI Infoframe
		 * as parameters from AVI Infoframe are used
		 * in TX Subsystem
		 */
		AVIInfoFramePtr = XV_HdmiRxSs_GetAviInfoframe(&HdmiRxSs);
		(void)memset((void *) &(HdmiTxSsPtr->AVIInfoframe),
				0x00, sizeof(XHdmiC_AVI_InfoFrame));
		memcpy (&(HdmiTxSsPtr->AVIInfoframe),
				AVIInfoFramePtr,
				sizeof(XHdmiC_AVI_InfoFrame));
	}
#endif
	TxPllType = XVphy_GetPllType(&Vphy,
	                             0,
				     XVPHY_DIR_TX,
				     XVPHY_CHANNEL_ID_CH1);

	if ((TxPllType == XVPHY_PLL_TYPE_CPLL)) {
		TxLineRate = XVphy_GetLineRateHz(&Vphy,
		                                 0,
						 XVPHY_CHANNEL_ID_CH1);

	} else if((TxPllType == XVPHY_PLL_TYPE_QPLL) ||
			  (TxPllType == XVPHY_PLL_TYPE_QPLL0) ||
			  (TxPllType == XVPHY_PLL_TYPE_PLL0)) {
		TxLineRate =
			XVphy_GetLineRateHz(&Vphy, 0, XVPHY_CHANNEL_ID_CMN0);
	} else {
		TxLineRate =
			XVphy_GetLineRateHz(&Vphy, 0, XVPHY_CHANNEL_ID_CMN1);
	}

	/* Copy Sampling Rate */
	XV_HdmiTxSs_SetSamplingRate(HdmiTxSsPtr, Vphy.HdmiTxSampleRate);

	/* Set colorbar pattern */
	Pattern = XTPG_BKGND_COLOR_BARS;

	/* Config and Run the TPG */
	XV_ConfigTpg(&Tpg);

#if defined(XPAR_XV_HDMITXSS_NUM_INSTANCES)
#if defined(XPAR_XI2SRX_NUM_INSTANCES)
	/* Do not start the Audio path here
	 * Audio path is handled separately with I2S
	 */
#else
#if defined(USE_HDMI_AUDGEN)
	XhdmiACRCtrl_TMDSClkRatio(&AudioGen,
				HdmiTxSsPtr->HdmiTxPtr->Stream.TMDSClockRatio);
	/* Select the Audio source */
	if (IsPassThrough) {

		/* Disable audio generator */
		XhdmiAudGen_Start(&AudioGen, FALSE);

		/* Select ACR from RX */
		XhdmiACRCtrl_Sel(&AudioGen, ACR_SEL_IN);

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		/* Set TX Audio Channels (From HDMI RX Audio) */
		XV_HdmiTxSs_SetAudioChannels(&HdmiTxSs,
				XV_HdmiRxSs_GetAudioChannels(&HdmiRxSs));
#endif
		/* Re-program audio clock */
		XhdmiAudGen_SetAudClk(&AudioGen, XAUD_SRATE_192K);
	} else {
		AudioInfoFramePtr = XV_HdmiTxSs_GetAudioInfoframe(&HdmiTxSs);

		/* Reset Audio InfoFrame */
		(void)memset((void *)AudioInfoFramePtr,
			0,
			sizeof(XHdmiC_AudioInfoFrame));

		/* Enable audio generator */
		XhdmiAudGen_Start(&AudioGen, TRUE);

		/* Select ACR from ACR Ctrl */
		XhdmiACRCtrl_Sel(&AudioGen, ACR_SEL_GEN);

		/* Enable 2-channel audio */
		XV_HdmiTxSs_SetAudioChannels(&HdmiTxSs, 2);
		XhdmiAudGen_SetEnabChannels(&AudioGen, 2);
		XhdmiAudGen_SetPattern(&AudioGen, 1, XAUD_PAT_PING);
		XhdmiAudGen_SetPattern(&AudioGen, 2, XAUD_PAT_PING);
		XhdmiAudGen_SetSampleRate(&AudioGen,
				   XV_HdmiTxSs_GetTmdsClockFreqHz(HdmiTxSsPtr),
				   XAUD_SRATE_48K);
		/* Refer to CEA-861-D for Audio InfoFrame Channel Allocation
		 * - - - - - - FR FL
		 */
		AudioInfoFramePtr->ChannelAllocation = 0x0;
		/* Refer to Stream Header */
		AudioInfoFramePtr->SampleFrequency = 0x0;
	}
#endif
#endif
#endif

	ReportStreamMode(HdmiTxSsPtr, IsPassThrough);

#ifdef USE_HDCP
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
	/* During Bonded Mode, push for authentication during
	 * StartAfterTX API
	 */
	if ((!(XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)))
			|| !StartTxAfterRxFlag) {
#endif
#endif
		/* Call HDCP stream-up callback */
		XHdcp_StreamUpCallback(&HdcpRepeater);
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
	}
#endif
#endif
#endif

#ifdef VIDEO_FRAME_CRC_EN
	/* Reset Video Frame CRC */
	XVidFrameCrc_Reset();
#endif

	/* Clear TX busy flag */
	TxBusy = (FALSE);

#ifdef XPAR_XI2SRX_NUM_INSTANCES
	tx_is_up = 1;
#endif

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	/* Release RX Video Bridge Reset */
	if (IsPassThrough) {
		XV_HdmiRxSs_VRST(&HdmiRxSs,FALSE);
	}
#endif
}

/*****************************************************************************/
/**
*
* This function is called when the TX stream is down.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxStreamDownCallback(void *CallbackRef) {
#ifdef XPAR_XI2SRX_NUM_INSTANCES
	tx_is_up = 0;
#endif
	/* If the system in the Pass-through
	 * reset the AUX FIFO
	 */
	if (IsPassThrough) {
		ResetAuxFifo();
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		/* Mute/Stop the HDMI RX SS Output
		 * Enable only when the downstream is ready
		 * (TPG & HDMI TX)
		 */
		XV_HdmiRxSs_VRST(&HdmiRxSs,TRUE);
#endif
	}

	xil_printf("TX stream is down\r\n");

#ifdef USE_HDCP
	/* Call HDCP stream-down callback */
	XHdcp_StreamDownCallback(&HdcpRepeater);
#endif
}

/*****************************************************************************/
/**
*
* This function is called when a TX Bridge Overflow event has occurred.
* TX Video Bridge Debug Utility
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxBrdgOverflowCallback(void *CallbackRef) {

	/* xil_printf(ANSI_COLOR_YELLOW "TX Video Bridge Overflow"
			ANSI_COLOR_RESET "\r\n"); */
}

/*****************************************************************************/
/**
*
* This function is called when a TX Bridge Underflow event has occurred.
* TX Video Bridge Debug Utility
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void TxBrdgUnderflowCallback(void *CallbackRef) {

	/* xil_printf(ANSI_COLOR_YELLOW "TX Video Bridge Underflow"
			ANSI_COLOR_RESET "\r\n"); */
}

/*****************************************************************************/
/**
*
* This function is called to start the TX stream after the RX stream
* was up and running.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void StartTxAfterRx(void) {

	/* Clear the Start Tx After Rx Flag */
	StartTxAfterRxFlag = (FALSE);

	/* Reset TPG */
	ResetTpg();

	/* Disable TX TDMS clock */
	XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_TX, (FALSE));

	XV_HdmiTxSs_StreamStart(&HdmiTxSs);

	/* Enable RX clock forwarding */
	XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_RX, (TRUE));

	/* Program external clock generator in locked mode
	 * Only when the GT TX and RX are not coupled
	 */
	if (!XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)) {
		I2cClk(Vphy.HdmiRxRefClkHz,Vphy.HdmiTxRefClkHz);
	} else {
#if (!defined XPS_BOARD_ZCU104)
		Si5324_Reset(XPAR_IIC_0_BASEADDR, I2C_CLK_ADDR);
		Si5324_Init(XPAR_IIC_0_BASEADDR, I2C_CLK_ADDR);
#else
		IDT_8T49N24x_Init(XPAR_IIC_0_BASEADDR, I2C_CLK_ADDR);
#endif
	}
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
	/* When the GT TX and RX are coupled, then start the TXPLL */
	if (XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)) {
		/* Config and Run the TPG */
		XV_ConfigTpg(&Tpg);
		/* Start TX MMCM */
		XVphy_MmcmStart(&Vphy, 0, XVPHY_DIR_TX);
		usleep(10000);
	}

	/* Enable TX TMDS Clock in bonded mode */
	if (XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)) {
		XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_TX, (TRUE));
	}

#ifdef USE_HDCP
	/* Trigger authentication after there are stable stream
	 * in bonded mode
	 */
	if (XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)) {
		/* Call HDCP stream-up callback */
		XHdcp_StreamUpCallback(&HdcpRepeater);
	}
#endif
#endif
#endif
}
#endif
/*****************************************************************************/
/**
*
* This function setups the interrupt system so interrupts can occur for the
* HDMI cores. The function is application-specific since the actual system
* may or may not have an interrupt controller. The HDMI cores could be
* directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param  None.
*
* @return
*   - XST_SUCCESS if interrupt setup was successful.
*   - A specific error code defined in "xstatus.h" if an error
*   occurs.
*
* @note   This function assumes a Microblaze system and no operating
*   system is used.
*
******************************************************************************/
int SetupInterruptSystem(void) {
	int Status;
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	XScuGic *IntcInstPtr = &Intc;
#else
	XIntc *IntcInstPtr = &Intc;
#endif

	/*
	 * Initialize the interrupt controller driver so that it's ready to
	 * use, specify the device ID that was generated in xparameters.h
	 */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	XScuGic_Config *IntcCfgPtr;
	IntcCfgPtr = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
	if(IntcCfgPtr == NULL) {
		xil_printf("ERR:: Interrupt Controller not found");
		return (XST_DEVICE_NOT_FOUND);
	}
	Status = XScuGic_CfgInitialize(IntcInstPtr,
				IntcCfgPtr,
				IntcCfgPtr->CpuBaseAddress);
#else
	Status = XIntc_Initialize(IntcInstPtr, XPAR_INTC_0_DEVICE_ID);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Intc initialization failed!\r\n");
		return XST_FAILURE;
	}


	/*
	 * Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */
#if defined (__MICROBLAZE__)
	Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
        /* Status = XIntc_Start(IntcInstPtr, XIN_SIMULATION_MODE); */
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				(XScuGic *)IntcInstPtr);
#else
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XIntc_InterruptHandler,
				(XIntc *)IntcInstPtr);
#endif

	return (XST_SUCCESS);
}

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function updates the ColorFormat for the current video stream
*
* @param VphyPtr is a pointer to the VPHY core instance.
* @param HdmiTxSsPtr is a pointer to the XV_HdmiTxSs instance.
* @param Requested ColorFormat
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void UpdateColorFormat(XVphy             *VphyPtr,
		       XV_HdmiTxSs       *HdmiTxSsPtr,
		       XVidC_ColorFormat ColorFormat) {

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(HdmiTxSsPtr);

	/* Check if the TX isn't busy already */
	if (!CheckTxBusy()) {
		/* Check passthrough */
		if (IsPassThrough) {
			xil_printf("Error: Color space conversion in "
				    "pass-through mode is not supported!\r\n");
			return;
		}
		/* Inform user that pixel repetition is not supported */
		if (((HdmiTxSsVidStreamPtr->VmId == XVIDC_VM_1440x480_60_I) ||
			(HdmiTxSsVidStreamPtr->VmId ==
					XVIDC_VM_1440x576_50_I)) &&
			(ColorFormat == XVIDC_CSF_YCRCB_422)) {

			xil_printf("The video bridge is unable to support "
				"pixel repetition in YUV 422 Color space\r\n");

		}

		EnableColorBar(VphyPtr,
			HdmiTxSsPtr,
			HdmiTxSsVidStreamPtr->VmId,
			ColorFormat,
			HdmiTxSsVidStreamPtr->ColorDepth);
	}
}

/*****************************************************************************/
/**
*
* This function updates the ColorDepth for the current video stream
*
* @param VphyPtr is a pointer to the VPHY core instance.
* @param HdmiTxSsPtr is a pointer to the XV_HdmiTxSs instance.
* @param Requested ColorFormat
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void UpdateColorDepth(XVphy             *VphyPtr,
		      XV_HdmiTxSs       *HdmiTxSsPtr,
		      XVidC_ColorDepth  ColorDepth) {

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(HdmiTxSsPtr);

	/* Check if the TX isn't busy already */
	if (!CheckTxBusy()) {
		/* Check Passthrough */
		if (IsPassThrough) {
			xil_printf("Color depth conversion in pass-through"
					   " mode not supported!\r\n");
			return;
		}

		/* Check Color Space: YUV422 */
		else if ((HdmiTxSsVidStreamPtr->ColorFormatId ==
							XVIDC_CSF_YCRCB_422) &&
				 (ColorDepth != XVIDC_BPC_12)) {
			xil_printf("YUV422 only supports 36-bits color"
								" depth!\r\n");
			return;
		}

		/* Check Rate more than 5.94 Gbps */
		else if (
			((HdmiTxSsVidStreamPtr->VmId ==
				XVIDC_VM_3840x2160_60_P) ||
			(HdmiTxSsVidStreamPtr->VmId ==
				XVIDC_VM_3840x2160_50_P)) &&
			((HdmiTxSsVidStreamPtr->ColorFormatId ==
				XVIDC_CSF_RGB) ||
			(HdmiTxSsVidStreamPtr->ColorFormatId ==
				XVIDC_CSF_YCRCB_444)) &&
			(ColorDepth != XVIDC_BPC_8)) {

			xil_printf("2160p60 & 2160p50 on RGB & YUV444 only"
					  " supports 24-bits colordepth!\r\n");
			return;
		}

		EnableColorBar(VphyPtr,
			HdmiTxSsPtr,
			HdmiTxSsVidStreamPtr->VmId,
			HdmiTxSsVidStreamPtr->ColorFormatId,
			ColorDepth);
	}
}

/*****************************************************************************/
/**
*
* This function updates the FrameRate for the current video stream
*
* @param VphyPtr is a pointer to the VPHY core instance.
* @param HdmiTxSsPtr is a pointer to the XV_HdmiTxSs instance.
* @param Requested FrameRate
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void UpdateFrameRate(XVphy           *VphyPtr,
		     XV_HdmiTxSs     *HdmiTxSsPtr,
		     XVidC_FrameRate FrameRate) {

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(HdmiTxSsPtr);

	/* Check if the TX isn't busy already */
	if (!CheckTxBusy()) {
		/* Check pass through */
		if (IsPassThrough) {
			xil_printf("Frame rate conversion in pass-through"
					   " mode not supported!\r\n");
			return;
		}


		/* Check if requested video mode is available */
		XVidC_VideoMode VmId =
			XVidC_GetVideoModeIdExtensive(
					&HdmiTxSsVidStreamPtr->Timing,
					FrameRate,
					HdmiTxSsVidStreamPtr->IsInterlaced,
					(FALSE));

		if (VmId != XVIDC_VM_NUM_SUPPORTED) {

			HdmiTxSsVidStreamPtr->VmId = VmId;

			EnableColorBar(VphyPtr,
				HdmiTxSsPtr,
				HdmiTxSsVidStreamPtr->VmId,
				HdmiTxSsVidStreamPtr->ColorFormatId,
				HdmiTxSsVidStreamPtr->ColorDepth);
		} else {
			xil_printf("%s : %d Hz is not supported!\r\n",
			    XVidC_GetVideoModeStr(HdmiTxSsVidStreamPtr->VmId),
			    FrameRate);
		}
	}
}

/*****************************************************************************/
/**
*
* This function enables the ColorBar
*
* @param VphyPtr is a pointer to the VPHY core instance.
* @param HdmiTxSsPtr is a pointer to the XV_HdmiTxSs instance.
* @param Requested Video mode
* @param Requested ColorFormat
* @param Requested ColorDepth
* @param Requested Pixels per clock
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void EnableColorBar(XVphy                *VphyPtr,
		    XV_HdmiTxSs          *HdmiTxSsPtr,
		    XVidC_VideoMode      VideoMode,
		    XVidC_ColorFormat    ColorFormat,
		    XVidC_ColorDepth     Bpc) {

	u32 TmdsClock = 0;
	u32 Result;
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
	XHdmiC_VSIF *VSIFPtr;

	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(HdmiTxSsPtr);
	AviInfoFramePtr = XV_HdmiTxSs_GetAviInfoframe(HdmiTxSsPtr);
	VSIFPtr = XV_HdmiTxSs_GetVSIF(HdmiTxSsPtr);

	/* Reset Avi InfoFrame */
	(void)memset((void *)AviInfoFramePtr, 0, sizeof(XHdmiC_AVI_InfoFrame));
	/* Reset Vendor Specific InfoFrame */
	(void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));

	/* Check if the TX isn't busy already */
	if (!CheckTxBusy()) {
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		/* Stop HDMI RX Video AXI-4 Stream */
		XV_HdmiRxSs_VRST(&HdmiRxSs,TRUE);
#endif

		IsPassThrough = (FALSE); /* Set Color Bar */
		TxBusy = (TRUE);         /* Set TX busy flag */
#if(CUSTOM_RESOLUTION_ENABLE == 1)
		if (VideoMode < XVIDC_VM_NUM_SUPPORTED ||
		       (VideoMode > XVIDC_VM_CUSTOM &&
		        VideoMode < (XVidC_VideoMode)XVIDC_CM_NUM_SUPPORTED)) {
#else
		if (VideoMode < XVIDC_VM_NUM_SUPPORTED) {
#endif
			xil_printf("Starting colorbar\r\n");

			/* Disable TX TDMS clock */
			XVphy_Clkout1OBufTdsEnable(VphyPtr,
						XVPHY_DIR_TX,
						(FALSE));

		} else {
			TxBusy = (FALSE);
			xil_printf("Video mode not supported, please change"
					" video mode\r\n");
			return;
		}

		TmdsClock = XV_HdmiTxSs_SetStream(HdmiTxSsPtr,
						VideoMode,
						ColorFormat,
						Bpc,
						NULL);

		/* Calling this API can enable or disable scrambler even
		 * if the TMDS clock is >340MHz.
		 * E.g:
		 * 	XV_HdmiTxSs_SetScrambler(HdmiTxSsPtr, TRUE);
		 */

		/* Update AVI InfoFrame */
		AviInfoFramePtr->Version = 2;
		AviInfoFramePtr->ColorSpace =
				XV_HdmiC_XVidC_To_IfColorformat(ColorFormat);
		AviInfoFramePtr->VIC =
				HdmiTxSsPtr->HdmiTxPtr->Stream.Vic;

		/* Set TX reference clock */
		VphyPtr->HdmiTxRefClkHz = TmdsClock;

		/* Set GT TX parameters */
		Result = XVphy_SetHdmiTxParam(VphyPtr,
					0,
					XVPHY_CHANNEL_ID_CHA,
					HdmiTxSsVidStreamPtr->PixPerClk,
					HdmiTxSsVidStreamPtr->ColorDepth,
					HdmiTxSsVidStreamPtr->ColorFormatId);

		if (Result == (XST_FAILURE)) {
			TxBusy = (FALSE);
			xil_printf
			  ("Unable to set requested TX video resolution.\r\n");
			xil_printf
			  ("Returning to previously TX video resolution.\r\n");
			return;
		}

		/* Disable RX clock forwarding */
		XVphy_Clkout1OBufTdsEnable(VphyPtr, XVPHY_DIR_RX, (FALSE));

		/* Program external clock generator in free running mode */
		I2cClk(0, VphyPtr->HdmiTxRefClkHz);
	}
}
#endif

void Xil_AssertCallbackRoutine(u8 *File, s32 Line) {
	xil_printf("Assertion in File %s, on line %0d\r\n", File, Line);
}

/*****************************************************************************/
/**
*
* Main function to call example with HDMI TX, HDMI RX and HDMI GT drivers.
*
* @param  None.
*
* @return
*   - XST_SUCCESS if HDMI example was successfully.
*   - XST_FAILURE if HDMI example failed.
*
* @note   None.
*
******************************************************************************/
int main() {
	u32 Status = XST_FAILURE;
	XVphy_Config *XVphyCfgPtr;
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;

	AuxFifoStartFlag = FALSE;
	AuxFifoEndIndex = 0;
	AuxFifoStartIndex = 0;
	AuxFifoCount = 0;
#endif
#if (defined (ARMR5) || (__aarch64__)) && (!defined XPS_BOARD_ZCU104)
	XIicPs_Config *XIic0Ps_ConfigPtr;
	XIicPs_Config *XIic1Ps_ConfigPtr;
#endif

	xil_printf("\r\n\r\n");
	xil_printf("--------------------------------------\r\n");
	xil_printf("---  HDMI SS + VPhy Example v%d.%d   ---\r\n",
			APP_MAJ_VERSION, APP_MIN_VERSION);
	xil_printf("---  (c) 2018 by Xilinx, Inc.      ---\r\n");
	xil_printf("--------------------------------------\r\n");
	xil_printf("Build %s - %s\r\n", __DATE__, __TIME__);
#ifdef XPAR_XI2SRX_NUM_INSTANCES
	xil_printf("This showcases the I2S Capture/Playback\r\n");
	xil_printf("capability for 2 Channel PCM Audio\r\n");
#endif
	xil_printf("--------------------------------------\r\n");
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	StartTxAfterRxFlag = (FALSE);

#if(LOOPBACK_MODE_EN != 1)
	TxBusy            = (TRUE);
	TxRestartColorbar = (FALSE);
#else
	TxBusy            = (FALSE);
	TxRestartColorbar = (TRUE);
#endif
#endif
	VphyErrorFlag = FALSE;
	VphyPllLayoutErrorFlag = FALSE;

	/* Start in color bar */
	IsPassThrough = (FALSE);

	/* Initialize platform */
	init_platform();

	/* Initialize IIC */
#if (defined (ARMR5) || (__aarch64__)) && (!defined XPS_BOARD_ZCU104)
	/* Initialize PS IIC0 */
	XIic0Ps_ConfigPtr = XIicPs_LookupConfig(XPAR_XIICPS_0_DEVICE_ID);
	if (NULL == XIic0Ps_ConfigPtr) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Ps_Iic0, XIic0Ps_ConfigPtr,
				XIic0Ps_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIicPs_Reset(&Ps_Iic0);
	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Ps_Iic0, PS_IIC_CLK);

	/* Initialize PS IIC1 */
	XIic1Ps_ConfigPtr = XIicPs_LookupConfig(XPAR_XIICPS_1_DEVICE_ID);
	if (NULL == XIic1Ps_ConfigPtr) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Ps_Iic1,
				XIic1Ps_ConfigPtr,
				XIic1Ps_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIicPs_Reset(&Ps_Iic1);
	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Ps_Iic1, PS_IIC_CLK);

#ifdef XPAR_XI2SRX_NUM_INSTANCES
	i2s_init();
	/* On Board SI5328 chip for Audio Clock */
	/* Initialize external clock generator  */
	/* Mux programmed to select Si5328      */
	I2cMux_Ps();
	/* Initialize external clock generator  */
	Si5324_Init_Ps(&Ps_Iic1, SI5328_I2C_ADDR);
	/* Generating a 18.432Mhz clock */
	I2cClk_Ps(0, 18432000);
	/* Delay 50ms to allow SI chip to lock  */
	usleep (50000);
	aud_clk_reset();
#else

	/* On Board SI5328 chip for DRU reference clock */
	I2cMux_Ps();
	/* Initialize external clock generator */
	Si5324_Init_Ps(&Ps_Iic1, 0x69);

	I2cClk_Ps(0, 156250000);

	/* Delay 50ms to allow SI chip to lock */
	usleep (50000);
#endif

#endif

	/* Initialize external clock generator */
#if (!defined XPS_BOARD_ZCU104)
	Si5324_Init(XPAR_IIC_0_BASEADDR, I2C_CLK_ADDR);
#else
	usleep(200000);
	IDT_8T49N24x_Init(XPAR_IIC_0_BASEADDR, I2C_CLK_ADDR);
#endif
#if !defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && \
		defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
#if XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTYE4
	/* Initialize DRU Clock Source */
	I2cClk(0, 156250000);
	/* Delay 50ms to allow SI chip to lock */
	usleep (50000);
#endif
#endif

	/* Disable TMDS181 HPD passthrough for ZCU106 Rev B and below */
	/* E.g.:
	 *	Disable_TMDS181_HPD_passthrough();
	 */

	/* Load HDCP keys from EEPROM */
#if defined (XPAR_XHDCP_NUM_INSTANCES) || \
		defined (XPAR_XHDCP22_RX_NUM_INSTANCES) || \
				defined (XPAR_XHDCP22_TX_NUM_INSTANCES)
	if (XHdcp_LoadKeys(Hdcp22Lc128,
			sizeof(Hdcp22Lc128),
			Hdcp22RxPrivateKey,
			sizeof(Hdcp22RxPrivateKey),
			Hdcp14KeyA,
			sizeof(Hdcp14KeyA),
			Hdcp14KeyB,
			sizeof(Hdcp14KeyB)) == XST_SUCCESS) {

		/* Set pointers to HDCP 2.2 Keys */
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#if XPAR_XHDCP22_TX_NUM_INSTANCES
		XV_HdmiTxSs_HdcpSetKey(&HdmiTxSs,
				XV_HDMITXSS_KEY_HDCP22_LC128,
				Hdcp22Lc128);
		XV_HdmiTxSs_HdcpSetKey(&HdmiTxSs,
				XV_HDMITXSS_KEY_HDCP22_SRM,
				Hdcp22Srm);
#endif
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
#if XPAR_XHDCP22_RX_NUM_INSTANCES
		XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs,
				XV_HDMIRXSS_KEY_HDCP22_LC128,
				Hdcp22Lc128);
		XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs,
				XV_HDMIRXSS_KEY_HDCP22_PRIVATE,
				Hdcp22RxPrivateKey);
#endif
#endif

		/* Set pointers to HDCP 1.4 keys */
#if XPAR_XHDCP_NUM_INSTANCES
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
		XV_HdmiTxSs_HdcpSetKey(&HdmiTxSs,
				XV_HDMITXSS_KEY_HDCP14,
				Hdcp14KeyA);
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs,
				XV_HDMIRXSS_KEY_HDCP14,
				Hdcp14KeyB);
#endif

		/* Initialize key manager */
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
		Status =
			XHdcp_KeyManagerInit(XPAR_HDCP_KEYMNGMT_BLK_0_BASEADDR,
					HdmiTxSs.Hdcp14KeyPtr);
		if (Status != XST_SUCCESS) {
			xil_printf
			("HDCP 1.4 TX Key Manager Initialization error\r\n");
			return XST_FAILURE;
		}
#endif

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		Status =
			XHdcp_KeyManagerInit(XPAR_HDCP_KEYMNGMT_BLK_1_BASEADDR,
					HdmiRxSs.Hdcp14KeyPtr);
		if (Status != XST_SUCCESS) {
			xil_printf
			("HDCP 1.4 RX Key Manager Initialization error\r\n");
			return XST_FAILURE;
		}
#endif
#endif

	}

	/* Clear pointers */
	else {
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
		/* Set pointer to NULL */
		XV_HdmiTxSs_HdcpSetKey(&HdmiTxSs,
				XV_HDMITXSS_KEY_HDCP22_LC128,
				(NULL));

		/* Set pointer to NULL */
		XV_HdmiTxSs_HdcpSetKey(&HdmiTxSs,
				XV_HDMITXSS_KEY_HDCP14,
				(NULL));

		/* Set pointer to NULL */
		XV_HdmiTxSs_HdcpSetKey(&HdmiTxSs,
				XV_HDMITXSS_KEY_HDCP22_SRM,
				(NULL));
#endif

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		/* Set pointer to NULL */
		XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs,
				XV_HDMIRXSS_KEY_HDCP22_LC128,
				(NULL));

		/* Set pointer to NULL */
		XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs,
				XV_HDMIRXSS_KEY_HDCP22_PRIVATE,
				(NULL));

		/* Set pointer to NULL */
		XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs,
				XV_HDMIRXSS_KEY_HDCP14,
				(NULL));
#endif


	}
#endif

#if defined(XPAR_XV_HDMITXSS_NUM_INSTANCES)
#if defined(USE_HDMI_AUDGEN)
#ifdef XPAR_XI2SRX_NUM_INSTANCES
	/* Initialize the Audio Generator */
	XhdmiAudGen_Init(&AudioGen,
		XPAR_AUDIO_I2S_SS_0_HDMI_ACR_CTRL_BASEADDR);

#else
	/* Initialize the Audio Generator */
	XhdmiAudGen_Init(&AudioGen,
			XPAR_AUDIO_SS_0_AUD_PAT_GEN_BASEADDR,
			XPAR_AUDIO_SS_0_HDMI_ACR_CTRL_BASEADDR,
			XPAR_AUDIO_SS_0_CLK_WIZ_BASEADDR);
#endif
#endif
#endif

#if defined (__arm__) && (!defined(ARMR5))
	/* Initialize on-board clock generator */
	OnBoardSi5324Init();

	/* Delay 15ms to allow SI chip to lock */
	usleep (15000);
#endif

	/* Initialize IRQ */
	Status = SetupInterruptSystem();
	if (Status == XST_FAILURE) {
		xil_printf("IRQ init failed.\r\n\r\n");
		return XST_FAILURE;
	}

#ifdef VIDEO_FRAME_CRC_EN
	XVidFrameCrc_Initialize(&VidFrameCRC);
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES

	/* Initialize HDMI TX Subsystem */

	XV_HdmiTxSs_ConfigPtr =
		XV_HdmiTxSs_LookupConfig(XPAR_XV_HDMITX_0_DEVICE_ID);

	if(XV_HdmiTxSs_ConfigPtr == NULL) {
		HdmiTxSs.IsReady = 0;
	}

	/* Initialize top level and all included sub-cores */
	Status = XV_HdmiTxSs_CfgInitialize(&HdmiTxSs, XV_HdmiTxSs_ConfigPtr,
					XV_HdmiTxSs_ConfigPtr->BaseAddress);
	if(Status != XST_SUCCESS) {
		xil_printf
		       ("ERR:: HDMI TX Subsystem Initialization failed %d\r\n",
		        Status);
	}

	/* Set the Application version in TXSs driver structure */
	XV_HdmiTxSS_SetAppVersion(&HdmiTxSs, APP_MAJ_VERSION, APP_MIN_VERSION);

	/* Register HDMI TX SS Interrupt Handler with Interrupt Controller */
#if defined(__arm__) || (__aarch64__)
#ifndef USE_HDCP
	Status |= XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_HDMITXSS_0_VEC_ID,
			(XInterruptHandler)XV_HdmiTxSS_HdmiTxIntrHandler,
			(void *)&HdmiTxSs);
#else
	Status |= XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_HDMITXSS_0_IRQ_VEC_ID,
			(XInterruptHandler)XV_HdmiTxSS_HdmiTxIntrHandler,
			(void *)&HdmiTxSs);
#endif


/* HDCP 1.4 */
#ifdef XPAR_XHDCP_NUM_INSTANCES
	/* HDCP 1.4 Cipher interrupt */
	Status |= XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_HDMITXSS_0_HDCP14_IRQ_VEC_ID,
			(XInterruptHandler)XV_HdmiTxSS_HdcpIntrHandler,
			(void *)&HdmiTxSs);

	/* HDCP 1.4 Timer interrupt */
	Status |= XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_HDMITXSS_0_HDCP14_TIMER_IRQ_VEC_ID,
			(XInterruptHandler)XV_HdmiTxSS_HdcpTimerIntrHandler,
			(void *)&HdmiTxSs);
#endif

/* HDCP 2.2 */
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
	Status |= XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_HDMITXSS_0_HDCP22_TIMER_IRQ_VEC_ID,
			(XInterruptHandler)XV_HdmiTxSS_Hdcp22TimerIntrHandler,
			(void *)&HdmiTxSs);
#endif

#else
	/* Register HDMI TX SS Interrupt Handler with Interrupt Controller */
	Status |= XIntc_Connect(&Intc,
#if defined(USE_HDCP)
			XPAR_INTC_0_V_HDMITXSS_0_IRQ_VEC_ID,
#else
			XPAR_INTC_0_V_HDMITXSS_0_VEC_ID,
#endif
			(XInterruptHandler)XV_HdmiTxSS_HdmiTxIntrHandler,
			(void *)&HdmiTxSs);

/* HDCP 1.4 */
#ifdef XPAR_XHDCP_NUM_INSTANCES
	/* HDCP 1.4 Cipher interrupt */
	Status |= XIntc_Connect(&Intc,
			XPAR_INTC_0_V_HDMITXSS_0_HDCP14_IRQ_VEC_ID,
			(XInterruptHandler)XV_HdmiTxSS_HdcpIntrHandler,
			(void *)&HdmiTxSs);

	/* HDCP 1.4 Timer interrupt */
	Status |= XIntc_Connect(&Intc,
			XPAR_INTC_0_V_HDMITXSS_0_HDCP14_TIMER_IRQ_VEC_ID,
			(XInterruptHandler)XV_HdmiTxSS_HdcpTimerIntrHandler,
			(void *)&HdmiTxSs);
#endif

/* HDCP 2.2 */
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
	Status |= XIntc_Connect(&Intc,
			XPAR_INTC_0_V_HDMITXSS_0_HDCP22_TIMER_IRQ_VEC_ID,
			(XInterruptHandler)XV_HdmiTxSS_Hdcp22TimerIntrHandler,
			(void *)&HdmiTxSs);
#endif

#endif

	if (Status == XST_SUCCESS) {
#if defined(__arm__) || (__aarch64__)
#ifndef USE_HDCP
		XScuGic_Enable(&Intc,
			XPAR_FABRIC_V_HDMITXSS_0_VEC_ID);
#else
		XScuGic_Enable(&Intc,
			XPAR_FABRIC_V_HDMITXSS_0_IRQ_VEC_ID);
#endif
/* HDCP 1.4 */
#ifdef XPAR_XHDCP_NUM_INSTANCES
		/* HDCP 1.4 Cipher interrupt */
		XScuGic_Enable(&Intc,
			XPAR_FABRIC_V_HDMITXSS_0_HDCP14_IRQ_VEC_ID);

		/* HDCP 1.4 Timer interrupt */
		XScuGic_Enable(&Intc,
			XPAR_FABRIC_V_HDMITXSS_0_HDCP14_TIMER_IRQ_VEC_ID);
#endif

/* HDCP 2.2 */
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
		XScuGic_Enable(&Intc,
			XPAR_FABRIC_V_HDMITXSS_0_HDCP22_TIMER_IRQ_VEC_ID);
#endif

#else
		XIntc_Enable(&Intc,
#if defined(USE_HDCP)
			XPAR_INTC_0_V_HDMITXSS_0_IRQ_VEC_ID
#else
			XPAR_INTC_0_V_HDMITXSS_0_VEC_ID
#endif
			);

/* HDCP 1.4 */
#ifdef XPAR_XHDCP_NUM_INSTANCES
		/* HDCP 1.4 Cipher interrupt */
		XIntc_Enable(&Intc,
			XPAR_INTC_0_V_HDMITXSS_0_HDCP14_IRQ_VEC_ID);

		/* HDCP 1.4 Timer interrupt */
		XIntc_Enable(&Intc,
			XPAR_INTC_0_V_HDMITXSS_0_HDCP14_TIMER_IRQ_VEC_ID);
#endif

/* HDCP 2.2 */
#if (XPAR_XHDCP22_TX_NUM_INSTANCES)
		XIntc_Enable(&Intc,
			XPAR_INTC_0_V_HDMITXSS_0_HDCP22_TIMER_IRQ_VEC_ID);
#endif

#endif
	} else {
		xil_printf
			("ERR:: Unable to register HDMI TX interrupt handler");
		xil_printf("HDMI TX SS initialization error\r\n");
		return XST_FAILURE;
	}

	/* HDMI TX SS callback setup */
	XV_HdmiTxSs_SetCallback(&HdmiTxSs,
				XV_HDMITXSS_HANDLER_CONNECT,
				(void *)TxConnectCallback,
				(void *)&HdmiTxSs);

	XV_HdmiTxSs_SetCallback(&HdmiTxSs,
				XV_HDMITXSS_HANDLER_TOGGLE,
				(void *)TxToggleCallback,
				(void *)&HdmiTxSs);

	XV_HdmiTxSs_SetCallback(&HdmiTxSs,
				XV_HDMITXSS_HANDLER_BRDGUNLOCK,
				(void *)TxBrdgUnlockedCallback,
				(void *)&HdmiTxSs);

	XV_HdmiTxSs_SetCallback(&HdmiTxSs,
				XV_HDMITXSS_HANDLER_BRDGOVERFLOW,
				(void *)TxBrdgOverflowCallback,
				(void *)&HdmiTxSs);

	XV_HdmiTxSs_SetCallback(&HdmiTxSs,
				XV_HDMITXSS_HANDLER_BRDGUNDERFLOW,
				(void *)TxBrdgUnderflowCallback,
				(void *)&HdmiTxSs);

	XV_HdmiTxSs_SetCallback(&HdmiTxSs,
				XV_HDMITXSS_HANDLER_VS,
				(void *)TxVsCallback,
				(void *)&HdmiTxSs);

	XV_HdmiTxSs_SetCallback(&HdmiTxSs,
				XV_HDMITXSS_HANDLER_STREAM_UP,
				(void *)TxStreamUpCallback,
				(void *)&HdmiTxSs);

	XV_HdmiTxSs_SetCallback(&HdmiTxSs,
				XV_HDMITXSS_HANDLER_STREAM_DOWN,
				(void *)TxStreamDownCallback,
				(void *)&HdmiTxSs);

#ifdef USE_HDCP
	if (XV_HdmiTxSs_HdcpIsReady(&HdmiTxSs)) {
		/* Initialize the HDCP instance */
		XHdcp_Initialize(&HdcpRepeater);

		/* Set HDCP downstream interface(s) */
		XHdcp_SetDownstream(&HdcpRepeater, &HdmiTxSs);
	}
#endif
#endif

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES

	/* Initialize HDMI RX Subsystem */

	/* Get User Edid Info */
	XV_HdmiRxSs_SetEdidParam(&HdmiRxSs, (u8*)&Edid, sizeof(Edid));
	XV_HdmiRxSs_ConfigPtr =
		XV_HdmiRxSs_LookupConfig(XPAR_XV_HDMIRX_0_DEVICE_ID);

	if(XV_HdmiRxSs_ConfigPtr == NULL) {
		HdmiRxSs.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	/* Initialize top level and all included sub-cores */
	Status = XV_HdmiRxSs_CfgInitialize(&HdmiRxSs,
					XV_HdmiRxSs_ConfigPtr,
					XV_HdmiRxSs_ConfigPtr->BaseAddress);
	if(Status != XST_SUCCESS) {
		xil_printf
		("ERR:: HDMI RX Subsystem Initialization failed %d\r\n",
		Status);
		return(XST_FAILURE);
	}

	/* Set the Application version in RXSs driver structure */
	XV_HdmiRxSS_SetAppVersion(&HdmiRxSs, APP_MAJ_VERSION, APP_MIN_VERSION);

	/* Register HDMI RX SS Interrupt Handler with Interrupt Controller */
#if defined(__arm__) || (__aarch64__)
#ifndef USE_HDCP
	Status |= XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_HDMIRXSS_0_VEC_ID,
			(XInterruptHandler)XV_HdmiRxSS_HdmiRxIntrHandler,
			(void *)&HdmiRxSs);
#else
	Status |= XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_HDMIRXSS_0_IRQ_VEC_ID,
			(XInterruptHandler)XV_HdmiRxSS_HdmiRxIntrHandler,
			(void *)&HdmiRxSs);
#endif


#ifdef XPAR_XHDCP_NUM_INSTANCES
	/* HDCP 1.4 Cipher interrupt */
	Status |= XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_HDMIRXSS_0_HDCP14_IRQ_VEC_ID,
			(XInterruptHandler)XV_HdmiRxSS_HdcpIntrHandler,
			(void *)&HdmiRxSs);

	Status |= XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_HDMIRXSS_0_HDCP14_TIMER_IRQ_VEC_ID,
			(XInterruptHandler)XV_HdmiRxSS_HdcpTimerIntrHandler,
			(void *)&HdmiRxSs);
#endif

#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
	/* HDCP 2.2 Timer interrupt */
	Status |= XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_HDMIRXSS_0_HDCP22_TIMER_IRQ_VEC_ID,
			(XInterruptHandler)XV_HdmiRxSS_Hdcp22TimerIntrHandler,
			(void *)&HdmiRxSs);
#endif

#else
	Status |= XIntc_Connect(&Intc,
#if defined(USE_HDCP)
			XPAR_INTC_0_V_HDMIRXSS_0_IRQ_VEC_ID,
#else
			XPAR_INTC_0_V_HDMIRXSS_0_VEC_ID,
#endif
			(XInterruptHandler)XV_HdmiRxSS_HdmiRxIntrHandler,
			(void *)&HdmiRxSs);

#ifdef XPAR_XHDCP_NUM_INSTANCES
	/* HDCP 1.4 Cipher interrupt */
	Status |= XIntc_Connect(&Intc,
				XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_IRQ_VEC_ID,
				(XInterruptHandler)XV_HdmiRxSS_HdcpIntrHandler,
				(void *)&HdmiRxSs);

	/* HDCP 1.4 Timer interrupt */
	Status |= XIntc_Connect(&Intc,
			XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_TIMER_IRQ_VEC_ID,
			(XInterruptHandler)XV_HdmiRxSS_HdcpTimerIntrHandler,
			(void *)&HdmiRxSs);
#endif

#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
	/* HDCP 2.2 Timer interrupt */
	Status |= XIntc_Connect(&Intc,
			XPAR_INTC_0_V_HDMIRXSS_0_HDCP22_TIMER_IRQ_VEC_ID,
			(XInterruptHandler)XV_HdmiRxSS_Hdcp22TimerIntrHandler,
			(void *)&HdmiRxSs);
#endif

#endif

	if (Status == XST_SUCCESS) {
#if defined(__arm__) || (__aarch64__)
#ifndef USE_HDCP
		XScuGic_Enable(&Intc,
				XPAR_FABRIC_V_HDMIRXSS_0_VEC_ID);
#else
		XScuGic_Enable(&Intc,
				XPAR_FABRIC_V_HDMIRXSS_0_IRQ_VEC_ID);
#endif
#ifdef XPAR_XHDCP_NUM_INSTANCES
		XScuGic_Enable(&Intc,
				XPAR_FABRIC_V_HDMIRXSS_0_HDCP14_IRQ_VEC_ID);
		XScuGic_Enable(&Intc,
			XPAR_FABRIC_V_HDMIRXSS_0_HDCP14_TIMER_IRQ_VEC_ID);
#endif
#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
		XScuGic_Enable(&Intc,
			XPAR_FABRIC_V_HDMIRXSS_0_HDCP22_TIMER_IRQ_VEC_ID);
#endif

#else
		XIntc_Enable(&Intc,
#if defined(USE_HDCP)
			XPAR_INTC_0_V_HDMIRXSS_0_IRQ_VEC_ID
#else
			XPAR_INTC_0_V_HDMIRXSS_0_VEC_ID
#endif
			);

#ifdef XPAR_XHDCP_NUM_INSTANCES
		/* HDCP 1.4 Cipher interrupt */
		XIntc_Enable(&Intc,
			XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_IRQ_VEC_ID);

		/* HDCP 1.4 Timer interrupt */
		XIntc_Enable(&Intc,
			XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_TIMER_IRQ_VEC_ID);
#endif

#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
		/* HDCP 2.2 Timer interrupt */
		XIntc_Enable(&Intc,
			XPAR_INTC_0_V_HDMIRXSS_0_HDCP22_TIMER_IRQ_VEC_ID);
#endif

#endif
	} else {
		xil_printf
			("ERR:: Unable to register HDMI RX interrupt handler");
		xil_printf("HDMI RX SS initialization error\r\n");
		return XST_FAILURE;
	}

	/* RX callback setup */
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_CONNECT,
				(void *)RxConnectCallback,
				(void *)&HdmiRxSs);
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_BRDGOVERFLOW,
				(void *)RxBrdgOverflowCallback,
				(void *)&HdmiRxSs);
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_AUX,
				(void *)RxAuxCallback,
				(void *)&HdmiRxSs);
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_AUD,
				(void *)RxAudCallback,
				(void *)&HdmiRxSs);
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_LNKSTA,
				(void *)RxLnkStaCallback,
				(void *)&HdmiRxSs);
	/*
	 *  XV_HdmiRxSs_SetCallback(&HdmiRxSs,
	 *	  	  	    XV_HDMIRXSS_HANDLER_DDC,
	 *	  	  	    RxDdcCallback,
	 *	  	  	    (void *)&HdmiRxSs);
	 */
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_STREAM_DOWN,
				(void *)RxStreamDownCallback,
				(void *)&HdmiRxSs);
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_STREAM_INIT,
				(void *)RxStreamInitCallback,
				(void *)&HdmiRxSs);
	XV_HdmiRxSs_SetCallback(&HdmiRxSs,
				XV_HDMIRXSS_HANDLER_STREAM_UP,
				(void *)RxStreamUpCallback,
				(void *)&HdmiRxSs);

#ifdef USE_HDCP
	/* Set HDCP upstream interface */
	XHdcp_SetUpstream(&HdcpRepeater, &HdmiRxSs);
#endif
#endif

	/*
	 *  Initialize Video PHY
	 *  The GT needs to be initialized after the HDMI RX and TX.
	 *  The reason for this is the GtRxInitStartCallback
	 *  calls the RX stream down callback.
	 *
         */
	XVphyCfgPtr = XVphy_LookupConfig(XPAR_VPHY_0_DEVICE_ID);
	if (XVphyCfgPtr == NULL) {
		xil_printf("Video PHY device not found\r\n\r\n");
		return XST_FAILURE;
	}

	/* Register VPHY Interrupt Handler */
#if defined(__arm__) || (__aarch64__)
	Status = XScuGic_Connect(&Intc,
				XPAR_FABRIC_VID_PHY_CONTROLLER_IRQ_INTR,
				(XInterruptHandler)XVphy_InterruptHandler,
				(void *)&Vphy);
#else
	Status = XIntc_Connect(&Intc,
				XPAR_INTC_0_VPHY_0_VEC_ID,
				(XInterruptHandler)XVphy_InterruptHandler,
				(void *)&Vphy);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("HDMI VPHY Interrupt Vec ID not found!\r\n");
		return XST_FAILURE;
	}

	/* Initialize HDMI VPHY */
	Status = XVphy_Hdmi_CfgInitialize(&Vphy, 0, XVphyCfgPtr);

	if (Status != XST_SUCCESS) {
		xil_printf("HDMI VPHY initialization error\r\n");
		return XST_FAILURE;
	}

	/* Enable VPHY Interrupt */
#if defined(__arm__) || (__aarch64__)
	XScuGic_Enable(&Intc,
			XPAR_FABRIC_VID_PHY_CONTROLLER_IRQ_INTR);
#else
	XIntc_Enable(&Intc,
			XPAR_INTC_0_VPHY_0_VEC_ID);
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	/* VPHY callback setup */
	XVphy_SetHdmiCallback(&Vphy,
				XVPHY_HDMI_HANDLER_TXINIT,
				(void *)VphyHdmiTxInitCallback,
				(void *)&Vphy);
	XVphy_SetHdmiCallback(&Vphy,
				XVPHY_HDMI_HANDLER_TXREADY,
				(void *)VphyHdmiTxReadyCallback,
				(void *)&Vphy);
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	XVphy_SetHdmiCallback(&Vphy,
				XVPHY_HDMI_HANDLER_RXINIT,
				(void *)VphyHdmiRxInitCallback,
				(void *)&Vphy);
	XVphy_SetHdmiCallback(&Vphy,
				XVPHY_HDMI_HANDLER_RXREADY,
				(void *)VphyHdmiRxReadyCallback,
				(void *)&Vphy);
#endif

	XVphy_SetErrorCallback(&Vphy,
				(void *)VphyErrorCallback,
				(void *)&Vphy);

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
	XVphy_SetPllLayoutErrorCallback(&Vphy,
					(void *)VphyPllLayoutErrorCallback,
					(void *)&Vphy);
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#ifdef XPAR_XV_TPG_NUM_INSTANCES
	/* Initialize GPIO for Tpg Reset */
	Gpio_Tpg_resetn_ConfigPtr =
		XGpio_LookupConfig(XPAR_V_TPG_SS_0_AXI_GPIO_DEVICE_ID);

	if(Gpio_Tpg_resetn_ConfigPtr == NULL) {
		Gpio_Tpg_resetn.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XGpio_CfgInitialize(&Gpio_Tpg_resetn,
				Gpio_Tpg_resetn_ConfigPtr,
				Gpio_Tpg_resetn_ConfigPtr->BaseAddress);
	if(Status != XST_SUCCESS) {
		xil_printf("ERR:: GPIO for TPG Reset ");
		xil_printf("Initialization failed %d\r\n", Status);
		return(XST_FAILURE);
	}

	ResetTpg();

	Tpg_ConfigPtr = XV_tpg_LookupConfig(XPAR_V_TPG_SS_0_V_TPG_DEVICE_ID);
	if(Tpg_ConfigPtr == NULL) {
		Tpg.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XV_tpg_CfgInitialize(&Tpg,
				Tpg_ConfigPtr, Tpg_ConfigPtr->BaseAddress);
	if(Status != XST_SUCCESS) {
		xil_printf("ERR:: TPG Initialization failed %d\r\n", Status);
		return(XST_FAILURE);
	}
#endif
#endif

	xil_printf("---------------------------------\r\n");

	/* Enable exceptions. */
	Xil_AssertSetCallback((Xil_AssertCallback) Xil_AssertCallbackRoutine);
	Xil_ExceptionEnable();

	/* Initialize menu */
	XHdmi_MenuInitialize(&HdmiMenu, UART_BASEADDR);

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	/* Start with 1080p stream */
	TxInfoFrameReset();
	XV_HdmiTxSs_SetStream(
		&HdmiTxSs,
		XVIDC_VM_1920x1080_60_P,
		XVIDC_CSF_RGB,
		XVIDC_BPC_8,
		NULL);
#endif

#if defined (USE_HDCP) && defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && \
				defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
#if ENABLE_HDCP_REPEATER
	if (XV_HdmiRxSs_HdcpIsReady(&HdmiRxSs) &&
					XV_HdmiTxSs_HdcpIsReady(&HdmiTxSs)) {
		/* Set HDCP upstream interface */
		XHdcp_SetRepeater(&HdcpRepeater, FALSE);
	}
#endif
#endif

	/* Enable Scrambling Override
	 * Note: Setting the override to TRUE will allow scrambling to be
	 *       disabled for video where TMDS Clock > 340 MHz which breaks the
	 *       HDMI Specification
	 * E.g.:
	 *   XV_HdmiTxSs_SetVideoStreamScramblingOverrideFlag(&HdmiTxSs, TRUE);
	 */

	/* Main loop */
	do {

#ifdef USE_HDCP
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && \
		defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
		if (XV_HdmiRxSs_HdcpIsReady(&HdmiRxSs) &&
					XV_HdmiTxSs_HdcpIsReady(&HdmiTxSs)) {
#elif defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
		if (XV_HdmiTxSs_HdcpIsReady(&HdmiTxSs)) {
#elif defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
		if (XV_HdmiRxSs_HdcpIsReady(&HdmiRxSs)) {
#endif
			/* Poll HDCP */
			XHdcp_Poll(&HdcpRepeater);
		}
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
		SinkReady = SinkReadyCheck(&HdmiTxSs, &EdidHdmi20_t);

		if (StartTxAfterRxFlag && SinkReady) {
			StartTxAfterRx();
		}

		else if (TxRestartColorbar && SinkReady) {
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
			if (!PLLBondedCheck()) {
#endif
				/* Clear TxRestartColorbar Flag */
				TxRestartColorbar = (FALSE);
				HdmiTxSsVidStreamPtr =
					XV_HdmiTxSs_GetVideoStream(&HdmiTxSs);
				EnableColorBar(&Vphy,
					&HdmiTxSs,
					HdmiTxSsVidStreamPtr->VmId,
					HdmiTxSsVidStreamPtr->ColorFormatId,
					HdmiTxSsVidStreamPtr->ColorDepth);
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
			} else {
				/* Clear TxRestartColorbar Flag */
				TxRestartColorbar = (FALSE);
			}
#endif
		}

		if (IsStreamUp && SinkReady) {
			IsStreamUp = FALSE;

			i2c_dp159(&Vphy, 0, TxLineRate);
			XVphy_Clkout1OBufTdsEnable
				(&Vphy, XVPHY_DIR_TX, (TRUE));
		}
#endif

		/* HDMI Menu */
		XHdmi_MenuProcess(&HdmiMenu);

		/* VPHY error */
		VphyProcessError();

#ifdef XPAR_XI2SRX_NUM_INSTANCES

		i2s_audio(XVphy_ClkDetGetRefClkFreqHz(&Vphy, XVPHY_DIR_RX),
				(&Vphy)->HdmiRxTmdsClockRatio,
				tx_is_up,
				rx_is_up, (&HdmiTxSs),
				(&HdmiRxSs),
				(&AudioGen));
#endif
	}
	while (1);

	return 0;
}
