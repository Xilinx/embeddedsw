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
* @file xhdmi_edid.c
*
* This file demonstrates application usage for EDID
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         19/02/18 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include "xhdmi_edid.h"
/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Function Definitions *****************************/
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function handles EDID reading and accessing the SCDC register during
* cable connect state
*
* @param  HdmiTxSsPtr HDMI TX Subsystem Pointer
* @param  CheckHdmi20Param Application Data Structure (EDID & HDMI 2.0 Check)
*
* @return
*   -NA
*
* @note   None.
*
******************************************************************************/
void EdidScdcCheck(XV_HdmiTxSs          *HdmiTxSsPtr,
                   EdidHdmi20           *CheckHdmi20Param)
{
    u8 Buffer [256];
    int Status;

    /*Below Check executed only when TX Cable Connect*/
    if (CheckHdmi20Param->EdidCableConnectRead) {
        /*Read & Parse the EDID upon the Cable Connect to check
		Sink's Capability*/
	Status = XV_HdmiTxSs_ReadEdid(HdmiTxSsPtr, (u8*)&Buffer);
	/* Only Parse the EDID when the Read EDID success */
	if (Status == XST_SUCCESS) {
		XV_VidC_parse_edid((u8*)&Buffer,
				&CheckHdmi20Param->EdidCtrlParam,
				XVIDC_VERBOSE_DISABLE);
	}

        /*Check whether the Sink able to support HDMI 2.0 by checking the
        maximum supported video bandwidth*/
        if (CheckHdmi20Param->EdidCtrlParam.MaxTmdsMhz >= 340) {
            /*Sink's EDID indicated HDMI 2.0 Capable*/
            CheckHdmi20Param->IsHDMI20SinkCapable = (TRUE);

            /*Check whether the SCDC is present or not (HF-VSDB EDID)*/
            if(CheckHdmi20Param->EdidCtrlParam.IsSCDCPresent ==
															XVIDC_SUPPORTED) {
                /*Check whether the SCDC is capable of initiating
				Read Request*/
                if (CheckHdmi20Param->EdidCtrlParam.IsSCDCReadRequestReady ==
                        XVIDC_SUPPORTED) {
                    Status = XV_HdmiTxSs_DetectHdmi20(HdmiTxSsPtr);
                    if (!Status) {
                        CheckHdmi20Param->IsReReadSinkEdid = (FALSE);
                    } else {
                        /*Fail: EDID and the Actual behavior mismatch*/
                        CheckHdmi20Param->IsReReadSCDC = (TRUE);
                        CheckHdmi20Param->HdmiSinkWarningFlag |=
                        		XV_HDMI_SINK_EDID_20_VSDB20_ACC_SCDC_FAIL;
                    }
                } else {
                    /*SCDC register is not ready for initiating read request*/
                    CheckHdmi20Param->IsReReadSinkEdid = (TRUE);
                }
            } else {
            	CheckHdmi20Param->HdmiSinkWarningFlag |=
            			XV_HDMI_SINK_EDID_20_VSDB20_NA_SCDC_PASS;

                /*Since the SCDC is not present, hence doesn't require,
                re-reading of the EDID.*/
                CheckHdmi20Param->IsReReadSinkEdid = (FALSE);

                /*Try to access the SCDC even the EDID declared
                SCDC not present, since it's a HDMI 2.0 capable
                Notes:
                Some commercial sink's has behavior of no EDID:HF-VSDB
                but the SCDC is accessible*/
                Status = XV_HdmiTxSs_DetectHdmi20(HdmiTxSsPtr);
                if (Status) {
                    CheckHdmi20Param->IsReReadSCDC = (TRUE);
                }
            }
        } else {
            /*Check whether the SCDC is accessible even though
              the EDID indicates non HDMI 2.0 Capable
              Notes:
              Some commercial sink's has behavior of when the
              EDID declared "HDMI2.0 not supported" but the SCDC are
              accessible*/
            Status = XV_HdmiTxSs_DetectHdmi20(HdmiTxSsPtr);
            if (!Status) {
                /*Since the SCDC is accessible assume the Sink can support
                HDMI 2.0*/
                CheckHdmi20Param->IsHDMI20SinkCapable = (TRUE);
                CheckHdmi20Param->HdmiSinkWarningFlag |=
                		XV_HDMI_SINK_EDID_14_SCDC_PASS;
            } else {
				/*The Sink are HDMI 1.4*/
				CheckHdmi20Param->IsHDMI20SinkCapable = (FALSE);
				CheckHdmi20Param->HdmiSinkWarningFlag |=
						XV_HDMI_SINK_20_NOT_CAPABLE;
            }
            CheckHdmi20Param->IsReReadSinkEdid = (FALSE);
            CheckHdmi20Param->IsReReadSCDC = (FALSE);
        }
        CheckHdmi20Param->EdidCableConnectRead =(FALSE);
    } else {
        /*If retry Re Read EDID or Re Access SCDC is enabled*/
        if (CheckHdmi20Param->IsReReadSinkEdid) {
            /*Read & Parse the EDID upon the Cable Connect to check
															  Sink Capability*/
            XV_HdmiTxSs_ReadEdid(HdmiTxSsPtr, (u8*)&Buffer);
            XV_VidC_parse_edid((u8*)&Buffer, &CheckHdmi20Param->EdidCtrlParam,
                                XVIDC_VERBOSE_DISABLE);

            if(CheckHdmi20Param->EdidCtrlParam.IsSCDCPresent ==
															XVIDC_SUPPORTED) {
                if (CheckHdmi20Param->EdidCtrlParam.IsSCDCReadRequestReady ==
                        XVIDC_SUPPORTED) {
                    CheckHdmi20Param->IsReReadSinkEdid = (FALSE);
                    Status = XV_HdmiTxSs_DetectHdmi20(HdmiTxSsPtr);
                    if (Status != XST_SUCCESS) {
                        CheckHdmi20Param->IsReReadSCDC = (TRUE);
                    }
                }
            }
        }

        if (CheckHdmi20Param->IsReReadSCDC) {
            Status = XV_HdmiTxSs_DetectHdmi20(HdmiTxSsPtr);
            if (Status == XST_SUCCESS) {
                CheckHdmi20Param->IsReReadSCDC = (FALSE);
            }
        }
    }
}

