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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* 2.13  YH     03/01/16 Fixed a system hang issue by clearing TxBusy flag when a
*                            non-supportedvideo resolution is set
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
#define APP_MAJ_VERSION 3
#define APP_MIN_VERSION 0

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int I2cMux(void);
int I2cClk(u32 InFreq, u32 OutFreq);

#if defined (__arm__) && (!defined(ARMR5))
int OnBoardSi5324Init(void);
#endif

void Info(void);



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

// Needed for ZCU106 RevB
#if defined (ARMR5) || (__aarch64__)
void Disable_TMDS181_HPD_passthrough();
#define TMDS181_ADDR    0x5c
#endif

/************************* Variable Definitions *****************************/
/* VPHY structure */
XVphy              Vphy;
u8                 VphyErrorFlag;
u8                 VphyPllLayoutErrorFlag;


#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/* HDMI RX SS structure */
XV_HdmiRxSs        HdmiRxSs;
XV_HdmiRxSs_Config *XV_HdmiRxSs_ConfigPtr;
#endif

#ifdef XPAR_XGPIO_NUM_INSTANCES
XGpio              Gpio_Tpg_resetn;
XGpio_Config       *Gpio_Tpg_resetn_ConfigPtr;
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

/*HDMI Application Menu: Data Structure*/
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
u8 PLLBondedCheck (void)
{
	// The colorbar can only be displayed when the GT is not bonded.
	if (XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)) {
		xil_printf("\r\nThe GT TX and RX are bonded and clocked by "
				   "the RX clock.\r\n");
		xil_printf("Please select independent PLL layout to enable "
				   "TX only mode.\r\n");
	}
	return (XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1));
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

		Status = Si5324_SetClock((XPAR_IIC_0_BASEADDR), (I2C_CLK_ADDR),
								 (SI5324_CLKSRC_XTAL), (SI5324_XTAL_FREQ), OutFreq);
		if (Status != (SI5324_SUCCESS)) {
			xil_printf("Error programming SI5324\r\n");
			return 0;
		}
	}

	/* Locked mode */
	else {
		Status = Si5324_SetClock((XPAR_IIC_0_BASEADDR), (I2C_CLK_ADDR),
								 (SI5324_CLKSRC_CLK1), InFreq, OutFreq);
		if (Status != (SI5324_SUCCESS)) {
			xil_printf("Error programming SI5324\r\n");
			return 0;
		}
	}
#else
	// Reset I2C controller before issuing new transaction. This is required to
	// recover the IIC controller in case a previous transaction is pending.
	XIic_WriteReg(XPAR_IIC_0_BASEADDR, XIIC_RESETR_OFFSET,
				  XIIC_RESET_MASK);

	/* Free running mode */
	if (InFreq == 0) {
		Status = IDT_8T49N24x_SetClock((XPAR_IIC_0_BASEADDR), (I2C_CLK_ADDR),
									   (IDT_8T49N24X_XTAL_FREQ), OutFreq, TRUE);

		if (Status != (XST_SUCCESS)) {
			print("Error programming IDT_8T49N241\n\r");
			return 0;
		}
	}

	/* Locked mode */
	else {
		Status = IDT_8T49N24x_SetClock((XPAR_IIC_0_BASEADDR), (I2C_CLK_ADDR),
									   InFreq, OutFreq, FALSE);

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
	Status = XIicPs_MasterSendPolled(&Ps_Iic1, (u8 *)&Buffer, 1, I2C_MUX_ADDR);

	return Status;
}

