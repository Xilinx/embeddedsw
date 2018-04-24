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
* @file xhdmi_edid.h
*
* This file contains set of EDID demonstrates different capability
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         12/02/18 Initial release.
* 1.01  EB     05/04/18 Updated EDID
* </pre>
*
******************************************************************************/
#ifndef _XHDMI_EDID_H_
/**  prevent circular inclusions by using protection macros */
#define _XHDMI_EDID_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xvidc_edid_ext.h"
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#include "xv_hdmitxss.h"
#endif

/************************** Constant Definitions *****************************/
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*Magic Number: Maximum Number of Retries for EDID Read*/
#define READEDIDRETRY 5
/*Magic Number: Maximum Number of Retries for SCDC Read*/
#define READSCDCRETRY 5
/*Magic Number: Maximum Number of Retries for EDID Read Interval*/
#define READINTERVAL 25000

/**
* These constants specify the flags of warning msg
*/
#define	XV_HDMI_SINK_NO_WARNINGS                  0x00
#define XV_HDMI_SINK_EDID_SCDC_MISMATCH           0x01
	/* Sink's EDID indicates supported TMDS more than 340Mbps
	 * and the HF-VSDB HDMI 2.0 is available but the
	 * RR_Capable/Read Ready is not asserted */
#define	XV_HDMI_SINK_EDID_20_VSDB20_NA_SCDC_PASS  0x02
	/* Sink's EDID indicates supported TMDS more than 340Mbps
	 * and the HF-VSDB HDMI 2.0 is not available but the SCDC
	 * is accessible*/
#define	XV_HDMI_SINK_EDID_20_VSDB20_ACC_SCDC_FAIL 0x04
	/* Sink's EDID indicates supported TMDS more than 340Mbps
	 * and the HF-VSDB HDMI 2.0 is available but the SCDC
	 * is not accessible*/
#define	XV_HDMI_SINK_EDID_14_SCDC_PASS            0x08
	/* Sink's EDID indicates supported TMDS less than 340Mbps
	 * (HDMI 1.4) but the SCDC Register is accessible*/
#define	XV_HDMI_SINK_20_NOT_CAPABLE               0x10
	/*Sink not HDMI 2.0 Capable */
#define	XV_HDMI_SINK_DEEP_COLOR_10_NOT_SUPP       0x20
	/*Deep Color 10bpc is not supported */
#define	XV_HDMI_SINK_DEEP_COLOR_12_NOT_SUPP       0x40
	/*Deep Color 12bpc is not supported */
#define XV_HDMI_SINK_DEEP_COLOR_16_NOT_SUPP       0x80
	/*Deep Color 16bpc is not supported */
#define XV_SINK_NOT_HDMI                          0x100
	/*Sink is not HDMI */

/*EDID Parsing Data Structure (Application)*/
typedef struct {
	XV_VidC_EdidCntrlParam EdidCtrlParam;
	/*Control Parameter from the EDID Driver*/

	u32 HdmiSinkWarningFlag;
	/*Hdmi Sink Warning Flag*/

	/*Scratch Pad */
	u8 EdidCableConnectRead;
	/* Flag indicate EDID Read during cable
	 * connect
	 */
	u8 IsReReadSinkEdid;
	/*Status Flag for Re-Read EDID*/
	u8 IsReReadSCDC;
	/*Status Flag for Re-Read SCDC*/
	u8 IsHDMI20SinkCapable;
	/*Status Flag for HDMI 2.0 Capable*/
	u32 SinkCheckRetryCount;
	/*Counter for Re-Read EDID Interval
	 * (Magic Number)*/
	u8 IsReReadSinkEdidRetry;
	/*Counter for Flag for Re-Read EDID Retry*/
	u8 IsReReadScdcRetry;
	/*Counter for Re-Read SCDC of Sink Retry
	 * (Non-EDID related)*/
} EdidHdmi20;

EdidHdmi20 EdidHdmi20_t;
#endif
/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
void EdidScdcCheck(XV_HdmiTxSs          *HdmiTxSsPtr,
                   EdidHdmi20           *CheckHdmi20Param);

u8 SinkReadyCheck (XV_HdmiTxSs          *HdmiTxSsPtr,
                    EdidHdmi20           *CheckHdmi20Param);

void EDIDConnectInit(EdidHdmi20           *CheckHdmi20Param);
void SinkCapWarningMsg(EdidHdmi20 *CheckHdmi20Param);
void SinkCapabilityCheck(EdidHdmi20 *CheckHdmi20Param);
#endif
#ifdef __cplusplus
}
#endif

#endif /* _XHDMI_EDID_H_ */