/*****************************************************************************/
/**
*
* This function re-reads EDID and re-parse the SCDC for a period of time
*   There are sink's or monitor have different startup time hence providing
*   multiple retry
*
* @param  HdmiTxSsPtr HDMI TX Subsystem Pointer
* @param  CheckHdmi20Param Application Data Structure (EDID & HDMI 2.0 Check)
*
* @return
*   -NA
*
* @note   None.
*
******************************************************************************/
u8 SinkReadyCheck (XV_HdmiTxSs          *HdmiTxSsPtr,
                    EdidHdmi20           *CheckHdmi20Param)
{
	u8 SinkReadyStatus;

    if (CheckHdmi20Param->IsReReadSinkEdid || CheckHdmi20Param->IsReReadSCDC) {
    	CheckHdmi20Param->SinkCheckRetryCount =
    			CheckHdmi20Param->SinkCheckRetryCount + 1;
        if (CheckHdmi20Param->SinkCheckRetryCount
        							== READINTERVAL) {
            /* Re-Read the EDID */
            EdidScdcCheck(HdmiTxSsPtr,CheckHdmi20Param);
            /*High Priority iif the Sink EDID not Ready */
            if (CheckHdmi20Param->IsReReadSinkEdid) {
            	CheckHdmi20Param->IsReReadSinkEdidRetry =
            			CheckHdmi20Param->IsReReadSinkEdidRetry + 1;
            	CheckHdmi20Param->SinkCheckRetryCount = 0;
            }
            /* 2nd Priority if the SCDC is not Ready */
            /*Try to read the SCDC Register even the read request not capable
                in EDID is not asserted,
                Notes: Some commercial sink's not updating this
                EDID: HF-VSDB: RR_Capable bit */
            else if (CheckHdmi20Param->IsReReadSCDC) {
            	CheckHdmi20Param->IsReReadScdcRetry =
            			CheckHdmi20Param->IsReReadScdcRetry + 1;
            	CheckHdmi20Param->SinkCheckRetryCount = 0;
            }
        }

        if (CheckHdmi20Param->IsReReadSinkEdidRetry == READEDIDRETRY) {
        	CheckHdmi20Param->IsReReadSinkEdidRetry = 0;
            CheckHdmi20Param->IsReReadSinkEdid = (FALSE);
            CheckHdmi20Param->IsReReadSCDC = (TRUE);
            CheckHdmi20Param->HdmiSinkWarningFlag |=
            		XV_HDMI_SINK_EDID_SCDC_MISMATCH;
        }

        if (CheckHdmi20Param->IsReReadScdcRetry == READSCDCRETRY) {
        	CheckHdmi20Param->IsReReadScdcRetry = 0;
            CheckHdmi20Param->IsReReadSCDC = (FALSE);
            CheckHdmi20Param->HdmiSinkWarningFlag |=
            		XV_HDMI_SINK_EDID_20_VSDB20_ACC_SCDC_FAIL;
        }
    }
    SinkReadyStatus = (!CheckHdmi20Param->IsReReadSinkEdid &&
    							!CheckHdmi20Param->IsReReadSCDC) ?
    									(TRUE):(FALSE);
    return (SinkReadyStatus);
}