int I2cClk_Ps(u32 InFreq, u32 OutFreq)
{
	int Status;

	/* Free running mode */
	if (InFreq == 0) {

		Status = Si5324_SetClock_Ps(&Ps_Iic1, (0x69),
									(SI5324_CLKSRC_XTAL), (SI5324_XTAL_FREQ), OutFreq);

		if (Status != (SI5324_SUCCESS)) {
			print("Error programming SI5324\r\n");
			return 0;
		}
	}

	/* Locked mode */
	else {
		Status = Si5324_SetClock_Ps(&Ps_Iic1, (0x69),
									(SI5324_CLKSRC_CLK1), InFreq, OutFreq);

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
	Status = Si5324_SetClock((XPAR_IIC_1_BASEADDR), (I2C_CLK_ADDR),
							 (SI5324_CLKSRC_XTAL), (SI5324_XTAL_FREQ), (u32)125000000);

	if (Status != (SI5324_SUCCESS)) {
		xil_printf("Error programming On-Board SI5324\r\n");
		return 0;
	}

	return Status;
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

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	XV_HdmiRxSs_ReportInfo(&HdmiRxSs);
#endif
#ifdef VIDEO_FRAME_CRC_EN
	XVidFrameCrc_Report();
#endif

	// GT
	xil_printf("------------\r\n");
	xil_printf("HDMI PHY\r\n");
	xil_printf("------------\r\n");
	Data = XVphy_GetVersion(&Vphy);
	xil_printf("  VPhy version : %02d.%02d (%04x)\r\n",
			   ((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
	xil_printf("\r\n");
	xil_printf("GT status\r\n");
	xil_printf("---------\r\n");
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	xil_printf("RX reference clock frequency: %0d Hz\r\n",
			   XVphy_ClkDetGetRefClkFreqHz(&Vphy, XVPHY_DIR_RX));
	if(Vphy.Config.DruIsPresent == (TRUE)) {
		xil_printf("DRU reference clock frequency: %0d Hz\r\n",
				   XVphy_DruGetRefClkFreqHz(&Vphy));
	}
#endif
	XVphy_HdmiDebugInfo(&Vphy, 0, XVPHY_CHANNEL_ID_CH1);

}


#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
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

	// RX cable is disconnected
	if(HdmiRxSsPtr->IsStreamConnected == (FALSE)) {
		Vphy.HdmiRxTmdsClockRatio = 0; // Clear GT RX TMDS clock ratio

#if(LOOPBACK_MODE_EN != 1)
		/* Check for Pass-through */
		/* Doesnt require to restart colorbar
		 * if the system is in colorbar mode
		 */
		if (IsPassThrough) {
			/* Clear pass-through flag
			 * as the system is in TX-Only
			 * mode
			 */
			IsPassThrough = (FALSE);
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
* This function is called when a RX Bridge Overflow event has occurred.
* This is Error Condition
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxBrdgOverflowCallback(void *CallbackRef) {
	xil_printf(ANSI_COLOR_YELLOW "RX Video Bridge Overflow"
			   ANSI_COLOR_RESET "\r\n");
/*	XV_HdmiRx_VideoEnable(HdmiRxSs.HdmiRxPtr, (FALSE));
	XV_HdmiRx_VideoEnable(HdmiRxSs.HdmiRxPtr, (TRUE));*/
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

	RxPllType = XVphy_GetPllType(VphyPtr, 0, XVPHY_DIR_RX,
								 XVPHY_CHANNEL_ID_CH1);
	if (!(RxPllType == XVPHY_PLL_TYPE_CPLL)) {
		XV_HdmiRxSs_SetStream(&HdmiRxSs, VphyPtr->HdmiRxRefClkHz,
			(XVphy_GetLineRateHz(&Vphy, 0, XVPHY_CHANNEL_ID_CMN0)/1000000));

	} else {
		XV_HdmiRxSs_SetStream(&HdmiRxSs, VphyPtr->HdmiRxRefClkHz,
			 (XVphy_GetLineRateHz(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)/1000000));
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
* 		  on the error type. For example, XVPHY_ERR_PLL_LAYOUT can be used to
* 		  automatically switch in and out of bonded mode for GTXE2 devices
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
				(Vphy.Quads[0].Plls[0].RxState == XVPHY_GT_STATE_READY)) {

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
#endif

#ifdef USE_HDCP
	/* Call HDCP stream-down callback */
	XHdcp_StreamDownCallback(&HdcpRepeater);
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
    // xil_printf("RxStreamInitCallback\r\n");
	// Calculate RX MMCM parameters
	// In the application the YUV422 colordepth is 12 bits
	// However the HDMI transports YUV422 in 8 bits.
	// Therefore force the colordepth to 8 bits when the colorspace is YUV422

	HdmiRxSsVidStreamPtr = XV_HdmiRxSs_GetVideoStream(HdmiRxSsPtr);

	if (HdmiRxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_YCRCB_422) {
		Status = XVphy_HdmiCfgCalcMmcmParam(&Vphy, 0, XVPHY_CHANNEL_ID_CH1,
											XVPHY_DIR_RX,
											HdmiRxSsVidStreamPtr->PixPerClk,
											XVIDC_BPC_8);
	}

	// Other colorspaces
	else {
		Status = XVphy_HdmiCfgCalcMmcmParam(&Vphy, 0, XVPHY_CHANNEL_ID_CH1,
											XVPHY_DIR_RX,
											HdmiRxSsVidStreamPtr->PixPerClk,
											HdmiRxSsVidStreamPtr->ColorDepth);
	}

	if (Status == XST_FAILURE) {
		return;
	}

	// Enable and configure RX MMCM
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
	XVidC_ReportStreamInfo(&HdmiRxSsPtr->HdmiRxPtr->Stream.Video);


#ifdef USE_HDCP
	/* Call HDCP stream-up callback */
	XHdcp_StreamUpCallback(&HdcpRepeater);
#endif

#ifdef VIDEO_FRAME_CRC_EN
	/* Reset Video Frame CRC */
	XVidFrameCrc_Reset();
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
//  Status = XIntc_Start(IntcInstPtr, XIN_SIMULATION_MODE);
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
	xil_printf("--------------------------------------\r\n");
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

	Status = XIicPs_CfgInitialize(&Ps_Iic0, XIic0Ps_ConfigPtr, XIic0Ps_ConfigPtr->BaseAddress);
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

	Status = XIicPs_CfgInitialize(&Ps_Iic1, XIic1Ps_ConfigPtr, XIic1Ps_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIicPs_Reset(&Ps_Iic1);
	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Ps_Iic1, PS_IIC_CLK);

	// On Board SI5328 chip for DRU reference clock
	I2cMux_Ps();
	/* Initialize external clock generator */
	Si5324_Init_Ps(&Ps_Iic1, 0x69);

	I2cClk_Ps(0, 156250000);

	// Delay 50ms to allow SI chip to lock
	usleep (50000);

#endif

	/* Initialize external clock generator */
#if (!defined XPS_BOARD_ZCU104)
	Si5324_Init(XPAR_IIC_0_BASEADDR, I2C_CLK_ADDR);
#else
	usleep(200000);
	IDT_8T49N24x_Init(XPAR_IIC_0_BASEADDR, I2C_CLK_ADDR);
#endif

	/* Disable TMDS181 HPD passthrough for ZCU106 Rev B and below */
	//Disable_TMDS181_HPD_passthrough();

	/* Load HDCP keys from EEPROM */
#if defined (XPAR_XHDCP_NUM_INSTANCES) || defined (XPAR_XHDCP22_RX_NUM_INSTANCES) || defined (XPAR_XHDCP22_TX_NUM_INSTANCES)
	if (XHdcp_LoadKeys(Hdcp22Lc128, sizeof(Hdcp22Lc128),
					   Hdcp22RxPrivateKey, sizeof(Hdcp22RxPrivateKey),
					   Hdcp14KeyA, sizeof(Hdcp14KeyA),
					   Hdcp14KeyB, sizeof(Hdcp14KeyB)) == XST_SUCCESS) {

		/* Set pointers to HDCP 2.2 Keys */
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
#if XPAR_XHDCP22_RX_NUM_INSTANCES
		XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS_KEY_HDCP22_LC128, Hdcp22Lc128);
		XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS_KEY_HDCP22_PRIVATE, Hdcp22RxPrivateKey);
#endif
#endif

		/* Set pointers to HDCP 1.4 keys */
#if XPAR_XHDCP_NUM_INSTANCES
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS_KEY_HDCP14, Hdcp14KeyB);
#endif

		/* Initialize key manager */

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		Status = XHdcp_KeyManagerInit(XPAR_HDCP_KEYMNGMT_BLK_1_BASEADDR, HdmiRxSs.Hdcp14KeyPtr);
		if (Status != XST_SUCCESS) {
			xil_printf("HDCP 1.4 RX Key Manager Initialization error\r\n");
			return XST_FAILURE;
		}
#endif
#endif

	}

	/* Clear pointers */
	else {

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		/* Set pointer to NULL */
		XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS_KEY_HDCP22_LC128, (NULL));

		/* Set pointer to NULL */
		XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS_KEY_HDCP22_PRIVATE, (NULL));

		/* Set pointer to NULL */
		XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS_KEY_HDCP14, (NULL));
#endif


	}
#endif


#if defined (__arm__) && (!defined(ARMR5))
	/* Initialize on-board clock generator */
	OnBoardSi5324Init();

	// Delay 15ms to allow SI chip to lock
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


#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	/////
	// Initialize HDMI RX Subsystem
	/////
	/* Get User Edid Info */
	XV_HdmiRxSs_SetEdidParam(&HdmiRxSs, (u8*)&Edid, sizeof(Edid));
	XV_HdmiRxSs_ConfigPtr =
		XV_HdmiRxSs_LookupConfig(XPAR_XV_HDMIRX_0_DEVICE_ID);

	if(XV_HdmiRxSs_ConfigPtr == NULL) {
		HdmiRxSs.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	//Initialize top level and all included sub-cores
	Status = XV_HdmiRxSs_CfgInitialize(&HdmiRxSs, XV_HdmiRxSs_ConfigPtr,
									   XV_HdmiRxSs_ConfigPtr->BaseAddress);
	if(Status != XST_SUCCESS) {
		xil_printf("ERR:: HDMI RX Subsystem Initialization failed %d\r\n", Status);
		return(XST_FAILURE);
	}

	/* Set the Application version in RXSs driver structure */
	XV_HdmiRxSS_SetAppVersion(&HdmiRxSs, APP_MAJ_VERSION, APP_MIN_VERSION);

	//Register HDMI RX SS Interrupt Handler with Interrupt Controller
#if defined(__arm__) || (__aarch64__)
	Status |= XScuGic_Connect(&Intc,
							  XPAR_FABRIC_V_HDMI_RX_SS_IRQ_INTR,
							  (XInterruptHandler)XV_HdmiRxSS_HdmiRxIntrHandler,
							  (void *)&HdmiRxSs);

#ifdef XPAR_XHDCP_NUM_INSTANCES
	// HDCP 1.4 Cipher interrupt
	Status |= XScuGic_Connect(&Intc,
							  XPAR_FABRIC_V_HDMI_RX_SS_HDCP14_IRQ_INTR,
							  (XInterruptHandler)XV_HdmiRxSS_HdcpIntrHandler,
							  (void *)&HdmiRxSs);

	Status |= XScuGic_Connect(&Intc,
							  XPAR_FABRIC_V_HDMI_RX_SS_HDCP14_TIMER_IRQ_INTR,
							  (XInterruptHandler)XV_HdmiRxSS_HdcpTimerIntrHandler,
							  (void *)&HdmiRxSs);
#endif

#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
	//HDCP 2.2 Timer interrupt */
	Status |= XScuGic_Connect(&Intc,
							  XPAR_FABRIC_V_HDMI_RX_SS_HDCP22_TIMER_IRQ_INTR,
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
	// HDCP 1.4 Cipher interrupt
	Status |= XIntc_Connect(&Intc,
							XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_IRQ_VEC_ID,
							(XInterruptHandler)XV_HdmiRxSS_HdcpIntrHandler,
							(void *)&HdmiRxSs);

	// HDCP 1.4 Timer interrupt
	Status |= XIntc_Connect(&Intc,
							XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_TIMER_IRQ_VEC_ID,
							(XInterruptHandler)XV_HdmiRxSS_HdcpTimerIntrHandler,
							(void *)&HdmiRxSs);
#endif

#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
	// HDCP 2.2 Timer interrupt
	Status |= XIntc_Connect(&Intc,
							XPAR_INTC_0_V_HDMIRXSS_0_HDCP22_TIMER_IRQ_VEC_ID,
							(XInterruptHandler)XV_HdmiRxSS_Hdcp22TimerIntrHandler,
							(void *)&HdmiRxSs);
#endif

#endif

	if (Status == XST_SUCCESS) {
#if defined(__arm__) || (__aarch64__)
		XScuGic_Enable(&Intc,
					   XPAR_FABRIC_V_HDMI_RX_SS_IRQ_INTR);
#ifdef XPAR_XHDCP_NUM_INSTANCES
		XScuGic_Enable(&Intc,
					   XPAR_FABRIC_V_HDMI_RX_SS_HDCP14_IRQ_INTR);
		XScuGic_Enable(&Intc,
					   XPAR_FABRIC_V_HDMI_RX_SS_HDCP14_TIMER_IRQ_INTR);
#endif
#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
		XScuGic_Enable(&Intc,
					   XPAR_FABRIC_V_HDMI_RX_SS_HDCP22_TIMER_IRQ_INTR);
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
		// HDCP 1.4 Cipher interrupt
		XIntc_Enable(&Intc,
					 XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_IRQ_VEC_ID);

		// HDCP 1.4 Timer interrupt
		XIntc_Enable(&Intc,
					 XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_TIMER_IRQ_VEC_ID);
#endif

#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
		// HDCP 2.2 Timer interrupt
		XIntc_Enable(&Intc,
					 XPAR_INTC_0_V_HDMIRXSS_0_HDCP22_TIMER_IRQ_VEC_ID);
#endif

#endif
	} else {
		xil_printf("ERR:: Unable to register HDMI RX interrupt handler");
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
	//XV_HdmiRxSs_SetCallback(&HdmiRxSs,
	//	  	  	  	  	  	  	  XV_HDMIRXSS_HANDLER_DDC,
	//	  	  	  	  	  	  	  RxDdcCallback,
	//	  	  	  	  	  	  	  (void *)&HdmiRxSs);
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

	/////
	// Initialize Video PHY
	// The GT needs to be initialized after the HDMI RX and TX.
	// The reason for this is the GtRxInitStartCallback
	// calls the RX stream down callback.
	/////

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


	xil_printf("---------------------------------\r\n");

	/* Enable exceptions. */
	Xil_AssertSetCallback((Xil_AssertCallback) Xil_AssertCallbackRoutine);
	Xil_ExceptionEnable();

	// Initialize menu
	XHdmi_MenuInitialize(&HdmiMenu, UART_BASEADDR);



	/* Enable Scrambling Override
	 * Note: Setting the override to TRUE will allow scrambling to be disabled
	 * 		 for video where TMDS Clock > 340 MHz which breaks the HDMI Specification
	 */
	// XV_HdmiTxSs_SetVideoStreamScramblingOverrideFlag(&HdmiTxSs, TRUE);

	/* Main loop */
	do {

#ifdef USE_HDCP
		if (XV_HdmiRxSs_HdcpIsReady(&HdmiRxSs)) {
			/* Poll HDCP */
			XHdcp_Poll(&HdcpRepeater);
		}
#endif


		// HDMI menu
		XHdmi_MenuProcess(&HdmiMenu);

		/* VPHY error */
		VphyProcessError();

	}
	while (1);

	return 0;
}