/*****************************************************************************/
/**
*
* This function initialize the EDID Application Data Structure
* This function should be called only once when the HDMI cable is connected
*
* @param  CheckHdmi20Param Application Data Structure (EDID & HDMI 2.0 Check)
*
* @return
*   -NA
*
* @note   None.
*
******************************************************************************/
void EDIDConnectInit(EdidHdmi20           *CheckHdmi20Param){
	/*Enable EDID Read During Cable Connect*/
	CheckHdmi20Param->EdidCableConnectRead = (TRUE);

	CheckHdmi20Param->IsHDMI20SinkCapable  = (FALSE);
	CheckHdmi20Param->IsReReadSCDC         = (FALSE);
	CheckHdmi20Param->IsReReadSinkEdid     = (FALSE);
	CheckHdmi20Param->SinkCheckRetryCount  = 0;
	CheckHdmi20Param->IsReReadSinkEdidRetry= 0;
	CheckHdmi20Param->IsReReadScdcRetry    = 0;
	CheckHdmi20Param->HdmiSinkWarningFlag  = XV_HDMI_SINK_NO_WARNINGS;
											 /*No Warnings*/
}

/*****************************************************************************/
/**
*
* This function check's parsed EDID value, and perform it's functionality
* based on application.
*
* @param  CheckHdmi20Param Application Data Structure (EDID & HDMI 2.0 Check)
*
* @return
*   -NA
*
* @note   None.
*
******************************************************************************/
void SinkCapabilityCheck(EdidHdmi20 *CheckHdmi20Param){
	if (!CheckHdmi20Param->EdidCtrlParam.Is48bppSupp) {
		CheckHdmi20Param->HdmiSinkWarningFlag |=
				XV_HDMI_SINK_DEEP_COLOR_16_NOT_SUPP;
	}
	if (!CheckHdmi20Param->EdidCtrlParam.Is36bppSupp) {
		CheckHdmi20Param->HdmiSinkWarningFlag |=
				XV_HDMI_SINK_DEEP_COLOR_12_NOT_SUPP;
	}
	if (!CheckHdmi20Param->EdidCtrlParam.Is30bppSupp) {
		CheckHdmi20Param->HdmiSinkWarningFlag |=
				XV_HDMI_SINK_DEEP_COLOR_10_NOT_SUPP;
	}
	if (!CheckHdmi20Param->EdidCtrlParam.IsHdmi) {
		CheckHdmi20Param->HdmiSinkWarningFlag |=
				XV_SINK_NOT_HDMI;
	}
}

/*****************************************************************************/
/**
*
* This function prints Sink Capability Warning Message
*
* @param  CheckHdmi20Param Application Data Structure (EDID & HDMI 2.0 Check)
*
* @return
*   -NA
*
* @note   None.
*
******************************************************************************/
void SinkCapWarningMsg(EdidHdmi20 *CheckHdmi20Param){
	if (CheckHdmi20Param->HdmiSinkWarningFlag &
			XV_HDMI_SINK_EDID_SCDC_MISMATCH) {
			xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's EDID "
			   "indicates HDMI 2.0 capable, but the SCDC read request "
			   "register bit (VSDB:RR_Capable) is not asserted"
			   ANSI_COLOR_RESET "\r\n");
	}
	if (CheckHdmi20Param->HdmiSinkWarningFlag &
			XV_HDMI_SINK_EDID_20_VSDB20_NA_SCDC_PASS) {
			xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's "
			   "EDID indicates HDMI 2.0 capable but SCDC is not "
			   "present (EDID: HF-VSDB)" ANSI_COLOR_RESET "\r\n");
	}
	if (CheckHdmi20Param->HdmiSinkWarningFlag &
			XV_HDMI_SINK_EDID_20_VSDB20_ACC_SCDC_FAIL) {
			xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's "
				"EDID indicates HDMI 2.0 capable, SCDC present "
				"and capable of initiating read request, however "
				"the SCDC is inaccessible" ANSI_COLOR_RESET "\r\n");
	}
	if (CheckHdmi20Param->HdmiSinkWarningFlag &
			XV_HDMI_SINK_EDID_14_SCDC_PASS) {
			xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's "
			"EDID indicates HDMI 2.0 not capable"
					ANSI_COLOR_RESET "\r\n");
	}
	if (CheckHdmi20Param->HdmiSinkWarningFlag &
			XV_HDMI_SINK_DEEP_COLOR_10_NOT_SUPP) {
			xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's "
			"EDID indicates Deep Color of 10 BpC Not Supported"
					ANSI_COLOR_RESET "\r\n");
	}
	if (CheckHdmi20Param->HdmiSinkWarningFlag &
			XV_HDMI_SINK_DEEP_COLOR_12_NOT_SUPP) {
		xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's "
		"EDID indicates Deep Color of 12 BpC Not Supported"
				ANSI_COLOR_RESET "\r\n");
	}
	if (CheckHdmi20Param->HdmiSinkWarningFlag &
			XV_HDMI_SINK_DEEP_COLOR_16_NOT_SUPP) {
		xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's "
		"EDID indicates Deep Color of 16 BpC Not Supported"
				ANSI_COLOR_RESET "\r\n");
	}
	if (CheckHdmi20Param->HdmiSinkWarningFlag &
			XV_SINK_NOT_HDMI) {
		xil_printf(ANSI_COLOR_YELLOW "Warning: Connected Sink's "
		"EDID indicates HDMI is not Supported"
				ANSI_COLOR_RESET "\r\n");
	}
}
#endif
